/*
    Copyright (C) 2001-2005 by Jorrit Tyberghein
		  2003-2005 by Frank Richter

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

#ifndef __CS_SUBREC_H__
#define __CS_SUBREC_H__

/**\file 
 * Stuff small rectangles into a bigger one
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"

#include "csgeom/csrect.h"

class csSubRect;

/**
 * A class managing allocations of sub-rectangles. i.e. this class represents
 * a rectangular region from which a client can allocate smaller rectangles
 * until the region is full.
 */
class CS_CRYSTALSPACE_EXPORT csSubRectangles
{
  friend class csSubRect;
protected:
  /// Dimensions of this region.
  csRect region;
  /// First empty region.
  csSubRect* root;

  csSubRect* AllocSubrect ();
  
  void Grow (csSubRect* sr, int ow, int oh, int nw, int nh);
public:
  /// Allocate a new empty region with the given size.
  csSubRectangles (const csRect& region);

  /// Remove this region and sub-regions.
  ~csSubRectangles ();

  /// Get the rectangle for this region.
  const csRect& GetRectangle () const { return region; }

  /**
   * Free all rectangles in this region.
   */
  void Clear ();

  /**
   * Allocate a new rectangle. Returns 0 if there is no room
   */
  csSubRect* Alloc (int w, int h, csRect& rect);

  /**
   * Reclaim a subrectangle, meaning, the space occupied by the subrect can be
   * reused by subsequent Alloc() calls.
   */
  void Reclaim (csSubRect* subrect);

  /**
   * Increase the size of the region.
   * You can only grow upwards.
   */
  bool Grow (int newWidth, int newHeight);

  /**
   * For debugging: dump all free rectangles.
   */
  void Dump ();
};

/** @} */

#endif // __CS_SUBREC_H__

