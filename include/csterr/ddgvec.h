/*
    Copyright (C) 1997, 1998, 1999 by Alex Pfaffe
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
#ifndef _ddgVector_Class_
#define _ddgVector_Class_

#include "csterr/ddgutil.h"

/**
 * Two dimensional vector class.
 */
class WEXP ddgVector2 {
public:
	/// Three floating point values.
	float v[2];
	/// return the value of the vector as a float array.
	operator float* () { return v; }
	/// return a pointer to a vector from a vector.
	operator ddgVector2* () { return this; }
	/// Get value of one dimension of the vector.
	float operator[](int n) const             { return v[n]; }
	/// Test for equivalence.
	bool operator==(ddgVector2 u) { return v[0] == u[0] && v[1] == u[1]; }
	/// Test for inequivalence.
	bool operator!=(ddgVector2 u) { return v[0] != u[0] || v[1] != u[1]; }
	/// Assign the value of a float array to this vector.
	void assign( float *a)               { a[0] = v[0]; a[1] = v[1]; }
	/// Create undefine vector.
	ddgVector2(void )                       { }
	///
	ddgVector2(float *a )                   { v[0] = a[0]; v[1] = a[1]; }
	///
	ddgVector2(float a, float b )  { v[0] = a; v[1] = b; }
	/// Construct with initial values from another vector.
	ddgVector2( ddgVector2* u )                { v[0] = u->v[0]; v[1] = u->v[1]; }
	/// Assign to another vector.
	ddgVector2& operator=(const ddgVector2& s) { v[0] = s[0]; v[1] = s[1]; return *this; }
	/// Initialize vector to zero.
	void zero( void )                    { v[0] = v[1] = 0.0;}
	/// Set vector components.
	void set(float a, float b ) { v[0] = a; v[1] = b; }
	/// Set vector equal to another.
	void set( ddgVector2 *u )                { v[0] = u->v[0]; v[1] = u->v[1]; }
    /// Return vector components.
    void get(float *a, float *b) { *a = v[0]; *b = v[1]; }
	///
	void add( ddgVector2 *u )                { v[0] += u->v[0]; v[1] += u->v[1]; }
	/// Add two vectors
	inline friend ddgVector2 operator+( const ddgVector2& u, const ddgVector2& w) 
		{ return ddgVector2(u[0]+w[0],u[1]+w[1]); }
	/// Subtract u from this vector.
	void subtract( ddgVector2 *u )           { v[0] -= u->v[0]; v[1] -= u->v[1]; }
	/// Subtract two vectors
	inline friend ddgVector2 operator-( const ddgVector2& u, const ddgVector2& w) 
		{ return ddgVector2(u[0]-w[0],u[1]-w[1]); }
	/// Multiply each component of vector by s.
	void multiply( float s )             { v[0] *= s; v[1] *= s; }
	/// Multiply two vectors
	inline friend ddgVector2 operator*( const ddgVector2& u, const ddgVector2& w) 
		{ return ddgVector2(u[0]*w[0],u[1]*w[1]); }
	/// Multiply his this and a vector.
	void multiply( ddgVector2 *u )            { v[0] *= u->v[0]; v[1] *= u->v[1]; }
	/// Divide each component of vector by s.
	void divide( float s )               { float s2=1.0f/s; multiply(s2); }
	///
	void divide( ddgVector2* u )              { v[0] /= u->v[0]; v[1] /= u->v[1]; }
	/// Divide two vectors
	inline friend ddgVector2 operator/( const ddgVector2& u, const ddgVector2& w) 
		{ return ddgVector2(u[0]/w[0],u[1]/w[1]); }
	///
	float sizesq( void )                 { return (v[0]*v[0]+v[1]*v[1]); }
	///
	float size( void )                   { return sqrtf(sizesq()); }
	///
	bool null( void )                    { return (v[0] == 0.0 && v[1] == 0.0); }
	///
	void normalize( void )               { divide(size()); }
	/// Returns the angle between 2 normalized vectors.
	float angle( ddgVector2 *u )            { return ddgAngle::radtodef(acosf(dot(u))); }


	/// Calculate the dot product of this and u.
	float dot ( ddgVector2 *u )
	{ return u->v[0]*v[0]+ u->v[1]*v[1]; }
	/// Dot product of two vectors.
	inline friend float operator|( const ddgVector2& u, const ddgVector2& w) 
	{ return u[0]*w[0]+ u[1]*w[1]; }
	/// Reverse direction of vector.
	void inverse() {multiply(-1.0);}

	/// Adjust this object to minimum values of given vector.
	void minimum(ddgVector2* m )
    {
		if (m->v[0] < v[0] ) v[0] = m->v[0];
		if (m->v[1] < v[1] ) v[1] = m->v[1];
    }
    ///
	void maximum(ddgVector2* m )
    {
		if (m->v[0] > v[0] ) v[0] = m->v[0];
		if (m->v[1] > v[1] ) v[1] = m->v[1];
    }
};


        
#endif
