/*
    Copyright (C) 2001 by Jorrit Tyberghein

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
#include "csgeom/spline.h"

//---------------------------------------------------------------------------
csSpline::csSpline (int d, int p) :
  dimensions(d),
  num_points(p)
{
  time_points = new float[p];
  points = new float[d * p];
  precalculation_valid = false;
}

csSpline::~csSpline ()
{
  delete[] time_points;
  delete[] points;
}

static void InsertFloat (float *dst, float const* src, int idx, int num)
{
  if (idx == -1)
  {
    memcpy (dst + 1, src, num * sizeof (float));
  }
  else if (idx >= num - 1)
  {
    memcpy (dst, src, num * sizeof (float));
  }
  else
  {
    memcpy (dst, src, (idx + 1) * sizeof (float));
    memcpy (dst + idx + 2, src + idx + 1, (num - idx - 1) * sizeof (float));
  }
}

void csSpline::InsertPoint (int idx)
{
  float *time_points2 = new float[num_points + 1];
  float *points2 = new float[dimensions * (num_points + 1)];
  InsertFloat (time_points2, time_points, idx, num_points);

  int dim;
  for (dim = 0; dim < dimensions; dim++)
  {
    float *d2 = &points2[dim * (num_points + 1)];
    float *d = &points[dim * num_points];
    InsertFloat (d2, d, idx, num_points);
  }

  delete[] time_points;
  time_points = time_points2;
  delete[] points;
  points = points2;
  num_points++;
  precalculation_valid = false;
}

static void RemoveFloat (float *dst, float const* src, int idx, int num)
{
  if (idx <= 0)
  {
    memcpy (dst, src + 1, (num - 1) * sizeof (float));
  }
  else if (idx < num - 1)
  {
    memcpy (dst, src, idx * sizeof (float));
    memcpy (dst + idx, src + idx + 1, (num - idx - 1) * sizeof (float));
  }
}

void csSpline::RemovePoint (int idx)
{
  float *time_points2 = new float[num_points - 1];
  float *points2 = new float[dimensions * (num_points - 1)];
  RemoveFloat (time_points2, time_points, idx, num_points);

  int dim;
  for (dim = 0; dim < dimensions; dim++)
  {
    float *d2 = &points2[dim * (num_points - 1)];
    float *d = &points[dim * num_points];
    RemoveFloat (d2, d, idx, num_points);
  }

  delete[] time_points;
  time_points = time_points2;
  delete[] points;
  points = points2;
  num_points--;
  precalculation_valid = false;
}

void csSpline::SetTimeValues (float const* t)
{
  memcpy (time_points, t, sizeof (float) * num_points);
  precalculation_valid = false;
}

void csSpline::SetTimeValue (int idx, float t)
{
  time_points[idx] = t;
  precalculation_valid = false;
}

void csSpline::SetDimensionValues (int dim, float const* d)
{
  memcpy (&points[dim * num_points], d, sizeof (float) * num_points);
  precalculation_valid = false;
}

void csSpline::SetDimensionValue (int dim, int idx, float d)
{
  float *p = &points[dim * num_points];
  p[idx] = d;
  precalculation_valid = false;
}

void csSpline::SetIndexValues (int idx, float const* d)
{
  for (int a = 0, n = GetDimensionCount(); a < n; ++a)
    points[a * num_points + idx] = d[a];
  precalculation_valid = false;
}

float* csSpline::GetIndexValues (int idx) const
{
  float* p = new float[GetDimensionCount()];
  for (int a = 0, n = GetDimensionCount(); a < n; ++a)
    p[a] = points[a * num_points + idx];
  return p;
}

//---------------------------------------------------------------------------
csCubicSpline::csCubicSpline (int d, int p) :
  csSpline(d, p)
{
  derivative_points = new float[d * p];
  precalculation_valid = false;
}

csCubicSpline::~csCubicSpline ()
{
  delete[] derivative_points;
}

void csCubicSpline::PrecalculateDerivatives (int dim)
{
  float *d = &points[dim * num_points];
  float *d2 = &derivative_points[dim * num_points];
  float *t = time_points;
  int i;

#if 0
  // Calculate d1.
  float d1[10000];  //@@@
  d1[0] = (d[1] - d[0]) / (t[1] - t[0]);
  for (i = 1; i < num_points - 1; i++)
    d1[i] = (d[i + 1] - d[i - 1]) / (t[i + 1] - t[i - 1]);
  d1[num_points - 1] = 0;

  // Calculate d2.
  d2[0] = (d1[1] - d1[0]) / (t[1] - t[0]);
  for (i = 1; i < num_points - 1; i++)
    d2[i] = (d1[i + 1] - d2[i - 1]) / (t[i + 1] - t[i - 1]);
  d2[num_points - 1] = d2[num_points - 2];
#endif
#if 1
  d2[0] = d2[num_points - 1] = 0;
  d2[1] = (d[2] - d[1]) / (time_points[2] - time_points[1]) - (d[1] - d[0]) / (time_points[1] - time_points[0]);
  for (i = 1; i < num_points - 2; i++)
  {
    //float temp =
    //(d[i+1]-d[i]) / (time_points[i+1]-time_points[i]) -
    //(d[i]-d[i-1]) / (time_points[i]-time_points[i-1]);
    //temp -=
    //d2[i-1] * (time_points[i]-time_points[i-1])/6.;
    //temp -=
    //d2[i] * (time_points[i+1]-time_points[i-1])/3.;
    //temp /=
    //(time_points[i+1]-time_points[i]) / 6.;

    //d2[i+1] = temp;
    d2[i + 1] = (d[i + 2] - d[i + 1]) / (t[i + 2] - t[i + 1])
    	- (d[i + 1] - d[i]) / (t[i + 1] - t[i]);
    d2[i + 1] *= 3.0f / (t[i + 2] - t[i]);
  }
#endif
}

void csCubicSpline::PrecalculateDerivatives ()
{
  if (precalculation_valid) return ;
  precalculation_valid = true;
  delete[] derivative_points;
  derivative_points = new float[dimensions * num_points];

  int dim;
  for (dim = 0; dim < dimensions; dim++) PrecalculateDerivatives (dim);
}

void csCubicSpline::Calculate (float time)
{
  PrecalculateDerivatives ();

  // First find the current 'idx'.
  for (idx = 0; idx < num_points - 1; idx++)
  {
    if (time >= time_points[idx] && time <= time_points[idx + 1]) break;
  }

  A = (time_points[idx + 1] - time) / (time_points[idx + 1] - time_points[idx]);
  B = 1 - A;

  float temp = (time_points[idx + 1] - time_points[idx]);
  temp = temp * temp / 6.0f;
  C = (A * A * A - A) * temp;
  D = (B * B * B - B) * temp;
}

float csCubicSpline::GetInterpolatedDimension (int dim) const
{
  float *p = &points[dim * num_points];
#if 0
  //@@@ TEMPORARY
  return A * p[idx] + B * p[idx + 1];
#else
  float *dp = &derivative_points[dim * num_points];
  return A * p[idx] + B * p[idx + 1] + C * dp[idx] + D * dp[idx + 1];
#endif
}

//---------------------------------------------------------------------------
csBSpline::csBSpline (int d, int p) :
  csSpline(d, p)
{
}

csBSpline::~csBSpline ()
{
}

float csBSpline::BaseFunction (int i, float t) const
{
  switch (i)
  {
    case -2:  return (((-t + 3) * t - 3) * t + 1) / 6;
    case -1:  return (((3 * t - 6) * t) * t + 4) / 6;
    case 0:   return (((-3 * t + 3) * t + 3) * t + 1) / 6;
    case 1:   return (t * t * t) / 6;
  }

  return 0;         // We only get here if an invalid i is specified.
}

void csBSpline::Calculate (float time)
{
  // First find the current 'idx'.
  for (idx = 0; idx < num_points - 1; idx++)
  {
    if (time >= time_points[idx] && time <= time_points[idx + 1]) break;
  }

  t = 1.0f - (time_points[idx + 1] - time)
  	/ (time_points[idx + 1] - time_points[idx]);
}

float csBSpline::GetInterpolatedDimension (int dim) const
{
  float *p = &points[dim * num_points];
  float val = 0;
  int j;
  for (j = -2; j <= 1; j++)
  {
    // @@@ Not very efficient but it will do for now...
    // We would need to cache p[-1] and p[-2]
    float pp;
    int id = idx + j + 1;
    if (id == -1)
      pp = p[0] - (p[1] - p[0]);
    else if (id == -2)
      pp = p[0] - 2 * (p[1] - p[0]);
    else if (id == num_points)
      pp = p[num_points - 1] - (p[num_points - 2] - p[num_points - 1]);
    else
      pp = p[id];
    val += BaseFunction (j, t) * pp;
  }

  return val;
}

//---------------------------------------------------------------------------
float csCatmullRomSpline::BaseFunction (int i, float t) const
{
  switch (i)
  {
    case -2:  return ((-t + 2) * t - 1) * t / 2;
    case -1:  return (((3 * t - 5) * t) * t + 2) / 2;
    case 0:   return ((-3 * t + 4) * t + 1) * t / 2;
    case 1:   return ((t - 1) * t * t) / 2;
  }

  return 0;         // We only get here if an invalid i is specified.
}
