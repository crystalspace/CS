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

#ifndef __CT_SOLVER_H__
#define __CT_SOLVER_H__

#include "csphyzik/phyztype.h"
#include "csphyzik/ctvector.h"

//!me need to add constructors and init vectors to 0

class ctDynamicEntity;

// abstract classes

/**
 * parent of classes that implement different algorithms and methods
 * to solve physical motion problems
 * responsible for calculating the change in state wrt time
 * uses forces list and current state to do this.
 */
class ctSolver
{
public:
  virtual void solve ( real t ) = 0;
  virtual void init () {};

};


/// calc accel of a simple body affected by simple forces and torques.
class ctSimpleDynamicsSolver : public ctSolver
{
public:
  ctSimpleDynamicsSolver ( ctDynamicEntity &pde )
    : de( pde ) {};
  virtual void solve ( real t );

protected:
  /// body this solver acts on
  ctDynamicEntity &de;

};

/// parent of solver for articulated bodies
class ctArticulatedSolver : public ctSolver
{
public:
  /// relative to it's body frame
  virtual ctVector3 get_linear_a () = 0;
  virtual ctVector3 get_angular_a () = 0;
  virtual void apply_impulse ( ctVector3 impulse_point,
			      ctVector3 impulse_vector ) = 0;

  virtual void get_impulse_m_and_I_inv ( real *pm, ctMatrix3 *pI_inv,
					 const ctVector3 &impulse_point,
			      const ctVector3 &unit_length_impulse_vector ) = 0;

};


/// tries to reach a goal by applying it's own forces and torques
class ctGoalSolver : public ctSolver
{
public:
  void set_goal ( const ctVector3 &pgoal ){ goal = pgoal; }

protected:
  ctVector3 goal;

};

/// IK or Inverse dyanmics parents
class ctArticulatedGoalSolver : public ctArticulatedSolver
{
public:
  void set_goal ( const ctVector3 &pgoal ){ goal = pgoal; }

protected:
  ctVector3 goal;
};

#endif // __CT_SOLVER_H__
