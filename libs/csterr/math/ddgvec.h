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
#ifndef _ddgVector_Class_
#define _ddgVector_Class_

#include "util/ddgutil.h"

/**
 * Two dimensional vector class.
 */
class WEXP ddgVector2 {
public:
	/// Two floating point values.
	float v[2];
	/// return the value of the vector as a float array.
	operator float* () { return v; }
	/// Get value of one dimension of the vector.
	inline float operator[](int n) const             { return v[n]; }
	/// Returns n-th component of the vector.
	inline float & operator[] (int n) { return v[n]; }
	/// Test for equivalence.
	bool operator==(ddgVector2 u) { return v[0] == u[0] && v[1] == u[1]; }
	/// Test for inequivalence.
	bool operator!=(ddgVector2 u) { return v[0] != u[0] || v[1] != u[1]; }
	/// Test for equivalence
	inline friend bool operator == (const ddgVector2& u, const ddgVector2& v)
	{ return v[0] == u[0] && v[1] == u[1]; 
	}
	/// Test for inequivalence.
	inline friend bool operator != (const ddgVector2& u, const ddgVector2& v)
	{ return v[0] != u[0] || v[1] != u[1]; 
	}
	/// Assign the value of a float array to this vector.
	void assign( float *a)               { a[0] = v[0]; a[1] = v[1]; }
	/// Create undefine vector.
	ddgVector2(void )                       { }
	///
	ddgVector2(float *a )                   { v[0] = a[0]; v[1] = a[1]; }
	///
	ddgVector2(float a, float b )  { v[0] = a; v[1] = b; }
	/// Construct with initial values from another vector.
	ddgVector2( const ddgVector2* u )                { v[0] = u->v[0]; v[1] = u->v[1]; }
	/// Assign to another vector.
	ddgVector2& operator=(const ddgVector2& s) { v[0] = s[0]; v[1] = s[1]; return *this; }
	/// Initialize vector to zero.
	void zero( void )                    { v[0] = v[1] = 0.0;}
	/// Set vector components.
	void set(float a, float b ) { v[0] = a; v[1] = b; }
	/// Set vector equal to another.
	void set( const ddgVector2 *u )                { v[0] = u->v[0]; v[1] = u->v[1]; }
    /// Return vector components.
    void get(float *a, float *b) { *a = v[0]; *b = v[1]; }
	///
	void add( const ddgVector2 *u )                { v[0] += u->v[0]; v[1] += u->v[1]; }
	/// Add two vectors
	ddgVector2& operator+=( const ddgVector2& u) 
		{ v[0]+=u[0]; v[1] += u[1]; return *this; }
	/// Subtract two vectors
	ddgVector2& operator-=( const ddgVector2& u) 
		{ v[0]-=u[0]; v[1] -= u[1]; return *this; }
	/// Multiply two vectors
	ddgVector2& operator*=( const ddgVector2& u) 
		{ v[0]*=u[0]; v[1] *= u[1]; return *this; }
	/// Multiply vector by scalar.
	ddgVector2& operator*=( const float d) 
		{ v[0]*=d; v[1] *= d; return *this; }
	/// Divide two vectors
	ddgVector2& operator/=( const ddgVector2& u) 
		{ v[0]/=u[0]; v[1] /= u[1]; return *this; }
	/// Divide vector by scalar.
	ddgVector2& operator/=( const float d) 
		{ v[0]/=d; v[1] /= d; return *this; }
	/// Add two vectors
	inline friend ddgVector2 operator+( const ddgVector2& u, const ddgVector2& w) 
		{ return ddgVector2(u[0]+w[0],u[1]+w[1]); }
	/// Subtract u from this vector.
	void subtract( ddgVector2 *u )           { v[0] -= u->v[0]; v[1] -= u->v[1]; }
	/// Subtract two vectors
	inline friend ddgVector2 operator-( const ddgVector2& u, const ddgVector2& w) 
		{ return ddgVector2(u[0]-w[0],u[1]-w[1]); }
	/// Scale vector.
	void multiply( float s )             { v[0] *= s; v[1] *= s; }
	/// Multiply two vectors
	inline friend ddgVector2 operator*( const ddgVector2& u, const ddgVector2& w) 
		{ return ddgVector2(u[0]*w[0],u[1]*w[1]); }
	/// Multiply this and a vector.
	void multiply( const ddgVector2 *u )            { v[0] *= u->v[0]; v[1] *= u->v[1]; }
	/// Divide each component of vector by s.
	void divide( float s )               { float s2=1.0f/s; multiply(s2); }
	///
	void divide( const ddgVector2* u )              { v[0] /= u->v[0]; v[1] /= u->v[1]; }
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
	float angle( const ddgVector2 *u )            { return ddgAngle::radtodeg(acosf(dot(u))); }


	/// Calculate the dot product of this and u.
	float dot ( const ddgVector2 *u )
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
	/// Scale a vector by a constant and assign result.
	inline friend ddgVector2 operator*( const float d, const ddgVector2& u) 
	{ return ddgVector2(u[0]*d,u[1]*d); }
	/// Scale a vector by a constant and assign result.
	inline friend ddgVector2 operator*( const ddgVector2& u, const float d) 
	{ return ddgVector2(u[0]*d,u[1]*d); }
	/// Divide a vector by a constant and assign result.
	inline friend ddgVector2 operator/( const ddgVector2& u, const float d) 
	{ return ddgVector2(u[0]/d,u[1]/d); }
};

#ifdef DDGSTREAM
///
WEXP ostream& WFEXP operator << ( ostream&s, ddgVector2 v );
///
WEXP ostream& WFEXP operator << ( ostream&s, const ddgVector2* v );
///
WEXP istream& WFEXP operator >> ( istream& s, ddgVector2& v);
#endif // DDGSTREAM

///
class Matrix3;
/**
 * Three dimensional vector class.
 */
class WEXP ddgVector3 {
public:
	/// Three floating point values.
	float v[3];
	/// return the value of the vector as a float array.
	operator float* () { return v; }
	/// Get value of one dimension of the vector.
	inline float operator[](int n) const             { return v[n]; }
	/// Returns n-th component of the vector.
	inline float & operator[] (int n) { return v[n]; }
	/// Test for equivalence.
	bool operator==(ddgVector3& u) { return v[0] == u[0] && v[1] == u[1] && v[2] == u[2]; }
	/// Test for inequivalence.
	bool operator!=(ddgVector3& u) { return v[0] != u[0] || v[1] != u[1] || v[2] != u[2]; }
	/// Test for equivalence
	inline friend bool operator == (const ddgVector3& u, const ddgVector3& v)
	{ return v[0] == u[0] && v[1] == u[1] && v[2] == u[2]; 
	}
	/// Test for inequivalence.
	inline friend bool operator != (const ddgVector3& u, const ddgVector3& v)
	{ return v[0] != u[0] || v[1] != u[1] || v[2] != u[2]; 
	}
	/// Assign the value of a float array to this vector.
	void assign( float *a)               { a[0] = v[0]; a[1] = v[1]; a[2] = v[2]; }
	/// Create undefine vector.
	ddgVector3(void )                       { }
	///
	ddgVector3(float *a )                   { v[0] = a[0]; v[1] = a[1]; v[2] = a[2]; }
	///
	ddgVector3(float a, float b, float c )  { v[0] = a; v[1] = b; v[2] = c;}
	/// Construct with initial values from another vector.
	ddgVector3( const ddgVector3* u )       { v[0] = u->v[0]; v[1] = u->v[1]; v[2] = u->v[2]; }
	/// Assign to another vector.
	ddgVector3& operator=(const ddgVector3& s) { v[0] = s[0]; v[1] = s[1]; v[2] = s[2]; return *this; }
	/// Initialize vector to zero.
	void zero( void )                    { v[0] = v[1] = v[2] = 0.0;}
	///
	void set(float a, float b, float c ) { v[0] = a; v[1] = b; v[2] = c; }
	///
	void set( const ddgVector3 *u )      { v[0] = u->v[0]; v[1] = u->v[1]; v[2] = u->v[2]; }
    ///
    void get(float *a, float *b, float *c) { *a = v[0]; *b = v[1]; *c = v[2]; }
	///
	void add( const ddgVector3 *u )      { v[0] += u->v[0]; v[1] += u->v[1]; v[2] += u->v[2]; }
	/// Add two vectors
	ddgVector3& operator+=( const ddgVector3& u) 
		{ v[0]+=u[0]; v[1] += u[1]; v[2] += u[2]; return *this; }
	/// Subtract two vectors
	ddgVector3& operator-=( const ddgVector3& u) 
		{ v[0]-=u[0]; v[1] -= u[1]; v[2] -= u[2]; return *this; }
	/// Multiply two vectors
	ddgVector3& operator*=( const ddgVector3& u) 
		{ v[0]*=u[0]; v[1] *= u[1]; v[2] *= u[2]; return *this; }
	/// Multiply vector by scalar
	ddgVector3& operator*=( const float d) 
		{ v[0]*= d; v[1] *= d; v[2] *= d; return *this; }
	/// Divide two vectors
	ddgVector3& operator/=( const ddgVector3& u) 
		{ v[0]/=u[0]; v[1] /= u[1]; v[2] /= u[2]; return *this; }
	/// Divide vector by scalar
	ddgVector3& operator/=( const float d) 
		{ v[0]/= d; v[1] /= d; v[2] /= d; return *this; }
	/// Add two vectors
	inline friend ddgVector3 operator+( const ddgVector3& u, const ddgVector3& w) 
		{ return ddgVector3(u[0]+w[0],u[1]+w[1],u[2]+w[2]); }
	///
	void subtract( const ddgVector3 *u ) { v[0] -= u->v[0]; v[1] -= u->v[1]; v[2] -= u->v[2]; }
	/// Subtract two vectors
	inline friend ddgVector3 operator-( const ddgVector3& u, const ddgVector3& w) 
		{ return ddgVector3(u[0]-w[0],u[1]-w[1],u[2]-w[2]); }
	///
	void multiply( float s )             { v[0] *= s; v[1] *= s; v[2] *= s; }
	/// Multiply two vectors
	inline friend ddgVector3 operator*( const ddgVector3& u, const ddgVector3& w) 
		{ return ddgVector3(u[0]*w[0],u[1]*w[1],u[2]*w[2]); }
	/// Multiply his this and a vector.
	void multiply( const ddgVector3 *u ) { v[0] *= u->v[0]; v[1] *= u->v[1]; v[2] *= u->v[2]; }
	///
	void divide( float s )               { float s2=1.0f/s; multiply(s2); }
	///
	void divide( const ddgVector3* u )   { v[0] /= u->v[0]; v[1] /= u->v[1]; v[2] /= u->v[2]; }
	/// Divide two vectors
	inline friend ddgVector3 operator/( const ddgVector3& u, const ddgVector3& w) 
		{ return ddgVector3(u[0]/w[0],u[1]/w[1],u[2]/w[2]); }
	///
	float sizesq( void )                 { return (v[0]*v[0]+v[1]*v[1]+v[2]*v[2]); }
	///
	float size( void )                   { return sqrtf(sizesq()); }
	///
	bool null( void )                    { return (v[0] == 0.0 && v[1] == 0.0 && v[2] == 0.0); }
	///
	void normalize( void )               { divide(size()); }
	/// Returns the angle between 2 normalized vectors.
	float angle( const ddgVector3 *u )   { return ddgAngle::radtodeg(acosf(dot(u))); }
	/** Convert this vector to yaw, pitch, roll angles in degrees.
	 * QUAKE II   UNTESTED
	 * See angletovector, problably needs correction for Left Handed coords sys.
     */
    void vectortoangle( void )
    {
		float   yaw, pitch;

		if (v[2] == 0 && v[0] == 0)  // Directly overhead or underfoot.
		{
			yaw = 0;
			if (v[1] > 0) pitch = 90.0f;
			else          pitch = 270.0f;
		}
		else
		{
			yaw = (float)(int) 90+ddgAngle::radtodeg(atan2f(v[2], v[0]));
			yaw = ddgAngle::mod(yaw);

			pitch = (float)(int) ddgAngle::radtodeg(atan2f(v[1], sqrtf(v[0]*v[0] + v[2]*v[2])));
			pitch = ddgAngle::mod(pitch);
		}
		v[PITCH] = -pitch;
		v[YAW] = yaw;
		v[ROLL] = 0;
	}

	/**
	 * Convert angles of this to vectors for forward, right and up.
	 * Any of the output vectors are optional.
	 * Quake II q_shared (73)
	 * For Q Right All angles at 0 looks down the X axis. Y is left.  Z is up.
	 * Right handed system.
	 * All angles at 0 looks down the -Z axis. X is right.  Y is up.
	 * Left handed system.
	 */
	void angletovector (ddgVector3 *forward, ddgVector3 *right =0, ddgVector3 *up =0)
    {
		float           angle;
		static float    sr, sp, sy, cr, cp, cy;

		angle = ddgAngle::degtorad(v[YAW]);
		sy = sinf(angle);
		cy = cosf(angle);
		angle = ddgAngle::degtorad(v[PITCH]);
		sp = sinf(angle);
		cp = cosf(angle);
		angle = ddgAngle::degtorad(v[ROLL]);
		sr = sinf(angle);
		cr = cosf(angle);

		if (forward)
		{
			forward->v[2] = -cp*cy;
			forward->v[0] = -cp*sy;
			forward->v[1] = -sp;
		}
		if (right)
		{
			right->v[2] = (sr*sp*cy+cr*-sy);
			right->v[0] = (sr*sp*sy+cr*cy);
			right->v[1] = -1*sr*cp;
		}
		if (up)
		{
			up->v[2] = (-cr*sp*cy+sr*-sy);
			up->v[0] = (-cr*sp*sy+sr*cy);
			up->v[1] = cr*cp;
		}
    }    

	/// Assign the cross product of u X w to this.
	void cross( const ddgVector3 *u, const ddgVector3 *w )
    {
		v[0] = u->v[2]*w->v[1] - u->v[1]*w->v[2];
		v[1] = u->v[0]*w->v[2] - u->v[2]*w->v[0];
		v[2] = u->v[1]*w->v[0] - u->v[0]*w->v[1];
	}
	/// Cross product of two vectors.
	inline friend ddgVector3 operator%( const ddgVector3& u, const ddgVector3& w) 
    {
		return ddgVector3( u[2]*w[1] - u[1]*w[2],
		                u[0]*w[2] - u[2]*w[0],
		                u[1]*w[0] - u[0]*w[1]);
	}

	/// Calculate the dot product of this and u.
	float dot ( const ddgVector3 *u )
	{ return u->v[0]*v[0]+ u->v[1]*v[1]+ u->v[2]*v[2]; }
	/// Dot product of two vectors.
	inline friend float operator|( const ddgVector3& u, const ddgVector3& w) 
	{ return u[0]*w[0]+ u[1]*w[1]+ u[2]*w[2]; }
	/// Reverse direction of vector.
	void inverse() {multiply(-1.0);}
	// Calculate the normal for 3 non colinear points in space.
	/// Assumes p0 p1 and p2 are in counterclockwise order.
	void normal( const ddgVector3 *p0, const ddgVector3 *p1, const ddgVector3 *p2 )
    {
		ddgVector3 u( *p0 - *p1 ), v(*p2 - *p1);
		cross(&u,&v);
    }
	// Calculate a nicer normal useful for regular grids.
	/// Uses all neigbouring points.
	void normal( const ddgVector3* v[9] );
	// Return the reflection vector of this, given a normal.
	// UNTESTED.
	/// Renderman spec.
	void reflect( ddgVector3 *n)  // I - 2*(I.N)*N
    {
		ddgVector3 r(this);
		r.multiply(-2*r.dot(n));
		r.multiply(n);
		r.add(this);
		set(&r);
    }
	/** Refrect a vector w.r.t. a normal and a relative index of refraction.
	 * eta is the ratio of the index of refraction inthe volume containing the
	 * indicent vector to that of the volume being entered.  This vector is
	 * computed using Snell's law.
	 * UNTESTED.
	 * Renderman spec.
	 */
	void refract( const ddgVector3 *n, float eta )
    {
		float IdotN = dot(n);
		float k = 1 - eta*eta*(1- IdotN*IdotN);
		if (k < 0) set(0,0,0);
		else
		{
			ddgVector3 t(n);
			t.multiply(eta*IdotN+sqrtf(k));
			multiply(eta);
			subtract(&t);
		}
	}
	/// Adjust this object to minimum values of given vector.
	void minimum(const ddgVector3* m )
    {
		if (m->v[0] < v[0] ) v[0] = m->v[0];
		if (m->v[1] < v[1] ) v[1] = m->v[1];
		if (m->v[2] < v[2] ) v[2] = m->v[2];
    }
    /// Adjust this object to maximum values of given vector.
	void maximum(const ddgVector3* m )
    {
		if (m->v[0] > v[0] ) v[0] = m->v[0];
		if (m->v[1] > v[1] ) v[1] = m->v[1];
		if (m->v[2] > v[2] ) v[2] = m->v[2];
    }
	/// Scale a vector by a constant and assign result.
	inline friend ddgVector3 operator*( const float d, const ddgVector3& u) 
	{ return ddgVector3(u[0]*d,u[1]*d,u[2]*d); }
	/// Scale a vector by a constant and assign result.
	inline friend ddgVector3 operator*( const ddgVector3& u, const float d) 
	{ return ddgVector3(u[0]*d,u[1]*d,u[2]*d); }
	/// Divide a vector by a constant and assign result.
	inline friend ddgVector3 operator/( const ddgVector3& u, const float d) 
	{ return ddgVector3(u[0]/d,u[1]/d,u[2]/d); }
};
#ifdef DDGSTREAM
///
WEXP ostream& WFEXP operator << ( ostream&s, ddgVector3 v );
///
WEXP ostream& WFEXP operator << ( ostream&s, const ddgVector3* v );
///
WEXP istream& WFEXP operator >> ( istream& s, ddgVector3& v);
#endif
/**
 * Four dimensional vector class.
 */
class ddgMatrix4;
class WEXP ddgVector4 {
public:
	/// 4 components of vector.
	float v[4];
	/// Cast operator to use object as a float array.
	operator float* () { return v; }
	/// Get value of one dimension of the vector.
	inline float operator[](int n) const             { return v[n]; }
	/// Returns n-th component of the vector.
	inline float & operator[] (int n) { return v[n]; }
	/// Test for equivalence.
	bool operator==(ddgVector4& u)
	{ return v[0] == u[0] && v[1] == u[1] && v[2] == u[2] && v[3] == u[3]; }
	/// Test for inequivalence.
	bool operator!=(ddgVector4& u)
	{ return v[0] != u[0] || v[1] != u[1] || v[2] != u[2] || v[3] != u[3]; }
	/// Test for equivalence
	inline friend bool operator == (const ddgVector4& u, const ddgVector4& v)
	{ return v[0] == u[0] && v[1] == u[1] && v[2] == u[2] && v[3] == u[3]; 
	}
	/// Test for inequivalence.
	inline friend bool operator != (const ddgVector4& u, const ddgVector4& v)
	{ return v[0] != u[0] || v[1] != u[1] || v[2] != u[2] || v[3] != u[3]; 
	}
	/// Create uninitialized.
	ddgVector4(void )                                { }
	/// Initialize from 4 floats.
	ddgVector4(float a, float b, float c, float d )  { v[0] = a; v[1] = b; v[2] = c; v[3] = d;}
	/// Initialize from a pointer to a Vector4
	ddgVector4( const ddgVector4* u )                { v[0] = u->v[0]; v[1] = u->v[1]; v[2] = u->v[2]; v[3] = u->v[3];}
	/// Initialize from a float array.
	ddgVector4( const float* u )                  { v[0] = u[0]; v[1] = u[1]; v[2] = u[2]; v[3] = u[3];}
	/// Initialize from a pointer to a Vector3
	ddgVector4( const ddgVector3* u )             { v[0] = u->v[0]; v[1] = u->v[1]; v[2] = u->v[2]; v[3] = 0;}
	/// Initialize from a Vector4
	ddgVector4& operator=(const ddgVector4& u)    { v[0] = u[0]; v[1] = u[1]; v[2] = u[2]; v[3] = u[3]; return *this; }
	///
	void zero( void )                             { v[0] = v[1] = v[2] = v[3] = 0.0; }
	///
	void set(float a, float b, float c, float d ) { v[0] = a; v[1] = b; v[2] = c; v[3] = d; }
	///
	void set( const ddgVector3 *u )               { v[0] = u->v[0]; v[1] = u->v[1]; v[2] = u->v[2]; v[3] = 0;}
	///
	void set( const ddgVector4 *u )               { v[0] = u->v[0]; v[1] = u->v[1]; v[2] = u->v[2]; v[3] = u->v[3];}
    ///
	void set( const float *u )					  { v[0] = u[0]; v[1] = u[1]; v[2] = u[2]; v[3] = u[3];}
    ///
    void get(float *a, float *b, float *c, float *d) { *a = v[0]; *b = v[1]; *c = v[2]; *d = v[3]; }
	///
	void add( const ddgVector4 *u )                { v[0] += u->v[0]; v[1] += u->v[1]; v[2] += u->v[2]; v[3] += u->v[3]; }
	/// Add two vectors
	ddgVector4& operator+=( const ddgVector4& u) 
		{ v[0]+=u[0]; v[1] += u[1]; v[2] += u[2]; v[3] += u[3]; return *this; }
	/// Subtract two vectors
	ddgVector4& operator-=( const ddgVector4& u) 
		{ v[0]-=u[0]; v[1] -= u[1]; v[2] -= u[2]; v[3] -= u[3]; return *this; }
	/// Multiply two vectors
	ddgVector4& operator*=( const ddgVector4& u) 
		{ v[0]*=u[0]; v[1] *= u[1]; v[2] *= u[2]; v[3] *= u[3]; return *this; }
	/// Multiply vector by scalar.
	ddgVector4& operator*=( const float d) 
		{ v[0]*=d; v[1] *= d; v[2] *= d; v[3] *= d; return *this; }
	/// Divide two vectors
	ddgVector4& operator/=( const ddgVector4& u) 
		{ v[0]/=u[0]; v[1] /= u[1]; v[2] /= u[2]; v[3] /= u[3]; return *this; }
	/// Divide  vector by scalar.
	ddgVector4& operator/=( float d) 
		{ v[0]/=d; v[1] /= d; v[2] /= d; v[3] /= d; return *this; }
	/// Add two vectors
	inline friend ddgVector4 operator+( const ddgVector4& u, const ddgVector4& w) 
		{ return ddgVector4(u[0]+w[0],u[1]+w[1],u[2]+w[2],u[3]+w[3]); }
	///
	void subtract( const ddgVector4 *u )          { v[0] -= u->v[0]; v[1] -= u->v[1]; v[2] -= u->v[2]; v[3] -= u->v[3]; }
	/// Subtract two vectors
	inline friend ddgVector4 operator-( const ddgVector4& u, const ddgVector4& w) 
		{ return ddgVector4(u[0]-w[0],u[1]-w[1],u[2]-w[2],u[3]-w[3]); }
	///
	void multiply( float s )                      { v[0] *= s; v[1] *= s; v[2] *= s; v[3] *= s; }
	///
	void multiply( ddgVector4* u )                     { v[0] *= u->v[0]; v[1] *= u->v[1]; v[2] *= u->v[2]; v[3] *= u->v[3];}
	/// Multiply two vectors
	inline friend ddgVector4 operator*( const ddgVector4& u, const ddgVector4& w) 
		{ return ddgVector4(u[0]*w[0],u[1]*w[1],u[2]*w[2],u[3]*w[3]); }
	///
	void divide( float s )                        { v[0] /= s; v[1] /= s; v[2] /= s; v[3] /= s; }
	///
	void divide( const ddgVector4* u )                     { v[0] /= u->v[0]; v[1] /= u->v[1]; v[2] /= u->v[2]; v[3] /= u->v[3];}
	/// Divide two vectors
	inline friend ddgVector4 operator/( const ddgVector4& u, const ddgVector4& w) 
		{ return ddgVector4(u[0]/w[0],u[1]/w[1],u[2]/w[2],u[3]/w[3]); }
	///
	bool null( void )                             { return ((v[0] == v[1]) == (v[2] == v[3])) == 0.0; }
	///
	ddgVector4* multiply( const ddgMatrix4 *m1, const ddgVector4 *v1 );
	/// Scale a vector by a constant and assign result.
	inline friend ddgVector4 operator*( const float d, const ddgVector4& u) 
	{ return ddgVector4(u[0]*d,u[1]*d,u[2]*d,u[3]*d); }
	/// Scale a vector by a constant and assign result.
	inline friend ddgVector4 operator*( const ddgVector4& u, const float d) 
	{ return ddgVector4(u[0]*d,u[1]*d,u[2]*d,u[3]*d); }
	/// Divide a vector by a constant and assign result.
	inline friend ddgVector4 operator/( const ddgVector4& u, const float d) 
	{ return ddgVector4(u[0]/d,u[1]/d,u[2]/d,u[3]/d); }
};
#ifdef DDGSTREAM
///
WEXP ostream& WFEXP operator << ( ostream&s, ddgVector4 v );
///
WEXP ostream& WFEXP operator << ( ostream&s, const ddgVector4* v );
///
WEXP istream& WFEXP operator >> ( istream& s, ddgVector4& v);
#endif // DDGSTREAM
        
#endif
