/*
    Copyright (C) 2003 by Jorrit Tyberghein, Daniel Duhprey

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

#endif // __CS_IVARIA_ODE_H__
