/*
    Copyright (C) 2003 by Jorrit Tyberghein

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

#ifndef __CS_IPATH_H__
#define __CS_IPATH_H__

/**\file 
 */
/**
 * \addtogroup geom_utils
 * @{ */

class csVector3;

SCF_VERSION (iPath, 0, 0, 1);

/**
 * A path in 3D.
 * An object or camera can use this object to trace a path in 3D.
 * This is particularly useful in combination with
 * csReversibleTransform::LookAt().
 */
struct iPath
{
  /// Set the position vectors (first three dimensions of the cubic spline).
  virtual void SetPositionVectors (csVector3* v) = 0;

  /// Set the up vectors (dimensions 3 to 5).
  virtual void SetUpVectors (csVector3* v) = 0;

  /// Set the forward vectors (dimensions 6 to 8).
  virtual void SetForwardVectors (csVector3* v) = 0;

  /// Set one position vector.
  virtual void SetPositionVector (int idx, const csVector3& v) = 0;

  /// Set one up vector.
  virtual void SetUpVector (int idx, const csVector3& v) = 0;

  /// Set one forward vector.
  virtual void SetForwardVector (int idx, const csVector3& v) = 0;

  /// Get one position vector.
  virtual void GetPositionVector (int idx, csVector3& v) = 0;

  /// Get one up vector.
  virtual void GetUpVector (int idx, csVector3& v) = 0;

  /// Get one forward vector.
  virtual void GetForwardVector (int idx, csVector3& v) = 0;

  /// Get the interpolated position.
  virtual void GetInterpolatedPosition (csVector3& pos) = 0;

  /// Get the interpolated up vector.
  virtual void GetInterpolatedUp (csVector3& pos) = 0;

  /// Get the interpolated forward vector.
  virtual void GetInterpolatedForward (csVector3& pos) = 0;
};


/** @} */

#endif // __CS_IPATH_H__
