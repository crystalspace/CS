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
#include "cssysdef.h"
#include "csengine/engine.h"
#include "csengine/polyplan.h"
#include "csgeom/textrans.h"
#include "csgeom/transfrm.h"

//---------------------------------------------------------------------------
csPolyPlane::csPolyPlane ()
{
  ref_count = 1;
}

csPolyPlane::~csPolyPlane ()
{
  if (ref_count) 
    csEngine::current_engine->Report ("csPolyPlane: ref_count=%d\n", ref_count);
}

void csPolyPlane::WorldToCamera (
  const csReversibleTransform &t,
  const csVector3 &vertex1)
{
  t.Other2This (plane_wor, vertex1, plane_cam);
}

void csPolyPlane::ObjectToWorld (
  const csReversibleTransform &obj,
  const csVector3 &vertex1)
{
  obj.This2Other (plane_obj, vertex1, plane_wor);

  // This is not efficient and only needed in those cases where the
  // thing is really scaled. We have to see if this is a problem. Normally
  // it is a good thing to avoid calling csThing::Transform() to often.
  // So normally it should not be a problem.
  plane_wor.Normalize ();
}

void csPolyPlane::GetCameraNormal (
  float *p_A,
  float *p_B,
  float *p_C,
  float *p_D) const
{
  *p_A = plane_cam.A ();
  *p_B = plane_cam.B ();
  *p_C = plane_cam.C ();
  *p_D = plane_cam.D ();
}

void csPolyPlane::GetWorldNormal (
  float *p_A,
  float *p_B,
  float *p_C,
  float *p_D) const
{
  *p_A = plane_wor.A ();
  *p_B = plane_wor.B ();
  *p_C = plane_wor.C ();
  *p_D = plane_wor.D ();
}

void csPolyPlane::GetObjectNormal (
  float *p_A,
  float *p_B,
  float *p_C,
  float *p_D) const
{
  *p_A = plane_obj.A ();
  *p_B = plane_obj.B ();
  *p_C = plane_obj.C ();
  *p_D = plane_obj.D ();
}

void csPolyPlane::ClosestPoint (csVector3 &v, csVector3 &isect) const
{
  float r = plane_wor.Classify (v);
  isect.x = r * (-plane_wor.A () - v.x) + v.x;
  isect.y = r * (-plane_wor.B () - v.y) + v.y;
  isect.z = r * (-plane_wor.C () - v.z) + v.z;
}

bool csPolyPlane::IntersectSegment (
  const csVector3 &start,
  const csVector3 &end,
  csVector3 &isect,
  float *pr) const
{
  float x1 = start.x;
  float y1 = start.y;
  float z1 = start.z;
  float x2 = end.x;
  float y2 = end.y;
  float z2 = end.z;
  float r, num, denom;

  // So now we have the plane equation of the polygon:
  // A*x + B*y + C*z + D = 0
  //
  // We also have the parameter line equations of the ray
  // going through 'start' and 'end':
  // x = r*(x2-x1)+x1
  // y = r*(y2-y1)+y1
  // z = r*(z2-z1)+z1
  //
  // =>   A*(r*(x2-x1)+x1) + B*(r*(y2-y1)+y1) + C*(r*(z2-z1)+z1) + D = 0
  // Set *pr to -1 to indicate error if we return false now.
  if (pr) *pr = -1;

  denom = plane_obj.A () *
    (x2 - x1) +
    plane_obj.B () *
    (y2 - y1) +
    plane_obj.C () *
    (z2 - z1);
  if (ABS (denom) < SMALL_EPSILON) return false;  // Lines are parallel
  num = -
    (
      plane_obj.A () *
      x1 +
      plane_obj.B () *
      y1 +
      plane_obj.C () *
      z1 +
      plane_obj.D ()
    );
  r = num / denom;

  // Calculate 'r' and 'isect' even if the intersection point is
  // not on the segment. That way we can use this function for testing
  // with rays as well.
  if (pr) *pr = r;

  isect.x = r * (x2 - x1) + x1;
  isect.y = r * (y2 - y1) + y1;
  isect.z = r * (z2 - z1) + z1;

  // If r is not in [0,1] the intersection point is not on the segment.
  if (r < 0 /*-SMALL_EPSILON*/ || r > 1) return false;

  return true;
}

//---------------------------------------------------------------------------
