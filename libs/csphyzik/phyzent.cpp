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

/********** PHYSICAL ENTITY ****************/

ctPhysicalEntity::ctPhysicalEntity() 
  :  RF( ctReferenceFrame::universe() ), 
     dRF(ctDeltaReferenceFrame::universe()), F(0), T(0)
{
  RF.add_ref( RF );
  dRF.add_ref( dRF );
}

ctPhysicalEntity::ctPhysicalEntity ( ctReferenceFrame &ref, 
				     ctDeltaReferenceFrame &dref  ) 
  : RF( ref ), dRF( dref ), F(0), T(0)
{
  RF.add_ref ( RF );
  dRF.add_ref ( dRF );
}

ctPhysicalEntity::~ctPhysicalEntity()
{
  RF.remove_ref( RF );
}


void ctPhysicalEntity::apply_given_F ( ctForce& /*frc*/ )
{
  // notin
}

//!me make sure it's the right direction.. should be counter-clockwise in RHS
void ctPhysicalEntity::rotate_around_line ( ctVector3 &paxis, real ptheta )
{
  ctMatrix3 new_T;
  
  R_from_vector_and_angle ( paxis, -ptheta, new_T );
//  RF.set_T(new_T*RF.get_T());  //!me is this right?
  RF.set_R(new_T*RF.get_R ());  //!me is this right?

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
int ctPhysicalEntity::set_delta_state ( real *state_array )
{
  ctMatrix3 M;

  M = RF.get_R();

  // dx/dt = v   change in position over time is velocity
  *state_array++ = dRF.v[0];
  *state_array++ = dRF.v[1];
  *state_array++ = dRF.v[2];

  // dR/dt = ~w*R   ~w is cross product matrix thing.  R is rotation matrix
  // ~w = [   0   -w.z   w.y  ]   pre-mult of ~w with R has the
  //    [ w.z      0  -w.x  ] same effect as x-prod of w with
  //    [  -w.y    w.x     0  ] each row of R
  ctVector3 w = dRF.w;
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
int ctPhysicalEntity::get_state ( const real *state_array )
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
  M.orthonormalize ();

  RF.set_offset ( o );
  RF.set_R ( M );
  return ctPhysicalEntity::get_state_size ();
}


void ctPhysicalEntity::set_v ( const ctVector3 &pv )
{
  dRF.v = pv; 
}

void ctPhysicalEntity::set_angular_v ( const ctVector3 &pw )
{ 
  dRF.w = pw; 
}


void ctPhysicalEntity::apply_impulse ( ctVector3 /*jx*/, ctVector3 jv )
{
  set_v( dRF.v + jv );
}

//************ ctDynamicEntity

ctDynamicEntity::ctDynamicEntity ()
{
  m = 10;
  solver = new ctSimpleDynamicsSolver ( *this );
}

ctDynamicEntity::ctDynamicEntity ( ctReferenceFrame &ref, 
				   ctDeltaReferenceFrame &dref ) 
  : ctPhysicalEntity ( ref, dref )
{
  m = 10;
  solver = new ctSimpleDynamicsSolver ( *this );
}

ctDynamicEntity::~ctDynamicEntity ()
{
}

void ctDynamicEntity::apply_given_F ( ctForce &frc )
{
  frc.apply_F( *this );
}


void ctDynamicEntity::set_m ( real pm )
{ 
  m = pm; 
}

