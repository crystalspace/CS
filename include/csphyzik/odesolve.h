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
#ifndef __CTODESOLVER_H__
#define __CTODESOLVER_H__

// this code was adapted from an ode solver by Brian Mirtich

#include <assert.h>
#include "csphyzik/phyztype.h"

#define ODE_INITIAL_STATE_SIZE	1024


typedef void (*dydt_function)(real t, const real statev[],  real delta_statev[] );


// virtual base-class on which concrete ode solver algorithms are built
// NOTE: don't derive any classes from any classes derived from this one.
class OdeSolver
{
public:

  OdeSolver();
  virtual ~OdeSolver();

  /**
   * Compute a step.
   * Current state in y0, state after time step t0 -> t1 is put in y1
   */
  virtual void calc_step (real y0[], real y1[], unsigned int len,
			  real t0, real t1, dydt_function dydt) = 0;

protected:

  virtual void ode_realloc (int new_size);

  /// limit of ~65,000 right now
  unsigned int state_size;

  ///the derivatives
  real *dy;
  real *Iy;
};

/**
 * Runga-Kutta order 4
 * pretty standard dynamics ODE solver.  Good stability/speed trade-off
 * NOTE: don't derive any classes from this one
 */
class OdeRungaKutta4 : public OdeSolver
{
public:

  OdeRungaKutta4 ();
  virtual ~OdeRungaKutta4 ();

  void calc_step (real y0[], real y1[], unsigned int len,
		  real t0, real t1, dydt_function dydt);

private:

  void ode_realloc (int new_size);

  /// work variables
  real *k1;
  real *k2;
  real *k3;
  real *k4;
};

/**
 * the fastest and least stable ODE-solver.
 * not recomended unless you REALLY need the speed.
 * use mid-point instead if you need speed.
 */
class OdeEuler : public OdeSolver
{
public:

  OdeEuler ();

  void calc_step (real y0[], real y1[], unsigned int len,
		  real t0, real t1, dydt_function dydt);

};


/**
 * If you need speed and don't have many forces more complicated than gravity
 * or propulsion this is a good one to use.  Better than euler, but not as
 * stable as runga-kutta ( much faster though ).
 */

class OdeMidPoint : public OdeSolver
{
public:

  OdeMidPoint();

  void calc_step (real y0[], real y1[], unsigned int len,
		  real t0, real t1, dydt_function dydt);

};

#endif // __CTODESOLVER_H__
