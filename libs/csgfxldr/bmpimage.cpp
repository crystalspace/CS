/*
    BMPImage class
    Copyright (C) 1998 by Olivier Langlois <olanglois@sympatic.ca>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <math.h>
#include <memory.h>

#include "sysdef.h"
#include "types.h"
#include "csgeom/math3d.h"
#include "csgfxldr/bmpimage.h"
#include "cssys/csendian.h"

//---------------------------------------------------------------------------

bool RegisterBMP ()
{
  static csBMPImageLoader loader;
  return csImageLoader::Register (&loader);
}

csImageFile* csBMPImageLoader::LoadImage (UByte* buf, ULong size)
{
  CHK (ImageBMPFile* i = new ImageBMPFile(buf, size));
  if (i && (i->get_status() & IFE_BadFormat))
  { 
    CHK(delete i);  
    i = NULL; 
  }
  return i;    
}

AlphaMapFile *csBMPImageLoader::LoadAlphaMap(UByte* buf,ULong size)
{
  (void) buf;
  (void) size;
  return NULL;
}

//---------------------------------------------------------------------------

//-----------------
// very private minihelpers
class minihelp{
public:
UShort* us_endian( char* ptr ){ us = convert_endian( *((UShort*)ptr) ); return &us; }
ULong* ul_endian( char* ptr ){ ul = convert_endian( *((ULong*)ptr) ); return &ul; }
long* l_endian( char* ptr ){ l = convert_endian( *((long*)ptr) ); return &l; }
ULong ul;
UShort us;
long l;
} vpmh;

#define BFTYPE(x)    vpmh.us_endian(((char *) (x)))
#define BFSIZE(x)    vpmh.ul_endian(((char *) (x))+2)
#define BFOFFBITS(x) vpmh.ul_endian(((char *) (x))+10)
#define BISIZE(x)    vpmh.ul_endian(((char *) (x))+14)
#define BIWIDTH(x)   vpmh.l_endian(((char *) (x))+18)
#define BIHEIGHT(x)  vpmh.l_endian(((char *) (x))+22)
#define BITCOUNT(x)  vpmh.us_endian(((char *) (x))+28)
#define BICOMP(x)    vpmh.ul_endian (((char *) (x))+30)
#define IMAGESIZE(x) vpmh.ul_endian(((char *) (x))+34)
#define BICLRUSED(x) vpmh.ul_endian(((char *) (x))+46)
#define BICLRIMP(x)  vpmh.ul_endian(((char *) (x))+50)
#define BIPALETTE(x) (char *)  (((char *) (x))+54)
#define PALENT(x,i)  (char *)  (((ULong *)(x))+i)

//-----------------

// Type ID
#define BM "BM" // Windows 3.1x, 95, NT, ...
#define BA "BA" // OS/2 Bitmap Array
#define CI "CI" // OS/2 Color Icon
#define CP "CP" // OS/2 Color Pointer
#define IC "IC" // OS/2 Icon
#define PT "PT" // OS/2 Pointer

// Possible values for the header size
#define WinHSize   0x28
#define OS21xHSize 0x0C
#define OS22xHSize 0xF0

// Possible values for the BPP setting
#define Mono          1  // Monochrome bitmap
#define _16Color      4  // 16 color bitmap
#define _256Color     8  // 256 color bitmap
#define HIGHCOLOR    16  // 16bit (high color) bitmap
#define TRUECOLOR24  24  // 24bit (true color) bitmap
#define TRUECOLOR32  32  // 32bit (true color) bitmap

// Compression Types
#ifndef BI_RGB
#define BI_RGB        0  // none
#define BI_RLE8       1  // RLE 8-bit / pixel
#define BI_RLE4       2  // RLE 4-bit / pixel
#define BI_BITFIELDS  3  // Bitfields
#endif

/*
#define BFTYPE(x)    (UShort *)(((char *) (x)))
#define BFSIZE(x)    (ULong *) (((char *) (x))+2)
#define BFOFFBITS(x) (ULong *) (((char *) (x))+10)
#define BISIZE(x)    (ULong *) (((char *) (x))+14)
#define BIWIDTH(x)   (long *)  (((char *) (x))+18)
#define BIHEIGHT(x)  (long *)  (((char *) (x))+22)
#define BITCOUNT(x)  (UShort *)(((char *) (x))+28)
#define BICOMP(x)    (ULong *) (((char *) (x))+30)
#define IMAGESIZE(x) (ULong *) (((char *) (x))+34)
#define BICLRUSED(x) (ULong *) (((char *) (x))+46)
#define BICLRIMP(x)  (ULong *) (((char *) (x))+50)
#define BIPALETTE(x) (char *)  (((char *) (x))+54)
#define PALENT(x,i)  (char *)  (((ULong *)(x))+i)
*/

//---------------------------------------------------------------------------

const char* ImageBMPFile::get_status_mesg() const
{
  if (status & IFE_BadFormat) 
    return "not a supported BMP format file";
  else 
    return "image successfully read";
}

ImageBMPFile::ImageBMPFile (UByte* ptr, long filesize) : csImageFile ()
{
  status = IFE_BadFormat;

  if( (memcmp( ptr, BM, 2 )==0) && 
      (*BISIZE(ptr)) == WinHSize && 
      ((*BICOMP(ptr)) == BI_RGB || (*BICOMP(ptr)) == BI_RLE8))
  {
     const int bmp_width  = (*BIWIDTH(ptr));
     const int bmp_height = (*BIHEIGHT(ptr));
     const int bmp_size   = bmp_width*bmp_height;

     set_dimensions(bmp_width, bmp_height);

     RGBPixel *bufPtr   = get_buffer();
     UByte    *iPtr     = ptr + *BFOFFBITS(ptr);
     int       pixelIdx = 0;

      // The last scanline in BMP corresponds to the top line in the image
     int buffer_y = bmp_width*(bmp_height - 1);
     int buffer_x = 0;
     bool blip = false;
/*
     if( (*BITCOUNT(ptr)) == _256Color && (*BICLRUSED(ptr)) && (*BICLRIMP(ptr)) )
     @@@ I dont know why the number of "important colors" ( BICLRIMP )  matters.
     In fact, many of my 256-color .bmp files have a ZERO there.
*/
     if( (*BITCOUNT(ptr)) == _256Color && (*BICLRUSED(ptr)) )
     {
       char *pal = BIPALETTE(ptr);

       if ( (*BICOMP(ptr)) == BI_RGB ){
         while( iPtr < ptr + filesize && pixelIdx < bmp_size )
         {
            char *palEnt = PALENT(pal,*iPtr++);
  
            RGBPixel *buf = &bufPtr[buffer_y + buffer_x];
            buf->blue    = *palEnt;
            buf->green   = *(palEnt+1);
            buf->red     = *(palEnt+2);
          
            if(++buffer_x >= bmp_width)
            {
              buffer_x  = 0;
              buffer_y -= bmp_width;
            }

            pixelIdx++;
         }
       }else if ((*BICOMP(ptr)) == BI_RLE8) {
         char *palEnt;
         UByte rl, rl1, i; // runlength
         UByte clridx, clridx1; // colorindex
         while( iPtr < ptr + filesize && pixelIdx < bmp_size )
         {
	    rl = rl1 = *iPtr++;
	    clridx = clridx1 = *iPtr++;
	    if ( rl == 0 )
	       if ( clridx == 0 ){
		  // new scanline
		  if ( !blip ){ // if we didnt already jumped to the new line, do it now
	             pixelIdx += ( bmp_width - buffer_x );
	             buffer_x  = 0;
	             buffer_y -= bmp_width;
		  }     
	          continue;
		}else if ( clridx == 1 ){
		  // end of bitmap
		  pixelIdx = bmp_size;
		  continue;
		}else if ( clridx == 2 ){
		  // next 2 bytes mean column- and scanlineoffset
		  buffer_x += *iPtr;
		  pixelIdx += *iPtr++;
		  buffer_y -= ( bmp_width * (*iPtr) );
		  pixelIdx += ( bmp_width * (*iPtr++) );
		  continue;
		}else if ( clridx > 2 )
		  rl1 = clridx;
		  
            for ( i = 0; i < rl1; i++ ){
	      if ( !rl )
	         clridx1 = *iPtr++;
	      palEnt = PALENT(pal,clridx1);	    
              RGBPixel *buf = &bufPtr[buffer_y + buffer_x];
              buf->blue    = *palEnt;
              buf->green   = *(palEnt+1);
              buf->red     = *(palEnt+2);
          
              if(++buffer_x >= bmp_width)
              {
                buffer_x  = 0;
                buffer_y -= bmp_width;
                blip = true;
	      }else
	        blip = false;

              pixelIdx++;
	    }
	    // pad in case rl == 0 and clridx in [3..255]
	    if ( rl == 0 && ( clridx & 0x01 )) iPtr++;
         }
       }
       status = IFE_OK;
     }
     else if( !(*BICLRUSED(ptr)) && (*BITCOUNT(ptr)) == TRUECOLOR24 )
     {
       while( iPtr < ptr + filesize && pixelIdx < bmp_size )
       {
          RGBPixel *buf = &bufPtr[buffer_y + buffer_x];

          buf->blue    = *iPtr;
          buf->green   = *(iPtr+1);
          buf->red     = *(iPtr+2);

          if(++buffer_x >= bmp_width)
          {
            buffer_x  = 0;
            buffer_y -= bmp_width;
          }

          pixelIdx++;
          iPtr += 3;
       }

       status = IFE_OK;
     }
  }
}


ImageBMPFile::~ImageBMPFile ()
{
}

//---------------------------------------------------------------------------
