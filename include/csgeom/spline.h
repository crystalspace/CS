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

#ifndef __SPLINE_H__
#define __SPLINE_H__

/**
 * A cubic spline.
 * This spline can control several dimensions at once.
 */
class csCubicSpline
{
private:
  int dimensions;
  int num_points;
  float* time_points;
  float* points;
  bool derivatives_valid;
  float* derivative_points;

  // The following values are calculated by Calculate() and
  // are used later by GetInterpolatedDimension().
  int idx;
  float A, B, C, D;	// For computation of a spline value.

private:
  void PrecalculateDerivatives (int dim);
  void PrecalculateDerivatives ();

public:
  /// Create a cubic spline with d dimensions and p points.
  csCubicSpline (int d, int p);

  /// Destroy the spline.
  ~csCubicSpline ();

  /// Get the number of dimensions.
  int GetNumDimensions () { return dimensions; }

  /// Get the number of points.
  int GetNumPoints () { return num_points; }

  /**
   * Set the time values. 't' should point to an array containing
   * 'num_points' values. These values typically start with 0 and end
   * with 1. Other values are also possible the but the values should
   * rise. The given array is copied.
   */
  void SetTimeValues (float* t);

  /**
   * Get the time values.
   */
  float* GetTimeValues () { return time_points; }

  /**
   * Set the values for some dimension.
   * 'd' should point to an array containing 'num_points' values.
   * These are the values that will be interpolated. The given
   * array is copied.
   */
  void SetDimensionValues (int dim, float* d);

  /**
   * Get the values for some dimension.
   */
  float* GetDimensionValues (int dim) { return &points[dim*num_points]; }

  /**
   * Calculate internal values for this spline given some time value.
   */
  void Calculate (float time);

  /**
   * After calling Calculate() you can use this to fetch the value of
   * some dimension.
   */
  float GetInterpolatedDimension (int dim);
};

#endif // __SPLINE_H__
