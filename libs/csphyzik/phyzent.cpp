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

#ifndef __NO_CRYSTALSPACE__
#include "cssysdef.h"
#endif
#include "csphyzik/phyzent.h"
#include "csphyzik/contact.h"
#include "csphyzik/refframe.h"
#include "csphyzik/mathutil.h"
#include "csphyzik/solver.h"
#include "csphyzik/debug.h"

// return the relative velocity between up to two bodies at a point in world space
ctVector3 ctReferenceFrameEntity::get_relative_v( ctReferenceFrameEntity *body_b, const ctVector3 &the_p )
{
  ctVector3 v_rel;
  ctVector3 body_x = get_pos();
  ctVector3 ra = the_p - body_x;

  ctVector3 ra_v = get_angular_v()%ra + get_v();

  if( body_b == NULL ){
    v_rel = ra_v;
  }else{
    ctVector3 rb = the_p - body_b->get_pos();
    ctVector3 rb_v = body_b->get_angular_v()%rb + body_b->get_v();
    v_rel = (ra_v - rb_v);
  }

  return v_rel;

}

// PONG collision model
// basic collision model for for objects with no mass.
/*void ctPhysicalEntity::resolve_collision( ctCollidingContact *cont )
{
ctVector3 j;

  if( cont == NULL )
    return;

  j = ( cont->n*((get_v())*cont->n) )*( -1.0 - cont->restitution );
  apply_impulse( cont->contact_p, j );

  if( cont->body_b != NULL ){
    j = ( cont->n*((cont->body_b->get_v())*cont->n) )
      *( -1.0 - cont->restitution );
    cont->body_b->apply_impulse( cont->contact_p, j );
  }
}
*/

// collision response
void ctReferenceFrameEntity::resolve_collision( ctCollidingContact *cont )
{
ctVector3 j;
real v_rel;       // relative velocity of collision points
//ctVector3 ra_v, rb_v;
real j_magnitude;
real bottom;
ctMatrix3 imp_I_inv;
real ma_inv, mb_inv;   // 1/mass_body
real rota, rotb;       // contribution from rotational inertia
ctVector3 n;
ctVector3 ra, rb;      // center of body to collision point in inertail ref frame 
// keep track of previous object collided with.
// in simultaneous collisions with the same object the restituion should
// only be factored in once.  So all subsequent collisions are handled as
// seperate collisions, but with a restitution of 1.0
// if they are different objects then treat them as multiple collisions with
// normal restitution.
//!me this isn't actually a very good method.... maybe something better can be 
//!me implemented once contact force solver is implemented
ctReferenceFrameEntity *prev;
ctCollidingContact *head_cont = cont;
//!debug
//int hit_cout = 0;

  // since NULL is used for an immovable object we need a 
  // different "nothing" pointer
  prev = this; 

  while( cont != NULL ){

    n = cont->n;
    // get component of relative velocity along collision normal
    v_rel = n*get_relative_v( cont->body_b, cont->contact_p );

    // if the objects are traveling towards each other do collision response
    if( v_rel < 0 ){

      ra = cont->contact_p - get_pos();

      get_impulse_m_and_I_inv( &ma_inv, &imp_I_inv, ra, n );
      ma_inv = 1.0/ma_inv;
      rota = n * ((imp_I_inv*( ra%n ) )%ra);  

      if( cont->body_b == NULL ){
        // hit some kind of immovable object
        mb_inv = 0;
        rotb = 0;
      }else{
        rb = cont->contact_p - cont->body_b->get_pos();
        cont->body_b->get_impulse_m_and_I_inv( &mb_inv, &imp_I_inv, rb, n*(-1.0) );
        mb_inv = 1.0/mb_inv;
        rotb = n * ((imp_I_inv*( rb%n ) )%rb);
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

        //!me start experimental code
              //!me this seems to actually work...
        //!me need a better way of calculating how much to nudge, and then
        //!me need to account for that when specifying resting contact ...
//!me nudge object away a little bit, so that if another collision happens
  //!me immediately in the next frame, it's closest feature wont get stuck on 
  //!me this one... it's a problem with collision detection and angular v
  //!me that will result in interpenetration 'cause most collision detection
  //!me codes only return closest features from when there was _no_ collision
  /*      if( get_impulse_m() < 1000 ){
  ctVector3 cur_pos = get_pos() + j.Unit()*0.05;
  set_pos( cur_pos[0], cur_pos[1], cur_pos[2] );
        }*/
  //!me end experiment

      if( cont->body_b != NULL ){
        cont->body_b->apply_impulse( rb, j*(-1.0) );

  //!me start experimental code
        //!me this seems to actually work...
        //!me need a better way of calculating how much to nudge, and then
        //!me need to account for that when specifying contact collision...
//!me nudge object away a little bit, so that if another collision happens
  //!me immediately in the next frame, it's closest feature wont get stuck on 
  //!me this one... it's a problem with collision detection and angular v
  //!me happens when there are simulatneous contacts between different features
  //!me between two objects.... could screw up when there are three objects...
        /*if( cont->body_b->get_impulse_m() < 1000 ){
       ctVector3 cur_pos = cont->body_b->get_pos() + j.Unit()*(-0.05);
  cont->body_b->set_pos( cur_pos[0], cur_pos[1], cur_pos[2] );
        }*/
//!me end experiment

      }

      // treat next simultaneous collision as a seperate collision.
      prev = cont->body_b;

    }
    cont = cont->next;  
  }

  // now check if any of the contacts are in resting contact
  cont = head_cont;

/*  while( cont != NULL ){
    // get component of relative velocity along collision normal
    v_rel = cont->n*get_relative_v( cont->body_b, cont->contact_p );

    if( fabs(v_rel) < MIN_CONTACT ){
      ctContact *r_cont = new ctContact;

      r_cont->body_a = (ctRigidBody *)(cont->body_a);  //!me bad but works for now
      r_cont->body_b = (ctRigidBody *)(cont->body_b);
      r_cont->n = cont->n;
      r_cont->ea = cont->ea;
      r_cont->eb = cont->eb;
      r_cont->vf = cont->vf;
      r_cont->contact_p = cont->contact_p;
   
    }

    cont = cont->next;
  }
*/

}


/********** PHYSICAL ENTITY ****************/

ctPhysicalEntity::ctPhysicalEntity() : 
  v(0), w(0), RF( ctReferenceFrame::universe() ), F(0), T(0)
{
  RF.add_ref( RF );
}

ctPhysicalEntity::ctPhysicalEntity( ctReferenceFrame &ref ) :
  v(0), w(0), RF( ref ), F(0), T(0)
{
  RF.add_ref( RF );
}

ctPhysicalEntity::~ctPhysicalEntity()
{
  RF.remove_ref( RF );
}


void ctPhysicalEntity::apply_given_F( ctForce& /*frc*/ )
{
  // notin
}

//!me make sure it's the right direction.. should be counter-clockwise in RHS
void ctPhysicalEntity::rotate_around_line( ctVector3 &paxis, real ptheta )
{
  ctMatrix3 new_T;
  
  R_from_vector_and_angle( paxis, -ptheta, new_T );
//  RF.set_T(new_T*RF.get_T());  //!me is this right?
  RF.set_R(new_T*RF.get_R());  //!me is this right?

}

// add this bodies state to the state vector buffer passed in. 
// increment state buffer to point after added state.  upload
int ctPhysicalEntity::set_state( real *state_array )
{
  ctVector3 o;
  ctMatrix3 M;

  o = RF.get_offset();
  M = RF.get_R();

  *state_array++ = o[0];
  *state_array++ = o[1];
  *state_array++ = o[2];

  *state_array++ = M[0][0];
  *state_array++ = M[0][1];
  *state_array++ = M[0][2];
  *state_array++ = M[1][0];
  *state_array++ = M[1][1];
  *state_array++ = M[1][2];
  *state_array++ = M[2][0];
  *state_array++ = M[2][1];
  *state_array++ = M[2][2];
  return ctPhysicalEntity::get_state_size();
}

// add change in state vector over time to state buffer parameter.
int ctPhysicalEntity::set_delta_state( real *state_array )
{
  ctMatrix3 M;

  M = RF.get_R();

  // dx/dt = v   change in position over time is velocity
  *state_array++ = v[0];
  *state_array++ = v[1];
  *state_array++ = v[2];

  // dR/dt = ~w*R   ~w is cross product matrix thing.  R is rotation matrix
  // ~w = [   0   -w.z   w.y  ]   pre-mult of ~w with R has the
  //    [ w.z      0  -w.x  ] same effect as x-prod of w with
  //    [  -w.y    w.x     0  ] each row of R

  *state_array++ = -w[2]*M[1][0] + w[1]*M[2][0];
  *state_array++ = -w[2]*M[1][1] + w[1]*M[2][1];
  *state_array++ = -w[2]*M[1][2] + w[1]*M[2][2];
  *state_array++ = w[2]*M[0][0] - w[0]*M[2][0];
  *state_array++ = w[2]*M[0][1] - w[0]*M[2][1];
  *state_array++ = w[2]*M[0][2] - w[0]*M[2][2];
  *state_array++ = -w[1]*M[0][0] + w[0]*M[1][0];
  *state_array++ = -w[1]*M[0][1] + w[0]*M[1][1];
  *state_array++ = -w[1]*M[0][2] + w[0]*M[1][2];  

  return ctPhysicalEntity::get_state_size();
}

// download state from buffer into this entity
int ctPhysicalEntity::get_state( const real *state_array )
{
  ctVector3 o;
  ctMatrix3 M;

  o[0] = *state_array++;
  o[1] = *state_array++;
  o[2] = *state_array++;

  M[0][0] = *state_array++;
  M[0][1] = *state_array++;
  M[0][2] = *state_array++;
  M[1][0] = *state_array++;
  M[1][1] = *state_array++;
  M[1][2] = *state_array++;
  M[2][0] = *state_array++;
  M[2][1] = *state_array++;
  M[2][2] = *state_array++;

  //!me probably don't need to do it every frame
  M.orthonormalize();

  RF.set_offset( o );
  RF.set_R( M );
  return ctPhysicalEntity::get_state_size();
}


void ctPhysicalEntity::set_v( const ctVector3 &pv )
{

  v = pv; 
}

void ctPhysicalEntity::set_angular_v( const ctVector3 &pw )
{ 
  w = pw; 
}


void ctPhysicalEntity::apply_impulse( ctVector3 /*jx*/, ctVector3 jv )
{
  set_v( v + jv );
}

//************ ctDynamicEntity

ctDynamicEntity::ctDynamicEntity()
{
  m = 10;
  solver = new ctSimpleDynamicsSolver( *this );
}

ctDynamicEntity::ctDynamicEntity( ctReferenceFrame &ref ) : ctPhysicalEntity( ref )
{
  m = 10;
  solver = new ctSimpleDynamicsSolver( *this );
}

ctDynamicEntity::~ctDynamicEntity()
{
}

void ctDynamicEntity::apply_given_F( ctForce &frc )
{
  frc.apply_F( *this );
}


void ctDynamicEntity::set_m( real pm ){ m = pm; }

