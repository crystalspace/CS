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

csCubicSpline::csCubicSpline (int d, int p) : dimensions (d), num_points (p)
{
  time_points = new float [p];
  points = new float [d*p];
  derivative_points = new float [d*p];
  derivatives_valid = false;
}

csCubicSpline::~csCubicSpline ()
{
  delete[] time_points;
  delete[] points;
  delete[] derivative_points;
}

void csCubicSpline::PrecalculateDerivatives (int dim)
{
  float* d = &points[dim*num_points];
  float* d2 = &derivative_points[dim*num_points];
  float* t = time_points;
  int i;

#if 0
  // Calculate d1.
  float d1[10000];//@@@
  d1[0] = (d[1]-d[0]) / (t[1]-t[0]);
  for (i = 1 ; i < num_points-1 ; i++)
    d1[i] = (d[i+1]-d[i-1]) / (t[i+1]-t[i-1]);
  d1[num_points-1] = 0;
  // Calculate d2.
  d2[0] = (d1[1]-d1[0]) / (t[1]-t[0]);
  for (i = 1 ; i < num_points-1 ; i++)
    d2[i] = (d1[i+1]-d2[i-1]) / (t[i+1]-t[i-1]);
  d2[num_points-1] = d2[num_points-2];
#endif

#if 1
  d2[0] = 0;
  d2[1] = (d[2]-d[1]) / (time_points[2]-time_points[1]) -
  	(d[1]-d[0])/(time_points[1]-time_points[0]);
  for (i = 1 ; i < num_points-1 ; i++)
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

    d2[i+1] = (d[i+2]-d[i+1]) / (t[i+2]-t[i+1]) - (d[i+1]-d[i])/(t[i+1]-t[i]);
    d2[i+1] *= 3.0 / (t[i+2]-t[i]);
  }
#endif
}

void csCubicSpline::PrecalculateDerivatives ()
{
  if (derivatives_valid) return;
  derivatives_valid = true;
  int dim;
  for (dim = 0 ; dim < dimensions ; dim++)
    PrecalculateDerivatives (dim);
}

void csCubicSpline::SetTimeValues (float* t)
{
  memcpy (time_points, t, sizeof (float) * num_points);
  derivatives_valid = false;
}

void csCubicSpline::SetDimensionValues (int dim, float* d)
{
  memcpy (&points[dim*num_points], d, sizeof (float) * num_points);
  derivatives_valid = false;
}

void csCubicSpline::Calculate (float time)
{
  PrecalculateDerivatives ();
  
  // First find the current 'idx'.
  for (idx = 0 ; idx < num_points-1 ; idx++)
  {
    if (time >= time_points[idx] && time <= time_points[idx+1])
      break;
  }

  A = (time_points[idx+1] - time) / (time_points[idx+1] - time_points[idx]);
  B = 1-A;
  float temp = (time_points[idx+1] - time_points[idx]);
  temp = temp * temp / 6.;
  C = (A*A*A - A) * temp;
  D = (B*B*B - B) * temp;
}

float csCubicSpline::GetInterpolatedDimension (int dim)
{
  //@@@ TEMPORARY
  float* p = &points[dim*num_points];
#if 0
  return A*p[idx] + B*p[idx+1];
#else
  float* dp = &derivative_points[dim*num_points];
  return A*p[idx] + B*p[idx+1] +
  	 C*dp[idx] + D*dp[idx+1];
#endif
}

//---------------------------------------------------------------------------

