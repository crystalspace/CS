/*
    Dynamics/Kinematics modeling and simulation library.
    Copyright (C) 1999 by Michael Alexander Ewert and Noah Gibbs

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

#ifndef __CT_PTMASS_H__
#define __CT_PTMASS_H__

#include "csphyzik/point.h"
#include "csphyzik/entity.h"

// This is an Aristotelean (i.e. F=mv) point.
class ctDampedPointMass : public ctPointObj, public ctEntity
{

 public:
  void set_mass (int newmass)
  { m = newmass; }

  real mass (void)
  { return m; }

  /// ctPointObj methods
  ctVector3 pos ()
  { return x; }

  ctVector3 vel ()
  { return v; }

  void apply_force (ctVector3 force)
  { v += force / m; }

  /// ctEntity methods
  void init_state ()
  { v = ctVector3(0,0,0); }

  int get_state_size ()
  { return 3; }

  int set_state (real *sa)
  {
    printf ("Set_state!\n");
    sa[0] = x[0]; sa[1] = x[1]; sa[2] = x[2];
    return get_state_size ();
  }

  int get_state (real *sa)
  {
    printf ("Get_state!\n");
    x[0] = sa[0]; x[1] = sa[1]; x[2] = sa[2];
    return get_state_size ();
  }
  int set_delta_state (real *sa)
  {
    printf ("Set_delta_state!\n");
    sa[0] = v[0]; sa[1] = v[1]; sa[2] = v[2];
    return get_state_size ();
  }

protected:
  real m;
  ctVector3 x;

  // Temp vars for dealing with derivatives & solver
  ctVector3 v;
};

// This is a Newtonian (i.e. F=ma) point.
class ctPointMass : public ctPointObj, public ctEntity
{
 public:
  ctPointMass (real mass)
    : m(mass), x(ctVector3(0.0, 0.0, 0.0)), v(ctVector3(0.0, 0.0, 0.0))  {}

  ctPointMass (ctVector3 pos = ctVector3(0.0, 0.0, 0.0),
	       ctVector3 vel = ctVector3(0.0, 0.0, 0.0), real mass = 1.0)
    : m(mass), x(pos), v(vel) {}

  ~ctPointMass() {}

  real mass (void)
  { return m; }

  void set_mass (int newmass)
  { m = newmass; }

  /// ctPointObj methods
  ctVector3 pos ()
  { return x; }

  ctVector3 vel ()
  { return v; }

  void apply_force (ctVector3 force)
  { F += force / m; }

  void set_pos (ctVector3 pos)
  { x = pos; }

  void set_vel (ctVector3 vel)
  { v = vel; }

  void set_force (ctVector3 force)
  { F = force; }

  /// ctEntity methods
  void init_state ()
  { F[0] = F[1] = F[2] = 0.0; }

  int  get_state_size ()
  { return 6; }

  int  set_state(real *sa)
  {
    sa[0] = x[0]; sa[1] = x[1]; sa[2] = x[2];
    sa[3] = v[0]; sa[4] = v[1]; sa[5] = v[2];
    return get_state_size();
  }

  int  get_state(const real *sa)
  {
    x[0] = sa[0]; x[1] = sa[1]; x[2] = sa[2];
    v[0] = sa[3]; v[1] = sa[4]; v[2] = sa[5];
    return get_state_size();
  }

  int set_delta_state(real *sa)
  {
    sa[0] = v[0]; sa[1] = v[1]; sa[2] = v[2];
    sa[3] = F[0]; sa[4] = F[1]; sa[5] = F[2];
    return get_state_size();
  }

protected:
  real      m;
  ctVector3 x;
  ctVector3 v;

  // Temp vars for dealing with derivatives & solver
  ctVector3 F;
};

#endif // __CT_PTMASS_H__
