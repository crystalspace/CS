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

#ifndef __CT_JOINT_H__
#define __CT_JOINT_H__

#include "csphyzik/phyztype.h"
#include "csphyzik/math3d.h"
#include "csphyzik/ctvspat.h"

class ctArticulatedBody;
class ctForce;

/// joint velocity and acceleration
#define JOINT_STATESIZE 2 

/**
 * a joint that links together and constrains the motion of two articulated 
 * bodies inboard means articulated body that is closer to the root of the whole
 * outboard is the opposite
 */
class ctJoint
{
public:
  /// static values
  static double joint_friction;

  ctJoint () 
    : inboard_offset(0), outboard_offset(0), joint_axis(0)
  { q = qv = qa = 0; }
  
  ctJoint ( ctArticulatedBody *in, ctVector3 &in_offset, 
	    ctArticulatedBody *out, ctVector3 &out_offset );
	
  /// Add v and w (angular v) of the outboard link, contributed from joints state
  virtual void calc_vw( ctVector3 &v, ctVector3 &w ) = 0;
  /**
   * calc corioulous force generated by this joint.  Result goes in c
   * r is vector from center of inboard handle to center of outboard handle.
   * w_f is the angular velocity of inboard handle.
   */
  virtual void calc_coriolus ( const ctVector3 &r, const ctVector3 &w_f, 
			       ctSpatialVector &c ) = 0;

  virtual ctSpatialVector get_spatial_joint_axis() = 0;

  /**  
   * Internal force generated at joint.  can be used to simulate a robotic
   * motor or muscle force on a limb.
   */
  virtual real get_actuator_magnitude( real external_f, real inertail_comp );

  virtual int get_state_size(){ return JOINT_STATESIZE; }// = 0;
  virtual int set_state( real *sa );
  virtual int get_state( const real *sa );
  virtual int set_delta_state( real *state_array ); 

  /// Update the body_to_world reference frame of the outboard handle 
  void update_link_RF ();
	
  /// return vector from inboard center to outboard center in outboard coords
  ctVector3 get_r ();

  ctArticulatedBody *inboard;
  /**
   * Offset from center of inboard link to this joint ( at end of inboard link )
   * in inboard frame.
   */
  ctVector3 inboard_offset;
  ctArticulatedBody *outboard;

  /**
   * Points from joint axis to origin of outboard entity in outboard frame
   * (d in Mirtch thesis)
   */
  ctVector3 outboard_offset;  
	
  /// Valid in both in and out frame of reference
  ctVector3 joint_axis;
  /// joint position, velocity and acceleration in radians
  real q, qv, qa;  

protected:
  //!me go through and hide all data that should be hidden


};

/// joint that slides in and out
///!me needs some work.  Needs proper constructor
class ctPrismaticJoint : public ctJoint
{
public:
  virtual void calc_vw ( ctVector3 &v, ctVector3& /*w*/ )
  { v = v + joint_axis *qv; }

  virtual void calc_coriolus ( const ctVector3 &r, const ctVector3 &w_f, 
			      ctSpatialVector &c );

  virtual ctSpatialVector get_spatial_joint_axis ();

};

/// a hinge
class ctRevoluteJoint : public ctJoint
{
public:
  ctRevoluteJoint ( ctArticulatedBody *in, ctVector3 &in_offset, 
		    ctArticulatedBody *out, ctVector3 &out_offset, 
		    ctVector3 &paxis );

  virtual void calc_vw ( ctVector3 &v, ctVector3 &w )
  { 
    w = w + joint_axis*qv;
    v = v + ( joint_axis % outboard_offset )*qv;
  }

  virtual void calc_coriolus ( const ctVector3 &r, const ctVector3 &w_f, 
			       ctSpatialVector &c );

  virtual ctSpatialVector get_spatial_joint_axis ();
};

/**
 * a hinge with a joint angle "constraint" enforced by PID controller techniques.
 * adjust spring and damping constants on a per joint basis to get it behaving 
 * right.  e.g. if your joint is bouncing back at the constrained angle, 
 * then reduce spring_constant.
 */
class ctConstrainedRevoluteJoint : public ctRevoluteJoint
{
public:
  ctConstrainedRevoluteJoint ( ctArticulatedBody *in, ctVector3 &in_offset, 
			       ctArticulatedBody *out, ctVector3 &out_offset, 
			       ctVector3 &paxis );

  /**
   * internal force generated at joint.  can be used to simulate a robotic
   * motor or muscle force on a limb.
   */
  virtual real get_actuator_magnitude ( real external_f, real inertail_comp );

  void set_constraint ( real pmax_angle, real pmin_angle )
  { max_angle = pmax_angle; min_angle = pmin_angle; }

  void set_spring_constant( real pk )
  { k = pk; }

  void set_damping_constant( real pk )
  { damping_k = pk; }

  real max_angle;
  real min_angle;

  ///!me tweak these for better stability, depending on your situation.

  /// Spring konstant
  real k;  
  /// Damping konstant.  
  real damping_k; 
};

#endif // __CT_JOINT_H__
