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

#include "csphyzik/math3d.h"
#include "csphyzik/refframe.h"
#include "csphyzik/rigidbod.h"
#include "csphyzik/debug.h"
#include "csphyzik/contact.h"

ctRigidBody::ctRigidBody()
{
}

ctRigidBody::ctRigidBody( ctReferenceFrame &ref ) : ctDynamicEntity( ref )
{

}

ctRigidBody::~ctRigidBody()
{

}


ctRigidBody *ctRigidBody::new_ctRigidBody()
{
ctReferenceFrame *rf = new ctReferenceFrame();
	return new ctRigidBody( *rf );

}
ctRigidBody *ctRigidBody::new_ctRigidBody( coord x, coord y, coord z )
{
ctReferenceFrame *rf = new ctReferenceFrame();
//!me set coords
	return new ctRigidBody( *rf );

}


// w is actually a secondary value that is calculated from L
// so need to calc L to set w
void ctRigidBody::set_angular_v( const ctVector3 &pw )
{
	ctMatrix3 I_inv_world = get_I_inv_world();

	// angular speed = 
	// inverse of inertia tensor in world coords * angular momentum
	w = pw;
	L = I_inv_world.get_transpose() * w;
}

// v is a secondary value that is calculated from momentum
void ctRigidBody::set_v( const ctVector3 &pv )
{ 
  v = pv; 
  P = v*m;
}

int ctRigidBody::set_state( real *state_array )
{
int len;

	len = ctPhysicalEntity::set_state( state_array );	
	state_array += len;
	
	*state_array++ = P[0];
	*state_array++ = P[1];
	*state_array++ = P[2];

	*state_array++ = L[0];
	*state_array++ = L[1];
	*state_array++ = L[2];

	return ctRigidBody::get_state_size(); 
}


int ctRigidBody::set_delta_state( real *state_array )
{
int len;

	len = ctPhysicalEntity::set_delta_state( state_array );	
	state_array += len;
	
	// dP/dt = F     derivitive of momentum with time = applied force
	*state_array++ = F[0];   
	*state_array++ = F[1];
	*state_array++ = F[2];

	// dL/dt = T     derivitive of angular momentum with time = applied torque
	*state_array++ = T[0];
	*state_array++ = T[1];
	*state_array++ = T[2];

	return ctRigidBody::get_state_size(); 

}

int ctRigidBody::get_state( const real *state_array )
{
int len;
	len = ctPhysicalEntity::get_state( state_array );
	state_array += len;

	// momentum
	P[0] = *state_array++;
	P[1] = *state_array++;
	P[2] = *state_array++;

	// angular momentum
	L[0] = *state_array++;
	L[1] = *state_array++;
	L[2] = *state_array++;

	// calc velocity from eqn:  p = mv    momentum = mass * velocity
	if( m >= MIN_REAL ){
		v = P/m;
	}

	// calculate inverse of inertia tensor in world coords 
	// this result is used to calculate angular speed w
	ctMatrix3 I_inv_world = get_I_inv_world();

	// angular speed = 
	// inverse of inertia tensor in world coords * angular momentum
	w = I_inv_world * L;

	return ctRigidBody::get_state_size(); 
}

// calc I for a block of given dimensions
void ctRigidBody::calc_simple_I_tensor( real x, real y, real z )
{
real k;
	
	if( m > MIN_REAL ){
		k = m/12.0;
	}else{
		k = MIN_REAL;
	}
	for( int i = 0; i < 3; i++ )
		for( int j = 0; j < 3; j++ )
			I[i][j] = 0.0;

	I[0][0] = (y*y + z*z)*k;
	I[1][1] = (x*x + z*z)*k;
	I[2][2] = (x*x + y*y)*k;
	
	I_inv[0][0] = ( y > MIN_REAL || z > MIN_REAL ) ? 1.0/I[0][0] : MAX_REAL;
	I_inv[1][1] = ( x > MIN_REAL || z > MIN_REAL ) ? 1.0/I[1][1] : MAX_REAL;
	I_inv[2][2] = ( x > MIN_REAL || y > MIN_REAL ) ? 1.0/I[2][2] : MAX_REAL;
}

void ctRigidBody::set_m( real pm )
{
	if( pm > MIN_REAL ){
		I *= ( pm/m );
		I_inv *= ( m/pm );
		m = pm;
	}
}


// collision response
void ctRigidBody::resolve_collision( ctCollidingContact *cont )
{
ctVector3 j;
real v_rel;       // relative velocity of collision points
ctVector3 ra_v, rb_v;
real j_magnitude;
real bottom;
real ma_inv, mb_inv;   // 1/mass_body
real rota, rotb;       // contribution from rotational inertia
ctVector3 & n = cont->n;
ctVector3 ra, rb;      // center of body to collision point in inertail ref frame 
// keep track of previous object collided with.
// in simultaneous collisions with the same object the restituion should
// only be factored in once.  So all subsequent collisions are handled as
// seperate collisions, but with a restitution of 1.0
// if they are different objects then treat them as multiple collisions with
// normal restitution.
//!me this isn't actually a very good method.... maybe something better can be 
//!me implemented once contact force solver is implemented
ctPhysicalEntity *prev;

//!debug
//int hit_cout = 0;

  // since NULL is used for an immovable object we need a 
  // different "nothing" pointer
  prev = this; 

  while( cont != NULL ){

    ctVector3 body_x = get_pos();
    ra = cont->contact_p - body_x;

    ra_v = get_angular_v()%ra + get_v();

    if( cont->body_b == NULL ){
      v_rel = n*ra_v;
    }else{
      //rb = (cont->body_b->get_this_to_world())*cont->p_b;
      rb = cont->contact_p - cont->body_b->get_pos();
      rb_v = cont->body_b->get_angular_v()%rb + cont->body_b->get_v();
      v_rel = n*(ra_v - rb_v);
    }

    // if the objects are traveling towards each other do collision response
    if( v_rel < 0 ){

 //         DEBUGLOGF2( "contact %lf o %lf, ", cont->contact_p[0], body_x[0] );
 //   DEBUGLOGF2( "contact %lf o %lf, ", cont->contact_p[1], body_x[1] );
 //   DEBUGLOGF2( "contact %lf o %lf\n ", cont->contact_p[2], body_x[2] );

      ma_inv = 1.0/get_impulse_m();
      rota = n * ((get_impulse_I_inv()*( ra%n ) )%ra);  

      if( cont->body_b == NULL ){
        // hit some kind of immovable object
        mb_inv = 0;
        rotb = 0;
      }else{
        mb_inv = 1.0/cont->body_b->get_impulse_m();
        rotb = n * ((cont->body_b->get_impulse_I_inv()*( rb%n ) )%rb);
      }

      // bottom part of equation
      bottom = ma_inv + mb_inv + rota + rotb;
      
      if( prev != cont->body_b ){
        j_magnitude = -(1.0 + cont->restitution ) * v_rel / bottom;
      }else{
        // if we are dealing with a simulatneous collisin with with
        // same object.
        j_magnitude = -(1.0 + 1.0 ) * v_rel / bottom;
      }

      j = n*j_magnitude;
      apply_impulse( ra, j );

      if( cont->body_b != NULL ){
        cont->body_b->apply_impulse( rb, j*(-1.0) );
      }

      // treat next simultaneous collision as a seperate collision.
      prev = cont->body_b;

  //    DEBUGLOGF( "respond %d\n", ++hit_cout );
    }
    cont = cont->next;  
  }
}

void ctRigidBody::apply_impulse( ctVector3 jx, ctVector3 jv )
{
real mass = get_impulse_m();
  
  P += jv;
  v = P * (( mass > MIN_REAL ) ? 1.0/mass : MAX_REAL);

  L += jx % jv;

	ctMatrix3 I_inv_world = get_I_inv_world();
  w = I_inv_world * L;

}