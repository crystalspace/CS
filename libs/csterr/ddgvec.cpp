/*
    Copyright (C) 1997, 1998, 1999 by Alex Pfaffe
	(Digital Dawn Graphics Inc)
  
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
#include <math.h>
//#include <iostream.h>
#include "sysdef.h"
#include "csterr/ddgvec.h"
#include "csterr/ddgmat.h"
// ----------------------------------------------------------------------

// Assumes that incoming vectors are normalized.
void ddgVector3::normal( ddgVector3* v[9])
{
	register int k;
	ddgVector3 n, d[8];
	// These eight vectors are located as follows:
	//     0 1 2
	//      \|/
	//     7-8-3
	//      /|\ 
	//     6 5 4

	// Now calculate the normals for every point on the grid
	for (k = 0; k < 8; k++)
    {
		if (v[k])
		{
			d[k].set(*v[8]);
			d[k].subtract(*v[k]);
		}
	}
	// Clear any current value.
	zero();
	// Calculate the normals for each triangle
	for (k =0; k < 8; k++)
    {
		if (v[k] && v[(k+1)%8])
			{
			// Calculate the normal, cross product of 2 vectors.
			n.cross(d[(k+1)%8],d[k]);
			// Calculate the size of the vector.
			add(n);
		}
    }
	normalize();
}

/*

ddgVector4* ddgVector4::multiply( ddgMatrix4 *m1, ddgVector4 *v1 )
{
  for (int i=0; i<4; i++) {
    v[i] = 0;
    for (int j=0; j <4; j++) v[i] += m1->m[i].v[j] * v1->v[j];
    }
  return this;
}

*/
