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

SCF_VERSION (iODEDynamicSystemState, 0, 0, 1);

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

/// @@@ Document me.
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

#endif // __CS_IVARIA_ODE_H__
