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

//---------------------------------------------------------------------------

bool RegisterBMP ()
{
  static BMPImageLoader loader;
  return ImageLoader::Register (&loader);
}

ImageFile* BMPImageLoader::LoadImage (UByte* buf, ULong size)
{
  CHK (ImageBMPFile* i = new ImageBMPFile(buf, size));
  if (i && (i->get_status() & IFE_BadFormat))
  { CHK(delete i);  i = NULL; }
  return i;    
}

//---------------------------------------------------------------------------

// Type ID
#define BM "BM" // Windows 3.1x, 95, NT, ...
#define BA "BA" // OS/2 Bitmap Array
#define CI "CI" // OS/2 Color Icon
#define CP "CP" // OS/2 Color Pointer
#define IC "IC" // OS/2 Icon
#define PT "PT" // OS/2 Pointer

// Valeurs possibles pour le champ header size
#define WinHSize   0x28
#define OS21xHSize 0x0C
#define OS22xHSize 0xF0

// Valeurs possibles pour le champ BPP
#define Mono          1  // Monochrome bitmap
#define _16Color      4  // 16 color bitmap
#define _256Color     8  // 256 color bitmap
#define HIGHCOLOR    16  // 16bit (high color) bitmap
#define TRUECOLOR24  24  // 24bit (true color) bitmap
#define TRUECOLOR32  32  // 32bit (true color) bitmap

// Types de compression
#ifndef BI_RGB
#define BI_RGB        0  // none
#define BI_RLE8       1  // RLE 8-bit / pixel
#define BI_RLE4       2  // RLE 4-bit / pixel
#define BI_BITFIELDS  3  // Bitfields
#endif

#define BFTYPE(x) (UShort *)(((char *)(x)))
#define BFSIZE(x) (ULong *)(((char *)(x))+2)
#define BFOFFBITS(x) (ULong *)(((char *)(x))+10)
#define BISIZE(x) (ULong *)(((char *)(x))+14)
#define BIWIDTH(x) (long *)(((char *)(x))+18)
#define BIHEIGHT(x) (long *)(((char *)(x))+22)
#define BITCOUNT(x) (UShort *)(((char *)(x))+28)
#define BICOMP(x) (ULong *)(((char *)(x))+30)
#define IMAGESIZE(x) (ULong *)(((char *)(x))+34)
#define BICLRUSED(x) (ULong *)(((char *)(x))+46)
#define BICLRIMP(x) (ULong *)(((char *)(x))+50)
#define BIPALETTE(x) (char *)(((char *)(x))+54)
#define PALENT(x,i) (char *)(((ULong *)(x))+i)

//---------------------------------------------------------------------------

const char* ImageBMPFile::get_status_mesg() const
{
  if (status & IFE_BadFormat) return "not a supported BMP format file";
  else return "image successfully read";
}

ImageBMPFile::ImageBMPFile (UByte* ptr, long filesize) : ImageFile ()
{
  status = IFE_BadFormat;

  if( !memcmp( ptr, BM, 2 ) && (*BISIZE(ptr)) == WinHSize && 
	  (*BICOMP(ptr)) == BI_RGB )
    {
         set_dimensions( *BIWIDTH(ptr), *BIHEIGHT(ptr) );

	 RGBPixel *bufPtr = get_buffer();
         UByte    *iPtr   = ptr + *BFOFFBITS(ptr);
	 int       pixelIdx = 0;

         const int bmp_width = (*BIWIDTH(ptr));
         const int bmp_height = (*BIHEIGHT(ptr));
         const int bmp_size = bmp_width*bmp_height;

          // The last scanline in BMP corresponds to the top line in the image
         int buffer_y = bmp_width*(bmp_height - 1), buffer_x = 0;

	 if( (*BITCOUNT(ptr)) == _256Color && (*BICLRUSED(ptr)) && (*BICLRIMP(ptr)) )
	 {
	   char *pal = BIPALETTE(ptr);

	   while( iPtr < ptr + filesize && pixelIdx < bmp_size )
	   {
		  char *palEnt = PALENT(pal,*iPtr++);

		  RGBPixel *buf = &bufPtr[buffer_y + buffer_x];
		  buf->blue    = *palEnt;
		  buf->green   = *(palEnt+1);
		  buf->red     = *(palEnt+2);
                  
                  if(++buffer_x > bmp_width)
                    {
                      buffer_x = 0;
                      buffer_y -= (bmp_width - 1);
                    }

		  pixelIdx++;
	   }
           status = IFE_OK;
	 }
	 else if( !(*BICLRUSED(ptr)) && !(*BICLRIMP(ptr)) &&
	     (*BITCOUNT(ptr)) == TRUECOLOR24 )
	 {
	   while( iPtr < ptr + filesize && pixelIdx < bmp_size )
	   {
	          RGBPixel *buf = &bufPtr[buffer_y + buffer_x];

		  buf->blue    = *iPtr;
		  buf->green   = *(iPtr+1);
		  buf->red     = *(iPtr+2);

                  if(++buffer_x > bmp_width)
                    {
                      buffer_x = 0;
                      buffer_y -= (bmp_width - 1);
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
