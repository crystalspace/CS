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

// bleh, bleh, I am articula; I don't drink.... wine.....

#include "cssysdef.h"
#include "csphyzik/articula.h"
#include "csphyzik/joint.h"
#include "csphyzik/debug.h"
#include "csphyzik/feathers.h"
#include "csphyzik/ik.h"

// set friction for this handles inboard joint
void ctArticulatedBody::set_joint_friction ( double pfrict )
{
  ctJoint::joint_friction = pfrict;

}
  	
// constructor
ctArticulatedBody::ctArticulatedBody () 
  : r_fg(0)
{
  handle = NULL; inboard_joint = NULL; 
  is_grounded = false;
  attached_to = NULL;
  solver = new ctFeatherstoneAlgorithm( *this );
//  solver = new ctInverseKinematics( *this );
}

// construct with a handle
ctArticulatedBody::ctArticulatedBody ( ctRigidBody *phandle ) 
  : ctPhysicalEntity( *(phandle->get_RF()), *(phandle->get_dRF()) ),
  r_fg(0)
{
  handle = phandle; inboard_joint = NULL; 
  is_grounded = false;
  attached_to = NULL;
  solver = new ctFeatherstoneAlgorithm( *this );
//	solver = new ctInverseKinematics( *this );
}

// clean up
ctArticulatedBody::~ctArticulatedBody ()
{
  ctArticulatedBody *out_link;

  if ( solver )
    delete solver;
  if ( handle )
    delete handle;
  if ( inboard_joint )
    delete inboard_joint;

  out_link = outboard_links.get_first();
  while ( out_link )
  {
    delete out_link;
    out_link = outboard_links.get_next();
  }	
}

//!me I don't really like the way this works....
// change the solver to featherstone
ctFeatherstoneAlgorithm *ctArticulatedBody::install_featherstone_solver ()
{
  ctArticulatedBody *out_link;
  ctFeatherstoneAlgorithm *feather_solve;

  if ( solver )
    delete solver;
	
  solver = feather_solve = new ctFeatherstoneAlgorithm( *this );
	
  out_link = outboard_links.get_first();
  while ( out_link )
  {
    out_link->install_featherstone_solver();
    out_link = outboard_links.get_next();
  }
  return feather_solve;
}

// change the solver to IK.  Set the goal for the returned solver
ctInverseKinematics *ctArticulatedBody::install_IK_solver ()
{
  ctArticulatedBody *out_link;
  ctInverseKinematics *ik_solve;

  if ( solver )
    delete solver;
	
  solver = ik_solve = new ctInverseKinematics ( *this );
	
  out_link = outboard_links.get_first();
  while( out_link )
  {
    out_link->install_IK_solver ();
    out_link = outboard_links.get_next ();
  }
  return ik_solve;
}


int ctArticulatedBody::get_state_size ()
{
  ctArticulatedBody *out_link;
  int sze = 0;

  if ( handle )
    sze += handle->get_state_size ();

  out_link = outboard_links.get_first();
  while ( out_link )
  {
    sze += JOINT_STATESIZE;
    sze += out_link->get_state_size();
    out_link = outboard_links.get_next();
  }
  return sze;
}



void ctArticulatedBody::apply_impulse 
  ( ctVector3 impulse_point, ctVector3 impulse_vector )
{
  ((ctArticulatedSolver *)solver)->apply_impulse ( impulse_point, impulse_vector );
}

void ctArticulatedBody::get_impulse_m_and_I_inv 
  ( real *pm, ctMatrix3 *pI_inv, const ctVector3 &impulse_point,
			      const ctVector3 &impulse_vector )
{
  ((ctArticulatedSolver *)solver)->get_impulse_m_and_I_inv 
                                   ( pm, pI_inv, impulse_point, impulse_vector );
}


void ctArticulatedBody::init_state ()
{
  ctArticulatedBody *out_link;

  if (handle) handle->init_state (); 
	
  out_link = outboard_links.get_first ();
  while ( out_link )
  {
    out_link->init_state();
    out_link = outboard_links.get_next();
  }
} //!me init joint as well???}//F.x = 0; F.y = 0; F.z = 0; T.x = 0; T.y = 0; T.z = 0; }


//!me would be better OOD to call joint->solve() and do all calculations there
// also calculates T_fg
// compute absolute velocities of all links from joint velocities of parents
void ctArticulatedBody::compute_link_velocities ()
{
  ctArticulatedBody *out_link;
  ctJoint *jnt;	// this bodies inboard joint
  ctPhysicalEntity *pe_f;  // parent ( inboard ) handle
  ctRigidBody *pe_g;  // this handle

  jnt = inboard_joint;
  pe_g = handle;

  //!me add error log
  if ( pe_g == NULL )
    return;

  // if we are at root
  if ( jnt == NULL || jnt->inboard == NULL )
  {
    const ctMatrix3 & T_wt = pe_g->get_world_to_this ();
    w_body = T_wt * pe_g->get_angular_v ();
    v_body = T_wt * pe_g->get_v ();
  }
  else
  {
    pe_f = jnt->inboard->handle;
    if ( !pe_f )
      return;

    // T_fg = T_world_g * T_f_world
    // T is coord transfrom matrix
    T_fg = pe_g->get_world_to_this ()*pe_f->get_this_to_world ();

    // vector from C.O.M. of F to G in G's ref frame.
    r_fg = pe_g->get_T ()*( pe_g->get_org_world () - pe_f->get_org_world () );

    // calc contribution to v and w from parent link.  
    w_body = T_fg*jnt->inboard->w_body;
    v_body = T_fg*jnt->inboard->v_body + (w_body % r_fg);
    
    // get joint to calculate final result for v and angular v ( w )
    jnt->calc_vw( v_body, w_body );
	  
    //!me this doesn't really need to be done, but better safe than sorry
    pe_g->set_angular_v( pe_g->get_this_to_world()*w_body );
    pe_g->set_v( pe_g->get_this_to_world()*v_body );
  }
  
  // iterate to next links
  out_link = outboard_links.get_first();
  while ( out_link )
  {
    out_link->compute_link_velocities();
    out_link = outboard_links.get_next();
  }
}

// apply a force to all links of this articulated body
void ctArticulatedBody::apply_given_F ( ctForce &frc )
{
  ctPhysicalEntity *pe;
  ctArticulatedBody *out_link;

  pe = handle;

  if ( pe )
  {
    pe->apply_given_F ( frc );
    out_link = outboard_links.get_first ();
    while( out_link )
    {
      out_link->apply_given_F ( frc );
      out_link = outboard_links.get_next ();
    }
  }
}

// calc F and torque from all applied forces
void ctArticulatedBody::apply_forces ( real t )
{
  ctPhysicalEntity *pe;
  ctArticulatedBody *out_link;

  pe = handle;

  if ( pe )
  {
    pe->solve ( t ); 
    out_link = outboard_links.get_first ();
    while ( out_link )
    {
      out_link->apply_forces ( t );
      out_link = outboard_links.get_next ();
    }
  }
}


int ctArticulatedBody::set_state ( real *state_array )
{
  int ret = 0;

  if ( handle )
  {
    ret += handle->set_state ( state_array );
    state_array += ret;
  }

  ret += set_state_links ( state_array );
  return ret;
}

int ctArticulatedBody::set_state_links ( real *state_array )
{
  int ret = 0;
  int one_size;
  ctArticulatedBody *out_link;

  if ( inboard_joint )
  {
    ret = inboard_joint->set_state ( state_array );
    state_array += ret;
  }
	
  out_link = outboard_links.get_first ();
  while ( out_link )
  {
    one_size = out_link->set_state_links ( state_array );
    state_array += one_size;
    ret += one_size;
    out_link = outboard_links.get_next ();
  }
  return ret;
}


int ctArticulatedBody::get_state ( const real *state_array )
{
  int ret = 0;

  if ( handle )
  {
    ret += handle->get_state( state_array );
    state_array += ret;
  }

  ret += get_state_links( state_array );

  return ret;
}

int ctArticulatedBody::get_state_links ( const real *state_array )
{
  int ret = 0;
  int one_size;
  ctArticulatedBody *out_link;

  if ( inboard_joint )
  {
    ret = inboard_joint->get_state( state_array );
    state_array += ret;

    // calc postion and orientation and store in handles state
    inboard_joint->update_link_RF();
  }
	
  out_link = outboard_links.get_first();
  while ( out_link )
  {
    one_size = out_link->get_state_links ( state_array );
    ret += one_size;
    state_array += one_size;
    out_link = outboard_links.get_next();
  }

  return ret;
}

int ctArticulatedBody::set_delta_state ( real *state_array )
{
  int ret = 0;

  if ( handle && solver )
  {
    // calc F and T from spatial acceleration
    // F = ma
    // HAHA!!!!!  a^ is relative to body frame!!!!
    const ctMatrix3 &R = handle->get_R ();
    ctVector3 lin_a = R * (((ctArticulatedSolver *)solver)->get_linear_a ());
    lin_a *= handle->get_m ();
    handle->set_F ( lin_a );

    // T = I*alpha    I is in world coords
    ctMatrix3 I_world = R * handle->get_I () * (R.get_transpose ());
    ctVector3 alpha = R * (((ctArticulatedSolver *)solver)->get_angular_a ());
    handle->set_torque ( I_world*alpha );

    ret += handle->set_delta_state ( state_array );
    state_array += ret;
  }

  ret += set_delta_state_links ( state_array );

  return ret;
}

int ctArticulatedBody::set_delta_state_links ( real *state_array )
{
  int ret = 0;
  int one_size;
  ctArticulatedBody *out_link;

  if ( inboard_joint )
  {
    ret = inboard_joint->set_delta_state ( state_array );
    state_array += ret;
  }

  out_link = outboard_links.get_first ();
  while ( out_link )
  {
    one_size = out_link->set_delta_state_links ( state_array );
    ret += one_size;
    state_array += one_size;
    out_link = outboard_links.get_next ();
  }
  return ret;
}

// update position and orientation of all links according to joint angles
void ctArticulatedBody::update_links ()
{
  ctArticulatedBody *out_link;
	
  if ( inboard_joint )
    // calc postion and orientation
    inboard_joint->update_link_RF();

  out_link = outboard_links.get_first ();
  while ( out_link )
  {
    out_link->update_links ();
    out_link = outboard_links.get_next ();
  }
}


// calc transform frame from parent to this
void ctArticulatedBody::calc_relative_frame ()
{
  ctJoint *jnt;	// this bodies inboard joint
  ctPhysicalEntity *pe_f;  // parent ( inboard ) handle
  ctRigidBody *pe_g;  // this handle

  jnt = inboard_joint;
  pe_g = handle;

  //!me add error log
  if ( pe_g == NULL )
    return;

  // if root
  if ( jnt == NULL || jnt->inboard == NULL )
  {
    r_fg = pe_g->get_pos ();
    T_fg = pe_g->get_world_to_this ();
    return;
  }

  pe_f = jnt->inboard->handle;
  if ( !pe_f )
    return;

  // T_fg = T_world_g * T_f_world
  // T is coord transfrom matrix
  T_fg = pe_g->get_world_to_this()*pe_f->get_this_to_world ();

  // vector from C.O.M. of F to G in G's ref frame.
  r_fg = pe_g->get_T ()*( pe_g->get_org_world () - pe_f->get_org_world () );
}

// link together two articulated bodies via a hing joint that hinges 
// along given axis
void ctArticulatedBody::link_revolute ( ctArticulatedBody *child, 
 ctVector3 &pin_joint_offset, ctVector3 &pout_joint_offset, ctVector3 &pjoint_axis )
{
  ctJoint *jnt = new ctRevoluteJoint ( this, pin_joint_offset, child, 
				       pout_joint_offset, pjoint_axis );

  child->inboard_joint = jnt;
  child->calc_relative_frame ();
  outboard_links.add_link ( child );
}

// link together two articulated bodies via a hing joint that hinges along given axis
void ctArticulatedBody::link_joint( ctJoint *jnt, ctArticulatedBody *child )
{
  child->inboard_joint = jnt;
  child->calc_relative_frame ();
  outboard_links.add_link ( child );
}

// explicit rotation
void ctArticulatedBody::rotate_around_axis ( real ptheta )
{
  if ( inboard_joint )
  {
    inboard_joint->q += ptheta;
    calc_relative_frame();
  }
}
