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

#include "csphyzik/mathutil.h"
#include "csphyzik/math3d.h"
#include <math.h>

#define VEC_X_NORM_THRESHOLD 0.01

void R_from_vector_and_angle( ctVector3 pvec, real theta, ctMatrix3 &pR )
{
ctVector3 Tx = pvec/pvec.length();
ctVector3 Ty = ctVector3( 0.0,0.0,1.0 ) % Tx;
ctVector3 Tz;

	if( Ty.length() < VEC_X_NORM_THRESHOLD ){
		Tz = Tx % ctVector3( 0.0,1.0,0.0);
		// make sure this is not a reflection.  That's why Tz is here not Ty
		Tz = Tz/Tz.length();
		Ty = Tz % Tx;
	}else{
		Ty = Ty/Ty.length ();
		Tz = Tx % Ty;
	}

// transform to rotation space ( where rotation will occur on x axis )
// the vector to be rotated around is the x-axis of this space
ctMatrix3 T;
	T.set(	Tx[0], Tx[1], Tx[2],
				Ty[0], Ty[1], Ty[2],
				Tz[0], Tz[1], Tz[2] );  

real cs = cos( theta );
real ss = sin( theta );
ctMatrix3 TRot;
	TRot.set(	1.0, 0.0, 0.0, 
				0.0, cs, -ss,
				0.0, ss, cs);

	// tranform to rotation space, rotate around x axis, then transform back
	// to world space.
	pR = T.get_transpose() * TRot * T; 

}

// use dot product to find the angle between two vectors
real angle_diff( ctVector3 v1, ctVector3 v2 )
{
real angle_abs;
ctVector3 vxross;

	v1.normalize();
	v2.normalize();
	
	vxross = v2 % v1;

	angle_abs = acos( v1*v2 );

	angle_abs *= sign( mu_max( 3, vxross[0], vxross[1], vxross[2] ) );
	
	return angle_abs;
}

real sign( real pwhat )
{
	if( pwhat >= 0 ){
		return 1.0;
	}else{
		return -1.0;
	}

}

real mu_max( int pnum, ... )
{
va_list parg;
real next_real;
real max_so_far = 0.0;

	va_start( parg, pnum );
	
	while( pnum ){
		next_real = va_arg( parg, real );
		if( fabs(next_real) > fabs(max_so_far) ){
			max_so_far = next_real;
		}
		pnum--;
	}

	va_end( parg );

	return max_so_far;
}
