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
 *
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
 *
 * The method behind the tesselator is a very simple structure of a point
 * interpolation algorithm and two tables containing the edge results of the
 * cube filter and a triangle interpretation table to complete the mapping.
 * Upon entry ( after initializing values ), the algorithm first evaluates each
 * corner of the grid cell to test if it is greater or less than zero.  For
 * each corner that is less than zero, an index flag is set on the edge index
 * (cubeindex).  It is probably not without coincidence that the number of
 * corner flags in a cube neatly fits into a single byte.
 *
 * The edge table represents a vertex outcome based on the the corner map it
 * was supplied.  It is a twelve bit mapping respresenting each edge of the
 * cube.  The order is specifc, and these are the edge vertices.  (This table
 * is based on the one earlier on.)
 *
 *<pre>
 *  bit 0: p[0] - p[1]
 *  bit 1: p[1] - p[2]
 *  bit 2: p[2] - p[3]
 *  bit 3: p[3] - p[0]
 *  bit 4: p[4] - p[5]
 *  bit 5: p[5] - p[6]
 *  bit 6: p[6] - p[7]
 *  bit 7: p[7] - p[4]
 *  bit 8: p[0] - p[4]
 *  bit 9: p[1] - p[5]
 *  bit 10: p[2] - p[6]
 *  bit 11: p[3] - p[7]
 *</pre>
 *
 * For each state where one vertex is greater than zero and its nieghbour is is
 * less than zero, a corresopnding flag is set by the edge table.  After all
 * edges that incur an intersection are set, the next step is interpolating the
 * intersection vertices.  The interpolation algorithm uses the values of each
 * corner (GridCell.val[]) to produce a linear estimate of where the
 * intersection with the edge occurs.  This is not 100% accurate, but does
 * provide a nice approximation.
 *
 * The triangle table is a scripted set of triangle maps based on the cube
 * index.  It contains 256 mappings with up to five triangles.  The sixteenth
 * element of every mapping is -1, to terminate the mapping.  In many cases,
 * the map table is filled with terminations after two or three triangles.  Now
 * although there are only twelve resultant vertices from the interpolation
 * unit, in some cases, a given edge vertex may be used up to four times in a
 * given mapping, resulting in a maximum of fifteen vertices in the output.
 *
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
