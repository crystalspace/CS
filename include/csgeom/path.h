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

#include "csutil/scf.h"
#include "csgeom/spline.h"
#include "csgeom/vector3.h"
#include "igeom/path.h"

/**
 * A path in 3D.
 * An object or camera can use this object to trace a path in 3D.
 * This is particularly useful in combination with
 * csReversibleTransform::LookAt().
 */
class CS_CRYSTALSPACE_EXPORT csPath : public csCatmullRomSpline, public iPath
{
private:
  void SetVectorAsDimensionValues (int dim, csVector3* v)
  {
    int i;
    float* x, * y, * z;
    x = new float [GetPointCount ()];
    y = new float [GetPointCount ()];
    z = new float [GetPointCount ()];
    for (i = 0 ; i < GetPointCount () ; i++)
    {
      x[i] = v[i].x;
      y[i] = v[i].y;
      z[i] = v[i].z;
    }
    SetDimensionValues (dim+0, x);
    SetDimensionValues (dim+1, y);
    SetDimensionValues (dim+2, z);
    delete[] x;
    delete[] y;
    delete[] z;
  }

public:
  SCF_DECLARE_IBASE;

  /// Create a path with p points.
  csPath (int p) : csCatmullRomSpline (9, p)
  {
    SCF_CONSTRUCT_IBASE (0);
  }

  /// Destroy the path.
  virtual ~csPath () { SCF_DESTRUCT_IBASE(); }

  /// Get the number of vector points in this spline
  int Length ()
  {
    return GetPointCount();
  }
  /// Calculate
  void CalculateAtTime (float time)
  {
    Calculate (time);
  }
  int GetCurrentIndex ()
  {
    return csCatmullRomSpline::GetCurrentIndex();
  }
  float GetTime (int idx)
  {
    return GetTimeValue(idx);
  }
  void SetTime (int idx, float t)
  {
    SetTimeValue(idx,t);
  }

  /// Set the position vectors (first three dimensions of the cubic spline).
  void SetPositionVectors (csVector3* v)
  {
    SetVectorAsDimensionValues (0, v);
  }
  /// Set the up vectors (dimensions 3 to 5).
  void SetUpVectors (csVector3* v)
  {
    SetVectorAsDimensionValues (3, v);
  }
  /// Set the forward vectors (dimensions 6 to 8).
  void SetForwardVectors (csVector3* v)
  {
    SetVectorAsDimensionValues (6, v);
  }
  /// Set one position vector.
  void SetPositionVector (int idx, const csVector3& v)
  {
    SetDimensionValue (0, idx, v.x);
    SetDimensionValue (1, idx, v.y);
    SetDimensionValue (2, idx, v.z);
  }
  /// Set one up vector.
  void SetUpVector (int idx, const csVector3& v)
  {
    SetDimensionValue (3, idx, v.x);
    SetDimensionValue (4, idx, v.y);
    SetDimensionValue (5, idx, v.z);
  }
  /// Set one forward vector.
  void SetForwardVector (int idx, const csVector3& v)
  {
    SetDimensionValue (6, idx, v.x);
    SetDimensionValue (7, idx, v.y);
    SetDimensionValue (8, idx, v.z);
  }
  /// Get one position vector.
  void GetPositionVector (int idx, csVector3& v)
  {
    v.x = GetDimensionValue (0, idx);
    v.y = GetDimensionValue (1, idx);
    v.z = GetDimensionValue (2, idx);
  }
  /// Get one up vector.
  void GetUpVector (int idx, csVector3& v)
  {
    v.x = GetDimensionValue (3, idx);
    v.y = GetDimensionValue (4, idx);
    v.z = GetDimensionValue (5, idx);
  }
  /// Get one forward vector.
  void GetForwardVector (int idx, csVector3& v)
  {
    v.x = GetDimensionValue (6, idx);
    v.y = GetDimensionValue (7, idx);
    v.z = GetDimensionValue (8, idx);
  }

  /// Get the interpolated position.
  void GetInterpolatedPosition (csVector3& pos)
  {
    pos.x = GetInterpolatedDimension (0);
    pos.y = GetInterpolatedDimension (1);
    pos.z = GetInterpolatedDimension (2);
  }
  /// Get the interpolated up vector.
  void GetInterpolatedUp (csVector3& pos)
  {
    pos.x = GetInterpolatedDimension (3);
    pos.y = GetInterpolatedDimension (4);
    pos.z = GetInterpolatedDimension (5);
  }
  /// Get the interpolated forward vector.
  void GetInterpolatedForward (csVector3& pos)
  {
    pos.x = GetInterpolatedDimension (6);
    pos.y = GetInterpolatedDimension (7);
    pos.z = GetInterpolatedDimension (8);
  }
};

/** @} */

#endif // __CS_PATH_H__
