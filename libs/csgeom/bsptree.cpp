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
#include "csutil/sysfunc.h"
#include "qint.h"
#include "qsqrt.h"
#include "csgeom/math3d.h"
#include "csgeom/tri.h"
#include "csgeom/bsptree.h"

//---------------------------------------------------------------------------

csBlockAllocator<csBSPTree> csBSPTree::tree_nodes (1000);

csBSPTree::csBSPTree ()
{
  child1 = 0;
  child2 = 0;
  split_poly = -1;
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

int csBSPTree::FindBestSplitter (csTriangle* triangles, csPlane3* planes,
	int num_triangles, csVector3* vertices)
{
  int i, j;
  for (i = 0 ; i < num_triangles ; i++)
  {
    int cnt_splits = 0;
    int cnt_left = 0;
    int cnt_right = 0;
    csPlane3& pl = planes[i];
    for (j = 0 ; j < num_triangles ; j++)
      if (i != j)
      {
        csTriangle& trj = triangles[j];
        int cla, clb, clc;
        cla = SIGN (pl.Classify (vertices[trj.a]));
        clb = SIGN (pl.Classify (vertices[trj.b]));
        clc = SIGN (pl.Classify (vertices[trj.c]));
      }
  }
  return -1;
}

void csBSPTree::Build (csTriangle* triangles, int num_triangles,
	csVector3* vertices)
{
  csPlane3* planes = new csPlane3[num_triangles];
  int i;
  for (i = 0 ; i < num_triangles ; i++)
  {
    csTriangle& t = triangles[i];
    planes[i].Set (vertices[t.a], vertices[t.b], vertices[t.c]);
  }

}

void csBSPTree::Front2Back (const csVector3& pos, csBSPTreeVisitFunc* func)
{
}

