/*
    Copyright (C) 2000 by W.C.A. Wijngaards

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

#ifndef __CS_BUMPMAP_H__
#define __CS_BUMPMAP_H__

#include "cstypes.h"

struct iImage;

/**
 * Bumpmap formats. The different hardware and software renderers may
 * require the bumpmap to be in a different format. They should all be
 * convertible from the height-map data.
 */

/// The height of each bump is stored in 8 bits. 255 = high, 0 = low.
#define CS_BUMPFMT_HEIGHT_8     0x01

/** (For software renderer) The slope in x and y are stored in
 *  the first 4 and last 4 bits of each byte.
 */
#define CS_BUMPFMT_SLOPE_44     0x02


/**
 * A class representing a bumpmap. It can be constructed from regular
 * images.
 */
class csBumpMap
{
protected:
  /// Width of bumpmap.
  int width;
  /// Height of bumpmap.
  int height;
  /// The bumpmap data.
  uint8 *bumpmap;
  /// BumpMap format (see CS_BUMPFMT_XXX above)
  int format;

public:

  /**
   * create a new bumpmap in the specified format from the image
   * provided.
   */
  csBumpMap(iImage* src, int fmt);

  /// deletes a bumpmap
  ~csBumpMap();

  /// get width
  int GetWidth() const {return width;}
  /// get height
  int GetHeight() const {return height;}
  /// get bumpmap data
  void *GetBumpData() const {return bumpmap;}
};

#endif // __CS_BUMPMAP_H__
