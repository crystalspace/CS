/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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

/**\file
 * Some helper functions to deal with iImage objects.
 */
 
/**\addtogroup gfx
 * @{ 
 */

#ifndef __CS_CSGFX_IMAGETOOLS_H__
#define __CS_CSGFX_IMAGETOOLS_H__

#include "csextern.h"
#include "igraphic/image.h"

/**
 * Some helper functions to deal with iImage objects.
 */
class CS_CRYSTALSPACE_EXPORT csImageTools
{
public:
  /// Compute the size of an image data, in bytes.
  static inline size_t ComputeDataSize (iImage* img) 
  {
    return img->GetWidth() * img->GetHeight() * img->GetDepth() * 
      (((img->GetFormat() & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8) ? 1 : 
      sizeof (csRGBpixel));
  }
  /**
   * Return the closest palette index to given color. 
   */
  static int ClosestPaletteIndex (const csRGBpixel* Palette, 
    const csRGBpixel& iColor, int palEntries = 256);
};

/** @} */

#endif // __CS_CSGFX_IMAGETOOLS_H__
