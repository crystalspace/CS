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
  static csPoly2DFactory* p = 0;
  if (p == 0)
    CHK (p = new csPoly2DFactory);
  return p;
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

bool csPoly2D::ClipAgainst (csClipper* view)
{
  MakeRoom (num_vertices+view->GetNumVertices ()+1);
  return view->Clip (vertices, num_vertices, max_vertices, &bbox);
}

//---------------------------------------------------------------------------
