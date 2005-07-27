/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein
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

#ifndef __CS_PLANE2_H__
#define __CS_PLANE2_H__


#include "csextern.h"

#include "csgeom/segment.h"
#include "csgeom/vector2.h"

/**\file 
 */
/**
 * \addtogroup geom_utils
 * @{ */

enum 
{
  CS_POLY_IN = 1,
  CS_POLY_ON = 0,
  CS_POLY_OUT = -1
};

/**
 * A plane in 2D space.
 * The plane is given by the equation AAx + BBy + CCz + DD = 0,
 * Where (AA,BB,CC) is given by the vector 'norm'.
 */
class csPlane2
{
public:
  /// The normal vector (or the (A,B) components).
  csVector2 norm;

  /// The C component of the plane.
  float CC;

  /// Initialize to the xy plane.
  csPlane2 () : norm (0,1), CC (0) {}

  /// Initialize the plane.
  csPlane2 (const csVector2& plane_norm, float c=0) : norm (plane_norm), CC (c) {}

  /// Initialize the plane.
  csPlane2 (float a, float b, float c=0) : norm (a,b), CC (c) {}

  /// Initialize the plane given two vectors.
  inline void Set (const csVector2& v1, const csVector2& v2)
  {
    norm.x = v2.y-v1.y;
    norm.y = -(v2.x-v1.x);
    CC = - (v2 * norm);
  }

  /// Initialize the plane given a segment.
  inline void Set (const csSegment2& s)
  {
    Set (s.Start (), s.End ());
  }

  /// Initialize the plane given two vectors.
  csPlane2 (const csVector2& v1, const csVector2& v2)
  {
    Set (v1, v2);
  }

  /// Initialize the plane given a segment.
  csPlane2 (const csSegment2& s)
  {
    Set (s);
  }

  /// Return the normal vector of this plane.
  inline csVector2& Normal () { return norm; }

  /// Return the normal vector of this plane (const version).
  inline csVector2 GetNormal () const { return norm; }

  /// Return the A component of this plane.
  inline float A () const { return norm.x; }
  /// Return the B component of this plane.
  inline float B () const { return norm.y; }
  /// Return the C component of this plane.
  inline float C () const { return CC; }

  /// Return the A component of this plane.
  inline float& A () { return norm.x; }
  /// Return the B component of this plane.
  inline float& B () { return norm.y; }
  /// Return the C component of this plane.
  inline float& C () { return CC; }

  /// Set the value of the four plane components.
  inline void Set (float a, float b, float c)
  { norm.x = a; norm.y = b; CC = c; }

  /// Classify the given vector with regards to this plane.
  inline float Classify (const csVector2& pt) const { return norm*pt+CC; }

  /// Classify a vector with regards to three plane components.
  static float Classify (float A, float B, float C,
                         const csVector2& pt)
  { return A*pt.x + B*pt.y + C; }

  /**
   * Compute the distance from the given vector to this plane.
   * This function assumes that 'norm' is a unit vector.  If not, the function
   * returns distance times the magnitude of 'norm'.
   */
  inline float Distance (const csVector2& pt) const
  { return ABS (Classify (pt)); }

  /**
   * Compute the squared distance between the given vector and
   * this plane. This function works even if the plane is not
   * normalized. Note that the returned distance will be negative
   * if the point is left of the plane and positive otherwise.
   */
  inline float SquaredDistance (const csVector2& pt) const
  {
    return Classify (pt) / norm.SquaredNorm ();
  }

  /// Reverses the direction of the plane while maintianing the plane itself.
  inline void Invert () { norm = -norm;  CC = -CC; }

  /// Normalizes the plane equation so that 'norm' is a unit vector.
  inline void Normalize ()
  {
    float f = norm.Norm ();
    if (f) { norm /= f;  CC /= f; }
  }
};

/** @} */

#endif // __CS_PLANE2_H__
