/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#include "sysdef.h"
#include "csgeom/poly3d.h"

csPoly3D::csPoly3D (int start_size)
{
  max_vertices = start_size;
  CHK (vertices = new csVector3 [max_vertices]);
  MakeEmpty ();
}

csPoly3D::csPoly3D (csPoly3D& copy)
{
  max_vertices = copy.max_vertices;
  CHK (vertices = new csVector3 [max_vertices]);
  num_vertices = copy.num_vertices;
  memcpy (vertices, copy.vertices, sizeof (csVector3)*num_vertices);
}

csPoly3D::~csPoly3D ()
{
  CHK (delete [] vertices);
}

void csPoly3D::MakeEmpty ()
{
  num_vertices = 0;
}

bool csPoly3D::In (const csVector3& v)
{
  int i, i1;
  i1 = num_vertices-1;
  for (i = 0 ; i < num_vertices ; i++)
  {
    if (csMath3::WhichSide3D (v, vertices[i1], vertices[i]) < 0) return false;
    i1 = i;
  }
  return true;
}

void csPoly3D::MakeRoom (int new_max)
{
  if (new_max <= max_vertices) return;
  CHK (csVector3* new_vertices = new csVector3 [new_max]);
  memcpy (new_vertices, vertices, num_vertices*sizeof (csVector3));
  CHK (delete [] vertices);
  vertices = new_vertices;
  max_vertices = new_max;
}

int csPoly3D::AddVertex (float x, float y, float z)
{
  if (num_vertices >= max_vertices)
    MakeRoom (max_vertices+5);
  vertices[num_vertices].x = x;
  vertices[num_vertices].y = y;
  vertices[num_vertices].z = z;
  num_vertices++;
  return num_vertices-1;
}

//---------------------------------------------------------------------------

int csVector3Array::AddVertexSmart (float x, float y, float z)
{
  int i;
  for (i = 0 ; i < num_vertices ; i++)
    if (ABS (x-vertices[i].x) < SMALL_EPSILON &&
    	ABS (y-vertices[i].y) < SMALL_EPSILON &&
	ABS (z-vertices[i].z) < SMALL_EPSILON)
      return i;
  AddVertex (x, y, z);
  return num_vertices-1;
}

//---------------------------------------------------------------------------

