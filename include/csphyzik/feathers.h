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

#ifndef CT_FEATHERSTONE_ALGORITHM
#define CT_FEATHERSTONE_ALGORITHM

//!me T = 2*pi*sqrt( length/g )
//!me period of a meter long rod should be 1.64s under g=9.81 and 1 world unit = 1 m
//!me actual period observed is ~1.70s  could be acceptable numerical error.. or not..

#include "csphyzik/solver.h"
#include "csphyzik/math3d.h"
#include "csphyzik/ctvspat.h"
#include "csphyzik/ctmspat.h"

class ctArticulatedBody;

// determine the motion of an articulated body acting under the influence of
// externally ( e.g. gravity ) applied forces and internally ( e.g. robot motor )
// applied forces
// Only works for tree-like topologies
class ctFeatherstoneAlgorithm : public ctArticulatedSolver
{
public:
  ctFeatherstoneAlgorithm ( ctArticulatedBody &pab )
    : ab( pab ){ sIsQsZIc_computed = false; };

  /// do it!  Apply algorithm to solve for motions
  virtual void solve ( real t );

  //	void init();

  /**
   * The algorithm result used by ctArticulatedBody::set_delta_state
   * this values are relative to it's body frame
   */
  ctVector3 get_linear_a()
  { ctVector3 aret( a[3], a[4], a[5] ); return aret; }

  ctVector3 get_angular_a()
  { ctVector3 aret( a[0], a[1], a[2] ); return aret; }

  /**
   * can use this to impart and impulse to this object.
   * impulse_point is vector from center of body to point of collision in
   * world coordinates.  impulse_vector is in world coords
   */
  void apply_impulse( ctVector3 impulse_point, ctVector3 impulse_vector );

  /**
   * Calculate virtual mass and behaviour for an impulse applied at a point
   * impulse_point is point of collision in world frame.
   */
  void get_impulse_m_and_I_inv ( real *pm, ctMatrix3 *pI_inv,
    const ctVector3 &impulse_point, const ctVector3 &unit_length_impulse_vector );

protected:

  void fsolve_grounded ( real t );
  void fsolve_floating ( real t );

  void init_link ();

  void compute_Ia_Za ();

  void compute_joint_a ();

  void impulse_to_v ();

  void test_impulse_response ();

  void propagate_impulse ();

  void zero_Ja_help ();

  void zero_Ja ();

  /// The articulated body we are solving motions for
  ctArticulatedBody &ab;

  // work variables

  ctSpatialVector a;

  /// spatial inertia matrix
  ctSpatialMatrix Ia;

  /**
   * bias vector or zero acceleration vector
   *  [ -m_i g_i - F_externaly_applied            ]   !note! negated!
   *  [ w_i x I_i w_i - Torque_externally_applied ]
   * some other symbol in featherstone's book ( P? )
   */
  ctSpatialVector Za;

  /// spatial coriolus force
  ctSpatialVector c;

  /// spatial tranform from link i-1 to i
  ctSpatialMatrix gXf;


  /// compute once per algo
  real sIs;
  real QsZIc;
  bool sIsQsZIc_computed;

  /// used for calculating impulse response
  ctSpatialVector Ja;
  ctSpatialVector dv;

private:
  // some work variables that I don't want piled on the stack during recursion
  // should convert to iteration actually for better performance
  ctMatrix3 Mwork;
  ctSpatialMatrix sMwork;
  ctSpatialVector ZaIac;
};

#endif
