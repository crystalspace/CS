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

#ifndef CT_FEATHERSTONE_ALGORITHM
#define CT_FEATHERSTONE_ALGORITHM

//!me something is may be wrong here.  Period of simple pendulum is taking too long.
//!me T = 2*pi*sqrt( length/g )  
//!me consistently higher by 50% or so.
//!me period of a meter long rod should be 1.64s under g=9.81 and 1 world unit = 1 m
//!me retest this sucker.  I may have calced the wrong size I at the time. may be OK

#include "csphyzik/solver.h"
#include "csphyzik/math3d.h"
#include "csphyzik/ctvspat.h"
#include "csphyzik/ctmspat.h"

class ctArticulatedBody;

// determine the motion of an articulated body acting under the influence of
// externally ( e.g. gravity ) applied forces and internally ( e.g. robot motor )
// applied forces
// Only works for tree-like topologies
class ctFeatherstoneAlgorithm : public ctArticulatedSolver
{
public:
	ctFeatherstoneAlgorithm( ctArticulatedBody &pab ) : ab( pab ){ sIsQsZIc_computed = false; };

	// do it!  Apply algorithm to solve for motions
	virtual void solve( real t );

	//	void init();

	// the algorithm result used by ctArticulatedBody::set_delta_state
	// this values are relative to it's body frame
	ctVector3 get_linear_a(){ ctVector3 aret( a[3], a[4], a[5] ); return aret; }
	ctVector3 get_angular_a(){ ctVector3 aret( a[0], a[1], a[2] ); return aret; }

protected:

	void fsolve_grounded( real t );
	void fsolve_floating( real t );

	void init_link();

	void compute_Ia_Za();

	void compute_joint_a();

	// the articulated body we are solving motions for
	ctArticulatedBody &ab;

	// work variables
	
	ctSpatialVector a;

	// spatial inertia matrix
	ctSpatialMatrix Ia;

	// bias vector or zero acceleration vector
    //  [ -m_i g_i - F_externaly_applied            ]   !note! negated!  
    //  [ w_i x I_i w_i - Torque_externally_applied ]
	ctSpatialVector Za;  // some other symbol in featherstone's book ( P? )

	// spatial coriolus force
	ctSpatialVector c;

	// spatial tranform from link i-1 to i
	ctSpatialMatrix gXf;


	// compute once per algo
	real sIs;
	real QsZIc;
	bool sIsQsZIc_computed;
};

#endif
