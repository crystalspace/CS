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

#ifndef __TESSELATE_H__
#define __TESSELATE_H__

/*
  Tesselate: A general purpose tesselator. To use, simply fill a grid
  cell struct with the required corner positions and values. Allocate
  at least 15 vertices in the vertices buffer and hand it to tesselate.
  It will return the filled buffer and the number of vertices it contains.
  The buffer will contain upto 15 verts in multiples of three, each
  representing a triangle. The normals point inward, so to obtain a
  concave hull from the list, assign the vertices in order to a triangle. 
  To obtain a convex hull, swap two of the vertices ( 1 and 3 does
  nicely ). The algorithm is just about bullet proof, however there are
  a few simple rules to using the algorithm:
  
	1) If you feed it nan's in the grid cell value, it will produce nans
		in the output vertex list. It wont barf, but your app might.
	2) If you dont allocate at least fifteen vertices for the vertex list,
		then you WILL get a segfault (eventually).
	3) The grid cell does NOT have to be a perfect cube. It could be a
		frustum for example ( hint for Jorrit: That includes a view
		frustum ).
	4) The grid cell DOES have a specific vertex order. The following is
	a list of the vertex order ( cube extends from (0,0,0) to (1,1,1) )
	
	p[0]: (0,1,1)
	p[1]: (1,1,1)
	p[2]: (1,1,0)
	p[3]: (0,1,0)
	p[4]: (0,0,1)
	p[5]: (1,0,1)
	p[6]: (1,0,0)
	p[7]: (0,0,0)

  Examples of fast methods to fill the grid cell can be found in the
  metaball mesh plugin and the metagen mesh plugin. 
  ( plugins/mesh/metaball/object/process.cpp and
	plugins/mesh/metagen/object/mgproc.cpp respectively )

*/

class csVector3;

struct GridCell
{
   csVector3 p[8];
   float val[8];
   GridCell() {} // NextStep 3.3 compiler barfs without this.
};

int Tesselate( const GridCell &gridcell, csVector3 *vertices );

#endif // __TESSELATE_H__






