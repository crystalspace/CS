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


#include "csphyzik/articula.h"
#include "csphyzik/world.h"
#include "csphyzik/rigidbod.h"
#include "csphyzik/debug.h"

static ctWorld *gcurrent_world = NULL;

void dydt(real t, const real y[], real dy[] )
{
//	assert_goto( gcurrent_world != NULL, "current world NULL in dydt\n", ASSERTFAIL );
	
	gcurrent_world->calc_delta_state( t, y, dy ); 
	
//ASSERTFAIL:
	
}

ctWorld::ctWorld()
{
	ctPhysicalEntity::ctPhysicalEntity(); 
	ctArticulatedBody::set_joint_friction( DEFAULT_JOINT_FRICTION );

	// default
	ode_to_math = new OdeRungaKutta4();
}

//!me delete _lists and ode
ctWorld::~ctWorld()
{
	if( ode_to_math ) delete ode_to_math;
	//!me lists delete here
}

void ctWorld::calc_delta_state( real t, const real y[], real dy[] ) 
{
	// move data from y array into all entities in this world
	reintegrate_state( y );

	// zero out force accumulator and what-not
	init_state();

	// solve forces and torques of bodies in this world
	solve( t );

	// load all delta step data from bodies into ydot
	load_delta_state( dy );

}

// zero out force accumulator and other transient variables
void ctWorld::init_state()
{
ctPhysicalEntity *pe;

	pe = body_list.get_first();
	while( pe ){
		pe->init_state();
		pe = body_list.get_next();
	}

}

// calculate new positions of world objects after time dt
errorcode ctWorld::evolve( real t0, real t1 )
{
long arr_size = 0;
ctPhysicalEntity *pe = body_list.get_first();

	while( pe ){
//		if( pe->uses_ODE() ){
			arr_size +=	pe->get_state_size();
//		}
		pe = body_list.get_next();
	}

	//!me slow.  change to having y0 and y1 as members of world that get resized as needed.
	gcurrent_world = this;
	real *y0 = new real[arr_size];
	real *y1 = new real[arr_size];

	load_state( y0 );
	
	if( ode_to_math ){
		ode_to_math->calc_step( y0, y1, arr_size, t0, t1, dydt );
	}else{
		//!me boom!  no ode, so get out.
		return WORLD_ERR_NOODE;
	}

	reintegrate_state( y1 );

	//!me auch langsam.  kuch mal uber.
	delete [] y0;
	delete [] y1;
	gcurrent_world = NULL;
	
	return WORLD_NOERR;

}

// find and resolve collisions
void ctWorld::collide()
{

}


// resolve forces on all bodies
void ctWorld::solve( real t )
{
ctPhysicalEntity *pe;
ctForce *frc;

	// solve for forces affecting contents of world.  the order matters

	// first apply all environmental forces of this world to bodies
	frc = enviro_force_list.get_first();
	while( frc ){
		pe = body_list.get_first();
		while( pe ){
			pe->apply_given_F( *frc );
			pe = body_list.get_next();
		}

		frc = enviro_force_list.get_next();
	}

	pe = body_list.get_first();
	while( pe ){
		pe->solve(t);
		pe = body_list.get_next();
	}

}


// state to array
void ctWorld::load_state( real *state_array )
{
ctPhysicalEntity *pe;
long state_size;
	pe = body_list.get_first();
	while( pe ){
		//if( pe->uses_ODE() ){
			state_size = pe->set_state( state_array );
			state_array += state_size;
		//}
		pe = body_list.get_next();
	}

}

// array to state
void ctWorld::reintegrate_state( const real *state_array )
{
ctPhysicalEntity *pe;
long state_size;
	pe = body_list.get_first();
	while( pe ){
		//if( pe->uses_ODE() ){
			state_size = pe->get_state( state_array );
			state_array += state_size;
		//}
		pe = body_list.get_next();
	}

}


void ctWorld::load_delta_state( real *state_array )
{
ctPhysicalEntity *pe;
long state_size;
	pe = body_list.get_first();
	while( pe ){
		//if( pe->uses_ODE() ){
			state_size = pe->set_delta_state( state_array );
			state_array += state_size;
		//}
		pe = body_list.get_next();
	}

}

// add a rigidbody to this world
errorcode ctWorld::add_rigidbody( ctRigidBody *rb )
{
	if( rb ){
		body_list.add_link( rb );
		return WORLD_NOERR;
	}else{
		return WORLD_ERR_NULLPARAMETER;
	}
}

// add an articulated body to this world. 
errorcode ctWorld::add_articulatedbodybase( ctArticulatedBody *ab )
{
	if( ab ){
		body_list.add_link( ab );
		return WORLD_NOERR;
	}else{
		return WORLD_ERR_NULLPARAMETER;
	}
}


errorcode ctWorld::add_enviro_force( ctForce *f )
{
	if( f ){
		enviro_force_list.add_link( f );
		return WORLD_NOERR;
	}else{
		return WORLD_ERR_NULLPARAMETER;
	}
}

errorcode ctWorld::delete_articulatedbody( ctArticulatedBody *pbase )
{
	if( pbase ){
		body_list.delete_link( pbase );
		return WORLD_NOERR;
	}else{
		return WORLD_ERR_NULLPARAMETER;
	}
}
