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
#include "csgeom/polyclip.h"
#include "csengine/quadcube.h"
#include "csengine/world.h"

bool csQuadtreePersp::DoPerspective (csVector3* verts, int num_verts,
	csPolygon2D& persp)
{
  int num_z_0;
  int i;

  // If all vertices are behind z plane then the node is
  // not visible. If some vertices are behind z plane then we
  // need to clip the polygon.

  num_z_0 = 0;
  for (i = 0 ; i < num_verts ; i++)
  {
    if (verts[i].z < SMALL_EPSILON) num_z_0++;
  }
  if (num_z_0 == num_verts)
  {
    // Node behind camera.
    return false;
  }
  persp.MakeEmpty ();
  if (num_z_0 == 0)
  {
    // No vertices are behind. Just perspective correct.
    for (i = 0 ; i < num_verts ; i++)
    {
      persp.AddPerspective (verts[i]);
    }
  }
  else
  {
    // Some vertices are behind. We need to clip.
    csVector3 isect;
    int i1 = num_verts-1;
    for (i = 0 ; i < num_verts ; i++)
    {
      if (verts[i].z < SMALL_EPSILON)
      {
	if (verts[i1].z < SMALL_EPSILON)
	{
	  // Just skip vertex.
	}
	else
	{
	  // We need to intersect and add intersection point.
	  csIntersect3::ZPlane (SMALL_EPSILON, verts[i], verts[i1], isect);
	  persp.AddPerspective (isect);
	}
      }
      else
      {
	if (verts[i1].z < SMALL_EPSILON)
	{
	  // We need to intersect and add both intersection point and this point.
	  csIntersect3::ZPlane (SMALL_EPSILON, verts[i], verts[i1], isect);
	  persp.AddPerspective (isect);
	}
	// Just perspective correct and add to the 2D polygon.
	persp.AddPerspective (verts[i]);
      }
      i1 = i;
    }
  }
  return true;
}

bool csQuadtreePersp::InsertPolygon (csVector3* verts, int num_verts,
	csClipper* clipper)
{
  static csPolygon2D persp;
  if (!DoPerspective (verts, num_verts, persp)) return false;
  if (clipper && !persp.ClipAgainst (clipper)) return false;
  return csQuadtree::InsertPolygon (persp.GetVertices (),
  	persp.GetNumVertices ());
}

bool csQuadtreePersp::TestPolygon (csVector3* verts, int num_verts,
	csClipper* clipper)
{
  static csPolygon2D persp;
  if (!DoPerspective (verts, num_verts, persp)) return false;
  if (clipper && !persp.ClipAgainst (clipper)) return false;
  return csQuadtree::TestPolygon (persp.GetVertices (),
  	persp.GetNumVertices ());
}

csQuadcube::csQuadcube (float z, int depth)
{
  float x = (csWorld::frame_width/2) * csCamera::default_inv_aspect * z;
  float y = (csWorld::frame_height/2) * csCamera::default_inv_aspect * z;
  z_dist = z;
  csVector2 corners[4];
  corners[0] = csVector2 (-x,  y);
  corners[1] = csVector2 ( x,  y);
  corners[2] = csVector2 ( x, -y);
  corners[3] = csVector2 (-x, -y);

  CHK (trees[0] = new csQuadtreePersp (corners, depth));
  CHK (trees[1] = new csQuadtreePersp (corners, depth));
  CHK (trees[2] = new csQuadtreePersp (corners, depth));
  CHK (trees[3] = new csQuadtreePersp (corners, depth));
  CHK (trees[4] = new csQuadtreePersp (corners, depth));
  CHK (trees[5] = new csQuadtreePersp (corners, depth));
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
  csVector3 cam[40];	// @@@ HARDCODED! BAD!
  int i;

  // -> Z
  bool rc1 = trees[0]->InsertPolygon (verts, num_verts);

  // -> -Z
  for (i = 0 ; i < num_verts ; i++)
  {
    cam[i].x = -verts[i].x;
    cam[i].y = verts[i].y;
    cam[i].z = -verts[i].z;
  }
  bool rc2 = trees[1]->InsertPolygon (cam, num_verts);

  // -> X
  for (i = 0 ; i < num_verts ; i++)
  {
    cam[i].x = verts[i].z;
    cam[i].y = verts[i].y;
    cam[i].z = -verts[i].x;
  }
  bool rc3 = trees[2]->InsertPolygon (cam, num_verts);

  // -> -X
  for (i = 0 ; i < num_verts ; i++)
  {
    cam[i].x = -verts[i].z;
    cam[i].y = verts[i].y;
    cam[i].z = verts[i].x;
  }
  bool rc4 = trees[3]->InsertPolygon (cam, num_verts);

  // -> Y
  for (i = 0 ; i < num_verts ; i++)
  {
    cam[i].x = verts[i].x;
    cam[i].y = verts[i].z;
    cam[i].z = -verts[i].y;
  }
  bool rc5 = trees[4]->InsertPolygon (cam, num_verts);

  // -> -Y
  for (i = 0 ; i < num_verts ; i++)
  {
    cam[i].x = verts[i].x;
    cam[i].y = -verts[i].z;
    cam[i].z = verts[i].y;
  }
  bool rc6 = trees[5]->InsertPolygon (cam, num_verts);
  return rc1 || rc2 || rc3 || rc4 || rc5 || rc6;
}

bool csQuadcube::TestPolygon (csVector3* verts, int num_verts)
{
  csVector3 cam[40];	// @@@ HARDCODED! BAD!
  int i;

  // -> Z
  if (trees[0]->InsertPolygon (verts, num_verts)) return true;

  // -> -Z
  for (i = 0 ; i < num_verts ; i++)
  {
    cam[i].x = -verts[i].x;
    cam[i].y = verts[i].y;
    cam[i].z = -verts[i].z;
  }
  if (trees[1]->InsertPolygon (cam, num_verts)) return true;

  // -> X
  for (i = 0 ; i < num_verts ; i++)
  {
    cam[i].x = verts[i].z;
    cam[i].y = verts[i].y;
    cam[i].z = -verts[i].x;
  }
  if (trees[2]->InsertPolygon (cam, num_verts)) return true;

  // -> -X
  for (i = 0 ; i < num_verts ; i++)
  {
    cam[i].x = -verts[i].z;
    cam[i].y = verts[i].y;
    cam[i].z = verts[i].x;
  }
  if (trees[3]->InsertPolygon (cam, num_verts)) return true;

  // -> Y
  for (i = 0 ; i < num_verts ; i++)
  {
    cam[i].x = verts[i].x;
    cam[i].y = verts[i].z;
    cam[i].z = -verts[i].y;
  }
  if (trees[4]->InsertPolygon (cam, num_verts)) return true;

  // -> -Y
  for (i = 0 ; i < num_verts ; i++)
  {
    cam[i].x = verts[i].x;
    cam[i].y = -verts[i].z;
    cam[i].z = verts[i].y;
  }
  if (trees[5]->InsertPolygon (cam, num_verts)) return true;

  return false;
}


