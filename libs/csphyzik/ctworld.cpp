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

static void __ctworld_dydt(real t, const real y[], real dy[]) {
  gcurrent_world->dydt_eval(t, y, dy);
}

void ctWorld::dydt_eval(real t, const real y[], real dy[] )
{
  //  assert_goto( gcurrent_world != NULL, "current world NULL in dydt\n",
  //               ASSERTFAIL );
  
  gcurrent_world->calc_delta_state( t, y, dy ); 

  //ASSERTFAIL:
}

ctWorld::ctWorld()
{
  ctArticulatedBody::set_joint_friction( DEFAULT_JOINT_FRICTION );

  fsm_state = CTWS_NORMAL;
  // default
  ode_to_math = new OdeRungaKutta4();

  max_state_size = DEFAULT_INIT_MAX_STATE_SIZE;
  y0 = new real[max_state_size];
  y1 = new real[max_state_size];
  y_save = new real[max_state_size];
  y_save_size = 0;
}

//!me delete _lists and ode
ctWorld::~ctWorld()
{
  if( ode_to_math ) delete ode_to_math;
  //!me lists delete here
  delete [] y0;
  delete [] y1;
  delete [] y_save;
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
  ctEntity *pe;

  pe = body_list.get_first();
  while( pe ){
    pe->init_state();
    pe = body_list.get_next();
  }

}

void ctWorld::resize_state_vector( long new_size )
{
  delete [] y0;
  delete [] y1;
  delete [] y_save;
  max_state_size = new_size + STATE_RESIZE_EXTRA;
  y0 = new real[max_state_size];
  y1 = new real[max_state_size];
  y_save = new real[max_state_size];
}


// calculate new positions of world objects after time dt
errorcode ctWorld::evolve( real t0, real t1 )
{
  long arr_size = 0;
  ctEntity *pe = body_list.get_first();

  while( pe ){
    arr_size += pe->get_state_size();
    pe = body_list.get_next();
  }

  gcurrent_world = this;
  
  if( arr_size > max_state_size ){
    resize_state_vector( arr_size );
  }

  load_state( y0 );
  
  // save this state for posible rewinding
  for( int i = 0; i < arr_size; i++ ){
    y_save[i] = y0[i];
  }
  y_save_size = arr_size;

  if( ode_to_math ){
    ode_to_math->calc_step( y0, y1, arr_size, t0, t1, __ctworld_dydt );
  }else{
    if( fsm_state == CTWS_REWOUND && t1 >= rewound_from ){
      fsm_state = CTWS_NORMAL;
      rewound_from = 0;
    }
    //!me boom!  no ode, so get out.
    return WORLD_ERR_NOODE;
  }

  reintegrate_state( y1 );

  gcurrent_world = NULL;
  
  if( fsm_state == CTWS_REWOUND && t1 >= rewound_from ){
    fsm_state = CTWS_NORMAL;
    rewound_from = 0;
  }

  return WORLD_NOERR;

}

errorcode ctWorld::rewind( real t1, real t2 )
{
  fsm_state = CTWS_REWOUND;
  rewound_from = t2;
  reintegrate_state( y_save );
  return WORLD_NOERR;
}

// find and resolve collisions
void ctWorld::collide()
{

}


// resolve forces on all bodies
void ctWorld::solve( real t )
{
  ctEntity *pe;
  ctForce *frc;

  // solve for forces affecting contents of world.  the order matters

  // first apply all environmental forces of this world to bodies
  frc = enviro_force_list.get_first();
  while( frc ){
    pe = body_list.get_first();
    while( pe ){
      if( !(fsm_state == CTWS_REWOUND && (pe->flags & CTF_NOREWIND)) )
        pe->apply_given_F( *frc );
      pe = body_list.get_next();
    }

    frc = enviro_force_list.get_next();
  }

  pe = body_list.get_first();
  while( pe ){
    if( !(fsm_state == CTWS_REWOUND && (pe->flags & CTF_NOREWIND)) )
      pe->solve(t);
    pe = body_list.get_next();
  }

}


// state to array
void ctWorld::load_state( real *state_array )
{
  ctEntity *pe;
  long state_size;

  pe = body_list.get_first();
  while( pe ){
    if( fsm_state == CTWS_REWOUND && (pe->flags & CTF_NOREWIND) ){
      state_size = pe->get_state_size();
    }else{
      state_size = pe->set_state( state_array );
    }
    state_array += state_size;
    pe = body_list.get_next();
  }

}

// array to state
void ctWorld::reintegrate_state( const real *state_array )
{
  ctEntity *pe;
  long state_size;

  pe = body_list.get_first();
  while( pe ){
    if( fsm_state == CTWS_REWOUND && (pe->flags & CTF_NOREWIND) ){
      state_size = pe->get_state_size();
    }else{
      state_size = pe->get_state( state_array );
      state_array += state_size;
    }
    pe = body_list.get_next();
  }

}


void ctWorld::load_delta_state( real *state_array )
{
  ctEntity *pe;
  long state_size;

  pe = body_list.get_first();
  while( pe ){
    if( fsm_state == CTWS_REWOUND && (pe->flags & CTF_NOREWIND) ){
      state_size = pe->get_state_size();
    }else{
      state_size = pe->set_delta_state( state_array );
      state_array += state_size;
    }
    pe = body_list.get_next();
  }

}


// add an entity to this world
errorcode ctWorld::add_entity( ctEntity *pe )
{
  if( pe ){
    body_list.add_link( pe );
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


// apply the given function to all physical entities in the system.
void ctWorld::apply_function_to_body_list( void(*fcn)( ctEntity *ppe ) )
{
  ctEntity *pe;

  pe = body_list.get_first();
  while( pe ){
    fcn( pe );
    pe = body_list.get_next();
  } 

}
