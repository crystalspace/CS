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

#ifndef __SCRNSHOT_H__
#define __SCRNSHOT_H__

#include <stdarg.h>

#include "sysdef.h"
#include "glide2common2d.h"
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
  csScreenShot (iGraphics2D *G2D, UShort*& imgdata);
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
  { (void) NewWidth; (void) NewHeight; }
  /// Create a new iImage which is a mipmapped version of this one.
  virtual iImage *MipMap (int step, RGBPixel *transp)
  { (void)step; (void)transp; return NULL; }
  /// Set image file name
  virtual void SetName (const char *iName)
  { (void) iName; }
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

#endif // __SCRNSHOT_H__
