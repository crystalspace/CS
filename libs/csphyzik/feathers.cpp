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

#include "sysdef.h"
#include "csphyzik/feathers.h"
#include "csphyzik/articula.h"
#include "csphyzik/joint.h"
#include "csphyzik/debug.h"

// resolve dynamics
void ctFeatherstoneAlgorithm::solve( real t )
{
  if( ab.is_grounded ){
		fsolve_grounded( t );
	}else{
		fsolve_floating( t );
	}
}

// solve using featherstones algorithm where the base of the robot
// is fixed to the inertial frame.
void ctFeatherstoneAlgorithm::fsolve_grounded( real t )
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
  
  init_link();
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
	out_link = ab.outboard_links.get_first();
	while( out_link ){

		out_link2 = out_link->outboard_links.get_first();
		while( out_link2 ){
			out_link_solver = (ctFeatherstoneAlgorithm *)out_link2->solver;
			out_link_solver->compute_Ia_Za();
			out_link2 = out_link->outboard_links.get_next();
		}
		out_link = ab.outboard_links.get_next();
	}

	// compute joint and spatial acceleration of links
	// link 0 is fixed and doesn't accelerate
	a.zero();
	out_link = ab.outboard_links.get_first();
	while( out_link ){
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
	while( out_link ){
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
	while( out_link ){
		out_link_solver = (ctFeatherstoneAlgorithm *)out_link->solver;
		out_link_solver->compute_joint_a();
		out_link = ab.outboard_links.get_next();
	}

}

// init values before processing with featherstone of each articulated body 
// init applied force and coriolous force for this link 
void ctFeatherstoneAlgorithm::init_link()
{
ctArticulatedBody *out_link;
ctJoint *jnt;
ctPhysicalEntity *pe_f;
ctRigidBody *pe_g;
ctFeatherstoneAlgorithm *out_link_solver;

	// spatial transform matrix from f to g.  Used later on
	gXf.form_spatial_transformation( ab.T_fg, ab.r_fg );

	pe_g = ab.handle;

	if( pe_g == NULL )
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
	if( jnt == NULL || jnt->inboard == NULL ){
		// this will happen with a floating links 0 handle
		ctSpatialVector ct( 0.0,0.0,0.0,0.0,0.0,0.0 );
		c = ct;
	}else{

		pe_f = jnt->inboard->handle;

		if( pe_f ){
      jnt->calc_coriolus( ab.r_fg, ab.T_fg*jnt->inboard->w_body, c );
    }else{
			ctSpatialVector ct( 0.0,0.0,0.0,0.0,0.0,0.0 );
			c = ct;
		}

	}


	// iterate to next link
	out_link = ab.outboard_links.get_first();
	while( out_link ){
		out_link_solver = (ctFeatherstoneAlgorithm *)out_link->solver;
		out_link_solver->init_link();
		out_link = ab.outboard_links.get_next();
	}

}

// featherstone algorithm first step
// calc spatial Inertia tensor and Bias force vector for all links
void ctFeatherstoneAlgorithm::compute_Ia_Za()
{
ctJoint *jnt;
ctArticulatedBody *out_link;
ctArticulatedBody *ab_f;
ctFeatherstoneAlgorithm *out_link_solver;
ctFeatherstoneAlgorithm *svr_f;
//ctMatrix3 Mwork;
ctVector3 vwork;
ctSpatialVector svwork;
//ctSpatialMatrix sMwork;
//ctSpatialVector ZaIac;

	// recurse to end, then unwind and do the work
	out_link = ab.outboard_links.get_first();
	while( out_link ){
		out_link_solver = (ctFeatherstoneAlgorithm *)out_link->solver;
		out_link_solver->compute_Ia_Za();
		out_link = ab.outboard_links.get_next();
	}
	// unwinding recursive stack starting here 
	// from link n to link 2

	ctSpatialMatrix fXg;
  // fXg.form_spatial_transformation( ab.T_fg.get_transpose(), ab.T_fg.get_transpose()*ab.r_fg * -1 );
  ab.T_fg.put_transpose( Mwork );
  Mwork.mult_v( vwork, ab.r_fg );
  vwork *= -1.0; 
  fXg.form_spatial_transformation( Mwork, vwork );

	jnt = ab.inboard_joint;
	if( jnt == NULL )
		return;

	ab_f = jnt->inboard;
	if( ab_f == NULL || ab_f->solver == NULL ){
		return;
	}
	
	svr_f = (ctFeatherstoneAlgorithm *)ab_f->solver;

	ctSpatialVector s = jnt->get_spatial_joint_axis();

  // sIs = (!s)*(Ia*s);
  Ia.mult_v( svwork, s );
  sIs = s.spatial_dot( svwork );

	ctSpatialMatrix innerM;
  // innerM = ( Ia*s*(!s)*Ia )*(1.0L/sIs);
  Ia.mult_v( svwork, s );
  sMwork = svwork*(!s);
  sMwork.mult_M( innerM, Ia );
  innerM *= (1.0L/sIs);

	// calc Ia for parent link
  // svr_f->Ia += fXg*( Ia - innerM )*gXf;
  sMwork.subtract2( Ia, innerM );
  fXg.mult_M( innerM, sMwork );
  innerM *= gXf;
  svr_f->Ia += innerM; 

	// force from any robot motors are factored in here.  also joint friction
  // external_f = -((!s)*(Za + Ia*c ));
  Ia.mult_v( ZaIac, c );
  ZaIac += Za;
  real external_f = -(s.spatial_dot( ZaIac ));
	QsZIc = ( jnt->get_actuator_magnitude( external_f, sIs ) + external_f );
	sIsQsZIc_computed = true;

	// calc Za for parent link
  // svr_f->Za = svr_f->Za + fXg*( Za + Ia*c + ( Ia*s*(QsZIc/sIs) ) );
  Ia.mult_v( svwork, s );
  svwork *= (QsZIc/sIs);
  ZaIac += svwork;
  fXg.mult_v( svwork, ZaIac );
  svr_f->Za += svwork;

	// unwind recursive stack
	return;
}

// final step of featherstone algorithm.  Compute joint accelerations and spatial accel
void ctFeatherstoneAlgorithm::compute_joint_a()
{
ctJoint *jnt;
ctArticulatedBody *out_link;
ctFeatherstoneAlgorithm *prev_link_solver;
ctFeatherstoneAlgorithm *out_link_solver;
ctSpatialVector svwork, gXfa;

	jnt = ab.inboard_joint;
	prev_link_solver = (ctFeatherstoneAlgorithm *)jnt->inboard->solver;

	ctSpatialVector s = jnt->get_spatial_joint_axis();

	if( !sIsQsZIc_computed ){
		sIs = (!s)*(Ia*s);
		// force from any robot motors are factored in here
		real external_f = -((!s)*(Za + Ia*c ));
		QsZIc = ( jnt->get_actuator_magnitude( external_f, sIs ) + external_f );
	}

	sIsQsZIc_computed = false;

	// Alright! finaly we get the result all that code was for.
  // jnt->qa = ( QsZIc - (!s)*( (Ia*gXf)*prev_link_solver->a ) )/sIs;
  gXf.mult_v( gXfa, prev_link_solver->a );
  Ia.mult_v( svwork, gXfa );
  jnt->qa = ( QsZIc - s.spatial_dot( svwork ) ) / sIs;
  // a = gXf*prev_link_solver->a + c + s*jnt->qa;
  gXfa += c;
  a.add2( gXfa, s*jnt->qa );

	// iterate to next link
	out_link = ab.outboard_links.get_first();
	while( out_link ){
		out_link_solver = (ctFeatherstoneAlgorithm *)out_link->solver;
		out_link_solver->compute_joint_a();
		out_link = ab.outboard_links.get_next();
	}
}
