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

#include "cssysdef.h"
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
  vertices = new csVector2 [max_vertices];
  MakeEmpty ();
}

csPoly2D::csPoly2D (const csPoly2D& copy)
{
  max_vertices = copy.max_vertices;
  vertices = new csVector2 [max_vertices];
  num_vertices = copy.num_vertices;
  memcpy (vertices, copy.vertices, sizeof (csVector2)*num_vertices);
  bbox = copy.bbox;
}

csPoly2D& csPoly2D::operator= (const csPoly2D& other)
{
  if (other.num_vertices <= max_vertices)
  {
    num_vertices = other.num_vertices;
    if (num_vertices)
      memcpy (vertices, other.vertices, sizeof (csVector2)*num_vertices);
  }
  else
  {
    delete [] vertices;
    max_vertices = other.max_vertices;
    vertices = new csVector2 [max_vertices];
    num_vertices = other.num_vertices;
    if (num_vertices)
      memcpy (vertices, other.vertices, sizeof (csVector2)*num_vertices);
  }
  bbox = other.bbox;
  return *this;
}

csPoly2D::~csPoly2D ()
{
  delete [] vertices;
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
  csVector2* new_vertices = new csVector2 [new_max];
  memcpy (new_vertices, vertices, num_vertices*sizeof (csVector2));
  delete [] vertices;
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

bool csPoly2D::ClipAgainst (csClipper *view)
{
  MakeRoom (num_vertices + view->GetNumVertices () + 1);
  return view->Clip (vertices, num_vertices, bbox);
}

void csPoly2D::Intersect (const csPlane2& plane,
	csPoly2D& left, csPoly2D& right) const
{
  int i, i1;
  float c, c1;
  csVector2 isect;
  float dist;

  // The skip variables hold the number of initial skipped vertices.
  // Those are vertices that are on the plane so in principle they should
  // get added to both polygons. However, we try not to generate degenerate
  // polygons (one edge only) so we only add those plane-vertices if
  // we know that the polygon has other vertices too.
  int skip_left = 0, skip_right = 0;
  // Ignore the specified number of vertices in the beginning (just
  // before skip_??? vertices).
  int ignore_left = 0, ignore_right = 0;

  left.MakeEmpty ();
  right.MakeEmpty ();

  i1 = num_vertices-1;
  c1 = plane.Classify (vertices[i1]);

  for (i = 0 ; i < num_vertices ; i++)
  {
    c = plane.Classify (vertices[i]);
    if (c > -EPSILON && c < EPSILON)
    {
      // This vertex is on the edge. Add it to both polygons
      // unless the polygon has no vertices yet. In that
      // case we remember it for later (skip_xxx var) so
      // that we can later add them if the polygon ever
      // gets vertices.
      if (left.GetNumVertices ())
        left.AddVertex (vertices[i]);
      else
        skip_left++;
      if (right.GetNumVertices ())
        right.AddVertex (vertices[i]);
      else
        skip_right++;
    }
    else if (c <= -EPSILON && c1 < EPSILON)
    {
      // This vertex is on the left and the previous
      // vertex is not right (i.e. on the left or on the edge).
      left.AddVertex (vertices[i]);
      if (!skip_right && !right.GetNumVertices ())
        ignore_right++;
    }
    else if (c >= EPSILON && c1 > -EPSILON)
    {
      // This vertex is on the right and the previous
      // vertex is not left.
      right.AddVertex (vertices[i]);
      if (!skip_left && !left.GetNumVertices ())
        ignore_left++;
    }
    else
    {
      // We need to split.
      csIntersect2::Plane (vertices[i1], vertices[i],
      	plane, isect, dist);
      right.AddVertex (isect);
      left.AddVertex (isect);
      if (c <= 0)
	left.AddVertex (vertices[i]);
      else
	right.AddVertex (vertices[i]);
    }

    i1 = i;
    c1 = c;
  }

  // If skip_xxx > 0 then there are a number of vertices in
  // the beginning that we ignored. These vertices are all on
  // 'plane'. We will add them to the corresponding polygon if
  // that polygon is not empty.
  i = ignore_left;
  if (left.GetNumVertices ())
    while (skip_left > 0)
    {
      left.AddVertex (vertices[i]);
      i++;
      skip_left--;
    }
  i = ignore_right;
  if (right.GetNumVertices ())
    while (skip_right > 0)
    {
      right.AddVertex (vertices[i]);
      i++;
      skip_right--;
    }
}

void csPoly2D::ClipPlane (const csPlane2& plane, csPoly2D& right) const
{
  int i, i1;
  float c, c1;
  csVector2 isect;
  float dist;

  // The skip variables hold the number of initial skipped vertices.
  // Those are vertices that are on the plane so in principle they should
  // get added to both polygons. However, we try not to generate degenerate
  // polygons (one edge only) so we only add those plane-vertices if
  // we know that the polygon has other vertices too.
  int skip_right = 0;
  // Ignore the specified number of vertices in the beginning (just
  // before skip_right vertices).
  int ignore_right = 0;

  right.MakeEmpty ();

  i1 = num_vertices-1;
  c1 = plane.Classify (vertices[i1]);

  for (i = 0 ; i < num_vertices ; i++)
  {
    c = plane.Classify (vertices[i]);
    if (c > -EPSILON && c < EPSILON)
    {
      // This vertex is on the edge. Add it to both polygons
      // unless the polygon has no vertices yet. In that
      // case we remember it for later (skip_xxx var) so
      // that we can later add them if the polygon ever
      // gets vertices.
      if (right.GetNumVertices ())
        right.AddVertex (vertices[i]);
      else
        skip_right++;
    }
    else if (c <= -EPSILON && c1 < EPSILON)
    {
      // This vertex is on the left and the previous
      // vertex is not right (i.e. on the left or on the edge).
      if (!skip_right && !right.GetNumVertices ())
        ignore_right++;
    }
    else if (c >= EPSILON && c1 > -EPSILON)
    {
      // This vertex is on the right and the previous
      // vertex is not left.
      right.AddVertex (vertices[i]);
    }
    else
    {
      // We need to split.
      csIntersect2::Plane (vertices[i1], vertices[i],
      	plane, isect, dist);
      right.AddVertex (isect);
      if (c > 0)
	right.AddVertex (vertices[i]);
    }

    i1 = i;
    c1 = c;
  }

  // If skip_xxx > 0 then there are a number of vertices in
  // the beginning that we ignored. These vertices are all on
  // 'plane'. We will add them to the corresponding polygon if
  // that polygon is not empty.
  i = ignore_right;
  if (right.GetNumVertices ())
    while (skip_right > 0)
    {
      right.AddVertex (vertices[i]);
      i++;
      skip_right--;
    }
}

void csPoly2D::ExtendConvex (const csPoly2D& other, int this_edge)
{
  int this_edge2 = (this_edge+1)%num_vertices;
  int i, i1, j;

  // First clip the other polygon to the two edges from this polygon
  // that connect to the shared edge. This way we can reduce the algorithm
  // to just having to connect the two remaining polygons. The resulting
  // connection will be convex.
  csPoly2D other1;
  csPlane2 pl;
  pl.Set (vertices[(this_edge-1+num_vertices)%num_vertices],
  	vertices[this_edge]);
  other.ClipPlane (pl, other1);
  if (other1.GetNumVertices () < 0) return;	// Nothing to be done.
  csPoly2D other2;
  pl.Set (vertices[this_edge2], vertices[(this_edge2+1)%num_vertices]);
  other1.ClipPlane (pl, other2);
  if (other2.GetNumVertices () < 0) return;	// Nothing to be done.

  // Find the common edge in this new polygon.
  int other_edge = -1, other_edge2 = -1;
  for (i = 0 ; i < other2.GetNumVertices () ; i++)
  {
    if ((vertices[this_edge]-other2[i]) < EPSILON)
      other_edge2 = i;
    if ((vertices[this_edge2]-other2[i]) < EPSILON)
      other_edge = i;
  }
  if (other_edge == -1 || other_edge2 == -1)
    printf ("INTERNAL ERROR: csPoly2D::ExtendConvex\n");

  csPoly2D orig (*this);
  MakeEmpty ();

  // Now join the two polygons. Note that this function
  // assumes that the two polygons are oriented the same way.
  // i.e. the polygons go along the adjacent edge in seperate directions.
  i1 = orig.num_vertices-1;
  for (i = 0 ; i < orig.num_vertices ; i++)
  {
    // Join other polygon.
    if (i == this_edge)
    {
      j = (other_edge2+1)%other2.GetNumVertices ();
      while (j != other_edge)
      {
	AddVertex (other2[j]);
        j = (j+1)%other2.GetNumVertices ();
      }
    }
    else if (i == this_edge2)
    {
      // Ignore this edge.
    }
    else if (i1 == this_edge2)
    {
      AddVertex (orig[i]);
    }
    else
    {
      AddVertex (orig[i]);
    }
    i1 = i;
  }
}

float csPoly2D::GetSignedArea ()
{
  float area = 0.0;
  // triangulize the polygon, triangles are (0,1,2), (0,2,3), (0,3,4), etc..
  for (int i=0 ; i < GetNumVertices()-2 ; i++)
    area += csMath2::Area2 ( vertices[0], vertices[i+1], vertices[i+2] );
  return area / 2.0;
}


//---------------------------------------------------------------------------
