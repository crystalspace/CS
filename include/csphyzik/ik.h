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

#ifndef __CT_IK_H__
#define __CT_IK_H__

#include "csphyzik/solver.h"

/**
 * max joint speed
 * anything greater than 2 leads to bad results for long chains
 */
#define DEFAULT_MAX_QV 0.5;

class ctArticulatedBody;

/**
 * no dynamics here.  Calc velocities to move the end effector to it's goal
 *!me really just a quick hack right now to demonstrate solver abstraction
 */
class ctInverseKinematics : public ctArticulatedGoalSolver
{
public:
  ctInverseKinematics( ctArticulatedBody &pab )
    : ab( pab ) { max_qv = DEFAULT_MAX_QV; };

  virtual void solve( real t );

  // values relative to it's body frame
  ctVector3 get_linear_a ()
  { return ctVector3(0.0, 0.0, 0.0); }

  ctVector3 get_angular_a ()
  { return ctVector3(0.0, 0.0, 0.0); }

  void apply_impulse ( ctVector3 /*impulse_point*/, ctVector3 /*impulse_vector*/ )
  {};

  void get_impulse_m_and_I_inv ( real* /*pm*/, ctMatrix3* /*pI_inv*/,
				 const ctVector3& /*impulse_point*/,
				 const ctVector3& /*impulse_vector*/ ) {}


protected:
  void solve_IK ( real t, ctVector3 &the_goal, ctVector3 &end_effector );
  void compute_joint_angle ( real t,
			     ctVector3 &the_goal, ctVector3 &end_effector );
  ctArticulatedBody &ab;
  real max_qv;

};

#endif // __CT_IK_H__
