#ifndef CTODESOLVER_H
#define CTODESOLVER_H

// this code was adapted from an ode solver by Brian Mirtich

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

	// compute a step.  current state in t0, state after time step t0 -> t1 is put in y1
	virtual void calc_step(real y0[], real y1[], unsigned int len, real t0, real t1, dydt_function dydt) = 0;
  
protected:

	virtual void ode_realloc(int new_size) = 0;
	unsigned int state_size;  // limit of ~65,000 right now
	real *dy;  //the derivatives
	real *Iy;
};


// Runga-Kutta
// NOTE: don't derive any classes from this one
class OdeRungaKutta4 : public OdeSolver
{
public:

	OdeRungaKutta4();
	virtual ~OdeRungaKutta4();

	void calc_step(real y0[], real y1[], unsigned int len, real t0, real t1, dydt_function dydt);
 
private:

	void ode_realloc(int new_size);

	// work variables
	real *k1;    
	real *k2;
	real *k3;
	real *k4;

};

#endif
