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

#ifndef CT_SPATIALMATRIX
#define CT_SPATIALMATRIX

#include "csphyzik/ctmatrix.h"

#define ctSpatialMatrix ctSpatialMatrix6

class ctSpatialMatrix6 : public ctMatrix6
{
public:
	// spatial transform based on rotation matrix pR and offset vector pr
	// bewteen two frames of reference.  Result will be a matrix that transforms
	// any spatial vector between those two frames
	void form_spatial_transformation( const ctMatrix3 &pR, const ctVector3 &pr );

	// spatial inertia tensor constructed from a rigid bodies inertia tensor
	// and it's mass
	void form_spatial_I( const ctMatrix3 &pI, real pmass );

	void operator=( const ctMatrix6 &pm ){
		for( int idx = 0; idx < 6; idx++ )
			rows[idx] = pm[idx];
	}
};

inline void ctSpatialMatrix::form_spatial_transformation( const ctMatrix3 &pR, const ctVector3 &pr )
{
	// -~rR     ~r is the x-product matrix thing
	rows[3][0]/*M21.m11*/ = pr[2]*pR[1][0] - pr[1] * pR[2][0];
	rows[3][1]/*M21.m12*/ = pr[2]*pR[1][1] - pr[1] * pR[2][1];
	rows[3][2]/*M21.m13*/ = pr[2]*pR[1][2] - pr[1] * pR[2][2];

	rows[4][0]/*M21.m21*/ = - pr[2]*pR[0][0] + pr[0] * pR[2][0];
	rows[4][1]/*M21.m22*/ = - pr[2]*pR[0][1] + pr[0] * pR[2][1];
	rows[4][2]/*M21.m23*/ = - pr[2]*pR[0][2] + pr[0] * pR[2][2];

	rows[5][0]/*M21.m31*/ =  pr[1]*pR[0][0] - pr[0] * pR[1][0];
	rows[5][1]/*M21.m32*/ =  pr[1]*pR[0][1] - pr[0] * pR[1][1];
	rows[5][2]/*M21.m33*/ =  pr[1]*pR[0][2] - pr[0] * pR[1][2];
	
	for( int idr = 0; idr < 3; idr++ ){
		for( int idc = 0; idc < 3; idc++ ){
			rows[idr][idc] = pR[idr][idc];
			rows[idr+3][idc+3] = pR[idr][idc];
			rows[idr][idc+3] = 0.0;
		}
	}

}

inline void ctSpatialMatrix::form_spatial_I( const ctMatrix3 &pI, real pmass ){
/*	M11 *= 0;
	M12.Identity();
	M12 *= pmass;
	M21 = pI;
	M22 *= 0;
*/
	for( int idr = 0; idr < 3; idr++ ){
		for( int idc = 0; idc < 3; idc++ ){
			rows[idr][idc] = 0;
			rows[idr][idc+3] = ( idc == idr ) ? pmass : 0.0;
			rows[idr+3][idc] = pI[idr][idc];
			rows[idr+3][idc+3] = 0.0;
		}
	}
}


#endif
