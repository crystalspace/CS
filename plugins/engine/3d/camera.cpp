/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
    Camera code written by Ivan Avramovic <ivan@avramovic.com>

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
#include "plugins/engine/3d/camera.h"
#include "plugins/engine/3d/sector.h"
#include "plugins/engine/3d/engine.h"


int csCamera:: default_aspect = 0;
float csCamera:: default_inv_aspect = 0;
float csCamera:: default_fov_angle = 90;
long csCamera:: cur_cameranr = 0;

csCamera::csCamera () :
  csOrthoTransform(), scfImplementationType (this)
{
  mirror = false;
  sector = 0;
  aspect = default_aspect;
  inv_aspect = default_inv_aspect;
  fov_angle = default_fov_angle;
  shift_x = csEngine::frameWidth / 2;
  shift_y = csEngine::frameHeight / 2;
  fp = 0;
  cameranr = cur_cameranr++;
  only_portals = true;
}

csCamera::csCamera (csCamera *c) :
  csOrthoTransform(), scfImplementationType (this)
{
  *this = *c;
  if (fp)
  {
    // Make a copy of the plane.
    fp = new csPlane3 (*fp);
  }

  cameranr = cur_cameranr++;
}

csCamera::csCamera (const csCamera &c) :
  csOrthoTransform(), scfImplementationType (this)
{
  *this = c;
  if (fp)
  {
    // Make a copy of the plane.
    fp = new csPlane3 (*fp);
  }

  cameranr = cur_cameranr++;
}

csCamera::~csCamera ()
{
  delete fp;
}

void csCamera::FireCameraSectorListeners (iSector* sector)
{
  size_t i;
  for (i = 0 ; i < listeners.Length () ; i++)
    listeners[i]->NewSector ((iCamera*)this, sector);
}

void csCamera::SetFarPlane (csPlane3 *farplane)
{
  delete fp;
  if (farplane)
    fp = new csPlane3 (*farplane);
  else
    fp = 0;
}

void csCamera::MoveWorld (const csVector3 &v, bool cd)
{
  csVector3 new_position = GetOrigin () + v;
  if (sector)
  {
    csVector3 remember_position = new_position;
    iSector *new_sector = sector->FollowSegment (
        *this,
        new_position,
        mirror,
        only_portals);
    if (new_sector == sector)
    {
      if (!cd) new_position = remember_position;
    }
    else
    {
      sector = new_sector;
      cameranr = cur_cameranr++;
      FireCameraSectorListeners (sector);
    }
  }

  SetOrigin (new_position);
  cameranr = cur_cameranr++;
}

void csCamera::Correct (int n)
{
  csVector3 w1, w2, w3;
  float *vals[5];
  if (n == 0) return ;

  w3 = m_t2o.Col3 ();
  vals[0] = &w3.x;
  vals[1] = &w3.y;
  vals[2] = &w3.z;
  vals[4] = 0;
  Correct (n, vals);  /* perform the snap-to operation on the forward vector */

  /* Maybe w3 should be normalized.  Only necessary if there is
   significant roundoff error: */

  //  w3 = csVector3::unit(w3);

  /* perhaps a snap-to should be performed on one of the other vectors as well */
  w1 = m_t2o.Col2 ();
  w2 = csVector3::Unit (w3 % w1);
  w1 = w2 % w3;

  SetT2O (csMatrix3 (w1.x, w2.x, w3.x, w1.y, w2.y, w3.y, w1.z, w2.z, w3.z));
}

void csCamera::Correct (int n, float *vals[])
{
  float r, angle;

  if (vals == 0) return ;
  if (vals[0] == 0) return ;
  if (vals[1] == 0) return ;
  if (vals[2] != 0)
  {
    if (*vals[0] < *vals[1])
    {
      r = *vals[2];
      *vals[2] = *vals[0];
      *vals[0] = r;
    }
    else
    {
      r = *vals[2];
      *vals[2] = *vals[1];
      *vals[1] = r;
    }
  }

  angle = (float)atan2 (*vals[1], *vals[0]);
  angle = (TWO_PI / n) * csQround (n * angle / TWO_PI);
  *vals[1] = csQsqrt ((*vals[0]) * (*vals[0]) + (*vals[1]) * (*vals[1]));
  Correct (n, vals + 1);
  r = *vals[1];
  *vals[0] = r * (float)cos (angle);
  *vals[1] = r * (float)sin (angle);
  cameranr = cur_cameranr++;
}

void csCamera::SetFOVAngle (float a, int width)
{
  // make sure we have valid angles
  if (a >= 180)
  {
    a = 180 - SMALL_EPSILON;
  }
  else if (a <= 0)
  {
    a = SMALL_EPSILON;
  }

  // This is our reference point.
  // It must be somewhere on the function graph.
  // This reference point was composed by testing.
  // If anyone knows a 100 percent correct reference point, please put it here.
  // But for now it's about 99% correct
  float vRefFOVAngle = 53;
  float vRefFOV = width;

  // We calculate the new aspect relative to a reference point
  aspect = (int) ((tan((vRefFOVAngle * 0.5) / 180 * PI) * vRefFOV) /
           tan((a * 0.5)  / 180 * PI));

  // set the other neccessary variables
  inv_aspect = 1.0f / aspect;
  fov_angle = a;
  cameranr = cur_cameranr++;
}

void csCamera::ComputeAngle (int width)
{
  float rview_fov = (float)GetFOV () * 0.5f;
  float disp_width = (float)width * 0.5f;
  float inv_disp_radius = csQisqrt (
      rview_fov * rview_fov + disp_width * disp_width);
  fov_angle = 2.0f * (float)acos (disp_width * inv_disp_radius)
  	* (360.0f / TWO_PI);
}

void csCamera::ComputeDefaultAngle (int width)
{
  float rview_fov = (float)GetDefaultFOV () * 0.5f;
  float disp_width = (float)width * 0.5f;
  float inv_disp_radius = csQisqrt (
      rview_fov * rview_fov + disp_width * disp_width);
  default_fov_angle = 2.0f * (float)acos (disp_width * inv_disp_radius)
  	* (360.0f / TWO_PI);
}
