/*
    Copyright (C) 2000 by Jorrit Tyberghein

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
#include "csgeom/polyedge.h"
#include "csgeom/polyclip.h"

csPoly2DEdges::csPoly2DEdges (int start_size)
{
  max_edges = start_size;
  edges = new csSegment2[max_edges];
  MakeEmpty ();
}

csPoly2DEdges::csPoly2DEdges (csPoly2DEdges &copy)
{
  max_edges = copy.max_edges;
  edges = new csSegment2[max_edges];
  num_edges = copy.num_edges;
  memcpy (edges, copy.edges, sizeof (csSegment2) * num_edges);
}

csPoly2DEdges::~csPoly2DEdges ()
{
  delete[] edges;
}

void csPoly2DEdges::MakeEmpty ()
{
  num_edges = 0;
}

bool csPoly2DEdges::In (const csVector2 &v)
{
  int i;
  for (i = 0; i < num_edges; i++)
  {
    if (csMath2::WhichSide2D (v, edges[i].Start (), edges[i].End ()) < 0)
      return false;
  }

  return true;
}

bool csPoly2DEdges::In (csSegment2 *poly, int num_edge, const csVector2 &v)
{
  int i;
  for (i = 0; i < num_edge; i++)
  {
    if (csMath2::WhichSide2D (v, poly[i].Start (), poly[i].End ()) < 0)
      return false;
  }

  return true;
}

void csPoly2DEdges::MakeRoom (int new_max)
{
  if (new_max <= max_edges) return ;

  csSegment2 *new_edges = new csSegment2[new_max];
  memcpy (new_edges, edges, num_edges * sizeof (csSegment2));
  delete[] edges;
  edges = new_edges;
  max_edges = new_max;
}

int csPoly2DEdges::AddEdge (const csVector2 &v1, const csVector2 &v2)
{
  if (num_edges >= max_edges) MakeRoom (max_edges + 5);
  edges[num_edges].Set (v1, v2);
  num_edges++;
  return num_edges - 1;
}

// The thickness of the plane used in the intersection routine below.
#define THICK (EPSILON * EPSILON)

//#define THICK (.00001*.00001)
#define ONPLANE(c)        ((c) > -THICK && (c) < THICK)
#define ATLEFT(c)         ((c) <= -THICK)
#define ATLEFTORPLANE(c)  ((c) < THICK)
#define ATRIGHT(c)        ((c) >= THICK)
#define ATRIGHTORPLANE(c) ((c) > -THICK)

void csPoly2DEdges::Intersect (
  const csPlane2 &plane,
  csPoly2DEdges &left,
  csPoly2DEdges &right,
  bool &onplane) const
{
  int i;
  float c1, c2;
  csVector2 isect;
  float dist;

  onplane = false;

  left.SetEdgeCount (0);
  right.SetEdgeCount (0);

  for (i = 0; i < num_edges; i++)
  {
    c1 = plane.SquaredDistance (edges[i].Start ());
    c2 = plane.SquaredDistance (edges[i].End ());

    if (ONPLANE (c1))
    {
      if (ONPLANE (c2))
      {
        // Both vertices are on the plane. In this case we ignore the
        // edge and set onplane to true.
        onplane = true;
      }
      else if (ATLEFT (c2))
        left.AddEdge (edges[i]);
      else
        right.AddEdge (edges[i]);
    }
    else if (ATLEFT (c1))
    {
      if (ATLEFTORPLANE (c2))
        left.AddEdge (edges[i]);
      else
      {
        csIntersect2::SegmentPlaneNoTest (edges[i], plane, isect, dist);
        left.AddEdge (edges[i].Start (), isect);
        right.AddEdge (isect, edges[i].End ());
      }
    }
    else  // ATRIGHT (c1)
    {
      if (ATRIGHTORPLANE (c2))
        right.AddEdge (edges[i]);
      else
      {
        csIntersect2::SegmentPlaneNoTest (edges[i], plane, isect, dist);
        right.AddEdge (edges[i].Start (), isect);
        left.AddEdge (isect, edges[i].End ());
      }
    }
  }
}

//---------------------------------------------------------------------------
