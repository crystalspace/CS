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

#ifndef __CS_PLANE3_H__
#define __CS_PLANE3_H__

/**\file 
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"

#include "csgeom/vector3.h"

class csString;

/**
 * A plane in 3D space.
 * The plane is given by the equation AAx + BBy + CCz + DD = 0,
 * Where (AA,BB,CC) is given by the vector 'norm'.
 */
class CS_CRYSTALSPACE_EXPORT csPlane3
{
public:
  /// The normal vector (or the (A,B,C) components).
  csVector3 norm;

  /// The D component of the plane.
  float DD;

  /**
   * Initialize to the xy plane (0,0,1,0).
   */
  csPlane3 () : norm(0,0,1), DD(0) {}

  /**
   * Initialize the plane with the given norm and D component.
   */
  csPlane3 (const csVector3& plane_norm, float d=0) : norm(plane_norm), DD(d) {}

  /**
   * Initialize the plane to the given components.
   */
  csPlane3 (float a, float b, float c, float d=0) : norm(a,b,c), DD(d) {}

  /**
   * Initialize the plane through the three given points. If the plane
   * is expressed as (N,D) with N the A,B,C components of the plane then
   * this will initialize the plane to (N',-N'*v1) with N' equal
   * to (v1-v2)%(v1-v3).
   */
  csPlane3 (const csVector3& v1, const csVector3& v2, const csVector3& v3);

  /**
   * Initialize the plane through 0 and the two given points. If the plane
   * is expressed as (N,D) with N the A,B,C components of the plane then
   * this will initialize the plane to (v2%v3,0).
   */
  csPlane3 (const csVector3& v2, const csVector3& v3)
  {
    norm = v2 % v3; DD = 0;
  }

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

  /// Return the normal of this plane.
  inline const csVector3& GetNormal () const { return norm; }

  /// Set the value of the four plane components.
  inline void Set (float a, float b, float c, float d)
  { norm.x = a; norm.y = b; norm.z = c; DD = d; }

  /// Set the value of the plane using a normal and D component.
  inline void Set (const csVector3& normal, float d)
  { norm = normal; DD = d; }

  /**
   * Initialize the plane through the three given points. If the plane
   * is expressed as (N,D) with N the A,B,C components of the plane then
   * this will initialize the plane to (N',-N'*v1) with N' equal
   * to (v1-v2)%(v1-v3).
   */
  void Set (const csVector3& v1, const csVector3& v2, const csVector3& v3);

  /**
   * Initialize the plane through 0 and the two given points. If the plane
   * is expressed as (N,D) with N the A,B,C components of the plane then
   * this will initialize the plane to (v2%v3,0).
   */
  inline void Set (const csVector3& v2, const csVector3& v3)
  {
    norm = v2 % v3; DD = 0;
  }

  /**
   * Set one point ("origin") through which the plane goes.
   * This is equal to setting DD = -N'*p where N' is the normal
   */
  inline void SetOrigin (const csVector3& p)
  {
    DD = -norm * p;
  }

  /**
   * Classify the given vector with regards to this plane. If the plane
   * is expressed as (N,D) with N the A,B,C components of the plane then
   * this will calculate and return N*pt+D. Note that in the Crystal Space
   * engine this function will return negative if used on the visible
   * side of a polygon. i.e. if you take the world space plane of the polygon,
   * then Classify() will return a negative value if the camera is located
   * at a point from which you can see the polygon. Back-face culling
   * will make the polygon invisible on the other side.
   */
  inline float Classify (const csVector3& pt) const { return norm*pt+DD; }

  /**
   * This static function classifies a vector with regards to four given plane
   * components. This will calculate and return A*pt.x+B*pt.y+C*pt.z+D.
   */
  static float Classify (float A, float B, float C, float D,
                         const csVector3& pt)
  {
    return A*pt.x + B*pt.y + C*pt.z + D;
  }

  /**
   * Compute the distance from the given vector to this plane.
   * This function assumes that 'norm' is a unit vector.  If not, the function
   * returns distance times the magnitude of 'norm'. This function corresponds
   * exactly to the absolute value of Classify().
   */
  inline float Distance (const csVector3& pt) const
  { return ABS (Classify (pt)); }

  /**
   * Reverses the direction of the plane while maintianing the plane itself.
   * This will basically reverse the result of Classify().
   */
  inline void Invert () { norm = -norm;  DD = -DD; }

  /**
   * Normalizes the plane equation so that 'norm' is a unit vector.
   */
  inline void Normalize ()
  {
    float f = norm.Norm ();
    if (f) { norm /= f;  DD /= f; }
  }

  /**
   * Find a point on this plane.
   */
  csVector3 FindPoint () const;

  /**
   * Clip the polygon in pverts (having num_verts vertices) to this plane.
   * The vertices are expected in camera space.
   * Method returns true if there is something visible, false otherwise.
   * Note that this function returns a pointer to a static array in csPlane3.
   * The contents of this array will only be valid until the next call to
   * ClipPolygon. Normally this function will consider the polygon visible
   * if it is on the negative side of the plane (Classify()). If 'reversed'
   * is set to true then the positive side will be used instead.
   */
  bool ClipPolygon (csVector3*& pverts, int& num_verts, bool reversed = false);

  /// Return a textual representation of the plane in the form "aa,bb,cc,dd".
  csString Description() const;
};

/** @} */

#endif // __CS_PLANE3_H__

