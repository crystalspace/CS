/*
    Copyright (C) 1998, 1999 by Nathaniel 'NooTe' Saint Martin
    Copyright (C) 1998, 1999 by Jorrit Tyberghein
    Written by Nathaniel 'NooTe' Saint Martin

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

#if !defined(__ISOUNDLISTENER_H__)
#define __ISOUNDLISTENER_H__

#include "isndrdr.h"

/// taken from eax preset environment
enum SoundEnvironment
{
  ENVIRONMENT_GENERIC = 0,
  ENVIRONMENT_PADDEDCELL,
  ENVIRONMENT_ROOM,
  ENVIRONMENT_BATHROOM,
  ENVIRONMENT_LIVINGROOM,
  ENVIRONMENT_STONEROOM,
  ENVIRONMENT_AUDITORIUM,
  ENVIRONMENT_CONCERTHALL,
  ENVIRONMENT_CAVE,
  ENVIRONMENT_ARENA,
  ENVIRONMENT_CARPETEDHALLWAY,
  ENVIRONMENT_HALLWAY,
  ENVIRONMENT_STONECORRIDOR,
  ENVIRONMENT_ALLEY,
  ENVIRONMENT_FOREST,
  ENVIRONMENT_CITY,
  ENVIRONMENT_MOUNTAINS,
  ENVIRONMENT_QUARRY,
  ENVIRONMENT_PLAIN,
  ENVIRONMENT_PARKINGLOT,
  ENVIRONMENT_SEWERPIPE,
  ENVIRONMENT_UNDERWATER,
  ENVIRONMENT_DRUGGED,
  ENVIRONMENT_DIZZY,
  ENVIRONMENT_PSYCHOTIC
};

SCF_INTERFACE (iSoundListener, 0, 0, 1) : public iBase
{
  /// Set direction of listener (front and top 3d vectors)
  virtual void SetDirection (float fx, float fy, float fz, float tx, float ty, float tz) = 0;
  /// Set position of listener
  virtual void SetPosition (float x, float y, float z) = 0;
  /// Set velocity of listener
  virtual void SetVelocity (float x, float y, float z) = 0;
  /// Set a distance attenuator
  virtual void SetDistanceFactor (float factor) = 0;
  /// Set a RollOff factor
  virtual void SetRollOffFactor (float factor) = 0;
  /// Set the Doppler attenuator
  virtual void SetDopplerFactor (float factor) = 0;
  /// Set distance between the 2 'ears' of listener
  virtual void SetHeadSize (float size) = 0;
  /// set type of environment where 'live' listener
  virtual void SetEnvironment (SoundEnvironment env) = 0;

  /// Get direction of listener (front and top 3d vectors)
  virtual void GetDirection (float &fx, float &fy, float &fz, float &tx, float &ty, float &tz) = 0;
  /// Get position of listener
  virtual void GetPosition (float &x, float &y, float &z) = 0;
  /// Get velocity of listener
  virtual void GetVelocity (float &x, float &y, float &z) = 0;
  /// Get a distance attenuator
  virtual float GetDistanceFactor () = 0;
  /// Get a RollOff factor
  virtual float GetRollOffFactor () = 0;
  /// Get the Doppler attenuator
  virtual float GetDopplerFactor () = 0;
  /// Get distance between the 2 'ears' of listener
  virtual float GetHeadSize () = 0;
  /// Get type of environment where 'live' listener
  virtual SoundEnvironment GetEnvironment () = 0;
};

#endif // __ISOUNDLISTENER_H__
