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
#include "cs3d/software/soft_g3d.h"
#include "igraph2d.h"
#include "iimage.h"

class csScreenShot : public iImage
{
  int Format;
  void *Data;
  RGBPixel *Palette;
  int Width, Height;
public:
  DECLARE_IBASE;
  /// Initialize the screenshot object
  csScreenShot (iGraphics2D *G2D);
  /// Destroy the screenshot object
  virtual ~csScreenShot ()
  {
    if ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8)
      delete [] (RGBPixel *)Data;
    else
      delete [] (UByte *)Data;
  }
  ///
  virtual void *GetImageData ()
  { return Data; }
  /// Query image width
  virtual int GetWidth ()
  { return Width; }
  /// Query image height
  virtual int GetHeight ()
  { return Height; }
  /// Query image size in bytes
  virtual int GetSize () { return 0; }
  /// Resize the image to the given size
  virtual void Resize (int NewWidth, int NewHeight)
  { }
  /// Create a new iImage which is a mipmapped version of this one.
  virtual iImage *MipMap (int step, RGBPixel *transp)
  { return NULL; }
  /// Set image file name
  virtual void SetName (const char *iName)
  { }
  /// Get image file name
  virtual const char *GetName ()
  { return "dummy"; }
  /// Qyery image format (see CS_IMGFMT_XXX above)
  virtual int GetFormat ()
  { return Format; }
  /// Get image palette (or NULL if no palette)
  virtual RGBPixel *GetPalette ()
  { return Palette; }
  /// Get alpha map for 8-bit paletted image.
  virtual UByte *GetAlpha ()
  { return NULL; }
};

IMPLEMENT_IBASE (csScreenShot)
  IMPLEMENTS_INTERFACE (iImage)
IMPLEMENT_IBASE_END

csScreenShot::csScreenShot (iGraphics2D *G2D)
{
  CONSTRUCT_IBASE (NULL);

  Width = G2D->GetWidth ();
  Height = G2D->GetHeight ();
  csPixelFormat *pfmt = G2D->GetPixelFormat ();

  if (pfmt->PalEntries)
  {
    Format = CS_IMGFMT_PALETTED8;
    Data = new UByte [Width * Height];
    Palette = G2D->GetPalette ();
    memcpy (Data, G2D->GetPixelAt (0, 0), Width * Height * sizeof (UByte));
  }
  else
  {
    Format = CS_IMGFMT_TRUECOLOR;
    Data = new RGBPixel [Width * Height];
    RGBPixel *dst = (RGBPixel *)Data;
    Palette = NULL;
    int rs = 8 - pfmt->RedBits;
    int gs = 8 - pfmt->GreenBits;
    int bs = 8 - pfmt->BlueBits;
    for (int y = 0; y < Height; y++)
      switch (pfmt->PixelBytes)
      {
        case 2:
        {
          UShort *src = (UShort *)G2D->GetPixelAt (0, y);
          for (int x = Width; x; x--)
          {
            UShort pix = *src++;
            dst->red   = ((pix & pfmt->RedMask)   >> pfmt->RedShift)   << rs;
            dst->green = ((pix & pfmt->GreenMask) >> pfmt->GreenShift) << gs;
            dst->blue  = ((pix & pfmt->BlueMask)  >> pfmt->BlueShift)  << bs;
            dst++;
          }
          break;
        }
        case 4:
        {
          ULong *src = (ULong *)G2D->GetPixelAt (0, y);
          for (int x = Width; x; x--)
          {
            ULong pix = *src++;
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

iImage *csGraphics3DSoftware::ScreenShot ()
{
  bool locked = !!(DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS));
  if (!locked) BeginDraw (CSDRAW_2DGRAPHICS);
  csScreenShot *ss = new csScreenShot (G2D);
  if (!locked) FinishDraw ();
  return ss;
}
