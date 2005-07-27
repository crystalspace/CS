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

csBlockAllocator<csBSPTree> csBSPTree::tree_nodes (1000);
csDirtyAccessArray<int> csBSPTree::b2f_array;

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
    tree_nodes.Free (child1);
    child1 = 0;
  }
  if (child2)
  {
    tree_nodes.Free (child2);
    child2 = 0;
  }
}

size_t csBSPTree::FindBestSplitter (csTriangle* triangles, csPlane3* planes,
	int num_triangles, csVector3* vertices,
	const csArray<int>& triidx)
{
  size_t i, j;
  float mincost = 1000000.0;
  size_t minidx = (size_t)-1;
  for (i = 0 ; i < triidx.Length () ; i++)
  {
    int cnt_splits = 0;
    int cnt_left = 0;
    int cnt_right = 0;
    csPlane3& pl = planes[triidx[i]];
    for (j = 0 ; j < triidx.Length () ; j++)
      if (i != j)
      {
        csTriangle& trj = triangles[triidx[j]];
	float fcla, fclb, fclc;
        fcla = pl.Classify (vertices[trj.a]);
        fclb = pl.Classify (vertices[trj.b]);
        fclc = pl.Classify (vertices[trj.c]);
	int cla, clb, clc;
	if (fcla < -EPSILON) cla = -1;
	else if (fcla > EPSILON) cla = 1;
	else cla = 0;
	if (fclb < -EPSILON) clb = -1;
	else if (fclb > EPSILON) clb = 1;
	else clb = 0;
	if (fclc < -EPSILON) clc = -1;
	else if (fclc > EPSILON) clc = 1;
	else clc = 0;
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
    float split = float (cnt_splits) / float (triidx.Length ());
    float balance = float (ABS (cnt_left-cnt_right)) / float (triidx.Length ());
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
	int num_triangles, csVector3* vertices,
	const csArray<int>& triidx)
{
  CS_ASSERT (triidx.Length () > 0);
  if (triidx.Length () == 1)
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
  for (i = 0 ; i < triidx.Length () ; i++)
    if (i != idx)
    {
      int idxi = triidx[i];
      csTriangle& trj = triangles[idxi];
      float fcla, fclb, fclc;
      fcla = split_plane.Classify (vertices[trj.a]);
      fclb = split_plane.Classify (vertices[trj.b]);
      fclc = split_plane.Classify (vertices[trj.c]);
      int cla, clb, clc;
      if (fcla < -EPSILON) cla = -1;
      else if (fcla > EPSILON) cla = 1;
      else cla = 0;
      if (fclb < -EPSILON) clb = -1;
      else if (fclb > EPSILON) clb = 1;
      else clb = 0;
      if (fclc < -EPSILON) clc = -1;
      else if (fclc > EPSILON) clc = 1;
      else clc = 0;
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
  if (left.Length () > 0)
  {
    child1 = tree_nodes.Alloc ();
    child1->Build (triangles, planes, num_triangles, vertices, left);
  }
  if (right.Length () > 0)
  {
    child2 = tree_nodes.Alloc ();
    child2->Build (triangles, planes, num_triangles, vertices, right);
  }
}

void csBSPTree::Build (csTriangle* triangles, int num_triangles,
	csVector3* vertices)
{
  csPlane3* planes = new csPlane3[num_triangles];
  csArray<int> triidx;
  int i;
  for (i = 0 ; i < num_triangles ; i++)
  {
    csTriangle& t = triangles[i];
    planes[i].Set (vertices[t.a], vertices[t.b], vertices[t.c]);
    triidx.Push (i);
  }

  Build (triangles, planes, num_triangles, vertices, triidx);
  delete[] planes;
}

void csBSPTree::Back2Front (const csVector3& pos, csDirtyAccessArray<int>& arr,
  	csSet<int>& used_indices)
{
  float cl = split_plane.Classify (pos);

  if (cl < 0)
  {
    if (child2) child2->Back2Front (pos, arr, used_indices);
  }
  else
  {
    if (child1) child1->Back2Front (pos, arr, used_indices);
  }

  size_t i;
  for (i = 0 ; i < splitters.Length () ; i++)
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
  b2f_array.Empty ();
  csSet<int> used_indices;
  Back2Front (pos, b2f_array, used_indices);
  return b2f_array;
}

