/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#include <stdarg.h>

#include "sysdef.h"
#include "scrnshot.h"

IMPLEMENT_IBASE (csScreenShot)
  IMPLEMENTS_INTERFACE (iImage)
IMPLEMENT_IBASE_END

csScreenShot::csScreenShot (iGraphics2D *G2D, UShort*& imgdata)
{
  CONSTRUCT_IBASE (NULL);

  Width = G2D->GetWidth ();
  Height = G2D->GetHeight ();
  csPixelFormat *pfmt = G2D->GetPixelFormat ();

  {
    int nPixel = Width * Height;
    UShort *src;
    Format = CS_IMGFMT_TRUECOLOR;
    
    Data = new RGBPixel [nPixel];
    RGBPixel *dst = (RGBPixel *)Data;
    Palette = NULL;
    int rs = 8 - pfmt->RedBits;
    int gs = 8 - pfmt->GreenBits;
    int bs = 8 - pfmt->BlueBits;
    
    src = imgdata;
    for (int y = 0; y < nPixel; y+=2){
      UShort pix1 = *src++;
      UShort pix2 = *src++;
      dst->red   = ((pix2 & pfmt->RedMask)   >> pfmt->RedShift)   << rs;
      dst->green = ((pix2 & pfmt->GreenMask) >> pfmt->GreenShift) << gs;
      dst->blue  = ((pix2 & pfmt->BlueMask)  >> pfmt->BlueShift)  << bs;
      dst++;
      dst->red   = ((pix1 & pfmt->RedMask)   >> pfmt->RedShift)   << rs;
      dst->green = ((pix1 & pfmt->GreenMask) >> pfmt->GreenShift) << gs;
      dst->blue  = ((pix1 & pfmt->BlueMask)  >> pfmt->BlueShift)  << bs;
      dst++;
    }
  }
}

