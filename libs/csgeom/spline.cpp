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
}

csCubicSpline::~csCubicSpline ()
{
  delete[] time_points;
  delete[] points;
  delete[] derivative_points;
}

void csCubicSpline::SetTimeValues (float* t)
{
  memcpy (time_points, t, sizeof (float) * num_points);
}

void csCubicSpline::SetDimensionValues (int dim, float* d)
{
  memcpy (&points[dim*num_points], d, sizeof (float) * num_points);
  float* deriv_vals = &derivative_points[dim*num_points];
  deriv_vals[0] = 0;
  deriv_vals[1] = 0;
  int i;
  for (i = 1 ; i < num_points-1 ; i++)
  {
    float temp =
      (d[i+1]-d[i]) / (time_points[i+1]-time_points[i]) -
      (d[i]-d[i-1]) / (time_points[i]-time_points[i-1]);
    temp -=
      deriv_vals[i-1] * (time_points[i]-time_points[i-1])/6.;
    temp -=
      deriv_vals[i] * (time_points[i+1]-time_points[-1])/3.;
    temp /=
      (time_points[i+1]-time_points[i]) / 6.;
    deriv_vals[i+1] = temp;
  }
}

void csCubicSpline::Calculate (float time)
{
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
  return A*points[idx] + B*points[idx+1] +
  	 C*derivative_points[idx] + D*derivative_points[idx+1];
}

//---------------------------------------------------------------------------

