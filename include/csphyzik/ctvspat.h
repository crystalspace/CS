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

#ifndef CT_SPATIALVECTOR
#define CT_SPATIALVECTOR

#include "csphyzik/ctvector.h"

#define ctSpatialVector ctSpatialVector6

class ctSpatialVector6 : public ctVector6
{
public:

	ctSpatialVector6(){
		for( int idx = 0; idx < 6; idx++ )
			elements[idx] = 0.0;
	}

	//!me should specialize this for <6> if I can figure out the right syntax
	ctSpatialVector6( const ctVector3 &pa, const ctVector3 &pb ){
		elements[0] = pa[0];
		elements[1] = pa[1];
		elements[2] = pa[2];
		elements[3] = pb[0];
		elements[4] = pb[1];
		elements[5] = pb[2];
	}

	//!me this is really error prone, so use with CAUTION!!!
	// make sure all args are real's and NOT INT's!!!!
	// ctVector a( 1,2,3 ) BAD!!!!!
	// ctVector a( 1.0, 2.0, 3.0 )  correct
	ctSpatialVector6( real pfirst, real psecond, ... )
	{
	va_list parg;
	real next_element;

		elements[0] = pfirst;
		elements[1] = psecond;

		va_start( parg, psecond );

		for( int tidx = 2; tidx < 6; tidx++ ){
			next_element = va_arg( parg, real );
			elements[tidx] = next_element;
		}

		va_end( parg );
	}

	ctVectorTranspose6 transpose(){
		ctVectorTranspose6 trans;
		trans[0] = elements[3];
		trans[1] = elements[4];
		trans[2] = elements[5];
		trans[3] = elements[0];
		trans[4] = elements[1];
		trans[5] = elements[2];
		return trans;
	}

	ctVectorTranspose6 operator!(){
		return transpose();
	}

	void set_a( const ctVector3 &pa ){
		elements[0] = pa[0];
		elements[1] = pa[1];
		elements[2] = pa[2];
	}

	void set_b( const ctVector3 &pb ){
		elements[3] = pb[0];
		elements[4] = pb[1];
		elements[5] = pb[2];
	}

	void operator=( const ctVector6 &pm ){
		for( int idx = 0; idx < 6; idx++ )
			elements[idx] = pm[idx];
	}

};

/*
inline ctVectorTranspose6 ctSpatialVector::transpose(){
	ctVectorTranspose6 trans;
	trans[0] = elements[3];
	trans[1] = elements[4];
	trans[2] = elements[5];
	trans[3] = elements[0];
	trans[4] = elements[1];
	trans[5] = elements[2];
	return trans;
}
*/
#endif
