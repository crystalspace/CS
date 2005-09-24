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

#ifndef __CS_SPLINE_H__
#define __CS_SPLINE_H__

/**\file 
 * Splines.
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"

/**
 * A spline superclass.
 * This spline can control several dimensions at once.
 */
class CS_CRYSTALSPACE_EXPORT csSpline
{
protected:
  int dimensions;
  int num_points;
  float* time_points;
  float* points;
  bool precalculation_valid;
  int idx;

public:
  /// Create a spline with d dimensions and p points.
  csSpline (int d, int p);

  /// Destroy the spline.
  virtual ~csSpline ();

  /// Get the number of dimensions.
  int GetDimensionCount () const { return dimensions; }

  /// Get the number of points.
  int GetPointCount () const { return num_points; }

  /**
   * Insert a point after some index. If index == -1 add a point before all
   * others.
   */
  void InsertPoint (int idx);

  /**
   * Remove a point at the index.
   */
  void RemovePoint (int idx);

  /**
   * Set the time values. 't' should point to an array containing
   * 'num_points' values. These values typically start with 0 and end
   * with 1. Other values are also possible the but the values should
   * rise. The given array is copied.
   */
  void SetTimeValues (float const* t);

  /**
   * Set one time point.
   */
  void SetTimeValue (int idx, float t);

  /**
   * Get the time values.
   */
  float const* GetTimeValues () const { return time_points; }

  /**
   * Get one time point.
   */
  float GetTimeValue (int idx) const { return GetTimeValues ()[idx]; }

  /**
   * Set the values for some dimension.
   * 'd' should point to an array containing 'num_points' values.
   * These are the values that will be interpolated. The given
   * array is copied.
   */
  void SetDimensionValues (int dim, float const* d);

  /**
   * Set a value for some dimension.
   */
  void SetDimensionValue (int dim, int idx, float d);

  /**
   * Get the values for some dimension.
   */
  float const* GetDimensionValues (int dim) const
  { return &points[dim*num_points]; }

  /**
   * Get the value for some dimension.
   */
  float GetDimensionValue (int dim, int idx) const
  {
    float const* d = &points[dim*num_points];
    return d[idx];
  }

  /**
   * Sets the values of the control point at index idx.
   */
  void SetIndexValues (int idx, float const* d);
  
  /**
   * Gets the values of the control point at index idx.  Caller is responsible
   * for invoking `delete[]' on returned array when no longer needed.
   */
  float* GetIndexValues (int idx) const;
  
  /**
   * Calculate internal values for this spline given some time value.
   */
  virtual void Calculate (float time) = 0;

  /**
   * Get the index of the current point we are in (valid after Calculate()).
   */
  int GetCurrentIndex () const { return idx; }

  /**
   * After calling Calculate() you can use this to fetch the value of
   * some dimension.
   */
  virtual float GetInterpolatedDimension (int dim) const = 0;
};

/**
 * A cubic spline.
 */
class CS_CRYSTALSPACE_EXPORT csCubicSpline : public csSpline
{
private:
  bool derivatives_valid;
  float* derivative_points;

  // The following values are calculated by Calculate() and
  // are used later by GetInterpolatedDimension().
  float A, B, C, D;	// For computation of a spline value.

private:
  void PrecalculateDerivatives (int dim);
  void PrecalculateDerivatives ();

public:
  /// Create a cubic spline with d dimensions and p points.
  csCubicSpline (int d, int p);

  /// Destroy the spline.
  virtual ~csCubicSpline ();

  /**
   * Calculate internal values for this spline given some time value.
   */
  virtual void Calculate (float time);

  /**
   * After calling Calculate() you can use this to fetch the value of
   * some dimension.
   */
  virtual float GetInterpolatedDimension (int dim) const;
};

/**
 * A B-spline.
 */
class CS_CRYSTALSPACE_EXPORT csBSpline : public csSpline
{
private:
  // The following values are calculated by Calculate() and
  // are used later by GetInterpolatedDimension().
  float t;

protected:
  /// Base function for a cubic B-spline (i=-2..1).
  virtual float BaseFunction (int i, float t) const;

public:
  /// Create a B-spline with d dimensions and p points.
  csBSpline (int d, int p);

  /// Destroy the spline.
  virtual ~csBSpline ();

  /**
   * Calculate internal values for this spline given some time value.
   */
  virtual void Calculate (float time);

  /**
   * After calling Calculate() you can use this to fetch the value of
   * some dimension.
   */
  virtual float GetInterpolatedDimension (int dim) const;
};

/**
 * A CatmullRom spline.
 */
class CS_CRYSTALSPACE_EXPORT csCatmullRomSpline : public csBSpline
{
protected:
  /// Base function for a cubic CatmullRom spline (i=-2..1).
  virtual float BaseFunction (int i, float t) const;

public:
  /// Create a CatmullRom spline with d dimensions and p points.
  csCatmullRomSpline (int d, int p) : csBSpline (d, p) { }

  /// Destroy the spline.
  virtual ~csCatmullRomSpline () {}

  /// Clones the spline
  csCatmullRomSpline * Clone();

};

/** @} */

#endif // __CS_SPLINE_H__
