/*
    Copyright (C) 1998,1999,2000 by Jorrit Tyberghein
    Largely rewritten by Ivan Avramovic <ivan@avramovic.com>
    Converted to double by Thomas Hieber

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

#ifndef __CS_MATH3D_D_H__
#define __CS_MATH3D_D_H__

#include "csextern.h"

#include "cstypes.h"

/**\file 
 * 3D mathematic utility functions (double precision variants).
 */
/**
 * \addtogroup geom_utils
 * @{ */

#ifndef ABS
#define ABS(x) ((x)<0?-(x):(x))
#endif

class csDVector3;
class csDMatrix3;
class csVector3;

inline double dSqr (double d)
{
  return d * d;
}

/**
 * A 3D vector.
 */
class CS_CRYSTALSPACE_EXPORT csDVector3
{
public:
  /// The X component of the vector
  double x;
  /// The Y component of the vector
  double y;
  /// The Z component of the vector
  double z;

  /**
   * Make a new vector. The vector is not
   * initialized. This makes the code slightly faster as
   * csDVector3 objects are used a lot.
   */
  csDVector3 () {}

  /**
   * Make a new initialized vector.
   * Creates a new vector and initializes it to m*<1,1,1>.  To create
   * a vector initialized to the zero vector, use csDVector3(0)
   */
  csDVector3 (double m) : x(m), y(m), z(m) {}

  /// Make a new vector and initialize with the given values.
  csDVector3 (double ix, double iy, double iz = 0) { x = ix; y = iy; z = iz; }

  /// Copy Constructor.
  csDVector3 (const csDVector3& v) { x = v.x; y = v.y; z = v.z; }

  /// Conversion from single precision vector to double.
  csDVector3 (const csVector3&);

  /// Add two vectors.
  inline friend
  csDVector3 operator+ (const csDVector3& v1, const csDVector3& v2)
  { return csDVector3(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z); }

  /// Subtract two vectors.
  inline friend
  csDVector3 operator- (const csDVector3& v1, const csDVector3& v2)
  { return csDVector3(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z); }

  /// Take the dot product of two vectors.
  inline friend double operator* (const csDVector3& v1, const csDVector3& v2)
  { return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z; }

  /// Take the cross product of two vectors.
  inline friend
  csDVector3 operator% (const csDVector3& v1, const csDVector3& v2)
  {
    return csDVector3 (v1.y*v2.z-v1.z*v2.y,
                       v1.z*v2.x-v1.x*v2.z,
                       v1.x*v2.y-v1.y*v2.x);
  }

  /// Take cross product of two vectors and put result in this vector.
  void Cross (const csDVector3 & px, const csDVector3 & py)
  {
    x = px.y*py.z - px.z*py.y;
    y = px.z*py.x - px.x*py.z;
    z = px.x*py.y - px.y*py.x;
  }

  /// Multiply a vector and a scalar.
  inline friend csDVector3 operator* (const csDVector3& v, double f)
  { return csDVector3(v.x*f, v.y*f, v.z*f); }

  /// Multiply a vector and a scalar.
  inline friend csDVector3 operator* (double f, const csDVector3& v)
  { return csDVector3(v.x*f, v.y*f, v.z*f); }

  /// Divide a vector by a scalar.
  inline friend csDVector3 operator/ (const csDVector3& v, double f)
  { f = 1.0f/f; return csDVector3(v.x*f, v.y*f, v.z*f); }

  /// Check if two vectors are equal.
  inline friend bool operator== (const csDVector3& v1, const csDVector3& v2)
  { return v1.x==v2.x && v1.y==v2.y && v1.z==v2.z; }

  /// Check if two vectors are not equal.
  inline friend bool operator!= (const csDVector3& v1, const csDVector3& v2)
  { return v1.x!=v2.x || v1.y!=v2.y || v1.z!=v2.z; }

  /// Project one vector onto another.
  inline friend
  csDVector3 operator>> (const csDVector3& v1, const csDVector3& v2)
  { return v2*(v1*v2)/(v2*v2); }

  /// Project one vector onto another.
  inline friend
  csDVector3 operator<< (const csDVector3& v1, const csDVector3& v2)
  { return v1*(v1*v2)/(v1*v1); }

  /// Test if each component of a vector is less than a small epsilon value.
  inline friend bool operator< (const csDVector3& v, double f)
  { return ABS(v.x)<f && ABS(v.y)<f && ABS(v.z)<f; }

  /// Test if each component of a vector is greater than a small epsilon value.
  inline friend bool operator> (double f, const csDVector3& v)
  { return ABS(v.x)<f && ABS(v.y)<f && ABS(v.z)<f; }

  /// Returns n-th component of the vector.
  inline double operator[](int n) const {return !n?x:n&1?y:z;}

  /// Returns n-th component of the vector.
  inline double & operator[](int n){return !n?x:n&1?y:z;}

  /// Add another vector to this vector.
  inline csDVector3& operator+= (const csDVector3& v)
  {
    x += v.x;
    y += v.y;
    z += v.z;

    return *this;
  }

  /// Subtract another vector from this vector.
  inline csDVector3& operator-= (const csDVector3& v)
  {
    x -= v.x;
    y -= v.y;
    z -= v.z;

    return *this;
  }

  /// Multiply this vector by a scalar.
  inline csDVector3& operator*= (double f)
  { x *= f; y *= f; z *= f; return *this; }

  /// Divide this vector by a scalar.
  inline csDVector3& operator/= (double f)
  { x /= f; y /= f; z /= f; return *this; }

  /// Unary + operator.
  inline csDVector3 operator+ () const { return *this; }

  /// Unary - operator.
  inline csDVector3 operator- () const { return csDVector3(-x,-y,-z); }

  /// Set the value of this vector.
  inline void Set (double sx, double sy, double sz) { x = sx; y = sy; z = sz; }

  /// Returns the norm of this vector.
  double Norm () const;

  /// Returns the norm of this vector.
  double SquaredNorm () const;

  /**
   * Returns the unit vector in the direction of this vector.
   * Attempting to normalize a zero-vector will result in a divide by
   * zero error.  This is as it should be... fix the calling code.
   */
  csDVector3 Unit () const { return (*this)/(this->Norm()); }

  /// Returns the norm (magnitude) of a vector.
  inline static double Norm (const csDVector3& v) { return v.Norm(); }

  /// Normalizes a vector to a unit vector.
  inline static csDVector3 Unit (const csDVector3& v) { return v.Unit(); }

  /// Scale this vector to length = 1.0.
  void Normalize();
};


/**
 * A 3x3 matrix.
 */
class CS_CRYSTALSPACE_EXPORT csDMatrix3
{
public:
  double m11, m12, m13;
  double m21, m22, m23;
  double m31, m32, m33;

public:
  /// Construct a matrix, initialized to be the identity.
  csDMatrix3 ();

  /// Construct a matrix and initialize it.
  csDMatrix3 (double m11, double m12, double m13,
  	    double m21, double m22, double m23,
  	    double m31, double m32, double m33);

  /// Get the first row of this matrix as a vector.
  inline csDVector3 Row1() const { return csDVector3 (m11,m12,m13); }

  /// Get the second row of this matrix as a vector.
  inline csDVector3 Row2() const { return csDVector3 (m21,m22,m23); }

  /// Get the third row of this matrix as a vector.
  inline csDVector3 Row3() const { return csDVector3 (m31,m32,m33); }

  /// Get the first column of this matrix as a vector.
  inline csDVector3 Col1() const { return csDVector3 (m11,m21,m31); }

  /// Get the second column of this matrix as a vector.
  inline csDVector3 Col2() const { return csDVector3 (m12,m22,m32); }

  /// Get the third column of this matrix as a vector.
  inline csDVector3 Col3() const { return csDVector3 (m13,m23,m33); }

  /// Set matrix values.
  inline void Set (double m11, double m12, double m13,
                   double m21, double m22, double m23,
                   double m31, double m32, double m33)
  {
    csDMatrix3::m11 = m11; csDMatrix3::m12 = m12; csDMatrix3::m13 = m13;
    csDMatrix3::m21 = m21; csDMatrix3::m22 = m22; csDMatrix3::m23 = m23;
    csDMatrix3::m31 = m31; csDMatrix3::m32 = m32; csDMatrix3::m33 = m33;
  }

  /// Add another matrix to this matrix.
  csDMatrix3& operator+= (const csDMatrix3& m);

  /// Subtract another matrix from this matrix.
  csDMatrix3& operator-= (const csDMatrix3& m);

  /// Multiply another matrix with this matrix.
  csDMatrix3& operator*= (const csDMatrix3& m);

  /// Multiply this matrix with a scalar.
  csDMatrix3& operator*= (double s);

  /// Divide this matrix by a scalar.
  csDMatrix3& operator/= (double s);

  /// Unary + operator.
  inline csDMatrix3 operator+ () const { return *this; }
  /// Unary - operator.
  inline csDMatrix3 operator- () const
  {
   return csDMatrix3(-m11,-m12,-m13,
                    -m21,-m22,-m23,
                    -m31,-m32,-m33);
  }

  /// Transpose this matrix.
  void Transpose ();

  /// Return the transpose of this matrix.
  csDMatrix3 GetTranspose () const;

  /// Return the inverse of this matrix.
  inline csDMatrix3 GetInverse () const
  {
    csDMatrix3 C(
             (m22*m33 - m23*m32), -(m12*m33 - m13*m32),  (m12*m23 - m13*m22),
            -(m21*m33 - m23*m31),  (m11*m33 - m13*m31), -(m11*m23 - m13*m21),
             (m21*m32 - m22*m31), -(m11*m32 - m12*m31),  (m11*m22 - m12*m21) );
    double s = (double)1./(m11*C.m11 + m12*C.m21 + m13*C.m31);

    C *= s;

    return C;
  }

  /// Invert this matrix.
  void Invert() { *this = GetInverse (); }

  /// Compute the determinant of this matrix.
  double Determinant () const;

  /// Set this matrix to the identity matrix.
  void Identity ();

  /// Add two matricies.
  friend csDMatrix3 operator+ (const csDMatrix3& m1, const csDMatrix3& m2);
  /// Subtract two matricies.
  friend csDMatrix3 operator- (const csDMatrix3& m1, const csDMatrix3& m2);
  /// Multiply two matricies.
  friend csDMatrix3 operator* (const csDMatrix3& m1, const csDMatrix3& m2);

  /// Multiply a vector by a matrix (transform it).
  inline friend csDVector3 operator* (const csDMatrix3& m, const csDVector3& v)
  {
   return csDVector3 (m.m11*v.x + m.m12*v.y + m.m13*v.z,
                     m.m21*v.x + m.m22*v.y + m.m23*v.z,
                     m.m31*v.x + m.m32*v.y + m.m33*v.z);
  }

  /// Multiply a matrix and a scalar.
  friend csDMatrix3 operator* (const csDMatrix3& m, double f);
  /// Multiply a matrix and a scalar.
  friend csDMatrix3 operator* (double f, const csDMatrix3& m);
  /// Divide a matrix by a scalar.
  friend csDMatrix3 operator/ (const csDMatrix3& m, double f);
  /// Check if two matricies are equal.
  friend bool operator== (const csDMatrix3& m1, const csDMatrix3& m2);
  /// Check if two matricies are not equal.
  friend bool operator!= (const csDMatrix3& m1, const csDMatrix3& m2);
  /// Test if each component of a matrix is less than a small epsilon value.
  friend bool operator< (const csDMatrix3& m, double f);
  /// Test if each component of a matrix is greater than a small epsilon value.
  friend bool operator> (double f, const csDMatrix3& m);
};


/**
 * A plane in 3D space.
 * The plane is given by the equation AAx + BBy + CCz + DD = 0,
 * Where (AA,BB,CC) is given by the vector 'norm'.
 */
class CS_CRYSTALSPACE_EXPORT csDPlane
{
public:
  /// The normal vector (or the (A,B,C) components).
  csDVector3 norm;

  /// The D component of the plane.
  double DD;

  /// Initialize to the xy plane.
  csDPlane () : norm(0,0,1), DD(0) {}

  /// Initialize the plane.
  csDPlane (const csDVector3& plane_norm, double d=0) :
  norm(plane_norm), DD(d) {}

  /// Initialize the plane.
  csDPlane (double a, double b, double c, double d=0) : norm(a,b,c), DD(d) {}

  /// Return the normal vector of this plane.
  inline csDVector3& Normal () { return norm; }
  /// Return the normal vector of this plane.
  inline const csDVector3& Normal () const { return norm; }

  /// Return the A component of this plane.
  inline double A () const { return norm.x; }
  /// Return the B component of this plane.
  inline double B () const { return norm.y; }
  /// Return the C component of this plane.
  inline double C () const { return norm.z; }
  /// Return the D component of this plane.
  inline double D () const { return DD; }

  /// Return the A component of this plane.
  inline double& A () { return norm.x; }
  /// Return the B component of this plane.
  inline double& B () { return norm.y; }
  /// Return the C component of this plane.
  inline double& C () { return norm.z; }
  /// Return the D component of this plane.
  inline double& D () { return DD; }

  /// Set the value of the four plane components.
  inline void Set (double a, double b, double c, double d)
   { norm.x = a; norm.y = b; norm.z = c; DD = d; }

  /// Classify the given vector with regards to this plane.
  inline double Classify (const csDVector3& pt) const { return norm*pt+DD; }

  /// Classify a vector with regards to four plane components.
  static double Classify (double A, double B, double C, double D,
                         const csDVector3& pt)
  { return A*pt.x + B*pt.y + C*pt.z + D; }

  /**
   * Compute the distance from the given vector to this plane.
   * This function assumes that 'norm' is a unit vector.  If not, the function
   * returns distance times the magnitude of 'norm'.
   */
  inline double Distance (const csDVector3& pt) const
  { return ABS (Classify (pt)); }

  /// Reverses the direction of the plane while maintianing the plane itself.
  void Invert () { norm = -norm;  DD = -DD; }

  /// Normalizes the plane equation so that 'norm' is a unit vector.
  void Normalize ()
  {
    double f = norm.Norm ();
    if (f) { norm /= f;  DD /= f; }
  }

};

/**
 * Various assorted 3D mathematical functions.
 * This is a static class and contains only static member functions.
 */
class CS_CRYSTALSPACE_EXPORT csDMath3
{
public:
  /**
   * Tests which side of a plane the given 3D point is on.
   * Return -1 if point p is left of plane '0-v1-v2',
   *         1 if point p is right of plane '0-v1-v2',
   *      or 0 if point p lies on plane '0-v1-v2'.
   * Plane '0-v1-v2' is the plane passing through points <0,0,0>, v1, and v2.
   */
  static int WhichSide3D (const csDVector3& p,
                          const csDVector3& v1, const csDVector3& v2)
  {
    // double s = p * (v1%v2); (original expression: expanded to the below:)
    double s = p.x*(v1.y*v2.z-v1.z*v2.y) + p.y*(v1.z*v2.x-v1.x*v2.z) +
              p.z*(v1.x*v2.y-v1.y*v2.x);
    if (s < 0) return 1;
    else if (s > 0) return -1;
    else return 0;
  }

  /**
   * Tests if the front face of a triangle is visible from the given point.
   * Visibility test (backface culling) to see if the triangle formed by
   * t1, t2, and t3 is visible from point p.
   */
  static bool Visible (const csDVector3& p, const csDVector3& t1,
                       const csDVector3& t2, const csDVector3& t3);

  /**
   * Check if the plane is visible from the given point.
   * This function does a back-face culling test to see whether the front
   * face of plane pl is visible from point p.
   */
  static bool Visible (const csDVector3& p, const csDPlane& pl)
  { return pl.Classify (p) <= 0; }

  /**
   * Calculates a vector lying a specified distance between two other vectors.
   * Given vectors v1 and v2, this function will calculate and return vector
   * v lying between them.
   * If pct != -1, vector v will be the point which is pct % of the
   * way from v1 to v2.
   * Otherwise, if pct equals -1, v will be the point along 'v1-v2' which is
   * distance wid from v1.
   */
  static void Between (const csDVector3& v1, const csDVector3& v2,
                       csDVector3& v, double pct, double wid);

  /**
   * Set the min and max vector if this vector exceeds their current limits.
   * This function will check each component of vector v against the maximum
   * and minimum values specified by min and max.  If the limits are
   * exceeded, new min or max values will be set.
   */
  static void SetMinMax (const csDVector3& v,
                         csDVector3& min, csDVector3& max)
  {
    if (v.x > max.x) max.x = v.x; else if (v.x < min.x ) min.x = v.x;
    if (v.y > max.y) max.y = v.y; else if (v.y < min.y ) min.y = v.y;
    if (v.z > max.z) max.z = v.z; else if (v.z < min.z ) min.z = v.z;
  }

  /**
   * Compute twice the area of triangle composed by three points.
   * This function returns 2 x the area of the triangle formed by the points
   * a, b, and c.
   */
  inline static double DoubleArea3 (const csDVector3 &a, const csDVector3 &b,
                             const csDVector3 &c)
  {
    csDVector3 v1 = b - a;
    csDVector3 v2 = c - a;
    return (v1 % v2).Norm ();
  }

  /**
   * Calculate a plane normal given three vectors.
   * This function will calculate the normal to the plane formed by vectors
   * v1, v2, and v3, and store the result in norm.
   */
  inline static void CalcNormal (csDVector3& norm,     const csDVector3& v1,
                                 const csDVector3& v2, const csDVector3& v3)
  {
    norm = (v1-v2)%(v1-v3);
  }

  /**
   * Compute the normal given two (u,v) vectors.
   * This function will calculat the normal to a polygon with two edges
   * represented by v and u.  The result is stored in norm.
   */
  static void CalcNormal (csDVector3& norm,
                          const csDVector3& v, const csDVector3& u)
  { norm = u%v; /* NOT v%u - vertexes are defined clockwise */ }

  /**
   * Calculate the plane equation given three vectors.
   * Given three vectors v1, v2, and v3, forming a plane, this function
   * will calculate the plane equation and return the result in 'normal'
   * and 'D'.
   */
  static void CalcPlane (const csDVector3& v1, const csDVector3& v2,
         const csDVector3& v3, csDVector3& normal, double& D)
  {
    normal = (v1-v2)%(v1-v3);
    D = - (normal * v1);
  }

  /**
   * Check if two planes are almost equal.
   * The function returns true iff each component of the plane equation for
   * one plane is within .001 of the corresponding component of the other
   * plane.
   */
  static bool PlanesEqual (const csDPlane& p1, const csDPlane& p2)
  {
    return ( ( p1.norm - p2.norm) < (double).001 ) &&
             (  ABS (p1.DD-p2.DD) < (double).001 );
  }

  /**
   * Check if two planes are close together.
   * Two planes are close if there are almost equal OR if
   * the normalized versions are almost equal.
   */
  static bool PlanesClose (const csDPlane& p1, const csDPlane& p2);
};

/**
 * Some functions to perform squared distance calculations.
 * This is a static class and contains only static member functions.
 */
class CS_CRYSTALSPACE_EXPORT csDSquaredDist
{
public:
  /// Returns the squared distance between two points.
  static double PointPoint (const csDVector3& p1, const csDVector3& p2)
  { return dSqr (p1.x - p2.x) + dSqr (p1.y - p2.y) + dSqr (p1.z - p2.z); }

  /// Returns the squared distance between a point and a line.
  static double PointLine (const csDVector3& p,
                          const csDVector3& l1, const csDVector3& l2);

  /// Returns the squared distance between a point and a normalized plane.
  static double PointPlane (const csDVector3& p, const csDPlane& plane)
  { double r = plane.Classify (p);  return r * r; }

  /**
   * Returns the squared distance between a point and a polygon.
   * If sqdist is >= 0, then it is used as the pre-calculated point to
   * plane distance.  V is an array of vertices, n is the number of
   * vertices, and plane is the polygon plane.
   */
  static double PointPoly (const csDVector3& p, csDVector3 *V, int n,
                          const csDPlane& plane, double sqdist = -1);
};

/**
 * Some functions to perform various intersection calculations with 3D
 * line segments.  This is a static class and contains only static member
 * functions.
 */
class CS_CRYSTALSPACE_EXPORT csDIntersect3
{
public:
  /**
   * Intersect a 3D segment with a plane.  Returns true if there is an
   * intersection, with the intersection point returned in isect.
   */
  static void Plane (
    const csDVector3& u, const csDVector3& v, // segment
    const csDVector3& normal, const csDVector3& a, // plane
    csDVector3& isect);                    // intersection point

  /**
   * Intersect a 3D segment with a plane.  Returns true if there is an
   * intersection, with the intersection point returned in isect.
   * The distance from u to the intersection point is returned in dist.
   * The distance that is returned is a normalized distance with respect
   * to the given input vector. i.e. a distance of 0.5 means that the
   * intersection point is halfway u and v.
   */
  static bool Plane (
    const csDVector3& u, const csDVector3& v, // segment
    double A, double B, double C, double D, // plane Ax+By+Cz+D=0
    csDVector3& isect,                     // intersection point
    double& dist);                       // distance from u to isect

  /**
   * Intersect a 3D segment with a plane.  Returns true if there is an
   * intersection, with the intersection point returned in isect.
   * The distance from u to the intersection point is returned in dist.
   * The distance that is returned is a normalized distance with respect
   * to the given input vector. i.e. a distance of 0.5 means that the
   * intersection point is halfway u and v.
   */
  static bool Plane (
    const csDVector3& u, const csDVector3& v, // segment
    const csDPlane& p,                     // plane Ax+By+Cz+D=0
    csDVector3& isect,                     // intersection point
    double& dist);                       // distance from u to isect

  /**
   * Intersect 3 planes, to get the point that is part of all three
   * planes. Returns true, if there is a single point that fits.
   * If some planes are parallel, then it will return false.
   */
  static bool Planes(const csDPlane& p1, const csDPlane& p2,
                     const csDPlane& p3, csDVector3& isect);

  /**
   * Intersect a 3D segment with the z = 0 plane.  Assumes that there
   * is an intersection (fails if the segment is parallel to the plane),
   * and returns the distance from u to the intersection point.
   * The intersection point is returned in isect.
   */
  static double Z0Plane (
    const csDVector3& u, const csDVector3& v, // segment
    csDVector3& isect);                    // intersection point

  /**
   * Intersect a 3D segment with the plane z = zval. Assumes that there
   * is an intersection (fails if the segment is parallel to the plane),
   * and returns the distance from u to the intersection point.
   * The intersection point is returned in isect.
   */
  static double ZPlane (double zval,      // plane z = zval
    const csDVector3& u, const csDVector3& v, // segment
    csDVector3& isect);                    // intersection point

  /**
   * Intersect a 3D segment with the frustum plane Ax + z = 0.
   * Assumes an intersection, and returns the intersection point in isect.
   */
  static double XFrustum (
    double A, const csDVector3& u, const csDVector3& v, csDVector3& isect);

  /**
   * Intersect a 3D segment with the frustum plane By + z = 0.
   * Assumes an intersection, and returns the intersection point in isect.
   */
  static double YFrustum (
    double B, const csDVector3& u, const csDVector3& v, csDVector3& isect);
};

/** @} */

#endif // __CS_MATH3D_D_H__
