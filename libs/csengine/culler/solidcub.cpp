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
#include "csgeom/polyclip.h"
#include "csengine/solidcub.h"
#include "csengine/engine.h"

bool csSolidBspPersp::DoPerspective (csVector3* verts, int num_verts,
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
    if (verts[i].z < EPSILON) num_z_0++;
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
      persp.AddPerspectiveUnit (verts[i]);
    }
  }
  else
  {
    // Some vertices are behind. We need to clip.
    csVector3 isect;
    int i1 = num_verts-1;
    for (i = 0 ; i < num_verts ; i++)
    {
      if (verts[i].z < EPSILON)
      {
	if (verts[i1].z < EPSILON)
	{
	  // Just skip vertex.
	}
	else
	{
	  // We need to intersect and add intersection point.
	  csIntersect3::ZPlane (EPSILON, verts[i], verts[i1], isect);
	  persp.AddPerspectiveUnit (isect);
	}
      }
      else
      {
	if (verts[i1].z < EPSILON)
	{
	  // We need to intersect and add both intersection point and this
	  // point.
	  csIntersect3::ZPlane (EPSILON, verts[i], verts[i1], isect);
	  persp.AddPerspectiveUnit (isect);
	}
	// Just perspective correct and add to the 2D polygon.
	persp.AddPerspectiveUnit (verts[i]);
      }
      i1 = i;
    }
  }
  return true;
}

bool csSolidBspPersp::InsertPolygon (csVector3* verts, int num_verts)
{
  static csPolygon2D persp;
  if (!DoPerspective (verts, num_verts, persp)) return false;
  return csSolidBsp::InsertPolygon (persp.GetVertices (),
  	persp.GetNumVertices ());
}

bool csSolidBspPersp::TestPolygon (csVector3* verts, int num_verts)
{
  static csPolygon2D persp;
  if (!DoPerspective (verts, num_verts, persp)) return false;
  return csSolidBsp::TestPolygon (persp.GetVertices (),
  	persp.GetNumVertices ());
}

//-----------------------------------------------------------------

csSolidBspCube::csSolidBspCube ()
{
  trees[0] = new csSolidBspPersp ();
  trees[1] = new csSolidBspPersp ();
  trees[2] = new csSolidBspPersp ();
  trees[3] = new csSolidBspPersp ();
  trees[4] = new csSolidBspPersp ();
  trees[5] = new csSolidBspPersp ();
}

csSolidBspCube::~csSolidBspCube ()
{
  delete trees[0];
  delete trees[1];
  delete trees[2];
  delete trees[3];
  delete trees[4];
  delete trees[5];
}

void csSolidBspCube::MakeEmpty ()
{
  trees[0]->MakeEmpty ();
  trees[1]->MakeEmpty ();
  trees[2]->MakeEmpty ();
  trees[3]->MakeEmpty ();
  trees[4]->MakeEmpty ();
  trees[5]->MakeEmpty ();
}

bool csSolidBspCube::IsFull ()
{
  if (trees[0]->IsFull ()) return true;
  if (trees[1]->IsFull ()) return true;
  if (trees[2]->IsFull ()) return true;
  if (trees[3]->IsFull ()) return true;
  if (trees[4]->IsFull ()) return true;
  if (trees[5]->IsFull ()) return true;
  return false;
}

bool csSolidBspCube::InsertPolygon (csVector3* verts, int num_verts)
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

bool csSolidBspCube::TestPolygon (csVector3* verts, int num_verts)
{
  csVector3 cam[40];	// @@@ HARDCODED! BAD!
  int i;

  // -> Z
  if (trees[0]->TestPolygon (verts, num_verts)) return true;

  // -> -Z
  for (i = 0 ; i < num_verts ; i++)
  {
    cam[i].x = -verts[i].x;
    cam[i].y = verts[i].y;
    cam[i].z = -verts[i].z;
  }
  if (trees[1]->TestPolygon (cam, num_verts)) return true;

  // -> X
  for (i = 0 ; i < num_verts ; i++)
  {
    cam[i].x = verts[i].z;
    cam[i].y = verts[i].y;
    cam[i].z = -verts[i].x;
  }
  if (trees[2]->TestPolygon (cam, num_verts)) return true;

  // -> -X
  for (i = 0 ; i < num_verts ; i++)
  {
    cam[i].x = -verts[i].z;
    cam[i].y = verts[i].y;
    cam[i].z = verts[i].x;
  }
  if (trees[3]->TestPolygon (cam, num_verts)) return true;

  // -> Y
  for (i = 0 ; i < num_verts ; i++)
  {
    cam[i].x = verts[i].x;
    cam[i].y = verts[i].z;
    cam[i].z = -verts[i].y;
  }
  if (trees[4]->TestPolygon (cam, num_verts)) return true;

  // -> -Y
  for (i = 0 ; i < num_verts ; i++)
  {
    cam[i].x = verts[i].x;
    cam[i].y = -verts[i].z;
    cam[i].z = verts[i].y;
  }
  if (trees[5]->TestPolygon (cam, num_verts)) return true;

  return false;
}
