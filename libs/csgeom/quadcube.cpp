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
#include "csgeom/quadcube.h"

csQuadcube::csQuadcube (const csVector3& min_bbox, const csVector3& max_bbox,
	int depth)
{
  csVector3 corners[4];
  corners[0] = csVector3 (min_bbox.x, max_bbox.y, max_bbox.z);
  corners[1] = csVector3 (max_bbox.x, max_bbox.y, max_bbox.z);
  corners[2] = csVector3 (max_bbox.x, min_bbox.y, max_bbox.z);
  corners[3] = csVector3 (min_bbox.x, min_bbox.y, max_bbox.z);
  CHK (trees[0] = new csQuadtree (corners, depth));
  corners[0] = csVector3 (max_bbox.x, max_bbox.y, min_bbox.z);
  corners[1] = csVector3 (min_bbox.x, max_bbox.y, min_bbox.z);
  corners[2] = csVector3 (min_bbox.x, min_bbox.y, min_bbox.z);
  corners[3] = csVector3 (max_bbox.x, min_bbox.y, min_bbox.z);
  CHK (trees[1] = new csQuadtree (corners, depth));
  corners[0] = csVector3 (max_bbox.x, max_bbox.y, max_bbox.z);
  corners[1] = csVector3 (max_bbox.x, max_bbox.y, min_bbox.z);
  corners[2] = csVector3 (max_bbox.x, min_bbox.y, min_bbox.z);
  corners[3] = csVector3 (max_bbox.x, min_bbox.y, max_bbox.z);
  CHK (trees[2] = new csQuadtree (corners, depth));
  corners[0] = csVector3 (min_bbox.x, max_bbox.y, min_bbox.z);
  corners[1] = csVector3 (min_bbox.x, max_bbox.y, max_bbox.z);
  corners[2] = csVector3 (min_bbox.x, min_bbox.y, max_bbox.z);
  corners[3] = csVector3 (min_bbox.x, min_bbox.y, min_bbox.z);
  CHK (trees[3] = new csQuadtree (corners, depth));
  corners[0] = csVector3 (min_bbox.x, max_bbox.y, min_bbox.z);
  corners[1] = csVector3 (max_bbox.x, max_bbox.y, min_bbox.z);
  corners[2] = csVector3 (max_bbox.x, max_bbox.y, max_bbox.z);
  corners[3] = csVector3 (min_bbox.x, max_bbox.y, max_bbox.z);
  CHK (trees[4] = new csQuadtree (corners, depth));
  corners[0] = csVector3 (max_bbox.x, min_bbox.y, min_bbox.z);
  corners[1] = csVector3 (min_bbox.x, min_bbox.y, min_bbox.z);
  corners[2] = csVector3 (min_bbox.x, min_bbox.y, max_bbox.z);
  corners[3] = csVector3 (max_bbox.x, min_bbox.y, max_bbox.z);
  CHK (trees[5] = new csQuadtree (corners, depth));
}

csQuadcube::~csQuadcube ()
{
  CHK (delete trees[0]);
  CHK (delete trees[1]);
  CHK (delete trees[2]);
  CHK (delete trees[3]);
  CHK (delete trees[4]);
  CHK (delete trees[5]);
}

void csQuadcube::MakeEmpty ()
{
  trees[0]->MakeEmpty ();
  trees[1]->MakeEmpty ();
  trees[2]->MakeEmpty ();
  trees[3]->MakeEmpty ();
  trees[4]->MakeEmpty ();
  trees[5]->MakeEmpty ();
}

bool csQuadcube::IsFull ()
{
  if (trees[0]->IsFull ()) return true;
  if (trees[1]->IsFull ()) return true;
  if (trees[2]->IsFull ()) return true;
  if (trees[3]->IsFull ()) return true;
  if (trees[4]->IsFull ()) return true;
  if (trees[5]->IsFull ()) return true;
  return false;
}

bool csQuadcube::InsertPolygon (csVector3* verts, int num_verts)
{
  bool rc1 = trees[0]->InsertPolygon (verts, num_verts);
  bool rc2 = trees[1]->InsertPolygon (verts, num_verts);
  bool rc3 = trees[2]->InsertPolygon (verts, num_verts);
  bool rc4 = trees[3]->InsertPolygon (verts, num_verts);
  bool rc5 = trees[4]->InsertPolygon (verts, num_verts);
  bool rc6 = trees[5]->InsertPolygon (verts, num_verts);
  return rc1 || rc2 || rc3 || rc4 || rc5 || rc6;
}

bool csQuadcube::TestPolygon (csVector3* verts, int num_verts)
{
  if (trees[0]->TestPolygon (verts, num_verts)) return true;
  if (trees[1]->TestPolygon (verts, num_verts)) return true;
  if (trees[2]->TestPolygon (verts, num_verts)) return true;
  if (trees[3]->TestPolygon (verts, num_verts)) return true;
  if (trees[4]->TestPolygon (verts, num_verts)) return true;
  if (trees[5]->TestPolygon (verts, num_verts)) return true;
  return false;
}


