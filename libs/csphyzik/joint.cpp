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

#include "csphyzik/joint.h"
#include "csphyzik/refframe.h"
#include "csphyzik/articula.h"
#include "csphyzik/mathutil.h"
#include "csphyzik/debug.h"

double ctJoint::joint_friction = 0;

ctJoint::ctJoint( ctArticulatedBody *in, ctVector3 &in_offset, ctArticulatedBody *out, ctVector3 &out_offset )
{
	q = qv = qa = 0;
	inboard = in;
	inboard_offset = in_offset;
	outboard = out;
	outboard_offset = out_offset;

}

int ctJoint::set_state( real *state_array )
{
	*state_array++ = q;
	*state_array++ = qv;

	return ctJoint::get_state_size();
}


int ctJoint::get_state( const real *state_array )
{
	q = *state_array++;
	qv = *state_array++;

	return ctJoint::get_state_size();
}


int ctJoint::set_delta_state( real *state_array )
{
	*state_array++ = qv;
	*state_array++ = qa;

	return ctJoint::get_state_size();
}

// update the body_to_world reference frame of the outboard body given a 
// transformation matrix from inboard to outboard frames of reference
void ctJoint::update_link_RF( ctMatrix3 &R_fg )
{
	// R in here actually refer's to Tranformation matrix ( R.Transpose() )
	// I use R instead of T to be consistent with Mirtch's thesis.
	// although I use R in other areas to denote a positive rotation in a given
	// frame of reference.
	if( inboard && outboard ){
		ctReferenceFrame *in_ref = inboard->get_handle_RF();
		ctReferenceFrame *out_ref = outboard->get_handle_RF();

		if( !in_ref || !out_ref )
			return;

		ctMatrix3 R_delta;
		ctMatrix3 R_in = in_ref->get_parent_to_this();
		R_from_vector_and_angle( R_fg.get_transpose()*joint_axis, q, R_delta );
		
		// R_fg is actually coord transform from f to g, not a rotation matrix in
		// the strictest sense of the word
		R_fg = R_delta.get_transpose();  

		ctMatrix3 R_new_out = R_fg*R_in;
		out_ref->set_parent_to_this( R_new_out );
		ctVector3 new_out_origin = in_ref->get_offset() + 
									R_in.get_transpose()*inboard_offset + R_new_out.get_transpose()*outboard_offset;
		out_ref->set_offset( new_out_origin );

	}
}


//!me just a pendulum now, check this friction model in a text book
real ctJoint::get_actuator_magnitude( real external_f, real inertail_comp )
{
real internal_f;

	if( outboard && outboard->get_handle() ){
		//!me not very accurate, but should prevent too much instability
//		return (-1.0*joint_friction*qv * outboard->get_handle()->get_m()/100.0 ); 
		internal_f = -1.0*joint_friction*qv * fabs( external_f );

		// try to limit instability conditions. 
		// friction force is never more than some factor of external force.
		if( fabs(internal_f) > 0.25*fabs(external_f) ){
			internal_f *= 0.25*fabs(external_f/internal_f);
		}
		//!me attempt to limit overshoot.  not working well....
/*		if( external_f > 0 ){
			if( internal_f + external_f < 0 ){
				internal_f = -external_f*0;  
			}
		}else if( external_f < 0 ){
			if( internal_f + external_f > 0 ){
				internal_f = -external_f*0;  
			}
		}else if( external_f == 0 ){
			internal_f = 0;
		}*/
		return internal_f;
	}
	return 0;
} 


ctSpatialVector ctPrismaticJoint::get_spatial_joint_axis()
{
	return ctSpatialVector( 0.0,0.0,0.0, joint_axis[0], joint_axis[1], joint_axis[2] );

}

void ctPrismaticJoint::calc_coriolus( const ctVector3 &r, const ctVector3 &w_f, ctSpatialVector &c ){
	c.set_a( ctVector3( 0,0,0) );
	c.set_b( w_f % ( w_f % r ) + (  w_f *2.0) % ( joint_axis*qv ));

}

ctRevoluteJoint::ctRevoluteJoint( ctArticulatedBody *in, ctVector3 &in_offset, ctArticulatedBody *out, ctVector3 &out_offset, ctVector3 &paxis )
{
	q = qv = qa = 0;
	inboard = in;
	inboard_offset = in_offset;
	outboard = out;
	outboard_offset = out_offset;
	joint_axis = paxis;
}


ctSpatialVector ctRevoluteJoint::get_spatial_joint_axis()
{
ctVector3 xross = joint_axis % outboard_offset;
	return ctSpatialVector( joint_axis, xross );

}

void ctRevoluteJoint::calc_coriolus( const ctVector3 &r, const ctVector3 &w_f, ctSpatialVector &c ){
	ctVector3 V = ( joint_axis*qv );
	c.set_a( w_f % V);
	c.set_b( w_f % ( w_f % r ) + (w_f*2.0) % ( V % outboard_offset ) + V % ( V % outboard_offset ));
}
