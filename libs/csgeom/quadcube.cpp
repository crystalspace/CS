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
  CHK (quad[0] = new csQuadtree (corners, depth));
  corners[0] = csVector3 (max_bbox.x, max_bbox.y, min_bbox.z);
  corners[1] = csVector3 (min_bbox.x, max_bbox.y, min_bbox.z);
  corners[2] = csVector3 (min_bbox.x, min_bbox.y, min_bbox.z);
  corners[3] = csVector3 (max_bbox.x, min_bbox.y, min_bbox.z);
  CHK (quad[1] = new csQuadtree (corners, depth));
  corners[0] = csVector3 (max_bbox.x, max_bbox.y, max_bbox.z);
  corners[1] = csVector3 (max_bbox.x, max_bbox.y, min_bbox.z);
  corners[2] = csVector3 (max_bbox.x, min_bbox.y, min_bbox.z);
  corners[3] = csVector3 (max_bbox.x, min_bbox.y, max_bbox.z);
  CHK (quad[2] = new csQuadtree (corners, depth));
  corners[0] = csVector3 (min_bbox.x, max_bbox.y, min_bbox.z);
  corners[1] = csVector3 (min_bbox.x, max_bbox.y, max_bbox.z);
  corners[2] = csVector3 (min_bbox.x, min_bbox.y, max_bbox.z);
  corners[3] = csVector3 (min_bbox.x, min_bbox.y, min_bbox.z);
  CHK (quad[3] = new csQuadtree (corners, depth));
  corners[0] = csVector3 (min_bbox.x, max_bbox.y, min_bbox.z);
  corners[1] = csVector3 (max_bbox.x, max_bbox.y, min_bbox.z);
  corners[2] = csVector3 (max_bbox.x, max_bbox.y, max_bbox.z);
  corners[3] = csVector3 (min_bbox.x, max_bbox.y, max_bbox.z);
  CHK (quad[4] = new csQuadtree (corners, depth));
  corners[0] = csVector3 (max_bbox.x, min_bbox.y, min_bbox.z);
  corners[1] = csVector3 (min_bbox.x, min_bbox.y, min_bbox.z);
  corners[2] = csVector3 (min_bbox.x, min_bbox.y, max_bbox.z);
  corners[3] = csVector3 (max_bbox.x, min_bbox.y, max_bbox.z);
  CHK (quad[5] = new csQuadtree (corners, depth));
}

csQuadcube::~csQuadcube ()
{
  CHK (delete quad[0]);
  CHK (delete quad[1]);
  CHK (delete quad[2]);
  CHK (delete quad[3]);
  CHK (delete quad[4]);
  CHK (delete quad[5]);
}

void csQuadcube::MakeEmpty ()
{
  quad[0]->MakeEmpty ();
  quad[1]->MakeEmpty ();
  quad[2]->MakeEmpty ();
  quad[3]->MakeEmpty ();
  quad[4]->MakeEmpty ();
  quad[5]->MakeEmpty ();
}

bool csQuadcube::IsFull ()
{
  if (quad[0]->IsFull ()) return true;
  if (quad[1]->IsFull ()) return true;
  if (quad[2]->IsFull ()) return true;
  if (quad[3]->IsFull ()) return true;
  if (quad[4]->IsFull ()) return true;
  if (quad[5]->IsFull ()) return true;
  return false;
}

bool csQuadcube::InsertPolygon (csVector3* verts, int num_verts)
{
  bool rc1 = quad[0]->InsertPolygon (verts, num_verts);
  bool rc2 = quad[1]->InsertPolygon (verts, num_verts);
  bool rc3 = quad[2]->InsertPolygon (verts, num_verts);
  bool rc4 = quad[3]->InsertPolygon (verts, num_verts);
  bool rc5 = quad[4]->InsertPolygon (verts, num_verts);
  bool rc6 = quad[5]->InsertPolygon (verts, num_verts);
  return rc1 || rc2 || rc3 || rc4 || rc5 || rc6;
}

bool csQuadcube::TestPolygon (csVector3* verts, int num_verts)
{
  if (quad[0]->TestPolygon (verts, num_verts)) return true;
  if (quad[1]->TestPolygon (verts, num_verts)) return true;
  if (quad[2]->TestPolygon (verts, num_verts)) return true;
  if (quad[3]->TestPolygon (verts, num_verts)) return true;
  if (quad[4]->TestPolygon (verts, num_verts)) return true;
  if (quad[5]->TestPolygon (verts, num_verts)) return true;
  return false;
}


