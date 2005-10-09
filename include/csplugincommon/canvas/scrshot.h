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

#ifndef __CS_CSPLUGINCOMMON_CANVAS_SCRSHOT_H__
#define __CS_CSPLUGINCOMMON_CANVAS_SCRSHOT_H__

/**\file
 * Simple iImage implementation for canvas screenshots.
 */

#include "csextern.h"

#include "csgfx/imagebase.h"
#include "csutil/scf_implementation.h"

struct iGraphics2D;

/**
 * \addtogroup plugincommon
 * @{ */

/// Simple iImage implementation for canvas screenshots.
class CS_CRYSTALSPACE_EXPORT csScreenShot : 
  public scfImplementationExt0<csScreenShot, csImageBase>
{
  int Format;
  void *Data;
  csRGBpixel *Palette;
  int Width, Height;

public:
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

/** @} */

#endif // __CS_CSPLUGINCOMMON_CANVAS_SCRSHOT_H__

