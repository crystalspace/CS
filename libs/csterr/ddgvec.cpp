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

#if 0
// Define an input and output stream operator for ddgVector2.
// Output format is "<xxx,yyy>"
ostream& operator << ( ostream&s, ddgVector2 v )
     { return s <<'<'<<v[0]<<','<<v[1]<<'>'; }
ostream& operator << ( ostream&s, ddgVector2* v )
     { return s <<'<'<<v->v[0]<<','<<v->v[1]<<'>'; }

// Input format is "<xxx,yyy>"
istream& operator >> ( istream& s, ddgVector2& v)
     { char c; s >> c >>v.v[0] >> c >> v.v[1] >> c ; return s; }

// Define an input and output stream operator for ddgVector3.
// Output format is "<xxx,yyy,zzz>"
ostream& operator << ( ostream&s, ddgVector3 v )
     { return s <<'<'<<v[0]<<','<<v[1]<<','<<v[2]<<'>'; }
ostream& operator << ( ostream&s, ddgVector3* v )
     { return s <<'<'<<v->v[0]<<','<<v->v[1]<<','<<v->v[2]<<'>'; }

// Input format is "<xxx,yyy,zzz>"
istream& operator >> ( istream& s, ddgVector3& v)
     { char c; s >> c >>v.v[0] >> c >> v.v[1] >> c >> v.v[2]>> c; return s; }
#endif

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

#if 0
// Define an input and output stream operator for ddgVector4.
// Output format is "<xxx,yyy,zzz,aaa>"
ostream& operator << ( ostream&s, ddgVector4 v )
     { return s <<'<'<<v[0]<<','<<v[1]<<','<<v[2]<<','<<v[3]<<'>'; }
ostream& operator << ( ostream&s, ddgVector4* v )
     { return s <<'<'<<v->v[0]<<','<<v->v[1]<<','<<v->v[2]<<','<<v->v[3]<<'>'; }

// Input format is "<xxx,yyy,zzz,aaa>"
istream& operator >> ( istream& s, ddgVector4& v)
     { char c; s >> c >>v.v[0] >> c >> v.v[1] >> c >> v.v[2]>> c >> v.v[3] >> c; return s; }
#endif


ddgVector4* ddgVector4::multiply( ddgMatrix4 *m1, ddgVector4 *v1 )
{
  for (int i=0; i<4; i++) {
    v[i] = 0;
    for (int j=0; j <4; j++) v[i] += m1->m[i].v[j] * v1->v[j];
    }
  return this;
}


