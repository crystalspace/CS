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
#include "csgeom/quaternion.h"

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

csQuaternion::csQuaternion(const csMatrix3& matrix)
{
      float mat[3][3] = { { matrix.m11, matrix.m21, matrix.m31 },
                          { matrix.m12, matrix.m22, matrix.m32 },
                          { matrix.m13, matrix.m23, matrix.m33 } };
      float qx, qy, qz, qw;
      // Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
      // article "Quaternion Calculus and Fast Animation".

      float fTrace = mat[0][0] + mat[1][1] + mat[2][2];
      float fRoot;

      if( fTrace > 0.0 )
      {
        fRoot = sqrtf( fTrace + 1.0f );

        qw = 0.5f * fRoot;

        fRoot = 0.5f / fRoot;

        qx = ( mat[2][1] - mat[1][2] ) * fRoot;
        qy = ( mat[0][2] - mat[2][0] ) * fRoot;
        qz = ( mat[1][0] - mat[0][1] ) * fRoot;
      }
      else
      {
        int iNext[3] = { 1, 2, 0 };

        int i = 0;
        if( mat[1][1] > mat[0][0] )
          i = 1;

        if( mat[2][2] > mat[i][i] )
          i = 2;

        int j = iNext[i];
        int k = iNext[j];

        fRoot = sqrtf( mat[i][i] - mat[j][j] - mat[k][k] + 1.0f );

        float *apfQuat[3] = { &qx, &qy, &qz };

        *(apfQuat[i]) = 0.5f * fRoot;

        fRoot = 0.5f / fRoot;

        qw = ( mat[k][j] - mat[j][k] ) * fRoot;

        *(apfQuat[j]) = ( mat[j][i] + mat[i][j] ) * fRoot;
        *(apfQuat[k]) = ( mat[k][i] + mat[i][k] ) * fRoot;
      }

	  x = qx;
	  y = qy;
	  z = qz;
	  r = qw;
      Normalize();
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

void csQuaternion::GetEulerAngles (csVector3& angles, bool radians)
{
  const float rad2deg = radians ? 1.0f : 180.0f / PI;
  const float case1 = PI / 2.0f * 180.0f / rad2deg;
  const float case2 = -PI / 2.0f * rad2deg;

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
