#ifndef __CSRECTREGION__
#define __CSRECTREGION__

/**************************************************************************
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
*****************************************************************************/

#include "csgeom/csrect.h"
#include "csutil/csvector.h"

/****************************************************************************

  A rect region is a class that implements splittable 2d rectangles.  The region
 may be composed of one or more rectangles that do not need to occupy the same
 area.  The idea being that you need to be able to have a number of disparate
 rectangular regions for updates.  This class uses csRect for nearly all rectangle
 operations.

  To use you can Include and Exclude rectangles from this region.  When finished,
 this class will have a list of optimal rectangles that occupy a region.  If used
 properly, it will result in faster overall painting performance, since several
 areas will not require overwriting.
 ****************************************************************************/


class csRectRegion
{
  /// The list of rects that compose this region
  csBasicVector region;

private:
  bool chopEdgeIntersection(csRect &r1, csRect &r2);
  void fragmentRect(csRect &r1, csRect &r2, bool testedContains, bool testedEdge);

public:
  /// Includes a rect into this region, may cause unions, but will not adjance.
  void Include(csRect &rect);

  /// Excludes a rect from this region.  May cause splitting.
  void Exclude(csRect &rect);
};


#endif