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
#include "csutil/sysfunc.h"

CS_IMPLEMENT_STATIC_CLASSVAR (csPoly2DFactory, sharedFactory, \
			      SharedFactory, csPoly2DFactory,())

csPoly2D::csPoly2D (size_t start_size)
{
  max_vertices = start_size;
  vertices = new csVector2[max_vertices];
  MakeEmpty ();
}

csPoly2D::csPoly2D (const csPoly2D &copy)
{
  max_vertices = copy.max_vertices;
  vertices = new csVector2[max_vertices];
  num_vertices = copy.num_vertices;
  memcpy (vertices, copy.vertices, sizeof (csVector2) * num_vertices);
}

csPoly2D &csPoly2D::operator= (const csPoly2D &other)
{
  if (other.num_vertices <= max_vertices)
  {
    num_vertices = other.num_vertices;
    if (num_vertices)
      memcpy (vertices, other.vertices, sizeof (csVector2) * num_vertices);
  }
  else
  {
    delete[] vertices;
    max_vertices = other.max_vertices;
    vertices = new csVector2[max_vertices];
    num_vertices = other.num_vertices;
    if (num_vertices)
      memcpy (vertices, other.vertices, sizeof (csVector2) * num_vertices);
  }

  return *this;
}

csPoly2D::~csPoly2D ()
{
  delete[] vertices;
}

void csPoly2D::MakeEmpty ()
{
  num_vertices = 0;
}

bool csPoly2D::In (const csVector2 &v)
{
  if (num_vertices > 0)
  {
    size_t i1 = num_vertices - 1;
    for (size_t i = 0; i < num_vertices; i++)
    {
      if (csMath2::WhichSide2D (v, vertices[i1], vertices[i]) < 0)
	return false;
      i1 = i;
    }
  }
  return true;
}

bool csPoly2D::In (csVector2 *poly, size_t num_poly, const csVector2 &v)
{
  if (num_poly > 0)
  {
    size_t i1 = num_poly - 1;
    for (size_t i = 0; i < num_poly; i++)
    {
      if (csMath2::WhichSide2D (v, poly[i1], poly[i]) < 0)
	return false;
      i1 = i;
    }
  }
  return true;
}

void csPoly2D::MakeRoom (size_t new_max)
{
  if (new_max <= max_vertices) return ;

  csVector2 *new_vertices = new csVector2[new_max];
  memcpy (new_vertices, vertices, num_vertices * sizeof (csVector2));
  delete[] vertices;
  vertices = new_vertices;
  max_vertices = new_max;
}

size_t csPoly2D::AddVertex (float x, float y)
{
  if (num_vertices >= max_vertices) MakeRoom (max_vertices + 5);
  vertices[num_vertices].x = x;
  vertices[num_vertices].y = y;
  num_vertices++;
  return num_vertices - 1;
}

bool csPoly2D::ClipAgainst (iClipper2D *view)
{
  MakeRoom (num_vertices + view->GetVertexCount () + 1);
  csBox2 bbox (vertices[0]);
  for (size_t i = 1 ; i < num_vertices ; i++)
    bbox.AddBoundingVertexSmart (vertices[i]);
  return view->ClipInPlace (vertices, num_vertices, bbox) ? true : false;
}

void csPoly2D::Intersect (
  const csPlane2 &plane,
  csPoly2D &left,
  csPoly2D &right) const
{
  left.MakeEmpty ();
  right.MakeEmpty ();
  if (num_vertices == 0)
    return;

  size_t i, i1;
  float c, c1;
  csVector2 isect;
  float dist;

  // The skip variables hold the number of initial skipped vertices.
  // Those are vertices that are on the plane so in principle they should
  // get added to both polygons. However, we try not to generate degenerate
  // polygons (one edge only) so we only add those plane-vertices if
  // we know that the polygon has other vertices too.
  size_t skip_left = 0, skip_right = 0;

  // Ignore the specified number of vertices in the beginning (just
  // before skip_??? vertices).
  size_t ignore_left = 0, ignore_right = 0;

  i1 = num_vertices - 1;
  c1 = plane.Classify (vertices[i1]);

  for (i = 0; i < num_vertices; i++)
  {
    c = plane.Classify (vertices[i]);
    if (c > -EPSILON && c < EPSILON)
    {
      // This vertex is on the edge. Add it to both polygons
      // unless the polygon has no vertices yet. In that
      // case we remember it for later (skip_xxx var) so
      // that we can later add them if the polygon ever
      // gets vertices.
      if (left.GetVertexCount ())
        left.AddVertex (vertices[i]);
      else
        skip_left++;
      if (right.GetVertexCount ())
        right.AddVertex (vertices[i]);
      else
        skip_right++;
    }
    else if (c <= -EPSILON && c1 < EPSILON)
    {
      // This vertex is on the left and the previous
      // vertex is not right (i.e. on the left or on the edge).
      left.AddVertex (vertices[i]);
      if (!skip_right && !right.GetVertexCount ()) ignore_right++;
    }
    else if (c >= EPSILON && c1 > -EPSILON)
    {
      // This vertex is on the right and the previous
      // vertex is not left.
      right.AddVertex (vertices[i]);
      if (!skip_left && !left.GetVertexCount ()) ignore_left++;
    }
    else
    {
      // We need to split.
      csIntersect2::SegmentPlane (vertices[i1], vertices[i], plane,
      	isect, dist);
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
  if (left.GetVertexCount ())
    while (skip_left > 0)
    {
      left.AddVertex (vertices[i]);
      i++;
      skip_left--;
    }

  i = ignore_right;
  if (right.GetVertexCount ())
    while (skip_right > 0)
    {
      right.AddVertex (vertices[i]);
      i++;
      skip_right--;
    }
}

void csPoly2D::ClipPlane (const csPlane2 &plane,
	csPoly2D &right) const
{
  right.MakeEmpty ();
  if (num_vertices == 0)
    return;

  size_t i, i1;
  float c, c1;
  csVector2 isect;
  float dist;

  // The skip variables hold the number of initial skipped vertices.
  // Those are vertices that are on the plane so in principle they should
  // get added to both polygons. However, we try not to generate degenerate
  // polygons (one edge only) so we only add those plane-vertices if
  // we know that the polygon has other vertices too.
  size_t skip_right = 0;

  // Ignore the specified number of vertices in the beginning (just
  // before skip_right vertices).
  size_t ignore_right = 0;

  i1 = num_vertices - 1;
  c1 = plane.Classify (vertices[i1]);

  for (i = 0; i < num_vertices; i++)
  {
    c = plane.Classify (vertices[i]);
    if (c > -EPSILON && c < EPSILON)
    {
      // This vertex is on the edge. Add it to both polygons
      // unless the polygon has no vertices yet. In that
      // case we remember it for later (skip_xxx var) so
      // that we can later add them if the polygon ever
      // gets vertices.
      if (right.GetVertexCount ())
      {
        right.AddVertex (vertices[i]);
      }
      else
      {
        skip_right++;
      }
    }
    else if (c <= -EPSILON && c1 < EPSILON)
    {
      // This vertex is on the left and the previous
      // vertex is not right (i.e. on the left or on the edge).
      if (!skip_right && !right.GetVertexCount ())
      {
        ignore_right++;
      }
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
      csIntersect2::SegmentPlane (vertices[i1], vertices[i], plane,
      	isect, dist);
      right.AddVertex (isect);
      if (c > 0)
      {
        right.AddVertex (vertices[i]);
      }
    }

    i1 = i;
    c1 = c;
  }

  // If skip_xxx > 0 then there are a number of vertices in
  // the beginning that we ignored. These vertices are all on
  // 'plane'. We will add them to the corresponding polygon if
  // that polygon is not empty.
  i = ignore_right;
  if (right.GetVertexCount ())
  {
    while (skip_right > 0)
    {
      right.AddVertex (vertices[i]);
      i++;
      skip_right--;
    }
  }
}

void csPoly2D::ExtendConvex (const csPoly2D &other, size_t i1)
{
  // Some conventions:
  //   i1, i2: edge of this polygon common with 'other'.
  //   j1, j2: edge of other polygon common with 'this'.
  //   i1 corresponds with j2
  //   i2 corresponds with j1
  size_t i2 = (i1 + 1) % num_vertices;
  size_t j1, j2;
  size_t i, j, jp;

  // First find j1 and j2.
  j2 = csArrayItemNotFound;
  for (j = 0; j < other.GetVertexCount (); j++)
  {
    if ((vertices[i1] - other[j]) < EPSILON)
    {
      j2 = j;
      break;
    }
  }

  CS_ASSERT (j2 != csArrayItemNotFound);

  j1 = (j2 - 1 + other.GetVertexCount ()) % other.GetVertexCount ();

  // Double check if i2 and j1 really match.
  if (!((vertices[i2] - other[j1]) < EPSILON))
  {
    for (i = 0; i < GetVertexCount (); i++)
      csPrintf ("  orig %lu: %f,%f\n",
		(unsigned long)i, (*this)[i].x, (*this)[i].y);
    for (i = 0; i < other.GetVertexCount (); i++)
      csPrintf ("  other %lu: %f,%f\n",
		(unsigned long)i, other[i].x, other[i].y);
    csPrintf ("  i1=%lu i2=%lu j1=%lu j2=%lu\n", (unsigned long)i1,
	      (unsigned long)i2, (unsigned long)j1, (unsigned long)j2);
    CS_ASSERT (false);
  }

  // Copy this polygon to 'orig' and clear this one.
  csPoly2D orig (*this);
  size_t orig_num = orig.GetVertexCount ();
  size_t other_num = other.GetVertexCount ();
  MakeEmpty ();

  // Add the vertex just before i1. We will start our new
  // polygon with this one.
  AddVertex (orig[(i1 - 1 + orig_num) % orig_num]);

  // Construct two 2D planes for i1-1 to i1 and i2 to i2+1. These
  // planes will be used to check what vertices of the other polygon
  // we will retain and which we will discard. These two planes in
  // effect define the subset of the other polygon that we are
  // interested in. This subset we add (union) to this polygon.
  csPlane2 pl1, pl2;
  pl1.Set (orig[(i1 - 1 + orig_num) % orig_num], orig[i1]);
  pl1.Normalize (); //@@@ Needed?
  pl2.Set (orig[i2], orig[(i2 + 1) % orig_num]);
  pl2.Normalize (); //@@@ Needed?

  // Start scanning the other polygon starting with j2+1.
  // While the vertices of the other polygon are on the left side
  // of pl1 we ignore them.
  jp = j2;
  j = (j2 + 1) % other_num;

  size_t cnt = other_num;
  while (pl1.Classify (other[j]) > EPSILON)
  {
    jp = j;
    j = (j + 1) % other_num;
    cnt--;
    if (cnt == (size_t)-1)
    {
      csPrintf ("INTERNAL ERROR! Looping forever!\n");
      for (i = 0; i < orig.GetVertexCount (); i++)
        csPrintf ("  orig %lu: %f,%f\n",
		  (unsigned long)i, orig[i].x, orig[i].y);
      for (i = 0; i < other.GetVertexCount (); i++)
        csPrintf ("  other %lu: %f,%f\n",
		  (unsigned long)i, other[i].x, other[i].y);
      csPrintf ("  i1=%lu i2=%lu j1=%lu j2=%lu\n", (unsigned long)i1,
		(unsigned long)i2, (unsigned long)j1, (unsigned long)j2);
      CS_ASSERT (false);
    }
  }

  csVector2 isect;
  float dist;

  // If jp == j2 then we know that the first vertex after i1 is already
  // right of pl1 so we consider j2 or i1 the intersection point to
  // continue with the rest of the processing below.
  if (jp == j2)
  {
    isect = other[j2];
  }
  else
  {
    // jp to j is an edge which is intersected by pl1. The intersection
    // point is what we need.
    csIntersect2::SegmentPlane (other[jp], other[j], pl1, isect, dist);
  }

  // If the intersection point is on the left of pl2 then we know
  // that the intersection point of the two planes itself will define
  // the new extended polygon. In that case we can simply add that
  // intersection point to the this polygon and add the rest of the
  // vertices as well.
  if (pl2.Classify (isect) > EPSILON)
  {
    csIntersect2::PlanePlane (pl1, pl2, isect);
    AddVertex (isect);
    i = (i2 + 1) % orig_num;
    while (i != (i1 - 1 + orig_num) % orig_num)
    {
      AddVertex (orig[i]);
      i = (i + 1) % orig_num;
    }
    return;
  }

  // Otherwise the intersection point is going to be part of the

  // polygon.
  AddVertex (isect);

  // Now we continue scanning the other polygon starting with the
  // vertex j where we ended. We add all vertices that are on the
  // right of the second plane.
  while (j != j1)
  {
    if (pl2.Classify (other[j]) < -EPSILON)
      AddVertex (other[j]);
    else
      break;
    jp = j;
    j = (j + 1) % other_num;
  }

  // If j == j1 then all other vertices are right so we must add
  // j1 (or i2) itself then continue with the rest of this polygon.
  if (j == j1)
  {
    i = i2;
    while (i != (i1 - 1 + orig_num) % orig_num)
    {
      AddVertex (orig[i]);
      i = (i + 1) % orig_num;
    }
    return;
  }

  // Otherwise the edge jp to j crosses the second plane. In this
  // case we intersect again and ignore the rest of 'other'.
  csIntersect2::SegmentPlane (other[jp], other[j], pl2, isect, dist);
  AddVertex (isect);
  i = (i2 + 1) % orig_num;
  while (i != (i1 - 1 + orig_num) % orig_num)
  {
    AddVertex (orig[i]);
    i = (i + 1) % orig_num;
  }
}

float csPoly2D::GetSignedArea ()
{
  float area = 0.0;
  // triangulize the polygon, triangles are (0,1,2), (0,2,3), (0,3,4), etc..
  for (size_t i = 0; i < GetVertexCount () - 2; i++)
    area += csMath2::Area2 (vertices[0], vertices[i + 1], vertices[i + 2]);
  return area / 2.0f;
}

static float randflt ()
{
  return ((float)rand ()) / RAND_MAX;
}

void csPoly2D::Random (size_t num, const csBox2 &max_bbox)
{
  MakeEmpty ();

  csVector2 v;
  float w = max_bbox.MaxX () - max_bbox.MinX ();
  float h = max_bbox.MaxY () - max_bbox.MinY ();
  float dx = max_bbox.MinX ();
  float dy = max_bbox.MinY ();
  for (size_t i = 0; i < 3; i++)
  {
    v.Set (randflt () * w + dx, randflt () * h + dy);
    AddVertex (v);
  }

  // @@@ Only triangles are supported right now.
  (void)num;
}
