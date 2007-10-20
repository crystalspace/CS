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

#include "csgeom/bsptree.h"
#include "csgeom/math3d.h"
#include "csgeom/tri.h"
#include "csutil/set.h"

//---------------------------------------------------------------------------

CS_IMPLEMENT_STATIC_CLASSVAR_REF (csBSPTree, b2fArray, B2fArray, 
  csDirtyAccessArray<int>, ());
namespace
{
  CS_IMPLEMENT_STATIC_VAR (TreeNodes, csBlockAllocator<csBSPTree>, (1000));
};

csBSPTree::csBSPTree ()
{
  child1 = 0;
  child2 = 0;
}

csBSPTree::~csBSPTree ()
{
  Clear ();
}

void csBSPTree::Clear ()
{
  if (child1)
  {
    TreeNodes()->Free (child1);
    child1 = 0;
  }
  if (child2)
  {
    TreeNodes()->Free (child2);
    child2 = 0;
  }
}

namespace
{
  static int ClassifyPlane (const csPlane3& plane, const csVector3& p)
  {
    float fcl = plane.Classify (p);
    if (fcl < 0) return -1;
    else if (fcl > 0) return 1;
    else return 0;
  }
}

size_t csBSPTree::FindBestSplitter (csTriangle* triangles, csPlane3* planes,
	size_t /*num_triangles*/, const csVector3* vertices,
	const csArray<int>& triidx)
{
  size_t i, j;
  float mincost = 1000000.0;
  size_t minidx = (size_t)-1;
  for (i = 0 ; i < triidx.GetSize () ; i++)
  {
    int cnt_splits = 0;
    int cnt_left = 0;
    int cnt_right = 0;
    csPlane3& pl = planes[triidx[i]];
    for (j = 0 ; j < triidx.GetSize () ; j++)
      if (i != j)
      {
        csTriangle& trj = triangles[triidx[j]];
	int cla = ClassifyPlane (pl, vertices[trj.a]);
	int clb = ClassifyPlane (pl, vertices[trj.b]);
	int clc = ClassifyPlane (pl, vertices[trj.c]);
	if ((cla == -clb && cla != 0) ||
	    (cla == -clc && cla != 0) ||
	    (clb == -clc && clb != 0))
	{
	  // There is a split.
	  cnt_splits++;
	}
	else
	{
	  if (cla == -1 || clb == -1 || clc == -1)
	    cnt_left++;
	  else if (cla == 1 || clb == 1 || clc == 1)
	    cnt_right++;
	}
      }
    float split = float (cnt_splits) / float (triidx.GetSize ());
    float balance = float (ABS (cnt_left-cnt_right)) / float (triidx.GetSize ());
    float cost = 10.0 * split + balance;
    if (cost < mincost)
    {
      minidx = i;
      mincost = cost;
    }
  }
  return minidx;
}

void csBSPTree::Build (csTriangle* triangles, csPlane3* planes,
	size_t num_triangles, const csVector3* vertices,
	const csArray<int>& triidx)
{
  CS_ASSERT (triidx.GetSize () > 0);
  if (triidx.GetSize () == 1)
  {
    splitters.Push (triidx[0]);
    return;
  }

  size_t idx = FindBestSplitter (triangles, planes, num_triangles, vertices,
  	triidx);
  CS_ASSERT (idx != (size_t)-1);
  splitters.Push (triidx[idx]);

  csArray<int> left;
  csArray<int> right;
  size_t i;
  split_plane = planes[triidx[idx]];
  for (i = 0 ; i < triidx.GetSize () ; i++)
    if (i != idx)
    {
      int idxi = triidx[i];
      csTriangle& trj = triangles[idxi];
      int cla = ClassifyPlane (split_plane, vertices[trj.a]);
      int clb = ClassifyPlane (split_plane, vertices[trj.b]);
      int clc = ClassifyPlane (split_plane, vertices[trj.c]);
      if ((cla == -clb && cla != 0) ||
	  (cla == -clc && cla != 0) ||
	  (clb == -clc && clb != 0))
      {
	// There is a split.
	left.Push (idxi);
	right.Push (idxi);
      }
      else
      {
	if (cla == -1 || clb == -1 || clc == -1)
	  left.Push (idxi);
	else if (cla == 1 || clb == 1 || clc == 1)
	  right.Push (idxi);
        else
	  splitters.Push (idxi);
      }
    }
  if (left.GetSize () > 0)
  {
    child1 = TreeNodes()->Alloc ();
    child1->Build (triangles, planes, num_triangles, vertices, left);
  }
  if (right.GetSize () > 0)
  {
    child2 = TreeNodes()->Alloc ();
    child2->Build (triangles, planes, num_triangles, vertices, right);
  }
}

void csBSPTree::Build (CS::TriangleIndicesStream<int>& triangles,
	               const csVector3* vertices)
{
  const size_t triComponents = triangles.GetRemainingComponents();
  csDirtyAccessArray<csPlane3> planes ((triComponents+2)/3);
  csArray<int> triidx;
  csDirtyAccessArray<csTriangle> tris ((triComponents+2)/3);
  while (triangles.HasNext())
  {
    CS::TriangleT<int> t (triangles.Next());
    planes.Push (csPlane3 (vertices[t.a], vertices[t.b], vertices[t.c]));
    triidx.Push (int (tris.Push (t)));
  }

  Build (tris.GetArray(), planes.GetArray(), tris.GetSize(), vertices, triidx);
}

void csBSPTree::Build (csTriangle* triangles, size_t num_triangles,
	               const csVector3* vertices)
{
  csPlane3* planes = new csPlane3[num_triangles];
  csArray<int> triidx;
  size_t i;
  for (i = 0 ; i < num_triangles ; i++)
  {
    csTriangle& t = triangles[i];
    planes[i].Set (vertices[t.a], vertices[t.b], vertices[t.c]);
    triidx.Push (int (i));
  }

  Build (triangles, planes, num_triangles, vertices, triidx);
  delete[] planes;
}

void csBSPTree::Back2Front (const csVector3& pos, csDirtyAccessArray<int>& arr,
  	csSet<int>& used_indices)
{
  int cl = ClassifyPlane (split_plane, pos);

  if (cl < 0)
  {
    if (child2) child2->Back2Front (pos, arr, used_indices);
  }
  else
  {
    if (child1) child1->Back2Front (pos, arr, used_indices);
  }

  size_t i;
  for (i = 0 ; i < splitters.GetSize () ; i++)
    if (!used_indices.In (splitters[i]))
    {
      used_indices.AddNoTest (splitters[i]);
      arr.Push (splitters[i]);
    }

  if (cl < 0)
  {
    if (child1) child1->Back2Front (pos, arr, used_indices);
  }
  else
  {
    if (child2) child2->Back2Front (pos, arr, used_indices);
  }
}

const csDirtyAccessArray<int>& csBSPTree::Back2Front (const csVector3& pos)
{
  B2fArray().Empty ();
  csSet<int> used_indices;
  Back2Front (pos, B2fArray(), used_indices);
  return B2fArray();
}

