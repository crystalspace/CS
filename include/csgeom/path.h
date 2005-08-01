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

#ifndef __CS_PATH_H__
#define __CS_PATH_H__


/**\file 
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"

#include "csgeom/spline.h"
#include "csgeom/vector3.h"
#include "csutil/scf_implementation.h"

#include "igeom/path.h"

/**
 * A path in 3D.
 * An object or camera can use this object to trace a path in 3D.
 * This is particularly useful in combination with
 * csReversibleTransform::LookAt().
 */
class CS_CRYSTALSPACE_EXPORT csPath :
  public scfImplementation1<csPath, iPath>
{
protected:
  csCatmullRomSpline spline;

private:
  void SetVectorAsDimensionValues (int dim, csVector3* v)
  {
    int i;
    int N = spline.GetPointCount();
    float* x, * y, * z;
    x = new float [N];
    y = new float [N];
    z = new float [N];
    for (i = 0 ; i < N ; i++)
    {
      x[i] = v[i].x;
      y[i] = v[i].y;
      z[i] = v[i].z;
    }
    spline.SetDimensionValues (dim+0, x);
    spline.SetDimensionValues (dim+1, y);
    spline.SetDimensionValues (dim+2, z);
    delete[] x;
    delete[] y;
    delete[] z;
  }

public:

  /// Create a path with p points.
  csPath (int p) : scfImplementationType(this), spline (9, p)
  { }

  /// Destroy the path.
  virtual ~csPath ()
  { }

  /// Get the number of vector points in this spline
  virtual int Length ()
  {
    return spline.GetPointCount();
  }
  /**
   * Get the number of vector points in this spline.
   * \deprecated Use Length() instead.
   */
  int GetPointCount()
  {
    return Length();
  }
  /// Calculate internal values for this spline given some time value.
  virtual void CalculateAtTime (float time)
  {
    spline.Calculate (time);
  }
  /**
   * Calculate internal values for this spline given some time value.
   * \deprecated Use CalculateAtTime() instead.
   */
  virtual void Calculate (float time)
  {
    CalculateAtTime(time);
  }
  /// Get current index.
  virtual int GetCurrentIndex ()
  {
    return spline.GetCurrentIndex();
  }
  /// Get one time point.
  virtual float GetTime (int idx)
  {
    return spline.GetTimeValue(idx);
  }
  /**
   * Get one time point.
   * \deprecated Use GetTime() instead.
   */
  float GetTimeValue (int idx) const
  {
    return spline.GetTimeValue(idx);
  }
  /// Set one time point.
  virtual void SetTime (int idx, float t)
  {
    spline.SetTimeValue(idx,t);
  }
  /**
   * Set one time point.
   * \deprecated Use SetTime() instead.
   */
  virtual void SetTimeValue (int idx, float t)
  {
    SetTime(idx, t);
  }
  /**
   * Set the time values. 't' should point to an array containing
   * 'num_points' values. These values typically start with 0 and end
   * with 1. Other values are also possible the but the values should
   * rise. The given array is copied.
   */
  void SetTimes (float const* t)
  {
    spline.SetTimeValues(t);
  }
  /**
   * Set the time values.
   * \deprecated Use SetTimes() instead.
   */
  void SetTimeValues (float const* t)
  {
    SetTimes(t);
  }
  /// Get the time values.
  float const* GetTimes () const
  {
    return spline.GetTimeValues();
  }
  /**
   * Get the time values.
   * \deprecated Use GetTimes() instead.
   */
  float const* GetTimeValues () const
  {
    return GetTimes();
  }
  /// Set the position vectors (first three dimensions of the cubic spline).
  virtual void SetPositionVectors (csVector3* v)
  {
    SetVectorAsDimensionValues (0, v);
  }
  /// Set the up vectors (dimensions 3 to 5).
  virtual void SetUpVectors (csVector3* v)
  {
    SetVectorAsDimensionValues (3, v);
  }
  /// Set the forward vectors (dimensions 6 to 8).
  virtual void SetForwardVectors (csVector3* v)
  {
    SetVectorAsDimensionValues (6, v);
  }
  /// Set one position vector.
  virtual void SetPositionVector (int idx, const csVector3& v)
  {
    spline.SetDimensionValue (0, idx, v.x);
    spline.SetDimensionValue (1, idx, v.y);
    spline.SetDimensionValue (2, idx, v.z);
  }
  /// Set one up vector.
  virtual void SetUpVector (int idx, const csVector3& v)
  {
    spline.SetDimensionValue (3, idx, v.x);
    spline.SetDimensionValue (4, idx, v.y);
    spline.SetDimensionValue (5, idx, v.z);
  }
  /// Set one forward vector.
  virtual void SetForwardVector (int idx, const csVector3& v)
  {
    spline.SetDimensionValue (6, idx, v.x);
    spline.SetDimensionValue (7, idx, v.y);
    spline.SetDimensionValue (8, idx, v.z);
  }
  /// Get one position vector.
  virtual void GetPositionVector (int idx, csVector3& v)
  {
    v.x = spline.GetDimensionValue (0, idx);
    v.y = spline.GetDimensionValue (1, idx);
    v.z = spline.GetDimensionValue (2, idx);
  }
  /// Get one up vector.
  virtual void GetUpVector (int idx, csVector3& v)
  {
    v.x = spline.GetDimensionValue (3, idx);
    v.y = spline.GetDimensionValue (4, idx);
    v.z = spline.GetDimensionValue (5, idx);
  }
  /// Get one forward vector.
  virtual void GetForwardVector (int idx, csVector3& v)
  {
    v.x = spline.GetDimensionValue (6, idx);
    v.y = spline.GetDimensionValue (7, idx);
    v.z = spline.GetDimensionValue (8, idx);
  }

  /// Get the interpolated position.
  virtual void GetInterpolatedPosition (csVector3& pos)
  {
    pos.x = spline.GetInterpolatedDimension (0);
    pos.y = spline.GetInterpolatedDimension (1);
    pos.z = spline.GetInterpolatedDimension (2);
  }
  /// Get the interpolated up vector.
  virtual void GetInterpolatedUp (csVector3& pos)
  {
    pos.x = spline.GetInterpolatedDimension (3);
    pos.y = spline.GetInterpolatedDimension (4);
    pos.z = spline.GetInterpolatedDimension (5);
  }
  /// Get the interpolated forward vector.
  virtual void GetInterpolatedForward (csVector3& pos)
  {
    pos.x = spline.GetInterpolatedDimension (6);
    pos.y = spline.GetInterpolatedDimension (7);
    pos.z = spline.GetInterpolatedDimension (8);
  }
  /// Get the values for some dimension.
  float const* GetDimensionValues (int dim) const
  {
    return spline.GetDimensionValues(dim);
  }
  /// Get the value for some dimension.
  float GetDimensionValue (int dim, int idx) const
  {
    return spline.GetDimensionValue(dim, idx);
  }
  /**
   * Insert a point after some index. If index == -1 add a point before all
   * others.
   */
  void InsertPoint (int idx)
  {
    spline.InsertPoint(idx);
  }
  /// Remove a point at the index.
  void RemovePoint (int idx)
  {
    spline.RemovePoint(idx);
  }
};

/** @} */

#endif // __CS_PATH_H__
