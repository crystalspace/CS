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

#include "cssysdef.h"
#include "csgfx/imagetools.h"
#include "csplugincommon/canvas/scrshot.h"

SCF_IMPLEMENT_IBASE (csScreenShot)
  SCF_IMPLEMENTS_INTERFACE (iImage)
SCF_IMPLEMENT_IBASE_END

csScreenShot::csScreenShot (iGraphics2D *G2D)
{
  SCF_CONSTRUCT_IBASE (0);

  Width = G2D->GetWidth ();
  Height = G2D->GetHeight ();
  csPixelFormat const* pfmt = G2D->GetPixelFormat ();
  int x;

  if (pfmt->PalEntries)
  {
    Format = CS_IMGFMT_PALETTED8;
    Palette = G2D->GetPalette ();
    Data = new uint8 [Width * Height];
    uint8 *dst = (uint8 *)Data;
    for (x = 0; x < Height; x++)
    {
      uint8 *src = (uint8 *)G2D->GetPixelAt (0, x);
      if (!src) continue;
      memcpy (dst, src, Width * sizeof (uint8));
      dst += Width;
    }
  }
  else
  {
    Format = CS_IMGFMT_TRUECOLOR;
    Data = new csRGBpixel [Width * Height];
    csRGBpixel *dst = (csRGBpixel *)Data;
    Palette = 0;
    int rs = 8 - pfmt->RedBits;
    int gs = 8 - pfmt->GreenBits;
    int bs = 8 - pfmt->BlueBits;
	int y;
    for (x = 0; x < Height; x++)
      switch (pfmt->PixelBytes)
      {
        case 2:
        {
          uint16 *src = (uint16 *)G2D->GetPixelAt (0, x);
          if (!src) continue;
          for (y = Width; y; y--)
          {
            uint16 pix = *src++;
            dst->red   = ((pix & pfmt->RedMask)   >> pfmt->RedShift)   << rs;
            dst->green = ((pix & pfmt->GreenMask) >> pfmt->GreenShift) << gs;
            dst->blue  = ((pix & pfmt->BlueMask)  >> pfmt->BlueShift)  << bs;
            dst++;
          }
          break;
        }
        case 4:
        {
          uint32 *src = (uint32 *)G2D->GetPixelAt (0, x);
          if (!src) continue;
          for (y = Width; y; y--)
          {
            uint32 pix = *src++;
            dst->red   = ((pix & pfmt->RedMask)   >> pfmt->RedShift)   << rs;
            dst->green = ((pix & pfmt->GreenMask) >> pfmt->GreenShift) << gs;
            dst->blue  = ((pix & pfmt->BlueMask)  >> pfmt->BlueShift)  << bs;
            dst++;
          }
          break;
        }
      }
  }
}

csScreenShot::~csScreenShot ()
{
  if ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8)
    delete [] (uint8 *)Data;
  else
    delete [] (csRGBpixel *)Data;

  SCF_DESTRUCT_IBASE();
}

int csScreenShot::GetClosestIndex (const csRGBpixel& color)
{ 
  return csImageTools::ClosestPaletteIndex (Palette, color);
}
