/*
    Copyright (C) 1998 by Ayal Zwi Pinkus

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
#include <math.h>
#include "cssysdef.h"
#include "bezier2.h"
#include "csgeom/vector3.h"
#include "csgeom/vector2.h"

// Bezier offsets for calculating up to 10 control points
int bezier_offsets[10] =
{
  OFFSET_1,
  OFFSET_2,
  OFFSET_3,
  OFFSET_4,
  OFFSET_5,
  OFFSET_6,
  OFFSET_7,
  OFFSET_8,
  OFFSET_9,
  OFFSET_10,
};

// An array of binomial coefficients for a 2nd degree polynomail
double csBezier2:: bincoeff[3] = { 1, 2, 1 };

// This should be approx. less than 82K
double csBezier2:: bernsteinMap[LUT_SIZE];
double csBezier2:: bernsteinDuMap[LUT_SIZE];
double csBezier2:: bernsteinDvMap[LUT_SIZE];

bool csBezier2::initialized = false;

/// Evaulate the bernstien polynomial defined by the given j & k at u & v
double csBezier2::BernsteinAt (double u, int j, double v, int k)
{
  return bincoeff[j] * bincoeff[k] * pow (u, j) * pow (1 - u, 2 - j) * pow (
      v,
      k) * pow (1 - v, 2 - k);
}

/**
 * Evaluate the derivite of the Berstein polynomial defined by j & k with
 * respect to u at coordinates u, v
 */
double csBezier2::BernsteinDuAt (double u, int j, double v, int k)
{
  double left = 0;
  double right = 0;

  if (j != 0)
  {
    left = j * pow (u, j - 1) * pow (1 - u, 2 - j);
  }

  if (j != 2)
  {
    right = pow (u, j) * (2 - j) * pow (1 - u, 2 - j - 1);
  }

  return bincoeff[j] * bincoeff[k] * pow (v, k) * pow (1 - v, 2 - k)
  	* (left - right);
}

/**
 * Evaluate the derivite of the Berstein polynomial defined by j & k with
 * respect to v at coordinates u, v
 */
double csBezier2::BernsteinDvAt (double u, int j, double v, int k)
{
  double left = 0;
  double right = 0;

  if (k != 0)
  {
    left = k * pow (v, k - 1) * pow (1 - v, 2 - k);
  }

  if (k != 2)
  {
    right = pow (v, k) * (2 - k) * pow (1 - v, 2 - k - 1);
  }

  return bincoeff[j] * bincoeff[k] * pow (u, j) * pow (1 - u, 2 - j)
  	* (left - right);
}

void csBezier2::Initialize ()
{
  if (initialized) return;
  initialized = true;
  int res;
  int index = 0;
  for (res = 1; res <= 9; res++)
  {
    // Test code

    //TODO remove ? int indexshouldbe = bezier_offsets[res-1];
    int i, j, k, l;
    for (i = 0; i <= res; i++)
    {
      for (j = 0; j <= res; j++)
      {
        for (k = 0; k < 3; k++)
        {
          for (l = 0; l < 3; l++)
          {
            double u = (1.0 * i) / res, v = (1.0 * j) / res;
            bernsteinMap[index] = BernsteinAt (u, k, v, l);
            bernsteinDuMap[index] = BernsteinDuAt (u, k, v, l);
            bernsteinDvMap[index] = BernsteinDvAt (u, k, v, l);
            index++;
          }
        }
      }
    }
  }
}

csVector3 csBezier2::GetNormal (
  double **aControls,
  int u,
  int v,
  int resolution)
{
  csVector3 result;

  // Our normal is the cross product of the vector derivitives in the u & v
  // directions
  result = GetPoint (aControls, u, v, resolution, bernsteinDuMap) % GetPoint (
      aControls,
      u,
      v,
      resolution,
      bernsteinDvMap);

  result.Normalize ();

  return result;
}

csVector2 csBezier2::GetTextureCoord (
  double **aControls,
  int u,
  int v,
  int resolution,
  double *map)
{
  if (!map)
  {
    map = bernsteinMap;
  }

  int j, k;
  double *localmap = &map[bezier_offsets[resolution - 1] + 9 *
    (u + (resolution + 1) * v)];

  csVector2 result (0, 0);

  for (j = 0; j < 3; j++)
  {
    for (k = 0; k < 3; k++)
    {
      int ctrlindex = j + 3 * k;
      double *ctrl = aControls[ctrlindex];
      double fact = localmap[ctrlindex];
      result.x += ctrl[3] * fact;
      result.y += ctrl[4] * fact;
    }
  }

  return result;
}

csVector3 csBezier2::GetPoint (
  double **aControls,
  int u,
  int v,
  int resolution,
  double *map)
{
  if (!map)
  {
    map = bernsteinMap;
  }

  int j, k;
  double *localmap = &map[bezier_offsets[resolution - 1] + 9 *
    (u + (resolution + 1) * v)];

  csVector3 result (0, 0, 0);

  for (j = 0; j < 3; j++)
  {
    for (k = 0; k < 3; k++)
    {
      int ctrlindex = j + 3 * k;
      double *ctrl = aControls[ctrlindex];
      double fact = localmap[ctrlindex];
      result.x += ctrl[0] * fact;
      result.y += ctrl[1] * fact;
      result.z += ctrl[2] * fact;

      //aResult[3] += ctrl[3] * fact;
      //aResult[4] += ctrl[4] * fact;
    }
  }

  return result;
}

csVector3 csBezier2::GetPoint (
  double **aControls,
  double u,
  double v,
  double (*f) (double, int, double, int))
{
  if (!f)
  {
    f = BernsteinAt;
  }

  csVector3 result (0, 0, 0);

  int j, k;
  for (j = 0; j < 3; j++)
  {
    for (k = 0; k < 3; k++)
    {
      double *ctrl = aControls[j + 3 * k];

      result.x += ctrl[0] * f (u, j, v, k);
      result.y += ctrl[1] * f (u, j, v, k);
      result.z += ctrl[2] * f (u, j, v, k);
    }
  }

  return result;
}

csVector3 csBezier2::GetNormal (double **aControls, double u, double v)
{
  csVector3 result;

  // Our normal is the cross product of the vector derivitives in the u & v
  // directions
  result = GetPoint (aControls, u, v, BernsteinDuAt) % GetPoint (
      aControls,
      u,
      v,
      BernsteinDvAt);
  result.Normalize ();

  return result;
}
