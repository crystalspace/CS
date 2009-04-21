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

#ifndef __CS_MATRIX4_H__
#define __CS_MATRIX4_H__

/**\file 
 * 4x4 matrix.
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"
#include "csgeom/math.h"
#include "csgeom/transfrm.h"
#include "csgeom/vector4.h"
#include "csutil/csstring.h"

namespace CS
{
  namespace Math
  {
    /**
     * A 4x4 matrix.
     */
    class CS_CRYSTALSPACE_EXPORT Matrix4
    {
    public:
      float m11, m12, m13, m14;
      float m21, m22, m23, m24;
      float m31, m32, m33, m34;
      float m41, m42, m43, m44;
    
    public:
      /// Construct a matrix, initialized to be the identity.
      Matrix4 ()
	  : m11(1), m12(0), m13(0), m14(0),
	    m21(0), m22(1), m23(0), m24(0),
	    m31(0), m32(0), m33(1), m34(0),
	    m41(0), m42(0), m43(0), m44(1)
      {}
    
      /// Construct a matrix and initialize it.
      Matrix4 (float am11, float am12, float am13, float am14,
	       float am21, float am22, float am23, float am24,
               float am31, float am32, float am33, float am34,
	       float am41, float am42, float am43, float am44)
	  : m11(am11), m12(am12), m13(am13), m14(am14),
	    m21(am21), m22(am22), m23(am23), m24(am24),
	    m31(am31), m32(am32), m33(am33), m34(am34),
	    m41(am41), m42(am42), m43(am43), m44(am44)
      {}
    
      /// Copy constructor.
      Matrix4 (Matrix4 const& o)
	: m11(o.m11), m12(o.m12), m13(o.m13), m14(o.m14),
	  m21(o.m21), m22(o.m22), m23(o.m23), m24(o.m24),
	  m31(o.m31), m32(o.m32), m33(o.m33), m34(o.m34),
	  m41(o.m41), m42(o.m42), m43(o.m43), m44(o.m44)
      {
      }
      
      /// Construct from a transform
      Matrix4 (csTransform const& o)
	: m11(o.GetO2T().m11), m12(o.GetO2T().m12), m13(o.GetO2T().m13),
	  m21(o.GetO2T().m21), m22(o.GetO2T().m22), m23(o.GetO2T().m23),
	  m31(o.GetO2T().m31), m32(o.GetO2T().m32), m33(o.GetO2T().m33), 
	  m41(0), m42(0), m43(0), m44(1)
      {
	csVector3 o_t2o = -o.GetO2T()*o.GetO2TTranslation();
	m14 = o_t2o.x;
	m24 = o_t2o.y;
	m34 = o_t2o.z;
      }
    
      /// Construct from a 3x3 matrix
      Matrix4 (csMatrix3 const& m)
	: m11(m.m11), m12(m.m12), m13(m.m13), m14 (0),
	  m21(m.m21), m22(m.m22), m23(m.m23), m24 (0),
	  m31(m.m31), m32(m.m32), m33(m.m33), m34 (0),
	  m41(0), m42(0), m43(0), m44(1)
      {
      }
    
      /// Return a textual representation of the matrix
      csString Description() const;
	
	  /// Return a csTransform object representation of the matrix
	  csTransform GetTransform() const;
      
      /// Get the first row of this matrix as a vector.
      inline csVector4 Row1() const { return csVector4 (m11,m12,m13,m14); }
    
      /// Get the second row of this matrix as a vector.
      inline csVector4 Row2() const { return csVector4 (m21,m22,m23,m24); }
    
      /// Get the third row of this matrix as a vector.
      inline csVector4 Row3() const { return csVector4 (m31,m32,m33,m34); }
    
      /// Get the third row of this matrix as a vector.
      inline csVector4 Row4() const { return csVector4 (m41,m42,m43,m44); }
    
      /// Get a row from this matrix as a vector.
      inline csVector4 Row(size_t n) const
      {
	switch (n)
	{
	  case 0: return Row1();
	  case 1: return Row2();
	  case 2: return Row3();
	  default:
	  case 3: return Row4();
	}
      }
    
      /// Get the first column of this matrix as a vector.
      inline csVector4 Col1() const { return csVector4 (m11,m21,m31,m41); }
    
      /// Get the second column of this matrix as a vector.
      inline csVector4 Col2() const { return csVector4 (m12,m22,m32,m42); }
    
      /// Get the third column of this matrix as a vector.
      inline csVector4 Col3() const { return csVector4 (m13,m23,m33,m43); }
    
      /// Get the third column of this matrix as a vector.
      inline csVector4 Col4() const { return csVector4 (m14,m24,m34,m44); }
    
      /// Get a column from this matrix as a vector.
      inline csVector4 Col(size_t n) const
      {
	switch (n)
	{
	  case 0: return Col1();
	  case 1: return Col2();
	  case 2: return Col3();
	  default:
	  case 3: return Col4();
	}
      }
    
      inline void Set (Matrix4 const &o)
      {
	m11 = o.m11; m12 = o.m12; m13 = o.m13; m14 = o.m14;
	m21 = o.m21; m22 = o.m22; m23 = o.m23; m24 = o.m24;
	m31 = o.m31; m32 = o.m32; m33 = o.m33; m34 = o.m34;
	m41 = o.m41; m42 = o.m42; m43 = o.m43; m44 = o.m44;
      }
    
      /// Assign another matrix to this one.
      Matrix4& operator= (const Matrix4& o) { Set(o); return *this; }
    
      /// Scale complete matrix.
      Matrix4& operator*= (float f)
      {
	m11 *= f; m12 *= f; m13 *= f; m14 *= f;
	m21 *= f; m22 *= f; m23 *= f; m24 *= f;
	m31 *= f; m32 *= f; m33 *= f; m34 *= f;
	m41 *= f; m42 *= f; m43 *= f; m44 *= f;
	return *this;
      }
    
      /// Scale complete matrix.
      Matrix4& operator/= (float f) 
      {
        *this *= 1.0f/f;	
	return *this;
      }
    
      /// Multiply two matrices.
      friend CS_CRYSTALSPACE_EXPORT Matrix4 operator* (const Matrix4& m1, 
	const Matrix4& m2);
      
      /// Multiply matrix with a Vector
      csVector4 operator* (const csVector4& v) const
      {
	return csVector4 (
          m11*v.x + m12*v.y + m13*v.z + m14*v.w,
	  m21*v.x + m22*v.y + m23*v.z + m24*v.w,
	  m31*v.x + m32*v.y + m33*v.z + m34*v.w,
	  m41*v.x + m42*v.y + m43*v.z + m44*v.w);
      }
      
      /// Compute determinant of this matrix
      float Determinant() const
      {
	return m14 * m23 * m32 * m41-m13 * m24 * m32 * m41-m14 * m22 * m33 * m41+m12 * m24 * m33 * m41
	  + m13 * m22 * m34 * m41-m12 * m23 * m34 * m41-m14 * m23 * m31 * m42+m13 * m24 * m31 * m42
	  + m14 * m21 * m33 * m42-m11 * m24 * m33 * m42-m13 * m21 * m34 * m42+m11 * m23 * m34 * m42
	  + m14 * m22 * m31 * m43-m12 * m24 * m31 * m43-m14 * m21 * m32 * m43+m11 * m24 * m32 * m43
	  + m12 * m21 * m34 * m43-m11 * m22 * m34 * m43-m13 * m22 * m31 * m44+m12 * m23 * m31 * m44
	  + m13 * m21 * m32 * m44-m11 * m23 * m32 * m44-m12 * m21 * m33 * m44+m11 * m22 * m33 * m44;
      }
      
      /// Return the inverse of this matrix. 
      Matrix4 GetInverse() const
      {
	Matrix4 m (
	  m23*m34*m42 - m24*m33*m42 + m24*m32*m43 - m22*m34*m43 - m23*m32*m44 + m22*m33*m44,
	  m14*m33*m42 - m13*m34*m42 - m14*m32*m43 + m12*m34*m43 + m13*m32*m44 - m12*m33*m44,
          m13*m24*m42 - m14*m23*m42 + m14*m22*m43 - m12*m24*m43 - m13*m22*m44 + m12*m23*m44,
	  m14*m23*m32 - m13*m24*m32 - m14*m22*m33 + m12*m24*m33 + m13*m22*m34 - m12*m23*m34,
   
          m24*m33*m41 - m23*m34*m41 - m24*m31*m43 + m21*m34*m43 + m23*m31*m44 - m21*m33*m44,
	  m13*m34*m41 - m14*m33*m41 + m14*m31*m43 - m11*m34*m43 - m13*m31*m44 + m11*m33*m44,
          m14*m23*m41 - m13*m24*m41 - m14*m21*m43 + m11*m24*m43 + m13*m21*m44 - m11*m23*m44,
	  m13*m24*m31 - m14*m23*m31 + m14*m21*m33 - m11*m24*m33 - m13*m21*m34 + m11*m23*m34,
	
          m22*m34*m41 - m24*m32*m41 + m24*m31*m42 - m21*m34*m42 - m22*m31*m44 + m21*m32*m44,
	  m14*m32*m41 - m12*m34*m41 - m14*m31*m42 + m11*m34*m42 + m12*m31*m44 - m11*m32*m44,
          m12*m24*m41 - m14*m22*m41 + m14*m21*m42 - m11*m24*m42 - m12*m21*m44 + m11*m22*m44,
	  m14*m22*m31 - m12*m24*m31 - m14*m21*m32 + m11*m24*m32 + m12*m21*m34 - m11*m22*m34,

          m23*m32*m41 - m22*m33*m41 - m23*m31*m42 + m21*m33*m42 + m22*m31*m43 - m21*m32*m43,
	  m12*m33*m41 - m13*m32*m41 + m13*m31*m42 - m11*m33*m42 - m12*m31*m43 + m11*m32*m43,
          m13*m22*m41 - m12*m23*m41 - m13*m21*m42 + m11*m23*m42 + m12*m21*m43 - m11*m22*m43,
	  m12*m23*m31 - m13*m22*m31 + m13*m21*m32 - m11*m23*m32 - m12*m21*m33 + m11*m22*m33);
	m /= Determinant();
	return m;
      }

      /// Invert this matrix. 
      void Invert() { *this = GetInverse(); }
    
      /// Transpose this matrix.
      inline void Transpose ()
      {
        CS::Swap (m12, m21); CS::Swap (m13, m31); CS::Swap (m14, m41); 
        CS::Swap (m23, m32); CS::Swap (m24, m42); 
        CS::Swap (m34, m43); 
      }
    
      /// Return the transpose of this matrix.
      Matrix4 GetTranspose () const
      {
	return Matrix4 (
	  m11, m21, m31, m41,
	  m12, m22, m32, m42,
	  m13, m23, m33, m43,
	  m14, m24, m34, m44);
      }
    };

  } // namespace Math
} // namespace CS

/** @} */

#endif // __CS_MATRIX4_H__
