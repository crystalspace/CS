/*
	  Crystal Space Tesselator
	  Copyright (C) 1999 by Denis Dmitriev
  
	  This library is free software; you can redistribute it and/or
	  modify it under the terms of the GNU Library General Public
	  License as published by the Free Software Foundation; either
	  version 2 of the License, or (at your option) any later version.
  
	  This library is distributed in the hope that it will be useful,
	  but WITHOUT ANY WARRANTY; without even the implied warranty of
	  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
	  Library General Public License for more details.
  
	  You should have received a copy of the GNU Library General Public
	  License along with this library; if not, write to the Free
	  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CSGEOM_TESSELAT_H__
#define __CSGEOM_TESSELAT_H__

#include "vector3.h"

/**
 * A general purpose tesselation mechanism.
 * To use, simply fill a grid cell struct with the required corner positions
 * and values.  Allocate at least 15 vertices in the vertices buffer and hand
 * it to tesselate.  It will return the filled buffer and the number of
 * vertices it contains.  The buffer will contain up to 15 vertices in
 * multiples of three, each representing a triangle.  The normals point inward,
 * so to obtain a concave hull from the list, assign the vertices in order to a
 * triangle.  To obtain a convex hull, swap two of the vertices (1 and 3 does
 * nicely).  The algorithm is just about bullet proof, however there are a few
 * simple rules to using the algorithm:
 * <ol>  
 * <li>If you feed it NaNs in the grid cell value, it will produce NaNs in the
 *     output vertex list.  It won't barf, but your app might.
 * <li>If you don't allocate at least fifteen vertices for the vertex list,
 *     then you <em>will</em> get a segmentation fault (eventually).
 * <li>The grid cell does <em>not</em> have to be a perfect cube.  It could be
 *     a frustum for example.
 * <li>The grid cell <em>does</em> have a specific vertex order.  The following
 *     is a list of the vertex order (cube extends from <0,0,0> to <1,1,1>).
 * <pre>	
 *     p[0]: (0,1,1)
 *     p[1]: (1,1,1)
 *     p[2]: (1,1,0)
 *     p[3]: (0,1,0)
 *     p[4]: (0,0,1)
 *     p[5]: (1,0,1)
 *     p[6]: (1,0,0)
 *     p[7]: (0,0,0)
 * </pre>
 * </ol>
 * Examples of fast methods to fill the grid cell can be found in the MetaBall
 * mesh plugin and the <code>metagen</code> mesh plugin
 * (<code>CS/plugins/mesh/metaball/object/process.cpp</code> and
 * <code>CS/plugins/mesh/metagen/object/mgproc.cpp</code>, respectively).
 */
class csTesselator
{
public:
  /**
   * A tesselation grid cell.
   */
  struct GridCell
  {
     csVector3 p[8];
     float val[8];
     GridCell() {} // NextStep 3.3 compiler barfs without this when a client
                   // attempts to construct a GridCell on the stack.
  };

  // Perform the tesselation.  See class documentation for a full description.
  static int Tesselate(const GridCell&, csVector3* vertices);
};

#endif // __CSGEOM_TESSELAT_H__
