/*
    Copyright (C) 2001 by Jorrit Tyberghein

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
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"

#include "csgeom/csrect.h"

/**
 * Sub-rectangle
 */
class csSubRect : public csRect
{
public:
  csSubRect* next, * prev;

  /// Create a new rectangle.
  csSubRect (int ixmin, int iymin, int ixmax, int iymax)
  	: csRect (ixmin, iymin, ixmax, iymax), next (0), prev (0) { }

  /// Copy constructor.
  csSubRect (const csRect &copy) : csRect (copy), next (0), prev (0) { }
};

/**
 * A class managing allocations of sub-rectangles. i.e. this class represents
 * a rectangular region from which a client can allocate smaller rectangles
 * until the region is full.
 */
class CS_CRYSTALSPACE_EXPORT csSubRectangles
{
protected:
  /// Dimensions of this region.
  csRect region;
  /// First empty region.
  csSubRect* first;

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
   * Allocate a new rectangle. Return false if there is no room
   */
  bool Alloc (int w, int h, csRect& rect);

  /**
   * For debugging: dump all free rectangles.
   */
  void Dump ();
};

/** @} */

#endif // __CS_SUBREC_H__

