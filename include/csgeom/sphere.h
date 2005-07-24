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

#ifndef __CS_SPHERE_H__
#define __CS_SPHERE_H__

/**\file 
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"

#include "cstypes.h"
#include "vector3.h"

class csTransform;

/**
 * This class represents a sphere.
 */
class CS_CRYSTALSPACE_EXPORT csSphere
{
private:
  csVector3 center;
  float radius;

public:
  /// Create a new empty sphere at (0,0,0).
  csSphere ()
  {
    center.Set (0, 0, 0);
    radius = 0;
  }

  /// Create a new sphere.
  csSphere (const csVector3& center, float radius)
  {
    csSphere::center = center;
    csSphere::radius = radius;
  }

  /// Copy Constructor.
  csSphere (const csSphere& s) { center = s.center; radius = s.radius; }

  /// Get the center of this sphere.
  csVector3& GetCenter () { return center; }
  /// Get the center of this sphere.
  const csVector3& GetCenter () const { return center; }
  /// Set the center of this sphere.
  void SetCenter (const csVector3& c) { center = c; }
  /// Get the radius of this sphere.
  float GetRadius () const { return radius; }
  /// Set the radius of this sphere.
  void SetRadius (float r) { radius = r; }

  /// Calculate the union of this sphere and another.
  void Union (const csVector3& ocenter, float oradius);

  /// Calculate the union of two spheres.
  friend csSphere operator+ (const csSphere& s1, const csSphere& s2);
  /// Calculate the union of this sphere and another one.
  csSphere& operator+= (const csSphere& s)
  {
    Union (s.center, s.radius);
    return *this;
  }
};

/**
 * This class represents an ellipsoid.
 */
class CS_CRYSTALSPACE_EXPORT csEllipsoid
{
private:
  csVector3 center;
  csVector3 radius;

public:
  /// Create a new empty ellipsoid at (0,0,0).
  csEllipsoid ()
  {
    center.Set (0, 0, 0);
    radius.Set (0, 0, 0);
  }

  /// Create a new ellipsoid.
  csEllipsoid (const csVector3& center, const csVector3& radius)
  {
    csEllipsoid::center = center;
    csEllipsoid::radius = radius;
  }

  /// Copy Constructor.
  csEllipsoid (const csEllipsoid& s) { center = s.center; radius = s.radius; }

  /// Get the center of this ellipsoid.
  csVector3& GetCenter () { return center; }
  /// Get the center of this ellipsoid.
  const csVector3& GetCenter () const { return center; }
  /// Set the center of this ellipsoid.
  void SetCenter (const csVector3& c) { center = c; }
  /// Get the radius of this ellipsoid.
  csVector3& GetRadius () { return radius; }
  /// Get the radius of this ellipsoid.
  const csVector3& GetRadius () const { return radius; }
  /// Set the radius of this ellipsoid.
  void SetRadius (const csVector3& r) { radius = r; }
};

/** @} */

#endif // __CS_SPHERE_H__

