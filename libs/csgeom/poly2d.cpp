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
#include "csgeom/poly2d.h"
#include "csgeom/polyclip.h"

csPoly2DFactory* csPoly2DFactory::SharedFactory()
{
  static csPoly2DFactory p;
  return &p;
}

csPoly2D::csPoly2D (int start_size)
{
  max_vertices = start_size;
  CHK (vertices = new csVector2 [max_vertices]);
  MakeEmpty ();
}

csPoly2D::csPoly2D (csPoly2D& copy)
{
  max_vertices = copy.max_vertices;
  CHK (vertices = new csVector2 [max_vertices]);
  num_vertices = copy.num_vertices;
  memcpy (vertices, copy.vertices, sizeof (csVector2)*num_vertices);
  bbox = copy.bbox;
}

csPoly2D::~csPoly2D ()
{
  CHK (delete [] vertices);
}

void csPoly2D::MakeEmpty ()
{
  num_vertices = 0;
  bbox.StartBoundingBox ();
}

bool csPoly2D::In (const csVector2& v)
{
  int i, i1;
  i1 = num_vertices-1;
  for (i = 0 ; i < num_vertices ; i++)
  {
    if (csMath2::WhichSide2D (v, vertices[i1], vertices[i]) < 0) return false;
    i1 = i;
  }
  return true;
}

bool csPoly2D::In (csVector2* poly, int num_poly, const csVector2& v)
{
  int i, i1;
  i1 = num_poly-1;
  for (i = 0 ; i < num_poly ; i++)
  {
    if (csMath2::WhichSide2D (v, poly[i1], poly[i]) < 0) return false;
    i1 = i;
  }
  return true;
}

void csPoly2D::MakeRoom (int new_max)
{
  if (new_max <= max_vertices) return;
  CHK (csVector2* new_vertices = new csVector2 [new_max]);
  memcpy (new_vertices, vertices, num_vertices*sizeof (csVector2));
  CHK (delete [] vertices);
  vertices = new_vertices;
  max_vertices = new_max;
}

int csPoly2D::AddVertex (float x, float y)
{
  if (num_vertices >= max_vertices)
    MakeRoom (max_vertices+5);
  vertices[num_vertices].x = x;
  vertices[num_vertices].y = y;
  num_vertices++;
  bbox.AddBoundingVertex (x, y);
  return num_vertices-1;
}

void csPoly2D::UpdateBoundingBox ()
{
  int i;
  bbox.StartBoundingBox (vertices[0]);
  for (i = 1 ; i < num_vertices ; i++)
    bbox.AddBoundingVertex (vertices[i]);
}

bool csPoly2D::ClipAgainst (csClipper* view)
{
  MakeRoom (num_vertices+view->GetNumVertices ()+1);
  return view->Clip (vertices, num_vertices, max_vertices, &bbox);
}

void csPoly2D::Intersect (const csPlane2& plane,
	csPoly2D* left, csPoly2D* right)
{
  int i, i1;
  float c, c1;
  csVector2 isect;
  float dist;

  // If 0 then don't know, else if 1 then prefer right,
  // else prefer left. This is used for trying to put edges
  // that coincide with the splitter plane on the prefered
  // side (i.e. a side that contains other edges).
  int preferred_dir = 0;

  // If skip is > 0 then we skipped the 'skip' edges because
  // we didn't have enough information for putting them
  // on the 'right' polygon.
  int skip = 0;

  left->SetNumVertices (0);
  right->SetNumVertices (0);

  i1 = num_vertices-1;
  c1 = plane.Classify (vertices[i1]);
  if (c1 <= -EPSILON) preferred_dir = -1;
  else if (c1 >= EPSILON) preferred_dir = 1;

  for (i = 0 ; i < num_vertices ; i++)
  {
    c = plane.Classify (vertices[i]);
    if (c > -EPSILON && c < EPSILON)
    {
      // This vertex is on the edge. Add it to the
      // left or right polygon depending on preferred_dir.
      if (preferred_dir == 0)
      {
        // The previous vertex was also on the edge and we
	// don't have enough information about the preferred
	// side to use. Skip this edge for later.
	skip++;
      }
      else if (preferred_dir == -1)
	left->AddVertex (vertices[i]);
      else
	right->AddVertex (vertices[i]);
    }
    else if (c <= -EPSILON && c1 < EPSILON)
    {
      // This vertex is on the left and the previous
      // vertex is not right (i.e. on the left or on the edge).
      left->AddVertex (vertices[i]);
      preferred_dir = -1;
    }
    else if (c >= EPSILON && c1 > -EPSILON)
    {
      // This vertex is on the right and the previous
      // vertex is not left.
      right->AddVertex (vertices[i]);
      preferred_dir = 1;
    }
    else
    {
      // We need to split.
      csIntersect2::Plane (vertices[i1], vertices[i],
      	plane, isect, dist);
      right->AddVertex (isect);
      left->AddVertex (isect);
      if (c <= 0)
      {
	left->AddVertex (vertices[i]);
	preferred_dir = -1;
      }
      else
      {
	right->AddVertex (vertices[i]);
	preferred_dir = 1;
      }
    }

    i1 = i;
    c1 = c;
  }

  // If skip > 0 then there are a number of edges in
  // the beginning that we ignored. These edges are all coplanar
  // with 'plane'. We will add them to the preferred side.
  i = 0;
  while (skip > 0)
  {
    if (preferred_dir == -1)
      left->AddVertex (vertices[i]);
    else
      right->AddVertex (vertices[i]);
    i++;
    skip--;
  }
}

//---------------------------------------------------------------------------
