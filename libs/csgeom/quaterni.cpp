/*
    Copyright (C) 2000 by Norman Kramer

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
#include "csgeom/quaterni.h"

#define EULERTOQUATCONST      0.0174532925199 // PI*1/(360/2)
#define QUATTOAXISANGLECONST  114.591559026 \
 \
  // (360/2)*(1/PI)*2
void csQuaternion::SetWithEuler (const csVector3 &rot)
{
  float rx, ry, rz, tx, ty, tz, cx, cy, cz, sx, sy, sz, cc, cs, sc, ss;

  // FIRST STEP, CONVERT ANGLES TO RADIANS
  rx = rot.x * (float)EULERTOQUATCONST;
  ry = rot.y * (float)EULERTOQUATCONST;
  rz = rot.z * (float)EULERTOQUATCONST;

  // GET THE HALF ANGLES
  tx = rx * (float)0.5;
  ty = ry * (float)0.5;
  tz = rz * (float)0.5;
  cx = (float)cos (tx);
  cy = (float)cos (ty);
  cz = (float)cos (tz);
  sx = (float)sin (tx);
  sy = (float)sin (ty);
  sz = (float)sin (tz);

  cc = cx * cz;
  cs = cx * sz;
  sc = sx * cz;
  ss = sx * sz;

  x = (cy * sc) - (sy * cs);
  y = (cy * ss) + (sy * cc);
  z = (cy * cs) - (sy * sc);
  r = (cy * cc) + (sy * ss);
}

csQuaternion csQuaternion::ToAxisAngle () const
{
  float invscale, tr;

  tr = (float)acos (r);
  invscale = 1.0f / ((float)sin (tr));
  return csQuaternion (
      tr * (float)QUATTOAXISANGLECONST,
      x * invscale,
      y * invscale,
      z * invscale);
}

#define SLERPDELTA  0.0001f \
 \
  // Difference at which to lerp instead of slerp
csQuaternion csQuaternion::Slerp (
  const csQuaternion &quat2,
  float slerp) const
{
  double omega, cosom, invsinom, scale0, scale1;

  csQuaternion quato(quat2);

  // decide if one of the quaternions is backwards
  double a = (x-quat2.x)*(x-quat2.x) + (y-quat2.y)*(y-quat2.y) + (z-quat2.z)*(z-quat2.z) + (r-quat2.r)*(r-quat2.r);
  double b = (x+quat2.x)*(x+quat2.x) + (y+quat2.y)*(y+quat2.y) + (z+quat2.z)*(z+quat2.z) + (r+quat2.r)*(r+quat2.r);
  if (a > b)
  {
      quato.Negate();
  }

  // Calculate dot between quats
  cosom = x * quato.x + y * quato.y + z * quato.z + r * quato.r;

  // Make sure the two quaternions are not exactly opposite? (within a little
  // slop).
  if ((1.0f + cosom) > SLERPDELTA)
  {
    // Are they more than a little bit different?  Avoid a divided by zero
    // and lerp if not.
    if ((1.0f - cosom) > SLERPDELTA)
    {
      // Yes, do a slerp
      omega = acos (cosom);
      invsinom = 1.0f / sin (omega);
      scale0 = sin ((1.0f - slerp) * omega) * invsinom;
      scale1 = sin (slerp * omega) * invsinom;
    }
    else
    {
      // Not a very big difference, do a lerp
      scale0 = 1.0f - slerp;
      scale1 = slerp;
    }

    return csQuaternion (
        scale0 * r + scale1 * quato.r,
        scale0 * x + scale1 * quato.x,
        scale0 * y + scale1 * quato.y,
        scale0 * z + scale1 * quato.z);
  }

  // The quaternions are nearly opposite so to avoid a divided by zero error
  // Calculate a perpendicular quaternion and slerp that direction
  scale0 = sin ((1.0f - slerp) * PI);
  scale1 = sin (slerp * PI);
  return csQuaternion (
      scale0 * r + scale1 * quato.z,
      scale0 * x + scale1 * -quato.y,
      scale0 * y + scale1 * quato.x,
      scale0 * z + scale1 * -quato.r);
}


csQuaternion::csQuaternion(const csMatrix3& mat)
{
    float  tr, s;
    int    i;

    tr = mat.m11 + mat.m22 + mat.m33 + 1.0;

    // check the diagonal
    if (tr > 0.0) {
        s = 0.5 / sqrt (tr);
        r = 0.25 / s;
        x = (mat.m32 - mat.m23) * s;
        y = (mat.m13 - mat.m31) * s;
        z = (mat.m21 - mat.m12) * s;
    } else {
        // diagonal is negative
        i = 1;
        if (mat.m22 > mat.m11) i = 2;
        if ((i == 1 && mat.m33 > mat.m11)
            || (i == 2 && mat.m33 > mat.m22)) i = 3;

            /*
            m11 = 0
            m12 = 1
            m13 = 2
            m21 = 4
            m22 = 5
            m23 = 6
            m31 = 8
            m32 = 9
            m33 = 10
            */

        switch(i) {
        case 1:
            s = sqrt ((mat.m11 - (mat.m22 + mat.m33)) + 1.0);

            x = s * 0.5;

            if (s != 0.0) s = 0.5 / s;

            y = (mat.m12 + mat.m21) * s;
            z = (mat.m13 + mat.m31) * s;
            r = (mat.m23 - mat.m32) * s;
            break;
        case 2:
            s = sqrt ((mat.m22 - (mat.m33 + mat.m11)) + 1.0);

            y = 0.5 * s;

            if (s != 0.0) s = 0.5 / s;

            x = (mat.m12 + mat.m21) * s;
            z = (mat.m23 + mat.m32) * s;
            r = (mat.m13 - mat.m31) * s;

            break;
        case 3:
            s = sqrt ((mat.m33 - (mat.m11 + mat.m22)) + 1.0);

            z = 0.5 * s;

            if (s != 0.0) s = 0.5 / s;

            x = (mat.m13 + mat.m31) * s;
            y = (mat.m23 + mat.m32) * s;
            r = (mat.m12 - mat.m21) * s;
            break;
        }
    }
}


void csQuaternion::Invert()
{
    float norm, invNorm;

    norm = x * x + y * y + z * z + r * r;

    invNorm = (float) (1.0 / norm);

    x = -x * invNorm;
    y = -y * invNorm;
    z = -z * invNorm;
    r =  r * invNorm;
}

void csQuaternion::GetAxisAngle(csVector3& axis, float& phi) const
{
    phi = 2.0 * acos(r);
    float ss = sin(phi/2.0);
    if (ss != 0) {
        axis.x = x / ss;
        axis.y = y / ss;
        axis.z = z / ss;
    }
    else
    {
        axis.x = 0;
        axis.y = 0;
        axis.z = 1;
    }
}

void csQuaternion::SetWithAxisAngle(csVector3 axis, float phi)
{
    axis.Normalize();
    float ss = sin(phi/2.0);
    r = cos(phi/2.0);
    x = axis.x * ss;
    y = axis.y * ss;
    z = axis.z * ss;
}

void csQuaternion::GetEulerAngles (csVector3& angles)
{
  static float rad2deg = 180.0f / PI;
  static float case1 = PI / 2.0f * 180.0f / rad2deg;
  static float case2 = -PI / 2.0f * rad2deg;

  angles.z = atan2 (2.0f * (x*y + r*z), (r*r + x*x - y*y - z*z)) * rad2deg;
  float sine = -2.0f * (x*z - r*y);

  if(sine >= 1)     //cases where value is 1 or -1 cause NAN
    angles.y = case1; //PI / 2 * 180.0f / rad2deg;
  else if ( sine <= -1 )
    angles.y = case2;//-PI / 2 * rad2deg;
  else
    angles.y = asin (sine) * rad2deg;

  angles.x = atan2 (2.0f * (r*x + y*z), (r*r - x*x - y*y + z*z)) * rad2deg;
}
