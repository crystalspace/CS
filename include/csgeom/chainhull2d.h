/*
    Copyright (C) 2005 by Jorrit Tyberghein

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

/*
    The code has been adapted from other code with the following
    copyright message:
    Copyright 2001, softSurfer (www.softsurfer.com)
    This code may be freely used and modified for any purpose
    providing that this copyright notice is included with it.
    SoftSurfer makes no warranty for this code, and cannot be held
    liable for any real or imagined damage resulting from its use.
    Users of this code must verify correctness for their application.
*/

#ifndef __CS_CHAINHULL2D_H__
#define __CS_CHAINHULL2D_H__

#include "csextern.h"
#include "csgeom/math2d.h"
#include "csgeom/vector2.h"

/**
 * This is Andrew's monotone chain 2D convex hull algorithm.
 */
class CS_CRYSTALSPACE_EXPORT csChainHull2D
{
public:
  /**
   * Sort the given array by increasing x and y coordinates so that it
   * can be used by CalculatePresorted().
   * \param points is the set of points
   * \param n is the number of points
   */
  static void SortXY (csVector2* points, size_t n);

  /**
   * This routine will calculate the convex hull of the presorted
   * input points (presorted by increasing x and y coordinates).
   * \param points is the presorted set of points.
   * \param n is the number of points
   * \param hull is the convex hull output. This array must be at least
   * as big as the 'points' array (i.e. n points)
   * \return the number of points in the convex hull
   */
  static size_t CalculatePresorted (csVector2* points, size_t n,
				    csVector2* hull);
};

#endif // __CS_CHAINHULL2D_H__

