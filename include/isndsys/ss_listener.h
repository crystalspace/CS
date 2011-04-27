/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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

#ifndef __CS_SNDSYS_LISTENER_H__
#define __CS_SNDSYS_LISTENER_H__

/**\file
 * Sound system: listener object
 */

#include "csutil/scf.h"
#include "csgeom/vector3.h"

/**\addtogroup sndsys
 * @{ */

/**
 * The sound listener is a unique object created by the sound renderer. It
 * can be used to setup 'yourself' (the player) for 3d sound: position,
 * orientation, speed and environment effects.
 */
struct iSndSysListener : public virtual iBase
{
  SCF_INTERFACE (iSndSysListener, 0, 0, 2);

  /// Set direction of listener (front and top 3d vectors)
  virtual void SetDirection (const csVector3 &Front, const csVector3 &Top) = 0;
  /// Set position of listener
  virtual void SetPosition (const csVector3 &pos) = 0;
  /// Set a distance attenuator
  virtual void SetDistanceFactor (float factor) = 0;
  /// Set a RollOff factor
  virtual void SetRollOffFactor (float factor) = 0;
  /// set type of environment where 'live' listener
  //virtual void SetEnvironment (csSoundEnvironment env) = 0;


  /// Get direction of listener (front and top 3d vectors)
  virtual void GetDirection (csVector3 &Front, csVector3 &Top) = 0;
  /// Get position of listener
  virtual const csVector3 &GetPosition () = 0;
  /// Get a distance attenuator
  virtual float GetDistanceFactor () = 0;
  /// Get a RollOff factor
  virtual float GetRollOffFactor () = 0;
  /// Get type of environment where 'live' listener
  //virtual csSoundEnvironment GetEnvironment () = 0;
};

/**
 * Extension to the iSndSysListener interface, allowing Doppler shift effects.
 * The Doppler effect that causes sound sources the change in pitch as their
 * relative velocities change. As an example the siren of an ambulance will
 * increase in pitch as it approaches you, and decrease once it has passed you.
 *
 * @see iSndSysSource3DDoppler
 */
struct iSndSysListenerDoppler : public virtual iBase
{
  SCF_INTERFACE (iSndSysListenerDoppler, 0, 0, 2);

  /// Set velocity (speed) of the listener
  virtual void SetVelocity (const csVector3 &Velocity) = 0;
  /// Set the Doppler factor
  virtual void SetDopplerFactor (const float DopplerFactor) = 0;
  /// Set the speed of sound
  virtual void SetSpeedOfSound (const float SpeedOfSound) = 0;

  /// Get velocity (speed) of th elistener
  virtual const csVector3 &GetVelocity () = 0;
  /// Get the Doppler factor
  virtual float GetDopplerFactor () = 0;
  /// Get the speed of sound
  virtual float GetSpeedOfSound () = 0;
};

/** @} */

#endif // __CS_SNDSYS_LISTENER_H__
