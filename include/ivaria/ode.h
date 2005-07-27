/*
    Copyright (C) 2003 by Jorrit Tyberghein, Daniel Duhprey,
    Leandro Motta Barros

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

#ifndef __CS_IVARIA_ODE_H__
#define __CS_IVARIA_ODE_H__

#include "csutil/scf.h"

#define int8 ode_int8
#define uint8 ode_uint8
#define int32 ode_int32
#define uint32 ode_uint32
#include <ode/ode.h>
#undef uint32
#undef int32
#undef uint8
#undef int8

SCF_VERSION (iODEFrameUpdateCallback, 0, 0, 1);

/**
 * This class can be passed in as a callback during the physics update
 * it is only called if FrameRate is enabled.
 */

struct iODEFrameUpdateCallback : public iBase
{
  /// Executes the per update callback
  virtual void Execute (float stepsize) = 0;
};

SCF_VERSION (iODEDynamicState, 0, 0, 1);

/**
 * This class exposes parameters specific to odedynam as an implementation
 * of iDynamics
 */
struct iODEDynamicState : public iBase
{

  /// Sets ODE's Error Resolution Parameter (see ode docs for details)
  virtual void SetGlobalERP (float erp) = 0;
  virtual float GlobalERP () = 0;

  /// Sets ODE's Constraint Force Mixing (see ode docs for details)
  virtual void SetGlobalCFM (float cfm) = 0;
  virtual float GlobalCFM () = 0;

  /// Enables the experimental StepFast code in ode
  virtual void EnableStepFast (bool enable) = 0;
  virtual bool StepFastEnabled () = 0;
  virtual void SetStepFastIterations (int iter) = 0;
  virtual int StepFastIterations () = 0;

  virtual void EnableQuickStep (bool enable) = 0;
  virtual bool QuickStepEnabled () = 0;
  virtual void SetQuickStepIterations (int iter) = 0;
  virtual int QuickStepIterations () = 0;

  /**
   * The following code enables a constant framerate on processing
   * this means if you set the frame rate to (default) 50  The stepsize
   * passed into Step is treated as the elapsed time in seconds received
   * from the virtual clock GetElapsedTicks.
   * The physics will iterate a number of steps at 1/50th
   * of a second until enough time has passed to account for the time
   * Beware the default setting for frame limit is 10, which means
   * if the stepsize passed to Step is longer than 1/10 a second
   * the physics will stop iterating and slow down.  Never set this
   * parameter to 0 or else you could incur cycle of death where
   * the number of physics steps increases the amount of elapsed
   * time between frames which increases the number of physics steps
   * toward infinity
   */
  virtual void EnableFrameRate (bool enable) = 0;
  virtual bool FrameRateEnabled () = 0;

  virtual void SetFrameRate (float hz) = 0;
  virtual float FrameRate () = 0;

  virtual void SetFrameLimit (float hz) = 0;
  virtual float FrameLimit () = 0;

  virtual void AddFrameUpdateCallback (iODEFrameUpdateCallback *cb) = 0;
  virtual void RemoveFrameUpdateCallback (iODEFrameUpdateCallback *cb) = 0;

  /**
   * This makes updates happen on the cscmdPreProcess
   * and invalidates calls to Step()
   * This should be used in conjuction with the FrameRate calls
   */
  virtual void EnableEventProcessing (bool enable) = 0;
  virtual bool EventProcessingEnabled () = 0;

  /**
   * The following enables special robustness checks for fast
   * moving objects to determine if they will tunneling and
   * adjusts the physics frame resolution (rate) to a double
   * for that step (possible doing this recursively down to
   * a potentially infinite resolution for a given step, depending
   * on the speed of the objects being tested)  Only enable
   * this if you are experiencing tunneling problems and can't
   * afford to increase the standard FrameRate in the settings
   * above
   */
  virtual void EnableFastObjects (bool enable) = 0;
  virtual bool FastObjectsEnabled () = 0;

};

SCF_VERSION (iODEDynamicSystemState, 0, 0, 2);

struct iODEBallJoint;
struct iODEHingeJoint;
struct iODEAMotorJoint;
struct iODEUniversalJoint;
struct iODESliderJoint;

/**
 * This class exposes parameters specific to odedynam as an implementation
 * of iDynamics.  In most cases SystemState should not be modified directly
 * unless you want the behavior of a specific system different from others.
 */
struct iODEDynamicSystemState : public iBase
{
  /**
   * Sets ODE's Error Resolution Parameter (see ode docs for details)
   * Setting this in iODEDynamicState will set it for each System
   * Use this only if you want a specific system to behave differently
   */
  virtual void SetERP (float erp) = 0;
  virtual float ERP () = 0;

  /**
   * Sets ODE's Constraint Force Mixing (see ode docs for details)
   * Setting this in iODEDynamicState will set it for each System
   * Use this only if you want a specific system to behave differently
   */
  virtual void SetCFM (float cfm) = 0;
  virtual float CFM () = 0;

  /**
   * Enables the experimental StepFast code in ode
   * Setting this in ODEDynamicState sets it here
   * Only modify it if you want a specific system to behave differently
   */
  virtual void EnableStepFast (bool enable) = 0;
  virtual bool StepFastEnabled () = 0;
  virtual void SetStepFastIterations (int iter) = 0;
  virtual int StepFastIterations () = 0;

  virtual void EnableQuickStep (bool enable) = 0;
  virtual bool QuickStepEnabled () = 0;
  virtual void SetQuickStepIterations (int iter) = 0;
  virtual int QuickStepIterations () = 0;

  /** Turn on/off AutoDisable functionality.
      AutoDisable will stop moving objects if they are stable in order
      to save processing time.
  */
  virtual void EnableAutoDisable (bool enable) = 0;
  virtual bool AutoDisableEnabled () =0;
  /**
    Set the parameters for AutoDisable.
    /param linear Maximum linear movement to disable a body
    /param angular Maximum angular movement to disable a body
    /param steps Minimum number of steps the body meets linear and angular
           requirements before it is disabled.
    /param time Minimum time the body needs to meet linear and angular movement
           requirements before it is disabled.
  */
  virtual void SetAutoDisableParams (float linear, float angular, int steps,
    float time)=0;

  /**
   * NOTE: This should not be done here if its been done in iODEDynamicState
   * The following code enables a constant framerate on processing
   * this means if you set the frame rate to (default) 50  The stepsize
   * passed into Step is treated as the elapsed time in seconds received
   * from the virtual clock GetElapsedTicks.
   * The physics will iterate a number of steps at 1/50th
   * of a second until enough time has passed to account for the time
   * Beware the default setting for frame limit is 10, which means
   * if the stepsize passed to Step is longer than 1/10 a second
   * the physics will stop iterating and slow down.  Never set this
   * parameter to 0 or else you could incur cycle of death where
   * the number of physics steps increases the amount of elapsed
   * time between frames which increases the number of physics steps
   * toward infinity
   */
  virtual void EnableFrameRate (bool enable) = 0;
  virtual bool FrameRateEnabled () = 0;

  virtual void SetFrameRate (float hz) = 0;
  virtual float FrameRate () = 0;

  virtual void SetFrameLimit (float hz) = 0;
  virtual float FrameLimit () = 0;

  virtual void AddFrameUpdateCallback (iODEFrameUpdateCallback *cb) = 0;
  virtual void RemoveFrameUpdateCallback (iODEFrameUpdateCallback *cb) = 0;

  /**
   * The following enables special robustness checks for fast
   * moving objects to determine if they will tunneling and
   * adjusts the physics frame resolution (rate) to a double
   * for that step (possible doing this recursively down to
   * a potentially infinite resolution for a given step, depending
   * on the speed of the objects being tested)  Only enable
   * this if you are experiencing tunneling problems and can't
   * afford to increase the standard FrameRate in the settings
   * above
   * Setting this in iODEDynamicState will set it for each System
   * Use this only if you want a specific system to behave differently
   */
  virtual void EnableFastObjects (bool enable) = 0;
  virtual bool FastObjectsEnabled () = 0;

  /// Create a ball joint and add it to he simulation
  virtual csPtr<iODEBallJoint> CreateBallJoint () = 0;

  /// Create a hinge joint and add it to he simulation
  virtual csPtr<iODEHingeJoint> CreateHingeJoint () = 0;

  /// Create a AMotor joint and add it to he simulation
  virtual csPtr<iODEAMotorJoint> CreateAMotorJoint () = 0;

  /// Create a Universal joint and add it to he simulation
  virtual csPtr<iODEUniversalJoint> CreateUniversalJoint () = 0;

  /// Create a Slider joint and add it to he simulation
  virtual csPtr<iODESliderJoint> CreateSliderJoint () = 0;

  /// Remove a ball joint from the simulation
  virtual void RemoveJoint (iODEBallJoint* joint) = 0;

  /// Remove a hinge joint from the simulation
  virtual void RemoveJoint (iODEHingeJoint* joint) = 0;

  /// Remove a AMotor joint from the simulation
  virtual void RemoveJoint (iODEAMotorJoint* joint) = 0;

  /// Remove a Universal joint from the simulation
  virtual void RemoveJoint (iODEUniversalJoint* joint) = 0;

  /// Remove a Slider joint from the simulation
  virtual void RemoveJoint (iODESliderJoint* joint) = 0;

  /** Set the maximum correcting velocity that contacts are
      allowed to generate. The default value is infinity (i.e. no
      limit). Reducing this value can help prevent "popping" of deeply
      embedded objects.
      @param v velocity
  */
  virtual void SetContactMaxCorrectingVel (float v) = 0;

  /** Get the maximum correcting velocity that contacts are
      allowed to generate. The default value is infinity (i.e. no
      limit). Reducing this value can help prevent "popping" of deeply
      embedded objects.
  */
  virtual float GetContactMaxCorrectingVel () = 0;

  /** Set the depth of the surface layer around all geometry
      objects. Contacts are allowed to sink into the surface layer up
      to the given depth before coming to rest. The default value is
      zero. Increasing this to some small value (e.g. 0.001) can help
      prevent jittering problems due to contacts being repeatedly made
      and broken.
      @param depth the distance two bodies are allowed to interpenetrate
   */
  virtual void SetContactSurfaceLayer (float depth) = 0;

  /** Get the depth of the surface layer around all geometry
      objects. Contacts are allowed to sink into the surface layer up
      to the given depth before coming to rest. The default value is
      zero. Increasing this to some small value (e.g. 0.001) can help
      prevent jittering problems due to contacts being repeatedly made
      and broken.
      @return the distance two bodies are allowed to interpenetrate
   */
  virtual float GetContactSurfaceLayer () = 0;
};

/**
 * TODO: Doc...
 * @@@ Document me
 */
enum ODEJointType
{
  CS_ODE_JOINT_TYPE_BALL = dJointTypeBall,
  CS_ODE_JOINT_TYPE_HINGE = dJointTypeHinge,
  CS_ODE_JOINT_TYPE_SLIDER = dJointTypeSlider,
  CS_ODE_JOINT_TYPE_CONTACT = dJointTypeContact,
  CS_ODE_JOINT_TYPE_UNIVERSAL = dJointTypeUniversal,
  CS_ODE_JOINT_TYPE_HINGE2 = dJointTypeHinge2,
  CS_ODE_JOINT_TYPE_FIXED = dJointTypeFixed,
  CS_ODE_JOINT_TYPE_AMOTOR = dJointTypeAMotor
};

SCF_VERSION (iODEJointState, 0, 0, 2);

/**
* General joint state. Here
*/
struct iODEJointState : public iBase
{
  virtual ODEJointType GetType() = 0;

  // Baaad interface. Are those number axes? If so, perhaps pass it as
  // parameter.
  virtual void SetLoStop (float value) = 0;
  virtual void SetHiStop (float value) = 0;
  virtual void SetVel (float value) = 0;
  virtual void SetFMax (float value) = 0;
  virtual void SetFudgeFactor (float value) = 0;
  virtual void SetBounce (float value) = 0;
  virtual void SetCFM (float value) = 0;
  virtual void SetStopERP (float value) = 0;
  virtual void SetStopCFM (float value) = 0;
  virtual void SetSuspensionERP (float value) = 0;
  virtual void SetSuspensionCFM (float value) = 0;

  virtual void SetLoStop2 (float value) = 0;
  virtual void SetHiStop2 (float value) = 0;
  virtual void SetVel2 (float value) = 0;
  virtual void SetFMax2 (float value) = 0;
  virtual void SetFudgeFactor2 (float value) = 0;
  virtual void SetBounce2 (float value) = 0;
  virtual void SetCFM2 (float value) = 0;
  virtual void SetStopERP2 (float value) = 0;
  virtual void SetStopCFM2 (float value) = 0;
  virtual void SetSuspensionERP2 (float value) = 0;
  virtual void SetSuspensionCFM2 (float value) = 0;

  virtual void SetLoStop3 (float value) = 0;
  virtual void SetHiStop3 (float value) = 0;
  virtual void SetVel3 (float value) = 0;
  virtual void SetFMax3 (float value) = 0;
  virtual void SetFudgeFactor3 (float value) = 0;
  virtual void SetBounce3 (float value) = 0;
  virtual void SetCFM3 (float value) = 0;
  virtual void SetStopERP3 (float value) = 0;
  virtual void SetStopCFM3 (float value) = 0;
  virtual void SetSuspensionERP3 (float value) = 0;
  virtual void SetSuspensionCFM3 (float value) = 0;

  virtual float GetLoStop () = 0;
  virtual float GetHiStop () = 0;
  virtual float GetVel () = 0;
  virtual float GetFMax () = 0;
  virtual float GetFudgeFactor () = 0;
  virtual float GetBounce () = 0;
  virtual float GetCFM () = 0;
  virtual float GetStopERP () = 0;
  virtual float GetStopCFM () = 0;
  virtual float GetSuspensionERP () = 0;
  virtual float GetSuspensionCFM () = 0;

  virtual float GetLoStop2 () = 0;
  virtual float GetHiStop2 () = 0;
  virtual float GetVel2 () = 0;
  virtual float GetFMax2 () = 0;
  virtual float GetFudgeFactor2 () = 0;
  virtual float GetBounce2 () = 0;
  virtual float GetCFM2 () = 0;
  virtual float GetStopERP2 () = 0;
  virtual float GetStopCFM2 () = 0;
  virtual float GetSuspensionERP2 () = 0;
  virtual float GetSuspensionCFM2 () = 0;

  virtual float GetLoStop3 () = 0;
  virtual float GetHiStop3 () = 0;
  virtual float GetVel3 () = 0;
  virtual float GetFMax3 () = 0;
  virtual float GetFudgeFactor3 () = 0;
  virtual float GetBounce3 () = 0;
  virtual float GetCFM3 () = 0;
  virtual float GetStopERP3 () = 0;
  virtual float GetStopCFM3 () = 0;
  virtual float GetSuspensionERP3 () = 0;
  virtual float GetSuspensionCFM3 () = 0;

  // This is a very ugly hack quite specific to NmS
  virtual void SetHinge2Axis1 (const csVector3& axis) = 0;
  virtual void SetHinge2Axis2 (const csVector3& axis) = 0;
  virtual void SetHinge2Anchor (const csVector3& point) = 0;
};

/**
 * General joint state.
 */
struct iODEGeneralJointState : public iBase
{
  /**
   * Set low stop angle or position. For rotational joints, this
   * stop must be greater than - pi to be effective.
   */
  virtual void SetLoStop (float value, int axis) = 0;

  /**
   * Set high stop angle or position. For rotational joints, this stop must
   * be less than pi to be effective. If the high stop is less than the low
   * stop then both stops will be ineffective.
   */
  virtual void SetHiStop (float value, int axis) = 0;

  /// Set desired motor velocity (this will be an angular or linear velocity).
  virtual void SetVel (float value, int axis) = 0;

  /**
   * Set the maximum force or torque that the motor will use to achieve the desired
   * velocity. This must always be greater than or equal to zero. Setting this to zero
   * turns off the motor.
   */
  virtual void SetFMax (float value, int axis) = 0;

  /**
   * Set the fudge factor. The current joint stop/motor implementation has a small
   * problem: when the joint is at one stop and the motor is set to move it away from
   * the stop, too much force may be applied for one time step, causing a ``jumping''
   * motion. This fudge factor is used to scale this excess force. It should have a value
   * between zero and one (the default value). If the jumping motion is too visible in a
   * joint, the value can be reduced. Making this value too small can prevent the motor
   * from being able to move the joint away from a stop.
   */
  virtual void SetFudgeFactor (float value, int axis) = 0;

  /**
   * Set the bouncyness of the stops. This is a restitution parameter in the range 0..1.
   * 0 means the stops are not bouncy at all, 1 means maximum bouncyness.
   */
  virtual void SetBounce (float value, int axis) = 0;

  /// Set the constraint force mixing (CFM) value for joint used when not at a stop.
  virtual void SetCFM (float value, int axis) = 0;

  /// Set the error reduction parameter (ERP) used by the stops.
  virtual void SetStopERP (float value, int axis) = 0;

  /**
   * Set the constraint force mixing (CFM) value for joint used by the stops.
   * Together with the ERP value this can be used to get spongy or soft stops.
   * Note that this is intended for unpowered joints, it does not really work as expected
   * when a powered joint reaches its limit.
   */
  virtual void SetStopCFM (float value, int axis) = 0;

  /// Set suspension error reduction parameter (ERP).
  virtual void SetSuspensionERP (float value, int axis) = 0;

  /// Set suspension constraint force mixing (CFM) value.
  virtual void SetSuspensionCFM (float value, int axis) = 0;

  /// Get low stop angle or position.
  virtual float GetLoStop (int axis) = 0;

  /// Get high stop angle or position.
  virtual float GetHiStop (int axis) = 0;

  /// Get desired motor velocity (this will be an angular or linear velocity).
  virtual float GetVel (int axis) = 0;

  /// Get the maximum force or torque that the motor will use to achieve the desired velocity.
  virtual float GetFMax (int axis) = 0;

  ///Get the fudge factor.
  virtual float GetFudgeFactor (int axis) = 0;

  /// Get the bouncyness of the stops.
  virtual float GetBounce (int axis) = 0;

  /// Get the constraint force mixing (CFM) value for joint used when not at a stop.
  virtual float GetCFM (int axis) = 0;

  /// Get the error reduction parameter (ERP) used by the stops.
  virtual float GetStopERP (int axis) = 0;

  /// Get the constraint force mixing (CFM) value for joint used by the stops.
  virtual float GetStopCFM (int axis) = 0;

  /// Get suspension error reduction parameter (ERP).
  virtual float GetSuspensionERP (int axis) = 0;

  /// Get suspension constraint force mixing (CFM) value.
  virtual float GetSuspensionCFM (int axis) = 0;

  /**
   * Attach the joint to some new bodies. If the joint is already attached, it
   * will be detached from the old bodies first. To attach this joint to only
   * one body, set body1 or body2 to zero - a zero body refers to the static
   * environment. Setting both bodies to zero puts the joint into "limbo", i.e.
   * it will have no effect on the simulation.
   */
  virtual void Attach (iRigidBody *body1, iRigidBody *body2) = 0;

  /// Get an attached body (valid values for body are 0 and 1)
  virtual csRef<iRigidBody> GetAttachedBody (int body) = 0;

  /// Get force that joint applies to body 1
  virtual csVector3 GetFeedbackForce1 () = 0;

  /// Get torque that joint applies to body 1
  virtual csVector3 GetFeedbackTorque1 () = 0;

  /// Get force that joint applies to body 2
  virtual csVector3 GetFeedbackForce2 () = 0;

  /// Get torque that joint applies to body 2
  virtual csVector3 GetFeedbackTorque2 () = 0;

};

SCF_VERSION (iODESliderJoint, 0, 0, 1);

struct iODESliderJoint : public iODEGeneralJointState
{
  ///Set the slider axis.
  virtual void SetSliderAxis (float x, float y, float z) = 0;

  ///Get the slider axis.
  virtual csVector3 GetSliderAxis () = 0;

  /**
   * Get the slider linear position (i.e. the slider's "extension").
   * When the axis is set, the current position of the attached bodies
   * is examined and that position will be the zero position.
   */
  virtual float GetSliderPosition () = 0;

  ///Get the time derivative of slider position.
  virtual float GetSliderPositionRate () = 0;
};

SCF_VERSION (iODEUniversalJoint, 0, 0, 1);
/**
 * A universal joint is like a ball and socket joint that constrains
 * an extra degree of rotational freedom. Given axis 1 on body 1, and
 * axis 2 on body 2 that is perpendicular to axis 1, it keeps them
 * perpendicular. In other words, rotation of the two bodies about the
 * direction perpendicular to the two axes will be equal.
 */
struct iODEUniversalJoint : public iODEGeneralJointState
{
  ///Set universal anchor.
  virtual void SetUniversalAnchor (float x, float y, float z) = 0;

  ///Set axis on body 1 (should be perpendicular to axis 2)
  virtual void SetUniversalAxis1 (float x, float y, float z) = 0;

  ///Set axis on body 2 (should be perpendicular to axis 1)
  virtual void SetUniversalAxis2 (float x, float y, float z) = 0;

  /**
   * Get the joint anchor point, in world coordinates. This returns
   * the point on body 1. If the joint is perfectly satisfied, this
   * will be the same as the point on body 2.
   */
  virtual csVector3 GetUniversalAnchor1 () = 0;

  /**
   * Get the joint anchor point, in world coordinates. This returns
   * the point on body 2. If the joint is perfectly satisfied, this
   * will be the same as the point on body 1.
   */
  virtual csVector3 GetUniversalAnchor2 () = 0;

  ///Get universal axis on body 1.
  virtual csVector3 GetUniversalAxis1 () = 0;

  ///Get universal axis on body 2.
  virtual csVector3 GetUniversalAxis2 () = 0;

};

enum ODEAMotorMode
{
  CS_ODE_AMOTOR_MODE_USER = dAMotorUser,
  CS_ODE_AMOTOR_MODE_EULER = dAMotorEuler
};

SCF_VERSION (iODEAMotorJoint, 0, 0, 1);

/**
 * ODE AMotor joint. An angular motor (AMotor) allows the relative
 * angular velocities of two bodies to be controlled. The angular
 * velocity can be controlled on up to three axes, allowing torque
 * motors and stops to be set for rotation about those axes. This
 * is mainly useful in conjunction with ball joints (which do not
 * constrain the angular degrees of freedom at all), but it can be
 * used in any situation where angular control is needed. To use an
 * AMotor with a ball joint, simply attach it to the same two bodies
 * that the ball joint is attached to.
 */
struct iODEAMotorJoint : public iODEGeneralJointState
{

  /**
   * Set the angular motor mode. The mode parameter must be one of the
   * following constants: CS_ODE_AMOTOR_MODE_USER (The AMotor axes and
   * joint angle settings are entirely controlled by the user, this is
   * the default mode), CS_ODE_AMOTOR_MODE_EULER ( Euler angles are
   * automatically computed, when this mode is initially set the current
   * relative orientations of the bodies will correspond to all euler
   * angles at zero).
   */
  virtual void SetAMotorMode (ODEAMotorMode mode) = 0;

  /**
   * Get the angular motor mode.
   */
  virtual ODEAMotorMode GetAMotorMode () = 0;

  /**
   * Set the number of angular axes that will be controlled by the
   * AMotor. The argument num can range from 0 (which effectively
   * deactivates the joint) to 3. This is automatically set to 3
   * in CS_ODE_AMOTOR_MODE_EULER mode.
   */
  virtual void SetAMotorNumAxes (int axis_num) = 0;

  /**
   * Get the number of angular axes that will be controlled by the
   * AMotor.
   */
  virtual int GetAMotorNumAxes () = 0;

  /**
    Set AMotor axis.
    /param axis_num - axis number
    /param rel_orient - ``relative orientation'' mode:
    0: The axis is anchored to the global frame.
    1: The axis is anchored to the first body.
    2: The axis is anchored to the second body.
    /param x, y, z - axis
   */
  virtual void SetAMotorAxis (int axis_num, int rel_orient, float x, float y, float z) = 0;

  /**
    Set AMotor axis.
    /param axis_num - axis number
    /param rel_orient - ``relative orientation'' mode:
    0: The axis is anchored to the global frame.
    1: The axis is anchored to the first body.
    2: The axis is anchored to the second body.
   */
  virtual void SetAMotorAxis (int axis_num, int rel_orient, const csVector3 &axis) = 0;

  /**
   * Get AMotor axis.
   */
  virtual csVector3 GetAMotorAxis (int axis_num) = 0;

  /**
   * Get ``relative orientation'' mode:
   * 0: The axis is anchored to the global frame.
   * 1: The axis is anchored to the first body.
   * 2: The axis is anchored to the second body.
   */
  virtual int GetAMotorAxisRelOrientation (int axis_num) = 0;

  /**
   * Tell the AMotor what the current angle is along axis anum. This
   * function should only be called in CS_ODE_AMOTOR_MODE_USER mode,
   * because in this mode the AMotor has no other way of knowing the
   * joint angles. The angle information is needed if stops have been
   * set along the axis, but it is not needed for axis motors.
   */
  virtual void SetAMotorAngle (int axis_num, float angle) = 0;

  /**
   * Return the current angle for axis anum. In CS_ODE_AMOTOR_MODE_USER
   * mode this is simply the value that was set with SetAMotorAngle. In
   * CS_ODE_AMOTOR_MODE_EULER mode this is the corresponding euler angle.
   */
  virtual float GetAMotorAngle (int axis_num) = 0;

  /**
   * Return the current angle rate for axis anum. In CS_ODE_AMOTOR_MODE_USER
   * mode this is always zero, as not enough information is available. In
   * CS_ODE_AMOTOR_MODE_EULER mode this is the corresponding euler angle rate.
   */
  virtual float GetAMotorAngleRate (int axis_num) = 0;

};

SCF_VERSION (iODEHingeJoint, 0, 0, 1);

/**
 * ODE hinge joint (contrainted translation and 1 free rotation axis).
 */
struct iODEHingeJoint : public iODEGeneralJointState
{
  /**
   * Set the joint anchor point. The joint will try to keep this point
   * on each body together. Input specified in world coordinates.
   */
  virtual void SetHingeAnchor (const csVector3 &pos) = 0;

  /**
   * Sets free hinge axis.
   */
  virtual void SetHingeAxis (const csVector3 &axis) = 0;

  /**
   * Get the joint anchor point, in world coordinates. This returns the
   * point on body 1.
   */
  virtual csVector3 GetHingeAnchor1 () = 0;

  /**
   * Get the joint anchor point, in world coordinates. This returns the
   * point on body 2.
   */
  virtual csVector3 GetHingeAnchor2 () = 0;

  /**
   * Get free hinge axis.
   */
  virtual csVector3 GetHingeAxis () = 0;

  /**
   * Get the hinge angle. The nagle is measured between the two bodies.
   * The angle will be between -pi..pi. When the hinge anchor or axis
   * is set, the current position of the attached bodies is examined and
   * that position will be the zero angle.
   */
  virtual float GetHingeAngle () = 0;

  /**
   * Get the time derivative of angle value.
   */
  virtual float GetHingeAngleRate () = 0;

  /**
   * This value will show you how far the joint has come apart.
   */
  virtual csVector3 GetAnchorError () = 0;

};

SCF_VERSION (iODEBallJoint, 0, 0, 1);

/**
 * ODE ball and socket joint (contrainted translation and free rotation).
 */
struct iODEBallJoint : public iBase
{
  /**
   * Set the joint anchor point. The joint will try to keep this point
   * on each body together. Input specified in world coordinates.
   */
  virtual void SetBallAnchor (const csVector3 &pos) = 0;

  /**
   * Get the joint anchor point, in world coordinates. This returns
   * the point on body 1.
   */
  virtual csVector3 GetBallAnchor1 () = 0;

  /**
   * Get the joint anchor point, in world coordinates. This returns the
   * point on body 2.
   */
  virtual csVector3 GetBallAnchor2 () = 0;

  /**
   * This value will show you how far the joint has come apart.
   */
  virtual csVector3 GetAnchorError () = 0;

   /**
   * Attach the joint to some new bodies. If the joint is already attached, it
   * will be detached from the old bodies first. To attach this joint to only
   * one body, set body1 or body2 to zero - a zero body refers to the static
   * environment. Setting both bodies to zero puts the joint into "limbo", i.e.
   * it will have no effect on the simulation.
   */
  virtual void Attach (iRigidBody *body1, iRigidBody *body2) = 0;

  /// Get an attached body (valid values for body are 0 and 1)
  virtual csRef<iRigidBody> GetAttachedBody (int body) = 0;

  /// Get force that joint applies to body 1
  virtual csVector3 GetFeedbackForce1 () = 0;

  /// Get torque that joint applies to body 1
  virtual csVector3 GetFeedbackTorque1 () = 0;

  /// Get force that joint applies to body 2
  virtual csVector3 GetFeedbackForce2 () = 0;

  /// Get torque that joint applies to body 2
  virtual csVector3 GetFeedbackTorque2 () = 0;
};



#endif // __CS_IVARIA_ODE_H__
