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

#ifndef __CS_SCRSHOT_H__
#define __CS_SCRSHOT_H__

#include "csextern.h"
#include "ivideo/graph2d.h"
#include "csgfx/imagebase.h"

class CS_CSPLUGINCOMMON_EXPORT csScreenShot : public csImageBase
{
  int Format;
  void *Data;
  csRGBpixel *Palette;
  int Width, Height;

public:
  SCF_DECLARE_IBASE;
  /// Initialize the screenshot object
  csScreenShot (iGraphics2D *G2D);
  /// Destroy the screenshot object
  virtual ~csScreenShot ();
  /// Get a pointer to image data
  virtual const void *GetImageData ()
  { return Data; }
  /// Query image width
  virtual int GetWidth () const
  { return Width; }
  /// Query image height
  virtual int GetHeight () const
  { return Height; }
  /// Qyery image format (see CS_IMGFMT_XXX above)
  virtual int GetFormat () const
  { return Format; }
  /// Get image palette (or 0 if no palette)
  virtual const csRGBpixel *GetPalette ()
  { return Palette; }
  virtual int GetClosestIndex (const csRGBpixel& color);
  /// Get alpha map for 8-bit paletted image.
};

#endif // __CS_SCRSHOT_H__

