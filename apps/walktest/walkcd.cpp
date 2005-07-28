/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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
#include "csqint.h"
#include "csqsqrt.h"

#include "csgeom/csrect.h"
#include "csgeom/frustum.h"
#include "csgeom/polymesh.h"
#include "cstool/collider.h"
#include "cstool/cspixmap.h"
#include "cstool/keyval.h"
#include "iengine/camera.h"
#include "iengine/movable.h"
#include "iengine/sector.h"
#include "igeom/objmodel.h"
#include "igeom/polymesh.h"
#include "imesh/object.h"
#include "imesh/thing.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "ivaria/collider.h"
#include "ivaria/view.h"

#include "walktest.h"
#include "infmaze.h"

extern WalkTest *Sys;

int FindIntersection(csCollisionPair& cd,csSegment3& line)
{
  csVector3 tri1[3]; tri1[0]=cd.a1; tri1[1]=cd.b1; tri1[2]=cd.c1;
  csVector3 tri2[3]; tri2[0]=cd.a2; tri2[1]=cd.b2; tri2[2]=cd.c2;

  bool coplanar;
  return csIntersect3::TriangleTriangle(tri1,tri2,line,coplanar);
}

// Define the player bounding box.
// The camera's lens or person's eye is assumed to be
// at 0,0,0.  The height (DY), width (DX) and depth (DZ).
// Is the size of the camera/person and the origin
// coordinates (OX,OY,OZ) locate the bbox with respect to the eye.
// This player is 1.8 metres tall (assuming 1cs unit = 1m) (6feet)
#define DX    cfg_body_width
#define DY    cfg_body_height
#define DZ    cfg_body_depth
#define OY    Sys->cfg_eye_offset

#define DX_L  cfg_legs_width
#define DZ_L  cfg_legs_depth

#define DX_2  (DX/2)
#define DZ_2  (DZ/2)

#define DX_2L (DX_L/2)
#define DZ_2L (DZ_L/2)

#define OYL  Sys->cfg_legs_offset
#define DYL  (OY-OYL)

void WalkTest::CreateColliders ()
{
  collider_actor.SetCollideSystem (collide_system);
  collider_actor.SetEngine (Engine);
  csVector3 legs (DX_2L * 2, DYL, DZ_2L * 2);
  csVector3 body (DX_2 * 2, DY, DZ_2 * 2);
  csVector3 shift (0, OYL, 0);
  collider_actor.InitializeColliders (view->GetCamera (),
  	legs, body, shift);
}

