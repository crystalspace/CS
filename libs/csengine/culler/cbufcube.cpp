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
#include "csengine/cbufcube.h"
#include "csengine/engine.h"

bool csCBufferPersp::DoPerspective (csVector3* verts, int num_verts,
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
      //@@@ HARDCODED 1024: is related to tree size.
      persp.AddPerspectiveAspect (verts[i], 512, 512);
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
	  persp.AddPerspectiveAspect (isect, 512, 512);
	}
      }
      else
      {
	if (verts[i1].z < EPSILON)
	{
	  // We need to intersect and add both intersection point and this
	  // point.
	  csIntersect3::ZPlane (EPSILON, verts[i], verts[i1], isect);
	  persp.AddPerspectiveAspect (isect, 512, 512);
	}
	// Just perspective correct and add to the 2D polygon.
	persp.AddPerspectiveAspect (verts[i], 512, 512);
      }
      i1 = i;
    }
  }
  return true;
}

bool csCBufferPersp::InsertPolygon (csVector3* verts, int num_verts,
	csClipper* clipper)
{
  static csPolygon2D persp;
  if (!DoPerspective (verts, num_verts, persp)) return false;
  if (clipper && !persp.ClipAgainst (clipper)) return false;
  return csCBuffer::InsertPolygon (persp.GetVertices (),
  	persp.GetVertexCount ());
}

bool csCBufferPersp::TestPolygon (csVector3* verts, int num_verts,
	csClipper* clipper)
{
  static csPolygon2D persp;
  if (!DoPerspective (verts, num_verts, persp)) return false;
  if (clipper && !persp.ClipAgainst (clipper)) return false;
  return csCBuffer::TestPolygon (persp.GetVertices (),
  	persp.GetVertexCount ());
}

//-----------------------------------------------------------------

csCBufferCube::csCBufferCube (int dim)
{
  trees[0] = new csCBufferPersp (0, dim-1, dim);
  trees[1] = new csCBufferPersp (0, dim-1, dim);
  trees[2] = new csCBufferPersp (0, dim-1, dim);
  trees[3] = new csCBufferPersp (0, dim-1, dim);
  trees[4] = new csCBufferPersp (0, dim-1, dim);
  trees[5] = new csCBufferPersp (0, dim-1, dim);
  csBox2 b (0, 0, dim, dim);
  clipper = new csBoxClipper (b);
  MakeEmpty ();
}

csCBufferCube::~csCBufferCube ()
{
  delete trees[0];
  delete trees[1];
  delete trees[2];
  delete trees[3];
  delete trees[4];
  delete trees[5];
  delete clipper;
}

void csCBufferCube::MakeEmpty ()
{
  trees[0]->Initialize ();
  trees[1]->Initialize ();
  trees[2]->Initialize ();
  trees[3]->Initialize ();
  trees[4]->Initialize ();
  trees[5]->Initialize ();
}

bool csCBufferCube::IsFull ()
{
  if (!trees[0]->IsFull ()) return false;
  if (!trees[1]->IsFull ()) return false;
  if (!trees[2]->IsFull ()) return false;
  if (!trees[3]->IsFull ()) return false;
  if (!trees[4]->IsFull ()) return false;
  if (!trees[5]->IsFull ()) return false;
  return true;
}

bool csCBufferCube::InsertPolygon (csVector3* verts, int num_verts)
{
  csVector3 cam[60];	// @@@ HARDCODED! BAD!
  int i;
  bool rc1, rc2, rc3, rc4, rc5, rc6;

  // -> Z
  if (!trees[0]->IsFull ())
    rc1 = trees[0]->InsertPolygon (verts, num_verts, clipper);
  else
    rc1 = false;

  // -> -Z
  if (!trees[1]->IsFull ())
  {
    for (i = 0 ; i < num_verts ; i++)
    {
      cam[i].x = -verts[i].x;
      cam[i].y = verts[i].y;
      cam[i].z = -verts[i].z;
    }
    rc2 = trees[1]->InsertPolygon (cam, num_verts, clipper);
  }
  else
    rc2 = false;

  // -> X
  if (!trees[2]->IsFull ())
  {
    for (i = 0 ; i < num_verts ; i++)
    {
      cam[i].x = verts[i].z;
      cam[i].y = verts[i].y;
      cam[i].z = -verts[i].x;
    }
    rc3 = trees[2]->InsertPolygon (cam, num_verts, clipper);
  }
  else
    rc3 = false;

  // -> -X
  if (!trees[3]->IsFull ())
  {
    for (i = 0 ; i < num_verts ; i++)
    {
      cam[i].x = -verts[i].z;
      cam[i].y = verts[i].y;
      cam[i].z = verts[i].x;
    }
    rc4 = trees[3]->InsertPolygon (cam, num_verts, clipper);
  }
  else
    rc4 = false;

  // -> Y
  if (!trees[4]->IsFull ())
  {
    for (i = 0 ; i < num_verts ; i++)
    {
      cam[i].x = verts[i].x;
      cam[i].y = verts[i].z;
      cam[i].z = -verts[i].y;
    }
    rc5 = trees[4]->InsertPolygon (cam, num_verts, clipper);
  }
  else
    rc5 = false;

  // -> -Y
  if (!trees[5]->IsFull ())
  {
    for (i = 0 ; i < num_verts ; i++)
    {
      cam[i].x = verts[i].x;
      cam[i].y = -verts[i].z;
      cam[i].z = verts[i].y;
    }
    rc6 = trees[5]->InsertPolygon (cam, num_verts, clipper);
  }
  else
    rc6 = false;

  return rc1 || rc2 || rc3 || rc4 || rc5 || rc6;
}

bool csCBufferCube::TestPolygon (csVector3* verts, int num_verts)
{
  csVector3 cam[60];	// @@@ HARDCODED! BAD!
  int i;

  // -> Z
  if (!trees[0]->IsFull ())
    if (trees[0]->TestPolygon (verts, num_verts, clipper)) return true;

  // -> -Z
  if (!trees[1]->IsFull ())
  {
    for (i = 0 ; i < num_verts ; i++)
    {
      cam[i].x = -verts[i].x;
      cam[i].y = verts[i].y;
      cam[i].z = -verts[i].z;
    }
    if (trees[1]->TestPolygon (cam, num_verts, clipper)) return true;
  }

  // -> X
  if (!trees[2]->IsFull ())
  {
    for (i = 0 ; i < num_verts ; i++)
    {
      cam[i].x = verts[i].z;
      cam[i].y = verts[i].y;
      cam[i].z = -verts[i].x;
    }
    if (trees[2]->TestPolygon (cam, num_verts, clipper)) return true;
  }

  // -> -X
  if (!trees[3]->IsFull ())
  {
    for (i = 0 ; i < num_verts ; i++)
    {
      cam[i].x = -verts[i].z;
      cam[i].y = verts[i].y;
      cam[i].z = verts[i].x;
    }
    if (trees[3]->TestPolygon (cam, num_verts, clipper)) return true;
  }

  // -> Y
  if (!trees[4]->IsFull ())
  {
    for (i = 0 ; i < num_verts ; i++)
    {
      cam[i].x = verts[i].x;
      cam[i].y = verts[i].z;
      cam[i].z = -verts[i].y;
    }
    if (trees[4]->TestPolygon (cam, num_verts, clipper)) return true;
  }

  // -> -Y
  if (!trees[5]->IsFull ())
  {
    for (i = 0 ; i < num_verts ; i++)
    {
      cam[i].x = verts[i].x;
      cam[i].y = -verts[i].z;
      cam[i].z = verts[i].y;
    }
    if (trees[5]->TestPolygon (cam, num_verts, clipper)) return true;
  }

  return false;
}
