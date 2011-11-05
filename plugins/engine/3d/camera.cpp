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
#include "csgeom/projections.h"
#include "plugins/engine/3d/camera.h"
#include "plugins/engine/3d/sector.h"
#include "plugins/engine/3d/engine.h"

CS_PLUGIN_NAMESPACE_BEGIN(Engine)
{

long csCameraBase:: cur_cameranr = 0;

csCameraBase::csCameraBase () : scfImplementationType (this), fp (0)
{
  mirror = false;
  sector = 0;
  BumpCamera();
  only_portals = true;
}

csCameraBase::csCameraBase (const csCameraBase* other)
  : scfImplementationType (this)
{
  *this = *other;
  if (fp)
  {
    // Make a copy of the plane.
    fp = new csPlane3 (*fp);
  }
  // Listeners are not supposed to be cloned
  listeners.DeleteAll();
  BumpCamera();
}

csCameraBase::~csCameraBase ()
{
  delete fp;
}

void csCameraBase::FireCameraSectorListeners (iSector* sector)
{
  size_t i;
  for (i = 0 ; i < listeners.GetSize () ; i++)
    listeners[i]->NewSector ((iCamera*)this, sector);
}

void csCameraBase::FireCameraMovedListeners ()
{
  size_t i;
  for (i = 0 ; i < listeners.GetSize () ; i++)
    listeners[i]->CameraMoved ((iCamera*)this);
}

void csCameraBase::SetFarPlane (csPlane3 *farplane)
{
  delete fp;
  if (farplane)
    fp = new csPlane3 (*farplane);
  else
    fp = 0;
}

void csCameraBase::MoveWorld (const csVector3 &v, bool cd)
{
  csVector3 new_position = GetOrigin () + v;

  if (sector)
  {
    csVector3 remember_position = new_position;

    // Test if the motion crosses a portal
    iSector *new_sector = sector->FollowSegment (
        *this,
        new_position,
        mirror,
        only_portals);

    if (new_sector != sector)
    {
      sector = new_sector;
      cameranr = cur_cameranr++;
      FireCameraSectorListeners (sector);
    }
  }

  FireCameraMovedListeners ();

  SetOrigin (new_position);
  cameranr = cur_cameranr++;
}

void csCameraBase::Correct (int n)
{
  if (n == 0) return;

  csVector3 w1, w2, w3;
  float *vals[5];

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

void csCameraBase::Correct (int n, float *vals[])
{
  if (vals == 0) return;
  if (vals[0] == 0) return;
  if (vals[1] == 0) return;
  
  float r;
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

  float angle;
  angle = (float)atan2 (*vals[1], *vals[0]);
  angle = (TWO_PI / n) * csQround (n * angle / TWO_PI);
  *vals[1] = csQsqrt ((*vals[0]) * (*vals[0]) + (*vals[1]) * (*vals[1]));
  Correct (n, vals + 1);
  r = *vals[1];
  *vals[0] = r * (float)cos (angle);
  *vals[1] = r * (float)sin (angle);
  cameranr = cur_cameranr++;
}

//---------------------------------------------------------------------------

float PerspectiveImpl:: default_aspect = 0;
float PerspectiveImpl:: default_inv_aspect = 0;
float PerspectiveImpl:: default_fov_angle = 90;

PerspectiveImpl::PerspectiveImpl (csEngine* engine)
  : nearClip (engine->csEngine::GetDefaultNearClipDistance()),
    matrixDirty (true), invMatrixDirty (true)
{
  aspect = default_aspect;
  inv_aspect = default_inv_aspect;
  fov_angle = default_fov_angle;
  shift_x = 0.5f;
  shift_y = 0.5f;
}

void PerspectiveImpl::SetDefaultFOVAngle (float a, float width)
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
  default_aspect = ((tan((vRefFOVAngle * 0.5) / 180 * PI) * vRefFOV) /
      tan((a * 0.5)  / 180 * PI));

  // set the other neccessary variables
  default_inv_aspect = 1.0f / default_aspect;
  default_fov_angle = a;
}


void PerspectiveImpl::SetFOVAngle (float a, float width)
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
  aspect = ((tan((vRefFOVAngle * 0.5) / 180 * PI) * vRefFOV) /
           tan((a * 0.5)  / 180 * PI));

  // set the other neccessary variables
  inv_aspect = 1.0f / aspect;
  fov_angle = a;
  Dirtify();
}

void PerspectiveImpl::ComputeAngle (float width)
{
  float rview_fov = (float)GetFOV () * 0.5f;
  float disp_width = (float)width * 0.5f;
  float inv_disp_radius = csQisqrt (
      rview_fov * rview_fov + disp_width * disp_width);
  fov_angle = 2.0f * (float)acos (disp_width * inv_disp_radius)
  	* (360.0f / TWO_PI);
  Dirtify();
}

void PerspectiveImpl::ComputeDefaultAngle (float width)
{
  float rview_fov = (float)GetDefaultFOV () * 0.5f;
  float disp_width = (float)width * 0.5f;
  float inv_disp_radius = csQisqrt (
      rview_fov * rview_fov + disp_width * disp_width);
  default_fov_angle = 2.0f * (float)acos (disp_width * inv_disp_radius)
  	* (360.0f / TWO_PI);
}

void PerspectiveImpl::UpdateMatrix ()
{
  if (!matrixDirty) return;
  
  matrix = CS::Math::Projections::CSPerspective (1.0f, 
    aspect, shift_x, shift_y*aspect, inv_aspect, nearClip);
  
  matrixDirty = false;
  invMatrixDirty = true;
}

void PerspectiveImpl::UpdateInvMatrix ()
{
  if (!invMatrixDirty) return;
  
  invMatrix = matrix.GetInverse();
  
  invMatrixDirty = false;
}

//---------------------------------------------------------------------------

void csCameraPerspective::UpdateClipPlanes()
{
  if (!clipPlanesDirty) return;
  
  float lx, rx, ty, by;
  lx = -shift_x * inv_aspect;
  rx = (1.0f - shift_x) * inv_aspect;
  ty = -shift_y;
  by = (1.0f - shift_y);
  
  csPlane3* frust = clipPlanes;
  csVector3 v1 (lx, ty, 1);
  csVector3 v2 (rx, ty, 1);
  frust[0].Set (v1 % v2, 0);
  frust[0].norm.Normalize ();

  csVector3 v3 (rx, by, 1);
  frust[1].Set (v2 % v3, 0);
  frust[1].norm.Normalize ();
  v2.Set (lx, by, 1);
  frust[2].Set (v3 % v2, 0);
  frust[2].norm.Normalize ();
  frust[3].Set (v2 % v1, 0);
  frust[3].norm.Normalize ();
  
  csPlane3 pz0 (0, 0, 1, 0);	// Inverted!!!.
  clipPlanes[4] = pz0;
  clipPlanesMask = 0x1f;

  if (fp)
  {
    clipPlanes[5] = *fp;
    clipPlanesMask |= 0x20;
  }
  
  clipPlanesDirty = false;
}
//---------------------------------------------------------------------------

csCameraCustomMatrix::csCameraCustomMatrix (csCameraBase* other)
  : scfImplementationType (this, other), invMatrixDirty (true),
    clipPlanesDirty (true)
{
  if (fp)
  {
    // Make a copy of the plane.
    fp = new csPlane3 (*fp);
  }
  // Listeners are not supposed to be cloned
  listeners.DeleteAll();
  matrix = other->GetProjectionMatrix();
  BumpCamera();
}

void csCameraCustomMatrix::UpdateInvMatrix ()
{
  if (!invMatrixDirty) return;
  
  invMatrix = matrix.GetInverse();
  
  invMatrixDirty = false;
}

const csPlane3* csCameraCustomMatrix::GetVisibleVolume (uint32& mask)
{
  if (clipPlanesDirty)
  {
    CS::Math::Matrix4 invMatrix_inv_t = matrix.GetTranspose();
      
    int n = 0;
    csPlane3 p;
    // Back plane
    p.Set (0, 0, 1, 1);
    clipPlanes[n] = invMatrix_inv_t * p;
    clipPlanes[n].Normalize();
    n++;
    // Far plane
    p.Set (0, 0, -1, 1);
    clipPlanes[n] = invMatrix_inv_t * p;
    clipPlanes[n].Normalize();
    n++;
    // Left plane
    p.Set (1, 0, 0, 1);
    clipPlanes[n] = invMatrix_inv_t * p;
    clipPlanes[n].Normalize();
    n++;
    // Right plane
    p.Set (-1, 0, 0, 1);
    clipPlanes[n] = invMatrix_inv_t * p;
    clipPlanes[n].Normalize();
    n++;
    // Bottom plane
    p.Set (0, 1, 0, 1);
    clipPlanes[n] = invMatrix_inv_t * p;
    clipPlanes[n].Normalize();
    n++;
    // Top plane
    p.Set (0, -1, 0, 1);
    clipPlanes[n] = invMatrix_inv_t * p;
    clipPlanes[n].Normalize();
    n++;
    
    clipPlanesMask = (1 << n) - 1;
    
    clipPlanesDirty = false;
  }
    
  mask = clipPlanesMask;
  return clipPlanes;
}

}
CS_PLUGIN_NAMESPACE_END(Engine)

