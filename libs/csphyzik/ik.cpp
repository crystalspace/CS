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

#include "cssysdef.h"
#include "csphyzik/ik.h"
#include "csphyzik/articula.h"
#include "csphyzik/joint.h"
#include "csphyzik/mathutil.h"

//!me this code is really lame!  rewrite it sometime.  Probably better iterative

void ctInverseKinematics::solve ( real t )
{
  ctVector3 nutnhoney;

  solve_IK ( t, goal, nutnhoney );
  ab.update_links ();
}

/*
void ctInverseKinematics::set_ik_goal( const ctVector3 &pgoal )
{
ctArticulatedBody *out_link;
ctInverseKinematics *out_link_solver;

	goal = pgoal;
	out_link = ab.outboard_links.get_first();
	if( out_link ){
		out_link_solver = (ctInverseKinematics *)out_link->solver;
		if( out_link_solver ){
			out_link_solver->set_ik_goal( pgoal );
		}
	}
}
*/

void ctInverseKinematics::solve_IK ( real t, 
				     ctVector3 &the_goal, ctVector3 &end_effector )
{
  ctArticulatedBody *out_link;
  ctInverseKinematics *out_link_solver;

  // compute joint angles useing Cyclical Descent Algorithm
  // compute angles from link n...1  ( 0 is base )
  out_link = ab.outboard_links.get_first();
  if ( out_link )
  {
    out_link_solver = (ctInverseKinematics *)out_link->solver;
    out_link_solver->solve_IK( t, the_goal, end_effector );

    // wind back up stack computing angle from tip to base
    out_link_solver->compute_joint_angle( t, the_goal, end_effector );
  }
  else
  {  
    // reached the end of the chain
    if ( ab.handle )
    {
      ctVector3 ef = ab.handle->get_org_world();  //!me change to tip
      end_effector[0] = ef[0];
      end_effector[1] = ef[1];
      end_effector[2] = ef[2];
    }
  }
}

void ctInverseKinematics::compute_joint_angle ( real t, 
				    ctVector3 &the_goal, ctVector3 &end_effector )
{
  ctJoint *joint_in;
  ctVector3 joint_to_goal;
  ctVector3 joint_to_end;
  ctVector3 joint_world;
  real angle_to_goal;
  ctMatrix3 dR;

  joint_in = ab.inboard_joint;

  if ( joint_in == NULL )
    return; //!me log error

  joint_world = joint_in->outboard_offset * -1.0;
  ab.handle->v_this_to_world( joint_world );

  joint_to_end = end_effector - joint_world;
  joint_to_goal = the_goal - joint_world;
	
  angle_to_goal = angle_diff( joint_to_goal, joint_to_end );

  if ( t > MIN_REAL )
    angle_to_goal = angle_to_goal/t;
  else
    angle_to_goal = angle_to_goal/(MIN_REAL * 10.0);

  //!me change to some kinda member variable
  if ( angle_to_goal > max_qv )
    angle_to_goal = max_qv;
  else if ( angle_to_goal < -max_qv )
    angle_to_goal = -max_qv;

  joint_in->qv = angle_to_goal;

  R_from_vector_and_angle ( ab.handle->get_v_this_to_world(joint_in->joint_axis),
			    angle_to_goal*t, dR );
  end_effector = dR*end_effector;
}

