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
#include "csgeom/polytree.h"
#include "csgeom/math3d.h"
#include "igeom/polymesh.h"

//---------------------------------------------------------------------------

csPolygonTree::csPolygonTree ()
{
  split_axis = CS_POLYTREE_AXISINVALID;
  child1 = 0;
  child2 = 0;
}

csPolygonTree::~csPolygonTree ()
{
  delete child1;
  delete child2;
}

void csPolygonTree::Clear ()
{
  delete child1;
  child1 = 0;
  delete child1;
  child2 = 0;
  polygons.DeleteAll ();
  split_axis = CS_POLYTREE_AXISINVALID;
}

void csPolygonTree::CalculateBBox (csArray<int>& polyidx,
	iPolygonMesh* mesh)
{
  size_t i;
  csVector3* vertices = mesh->GetVertices ();
  csMeshedPolygon* polys = mesh->GetPolygons ();
  bbox.StartBoundingBox ();
  for (i = 0 ; i < polyidx.Length () ; i++)
  {
    int j;
    csMeshedPolygon& p = polys[polyidx[i]];
    int* vt = p.vertices;
    for (j = 0 ; j < p.num_vertices ; j++)
      bbox.AddBoundingVertex (vertices[vt[j]]);
  }
}

void csPolygonTree::MakeLeaf (csArray<int>& polyidx)
{
  polygons = polyidx;
}

void csPolygonTree::Build (csArray<int>& polyidx,
  	iPolygonMesh* mesh)
{
  CalculateBBox (polyidx, mesh);

  if (polyidx.Length () < 10)
  {
    MakeLeaf (polyidx);
    return;
  }

  float xsize = bbox.MaxX () - bbox.MinX ();
  float ysize = bbox.MaxY () - bbox.MinY ();
  float zsize = bbox.MaxZ () - bbox.MinZ ();
  if (xsize > ysize && xsize > zsize)
  {
    split_axis = CS_POLYTREE_AXISX;
    split_location = (bbox.MaxX () + bbox.MinX ()) / 2.0;
  }
  else if (ysize > zsize)
  {
    split_axis = CS_POLYTREE_AXISY;
    split_location = (bbox.MaxY () + bbox.MinY ()) / 2.0;
  }
  else
  {
    split_axis = CS_POLYTREE_AXISZ;
    split_location = (bbox.MaxZ () + bbox.MinZ ()) / 2.0;
  }

  size_t i;
  csArray<int> left;
  csArray<int> right;
  csMeshedPolygon* polys = mesh->GetPolygons ();
  csVector3* vertices = mesh->GetVertices ();
  for (i = 0 ; i < polyidx.Length () ; i++)
  {
    csMeshedPolygon& p = polys[polyidx[i]];
    int* vt = p.vertices;
    int j;
    bool do_left = false, do_right = false;
    for (j = 0 ; j < p.num_vertices ; j++)
    {
      csVector3& v = vertices[vt[j]];
      if (v[split_axis] <= split_location)
        do_left = true;
      else
        do_right = true;
    }
    if (do_left) left.Push (polyidx[i]);
    if (do_right) right.Push (polyidx[i]);
  }
  if (left.Length () == polyidx.Length () || right.Length () == polyidx.Length () ||
      left.Length () == 0 || right.Length () == 0)
  {
    // We can't distribute further so this becomes a leaf.
    MakeLeaf (polyidx);
  }
  else
  {
    child1 = new csPolygonTree ();
    child1->Build (left, mesh);
    child2 = new csPolygonTree ();
    child2->Build (right, mesh);
  }
}

void csPolygonTree::Build (iPolygonMesh* mesh)
{
  csArray<int> polyidx;
  int i;
  for (i = 0 ; i < mesh->GetPolygonCount () ; i++)
    polyidx.Push (i);
  Build (polyidx, mesh);
}

void csPolygonTree::IntersectBox (csArray<int>& polyidx, const csBox3& box)
{
  if (box.TestIntersect (bbox))
  {
    size_t i;
    for (i = 0 ; i < polygons.Length () ; i++)
      polyidx.Push (polygons[i]);
    if (child1) child1->IntersectBox (polyidx, box);
    if (child2) child2->IntersectBox (polyidx, box);
  }
}

void csPolygonTree::IntersectSphere (csArray<int>& polyidx,
	const csVector3& center, float sqradius)
{
  if (csIntersect3::BoxSphere (bbox, center, sqradius))
  {
    size_t i;
    for (i = 0 ; i < polygons.Length () ; i++)
      polyidx.Push (polygons[i]);
    if (child1) child1->IntersectSphere (polyidx, center, sqradius);
    if (child2) child2->IntersectSphere (polyidx, center, sqradius);
  }
}

static int intsort (int const& i1, int const& i2)
{
  if (i1 < i2) return -1;
  else if (i2 < i1) return 1;
  else return 0;
}

void csPolygonTree::RemoveDoubles (csArray<int>& polyidx)
{
  polyidx.Sort (intsort);
  size_t i;
  int i1 = 0;
  int prev = -1;
  for (i = 0 ; i < polyidx.Length () ; i++)
  {
    if (polyidx[i] != prev)
    {
      polyidx[i1++] = polyidx[i];
      prev = polyidx[i];
    }
  }
  polyidx.Truncate (i1);
}

