/*
    Copyright (C) 1998 by Jorrit Tyberghein
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

#ifndef MATH2D_H
#define MATH2D_H

#define CS_POLY_IN 1
#define CS_POLY_ON 0
#define CS_POLY_OUT -1

#include "types.h"	// for bool

class csBox;
class csPoly2D;

/**
 * A 2D vector.
 */
class csVector2
{
public:
  ///
  float x;
  ///
  float y;

  /// Make a new vector. No initialization is done.
  csVector2 () {}

  /// Make a new vector and initialize with the given values.
  csVector2 (float x, float y) { csVector2::x = x; csVector2::y = y; }

  /// Set vector to given values.
  inline void Set (float ix, float iy)
  { x = ix; y = iy; }

  /// Return the norm (magnitude) of a 2D vector.
  static float Norm (const csVector2& v);

  /// Return the norm (magnitude) of this vector.
  float Norm () const;

  /// Return the squared norm (magnitude) of this vector.
  float SquaredNorm () const
  { return x * x + y * y; }

  /// Rotate vector around the origin by a given angle in radians.
  void Rotate (float angle);

  /// Add another vector to this vector.
  csVector2& operator+= (const csVector2& v)
  { x += v.x;  y += v.y;  return *this; }

  /// Subtract another vector from this vector.
  csVector2& operator-= (const csVector2& v)
  { x -= v.x;  y -= v.y;  return *this; }

  /// Multiply this vector by a scalar.
  csVector2& operator*= (float f) { x *= f;  y *= f;  return *this; }

  /// Divide this vector by a scalar.
  csVector2& operator/= (float f) { x /= f;  y /= f;  return *this; }

  /// Unary + operator.
  inline csVector2 operator+ () const { return *this; }

  /// Unary - operator.
  inline csVector2 operator- () const { return csVector2(-x,-y); }

  /// Add two vectors.
  friend csVector2 operator+ (const csVector2& v1, const csVector2& v2);
  /// Subtract two vectors.
  friend csVector2 operator- (const csVector2& v1, const csVector2& v2);
  /// Take the dot product of two vectors.
  friend float operator* (const csVector2& v1, const csVector2& v2);
  /// Multiply a vector and a scalar.
  friend csVector2 operator* (const csVector2& v, float f);
  /// Multiply a vector and a scalar.
  friend csVector2 operator* (float f, const csVector2& v);
  /// Divide a vector by a scalar.
  friend csVector2 operator/ (const csVector2& v, float f);
  /// Check if two vectors are equal.
  friend bool operator== (const csVector2& v1, const csVector2& v2);
  /// Check if two vectors are not equal.
  friend bool operator!= (const csVector2& v1, const csVector2& v2);

  /// Test if each component of a vector is less than a small epsilon value.
  inline friend bool operator< (const csVector2& v, float f)
  { return ABS(v.x)<f && ABS(v.y)<f; }

  /// Test if each component of a vector is greater than a small epsilon value.
  inline friend bool operator> (float f, const csVector2& v)
  { return ABS(v.x)<f && ABS(v.y)<f; }
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

  /// Initialize the plane given two vectors.
  csPlane2 (const csVector2& v1, const csVector2& v2)
  {
    Set (v1, v2);
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
  void Invert () { norm = -norm;  CC = -CC; }

  /// Normalizes the plane equation so that 'norm' is a unit vector.
  void Normalize ()
  {
    float f = norm.Norm ();
    if (f) { norm /= f;  CC /= f; }
  }

  /**
   * Intersect this plane with a 2D polygon and return
   * the line segment corresponding with this intersection.
   * Returns true if there is an intersection. If false
   * then 'v1' and 'v2' will not be valid.
   */
  bool IntersectPolygon (csPoly2D* poly, csVector2& v1, csVector2& v2);
};

/**
 * Various functions in 2D, such as 2D vector functions.
 * This is a static class and contains only static member functions.
 */
class csMath2
{
public:
  /**
   * Calculates which side of a line a given point is on.
   * Returns -1 if point v is left of line segment 's1-s2',
   *          1 if point v is right of segment 's1-s2'
   *       or 0 if point v lies on segment 's1-s2'.
   */
  static int WhichSide2D (const csVector2& v, 
                          const csVector2& s1, const csVector2& s2)
  {
    float k  = (s1.y - v.y)*(s2.x - s1.x);
    float k1 = (s1.x - v.x)*(s2.y - s1.y);
    if (k < k1) return -1;
    else if (k > k1) return 1;
    else return 0;
  }

  /**
   * Calculates whether a vector lies inside a given 2D polygon.
   * Return CS_POLY_IN, CS_POLY_OUT, or CS_POLY_ON for this vector with
   * respect to the given polygon. The polygon is given as an array of 2D
   * vectors with a bounding box.
   * WARNING: does no safety checking for P or bounding_box.
   */
  static int InPoly2D (const csVector2& v,
                       csVector2* P, int n, csBox* bounding_box);

  /**
   * Calculates 2 x the area of a given triangle.
   * Returns twice the signed area of the triangle determined by a,b,c,
   * positive if a,b,c are oriented ccw, and negative if cw.
   */
  static float Area2 (const csVector2& a,
  		      const csVector2& b,
  		      const csVector2& c)
  {
    return
      a.x * b.y - a.y * b.x +
      a.y * c.x - a.x * c.y +
      b.x * c.y - c.x * b.y;
  }

  /**
   * Calculates whether a point lies to the right of a given line.
   * Returns true iff c is strictly to the right of the directed
   * line through a to b.
   */
  static float Right (const csVector2& a,
  		      const csVector2& b,
  		      const csVector2& c)
  {
    return Area2 (a, b, c) <= -SMALL_EPSILON;
  }

  /**
   * Calculates whether a point lies to the left of a given line.
   * Returns true iff c is strictly to the left of the directed
   * line through a to b.
   */
  static float Left (const csVector2& a,
  		     const csVector2& b,
  		     const csVector2& c)
  {
    return Area2 (a, b, c) >= SMALL_EPSILON;
  }

  /**
   * Check if the plane is visible from the given point.
   * This function does a back-face culling test to see whether the front
   * face of plane pl is visible from point p.
   */
  static bool Visible (const csVector2& p, const csPlane2& pl)
  { return pl.Classify (p) <= 0; }

  /**
   * Check if two planes are almost equal.
   * The function returns true iff each component of the plane equation for
   * one plane is within .001 of the corresponding component of the other
   * plane.
   */
  static bool PlanesEqual (const csPlane2& p1, const csPlane2& p2)
  {
    return ( ( p1.norm - p2.norm) < (float).001 ) &&
             (  ABS (p1.CC-p2.CC) < (float).001 );
  }

  /**
   * Check if two planes are close together.
   * Two planes are close if there are almost equal OR if
   * the normalized versions are almost equal.
   */
  static bool PlanesClose (const csPlane2& p1, const csPlane2& p2);
};

/**
 * Some functions to perform various intersection calculations with 2D
 * line segments.  This is a static class and contains only static member
 * functions.
 */
class csIntersect2
{
public:
  /**
   * Compute the intersection of the 2D segments.  Return true if they
   * intersect, with the intersection point returned in isect,  and the
   * distance from a1 of the intersection in dist.
   */
  static bool Segments (
    const csVector2& a1, const csVector2& a2, // first segment
    const csVector2& b1, const csVector2& b2, // second segment
    csVector2& isect, float& dist);         // intersection point and distance

  /**
   * Compute the intersection of a 2D segment and a line.  Return true if they
   * intersect, with the intersection point returned in isect,  and the
   * distance from a1 of the intersection in dist.
   */
  static bool SegmentLine (
    const csVector2& a1, const csVector2& a2, // first segment
    const csVector2& b1, const csVector2& b2, // second line
    csVector2& isect, float& dist);         // intersection point and distance

  /**
   * Compute the intersection of 2D lines.  Return true if they
   * intersect, with the intersection point returned in isect.
   */
  static bool Lines (
    const csVector2& a1, const csVector2& a2, // first line
    const csVector2& b1, const csVector2& b2, // second line
    csVector2& isect);                      // intersection point

  /**
   * Intersect a 2D segment with a plane.  Returns true if there is an
   * intersection, with the intersection point returned in isect.
   * The distance from u to the intersection point is returned in dist.
   * The distance that is returned is a normalized distance with respect
   * to the given input vector. i.e. a distance of 0.5 means that the
   * intersection point is halfway u and v.
   */
  static bool Plane (
    const csVector2& u, const csVector2& v, // segment
    const csPlane2& p,                     // plane Ax+By+Cz+D=0
    csVector2& isect,                     // intersection point
    float& dist);                       // distance from u to isect

  /**
   * Return the intersection point. This version does not test if
   * there really is an intersection. It just assumes there is one.
   */
  static void PlaneNoTest (const csVector2& u, const csVector2& v,
                     const csPlane2& p, csVector2& isect, float& dist)
  {
    float x,y, denom;
    x = v.x-u.x;  y = v.y-u.y;
    denom = p.norm.x*x + p.norm.y*y;
    dist = -(p.norm*u + p.CC) / denom;
    isect.x = u.x + dist*x;  isect.y = u.y + dist*y;
  }

};

/**
 * A 2x2 matrix.
 */
class csMatrix2
{
public:
  float m11, m12;
  float m21, m22;

public:
  /// Construct a matrix, initialized to be the identity.
  csMatrix2 ();

  /// Construct a matrix and initialize it.
  csMatrix2 (float m11, float m12,
             float m21, float m22);

  /// Get the first row of this matrix as a vector.
  inline csVector2 Row1() const { return csVector2 (m11,m12); }

  /// Get the second row of this matrix as a vector.
  inline csVector2 Row2() const { return csVector2 (m21,m22); }

  /// Get the first column of this matrix as a vector.
  inline csVector2 Col1() const { return csVector2 (m11,m21); }

  /// Get the second column of this matrix as a vector.
  inline csVector2 Col2() const { return csVector2 (m12,m22); }

  /// Set matrix values.
  inline void Set (float m11, float m12,
                   float m21, float m22)
  {
    csMatrix2::m11 = m11; csMatrix2::m12 = m12;
    csMatrix2::m21 = m21; csMatrix2::m22 = m22;
  }

  /// Add another matrix to this matrix.
  csMatrix2& operator+= (const csMatrix2& m);

  /// Subtract another matrix from this matrix.
  csMatrix2& operator-= (const csMatrix2& m);

  /// Multiply another matrix with this matrix.
  csMatrix2& operator*= (const csMatrix2& m);

  /// Multiply this matrix with a scalar.
  csMatrix2& operator*= (float s);

  /// Divide this matrix by a scalar.
  csMatrix2& operator/= (float s);

  /// Unary + operator.
  inline csMatrix2 operator+ () const { return *this; }
  /// Unary - operator.
  inline csMatrix2 operator- () const
  {
   return csMatrix2(-m11,-m12,
                    -m21,-m22);
  }

  /// Transpose this matrix.
  void Transpose ();

  /// Return the transpose of this matrix.
  csMatrix2 GetTranspose () const;

  /// Return the inverse of this matrix.
  inline csMatrix2 GetInverse () const
  {
    float inv_det = 1 / (m11 * m22 - m12 * m21);
    return csMatrix2 (m22 * inv_det, -m12 * inv_det, -m21 * inv_det, m11 * inv_det);
  }

  /// Invert this matrix.
  void Invert () { *this = GetInverse (); }

  /// Compute the determinant of this matrix.
  float Determinant () const;

  /// Set this matrix to the identity matrix.
  void Identity ();

  /// Add two matricies.
  friend csMatrix2 operator+ (const csMatrix2& m1, const csMatrix2& m2);
  /// Subtract two matricies.
  friend csMatrix2 operator- (const csMatrix2& m1, const csMatrix2& m2);
  /// Multiply two matricies.
  friend csMatrix2 operator* (const csMatrix2& m1, const csMatrix2& m2);

  /// Multiply a vector by a matrix (transform it).
  inline friend csVector2 operator* (const csMatrix2& m, const csVector2& v)
  {
    return csVector2 (m.m11*v.x + m.m12*v.y, m.m21*v.x + m.m22*v.y);
  }

  /// Multiply a matrix and a scalar.
  friend csMatrix2 operator* (const csMatrix2& m, float f);
  /// Multiply a matrix and a scalar.
  friend csMatrix2 operator* (float f, const csMatrix2& m);
  /// Divide a matrix by a scalar.
  friend csMatrix2 operator/ (const csMatrix2& m, float f);
};

#endif /*MATH_H*/
