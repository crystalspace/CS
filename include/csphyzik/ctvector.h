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
	int idx;
    for(idx = 0; idx < pnum; idx++ )
    {
      elements[idx] = *pele;
      pele++;
    }
  }

  void set ( real *pele )
  {
	int idx;
    for (idx = 0; idx < 3; idx++)
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
	int idx;
    for(idx = 0; idx < 3; idx++ ) 
      scaled.elements[idx] = elements[idx] * pk;  
    return scaled;
  }
  
  void operator*= (const real p) 
  { int idx; for (idx=0; idx<3; ++idx) elements[idx] *= p; }

  void operator/= (const real p) 
  { int idx; for (idx=0; idx<3; ++idx) elements[idx] /= p; }

  real operator* ( const ctVector3 &bs );

protected:
  real elements[ 3 ];
};


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

#endif // __CT_VECTOR__
