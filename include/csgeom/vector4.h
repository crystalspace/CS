/*
    Copyright (C) 1998,1999,2000 by Jorrit Tyberghein
    Largely rewritten by Ivan Avramovic <ivan@avramovic.com>
    Extended (and some methods removed) to 4 component by Marten
    Svanfeldt

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

#ifndef __CS_VECTOR4_H__
#define __CS_VECTOR4_H__

/**\file 
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"
#include "csgeom/vector3.h"

class csVector4;


/**
 * A 4D vector with "double" components.
 */
class CS_CRYSTALSPACE_EXPORT csDVector4
{
public:
  /// The X component of the vector
  double x;
  /// The Y component of the vector
  double y;
  /// The Z component of the vector
  double z;
  /// The W component of the vector
  double w;

  /**
   * Make a new vector. The vector is not
   * initialized. This makes the code slightly faster.
   */
  csDVector4 () {}

  /**
   * Make a new initialized vector.
   * Creates a new vector and initializes it to m*<1,1,1,1>.  To create
   * a vector initialized to the zero vector, use csDVector4(0)
   */
  csDVector4 (double m) : x(m), y(m), z(m), w(m) {}

  /// Make a new vector and initialize with the given values.
  csDVector4 (double ix, double iy, double iz = 0, double iw = 1)
  { x = ix; y = iy; z = iz; w = iw;}

  /// Copy Constructor.
  csDVector4 (const csDVector4& v) { x = v.x; y = v.y; z = v.z; w = v.w; }

  /// Conversion from single precision vector to double.
  csDVector4 (const csVector4&);

  /// Conversion from a three-component vector. w is set to 1.
  csDVector4 (const csDVector3& v) { x = v.x; y = v.y; z = v.z; w = 1.0; }

  /// Add two vectors.
  inline friend
  csDVector4 operator+ (const csDVector4& v1, const csDVector4& v2)
  { return csDVector4(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z, v1.w+v2.w); }

  /// Subtract two vectors.
  inline friend
  csDVector4 operator- (const csDVector4& v1, const csDVector4& v2)
  { return csDVector4(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z, v1.w-v2.w); }

  /// Take the dot product of two vectors.
  inline friend double operator* (const csDVector4& v1, const csDVector4& v2)
  { return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w; }

  /// Take the cross product of two vectors.
  inline friend csDVector4 operator% (const csDVector4& v1,
  	const csDVector4& v2)
  {
    
    return csDVector4 (
      (v1.x*v2.y-v1.y*v2.x) + (v1.x*v2.z-v1.z*v2.x) + (v1.y*v2.z-v1.z*v2.y),
      (v1.z*v2.y-v1.y*v2.z) + (v1.y*v2.w-v1.w*v2.y) + (v1.z*v2.w-v1.w*v2.z),
      (v1.x*v2.z-v1.z-v2.x) + (v1.w*v2.x-v1.x*v2.w) + (v1.z*v2.w-v1.w*v2.z),
      (v1.y*v2.x-v1.x*v2.y) + (v1.w*v2.x-v1.x*v2.w) + (v1.w*v2.y-v1.y*v2.w) );
  }

  /// Take cross product of two vectors and put result in this vector.
  void Cross (const csDVector4 & v1, const csDVector4 & v2)
  {
    x = (v1.x*v2.y-v1.y*v2.x) + (v1.x*v2.z-v1.z*v2.x) + (v1.y*v2.z-v1.z*v2.y);
    y = (v1.z*v2.y-v1.y*v2.z) + (v1.y*v2.w-v1.w*v2.y) + (v1.z*v2.w-v1.w*v2.z);
    z = (v1.x*v2.z-v1.z-v2.x) + (v1.w*v2.x-v1.x*v2.w) + (v1.z*v2.w-v1.w*v2.z);
    w = (v1.y*v2.x-v1.x*v2.y) + (v1.w*v2.x-v1.x*v2.w) + (v1.w*v2.y-v1.y*v2.w);
  }

  /// Multiply a vector and a scalar.
  inline friend csDVector4 operator* (const csDVector4& v, double f)
  { return csDVector4(v.x*f, v.y*f, v.z*f, v.w*f); }

  /// Multiply a vector and a scalar.
  inline friend csDVector4 operator* (double f, const csDVector4& v)
  { return csDVector4(v.x*f, v.y*f, v.z*f, v.w*f); }

  /// Divide a vector by a scalar.
  inline friend csDVector4 operator/ (const csDVector4& v, double f)
  { f = 1.0f/f; return csDVector4(v.x*f, v.y*f, v.z*f, v.w*f); }

  /// Check if two vectors are equal.
  inline friend bool operator== (const csDVector4& v1, const csDVector4& v2)
  { return v1.x==v2.x && v1.y==v2.y && v1.z==v2.z && v1.w == v2.w; }

  /// Check if two vectors are not equal.
  inline friend bool operator!= (const csDVector4& v1, const csDVector4& v2)
  { return v1.x!=v2.x || v1.y!=v2.y || v1.z!=v2.z || v1.w!=v2.w; }

  /// Project one vector onto another.
  inline friend
  csDVector4 operator>> (const csDVector4& v1, const csDVector4& v2)
  { return v2*(v1*v2)/(v2*v2); }

  /// Project one vector onto another.
  inline friend
  csDVector4 operator<< (const csDVector4& v1, const csDVector4& v2)
  { return v1*(v1*v2)/(v1*v1); }

  /// Test if each component of a vector is less than a small epsilon value.
  inline friend bool operator< (const csDVector4& v, double f)
  { return ABS(v.x)<f && ABS(v.y)<f && ABS(v.z)<f && ABS(v.w)<f; }

  /// Test if each component of a vector is greater than a small epsilon value.
  inline friend bool operator> (double f, const csDVector4& v)
  { return ABS(v.x)<f && ABS(v.y)<f && ABS(v.z)<f && ABS(v.w)<f; }

  /// Returns n'th component of the vector.
  inline double operator[](int n) const
  { return !n?x:n&1?y:n&2?z:w; }

  /// Returns n'th component of the vector.
  inline double & operator[](int n)
  { return !n?x:n&1?y:n&2?z:w; }

  /// Add another vector to this vector.
  inline csDVector4& operator+= (const csDVector4& v)
  {
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;

    return *this;
  }

  /// Subtract another vector from this vector.
  inline csDVector4& operator-= (const csDVector4& v)
  {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;

    return *this;
  }

  /// Multiply this vector by a scalar.
  inline csDVector4& operator*= (double f)
  { x *= f; y *= f; z *= f; w *= f; return *this; }

  /// Divide this vector by a scalar.
  inline csDVector4& operator/= (double f)
  { x /= f; y /= f; z /= f; w /= f; return *this; }

  /// Unary + operator.
  inline csDVector4 operator+ () const { return *this; }

  /// Unary - operator.
  inline csDVector4 operator- () const { return csDVector4(-x,-y,-z,-w); }

  /// Set the value of this vector.
  inline void Set (double sx, double sy, double sz, double sw)
  { x = sx; y = sy; z = sz; w = sw; }

  /// Returns the norm of this vector.
  double Norm () const;

  /// Returns the norm of this vector.
  double SquaredNorm () const;

  /**
   * Returns the unit vector in the direction of this vector.
   * Attempting to normalize a zero-vector will result in a divide by
   * zero error.  This is as it should be... fix the calling code.
   */
  csDVector4 Unit () const { return (*this)/(this->Norm()); }

  /// Returns the norm (magnitude) of a vector.
  inline static double Norm (const csDVector4& v) { return v.Norm(); }

  /// Normalizes a vector to a unit vector.
  inline static csDVector4 Unit (const csDVector4& v) { return v.Unit(); }

  /// Scale this vector to length = 1.0.
  void Normalize();
};


/**
 * A 4D vector with "float" components.
 */
class CS_CRYSTALSPACE_EXPORT csVector4
{
public:
  /// The X component of the vector
  float x;
  /// The Y component of the vector
  float y;
  /// The Z component of the vector
  float z;
  /// The W component of the vector
  float w;

  /**
   * Make a new vector. The vector is not
   * initialized. This makes the code slightly faster.
   */
  csVector4 () {}

  /**
   * Make a new initialized vector.
   * Creates a new vector and initializes it to m*<1,1,1,1>.  To create
   * a vector initialized to the zero vector, use csVector4(0)
   */
  csVector4 (float m) : x(m), y(m), z(m), w(m) {}

  /// Make a new vector and initialize with the given values.
  csVector4 (float ix, float iy, float iz = 0, float iw = 1)
  	: x(ix), y(iy), z(iz), w(iw) {}

  /// Copy Constructor.
  csVector4 (const csVector4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}

  /// Copy from a double-vector
  csVector4 (const csDVector4 &v);

  /// Convert from a three-component vector. w is set to 1.
  csVector4 (const csVector3 &v) : x(v.x), y(v.y), z(v.z), w(1.0f) {}

  /// Add two vectors.
  inline friend csVector4 operator+ (const csVector4& v1, const csVector4& v2)
  { return csVector4(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z, v1.w+v2.w); }

  /// Add two vectors of differing type, raise the csVector4 to csDVector4.
  inline friend csDVector4 operator+ (const csDVector4& v1, const csVector4& v2)
  { return csDVector4(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z, v1.w+v2.w); }

  /// Add two vectors of differing type, raise the csVector4 to csDVector4.
  inline friend csDVector4 operator+ (const csVector4& v1, const csDVector4& v2)
  { return csDVector4(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z, v1.w+v2.w); }

  /// Subtract two vectors.
  inline friend csVector4 operator- (const csVector4& v1, const csVector4& v2)
  { return csVector4(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z, v1.w-v2.w); }

  /// Subtract two vectors of differing type, raise the csVector4 to csDVector4.
  inline friend csDVector4 operator- (const csVector4& v1, const csDVector4& v2)
  { return csDVector4(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z, v1.w-v2.w); }

  /// Subtract two vectors of differing type, raise the csVector4 to csDVector4.
  inline friend csDVector4 operator- (const csDVector4& v1, const csVector4& v2)
  { return csDVector4(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z, v1.w-v2.w); }


  /// Take the dot product of two vectors.
  inline friend float operator* (const csVector4& v1, const csVector4& v2)
  { return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w; }

  /// Take the cross product of two vectors.
  inline friend csVector4 operator% (const csVector4& v1, const csVector4& v2)
  {
    return csVector4 (
      (v1.x*v2.y-v1.y*v2.x) + (v1.x*v2.z-v1.z*v2.x) + (v1.y*v2.z-v1.z*v2.y),
      (v1.z*v2.y-v1.y*v2.z) + (v1.y*v2.w-v1.w*v2.y) + (v1.z*v2.w-v1.w*v2.z),
      (v1.x*v2.z-v1.z*v2.x) + (v1.w*v2.x-v1.x*v2.w) + (v1.z*v2.w-v1.w*v2.z),
      (v1.y*v2.x-v1.x*v2.y) + (v1.w*v2.x-v1.x*v2.w) + (v1.w*v2.y-v1.y*v2.w) );
  }

  /// Take cross product of two vectors and put result in this vector.
  void Cross (const csVector4 & v1, const csVector4 & v2)
  {
    x = (v1.x*v2.y-v1.y*v2.x) + (v1.x*v2.z-v1.z*v2.x) + (v1.y*v2.z-v1.z*v2.y);
    y = (v1.z*v2.y-v1.y*v2.z) + (v1.y*v2.w-v1.w*v2.y) + (v1.z*v2.w-v1.w*v2.z);
    z = (v1.x*v2.z-v1.z*v2.x) + (v1.w*v2.x-v1.x*v2.w) + (v1.z*v2.w-v1.w*v2.z);
    w = (v1.y*v2.x-v1.x*v2.y) + (v1.w*v2.x-v1.x*v2.w) + (v1.w*v2.y-v1.y*v2.w);
  }

  /// Multiply a vector and a scalar.
  inline friend csVector4 operator* (const csVector4& v, float f)
  { return csVector4(v.x*f, v.y*f, v.z*f, v.w*f); }

  /// Multiply a vector and a scalar.
  inline friend csVector4 operator* (float f, const csVector4& v)
  { return csVector4(v.x*f, v.y*f, v.z*f, v.w*f); }

  /// Multiply a vector and a scalar double. Upgrade v to csDVector4.
  inline friend csDVector4 operator* (const csVector4& v, double f)
  { return csDVector4(v) * f; }

  /// Multiply a vector and a scalar double. Upgrade v to csDVector4.
  inline friend csDVector4 operator* (double f, const csVector4& v)
  { return csDVector4(v) * f; }

  /// Multiply a vector and a scalar int.
  inline friend csVector4 operator* (const csVector4& v, int f)
  { return v * (float)f; }

  /// Multiply a vector and a scalar int.
  inline friend csVector4 operator* (int f, const csVector4& v)
  { return v * (float)f; }

  /// Divide a vector by a scalar.
  inline friend csVector4 operator/ (const csVector4& v, float f)
  { f = 1.0f/f; return csVector4(v.x*f, v.y*f, v.z*f, v.w*f); }

  /// Divide a vector by a scalar double. Upgrade v to csDVector4.
  inline friend csDVector4 operator/ (const csVector4& v, double f)
  { return csDVector4(v) / f; }

  /// Divide a vector by a scalar int.
  inline friend csVector4 operator/ (const csVector4& v, int f)
  { return v / (float)f; }

  /// Check if two vectors are equal.
  inline friend bool operator== (const csVector4& v1, const csVector4& v2)
  { return v1.x==v2.x && v1.y==v2.y && v1.z==v2.z && v1.w==v2.w; }

  /// Check if two vectors are not equal.
  inline friend bool operator!= (const csVector4& v1, const csVector4& v2)
  { return v1.x!=v2.x || v1.y!=v2.y || v1.z!=v2.z || v1.w!=v2.w; }

  /// Project one vector onto another.
  inline friend csVector4 operator>> (const csVector4& v1, const csVector4& v2)
  { return v2*(v1*v2)/(v2*v2); }

  /// Project one vector onto another.
  inline friend csVector4 operator<< (const csVector4& v1, const csVector4& v2)
  { return v1*(v1*v2)/(v1*v1); }

  /// Test if each component of a vector is less than a small epsilon value.
  inline friend bool operator< (const csVector4& v, float f)
  { return ABS(v.x)<f && ABS(v.y)<f && ABS(v.z)<f && ABS(v.w)<f; }

  /// Test if each component of a vector is less than a small epsilon value.
  inline friend bool operator> (float f, const csVector4& v)
  { return ABS(v.x)<f && ABS(v.y)<f && ABS(v.z)<f && ABS(v.w)<f; }

  /// Returns n-th component of the vector.
  inline float operator[] (int n) const
  { return !n?x:n&1?y:n&2?z:w; }

  /// Returns n-th component of the vector.
  inline float & operator[] (int n)
  { return !n?x:n&1?y:n&2?z:w; }

  /// Add another vector to this vector.
  inline csVector4& operator+= (const csVector4& v)
  {
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;

    return *this;
  }

  /// Subtract another vector from this vector.
  inline csVector4& operator-= (const csVector4& v)
  {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;

    return *this;
  }

  /// Multiply this vector by a scalar.
  inline csVector4& operator*= (float f)
  { x *= f; y *= f; z *= f; w *= f; return *this; }

  /// Divide this vector by a scalar.
  inline csVector4& operator/= (float f)
  { f = 1.0f / f; x *= f; y *= f; z *= f; w *= f; return *this; }

  /// Unary + operator.
  inline csVector4 operator+ () const { return *this; }

  /// Unary - operator.
  inline csVector4 operator- () const { return csVector4(-x,-y,-z, -w); }

  /// Set the value of this vector.
  inline void Set (float sx, float sy, float sz, float sw)
  { x = sx; y = sy; z = sz; w = sw; }

  /// Set the value of this vector.
  inline void Set (csVector4 const& v) { x = v.x; y = v.y; z = v.z; w = v.w; }

  /// Set the value of this vector.
  inline void Set (float const* v) { x = v[0]; y = v[1]; z = v[2]; w = v[3]; }

  /// Set the value of this vector so that all components are the same.
  inline void Set (float v) { x = y = z = w = v; }

  /// Get the value of this vector.
  inline void Get (float* v) { v[0] = x; v[1] = y; v[2] = z; v[3] = w; }

  /// Returns the norm of this vector.
  float Norm () const;

  /// Return the squared norm (magnitude) of this vector.
  float SquaredNorm () const
  { return x * x + y * y + z * z + w * w; }

  /**
   * Returns the unit vector in the direction of this vector.
   * Attempting to normalize a zero-vector will result in a divide by
   * zero error.  This is as it should be... fix the calling code.
   */
  csVector4 Unit () const { return (*this)/(this->Norm()); }

  /// Returns the norm (magnitude) of a vector.
  inline static float Norm (const csVector4& v) { return v.Norm(); }

  /// Normalizes a vector to a unit vector.
  inline static csVector4 Unit (const csVector4& v) { return v.Unit(); }

  /// Scale this vector to length = 1.0;
  void Normalize ();

  /// Query if the vector is zero
  inline bool IsZero (float precision = SMALL_EPSILON) const
  { return (ABS(x) < precision) && (ABS(y) < precision)
            && (ABS(z) < precision) &&  (ABS(w) < precision);
  }
};

/** @} */

#endif // __CS_VECTOR3_H__
