/*
    Copyright (C) 1997, 1998, 1999, 2000 by Alex Pfaffe
	(Digital Dawn Graphics Inc)
  
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
#ifndef _ddgMatrix_Class_
#define _ddgMatrix_Class_

#ifdef WIN32
#include <string.h>
#endif

#include "math/ddgvec.h"

/**
 * A simple general purpose 3x3 matrix object.
 */
class WEXP ddgMatrix3 {
public:
  /// A matrix is composed of 3 vectors.
  ddgVector3 m[3];
  /// Return an element from the matrix.
  ddgVector3 operator[](int n)            { return m[n]; }
  /// Set a matrix to the value of 3 vectors.
  ddgMatrix3( ddgVector3 *v1, ddgVector3 *v2, ddgVector3 *v3 ) 
    { m[0].set(v1); m[1].set(v2); m[2].set(v3); }
};

/**
 * A simple general purpose 4x4 matrix object.
 */
class WEXP ddgMatrix4 {
public:
	/// A matrix is composed of 4 vectors.
	ddgVector4 m[4];
	/// Default constructor.
	ddgMatrix4(void) { }
	/// Return an Vector element from the matrix.
	float& operator[](int i)
	{ return m[i/4].v[i%4]; }
	/// Get a matrix value using [i,j] notation.
	float& operator()(int i, int j)
	{ return m[i].v[j]; }
	/// Get a matrix value using a 0-15 index.
	float& operator()(int i)
	{ return m[i/4].v[i%4]; }
	/// Addition.
	ddgMatrix4& operator += ( const ddgMatrix4& m1 )
	{ m[0] += m1.m[0]; m[1] += m1.m[1]; m[2] += m1.m[2]; m[3] += m1.m[3]; return *this;}
	/// Subtraction.
	ddgMatrix4& operator -= ( const ddgMatrix4& m1 )
	{ m[0] -= m1.m[0]; m[1] -= m1.m[1]; m[2] -= m1.m[2]; m[3] -= m1.m[3]; return *this;}
	/// Multiplication of this by another matrix.
	ddgMatrix4& operator *= ( const ddgMatrix4& m1 );
	/// Multiply 2 matrices together and return result.
	inline friend ddgMatrix4 operator *( const ddgMatrix4& m1, const ddgMatrix4& m2 )
	{
	ddgMatrix4 out(m1);
	out *= m2;
	return out;
	}
	// Scale.
	ddgMatrix4& operator *= ( const float d )
	{ m[0] *= d; m[1] *= d; m[2] *= d; m[3] *= d; return *this;}
	// Division
	ddgMatrix4& operator /= ( const float d )
	{ m[0] /= d; m[1] /= d; m[2] /= d; m[3] /= d; return *this;}

	/// Assign one matrix to another.
	ddgMatrix4& operator=(const ddgMatrix4& s)
	{ m[0] = s.m[0]; m[1] = s.m[1]; m[2] = s.m[2]; m[3] = s.m[3]; return *this; }
	/// Load the identity matrix.
	void identity(void)
	{
		m[0].set(1,0,0,0);
		m[1].set(0,1,0,0);
		m[2].set(0,0,1,0);
		m[3].set(0,0,0,1);
	}
	inline friend ddgMatrix4 operator - (const ddgMatrix4& a)					// -m1
	{
		ddgMatrix4 m1;
		m1 = a;
		m1.m[0] *= -1; m1.m[1] *= -1; m1.m[2] *= -1; m1.m[3] *= -1;
		return m1;
	}
	inline friend ddgMatrix4 operator + (const ddgMatrix4& a, const ddgMatrix4& b)  // m1 + m2
	{
		ddgMatrix4 m1;
		m1.m[0] = a.m[0] + b.m[0]; m1.m[1] = a.m[1] + b.m[1]; m1.m[2] = a.m[2] + b.m[2]; m1.m[3] = a.m[3] + b.m[3]; 
		return m1;
	}
	inline friend ddgMatrix4 operator - (const ddgMatrix4& a, const ddgMatrix4& b)  // m1 - m2
	{
		ddgMatrix4 m1;
		m1.m[0] = a.m[0] - b.m[0]; m1.m[1] = a.m[1] - b.m[1]; m1.m[2] = a.m[2] - b.m[2]; m1.m[3] = a.m[3] - b.m[3]; 
		return m1;
	}
	inline friend ddgMatrix4 operator / (const ddgMatrix4& a, const float d)  // m1 / d
	{
		ddgMatrix4 m1;
		m1 = a;
		m1.m[0] /=d; m1.m[1] /=d; m1.m[2] /=d; m1.m[3] /=d; 
		return m1;
	}

	// Check equality
	inline friend bool operator == (const ddgMatrix4& a, const ddgMatrix4& b)  // m1 == m2 ?
	{
		return a.m[0] == b.m[0] && a.m[1] == b.m[1] && a.m[2] == b.m[2] && a.m[3] == b.m[3];
	}
	// Check inequality
	inline friend bool operator != (const ddgMatrix4& a, const ddgMatrix4& b)  // m1 != m2 ?
	{
		return (a.m[0] != b.m[0]) || (a.m[1] != b.m[1]) || (a.m[2] != b.m[2]) || (a.m[3] != b.m[3]);
	}
	/// Set a matrix to the value of 4 vectors.
	ddgMatrix4( ddgVector4 *v1, ddgVector4 *v2, ddgVector4 *v3, ddgVector4 *v4) 
	{ m[0].set(v1); m[1].set(v2); m[2].set(v3); m[3].set(v4); }
	/// Set a matrix to the value of a 16 element float array.
	ddgMatrix4( float u[16] )
	{ m[0].set(&u[0]);  m[1].set(&u[4]); m[2].set(&u[8]); m[3].set(&u[12]);}
	/// Assign the value of a matrix to a 16 element float array.
	void assignto( float u[16] )
	{ int i = 0; do { u[i] = m[i/4].v[i%4]; } while (++i < 16 ); }
	/// Return object a pointer to array of floats.
	operator float*() { return &m[0].v[0]; };
	/// Find the inverset matrix A' such that A A' = I (identity).
	void invert(void);
	/// Transpose this matrix.
	void transpose(void);
};

/// Multiply two matrices together, product may be equal to a.
void WEXP ddgMatrixMultiply( float product[], const float a[], const float b[] );   
/// Set a matrix for rotation about axis by a given angle.
void WEXP ddgRotationMatrix( float angle, float x, float y, float z, float m[] );
#endif
