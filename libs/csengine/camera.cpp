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
#include "qint.h"
#include "qsqrt.h"
#include "csengine/camera.h"
#include "csengine/sector.h"
#include "csengine/engine.h"

SCF_IMPLEMENT_IBASE (csCamera)
  SCF_IMPLEMENTS_INTERFACE (iBase)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iCamera)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csCamera::Camera)
  SCF_IMPLEMENTS_INTERFACE (iCamera)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

int csCamera::default_aspect = 0;
float csCamera::default_inv_aspect = 0;
float csCamera::default_fov_angle = 90;
long csCamera::cur_cameranr = 0;

csCamera::csCamera () : csOrthoTransform()
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiCamera);
  mirror = false;
  sector = NULL;
  aspect = default_aspect;
  inv_aspect = default_inv_aspect;
  fov_angle = default_fov_angle;
  shift_x = csEngine::frame_width / 2;
  shift_y = csEngine::frame_height / 2;
  fp = NULL;
  cameranr = cur_cameranr++;
  only_portals = true;
}

csCamera::csCamera (csCamera* c) : csOrthoTransform ()
{
  *this = *c;
  if (fp)
  {
    // Make a copy of the plane.
    fp = new csPlane3 (*fp);
  }
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiCamera);
  cameranr = cur_cameranr++;
}

csCamera::csCamera (const csCamera& c) : csOrthoTransform (), iBase ()
{
  *this = c;
  if (fp)
  {
    // Make a copy of the plane.
    fp = new csPlane3 (*fp);
  }
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiCamera);
  cameranr = cur_cameranr++;
}

csCamera::~csCamera ()
{
  delete fp;
}

void csCamera::SetFarPlane (const csPlane3* farplane)
{
  delete fp;
  if (farplane)
    fp = new csPlane3 (*farplane);
  else
    fp = NULL;
}

csPolygon3D* csCamera::GetHit (csVector3& v)
{
  csVector3 isect;
  return sector->HitBeam (v_o2t, v, isect);
}

void csCamera::MoveWorld (const csVector3& v, bool cd)
{
  csVector3 new_position = GetOrigin () + v;
  if (sector)
  {
    csVector3 remember_position = new_position;
    csSector* new_sector = sector->FollowSegment (*this, new_position,
	mirror, only_portals);
    if (new_sector == sector)
    {
      if (!cd) new_position = remember_position;
    }
    else sector = new_sector;
  }
  SetOrigin (new_position);
  cameranr = cur_cameranr++;
}

void csCamera::Correct (int n)
{
  csVector3 w1, w2, w3;
  float* vals[5];
  if (n==0) return;

  w3 = m_t2o.Col3();
  vals[0] = &w3.x;  vals[1] = &w3.y;  vals[2] = &w3.z;  vals[4] = NULL;
  Correct (n, vals);  /* perform the snap-to operation on the forward vector */

/* Maybe w3 should be normalized.  Only necessary if there is
   significant roundoff error: */
//  w3 = csVector3::unit(w3); 
  
/* perhaps a snap-to should be performed on one of the other vectors as well */

  w1 = m_t2o.Col2();
  w2 = csVector3::Unit(w3 % w1);
  w1 = w2 % w3;
  
  SetT2O (
    csMatrix3 (w1.x, w2.x, w3.x,
               w1.y, w2.y, w3.y,
               w1.z, w2.z, w3.z));
}

void csCamera::Correct (int n, float* vals[])
{
  float r, angle;

  if (vals==NULL) return;
  if (vals[0]==NULL) return;
  if (vals[1]==NULL) return;
  if (vals[2]!=NULL)
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
  
  angle = atan2 (*vals[1], *vals[0]);
  angle = (TWO_PI / n) * QRound (n * angle / TWO_PI);
  *vals[1] = qsqrt ( (*vals[0])*(*vals[0]) + (*vals[1])*(*vals[1]) );
  Correct (n, vals+1);
  r = *vals[1];
  *vals[0] = r*cos (angle);
  *vals[1] = r*sin (angle); 
  cameranr = cur_cameranr++;
}

void csCamera::SetFOVAngle (float a, int width)
{
  float disp_width = (float)width/2.;
  float disp_radius = disp_width / cos (a / (2. * (360. / TWO_PI)));
  float rview_fov = qsqrt (disp_radius*disp_radius - disp_width*disp_width);
  aspect = int (rview_fov*2.);
  inv_aspect = 1.0f / (rview_fov*2.);
  fov_angle = a;
  cameranr = cur_cameranr++;
}

void csCamera::ComputeAngle (int width)
{
  float rview_fov = (float)GetFOV ()/2.;
  float disp_width = (float)width/2.;
  float inv_disp_radius = qisqrt (rview_fov*rview_fov + disp_width*disp_width);
  fov_angle = 2. * acos (disp_width * inv_disp_radius) * (360. / TWO_PI);
}

void csCamera::ComputeDefaultAngle (int width)
{
  float rview_fov = (float)GetDefaultFOV ()/2.;
  float disp_width = (float)width/2.;
  float inv_disp_radius = qisqrt (rview_fov*rview_fov + disp_width*disp_width);
  default_fov_angle = 2. * acos (disp_width * inv_disp_radius)
  	* (360. / TWO_PI);
}

