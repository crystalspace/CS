/*
    Copyright (C) 2001 by Christopher Nelson
    
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

#ifndef __CS_CSRECTREGION_H__
#define __CS_CSRECTREGION_H__

#include "csgeom/csrect.h"

/**
 * A rect region is a class that implements splittable 2d rectangles.  The
 * region may be composed of one or more rectangles that do not need to occupy
 * the same area.  The idea being that you need to be able to have a number of
 * disparate rectangular regions for updates.  This class uses csRect for
 * nearly all rectangle operations.
 *<p>
 * To use you can Include and Exclude rectangles from this region.  When
 * finished, this class will have a list of optimal rectangles that occupy a
 * region.  If used properly, it will result in faster overall painting
 * performance, since several areas will not require overwriting.
 */
class csRectRegion
{
protected:
  csRect* region;
  int region_count;
  int region_max;

  void pushRect(csRect const &);
  void deleteRect(int);
  bool chopEdgeIntersection(csRect&, csRect&);
  void fragmentRect(csRect&, csRect&, bool testedContains, bool testedEdge);
  void fragmentContainedRect(csRect &r1, csRect &r2);

public:
  /// Constructor.
  csRectRegion();
  /// Destructor
  ~csRectRegion();

  /// Add a rect to this region; may cause unions, but will not adjance (see csRect).
  void Include(csRect &rect);
  /// Exclude a rect from this region; may cause splitting.
  void Exclude(csRect &rect);

  /// Returns the number of rectangles in this region
  int Count() { return region_count; }
  /// Returns the rect at a specific index
  csRect& RectAt(int i) {  return region[i]; }
};

#endif //__CS_CSRECTREGION_H__
