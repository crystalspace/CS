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

#ifndef __MATH3D_H__
#define __MATH3D_H__

#if defined(COMP_VC) && defined(DO_ASM)
#  include "csgeom/vc_asm.inc"
#endif

class csVector3;
class csMatrix3;

/**
 * A 3D vector.
 */
class csVector3
{
public:
  ///
  float x;
  ///
  float y;
  ///
  float z;

  /// Make a new vector, initialized to the zero vector.
  csVector3 () { x = y = z = 0; }

  /// Make a new vector and initialize with the given values.
  csVector3 (float x, float y, float z = 0)
   { csVector3::x = x; csVector3::y = y; csVector3::z = z; }

  /// Add two vectors.
  inline friend csVector3 operator+ (const csVector3& v1, const csVector3& v2)
  { return csVector3(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z); }

  /// Subtract two vectors.
  inline friend csVector3 operator- (const csVector3& v1, const csVector3& v2)
  { return csVector3(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z); }

  /// Take the dot product of two vectors.
  inline friend float operator* (const csVector3& v1, const csVector3& v2)
  { return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z; }

  /// Take the cross product of two vectors.
  inline friend csVector3 operator% (const csVector3& v1, const csVector3& v2)
  {
    return csVector3 (v1.y*v2.z-v1.z*v2.y,
                    v1.z*v2.x-v1.x*v2.z,
                    v1.x*v2.y-v1.y*v2.x);
  }

  /// Multiply a vector and a scalar.
  inline friend csVector3 operator* (const csVector3& v, float f)
  { return csVector3(v.x*f, v.y*f, v.z*f); }

  /// Multiply a vector and a scalar.
  inline friend csVector3 operator* (float f, const csVector3& v)
  { return csVector3(v.x*f, v.y*f, v.z*f); }

  /// Divide a vector by a scalar.
  inline friend csVector3 operator/ (const csVector3& v, float f)
  { f = 1.0/f; return csVector3(v.x*f, v.y*f, v.z*f); }

  /// Check if two vectors are equal.
  inline friend bool operator== (const csVector3& v1, const csVector3& v2)
  { return v1.x==v2.x && v1.y==v2.y && v1.z==v2.z; }

  /// Check if two vectors are not equal.
  inline friend bool operator!= (const csVector3& v1, const csVector3& v2)
  { return v1.x!=v2.x || v1.y!=v2.y || v1.z!=v2.z; }

  /// Project one vector onto another.
  inline friend csVector3 operator>> (const csVector3& v1, const csVector3& v2)
  { return v2*(v1*v2)/(v2*v2); }

  /// Project one vector onto another.
  inline friend csVector3 operator<< (const csVector3& v1, const csVector3& v2)
  { return v1*(v1*v2)/(v1*v1); }

  /// Test if each component of a vector is less than a small epsilon value.
  inline friend bool operator< (const csVector3& v, float f)
  { return ABS(v.x)<f && ABS(v.y)<f && ABS(v.z)<f; }

  /// Test if each component of a vector is greater than a small epsilon value.
  inline friend bool operator> (float f, const csVector3& v)
  { return ABS(v.x)<f && ABS(v.y)<f && ABS(v.z)<f; }

	/// Returns n-th component of the vector
  inline float operator[](int n) const {return !n?x:n&1?y:z;}

  /// Add another vector to this vector.
  inline csVector3& operator+= (const csVector3& v)
  {
#   ifdef CS_ASM__VECTOR3_PLUSEQ
      CS_ASM__VECTOR3_PLUSEQ
#   else
    x += v.x;
    y += v.y;
    z += v.z;
#   endif
    return *this;
  }

  /// Subtract another vector from this vector.
  inline csVector3& operator-= (const csVector3& v)
  {
#   ifdef CS_ASM__VECTOR3_MINUSEQ
      CS_ASM__VECTOR3_MINUSEQ
#   else
    x -= v.x;
    y -= v.y;
    z -= v.z;
#   endif
    return *this;
  }

  /// Multiply this vector by a scalar.
  inline csVector3& operator*= (float f) { x *= f; y *= f; z *= f; return *this; }

  /// Divide this vector by a scalar.
  inline csVector3& operator/= (float f) { x /= f; y /= f; z /= f; return *this; }

  /// Unary + operator.
  inline csVector3 operator+ () const { return *this; }

  /// Unary - operator.
  inline csVector3 operator- () const { return csVector3(-x,-y,-z); }

  /// Set the value of this vector.
  inline void Set (float sx, float sy, float sz) { x = sx; y = sy; z = sz; }

  /// Returns the norm of this vector.
  float Norm () const;

  /**
   * Returns the unit vector in the direction of this vector.
   * Attempting to normalize a zero-vector will result in a divide by
   * zero error.  This is as it should be... fix the calling code.
   */
  csVector3 Unit () const { return (*this)/(this->Norm()); }

  /// Returns the norm (magnitude) of a vector.
  inline static float Norm (const csVector3& v) { return v.Norm(); }

#if 1
  // Normalizes a vector to a unit vector.
  // @@@ JTY: For some very strange reason this version does not work
  // for me (PGCC on Linux).
  // When I use this version pressing 'b' will distort the camera.
  inline static csVector3 Unit (const csVector3& v) { return v.Unit(); }
#else
  /// Normalizes a vector to a unit vector.
  static csVector3 Unit (const csVector3& v) { return v/v.Norm (); }
#endif

};

/**
 * A 3x3 matrix.
 */
class csMatrix3
{
public:
  float m11, m12, m13;
  float m21, m22, m23;
  float m31, m32, m33;

public:
  /// Construct a matrix, initialized to be the identity.
  csMatrix3 ();

  /// Construct a matrix and initialize it.
  csMatrix3 (float m11, float m12, float m13,
  	    float m21, float m22, float m23,
  	    float m31, float m32, float m33);

  /// Get the first row of this matrix as a vector.
  inline csVector3 Row1() const { return csVector3 (m11,m12,m13); }

  /// Get the second row of this matrix as a vector.
  inline csVector3 Row2() const { return csVector3 (m21,m22,m23); }

  /// Get the third row of this matrix as a vector.
  inline csVector3 Row3() const { return csVector3 (m31,m32,m33); }

  /// Get the first column of this matrix as a vector.
  inline csVector3 Col1() const { return csVector3 (m11,m21,m31); }

  /// Get the second column of this matrix as a vector.
  inline csVector3 Col2() const { return csVector3 (m12,m22,m32); }

  /// Get the third column of this matrix as a vector.
  inline csVector3 Col3() const { return csVector3 (m13,m23,m33); }

  /// Set matrix values.
  inline void Set (float m11, float m12, float m13,
                   float m21, float m22, float m23,
                   float m31, float m32, float m33)
  {
    csMatrix3::m11 = m11; csMatrix3::m12 = m12; csMatrix3::m13 = m13;
    csMatrix3::m21 = m21; csMatrix3::m22 = m22; csMatrix3::m23 = m23;
    csMatrix3::m31 = m31; csMatrix3::m32 = m32; csMatrix3::m33 = m33;
  }

  /// Add another matrix to this matrix.
  csMatrix3& operator+= (const csMatrix3& m);

  /// Subtract another matrix from this matrix.
  csMatrix3& operator-= (const csMatrix3& m);

  /// Multiply another matrix with this matrix.
  csMatrix3& operator*= (const csMatrix3& m);

  /// Multiply this matrix with a scalar.
  csMatrix3& operator*= (float s);

  /// Divide this matrix by a scalar.
  csMatrix3& operator/= (float s);

  /// Unary + operator.
  inline csMatrix3 operator+ () const { return *this; }
  /// Unary - operator.
  inline csMatrix3 operator- () const
  {
   return csMatrix3(-m11,-m12,-m13,
                    -m21,-m22,-m23,
                    -m31,-m32,-m33);
  }

  /// Transpose this matrix.
  void Transpose ();

  /// Return the transpose of this matrix.
  csMatrix3 GetTranspose () const;

  /// Return the inverse of this matrix.
  inline csMatrix3 GetInverse () const
  {
    csMatrix3 C(
             (m22*m33 - m23*m32), -(m12*m33 - m13*m32),  (m12*m23 - m13*m22),
            -(m21*m33 - m23*m31),  (m11*m33 - m13*m31), -(m11*m23 - m13*m21),
             (m21*m32 - m22*m31), -(m11*m32 - m12*m31),  (m11*m22 - m12*m21) );
    float s = (float)1./(m11*C.m11 + m12*C.m21 + m13*C.m31);

#   ifdef CS_ASM__MATRIX3_MULTSCL
      CS_ASM__MATRIX3_MULTSCL
#   else
      C *= s;
#   endif
    return C;
  }

  /// Invert this matrix.
  void Invert() { *this = GetInverse (); }

  /// Compute the determinant of this matrix.
  float Determinant () const;

  /// Compute the eigen values and eigen vectors of this matrix.
  int Eigen (csMatrix3* evectors, csVector3* evalues);

  /// Compute the eigen vectors return largest one in 1st column.
  int Eigens1 (csMatrix3 *evecs);

  /// Set this matrix to the identity matrix.
  void Identity ();

  /// Add two matricies.
  friend csMatrix3 operator+ (const csMatrix3& m1, const csMatrix3& m2);
  /// Subtract two matricies.
  friend csMatrix3 operator- (const csMatrix3& m1, const csMatrix3& m2);
  /// Multiply two matricies.
  friend csMatrix3 operator* (const csMatrix3& m1, const csMatrix3& m2);

  /// Multiply a vector by a matrix (transform it).
  inline friend csVector3 operator* (const csMatrix3& m, const csVector3& v)
  {
   return csVector3 (m.m11*v.x + m.m12*v.y + m.m13*v.z,
                     m.m21*v.x + m.m22*v.y + m.m23*v.z,
                     m.m31*v.x + m.m32*v.y + m.m33*v.z);
  }

  /// Multiply a matrix and a scalar.
  friend csMatrix3 operator* (const csMatrix3& m, float f);
  /// Multiply a matrix and a scalar.
  friend csMatrix3 operator* (float f, const csMatrix3& m);
  /// Divide a matrix by a scalar.
  friend csMatrix3 operator/ (const csMatrix3& m, float f);
  /// Check if two matricies are equal.
  friend bool operator== (const csMatrix3& m1, const csMatrix3& m2);
  /// Check if two matricies are not equal.
  friend bool operator!= (const csMatrix3& m1, const csMatrix3& m2);
  /// Test if each component of a matrix is less than a small epsilon value.
  friend bool operator< (const csMatrix3& m, float f);
  /// Test if each component of a matrix is greater than a small epsilon value.
  friend bool operator> (float f, const csMatrix3& m);
};

/**
 * A plane in 3D space.
 * The plane is given by the equation AAx + BBy + CCz + DD = 0,
 * Where (AA,BB,CC) is given by the vector 'norm'.
 */
class csPlane
{
public:
  /// The normal vector (or the (A,B,C) components).
  csVector3 norm;

  /// The D component of the plane.
  float DD;

  /// Initialize to the xy plane.
  csPlane () : norm(0,0,1), DD(0) {}

  /// Initialize the plane.
  csPlane (const csVector3& plane_norm, float d=0) : norm(plane_norm), DD(d) {}

  /// Initialize the plane.
  csPlane (float a, float b, float c, float d=0) : norm(a,b,c), DD(d) {}

  /// Return the normal vector of this plane.
  inline csVector3& Normal () { return norm; }

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

/**
 * Various assorted 3D mathematical functions.
 * This is a static class and contains only static member functions.
 */
class csMath3
{
public:
  /**
   * Tests which side of a plane the given 3D point is on.
   * Return -1 if point p is left of plane '0-v1-v2',
   *         1 if point p is right of plane '0-v1-v2',
   *      or 0 if point p lies on plane '0-v1-v2'.
   * Plane '0-v1-v2' is the plane passing through points <0,0,0>, v1, and v2.
   */
  static int WhichSide3D (const csVector3& p,
                          const csVector3& v1, const csVector3& v2);

  /**
   * Tests if the front face of a triangle is visible from the given point.
   * Visibility test (backface culling) to see if the triangle formed by
   * t1, t2, and t3 is visible from point p.
   */
  static bool Visible (const csVector3& p, const csVector3& t1,
                       const csVector3& t2, const csVector3& t3);

  /**
   * Check if the plane is visible from the given point.
   * This function does a back-face culling test to see whether the front
   * face of plane pl is visible from point p.
   */
  static bool Visible (const csVector3& p, const csPlane& pl)
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
  static void Between (const csVector3& v1, const csVector3& v2, csVector3& v,
                       float pct, float wid);

  /**
   * Set the min and max vector if this vector exceeds their current limits.
   * This function will check each component of vector v against the maximum
   * and minimum values specified by min and max.  If the limits are
   * exceeded, new min or max values will be set.
   */
  static void SetMinMax (const csVector3& v,
                         csVector3& min, csVector3& max)
  {
    if (v.x > max.x) max.x = v.x; else if (v.x < min.x ) min.x = v.x;
    if (v.y > max.y) max.y = v.y; else if (v.y < min.y ) min.y = v.y;
    if (v.z > max.z) max.z = v.z; else if (v.z < min.z ) min.z = v.z;
  }

  /**
   * Compute twice the signed area of triangle composed by three points.
   * This function returns 2 x the area of the triangle formed by the points
   * a, b, and c.
   */
  inline static float Area3 (const csVector3 &a, const csVector3 &b,
                             const csVector3 &c)
  {
    csVector3 v1 = b - a;
    csVector3 v2 = c - a;
    return ((v1.y * v2.z + v1.z * v2.x + v1.x * v2.y) -
            (v1.y * v2.x + v1.x * v2.z + v1.z * v2.y));
  }

  /**
   * Calculate a plane normal given three vectors.
   * This function will calculate the normal to the plane formed by vectors
   * v1, v2, and v3, and store the result in norm.
   */
  inline static void CalcNormal (csVector3& norm,     const csVector3& v1,
                                 const csVector3& v2, const csVector3& v3)
  {
#   ifdef CS_ASM__CALC_PLANE_NORMAL
      CS_ASM__CALC_PLANE_NORMAL
#   else
      norm = (v1-v2)%(v1-v3);
#   endif
  }

  /**
   * Compute the normal given two (u,v) vectors.
   * This function will calculat the normal to a polygon with two edges
   * represented by v and u.  The result is stored in norm.
   */
  static void CalcNormal (csVector3& norm,
                          const csVector3& v, const csVector3& u)
  { norm = u%v; /* NOT v%u - vertexes are defined clockwise */ }

  /**
   * Calculate the plane equation given three vectors.
   * Given three vectors v1, v2, and v3, forming a plane, this function
   * will calculate the plane equation and return the result in 'normal'
   * and 'D'.
   */
  static void CalcPlane (const csVector3& v1, const csVector3& v2,
         const csVector3& v3, csVector3& normal, float& D)
  {
    normal = (v1-v2)%(v1-v3);
    D = - (normal * v1);
  }

  /**
   * Check if two planes are close together.
   * The function returns true iff each component of the plane equation for
   * one plane is within .001 of the corresponding component of the other
   * plane.
   */
  static bool PlanesClose (const csPlane& p1, const csPlane& p2)
  {
    return ( ( p1.norm - p2.norm) < (float).001 ) &&
             (  ABS (p1.DD-p2.DD) < (float).001 );
  }

};

/**
 * Some functions to perform squared distance calculations.
 * This is a static class and contains only static member functions.
 */
class csSquaredDist
{
public:
  /// Returns the squared distance between two points.
  static float PointPoint (const csVector3& p1, const csVector3& p2)
  {  csVector3 v = p1-p2;  return v*v; }

  /// Returns the squared distance between a point and a line.
  static float PointLine (const csVector3& p,
                          const csVector3& l1, const csVector3& l2);

  /// Returns the squared distance between a point and a normalized plane.
  static float PointPlane (const csVector3& p, const csPlane& plane)
  { float r = plane.Classify(p);  return r*r; }

  /**
   * Returns the squared distance between a point and a polygon.
   * If sqdist is >= 0, then it is used as the pre-calculated point to
   * plane distance.  V is an array of vertices, n is the number of
   * vertices, and plane is the polygon plane.
   */
  static float PointPoly (const csVector3& p, csVector3 *V, int n,
                          const csPlane& plane, float sqdist = -1);
};

/**
 * Some functions to perform various intersection calculations with 3D
 * line segments.  This is a static class and contains only static member
 * functions.
 */
class csIntersect3
{
public:
  /**
   * Intersect a 3D segment with a plane.  Returns true if there is an
   * intersection, with the intersection point returned in isect.
   * The distance from u to the intersection point is returned in dist.
   */
  static void Plane (
    const csVector3& u, const csVector3& v, // segment
    const csVector3& normal, const csVector3& a, // plane
    csVector3& isect);                    // intersection point

  /**
   * Intersect a 3D segment with a plane.  Returns true if there is an
   * intersection, with the intersection point returned in isect.
   * The distance from u to the intersection point is returned in dist.
   */
  static bool Plane (
    const csVector3& u, const csVector3& v, // segment
    float A, float B, float C, float D, // plane Ax+By+Cz+D=0
    csVector3& isect,                     // intersection point
    float& dist);                       // distance from u to isect

  /**
   * Intersect a 3D segment with a plane.  Returns true if there is an
   * interection, with the intersection point returned in isect.
   * The distance from u to the intersection point is returned in dist.
   */
  static bool Plane (
    const csVector3& u, const csVector3& v, // segment
    const csPlane& p,                     // plane Ax+By+Cz+D=0
    csVector3& isect,                     // intersection point
    float& dist);                       // distance from u to isect

  /**
   * Intersect a 3D segment with the z = 0 plane.  Assumes that there
   * is an intersection (fails if the segment is parallel to the plane),
   * and returns the distance from u to the intersection point.
   * The intersection point is returned in isect.
   */
  static float Z0Plane (
    const csVector3& u, const csVector3& v, // segment
    csVector3& isect);                    // intersection point

  /**
   * Intersect a 3D segment with the plane z = zval. Assumes that there
   * is an intersection (fails if the segment is parallel to the plane),
   * and returns the distance from u to the intersection point.
   * The intersection point is returned in isect.
   */
  static float ZPlane (float zval,      // plane z = zval
    const csVector3& u, const csVector3& v, // segment
    csVector3& isect);                    // intersection point

  /**
   * Intersect a 3D segment with the frustrum plane Ax + z = 0.
   * Assumes an intersection, and returns the intersection point in isect.
   */
  static float XFrustrum (
    float A, const csVector3& u, const csVector3& v, csVector3& isect);

  /**
   * Intersect a 3D segment with the frustrum plane By + z = 0.
   * Assumes an intersection, and returns the intersection point in isect.
   */
  static float YFrustrum (
    float B, const csVector3& u, const csVector3& v, csVector3& isect);
};


#endif /*__MATH3D_H__*/

