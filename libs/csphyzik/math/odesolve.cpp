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

#include <stdio.h>
#include "cssysdef.h"
#include "csphyzik/odesolve.h"

static const real ONE_OVER_SIX = 1.0/6.0;
static const real ONE_OVER_THREE = 1.0/3.0;

OdeSolver::OdeSolver ()
{
  state_size = 0;

  dy = NULL;
  Iy = NULL;
}


// clean up
OdeSolver::~OdeSolver ()
{
  if (dy) delete [] dy;
  if (Iy) delete [] Iy;
}


// reallocate memory for all work variables
void OdeSolver::ode_realloc (int new_size)
{
  if (dy) delete [] dy;
  if (Iy) delete [] Iy;

  state_size = new_size;

  dy = new real[state_size];
  Iy = new real[state_size];
  assert (dy);
  assert (Iy);
}


//*****************************
//*   Runga-Kutta
//*****************************

OdeRungaKutta4::OdeRungaKutta4 ()
{
  k1 = NULL;
  k2 = NULL;
  k3 = NULL;
  k4 = NULL;

  ode_realloc (ODE_INITIAL_STATE_SIZE);
}

// clean up
OdeRungaKutta4::~OdeRungaKutta4 ()
{
  if (k1) delete [] k1;
  if (k2) delete [] k2;
  if (k3) delete [] k3;
  if (k4) delete [] k4;
}

// Compute a step. Current state in t0, state after time step t0 -> t1 is put in y1
// fourth order runga-kutta
void OdeRungaKutta4::calc_step (real y0[], real y1[], 
			   unsigned int len, real t0, real t1, dydt_function dydt)
{
  unsigned int i;
  // reallocate if necessary
  if (len > state_size)
    ode_realloc(len);

  real h = t1 - t0;
  real h_2 = h * 0.5;

  dydt(t0, y0, dy );
  for (i = 0; i < len; i++) 
    k1[i] = h*dy[i];  // first iteration

  for (i = 0; i < len; i++)
    Iy[i] = y0[i] + k1[i] * 0.5;

  dydt(t0 + h_2, Iy, dy);
  for (i = 0; i < len; i++)
    k2[i] = h*dy[i];  // second iteration

  for (i = 0; i < len; i++)
    Iy[i] = y0[i] + k2[i] * 0.5;

  dydt(t0 + h_2, Iy, dy );
  for (i = 0; i < len; i++)
    k3[i] = h*dy[i]; // third iteration

  for (i = 0; i < len; i++)
    Iy[i] = y0[i] + k3[i];

  dydt(t0 + h, Iy, dy );
  for (i = 0; i < len; i++)   
    k4[i] = h*dy[i]; // fourth iteration

  for (i = 0; i < len; i++)
    y1[i] = y0[i] + (k1[i] * ONE_OVER_SIX) + 
                    (k2[i] * ONE_OVER_THREE) + 
                    (k3[i] * ONE_OVER_THREE) + 
                    (k4[i] * ONE_OVER_SIX);
 
}

// reallocate memory for all work variables
void OdeRungaKutta4::ode_realloc(int new_size)
{
  OdeSolver::ode_realloc( new_size );
  if (k1)   delete [] k1;
  if (k2)   delete [] k2;
  if (k3)   delete [] k3;
  if (k4)   delete [] k4;

  state_size = new_size;

  k1 = new real[state_size];
  k2 = new real[state_size];
  k3 = new real[state_size];
  k4 = new real[state_size];
  assert(k1); assert(k2); assert(k3); assert(k4);
}


// euler integration.  Real dumb. Real fast.
OdeEuler::OdeEuler()
{
  ode_realloc(ODE_INITIAL_STATE_SIZE);
}

void OdeEuler::calc_step (real y0[], real y1[], 
			  unsigned int len, real t0, real t1, dydt_function dydt)
{
  unsigned int i;
  // reallocate if necessary
  if (len > state_size)
    ode_realloc(len);

  real h = t1 - t0;

  dydt(t0, y0, dy );
 
  for (i = 0; i < len; i++)
    y1[i] = y0[i] + dy[i] * h;
}

// mid-point

// mid-point integration.  Not so dumb. pretty fast.
OdeMidPoint::OdeMidPoint()
{
  ode_realloc(ODE_INITIAL_STATE_SIZE);
}

// k1 = f( t, x )  : eval F and T at (t, x)
// k2 = f( t + dt/2, x + dt/2*k1 ) 
//    x + dt/2*k1 -> take step of size dt/2 from x. AddRB( k2, k1, dt/2,  
//      f()         -> eval F and T at stepped ( t + dt/2, x + dt/2*k1 ) postion  
// y' = y + dt*k2   -> take step of size dt from x
void OdeMidPoint::calc_step (real y0[], real y1[], 
			   unsigned int len, real t0, real t1, dydt_function dydt)
{
  unsigned int i;
  // reallocate if necessary
  if (len > state_size)
    ode_realloc(len);

  real h = (t1 - t0) * 0.5;

  dydt(t0, y0, dy );
  for (i = 0; i < len; i++)
    Iy[i] = y0[i] + dy[i] * h;  // first iteration

  dydt(t0 + h, Iy, dy );
  
  h *= 2.0;
  for (i = 0; i < len; i++)
    y1[i] = y0[i] + dy[i] * h;
}
