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
#include "csphyzik/joint.h"
#include "csphyzik/refframe.h"
#include "csphyzik/articula.h"
#include "csphyzik/mathutil.h"
#include "csphyzik/debug.h"

double ctJoint::joint_friction = 0.0;

ctJoint::ctJoint ( ctArticulatedBody *in, 
	    ctVector3 &in_offset, ctArticulatedBody *out, ctVector3 &out_offset ) 
  : inboard (in), inboard_offset (in_offset), outboard ( out ), 
    outboard_offset(out_offset), joint_axis(0)
{
  q = qv = qa = 0;
}

int ctJoint::set_state ( real *state_array )
{
  *state_array++ = q;
  *state_array++ = qv;

  return ctJoint::get_state_size ();
}


int ctJoint::get_state ( const real *state_array )
{
  q = *state_array++;
  qv = *state_array++;

  return ctJoint::get_state_size ();
}


int ctJoint::set_delta_state( real *state_array )
{
  *state_array++ = qv;
  *state_array++ = qa;

  return ctJoint::get_state_size ();
}

// update the body_to_world reference frame of the outboard body
void ctJoint::update_link_RF ()
{

  if ( inboard && outboard )
  {
    ctReferenceFrame *in_ref = inboard->get_handle_RF ();
    ctReferenceFrame *out_ref = outboard->get_handle_RF ();

    if ( !in_ref || !out_ref )
      return;

    ctMatrix3 R_delta;
    ctMatrix3 T_in = in_ref->get_parent_to_this ();

    // get rotation matrix from rotation around joint axis
    R_from_vector_and_angle( joint_axis, q, R_delta );

    ctMatrix3 T_new_out = R_delta.get_transpose ()*T_in;
    out_ref->set_parent_to_this ( T_new_out );
    ctVector3 new_out_origin = in_ref->get_offset () + 
                               T_in.get_transpose()*inboard_offset + 
                               T_new_out.get_transpose()*outboard_offset;
    out_ref->set_offset( new_out_origin );
  }
}


//!me just a pendulum now, check this friction model in a text book
real ctJoint::get_actuator_magnitude( real external_f, real /*inertail_comp*/ )
{
  real internal_f;

  if ( outboard && outboard->get_handle() )
  {
    //!me not very accurate, but should prevent too much instability
    // return (-1.0*joint_friction*qv * outboard->get_handle()->get_m()/100.0 ); 
    internal_f = -1.0*joint_friction*qv * fabs( external_f );

    // try to limit instability conditions. 
    // friction force is never more than some factor of external force.
    if ( fabs(internal_f) > 0.25*fabs(external_f) )
      internal_f *= 0.25*fabs(external_f/internal_f);

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


ctSpatialVector ctPrismaticJoint::get_spatial_joint_axis ()
{
  return ctSpatialVector( 0.0,0.0,0.0, joint_axis[0], joint_axis[1], joint_axis[2] );
}

void ctPrismaticJoint::calc_coriolus
  ( const ctVector3 &r, const ctVector3 &w_f, ctSpatialVector &c )
{
  c.set_a( ctVector3( 0,0,0) );
  c.set_b( w_f % ( w_f % r ) + (  w_f *2.0) % ( joint_axis*qv ));
}

ctRevoluteJoint::ctRevoluteJoint( ctArticulatedBody *in, ctVector3 &in_offset, 
				  ctArticulatedBody *out, ctVector3 &out_offset, 
				  ctVector3 &paxis)
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

void ctRevoluteJoint::calc_coriolus ( const ctVector3 &r, const ctVector3 &w_f, 
				      ctSpatialVector &c )
{
  ctVector3 V = ( joint_axis*qv );
//  c.set_a( w_f % V);
//  c.set_b( w_f % ( w_f % r ) + (w_f*2.0) % ( V % outboard_offset ) + 
//           V % ( V % outboard_offset ));

  c[0] = w_f[1]*V[2] - w_f[2]*V[1];
  c[1] = w_f[2]*V[0] - w_f[0]*V[2];
  c[2] = w_f[0]*V[1] - w_f[1]*V[0];

  ctVector3 Vo(
    V[1]*outboard_offset[2] - V[2]*outboard_offset[1],
    V[2]*outboard_offset[0] - V[0]*outboard_offset[2],
    V[0]*outboard_offset[1] - V[1]*outboard_offset[0]
  );
   
  ctVector3 vwork(
    w_f[1]*(w_f[0]*r[1] - w_f[1]*r[0]) - w_f[2]*(w_f[2]*r[0] - w_f[0]*r[2]),
    w_f[2]*(w_f[1]*r[2] - w_f[2]*r[1]) - w_f[0]*(w_f[0]*r[1] - w_f[1]*r[0]),
    w_f[0]*(w_f[2]*r[0] - w_f[0]*r[2]) - w_f[1]*(w_f[1]*r[2] - w_f[2]*r[1])
  );

  c[3] = vwork[0] + 2.0*(w_f[1]*Vo[2] - w_f[2]*Vo[1]) + (V[1]*Vo[2] - V[2]*Vo[1]);
  c[4] = vwork[1] + 2.0*(w_f[2]*Vo[0] - w_f[0]*Vo[2]) + (V[2]*Vo[0] - V[0]*Vo[2]);
  c[5] = vwork[2] + 2.0*(w_f[0]*Vo[1] - w_f[1]*Vo[0]) + (V[0]*Vo[1] - V[1]*Vo[0]);

}

ctConstrainedRevoluteJoint::ctConstrainedRevoluteJoint
  ( ctArticulatedBody *in, ctVector3 &in_offset, 
    ctArticulatedBody *out, ctVector3 &out_offset, ctVector3 &paxis)
 : ctRevoluteJoint( in,  in_offset,  out,  out_offset, paxis)
{
  max_angle = 0;
  min_angle = 0;
  k = 10.0;  //!me set to a #define default
  damping_k = 50.0;
}

//!me best thing to do would be exert -external_f and apply impulse to stop qv....
// That would be perfect
real ctConstrainedRevoluteJoint::get_actuator_magnitude 
   ( real external_f, real inertail_comp )
{
  real internal_f;
  real response_sign = 0;
  real spring_response = 0;
  if ( outboard && outboard->get_handle() )
  {
    if ( q < min_angle )
    {
      response_sign = 1.0;
      spring_response = k*( min_angle - q) * inertail_comp;
    }
    else if( q > max_angle )
    {
      response_sign = -1.0;
      spring_response = k*( max_angle - q) * inertail_comp;
    }

    if ( response_sign != 0 )
    {
      internal_f = -external_f;  //!me intertal_comp is never negative is it?
      // only have a response force if it is working to satisfy constraint, 
      // not make it worse.
      // if working to maintain constraint one will be + and the other will be -.
      if ( -response_sign * internal_f > 0.0 )
	internal_f = 0.0;

      internal_f += spring_response;
      // add in drag to damp things
      if ( qv*response_sign < 0 )
	internal_f += - k*(qv)*0.25 * inertail_comp * damping_k;

      return internal_f;
    }
  }
  return ctRevoluteJoint::get_actuator_magnitude( external_f, inertail_comp );
} 
