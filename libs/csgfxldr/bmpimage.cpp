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

#include "cssysdef.h"
#include "cstypes.h"
#include "csgeom/math3d.h"
#include "csgfxldr/bmpimage.h"
#include "cssys/csendian.h"

bool RegisterBMP ()
{
  static csBMPImageLoader loader;
  return csImageLoader::Register (&loader);
}

csImageFile* csBMPImageLoader::LoadImage (UByte* iBuffer, ULong iSize,
  int iFormat)
{
  ImageBMPFile* i = new ImageBMPFile (iFormat);
  if (i && !i->Load (iBuffer, iSize))
  {
    delete i;
    return NULL;
  }
  return i;
}

//-----------------------------------------------------------------------------
// Some platforms require strict-alignment, which means that values of
// primitive types must be accessed at memory locations which are multiples
// of the size of those types.  For instance, a 'long' can only be accessed
// at a memory location which is a multiple of four.  Consequently, the
// following endian-conversion functions first copy the raw data into a
// variable of the proper data type using memcpy() prior to attempting to
// access it as the given type.
//-----------------------------------------------------------------------------

static inline UShort us_endian (const UByte* ptr)
{
  short n;
  memcpy(&n, ptr, sizeof(n));
  return convert_endian(n);
}

static inline ULong ul_endian (const UByte* ptr)
{
  long n;
  memcpy(&n, ptr, sizeof(n));
  return convert_endian(n);
}

static inline long l_endian (const UByte* ptr)
{
  long n;
  memcpy(&n, ptr, sizeof(n));
  return convert_endian(n);
}

#define BFTYPE(x)    us_endian((x) +  0)
#define BFSIZE(x)    ul_endian((x) +  2)
#define BFOFFBITS(x) ul_endian((x) + 10)
#define BISIZE(x)    ul_endian((x) + 14)
#define BIWIDTH(x)   l_endian ((x) + 18)
#define BIHEIGHT(x)  l_endian ((x) + 22)
#define BITCOUNT(x)  ul_endian((x) + 28)  /* was us_endian */
#define BICOMP(x)    ul_endian((x) + 30)
#define IMAGESIZE(x) ul_endian((x) + 34)
#define BICLRUSED(x) ul_endian((x) + 46)
#define BICLRIMP(x)  ul_endian((x) + 50)
#define BIPALETTE(x) ((x) + 54)

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

//---------------------------------------------------------------------------

bool ImageBMPFile::Load (UByte* iBuffer, ULong iSize)
{
  if ((memcmp (iBuffer, BM, 2) == 0) && BISIZE(iBuffer) == WinHSize)
    return LoadWindowsBitmap (iBuffer, iSize);
  return false;
}

bool ImageBMPFile::LoadWindowsBitmap (UByte* iBuffer, ULong iSize)
{
  set_dimensions (BIWIDTH(iBuffer), BIHEIGHT(iBuffer));
  const int bmp_size = Width * Height;

  UByte *iPtr = iBuffer + BFOFFBITS(iBuffer);

  // No alpha for BMP.
  Format &= ~CS_IMGFMT_ALPHA;

  // The last scanline in BMP corresponds to the top line in the image
  int  buffer_y = Width * (Height - 1);
  bool blip     = false;

  if (BITCOUNT(iBuffer) == _256Color && BICLRUSED(iBuffer))
  {
    UByte    *buffer  = new UByte [bmp_size];
    csRGBpixel *palette = new csRGBpixel [256];
    csRGBpixel *pwork   = palette;
    UByte    *inpal   = BIPALETTE(iBuffer);
	int scanlinewidth = 4 * ((Width+3) / 4);
    for (int color = 0; color < 256; color++, pwork++)
    {  
      // Whacky BMP palette is in BGR order.
      pwork->blue  = *inpal++;
      pwork->green = *inpal++;
      pwork->red   = *inpal++;
      pwork->alpha = 0;
      inpal++; // Skip unused byte.
    }

    if (BICOMP(iBuffer) == BI_RGB)
    {
      // Read the pixels from "top" to "bottom"
      while (iPtr < iBuffer + iSize && buffer_y >= 0)
      {
        memcpy (buffer + buffer_y, iPtr, Width);
        iPtr += scanlinewidth;
        buffer_y -= Width;
      } /* endwhile */
    }
    else if (BICOMP(iBuffer) == BI_RLE8)
    {
      // Decompress pixel data
      UByte rl, rl1, i;			// runlength
      UByte clridx, clridx1;		// colorindex
      int buffer_x = 0;
      while (iPtr < iBuffer + iSize && buffer_y >= 0)
      {
        rl = rl1 = *iPtr++;
        clridx = clridx1 = *iPtr++;
        if (rl == 0)
           if (clridx == 0)
        {
    	  // new scanline
          if (!blip)
          {
            // if we didnt already jumped to the new line, do it now
            buffer_x  = 0;
            buffer_y -= Width;
          }
          continue;
        }
        else if (clridx == 1)
          // end of bitmap
          break;
        else if (clridx == 2)
        {
          // next 2 bytes mean column- and scanline- offset
          buffer_x += *iPtr++;
          buffer_y -= (Width * (*iPtr++));
          continue;
        }
        else if (clridx > 2)
          rl1 = clridx;

        for ( i = 0; i < rl1; i++ )
        {
          if (!rl) clridx1 = *iPtr++;
          buffer [buffer_y + buffer_x] = clridx1;

          if (++buffer_x >= Width)
          {
            buffer_x  = 0;
            buffer_y -= Width;
            blip = true;
          }
          else
            blip = false;
        }
        // pad in case rl == 0 and clridx in [3..255]
        if (rl == 0 && (clridx & 0x01)) iPtr++;
      }
    }
    // Now transform the image data to target format
    convert_pal8 (buffer, palette);
    return true;
  }
  else if (!BICLRUSED(iBuffer) && BITCOUNT(iBuffer) == TRUECOLOR24)
  {
    csRGBpixel *buffer = new csRGBpixel [bmp_size];

    while (iPtr < iBuffer + iSize && buffer_y >= 0)
    {
      csRGBpixel *d = buffer + buffer_y;
      for (int x = Width; x; x--)
      {
        d->blue    = *iPtr++;
        d->green   = *iPtr++;
        d->red     = *iPtr++;
        d++;
      } /* endfor */

      buffer_y -= Width;
    }
    // Now transform the image data to target format
    convert_rgba (buffer);
    return true;
  }

  return false;
}
