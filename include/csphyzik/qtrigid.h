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

#ifndef __CT_QUATRIGIDBODY_H__
#define __CT_QUATRIGIDBODY_H__

#include "csphyzik/phyztype.h"
#include "csphyzik/entity.h"
#include "csphyzik/ctquat.h"
#include "csgeom/quaterni.h"

#define QUATRIGID_STATE_SIZE 13

class ctQuatRigidBodyConnector;

class ctQuatRigidBody : public ctEntity
{
 public:
  /// Constructor, Destructor
  ctQuatRigidBody (ctVector3 x = ctVector3(0,0,0), real M = 0);
  ~ctQuatRigidBody () {}

  /// Getters & Setters (including derived)
  real get_mass ()
  { return mass; }

  void set_mass (real m)
  { mass = m;  precalculated = false; }

  ctVector3 get_center ()
  { return pos; }

  void set_center (ctVector3 x)
  { pos = x; }

  ctVector3 get_linear_velocity ()
  { return mom / mass; }

  void set_linear_velocity (ctVector3 vel)
  { mom = vel * mass; }

  ctVector3 get_linear_momentum (void)
  { return mom; }

  void set_linear_momentum (ctVector3 newmom)
  { mom = newmom; }

  ctQuaternion get_orientation ()
  { return quat; }

  void set_orientation (ctQuaternion ori)
  {
    quat = ori;
    quat.Normalize();
    precalculated = false;
  }

  ctVector3 get_rotation()
  {
    real angle = 2.0 * acos(quat.r);
    real fact = angle / sin(angle);
    return ctVector3(quat.x * fact, quat.y * fact, quat.z*fact);
  }

  void set_rotation (ctVector3 rot)
  {
    real len = rot.Norm ();
    if (len <= 3*MIN_REAL)
      set_orientation(ctQuaternion(1.0, 0.0, 0.0, 0.0));
    else
    {
      rot /= len;
      real sine = sin(len);
      set_orientation(ctQuaternion(cos(len / 2), sine*rot[0], sine*rot[1],
				   sine*rot[2]));
    }
  }

  ctMatrix3 get_R ()
  {
    Precalculate();
    return R;
  }

  void set_R (ctMatrix3& new_R)
  {
    quat.from_matrix(new_R);
    precalculated = false;
  }

  ctMatrix3 get_Ibody (void)
  { return Ibody; }

  void set_Ibody (const ctMatrix3& Ib)
  {
    Ibody = Ib;
    Ibodyinv = Ibody.inverse();
    precalculated = false;
  }

  ctMatrix3 get_Ibodyinv (void)
  { return Ibodyinv; }

  ctMatrix3 get_Iinv (void)
  {
    Precalculate();
    return Iinv;
  }

  ctVector3  get_angular_velocity ()
  {
    Precalculate();
    return omega;
  }

  void set_angular_velocity (ctVector3 new_omega)
  {
    // Omega = Iinv * ang_mom
    // Iinv * ang_mom = new_omega
    // ang_mom = I * new_omega
    // I = R * Ibody * R^T
    Precalculate ();
    ctMatrix3 I(R * Ibody * R.get_transpose());
    ang = I * new_omega;
    precalculated = false;
  }

  // Workhorse functions
  void init_state ()
  {
    F[0] = F[1] = F[2] = 0.0;
    T[0] = T[1] = T[2] = 0.0;
  }

  int get_state_size ()
  { return QUATRIGID_STATE_SIZE; }

  int get_state (const real *sa);
  int set_state (real *sa);
  int set_delta_state (real *sa);

  void apply_F (ctVector3 newF)
  { F += newF; }

  void apply_T (ctVector3 newT)
  { T += newT; }

  // Connector method
  ctQuatRigidBodyConnector *new_connector (ctVector3 offs);

  // Shorthand methods, just call other methods
  inline ctVector3 get_velocity ()
  { return get_linear_velocity(); }

  inline void set_velocity (ctVector3 vel)
  { set_linear_velocity(vel); }

  inline ctVector3 get_omega ()
  { return get_angular_velocity(); }

  inline void set_omega (ctVector3 new_omega)
  { set_angular_velocity(new_omega); }

 protected:
  /// Protected internal utility functions
  void Precalculate ();

  real         mass;
  /// Spatial position of center of mass
  ctVector3    pos;
  /// Linear momentum, P, equals v * mass
  ctVector3    mom;
  /// Angular orientation
  ctQuaternion quat;
  /// Angular momentum
  ctVector3    ang;

  /// Force on center of mass
  ctVector3    F;
  /// Torque
  ctVector3    T;
  /// Inertia tensor
  ctMatrix3    Ibody;

  /// Additional derived quantities:
  bool         precalculated;
  /// Ibody^-1
  ctMatrix3    Ibodyinv;
  ctMatrix3    R;
  /// R * Ibodyinv * transpose(R)
  ctMatrix3    Iinv;
  /// Ibodyinv * angular_momentum
  ctVector3    omega;

};

#endif // __CT_QUATRIGIDBODY_H__
