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

//!me last readable version before optimization in Crystal Space 15.002

// this algorithm largely taken from Brian Mirtich's master thesis.
// original algorithm is Dr. Featherstone's.  
// ( his book is much harder to understand ).


#ifndef __NO_CRYSTALSPACE__
#include "cssysdef.h"
#endif
#include "csphyzik/feathers.h"
#include "csphyzik/articula.h"
#include "csphyzik/joint.h"
#include "csphyzik/debug.h"
#include "csphyzik/kinemat.h"

// resolve dynamics
void ctFeatherstoneAlgorithm::solve ( real t )
{
  if ( ab.is_grounded )
    fsolve_grounded( t );
  else
    fsolve_floating( t );
}

// solve using featherstones algorithm where the base of the robot
// is fixed to the inertial frame.
void ctFeatherstoneAlgorithm::fsolve_grounded ( real t )
{
  ctArticulatedBody *out_link;
  ctArticulatedBody *out_link2;
  //ctArticulatedBody *current_ab;
  ctFeatherstoneAlgorithm *out_link_solver;

  // calc forces applied to this handle.  Doesn't include forces from other links
  ab.apply_forces( t );

  // propagate velocity of links ( w & v ) from base to leaves
  // also computes T_fg and r_fg
  ab.compute_link_velocities();

  // initialize values for each articulated body/joint
  init_link ();
  //!me it's questionable if I got better performance from this interation scheme
  //!me only like 1% which is definitly within standard deviation of tests
  /* ctLinkList <ctArticulatedBody> call_stack;
     call_stack.push( &ab );
     while( call_stack.get_size() > 0 ){
     current_ab = call_stack.pop();
     out_link_solver = (ctFeatherstoneAlgorithm *)current_ab->solver;
     out_link_solver->init_link();

    //!me this is kind of reverse order than in Mirtch's thesis.
    //!me should have a queu here then add queu to stack.
    //!me shouldn't matter, except that Mirtch specifically used 
    //!me that order....
    out_link = current_ab->outboard_links.get_first();
	  while( out_link ){
      call_stack.push( out_link );
		  out_link = current_ab->outboard_links.get_next();
	  }
  }*/
  //!me opt end

  // only modify Ia and Za for links 2 to n 
  // ugh, this is a bit ugly with this nested while, should be a better way
  out_link = ab.outboard_links.get_first ();
  while( out_link )
  {
    out_link2 = out_link->outboard_links.get_first ();
    while ( out_link2 )
    {
      out_link_solver = (ctFeatherstoneAlgorithm *)out_link2->solver;
      out_link_solver->compute_Ia_Za();
      out_link2 = out_link->outboard_links.get_next();
    }
    out_link = ab.outboard_links.get_next();
  }

  // compute joint and spatial acceleration of links
  // link 0 is fixed and doesn't accelerate
  if ( ab.attached_to == NULL )
  {
    a.zero();
    // or it accelerates according to some other object it is attached to
  }
  else
  { 
    //!me optimize
    ctVector3 angular_v = ab.attached_to->get_angular_v();
    ctVector3 angular_a = ab.attached_to->get_angular_a();
    ctVector3 r = ab.handle->get_pos() - ab.attached_to->get_x();
    // angular acceleration
    a.set_a ( angular_a );

    // linear acceleration
    a.set_b( ab.attached_to->get_a() + angular_a%r + angular_v%angular_v%r );
  }

  out_link = ab.outboard_links.get_first();
  while( out_link )
  {
    out_link_solver = (ctFeatherstoneAlgorithm *)out_link->solver;
    out_link_solver->compute_joint_a();
    out_link = ab.outboard_links.get_next();
  }
}

// solve using featherstones algorithm where the base of the robot
// is freely floating.
void ctFeatherstoneAlgorithm::fsolve_floating( real t )
{
  ctArticulatedBody *out_link;
  ctFeatherstoneAlgorithm *out_link_solver;
  //ctArticulatedBody *current_ab;

  // calc forces applied to this handle. Doesn't include forces from other links
  ab.apply_forces( t );

  // propagate velocity of links ( w & v ) from base to leaves
  // also computes T_fg and r_fg
  ab.compute_link_velocities();
	
  // initialize values for each articulated body/joint
  init_link();
//!me opt init_link();
/*  ctLinkList <ctArticulatedBody> call_stack;
  call_stack.push( &ab );
  while( call_stack.get_size() > 0 ){
    current_ab = call_stack.pop();
    out_link_solver = (ctFeatherstoneAlgorithm *)current_ab->solver;
    out_link_solver->init_link();

    //!me this is kind of reverse order than in Mirtch's thesis.
    //!me should have a queu here then add queu to stack.
    //!me shouldn't matter, except that Mirtch specifically used 
    //!me that order....
    out_link = current_ab->outboard_links.get_first();
	  while( out_link ){
      call_stack.push( out_link );
		  out_link = current_ab->outboard_links.get_next();
	  }
  }*/
//!me opt end


  // modify Ia and Za for all links ( start by calling compute_Ia_Za on 
  // link 1 ( not 0 ) )
  out_link = ab.outboard_links.get_first();
  while ( out_link )
  {
    out_link_solver = (ctFeatherstoneAlgorithm *)out_link->solver;
    out_link_solver->compute_Ia_Za();
    out_link = ab.outboard_links.get_next();
  }

  // compute joint and spatial acceleration of links
	
  // linear a and alpha for link 0 is calculated by solving for spatial a:
  // Ia*a = -Za
  Ia.solve( a, Za*(-1.0) );

 // 	a.zero();
  out_link = ab.outboard_links.get_first();
  while ( out_link )
  {
    out_link_solver = (ctFeatherstoneAlgorithm *)out_link->solver;
    out_link_solver->compute_joint_a();
    out_link = ab.outboard_links.get_next();
  }
}

// init values before processing with featherstone of each articulated body 
// init applied force and coriolous force for this link 
void ctFeatherstoneAlgorithm::init_link ()
{
  ctArticulatedBody *out_link;
  ctJoint *jnt;
  ctPhysicalEntity *pe_f;
  ctRigidBody *pe_g;
  ctFeatherstoneAlgorithm *out_link_solver;

  // spatial transform matrix from f to g.  Used later on
  gXf.form_spatial_transformation( ab.T_fg, ab.r_fg );

  pe_g = ab.handle;
  
  if ( pe_g == NULL )
    return;

  // calc spatial inertia
  Ia.form_spatial_I( pe_g->get_I(), pe_g->get_m() ); 
	
  // applied force and torque + centripital force goes into here.
  // -applied because this is actualy force and torque needed to 
  // keep link from NOT accelerating.
  Za.set_a( pe_g->get_T()*(-pe_g->get_F()));
  Za.set_b( ab.w_body % (pe_g->get_I() * ab.w_body) - (pe_g->get_T()*pe_g->get_torque()) );  

  jnt = ab.inboard_joint;

  // calc coriolous force
  // base of chain
  if ( jnt == NULL || jnt->inboard == NULL )
  {
    // this will happen with a floating links 0 handle
    ctSpatialVector ct( 0.0,0.0,0.0,0.0,0.0,0.0 );
    c = ct;
  }
  else
  {
    pe_f = jnt->inboard->handle;

    if ( pe_f )
    {
      jnt->calc_coriolus( ab.r_fg, ab.T_fg*jnt->inboard->w_body, c );
    }
    else
    {
      ctSpatialVector ct( 0.0,0.0,0.0,0.0,0.0,0.0 );
      c = ct;
    }
  }

  // iterate to next link
  out_link = ab.outboard_links.get_first();
  while ( out_link )
  {
    out_link_solver = (ctFeatherstoneAlgorithm *)out_link->solver;
    out_link_solver->init_link();
    out_link = ab.outboard_links.get_next();
  }
}

// featherstone algorithm first step
// calc spatial Inertia tensor and Bias force vector for all links
void ctFeatherstoneAlgorithm::compute_Ia_Za ()
{
  ctJoint *jnt;
  ctArticulatedBody *out_link;
  ctArticulatedBody *ab_f;
  ctFeatherstoneAlgorithm *out_link_solver;
  ctFeatherstoneAlgorithm *svr_f;
  //ctMatrix3 Mwork;
  ctVector3 vwork(0);
  ctSpatialVector svwork;
  //ctSpatialMatrix sMwork;
  //ctSpatialVector ZaIac;

  // recurse to end, then unwind and do the work
  out_link = ab.outboard_links.get_first ();
  while ( out_link )
  {
    out_link_solver = (ctFeatherstoneAlgorithm *)out_link->solver;
    out_link_solver->compute_Ia_Za();
    out_link = ab.outboard_links.get_next();
  }
  // unwinding recursive stack starting here 
  // from link n to link 2

  ctSpatialMatrix fXg;
  // fXg.form_spatial_transformation( ab.T_fg.get_transpose(), ab.T_fg.get_transpose()*ab.r_fg * -1 );
  ab.T_fg.put_transpose ( Mwork );
  Mwork.mult_v ( vwork, ab.r_fg );
  vwork *= -1.0; 
  fXg.form_spatial_transformation ( Mwork, vwork );

  jnt = ab.inboard_joint;
  if ( jnt == NULL )
    return;

  ab_f = jnt->inboard;
  if ( ab_f == NULL || ab_f->solver == NULL )
    return;
	
  svr_f = (ctFeatherstoneAlgorithm *)ab_f->solver;

  ctSpatialVector s = jnt->get_spatial_joint_axis ();

  // sIs = (!s)*(Ia*s);
  Ia.mult_v ( svwork, s );
  sIs = s.spatial_dot ( svwork );

  ctSpatialMatrix innerM;
  // innerM = ( Ia*s*(!s)*Ia )*(1.0L/sIs);
  Ia.mult_v ( svwork, s );
  sMwork = svwork*(!s);
  sMwork.mult_M ( innerM, Ia );
  innerM *= (1.0L/sIs);

  // calc Ia for parent link
  // svr_f->Ia += fXg*( Ia - innerM )*gXf;
  sMwork.subtract2 ( Ia, innerM );
  fXg.mult_M ( innerM, sMwork );
  innerM *= gXf;
  svr_f->Ia += innerM; 

  // force from any robot motors are factored in here.  also joint friction
  // external_f = -((!s)*(Za + Ia*c ));
  Ia.mult_v ( ZaIac, c );
  ZaIac += Za;
  real external_f = -(s.spatial_dot ( ZaIac ));
  QsZIc = ( jnt->get_actuator_magnitude( external_f, sIs ) + external_f );
  sIsQsZIc_computed = true;

  // calc Za for parent link
  // svr_f->Za = svr_f->Za + fXg*( Za + Ia*c + ( Ia*s*(QsZIc/sIs) ) );
  Ia.mult_v ( svwork, s );
  svwork *= (QsZIc/sIs);
  ZaIac += svwork;
  fXg.mult_v ( svwork, ZaIac );
  svr_f->Za += svwork;

  // unwind recursive stack
  return;
}

// Final step of featherstone algorithm.  
// Compute joint accelerations and spatial accel
void ctFeatherstoneAlgorithm::compute_joint_a ()
{
  ctJoint *jnt;
  ctArticulatedBody *out_link;
  ctFeatherstoneAlgorithm *prev_link_solver;
  ctFeatherstoneAlgorithm *out_link_solver;
  ctSpatialVector svwork, gXfa;
 
  jnt = ab.inboard_joint;
  prev_link_solver = (ctFeatherstoneAlgorithm *) jnt->inboard->solver;

  ctSpatialVector s = jnt->get_spatial_joint_axis ();

  if ( !sIsQsZIc_computed )
  {
    sIs = (!s)*(Ia*s);
    // force from any robot motors are factored in here
    real external_f = -((!s)*(Za + Ia*c ));
    QsZIc = ( jnt->get_actuator_magnitude ( external_f, sIs ) + external_f );
  }

  sIsQsZIc_computed = false;

  // Alright! finaly we get the result all that code was for.
  // jnt->qa = ( QsZIc - (!s)*( (Ia*gXf)*prev_link_solver->a ) )/sIs;
  gXf.mult_v ( gXfa, prev_link_solver->a );
  Ia.mult_v ( svwork, gXfa );
  jnt->qa = ( QsZIc - s.spatial_dot( svwork ) ) / sIs;
  // a = gXf*prev_link_solver->a + c + s*jnt->qa;
  gXfa += c;
  a.add2 ( gXfa, s*jnt->qa );

  // iterate to next link
  out_link = ab.outboard_links.get_first();
  while ( out_link )
  {
    out_link_solver = (ctFeatherstoneAlgorithm *)out_link->solver;
    out_link_solver->compute_joint_a();
    out_link = ab.outboard_links.get_next();
  }
}

// used to see what the effect of a "test" impulse is upon the velocity 
// of one link
void ctFeatherstoneAlgorithm::test_impulse_response ()
{
  ctArticulatedBody *inboard_link;
  ctFeatherstoneAlgorithm *in_feather=NULL;
  ctSpatialVector s;

  if ( ab.inboard_joint != NULL )
  {
    inboard_link = ab.inboard_joint->inboard;
    if ( inboard_link != NULL )
    {
      in_feather = (ctFeatherstoneAlgorithm *)inboard_link->solver;
      ctSpatialMatrix fXg;
      ctVector3 vwork (0);
      s = ab.inboard_joint->get_spatial_joint_axis ();
      // fXg.form_spatial_transformation( ab.T_fg.get_transpose(), ab.T_fg.get_transpose()*ab.r_fg * -1 );
      ab.T_fg.put_transpose ( Mwork );
      Mwork.mult_v ( vwork, ab.r_fg );
      vwork *= -1.0; 
      fXg.form_spatial_transformation ( Mwork, vwork );

      // calc impulse transfered to parent
      ctSpatialMatrix6 Ident;
      Ident.identity();
      ctSpatialMatrix6 Ident1;
      Ident1 = Ia*s*(!s)*(1.0L/sIs);
      in_feather->Ja = fXg*( Ident - Ident1 )*Ja;
      // recurse
      in_feather->test_impulse_response ();
    }
  }
  
  // so go back down and calc v changes from impulses
  // as the stack unwinds on path from root to impulse link
  
  // root gets special treatment
  if ( ab.inboard_joint == NULL )
  {
    if ( ab.is_grounded )
      dv.zero ();
    else
      Ia.solve ( dv, Ja*(-1.0) );
  }
  else
  {
    // instantaneous change in joint acceleration
    real dqv;  
    ctSpatialVector s = ab.inboard_joint->get_spatial_joint_axis ();

    dqv = !s*( 1.0L/sIs )*( Ia*gXf*in_feather->dv + Ja )*(-1.0);
    dv = gXf*in_feather->dv + s*dqv;

    ab.inboard_joint->qv += dqv;
  }
}


void ctFeatherstoneAlgorithm::impulse_to_v ()
{
  ctFeatherstoneAlgorithm *in = (ctFeatherstoneAlgorithm *)(ab.inboard_joint->inboard->solver);
  // instantaneous change in joint acceleration
  real dqv;  
  ctSpatialVector s = ab.inboard_joint->get_spatial_joint_axis ();

  dqv = !s*( 1.0L/sIs )*( Ia*gXf*in->dv + Ja )*(-1.0);
  dv = gXf*in->dv + s*dqv;

  ab.inboard_joint->qv += dqv; 

  ctArticulatedBody *out_link = ab.outboard_links.get_first ();
  ctFeatherstoneAlgorithm *out_link_solver;
  while ( out_link )
  {
    out_link_solver = (ctFeatherstoneAlgorithm *)out_link->solver;
    out_link_solver->impulse_to_v();
    out_link = ab.outboard_links.get_next();
  }
}

void ctFeatherstoneAlgorithm::propagate_impulse ()
{
  ctArticulatedBody *inboard_link;
  ctFeatherstoneAlgorithm *in_feather;
  
  if ( ab.inboard_joint != NULL )
  {
    inboard_link = ab.inboard_joint->inboard;
    if ( inboard_link != NULL )
    {
      in_feather = (ctFeatherstoneAlgorithm *)inboard_link->solver;
      ctSpatialMatrix fXg;
      ctVector3 vwork (0);
      ctSpatialVector s = ab.inboard_joint->get_spatial_joint_axis ();
      // fXg.form_spatial_transformation( ab.T_fg.get_transpose(), ab.T_fg.get_transpose()*ab.r_fg * -1 );
      ab.T_fg.put_transpose ( Mwork );
      Mwork.mult_v ( vwork, ab.r_fg );
      vwork *= -1.0; 
      fXg.form_spatial_transformation ( Mwork, vwork );

      // calc impulse transfered to parent
      ctSpatialMatrix6 Ident;
      Ident.identity ();
      ctSpatialMatrix6 Ident1;
      Ident1 = Ia*s*(!s)*(1.0L/sIs);
      in_feather->Ja = fXg*( Ident - Ident1 )*Ja;
      // recurse
      in_feather->propagate_impulse ();
      return;
    }
  }
  
  // we have reached the root, so go back down and calc v changes from impulses
  if ( ab.is_grounded )
    dv.zero();
  else
  {
    Ia.solve( dv, Ja*(-1.0) );
    // !me 27May2000 added this_to_world rb transform... 
    // is that right?  Or should I use spatial trasform..
    //!me tests look totally correct...
    ab.handle->add_v ( ab.handle->get_this_to_world ()*ctVector3 ( dv[3], dv[4], dv[5] ) ); 
    ab.handle->add_angular_v ( ab.handle->get_this_to_world ()*ctVector3 ( dv[0], dv[1], dv[2] ) ); 
  }
  ctArticulatedBody *out_link = ab.outboard_links.get_first ();
  ctFeatherstoneAlgorithm *out_link_solver;
  while ( out_link )
  {
    out_link_solver = (ctFeatherstoneAlgorithm *)out_link->solver;
    out_link_solver->impulse_to_v();
    out_link = ab.outboard_links.get_next();
  }
}

void ctFeatherstoneAlgorithm::zero_Ja_help ()
{
  Ja *= 0.0;
  ctArticulatedBody *out_link = ab.outboard_links.get_first ();
  ctFeatherstoneAlgorithm *out_link_solver;
  while ( out_link )
  {
    out_link_solver = (ctFeatherstoneAlgorithm *)out_link->solver;
    out_link_solver->zero_Ja_help();
    out_link = ab.outboard_links.get_next();
  }
}


void ctFeatherstoneAlgorithm::zero_Ja ()
{
  ctArticulatedBody *inboard_link;
  ctFeatherstoneAlgorithm *in_feather;

  if ( ab.inboard_joint != NULL )
  {
    inboard_link = ab.inboard_joint->inboard;
    in_feather = (ctFeatherstoneAlgorithm *)inboard_link->solver;
    in_feather->zero_Ja ();
    return;
  }

  // we are at root so go down and zero out Ja
  zero_Ja_help();
}


//!me mirtich uses a "F_coll" Frame of collision where the z-axis is the collision
//!me normal...  kinda wierd.
//!me So I stray from Mirtich here and use world coords instead of coll frame.
//!me looks like things are working... needs some testing to make sure it all works
void ctFeatherstoneAlgorithm::apply_impulse ( ctVector3 impulse_point, 
					      ctVector3 impulse_vector )
{
  ctMatrix3 iR = ab.handle->get_world_to_this ();
  ctVector3 ir = iR*(-impulse_point);
  ctVector3 ija = iR*impulse_vector;
  ctVector3 ijb = -ir % ija;
  ctSpatialVector j_coll( ija, ijb );

  zero_Ja();
  Ja = j_coll*(-1.0);

  propagate_impulse();
}

// calculate virtual mass and behaviour for an impulse applied at a point
void ctFeatherstoneAlgorithm::get_impulse_m_and_I_inv ( real *pm, ctMatrix3 *pI_inv, 
   const ctVector3 &impulse_point, const ctVector3 &impulse_vector )
{
  // theory is that I can calculate what the mass and the inertia tensor is
  // based on how this articulated body responds to a test impulse...
  // Mirtich uses 3 orthogonal test impulses... but he is doing some kind of 
  // continuous "collision integration".  I think I can get away with just one test
  //!me I'll may have to do some special magic for friction...
  ctVector3 test_j = impulse_vector;
  test_j.Normalize ();
  ctMatrix3 iR = ab.handle->get_world_to_this ();
  ctVector3 ir = iR*(-impulse_point);
  ctVector3 ija = iR*test_j;
  ctVector3 ijb = -ir % ija;
  ctSpatialVector j_coll ( ija, ijb );

  Ja = j_coll*(-1.0);
  test_impulse_response ();

  ctMatrix3 iRt = iR.get_transpose();

  ir = iRt*ir;
  ctVector3 dv_world = iRt*dv.get_b();
  *pm = 1.0/(dv_world*impulse_vector);
  ctVector3 dw_world = iRt*dv.get_a();
  ctVector3 r_x_i = -ir%impulse_vector;

  // discontinuity if ir and impulse are same direction.... 
  // I think I_inv is really ignored by collision response code 
  // in that case so safe to do this...
  //!me todo: work through equations and confirm above statement
  if( r_x_i*r_x_i < MIN_REAL )
    pI_inv->identity();
  else
  {
    // obtained by solving dw = Ia^-1(r x J)  for Ia^-1
    // !me this sucker comes away looking just like unmodified I... 
    // wierd, everything seems to work out
    // !me in the end however, tests look totaly accurate. 
    *pI_inv = ctMatrix3(dw_world*r_x_i/(r_x_i*r_x_i));
  }
}
