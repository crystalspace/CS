/*
    Dynamics/Kinematics modeling and simulation library.
    Copyright (C) 1999 by Michael Alexander Ewert

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

#ifndef __CT_VECTOR__
#define __CT_VECTOR__

// very annoying that I can't use templates.  
// Crystal Space people have some beef with templates.

class ctVector3;
class ctMatrix3;

/*
* written by: Michael Alexander Ewert
*/

// seems like I could abstract a parent class of ctVector3 and ctVectorTranspose3

#include <stdarg.h>
#include <math.h>
#include "csphyzik/phyztype.h"
#include "csphyzik/debug.h"

class ctVectorTranspose3 
{
public:
  ctVectorTranspose3 ()
  { elements[0] = elements[1] = elements[2] = 0.0; }

  ctVectorTranspose3 ( real pfirst, real psecond, real pthird )
  {
    elements[0] = pfirst;
    elements[1] = psecond;
    elements[2] = pthird;
  }

  void set ( real pfirst, real psecond, real pthird )
  {
    elements[0] = pfirst;
    elements[1] = psecond;
    elements[2] = pthird;
  }

  void set ( int pnum, real *pele )
  {
    for( int idx = 0; idx < pnum; idx++ )
    {
      elements[idx] = *pele;
      pele++;
    }
  }

  void set ( real *pele )
  {
    for ( int idx = 0; idx < 3; idx++ )
    {
      elements[idx] = *pele;
      pele++;
    }
  }

  real operator[] (const int index) const 
  { return elements[index]; } 

  real& operator[] (const int index) 
  { return elements[index]; }

  ctVectorTranspose3 operator* ( const real pk ) 
  { 
    ctVectorTranspose3 scaled;
    for( int idx = 0; idx < 3; idx++ ) 
      scaled.elements[idx] = elements[idx] * pk;  
    return scaled;
  }
  
  void operator*= (const real p) 
  { for (int idx=0; idx<3; ++idx) elements[idx] *= p; }

  void operator/= (const real p) 
  { for (int idx=0; idx<3; ++idx) elements[idx] /= p; }

  real operator* ( const ctVector3 &bs );

protected:
  real elements[ 3 ];
};


#ifdef __CRYSTALSPACE__

#include "csgeom/math3d_d.h"
#define ctVector3 csDVector3
/*
class ctVector3 : public csDVector3
{
public:

  ctVector3(){
    x = y = z = 0.0;
  }

  ctVector3( real pone, real ptwo, real pthree ){
    x = pone;
    y = ptwo;
    z = pthree;
  }

  ctVector3( const csDVector3 &csv ){ x = csv.x; y = csv.y; z = csv.z; }

  void operator=( const csDVector3 &csv ){ x = csv.x; y = csv.y; z = csv.z; }

	/// Returns n-th component of the vector
  inline real & operator[](int n){return !n?x:n&1?y:z;}
	/// Returns n-th component of the vector
  inline real operator[](int n) const {return !n?x:n&1?y:z;}

  ctMatrix3 operator*( const ctVectorTranspose3 &pvt );
  
  void cross(const ctVector3 & px, const ctVector3 & py){
    x = px.y*py.z - px.z*py.y;
    y = px.z*py.x - px.x*py.z;
    z = px.x*py.y - px.y*py.x;
  }

  ctVector3 unit() const { return Unit(); } 

  void normalize();
  real length(){ return Norm(); }
};
*/
#else

class ctVector3
{
public:
  ctVector3 ( real pval = 0.0 )
  { elements[0] = elements[1] = elements[2] = pval; }

  ctVector3 ( real pone, real ptwo, real pthree )
  {
    elements[0] = pone;
    elements[1] = ptwo;
    elements[2] = pthree;
  }

  real operator[] ( const int index ) const 
  { return elements[index]; } 

  real& operator[] (const int index) 
  { return elements[index]; }

  ctVectorTranspose3 transpose ()
  {
    ctVectorTranspose3 trans;
    trans.set ( elements );
    return trans;
  }

  // return length of this vector
  //real length();
  real Norm ();

  // return a vector of unit length in same direction as this vector
  ctVector3 Unit ();
  void Normalize ();

  // set all elements to zero
  void zero ()
  { for( int idx = 0; idx < 3; idx++ ) elements[idx] = 0.0; }

  // this = this + x
  void add ( const ctVector3 & px )
  {
    elements[0] += px.elements[0];
    elements[1] += px.elements[1];
    elements[2] += px.elements[2];
  }

  // this = x + y
  void add2 (const ctVector3 & px, const ctVector3 & py)
  {
    elements[0] = px.elements[0] + py.elements[0];  
    elements[1] = px.elements[1] + py.elements[1];  
    elements[2] = px.elements[2] + py.elements[2];  
  
  }

  // dest = x + y
  void add3 (ctVector3 & pdest, const ctVector3 & px, const ctVector3 & py)
  {
    pdest.elements[0] = px.elements[0] + py.elements[0];  
    pdest.elements[1] = px.elements[1] + py.elements[1];  
    pdest.elements[2] = px.elements[2] + py.elements[2];  
  }
  
  void add_scaled ( ctVector3 & padme, real pk )
  {
    elements[0] += pk*padme.elements[0];    
    elements[1] += pk*padme.elements[1];    
    elements[2] += pk*padme.elements[2];    
  }

  void add_scaled ( real pk, ctVector3 & padme )
  {
    elements[0] += pk*padme.elements[0];    
    elements[1] += pk*padme.elements[1];    
    elements[2] += pk*padme.elements[2];    
  }

  void operator += (const ctVector3 & p)
  { for ( int idx = 0; idx < 3; idx++ ) elements[idx] += p.elements[idx]; }

  ctVector3 operator+ ( const ctVector3 & p) const 
  {
    ctVector3 sum;
    for ( int idx = 0; idx < 3; idx++ ) 
      sum.elements[idx] = elements[idx] + p.elements[idx];  
    return sum;
  }

  // this = this - x
  void subtract ( const ctVector3 & px )
  { for( int idx = 0; idx < 3; idx++ )  elements[idx] -= px.elements[idx]; }

  // this = x - y
  void subtract2 (const ctVector3 & px, const ctVector3 & py) 
  {
    for( int idx = 0; idx < 3; idx++ )
      elements[idx] = px.elements[idx] - py.elements[idx];
  }

  // dest = x - y
  void subtract3 ( ctVector3 & pdest, const ctVector3 & px, const ctVector3 & py)
  {
    for( int idx = 0; idx < 3; idx++ )
      pdest.elements[idx] = px.elements[idx] - py.elements[idx];
  }

  void operator-= ( const ctVector3 & p ) 
  { for( int idx = 0; idx < 3; idx++ ) elements[idx] -= p.elements[idx]; }

  ctVector3 operator- (const ctVector3 & p )
  {
    ctVector3 sum;
    for( int idx = 0; idx < 3; idx++ ) 
      sum.elements[idx] = elements[idx] - p.elements[idx];  
    return sum;
  }

  ctVector3 operator- (const ctVector3 & p) const 
  {
    ctVector3 sum;
    for( int idx = 0; idx < 3; idx++ ) 
      sum.elements[idx] = elements[idx] - p.elements[idx];  
    return sum;
  }

  ctVector3 operator-()
  {
    ctVector3 sum;
    for( int idx = 0; idx < 3; idx++ ) 
      sum.elements[idx] = -elements[idx];
    return sum;
  }

  ctVector3 operator-() const 
  {
    ctVector3 sum;
    for( int idx = 0; idx < 3; idx++ ) 
      sum.elements[idx] = -elements[idx];
    return sum;
  }

  real operator*( const ctVector3 & p )
  {
    real dotp = 0.0;
    for ( int idx = 0; idx < 3; idx++ ) dotp += elements[idx] * p.elements[idx]; 
    return dotp;
  }

  real operator*( const ctVector3 & p ) const 
  {
    real dotp = 0.0;
    for ( int idx = 0; idx < 3; idx++ ) dotp += elements[idx] * p.elements[idx]; 
    return dotp;
  }

  ctVector3 operator*( const real pk ) 
  { 
    ctVector3 scaled;
    for ( int idx = 0; idx < 3; idx++ ) 
      scaled.elements[idx] = elements[idx] * pk;  
    return scaled;
  }

  ctVector3 operator* ( const real pk ) const 
  { 
    ctVector3 scaled;
    for( int idx = 0; idx < 3; idx++ ) 
      scaled.elements[idx] = elements[idx] * pk;  
    return scaled;
  }

  ctVector3 operator/ ( const real pk ) 
  { 
    ctVector3 scaled;
    for( int idx = 0; idx < 3; idx++ ) 
      scaled.elements[idx] = elements[idx] / pk;  
    return scaled;
  }

  void operator*= ( const real p ) 
  { for (int idx=0; idx<3; ++idx) elements[idx] *= p; }
 
  void operator/= ( const real p ) 
  { for (int idx=0; idx<3; ++idx) elements[idx] /= p; }
 
  void Cross (const ctVector3 & px, const ctVector3 & py)
  {
    elements[0] = px.elements[1]*py.elements[2] - px.elements[2]*py.elements[1];
    elements[1] = px.elements[2]*py.elements[0] - px.elements[0]*py.elements[2];
    elements[2] = px.elements[0]*py.elements[1] - px.elements[1]*py.elements[0];
  }

  ctVector3 operator% ( const ctVector3 & py ) const  
  {
    ctVector3 xross;
    xross.elements[0] = elements[1]*py.elements[2] - elements[2]*py.elements[1];
    xross.elements[1] = elements[2]*py.elements[0] - elements[0]*py.elements[2];
    xross.elements[2] = elements[0]*py.elements[1] - elements[1]*py.elements[0];
    return xross;
  }

  ctMatrix3 operator* ( const ctVectorTranspose3 &pvt );

  int get_dimension ()
  { return 3; }

  real *get_elements () 
  { return elements; }

  void debug_print()
  {
    DEBUGLOGF( "%lf, ", elements[0] );
    DEBUGLOGF( "%lf, ", elements[1] );
    DEBUGLOGF( "%lf\n", elements[2] );
  }

//  friend ctVectorTranspose3<D>;
protected:
  real elements[ 3 ];

};

inline real ctVector3::Norm () 
{
  return sqrt(   elements[0]*elements[0] 
	       + elements[1]*elements[1] 
	       + elements[2]*elements[2] );
}

inline ctVector3 ctVector3::Unit () 
{
  return ( (*this)/this->Norm() );
}



inline real ctVectorTranspose3::operator* ( const ctVector3 &pv )
{ 
  real dotp = 0.0;
  for ( int idx = 0; idx < 3; idx++ ) 
    dotp += elements[idx] * pv[idx]; 
  return dotp;
}

inline void ctVector3::Normalize() 
{
  real len;
  len = this->Norm ();
  if ( len > MIN_REAL )
    *this /= len;
}

#endif // !__CRYSTALSPACE__


#endif // __CT_VECTOR__
