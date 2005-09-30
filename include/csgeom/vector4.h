/*
    Copyright (C) 1998,1999,2000 by Jorrit Tyberghein
    Largely rewritten by Ivan Avramovic <ivan@avramovic.com>
    Extended (and some methods removed) to 4 component by Marten Svanfeldt
    Templatized by Frank Richter

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
 * 4D vector.
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"
#include "csgeom/vector3.h"
#include "csutil/csstring.h"

/**
 * A 4D vector with variable type components.
 */
template<typename T>
class csVector4T
{
public:
#if !defined(__STRICT_ANSI__) && !defined(SWIG)
  union
  {
    struct 
    {
#endif
      /// The X component of the vector
      T x;
      /// The Y component of the vector
      T y;
      /// The Z component of the vector
      T z;
      /// The W component of the vector
      T w;
#if !defined(__STRICT_ANSI__) && !defined(SWIG)
    };
    /// All components
    T m[4];
  };
#endif
  /* Note: since T is used in an union above, it cannot have custom ctors.
   * So be careful when creating new Ts; e.g.: don't use T(x), but something
   * like T y = x.
   */
  
  /**
   * Make a new vector. The vector is not
   * initialized. This makes the code slightly faster.
   */
  csVector4T () {}

  /**
   * Make a new initialized vector.
   * Creates a new vector and initializes it to m*<1,1,1,1>.  To create
   * a vector initialized to the zero vector, use csVector4(0)
   */
  csVector4T (const T& m) : x(m), y(m), z(m), w(m) {}

  /// Make a new vector and initialize with the given values.
  csVector4T (const T& ix, const T& iy, const T& iz = T(0), 
    const T& iw = T(1))
  	: x(ix), y(iy), z(iz), w(iw) {}

  /// Copy Constructor.
  csVector4T (const csVector4T& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}

  /// Convert from a three-component vector. w is set to 1.
  csVector4T (const csVector3 &v) : x(v.x), y(v.y), z(v.z), w(1.0f) {}

  /// Assignment operator.
  template<typename T2>
  csVector4T& operator= (const csVector4T<T2>& other)
  {
    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
    return *this;
  }

  /// Return a textual representation of the vector in the form "x,y,z,w".
  csString Description() const
  { 
    csString str;
    str << x << "," << y << "," << z << "," << w;
    return str;
  }
    
  /// Add two vectors.
  inline friend csVector4T operator+ (const csVector4T& v1, 
    const csVector4T& v2)
  { return csVector4T(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z, v1.w+v2.w); }

  /// Subtract two vectors.
  inline friend csVector4T operator- (const csVector4T& v1, 
    const csVector4T& v2)
  { return csVector4T(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z, v1.w-v2.w); }


  /// Take the dot product of two vectors.
  inline friend float operator* (const csVector4T& v1, 
    const csVector4T& v2)
  { return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w; }

  /// Take the cross product of two vectors.
  inline friend csVector4T operator% (const csVector4T& v1, 
    const csVector4T& v2)
  {
    return csVector4T<T> (
      (v1.x*v2.y-v1.y*v2.x) + (v1.x*v2.z-v1.z*v2.x) + (v1.y*v2.z-v1.z*v2.y),
      (v1.z*v2.y-v1.y*v2.z) + (v1.y*v2.w-v1.w*v2.y) + (v1.z*v2.w-v1.w*v2.z),
      (v1.x*v2.z-v1.z*v2.x) + (v1.w*v2.x-v1.x*v2.w) + (v1.z*v2.w-v1.w*v2.z),
      (v1.y*v2.x-v1.x*v2.y) + (v1.w*v2.x-v1.x*v2.w) + (v1.w*v2.y-v1.y*v2.w) );
  }

  /// Take cross product of two vectors and put result in this vector.
  void Cross (const csVector4T & v1, const csVector4T & v2)
  {
    x = (v1.x*v2.y-v1.y*v2.x) + (v1.x*v2.z-v1.z*v2.x) + (v1.y*v2.z-v1.z*v2.y);
    y = (v1.z*v2.y-v1.y*v2.z) + (v1.y*v2.w-v1.w*v2.y) + (v1.z*v2.w-v1.w*v2.z);
    z = (v1.x*v2.z-v1.z*v2.x) + (v1.w*v2.x-v1.x*v2.w) + (v1.z*v2.w-v1.w*v2.z);
    w = (v1.y*v2.x-v1.x*v2.y) + (v1.w*v2.x-v1.x*v2.w) + (v1.w*v2.y-v1.y*v2.w);
  }

  /// Multiply a vector and a scalar.
  inline friend csVector4T operator* (const csVector4T& v, T f)
  { return csVector4T(v.x*f, v.y*f, v.z*f, v.w*f); }

  /// Multiply a vector and a scalar.
  inline friend csVector4T operator* (float f, const csVector4T& v)
  { return csVector4(v.x*f, v.y*f, v.z*f, v.w*f); }

  /// Multiply a vector and a scalar int.
  inline friend csVector4T operator* (const csVector4T& v, int f)
  { T _f = f; return v * _f; }

  /// Multiply a vector and a scalar int.
  inline friend csVector4T operator* (int f, const csVector4T& v)
  { T _f = f; return v * _f; }

  /// Divide a vector by a scalar.
  inline friend csVector4T operator/ (const csVector4T& v, T f)
  { f = 1.0f/f; return csVector4T(v.x*f, v.y*f, v.z*f, v.w*f); }

  /// Divide a vector by a scalar int.
  inline friend csVector4T operator/ (const csVector4T& v, int f)
  { T _f = f; return v / _f; }

  /// Check if two vectors are equal.
  inline friend bool operator== (const csVector4T& v1, const csVector4T& v2)
  { return v1.x==v2.x && v1.y==v2.y && v1.z==v2.z && v1.w==v2.w; }

  /// Check if two vectors are not equal.
  inline friend bool operator!= (const csVector4T& v1, const csVector4T& v2)
  { return v1.x!=v2.x || v1.y!=v2.y || v1.z!=v2.z || v1.w!=v2.w; }

  /// Project one vector onto another.
  inline friend csVector4T operator>> (const csVector4T& v1, const csVector4T& v2)
  { return v2*(v1*v2)/(v2*v2); }

  /// Project one vector onto another.
  inline friend csVector4T operator<< (const csVector4T& v1, const csVector4T& v2)
  { return v1*(v1*v2)/(v1*v1); }

  /// Test if each component of a vector is less than a small epsilon value.
  inline friend bool operator< (const csVector4T& v, float f)
  { return ABS(v.x)<f && ABS(v.y)<f && ABS(v.z)<f && ABS(v.w)<f; }

  /// Test if each component of a vector is less than a small epsilon value.
  inline friend bool operator> (float f, const csVector4T& v)
  { return ABS(v.x)<f && ABS(v.y)<f && ABS(v.z)<f && ABS(v.w)<f; }

  /// Returns n-th component of the vector.
#ifdef __STRICT_ANSI__
  inline float operator[] (int n) const 
  { return (n&2)?((n&1)?w:z):((n&1)?y:x); }
#else
  inline float operator[] (int n) const { return m[n]; }
#endif

  /// Returns n-th component of the vector.
#ifdef __STRICT_ANSI__
  inline float & operator[] (int n) 
  { return (n&2)?((n&1)?w:z):((n&1)?y:x); }
#else
  inline float & operator[] (int n) { return m[n]; }
#endif

  /// Add another vector to this vector.
  inline csVector4T& operator+= (const csVector4T& v)
  {
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;

    return *this;
  }

  /// Subtract another vector from this vector.
  inline csVector4T& operator-= (const csVector4T& v)
  {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;

    return *this;
  }

  /// Multiply this vector by a scalar.
  inline csVector4T& operator*= (T f)
  { x *= f; y *= f; z *= f; w *= f; return *this; }

  /// Divide this vector by a scalar.
  inline csVector4T& operator/= (T f)
  { f = 1.0f / f; x *= f; y *= f; z *= f; w *= f; return *this; }

  /// Unary + operator.
  inline csVector4T operator+ () const { return *this; }

  /// Unary - operator.
  inline csVector4T operator- () const { return csVector4(-x,-y,-z, -w); }

  /// Set the value of this vector.
  inline void Set (T sx, T sy, T sz, T sw)
  { x = sx; y = sy; z = sz; w = sw; }

  /// Set the value of this vector.
  inline void Set (csVector4T const& v) { x = v.x; y = v.y; z = v.z; w = v.w; }

  /// Set the value of this vector.
  inline void Set (T const* v) { x = v[0]; y = v[1]; z = v[2]; w = v[3]; }

  /// Set the value of this vector so that all components are the same.
  inline void Set (T v) { x = y = z = w = v; }

  /// Get the value of this vector.
  inline void Get (T* v) { v[0] = x; v[1] = y; v[2] = z; v[3] = w; }

  /// Returns the norm of this vector.
  T Norm () const { return sqrtf (x * x + y * y + z * z + w * w); }

  /// Return the squared norm (magnitude) of this vector.
  T SquaredNorm () const
  { return x * x + y * y + z * z + w * w; }

  /**
   * Returns the unit vector in the direction of this vector.
   * Attempting to normalize a zero-vector will result in a divide by
   * zero error.  This is as it should be... fix the calling code.
   */
  csVector4T Unit () const { return (*this)/(this->Norm()); }

  /// Returns the norm (magnitude) of a vector.
  inline static T Norm (const csVector4T& v) { return v.Norm(); }

  /// Normalizes a vector to a unit vector.
  inline static csVector4T Unit (const csVector4T& v) { return v.Unit(); }

  /// Scale this vector to length = 1.0;
  void Normalize ()
  {
    T sqlen = x * x + y * y + z * z + w * w;
    if (sqlen < SMALL_EPSILON) return ;
  
    T invlen = csQisqrt (sqlen);
    *this *= invlen;
  }


  /// Query if the vector is zero
  inline bool IsZero (T precision = SMALL_EPSILON) const
  { return (ABS(x) < precision) && (ABS(y) < precision)
            && (ABS(z) < precision) &&  (ABS(w) < precision);
  }
};

/**
 * A 4D vector with "float" components.
 */
class csVector4 : public csVector4T<float>
{
public:
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
  csVector4 (const float& m) : csVector4T<float> (m) {}

  /// Make a new vector and initialize with the given values.
  csVector4 (float ix, float iy, float iz = 0, float iw = 1)
  	: csVector4T<float> (ix, iy, iz, iw) {}

  /// Copy Constructor.
  csVector4 (const csVector4& v) : csVector4T<float> (v) {}

  /// Copy Constructor.
  csVector4 (const csVector4T<float>& v) : csVector4T<float> (v) {}

  /// Convert from a three-component vector. w is set to 1.
  csVector4 (const csVector3 &v) : 
    csVector4T<float> (v.x, v.y, v.z, 1.0f) {}

  /// Assignment operator.
  csVector4& operator= (const csVector4T<float>& other)
  {
    Set (other.x, other.y, other.z, other.w);
    return *this;
  }

  /// Assignment operator.
  csVector4& operator= (const csVector3& other)
  {
    Set (other.x, other.y, other.z, 1.0f);
    return *this;
  }
};

/** @} */

#endif // __CS_VECTOR3_H__
