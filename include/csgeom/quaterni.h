/*
    Copyright (C) 2000 by Norman Krämer
  
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

#ifndef _QUATERNION_H_
#define _QUATERNION_H_

#include "csgeom/math3d.h"

class csQuaternion{
 public:
  csQuaternion(){ init( 0, 0, 0, 0 ); }
  csQuaternion( double theR, double theX=0.0, double theY=0.0, double theZ=0.0 )
    { init( theR, theX, theY, theZ ); }
  csQuaternion( const csQuaternion& q){ init( q.r, q.x, q.y, q.z ); }
  csQuaternion( const csVector3& q){ init( 0, q.x, q.y, q.z ); }
  
  inline friend csQuaternion operator +( csQuaternion& q1, csQuaternion& q2 )
    { return csQuaternion( q1.r + q2.r, q1.x + q2.x, q1.y + q2.y, q1.z + q2.z ); }
  inline friend csQuaternion operator -( csQuaternion& q1, csQuaternion& q2 )
    { return csQuaternion( q1.r - q2.r, q1.x - q2.x, q1.y - q2.y, q1.z - q2.z ); }
  inline friend csQuaternion operator *( csQuaternion& q1, csQuaternion& q2 )
    { return csQuaternion( q1.r*q2.r -  q1.x*q2.x - q1.y*q2.y - q1.z*q2.z,
			   q1.y*q2.z -  q1.z*q2.y + q1.r*q2.x + q1.x*q2.r,
			   q1.z*q2.x -  q1.x*q2.z + q1.r*q2.y + q1.y*q2.r,
			   q1.x*q2.y -  q1.y*q2.x + q1.r*q2.z + q1.z*q2.r ); }

  csQuaternion& operator *=( csQuaternion& q2 )
    { init( r*q2.r -  x*q2.x - y*q2.y - z*q2.z,
	    y*q2.z -  z*q2.y + r*q2.x + x*q2.r,
	    z*q2.x -  x*q2.z + r*q2.y + y*q2.r,
	    x*q2.y -  y*q2.x + r*q2.z + z*q2.r ); 
    return *this;
    }
  

  void conjugate(){ init( r, -x, -y, -z ); }

  // prepare a rotation quaternion, we do a rotation around vec by an angle of "angle"
  // note that vec needs to be a normalized vector ( we don't check this )
  void prepRotation( double angle, csVector3 vec )
    { 
      double theSin = sin( angle/2 ); 
      init( cos( angle/2 ), vec.x*theSin, vec.y*theSin, vec.z*theSin ); 
    }

  // rotated = q * vec * qConj
  csVector3 rotate( csVector3 vec )
    { 
      csQuaternion p( vec );
      csQuaternion qConj( r, -x, -y, -z ); 

      p = *this * p;
      p *= qConj;
      return csVector3( p.x, p.y, p.z );
  }

  inline void init( double theR, double theX, double theY, double theZ )
    { r = theR; x = theX; y = theY; z = theZ; } 

  double r,x,y,z;

};

#endif
