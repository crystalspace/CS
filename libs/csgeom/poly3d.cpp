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
#include "csgeom/poly2d.h"

csPoly3D::csPoly3D (int start_size)
{
  max_vertices = start_size;
  vertices = new csVector3 [max_vertices];
  MakeEmpty ();
}

csPoly3D::csPoly3D (csPoly3D& copy)
{
  max_vertices = copy.max_vertices;
  vertices = new csVector3 [max_vertices];
  num_vertices = copy.num_vertices;
  memcpy (vertices, copy.vertices, sizeof (csVector3)*num_vertices);
}

csPoly3D::~csPoly3D ()
{
  delete [] vertices;
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

bool csPoly3D::In (csVector3* poly, int num_poly, const csVector3& v)
{
  int i, i1;
  i1 = num_poly-1;
  for (i = 0 ; i < num_poly ; i++)
  {
    if (csMath3::WhichSide3D (v, poly[i1], poly[i]) < 0) return false;
    i1 = i;
  }
  return true;
}

void csPoly3D::MakeRoom (int new_max)
{
  if (new_max <= max_vertices) return;
  csVector3* new_vertices = new csVector3 [new_max];
  memcpy (new_vertices, vertices, num_vertices*sizeof (csVector3));
  delete [] vertices;
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

void csPoly3D::ProjectXPlane (const csVector3& point, float plane_x,
	csPoly2D* poly2d)
{
  poly2d->SetNumVertices (0);
  poly2d->GetBoundingBox ().StartBoundingBox ();
  csVector2 p;
  csVector3 v;
  float x_dist = plane_x - point.x;
  int i;
  for (i = 0 ; i < num_vertices ; i++)
  {
    v = vertices[i]-point;
    p.x = point.y + x_dist * v.y / v.x;
    p.y = point.z + x_dist * v.z / v.x;
    poly2d->AddVertex (p);
  }
}

void csPoly3D::ProjectYPlane (const csVector3& point, float plane_y,
	csPoly2D* poly2d)
{
  poly2d->SetNumVertices (0);
  poly2d->GetBoundingBox ().StartBoundingBox ();
  csVector2 p;
  csVector3 v;
  float y_dist = plane_y - point.y;
  int i;
  for (i = 0 ; i < num_vertices ; i++)
  {
    v = vertices[i]-point;
    p.x = point.x + y_dist * v.x / v.y;
    p.y = point.z + y_dist * v.z / v.y;
    poly2d->AddVertex (p);
  }
}

void csPoly3D::ProjectZPlane (const csVector3& point, float plane_z,
	csPoly2D* poly2d)
{
  poly2d->SetNumVertices (0);
  poly2d->GetBoundingBox ().StartBoundingBox ();
  csVector2 p;
  csVector3 v;
  float z_dist = plane_z - point.z;
  int i;
  for (i = 0 ; i < num_vertices ; i++)
  {
    v = vertices[i]-point;
    p.x = point.x + z_dist * v.x / v.z;
    p.y = point.y + z_dist * v.y / v.z;
    poly2d->AddVertex (p);
  }
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

