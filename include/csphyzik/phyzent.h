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

#ifndef __CT_PHYZENT_H__
#define __CT_PHYZENT_H__

#include "csphyzik/phyztype.h"
#include "csphyzik/linklist.h"
#include "csphyzik/refframe.h"
#include "csphyzik/entity.h"
class ctForce;

#define DEFAULT_ENTITY_MASS 10.0

#define PHYSICALENTITY_STATESIZE 12  // Rmatrix, x

class ctSolver;
class ctCollidingContact;

/// parent class of physical bodies
class ctPhysicalEntity : public ctEntity
{
public:
/*
**  Constructors/destructors/statics
*/
  /// Default refrence frame will be inertial frame
  ctPhysicalEntity ();
  /// Use specified reference frame
  ctPhysicalEntity( ctReferenceFrame &ref, ctDeltaReferenceFrame &dref );
  virtual ~ctPhysicalEntity();

/*
**  Member functions
*/
  //**********  ODE interface ****************
  /// Init values that will influence the calculation of the change in state.
  virtual void init_state ()
  {
    F[0] = 0; F[1] = 0; F[2] = 0;
    T[0] = 0; T[1] = 0; T[2] = 0;
  }
  /// Return the size of this entity's state vector
  virtual int get_state_size ()
  { return PHYSICALENTITY_STATESIZE; }

  /**
   * Add this body's state to the state vector buffer passed in.
   * Increment state buffer to point after added state.  Upload
   */
  virtual int set_state ( real *sa );

  /// Download state from buffer into this entity
  virtual int get_state( const real *sa );

  /// Add change in state vector over time to state buffer parameter.
  virtual int set_delta_state( real *state_array );

  /// change orientation in radians
  virtual void rotate_around_line( ctVector3 &paxis, real ptheta );
  /// Set position
  virtual void set_pos ( const ctVector3 &px )
  { RF.set_offset( px ); }
  /// virtual because v is calculated from P ( momentum ) in rigid bodies
  virtual void set_v ( const ctVector3 &pv );
  /// Get linear velocity
  virtual ctVector3 get_v ()
  { return dRF.v; }
  /// Set angular velocity
  virtual void set_angular_v ( const ctVector3 &pw );
  /// Get angular velocity
  virtual ctVector3 get_angular_v ()
  { return dRF.w; }
  /// Get total added force for frame
  ctVector3 get_F ()
  { return F; }
  /// Get total torque for frame
  ctVector3 get_torque ()
  { return T; }
  /// Set total force for frame
  void set_F( const ctVector3 &pF )
  { F = pF; }
  /// Set total force for frame
  void set_torque( const ctVector3 &pT )
  { T = pT; }
  //void add_force( ctForce *f ){ forces.add_link( f ); }
  /// Add additional force
  void sum_force ( const ctVector3 &f )
  { F += f; }
  /// Add additional torque
  void sum_torque( const ctVector3 &t ){ T += t; }
  //void remove_force( ctForce *f ){ forces.remove_link( f ); }

  /// Add this force to the list of forces that will be applied each frame
  virtual void apply_given_F( ctForce &frc );
  /// For debugging
  void print_force ()
  {/*cout << "F: " << F*F << "\n";*/ }

  /// collision routines
  virtual ctPhysicalEntity *get_collidable_entity ()
  { return this; }

  /**
   * Can use this to impart and impulse to this object.
   * Impulse_point is vector from center of body to point of collision in
   * world coordinates.  Impulse_vector is in world coords
   */
  virtual void apply_impulse ( ctVector3 impulse_point, ctVector3 impulse_vector );

  //  virtual real get_impulse_m(){ return DEFAULT_ENTITY_MASS; }
//  virtual ctMatrix3 get_impulse_I_inv() {
//    return ctMatrix3( 1.0/get_impulse_m() );
//  }

  /**
   * Fill out mass and inverse inertia tensor behaviour for an impulse response
   * impulse_point is point of collision in world frame.
   */
  virtual void get_impulse_m_and_I_inv ( real *pm, ctMatrix3 *pI_inv,
       const ctVector3 &impulse_point, const ctVector3 &unit_length_impulse_vector )
  {
    (void)unit_length_impulse_vector;
    (void)impulse_point;
    *pm = DEFAULT_ENTITY_MASS;
    pI_inv->identity();
    *pI_inv *= 1.0/DEFAULT_ENTITY_MASS;
  }

  const ctMatrix3 &get_R ()              { return RF.get_R(); }
  const ctMatrix3 &get_T ()              { return RF.get_T(); }
  const ctVector3 &get_pos ()            { return RF.get_offset(); }
  const ctVector3 &get_org_world ()      { return RF.get_offset(); }
  const ctMatrix3 &get_world_to_this ()  { return RF.get_parent_to_this(); }
  const ctMatrix3 &get_this_to_world ()  { return RF.get_this_to_parent(); }
  void v_this_to_world ( ctVector3 &pv ) { RF.this_to_world( pv ); }

  ctVector3 get_v_this_to_world ( ctVector3 &pv )
  { ctVector3 pret = pv;  RF.this_to_world( pret ); return pret; }

  ctReferenceFrame *get_RF()             { return &RF; }
  ctDeltaReferenceFrame *get_dRF()       { return &dRF; }

protected:

  /// Frame of reference. Orientation and position are stored here
  ctReferenceFrame &RF;
  /// Change of RF wrt time.  ( angular and linear velocity )
  ctDeltaReferenceFrame &dRF;
  /// Total added Force for a frame
  ctVector3 F;
  /// Total Torque for a frame
  ctVector3 T;
};

class ctSimpleDynamicsSolver;

// solved with ctSimpleDynamicsSolver
// just a body affected by simple forces
class ctDynamicEntity : public ctPhysicalEntity
{
public:
  friend class ctSimpleDynamicsSolver;

  ctDynamicEntity();
  ctDynamicEntity( ctReferenceFrame &ref, ctDeltaReferenceFrame &dref );
  virtual ~ctDynamicEntity();

  virtual void apply_given_F( ctForce &frc );

  virtual void set_m ( real pm );

  real get_m ()
  { return m; }
//  virtual real get_impulse_m () { return m; }

protected:
  /// mass
  real m;
};

#endif // __CT_PHYZENT_H__
