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

#ifndef __CT_ENTITY_H__
#define __CT_ENTITY_H__

#include <stdarg.h>

#include "csphyzik/phyztype.h"
#include "csphyzik/force.h"
#include "csphyzik/linklist.h"
#include "csphyzik/refframe.h"

class ctSolver;
class ctPhysicalEntity;


// flags
#define CTF_NOREWIND 0x1

// parent class of all physical bodies ( or even some non-physical ones... )
class ctEntity
{
protected:
  int state_offset;

  /**
   * object responsible for calculating the change in state wrt time
   * uses forces list and current state to do this.
   */
  ctSolver *solver;

  /// list of all forces affecting this object
  ctLinkList<ctForce> forces;

 public:
/*
**  Constructors/destructors/statics
*/
  ctEntity();
  virtual ~ctEntity();

/*
**  Member functions
*/
  /**
   * Compute the derivative of this body's state vector ( position,
   * velocity... ) from applied forces.  Just passes control to the
   * ctSolver for this object.
   */
  virtual void solve ( real t );

  /// Set the class responsible for solving change in state wrt time.
  void set_solver ( ctSolver *pslv )
  { solver = pslv; }

  //**********  ODE interface ****************
  // Init values that will influence the calculation of the change in state.
  // These are normally forces, torques, etc.
  virtual void init_state() {}
  // Return the size of this enities state vector
  virtual int get_state_size() { return 0; }
  // Add this body's state to the state vector buffer passed in.
  virtual int set_state( real *sa ) = 0;
  // download state from buffer into this entity
  virtual int get_state( const real *sa ) = 0;
  // add change in state vector over time to state buffer parameter.
  virtual int set_delta_state( real *state_array ) = 0;

  int  get_state_offset() { return state_offset; }
  void set_state_offset(int of) { state_offset = of; }

  void add_force( ctForce *f ){ forces.add_link( f ); }
  void remove_force( ctForce *f ){ forces.remove_link( f ); }

  // add this force to the list of forces that will be applied each frame
  virtual void apply_given_F( ctForce &frc );
  void print_force() {}

  // collision routines.  return something we can collide with if possible
  virtual ctPhysicalEntity *get_collidable_entity(){ return NULL; }

  // Stereotype of funcs to replace flags array
  virtual bool will_rewind () { return !(flags & CTF_NOREWIND); }
  virtual void set_rewind (bool dorewind)
  {
    flags &= ~CTF_NOREWIND;
    if(!dorewind) flags |= CTF_NOREWIND;
  }

  /// flags
  unsigned long flags;
};

#endif // __CT_ENTITY_H__
