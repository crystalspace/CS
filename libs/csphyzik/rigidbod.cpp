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
#include "csphyzik/math3d.h"
#include "csphyzik/refframe.h"
#include "csphyzik/rigidbod.h"
#include "csphyzik/debug.h"
#include "csphyzik/contact.h"

ctRigidBody::ctRigidBody() 
  : P(0), L(0)
{
}

ctRigidBody::ctRigidBody( ctReferenceFrame &ref, ctDeltaReferenceFrame &dref ) 
  : ctDynamicEntity( ref, dref ), P(0), L(0)
{

}

ctRigidBody::~ctRigidBody()
{

}

ctRigidBody *ctRigidBody::new_ctRigidBody ()
{
  ctReferenceFrame *rf = new ctReferenceFrame ();
  ctDeltaReferenceFrame *drf = new ctDeltaReferenceFrame ();

  return new ctRigidBody ( *rf, *drf );

}

ctRigidBody *ctRigidBody::new_ctRigidBody( coord x, coord y, coord z )
{
  ctReferenceFrame *rf = new ctReferenceFrame ();
  ctDeltaReferenceFrame *drf = new ctDeltaReferenceFrame ();

  (void) x;
  (void) y;
  (void) z;
  //!me set coords
  return new ctRigidBody ( *rf, *drf );
}

// w is actually a secondary value that is calculated from L
// so need to calc L to set w
void ctRigidBody::set_angular_v ( const ctVector3 &pw )
{
//  ctMatrix3 I_world = get_I_world();
  const ctMatrix3 &R = RF.get_R ();
  ctMatrix3 I_world;
  R.similarity_transform ( I_world, I );

  // angular speed = 
  // inverse of inertia tensor in world coords * angular momentum
  dRF.w = pw;

  // from w = I_inv_world*L
  L = I_world * pw;
}

// v is a secondary value that is calculated from momentum
void ctRigidBody::set_v ( const ctVector3 &pv )
{ 
  dRF.v = pv; 
  P = pv*m;
}

// w is actually a secondary value that is calculated from L
// so need to calc L to set w
void ctRigidBody::add_angular_v ( const ctVector3 &pw )
{
//  ctMatrix3 I_world = get_I_world();
  const ctMatrix3 &R = RF.get_R ();
  ctMatrix3 I_world;
  R.similarity_transform ( I_world, I );

  // angular speed = 
  // inverse of inertia tensor in world coords * angular momentum
  dRF.w += pw;

  // from w = I_inv_world*L
  L += I_world * pw;
}

// v is a secondary value that is calculated from momentum
void ctRigidBody::add_v ( const ctVector3 &pv )
{ 
  dRF.v += pv; 
  P += pv*m;
}

int ctRigidBody::set_state ( real *state_array )
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

  return ctRigidBody::get_state_size ();
}


int ctRigidBody::set_delta_state ( real *state_array )
{
  int len;

  len = ctPhysicalEntity::set_delta_state ( state_array ); 
  state_array += len;
  
  // dP/dt = F     derivitive of momentum with time = applied force
  *state_array++ = F[0];   
  *state_array++ = F[1];
  *state_array++ = F[2];

  // dL/dt = T     derivitive of angular momentum with time = applied torque
  *state_array++ = T[0];
  *state_array++ = T[1];
  *state_array++ = T[2];

  return ctRigidBody::get_state_size ();
}

int ctRigidBody::get_state ( const real *state_array )
{
  int len;
  len = ctPhysicalEntity::get_state ( state_array );
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
  if ( m >= MIN_REAL )
    dRF.v = P/m;

  // calculate inverse of inertia tensor in world coords 
  // this result is used to calculate angular speed w
  // ctMatrix3 I_inv_world = get_I_inv_world ();
  const ctMatrix3 &R = RF.get_R ();
  ctMatrix3 I_inv_world;
  R.similarity_transform ( I_inv_world, I_inv );

  // angular speed = 
  // inverse of inertia tensor in world coords * angular momentum
  dRF.w = I_inv_world * L;

  return ctRigidBody::get_state_size ();
}

// calc I for a block of given dimensions
void ctRigidBody::calc_simple_I_tensor( real x, real y, real z )
{
  real k;
  
  if ( m > MIN_REAL )
    k = m/12.0;
  else
    k = MIN_REAL;

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

void ctRigidBody::set_m ( real pm )
{
  if ( pm > MIN_REAL )
  {
    I *= ( pm/m );
    I_inv *= ( m/pm );
    m = pm;
  }
}


void ctRigidBody::apply_impulse ( ctVector3 jx, ctVector3 jv )
{
  real mass = get_m ();
  
  P += jv;
  dRF.v = P * (( mass > MIN_REAL ) ? 1.0/mass : MAX_REAL);

  L += (jx) % jv;

  ctMatrix3 I_inv_world = get_I_inv_world ();
  dRF.w = I_inv_world * L;

}
