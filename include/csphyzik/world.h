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

#ifndef PHYZ_WORLD_H
#define PHYZ_WORLD_H

#include "csphyzik/phyztype.h"
#include "csphyzik/phyzent.h"
#include "csphyzik/linklist.h"
#include "csphyzik/odesolve.h"

enum errorcode { WORLD_NOERR, WORLD_ERR_NULLPARAMETER, WORLD_ERR_NOODE, WORLD_ERR_SHITHAPPEND, WORLD_ERR_OTHERSTUFF };

#define DEFAULT_INIT_MAX_STATE_SIZE  1024
#define STATE_RESIZE_EXTRA 256

class ctRigidBody;
class ctArticulatedBody;
class ctForce;

class ctWorld : public ctPhysicalEntity
{
public:

	ctWorld();
	virtual ~ctWorld();  //!me delete _lists and ode

	void calc_delta_state( real t, const real y[], real ydot[] );

  // evolve the state of the system.  calc forces, determine new v, etc..
	errorcode evolve( real t1, real t2 );
  
  // rewind the state of the system to the time just before evolve was called.
  errorcode rewind();  
	
  void solve( real t );
	errorcode add_rigidbody( ctRigidBody *rb );
	errorcode add_articulatedbodybase( ctArticulatedBody *ab );
	errorcode add_enviro_force( ctForce *f );

	errorcode delete_articulatedbody( ctArticulatedBody *pbase );

protected:
	// take state values( position, velocity, orientation, ... ) from 
	// this worlds physical entities and put them into the array
	void load_state( real *state_array );

	// take state values from array and reintegrate them into physicalentity 
	// structures of this world.  the reverse operation of load_state.
	// state_array must have been filled out by load_state.
	void reintegrate_state( const real *state_array );
	
	// put first derivative of all world entities into state 
	void load_delta_state( real *state_array );

	void init_state();

	void collide();

  void resize_state_vector( long new_size );

	ctLinkList_ctPhysicalEntity body_list;
	ctLinkList_ctForce enviro_force_list;

	OdeSolver *ode_to_math;  // would an equation by any other name smell as sweet?

  // state vectors for ODE interface
  real *y_save;  // used to save state so it can be rewound to pre-ODE state
	real *y0;
	real *y1;

  long y_save_size;
  long max_state_size;

};


#endif
