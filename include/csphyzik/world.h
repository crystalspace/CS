/*
    Dynamics/Kinematics modeling and simulation library.
    Copyright (C) 1999 by Michael Alexander Ewert & Noah Gibbs

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
#include "csphyzik/entity.h"
#include "csphyzik/phyzent.h"
#include "csphyzik/linklist.h"
#include "csphyzik/odesolve.h"
#include "csphyzik/ctcat.h"
#include "csutil/csdllist.h"

enum worldstate { CTWS_NORMAL, CTWS_REWOUND };

enum errorcode { WORLD_NOERR, WORLD_ERR_NULLPARAMETER, WORLD_ERR_NOODE,
                 WORLD_ERR_SHITHAPPEND, WORLD_ERR_OTHERSTUFF };

#define DEFAULT_INIT_MAX_STATE_SIZE  1024
#define STATE_RESIZE_EXTRA 256

class ctForce;
// State-alloc tracking structure:
class AllocNode
{
 public:
  int offset;
  int size;
};

class ctWorld : public ctEntity
{
public:

  ctWorld ();
  /// !me delete _lists and ode
  virtual ~ctWorld ();

  void calc_delta_state ( real t, const real y[], real ydot[] );

  /// evolve the state of the system.  calc forces, determine new v, etc..
  errorcode evolve ( real t1, real t2 );

  /**
   * Rewind the state of the system to the time just before evolve was called.
   * pass in the correct time frame.
   */
  errorcode rewind ( real t1, real t2 );

  /// Maximum number of times to back-up in an evolve loop.
  void set_max_time_subdivisions ( long pmt )
  { max_time_subdivisions = pmt; }

  void register_catastrophe_manager ( ctCatastropheManager *pcm );

  void solve ( real t );
  errorcode add_entity ( ctEntity *pe );
  errorcode add_enviro_force ( ctForce *f );

  errorcode delete_entity ( ctEntity *pb );

  /// set the ODE solver used to evolve the system
  void set_ODE_solver ( OdeSolver *pode )
  {
    if( ode_to_math ) delete ode_to_math;
    ode_to_math = pode;
  }

  /// Apply the given function to all physical entities in the system.
  void apply_function_to_body_list ( void(*fcn)( ctEntity *pe ) );

  /**
   * This function evaluates dydt().  By overriding it in child classes,
   * ctWorld objects that use nondefault derivatives (like catastrophes,
   * constraints, and certain forces) can be created.
   */
  virtual void dydt_eval (real t, const real y[], real dy[]);

  /// Collision response routines and utils
  void resolve_collision ( ctCollidingContact *cont );
  ctVector3 get_relative_v
   ( ctPhysicalEntity *body_a, ctPhysicalEntity *body_b, const ctVector3 &the_p );

  virtual int get_state_size ()
  { return 0; }
  /// Add this body's state to the state vector buffer passed in.
  virtual int set_state ( real * )
  { return 0; }
  /// download state from buffer into this entity
  virtual int get_state ( const real * )
  { return 0; }
  /// add change in state vector over time to state buffer parameter.
  virtual int set_delta_state ( real * )
  { return 0; }

  /// Static state-alloc stuff
  int state_size;
  csDLinkList free_blocks;
  csDLinkList used_blocks;

  ///**********  State vector interface *********
  virtual int  state_alloc (int size);
  virtual void state_free (int offset);
  virtual int  state_realloc (int offset, int newsize)
  {
    int newloc = state_alloc(newsize);
    if(!newloc) return 0;
    // Okay, got state-space -- free old loc, return new one
    state_free(offset);
    return newloc;
  }

  /// State-alloc helper functions

  /**
   * Removes a block from the used_blocks list.
   * Returns the block if successful, 0 if a block with that offset
   * wasn't found.
   */
  AllocNode *sa_make_unused (int offset);

  /**
   * Adds a block at its appropriate place in the used_blocks list
   * Returns true if successful, false otherwise
   */
  bool sa_make_used (AllocNode *block);

protected:
  /// take a step forward in time
  errorcode do_time_step( real t1, real t2 );

  /**
   * Take state values( position, velocity, orientation, ... ) from
   * this world's entities and put them into the array.
   */
  void load_state ( real *state_array );

  /**
   * Take state values from array and reintegrate them into ctEntity
   * structures of this world.  The reverse operation of load_state.
   * State_array must have been filled out by load_state.
   */
  void reintegrate_state ( const real *state_array );

  /// Put first derivative of all world entities into state
  void load_delta_state ( real *state_array );

  void init_state ();

  void collide ();

  void resize_state_vector ( long new_size );

  /// The current state of the world, as in finite state machine state
  worldstate fsm_state;
  real rewound_from;

  ctLinkList<ctEntity> body_list;
  ctLinkList<ctForce> enviro_force_list;
  ctLinkList<ctCatastropheManager> catastrophe_list;

  /// Would an equation by any other name smell as sweet?
  OdeSolver *ode_to_math;

  /// State vectors for ODE interface

  /// Used to save state so it can be rewound to pre-ODE state
  real *y_save;
  real *y0;
  real *y1;

  /// !me should be a per "group" member ( when groups are implemented )
  /// indicate if there was a catstrophe last frame
  bool was_catastrophe_last_frame;

  /// Maximum number of times to back-up in an evolve loop.
  long max_time_subdivisions;

  long y_save_size;
  long max_state_size;
};


#endif
