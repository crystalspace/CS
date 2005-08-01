/*
    Copyright (C) 2003 by Jorrit Tyberghein

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

#include "cssysdef.h"
#include <math.h>
#include "csgeom/polymesh.h"
#include "csgeom/pmtools.h"


void csPolygonMesh::Triangulate ()
{
  if (triangles) return;
  csPolygonMeshTools::Triangulate (this, triangles, triangle_count);
}

csTriangle* csPolygonMeshBox::GetTriangles ()
{
  if (triangles) return triangles;
  int tc;
  csPolygonMeshTools::Triangulate (this, triangles, tc);
  return triangles;
}

