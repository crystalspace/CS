/*
    Copyright (C) 1998,1999,2000 by Jorrit Tyberghein
    Largely rewritten by Ivan Avramovic <ivan@avramovic.com>

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

#ifndef __PLANE3_H__
#define __PLANE3_H__

#ifndef __CS_CSSYSDEFS_H__
#error "cssysdef.h must be included in EVERY source file!"
#endif

#include "csgeom/vector3.h"

/**
 * A plane in 3D space.
 * The plane is given by the equation AAx + BBy + CCz + DD = 0,
 * Where (AA,BB,CC) is given by the vector 'norm'.
 */
class csPlane3
{
public:
  /// The normal vector (or the (A,B,C) components).
  csVector3 norm;

  /// The D component of the plane.
  float DD;

  /// Initialize to the xy plane.
  csPlane3 () : norm(0,0,1), DD(0) {}

  /// Initialize the plane.
  csPlane3 (const csVector3& plane_norm, float d=0) : norm(plane_norm), DD(d) {}

  /// Initialize the plane.
  csPlane3 (float a, float b, float c, float d=0) : norm(a,b,c), DD(d) {}

  /// Initialize the plane through the three given points.
  csPlane3 (const csVector3& v1, const csVector3& v2, const csVector3& v3);

  /// Return the normal vector of this plane.
  inline csVector3& Normal () { return norm; }
  /// Return the normal vector of this plane.
  inline const csVector3& Normal () const { return norm; }

  /// Return the A component of this plane.
  inline float A () const { return norm.x; }
  /// Return the B component of this plane.
  inline float B () const { return norm.y; }
  /// Return the C component of this plane.
  inline float C () const { return norm.z; }
  /// Return the D component of this plane.
  inline float D () const { return DD; }

  /// Return the A component of this plane.
  inline float& A () { return norm.x; }
  /// Return the B component of this plane.
  inline float& B () { return norm.y; }
  /// Return the C component of this plane.
  inline float& C () { return norm.z; }
  /// Return the D component of this plane.
  inline float& D () { return DD; }

  /// Set the value of the four plane components.
  inline void Set (float a, float b, float c, float d)
  { norm.x = a; norm.y = b; norm.z = c; DD = d; }

  /// Same but takes a vector directly.
  inline void Set (const csVector3& normal, float d)
  { norm = normal; DD = d; }

  /// Initialize the plane through the three given points.
  void Set (const csVector3& v1, const csVector3& v2, const csVector3& v3);

  /// Classify the given vector with regards to this plane.
  inline float Classify (const csVector3& pt) const { return norm*pt+DD; }

  /// Classify a vector with regards to four plane components.
  static float Classify (float A, float B, float C, float D,
                         const csVector3& pt)
  { return A*pt.x + B*pt.y + C*pt.z + D; }

  /**
   * Compute the distance from the given vector to this plane.
   * This function assumes that 'norm' is a unit vector.  If not, the function
   * returns distance times the magnitude of 'norm'.
   */
  inline float Distance (const csVector3& pt) const
  { return ABS (Classify (pt)); }

  /// Reverses the direction of the plane while maintianing the plane itself.
  void Invert() { norm = -norm;  DD = -DD; }

  /// Normalizes the plane equation so that 'norm' is a unit vector.
  void Normalize()
  {
    float f = norm.Norm();
    if (f) { norm /= f;  DD /= f; }
  }
};

#endif // __PLANE3_H__
