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

#ifndef QUATRIGIDBODY_H
#define QUATRIGIDBODY_H

#include "csphyzik/phyztype.h"
#include "csphyzik/entity.h"
#include "csphyzik/ctquat.h"
#include "csgeom/quaterni.h"

#define QUATRIGID_STATE_SIZE 13

class ctQuatRigidBody : public ctEntity {
 protected:
  real         mass;
  ctVector3    pos;    // Spatial position of center of mass
  ctVector3    mom;    // Linear momentum, P, equals v * mass
  ctQuaternion quat;   // Angular orientation
  ctVector3    ang;    // Angular momentum

  ctVector3    F;      // Force on center of mass
  ctVector3    T;      // Torque

  ctMatrix3    Ibody;  // Inertia tensor

  // Additional derived quantities:
  bool         precalculated;
  ctMatrix3    Ibodyinv;  // Ibody^-1
  ctMatrix3    R;
  ctMatrix3    Iinv;      // R * Ibodyinv * transpose(R)
  ctVector3    omega;     // Ibodyinv * angular_momentum

 public:
  // Constructor, Destructor
  ctQuatRigidBody(ctVector3 x = ctVector3(0,0,0), real M = 0);
  ~ctQuatRigidBody() {}

  // Getters & Setters (including derived)
  real get_mass() { return mass; }
  void set_mass(real m) { mass = m;  precalculated = false; }

  ctVector3 get_center() { return pos; }
  void      set_center(ctVector3 x) { pos = x; }

  ctVector3 get_linear_velocity() { return mom * (1.0 / mass); }
  void      set_linear_velocity(ctVector3 vel) { mom = vel * mass; }

  ctQuaternion get_orientation() { return quat; }
  void         set_orientation(ctQuaternion ori) { 
    quat = ori;
    precalculated = false;
  }

  ctMatrix3    get_Ibody(void) { return Ibody; }
  void         set_Ibody(ctMatrix3& Ib) {
    Ibody = Ib;
    Ibodyinv = Ibody.inverse();
    precalculated = false;
  }

  ctMatrix3    get_Ibodyinv(void) { return Ibodyinv; }

  ctVector3    get_omega() { return get_angular_velocity(); }
  ctVector3    get_angular_velocity() {
    Precalculate();
    return omega;
  }

  // Workhorse functions
  void init_state() {
  }

  int get_state_size() { return QUATRIGID_STATE_SIZE; }

  int get_state(real *sa);

  int set_state(real *sa);
  int set_delta_state(real *sa);

 protected:
  // Protected internal utility functions
  void Precalculate();
};

#endif // QUATRIGIDBODY_H
