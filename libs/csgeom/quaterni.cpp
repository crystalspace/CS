/*
    Copyright (C) 2000 by Norman Kramer
                  2006 by Marten Svanfeldt

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

#include "csgeom/matrix3.h"
#include "csgeom/quaternion.h"
#include "csgeom/math.h"


void csQuaternion::SetEulerAngles (const csVector3& angles)
{
  csVector3 halfAngles = angles / 2.0f;
  const float cx = cosf (halfAngles.x);
  const float sx = sinf (halfAngles.x);
  const float cy = cosf (halfAngles.y);
  const float sy = sinf (halfAngles.y);
  const float cz = cosf (halfAngles.z);
  const float sz = sinf (halfAngles.z);

  const float cxcz = cx*cz;
  const float cxsz = cx*sz;
  const float sxcz = sx*cz;
  const float sxsz = sx*sz;
  
  v.x = (cy * sxcz) - (sy * cxsz);
  v.y = (cy * sxsz) + (sy * cxcz);
  v.z = (cy * cxsz) - (sy * sxcz);
  w   = (cy * cxcz) + (sy * sxsz);
}


csVector3 csQuaternion::GetEulerAngles () const
{
  csVector3 angles;
  const float case1 = PI / 2.0f;
  const float case2 = -PI / 2.0f;

  angles.z = atan2f (2.0f * (v.x*v.y + w*v.z), (w*w + v.x*v.x - v.y*v.y - v.z*v.z));
  float sine = -2.0f * (v.x*v.z - w*v.y);

  if(sine >= 1)     //cases whewe value is 1 ow -1 cause NAN
    angles.y = case1;
  else if ( sine <= -1 )
    angles.y = case2;
  else
    angles.y = asinf (sine);

  angles.x = atan2f (2.0f * (w*v.x + v.y*v.z), (w*w - v.x*v.x - v.y*v.y + v.z*v.z)) ;

  return angles;
}

#if 0
void csQuaternion::SetMatrix (const csMatrix3& matrix)
{
  // Ken Shoemake's article in 1987 SIGGRAPH course notes
 
  csMatrix3 mat = csMatrix3( matrix.m11, matrix.m21, matrix.m31,
                          matrix.m12, matrix.m22, matrix.m32,
                          matrix.m13, matrix.m23, matrix.m33);
 
  const float trace = mat.m11 + mat.m22 + mat.m33;
 
  if (trace >= 0.0f)
  {
    // Quick-route
    float s = sqrtf (trace + 1.0f);
    w = 0.5f * s;
    s = 0.5f / s;
    v.x = (mat.m32 - mat.m23) * s;
    v.y = (mat.m13 - mat.m31) * s;
    v.z = (mat.m21 - mat.m12) * s;
  }
  else
  {
    //Check biggest diagonal elmenet
    if (mat.m11 > mat.m22 && mat.m11 > mat.m33)
    {
      //X biggest
      float s = sqrtf (1.0f + mat.m11 - mat.m22 - mat.m33);
      v.x = 0.5f * s;
      s = 0.5f / s;
      w = (mat.m32 - mat.m23) * s;
      v.y = (mat.m12 + mat.m21) * s;
      v.z = (mat.m31 + mat.m13) * s;
    }
    else if (mat.m22 > mat.m33)
    {
      //Y biggest
      float s = sqrtf (1.0f + mat.m22 - mat.m11 - mat.m33);
      v.y = 0.5f * s;
      s = 0.5f / s;
      w = (mat.m13 - mat.m31) * s;
      v.x = (mat.m12 + mat.m21) * s;
      v.z = (mat.m23 + mat.m32) * s;
    }
    else
    {
      //Z biggest
      float s = sqrtf (1.0f + mat.m33 - mat.m11 - mat.m22);
      v.z = 0.5f * s;
      s = 0.5f / s;
      w = (mat.m21 - mat.m12) * s;
      v.x = (mat.m31 + mat.m13) * s;
      v.y = (mat.m23 + mat.m32) * s;
    }
  }
}
#else
void csQuaternion::SetMatrix (const csMatrix3& matrix)
{
  // Ken Shoemake's article in 1987 SIGGRAPH course notes
  const float trace = matrix.m11 + matrix.m22 + matrix.m33;

  if (trace >= 0.0f)
  {
    // Quick-route
    float s = sqrtf (trace + 1.0f);
    w = 0.5f * s;
    s = 0.5f / s;
    v.x = (matrix.m32 - matrix.m23) * s;
    v.y = (matrix.m13 - matrix.m31) * s;
    v.z = (matrix.m21 - matrix.m12) * s;
  }
  else
  {
    //Check biggest diagonal elmenet
    if (matrix.m11 > matrix.m22 && matrix.m11 > matrix.m33)
    {
      //X biggest
      float s = sqrtf (1.0f + matrix.m11 - matrix.m22 - matrix.m33);
      v.x = 0.5f * s;
      s = 0.5f / s;
      w = (matrix.m32 - matrix.m23) * s;
      v.y = (matrix.m12 + matrix.m21) * s;
      v.z = (matrix.m31 + matrix.m13) * s;
    }
    else if (matrix.m22 > matrix.m33)
    {
      //Y biggest
      float s = sqrtf (1.0f + matrix.m22 - matrix.m11 - matrix.m33);
      v.y = 0.5f * s;
      s = 0.5f / s;
      w = (matrix.m13 - matrix.m31) * s;
      v.x = (matrix.m12 + matrix.m21) * s;
      v.z = (matrix.m23 + matrix.m32) * s;
    }
    else
    {
      //Z biggest
      float s = sqrtf (1.0f + matrix.m33 - matrix.m11 - matrix.m22);
      v.z = 0.5f * s;
      s = 0.5f / s;
      w = (matrix.m21 - matrix.m12) * s;
      v.x = (matrix.m31 + matrix.m13) * s;
      v.y = (matrix.m23 + matrix.m32) * s;
    }
  }
}
#endif

csMatrix3 csQuaternion::GetMatrix () const
{
  const float x2 = v.x*2.0f;
  const float y2 = v.y*2.0f;
  const float z2 = v.z*2.0f;

  const float xx2 = v.x*x2;
  const float xy2 = v.y*x2;
  const float xz2 = v.z*x2;
  const float xw2 = w*x2;

  const float yy2 = v.y*y2;
  const float yz2 = v.z*y2;
  const float yw2 = w*y2;

  const float zz2 = v.z*z2;
  const float zw2 = w*z2;

  
  return csMatrix3 (
    1.0f - (yy2+zz2), xy2 - zw2,        xz2 + yw2,
    xy2 + zw2,        1.0f - (xx2+zz2), yz2 - xw2,
    xz2 - yw2,        yz2 + xw2,        1.0f - (xx2+yy2));
}

csQuaternion csQuaternion::NLerp (const csQuaternion& q2, float t) const
{
  return (*this + t * (q2 - (*this))).Unit ();
}

csQuaternion csQuaternion::SLerp (const csQuaternion& q2, float t) const
{
  float omega, cosom, invsinom, scale0, scale1;

  csQuaternion quato(q2);

  // decide if one of the quaternions is backwards  
  float a = (*this-q2).SquaredNorm ();
  float b = (*this+q2).SquaredNorm ();
  if (a > b)
  {
    quato = -q2;
  }

  // Calculate dot between quats
  cosom = Dot (quato);

  // Make sure the two quaternions are not exactly opposite? (within a little
  // slop).
  if (cosom > -0.9998f)
  {
    // Are they more than a little bit different?  Avoid a divided by zero
    // and lerp if not.
    if (cosom < 0.9998f)
    {
      // Yes, do a slerp
      omega = acos (cosom);
      invsinom = 1.0f / sin (omega);
      scale0 = sin ((1.0f - t) * omega) * invsinom;
      scale1 = sin (t * omega) * invsinom;
    }
    else
    {
      // Not a very big difference, do a lerp
      scale0 = 1.0f - t;
      scale1 = t;
    }

    return csQuaternion (
      scale0 * v.x + scale1 * quato.v.x,
      scale0 * v.y + scale1 * quato.v.y,
      scale0 * v.z + scale1 * quato.v.z,
      scale0 * w + scale1 * quato.w);
  }

  // The quaternions are nearly opposite so to avoid a divided by zero error
  // Calculate a perpendicular quaternion and slerp that direction
  scale0 = sin ((1.0f - t) * PI);
  scale1 = sin (t * PI);
  return csQuaternion (
    scale0 * v.x + scale1 * -quato.v.y,
    scale0 * v.y + scale1 * quato.v.x,
    scale0 * v.z + scale1 * -quato.w,
    scale0 * w + scale1 * quato.v.z);
}

csQuaternion csQuaternion::Log () const
{
  csVector3 _v;
  if (fabs(w) < 1.0f)
  {
    float angle = acos(w);
    float sin_angle = sin(angle);
    if (fabs(sin_angle) >= SMALL_EPSILON)
    {
      float coef = angle/sin_angle;
      _v = v*coef;
      return csQuaternion (_v, 0);
    }
  }
  _v = v;
  return csQuaternion (_v, 0);
}

csQuaternion csQuaternion::Exp () const
{
  float angle = sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
  float sin_angle = sin(angle);
  csVector3 _v;
  if (fabs(angle) >= SMALL_EPSILON )
  {
    float coef = sin_angle/angle;
    _v = v*coef;
  }
  else
  {
    _v = v;
  }
  return csQuaternion (_v, cosf(angle));
}

csQuaternion csQuaternion::Squad (const csQuaternion & t1, const csQuaternion & t2,
  const csQuaternion & q, float t) const
{
  return SLerp(q, t).SLerp(t1.SLerp(t2, t), 2.0f*t*(1.0f-t));
}
