/*
  Copyright (C) 2002 by Jorrit Tyberghein

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
#include "qsqrt.h"
#include "csgeom/vector3.h"
#include "csgeom/pmtools.h"
#include "igeom/polymesh.h"

void csPolygonMeshTools::CalculateNormals (iPolygonMesh* mesh,
  csVector3* normals)
{
  int p;
  csVector3* verts = mesh->GetVertices ();
  int num_poly = mesh->GetPolygonCount ();
  csMeshedPolygon* poly = mesh->GetPolygons ();
  for (p = 0 ; p < num_poly ; p++)
  {
    float ayz = 0;
    float azx = 0;
    float axy = 0;
    int i, i1;
    float x1, y1, z1, x, y, z;

    int* vi = poly->vertices;
    i1 = poly->num_vertices - 1;
    x1 = verts[vi[i1]].x;
    y1 = verts[vi[i1]].y;
    z1 = verts[vi[i1]].z;
    for (i = 0 ; i < poly->num_vertices ; i++)
    {
      x = verts[vi[i]].x;
      y = verts[vi[i]].y;
      z = verts[vi[i]].z;
      ayz += (z1 + z) * (y - y1);
      azx += (x1 + x) * (z - z1);
      axy += (y1 + y) * (x - x1);
      x1 = x;
      y1 = y;
      z1 = z;
    }

    float sqd = ayz * ayz + azx * azx + axy * axy;
    float invd;
    if (sqd < SMALL_EPSILON)
      invd = 1.0f / SMALL_EPSILON;
    else
      invd = qisqrt (sqd);
    normals[p].Set (ayz * invd, azx * invd, axy * invd);

    poly++;
  }
}

