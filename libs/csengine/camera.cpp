/*
    Copyright (C) 1998, 2000 by Jorrit Tyberghein
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

IMPLEMENT_IBASE (csCamera)
  IMPLEMENTS_INTERFACE (iBase)
  IMPLEMENTS_EMBEDDED_INTERFACE (iCamera)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csCamera::Camera)
  IMPLEMENTS_INTERFACE (iCamera)
IMPLEMENT_EMBEDDED_IBASE_END

int csCamera::default_aspect = 0;
float csCamera::default_inv_aspect = 0;
float csCamera::default_fov_angle = 90;

csCamera::csCamera () : csOrthoTransform()
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiCamera);
  mirror = false;
  sector = NULL;
  aspect = default_aspect;
  inv_aspect = default_inv_aspect;
  fov_angle = default_fov_angle;
  shift_x = csEngine::frame_width / 2;
  shift_y = csEngine::frame_height / 2;
  use_farplane = false;
  fp = NULL;
}

csCamera::csCamera (csCamera* c) : csOrthoTransform ()
{
  *this = *c;
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiCamera);
}

csCamera::csCamera (const csCamera& c) : csOrthoTransform ()
{
  *this = c;
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiCamera);
}

csCamera::~csCamera ()
{
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
    csSector* new_sector = sector->FollowSegment (*this, new_position, mirror);
    if (new_sector == sector)
    {
      if (!cd) new_position = remember_position;
    }
    else sector = new_sector;
  }
  SetOrigin (new_position);
}

void csCamera::RotateWorld (const csVector3& v, float angle)
{
  csVector3 u = v;
  float ca, sa;
  u = csVector3::Unit (u);
  ca = cos (angle);
  sa = sin (angle);
  
  SetT2O ( 

   csMatrix3(
         ca+(1-ca)*u.x*u.x, u.x*u.y*(1-ca)-u.z*sa, u.x*u.z*(1-ca)+u.y*sa,
     u.x*u.y*(1-ca)+u.z*sa,     ca+(1-ca)*u.y*u.y, u.y*u.z*(1-ca)-u.x*sa,
     u.x*u.z*(1-ca)-u.y*sa, u.y*u.z*(1-ca)+u.x*sa,     ca+(1-ca)*u.z*u.z)

   * GetT2O () );
}

void csCamera::Rotate (const csVector3& v, float angle)
{
  csVector3 u = v;
  float ca, sa;
  u = csVector3::Unit (u);
  ca = cos (angle);
  sa = sin (angle);
  
  SetT2O (GetT2O () *

   csMatrix3(
         ca+(1-ca)*u.x*u.x, u.x*u.y*(1-ca)-u.z*sa, u.x*u.z*(1-ca)+u.y*sa,
     u.x*u.y*(1-ca)+u.z*sa,     ca+(1-ca)*u.y*u.y, u.y*u.z*(1-ca)-u.x*sa,
     u.x*u.z*(1-ca)-u.y*sa, u.y*u.z*(1-ca)+u.x*sa,     ca+(1-ca)*u.z*u.z)

   );
}

void csCamera::LookAt (const csVector3& v, const csVector3& up)
{
  csMatrix3 m; /* initialized to be the identity matrix */
  csVector3 w1, w2, w3 = v;

  float sqr;
  sqr = v*v;
  if (sqr > SMALL_EPSILON)
  {
    w3 *= qisqrt (sqr);
    w1 = w3 % up;
    sqr = w1*w1;
    if (sqr < SMALL_EPSILON)
    {
      w1 = w3 % csVector3(0,0,-1);
      sqr = w1*w1;
      if (sqr < SMALL_EPSILON)
      {
       w1 = w3 % csVector3(0,-1,0);
       sqr = w1*w1;
      }
    }
    w1 *= qisqrt (sqr);
    w2 = w3 % w1;

    m.m11 = w1.x;  m.m12 = w2.x;  m.m13 = w3.x;
    m.m21 = w1.y;  m.m22 = w2.y;  m.m23 = w3.y;
    m.m31 = w1.z;  m.m32 = w2.z;  m.m33 = w3.z;
  }

  SetT2O (m);
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
    { r = *vals[2];  *vals[2] = *vals[0];  *vals[0] = r; }
    else
    { r = *vals[2];  *vals[2] = *vals[1];  *vals[1] = r; }
  }
  
  angle = atan2 (*vals[1], *vals[0]);
  angle = (2.*M_PI/n) * QRound(n * angle / (2*M_PI) );
  *vals[1] = qsqrt( (*vals[0])*(*vals[0]) + (*vals[1])*(*vals[1]) );
  Correct(n, vals+1);
  r = *vals[1];
  *vals[0] = r*cos(angle);  *vals[1] = r*sin(angle); 
}

void csCamera::SetFOVAngle (float a, int width)
{
  float disp_width = (float)width/2.;
  float disp_radius = disp_width / cos (a/(2.*(360./(2.*M_PI))));
  float rview_fov = qsqrt (disp_radius*disp_radius - disp_width*disp_width);
  aspect = int (rview_fov*2.);
  inv_aspect = 1.0f / (rview_fov*2.);
  fov_angle = a;
}

void csCamera::ComputeAngle (int width)
{
  float rview_fov = (float)GetFOV ()/2.;
  float disp_width = (float)width/2.;
  float inv_disp_radius = qisqrt (rview_fov*rview_fov + disp_width*disp_width);
  fov_angle = 2. * acos (disp_width * inv_disp_radius) * (360./(2.*M_PI));
}

void csCamera::ComputeDefaultAngle (int width)
{
  float rview_fov = (float)GetDefaultFOV ()/2.;
  float disp_width = (float)width/2.;
  float inv_disp_radius = qisqrt (rview_fov*rview_fov + disp_width*disp_width);
  default_fov_angle = 2. * acos (disp_width * inv_disp_radius) * (360./(2.*M_PI));
}
