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
  const float c1 = cosf (halfAngles.y);
  const float s1 = sinf (halfAngles.y);
  const float c2 = cosf (halfAngles.x);
  const float s2 = sinf (halfAngles.x);
  const float c3 = cosf (halfAngles.z);
  const float s3 = sinf (halfAngles.z);

  const float c1c2 = c1*c2;
  const float s1s2 = s1*s2;

  w = c1c2*c3 - s1s2*s3;
  v.x = c1c2*s3 + s1s2*c3;
  v.y = s1*c2*c3 + c1*s2*s3;
  v.z = c1*s2*c3 - s1*c2*s3;
}


csVector3 csQuaternion::GetEulerAngles () const
{
  csVector3 ret;

  const float test = v.x*v.y + v.z*w;
  
  if (test > 0.499f) //"north pole" singularitv.y
  {
    return csVector3 (PI/2.0f, 2.0f * atan2f (v.x, w), 0);
  }
  else if (test < -0.499f) //"south pole" singularitv.y
  {
    return csVector3 (-PI/2.0f, -2.0f * atan2f (v.x,w), 0);
  }

  const float sqx2 = 2.0f*v.x*v.x;
  const float sqy2 = 2.0f*v.y*v.y;
  const float sqz2 = 2.0f*v.z*v.z;

  return csVector3 (
    asinf (2.0f*test), 
    atan2f (2.0f*(v.y*w - v.x*v.z), 1.0f - sqy2 - sqz2),
    atan2f (2.0f*(v.x*w - v.y*v.z), 1.0f - sqx2 - sqz2));
}

void csQuaternion::SetMatrix (const csMatrix3& matrix)
{
  // Ken Shoemake's article in 1987 SIGGRAPH course notes
  const float trace = matrix.m11 + matrix.m22 + matrix.m33;

  if (trace > 0.0f)
  {
    // Quick-route
    float s = 1.0f / sqrtf (trace + 1.0f);
    w = 0.25f / s;
    v.x = (matrix.m23 - matrix.m32) * s;
    v.y = (matrix.m31 - matrix.m13) * s;
    v.z = (matrix.m12 - matrix.m21) * s;
  }
  else
  {
    //Check biggest diagonal elmenet
    if (matrix.m11 > matrix.m22 && matrix.m11 > matrix.m33)
    {
      //X biggest
      float s = 1.0f / (2.0f * sqrtf (1.0f + matrix.m11 - matrix.m22 - matrix.m33));
      w = (matrix.m32 - matrix.m23) * s;
      v.x = 0.25f / s;
      v.y = (matrix.m21 + matrix.m12) * s;
      v.z = (matrix.m31 + matrix.m13) * s;
    }
    else if (matrix.m22 > matrix.m33)
    {
      //Y biggest
      float s = 1.0f / (2.0f * sqrtf (1.0f + matrix.m22 - matrix.m11 - matrix.m33));
      w = (matrix.m31 - matrix.m13) * s;
      v.x = (matrix.m21 + matrix.m12) * s;
      v.y = 0.25f / s;
      v.z = (matrix.m32 + matrix.m23) * s;
    }
    else
    {
      //Z biggest
      float s = 1.0f / (2.0f * sqrtf (1.0f + matrix.m33 - matrix.m11 - matrix.m22));
      w = (matrix.m21 - matrix.m12) * s;
      v.x = (matrix.m31 + matrix.m13) * s;
      v.y = (matrix.m32 + matrix.m23) * s;
      v.z = 0.25f / s;
    }
  }
}

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
  //compute cos between them and clamp to -1, 1
  const float dA = csClamp (Dot (q2), 1.0f, -1.0f); 

  if (dA > 0.998f) //really small, NLerp instead of slerp
  {
    return NLerp (q2, t);
  }
  else if (dA < -0.998f) //almost opposite, special treatment too
  {
    const float s0 = sinf ((1.0f - t)*PI);
    const float s1 = sinf (t*PI);

    return csQuaternion (
      s0*v.x - s1*q2.v.y,
      s0*v.y + s1*q2.v.x,
      s0*v.z - s1*q2.w,
      s0*w + s1*q2.v.z);
  }

  const float angle = acosf (dA);

  // Do a normal slerp
  const float invSinA = 1.0/sinf (angle);
  const float s0 = sinf ((1.0f - t)*angle) * invSinA;
  const float s1 = sinf (t*angle) * invSinA;

  if (angle < 0.0f)
  {
    //need to invert
    return (s0*(*this) + -s1*q2).Unit ();
  }
  
  return s0*(*this) + s1*q2;
}
