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

#ifndef __CT_RIGIDBODY_H__
#define __CT_RIGIDBODY_H__

#include "csphyzik/phyzent.h"
#include "csphyzik/phyztype.h"

#define RBSTATESIZE (6 + PHYSICALENTITY_STATESIZE)
struct iObjectRegistry;

/// a rigid body.  has inertia tensor.
class ctRigidBody : public ctDynamicEntity
{
public:
/*
**  Constructors/destructors/statics
*/
  ctRigidBody();
  ctRigidBody( ctReferenceFrame &ref, ctDeltaReferenceFrame &dref );
  //	ctRigidBody( ctReferenceFrame &ref,  );
  virtual ~ctRigidBody();

  /// Convienent to create enitites with parent as world frame of reference
  static ctRigidBody *new_ctRigidBody ();
  ///
  static ctRigidBody *new_ctRigidBody ( coord x, coord y, coord z );
  /// Get Rigid Body state size.
  static int get_RB_state_size ()
  { return RBSTATESIZE; }


  iObjectRegistry *object_reg;

/*
**  Member functions
*/
  /**
   * Calculate interia tensor as if body was a solid block of given dimensions.
   * NOTE: m must be given first.
   */
  void calc_simple_I_tensor( real width, real height, real depth );

  /// Get Inertia tensor
  const ctMatrix3 &get_I ()
  { return I; }

  /// Get Inverse Inertia tensor
  const ctMatrix3 &get_I_inv ()
  { return I_inv; }

/*
  ctMatrix3 get_I_inv_world(){
    const ctMatrix3 &R = RF.get_R();
    ctMatrix3 I_inv_world = R * I_inv * (R.get_transpose());
    return I_inv_world;
  }

  ctMatrix3 get_I_world(){
    const ctMatrix3 &R = RF.get_R();
    ctMatrix3 I_world = R * I * (R.get_transpose());
    return I_world;
  }
*/

  ctMatrix3 get_I_inv_world ()
  {
    const ctMatrix3 &R = RF.get_R();
    ctMatrix3 I_inv_world;
    R.similarity_transform( I_inv_world, I_inv );
    return I_inv_world;
  }

  ctMatrix3 get_I_world ()
  {
    const ctMatrix3 &R = RF.get_R();
    ctMatrix3 I_world;
    R.similarity_transform( I_world, I );
    return I_world;
  }

/*
  virtual ctMatrix3 get_impulse_I_inv(){
//    return get_I_inv_world();
    const ctMatrix3 &R = RF.get_R();
    ctMatrix3 Mret;
    R.similarity_transform( Mret, I_inv );
    return Mret;
  }
*/

  virtual void get_impulse_m_and_I_inv ( real *pm, ctMatrix3 *pI_inv,
       const ctVector3 &impulse_point, const ctVector3 &unit_length_impulse_vector )
  {
    (void)unit_length_impulse_vector;
    (void)impulse_point;
    *pm = m;
    const ctMatrix3 &R = RF.get_R();
    R.similarity_transform( *pI_inv, I_inv );
  }

  /// Get Angular Momentum
  ctVector3 get_angular_P ()
  { return L; }

  ctVector3 get_P ()
  { return P; }

  /// ODE interface
  virtual int get_state_size () { return RBSTATESIZE; }
  virtual int set_state ( real *state_array );
  virtual int get_state ( const real *state_array );
  virtual int set_delta_state ( real *state_array );
  virtual void set_angular_v ( const ctVector3 &pw );
  virtual void set_v ( const ctVector3 &pv );
  virtual void add_angular_v ( const ctVector3 &pw );
  virtual void add_v ( const ctVector3 &pv );
  virtual void set_m ( real pm );

  /**
   * Impulse_point is vector from center of body to point of collision in
   * world coordinates.  impulse_vector is in world coords.
   */
  virtual void apply_impulse( ctVector3 impulse_point, ctVector3 impulse_vector );

protected:
  /// Momentum
  ctVector3 P;
  /// Angular momentum
  ctVector3 L;
  /// Intertia tensor
  ctMatrix3 I;
  /// Inverse of Inertia tensor
  ctMatrix3 I_inv;

};

#endif // __CT_RIGIDBODY_H__
