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

#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include "csphyzik/phyzent.h"
#include "csphyzik/phyztype.h"

#define RBSTATESIZE (6 + PHYSICALENTITY_STATESIZE)  // 

// a rigid body.  has inertia tensor.
class ctRigidBody : public ctDynamicEntity
{
public:
/*
**	Constructors/destructors/statics
*/
	ctRigidBody();
	ctRigidBody( ctReferenceFrame &ref );
//	ctRigidBody( ctReferenceFrame &ref,  );
	virtual ~ctRigidBody();

	// convienent to create enitites with parent as world frame of reference
	static ctRigidBody *new_ctRigidBody();
	static ctRigidBody *new_ctRigidBody( coord x, coord y, coord z );

	static int get_RB_state_size(){ return RBSTATESIZE; }

/*
**	Member functions
*/
	// calculate interia tensor as if body was a solid 
	// block of given dimensions.  
	// NOTE: m must be given first
	void calc_simple_I_tensor( real width, real height, real depth );

	const ctMatrix3 &get_I(){ return I; }
	const ctMatrix3 &get_I_inv(){ return I_inv; }

  virtual ctMatrix3 get_impulse_I_inv(){ 
    const ctMatrix3 &R = RF.get_R();
    ctMatrix3 I_inv_world = R * I_inv * (R.get_transpose()); 
    return I_inv_world;
  }

	ctVector3 get_angular_P(){ return L; }
	
	// ODE interface
	virtual int get_state_size(){ return RBSTATESIZE; }
	virtual int set_state( real *state_array ); 
	virtual int get_state( const real *state_array );  
	virtual int set_delta_state( real *state_array ); 
	virtual void set_angular_v( const ctVector3 &pw );
  virtual void set_v( const ctVector3 &pv );
	virtual void set_m( real pm );
  
  // collision response
  virtual void resolve_collision( ctCollidingContact *cont );
  virtual void apply_impulse( ctVector3 impulse_point, ctVector3 impulse_vector );


protected:
	ctVector3 P;	// momentum
	ctVector3 L;    // angular momentum
	ctMatrix3 I;    // Intertia tensor
	ctMatrix3 I_inv; // inverse of I

};

#endif
