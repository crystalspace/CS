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
#ifndef _ddgMatrix_Class_
#define _ddgMatrix_Class_

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
  ddgMatrix3( ddgVector3 v1, ddgVector3 v2, ddgVector3 v3 ) 
    { m[0].set(v1); m[1].set(v2); m[2].set(v3); }
};

/**
 * A simple general purpose 4x4 matrix object.
 */
class WEXP ddgMatrix4 {
public:
  /// A matrix is composed of 4 vectors.
  ddgVector4 m[4];
  /// Return an element from the matrix.
  ddgVector4 operator[](int n)            { return m[n]; }
  /// Set a matrix value.
  float operator()(int i, int j)       { return m[i].v[j]; }
  /// Assign one matrix to another.
  ddgMatrix4& operator=(const ddgMatrix4& s)
   { m[0] = s.m[0]; m[1] = s.m[1]; m[2] = s.m[2]; m[3] = s.m[3]; return *this; }
  /// Load the identity matrix.
  void identity(void)  { m[0].set(1,0,0,0);m[1].set(0,1,0,0);
                         m[2].set(0,0,1,0);m[3].set(0,0,0,1); }
  /// Set a matrix to the value of 4 vectors.
  ddgMatrix4( ddgVector4 *v1, ddgVector4 *v2, ddgVector4 *v3, ddgVector4 *v4) 
    { m[0].set(v1); m[1].set(v2); m[2].set(v3); m[3].set(v4); }
  /// Set a matrix to the value of a 16 element float array.
  ddgMatrix4( float u[16] )
    { m[0].v[0]=u[0];  m[0].v[1]=u[1];  m[0].v[2]=u[2];  m[0].v[3]=u[3];
      m[1].v[0]=u[4];  m[1].v[1]=u[5];  m[1].v[2]=u[6];  m[1].v[3]=u[7]; 
      m[2].v[0]=u[8];  m[2].v[1]=u[9];  m[2].v[2]=u[10]; m[2].v[3]=u[11]; 
      m[3].v[0]=u[12]; m[3].v[1]=u[13]; m[3].v[2]=u[14]; m[3].v[3]=u[15]; }
  /// Assign the value of a matrix to a 16 element float array.
  void assignto( float u[16] )
    { u[0] =  m[0].v[0]; u[1]  = m[0].v[1]; u[2] =  m[0].v[2]; u[3] =  m[0].v[3];
      u[4] =  m[1].v[0]; u[5]  = m[1].v[1]; u[6] =  m[1].v[2]; u[7] =  m[1].v[3];
      u[8] =  m[2].v[0]; u[9]  = m[2].v[1]; u[10] = m[2].v[2]; u[11] = m[2].v[3];
      u[12] = m[3].v[0]; u[13] = m[3].v[1]; u[14] = m[3].v[2]; u[15] = m[3].v[3]; }
  /// Multiply 2 matrices together.
  ddgMatrix4* multiply( ddgMatrix4 *m1, ddgMatrix4 *m2 );
};

/// Multiply two matrices together, product may be equal to a.
void WEXP ddgMatrixMultiply( float product[], const float a[], const float b[] );   
/// Set a matrix for rotation about axis by a given angle.
void WEXP ddgRotationMatrix( float angle, float x, float y, float z, float m[] );
#endif
