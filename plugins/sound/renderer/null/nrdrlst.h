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

#if !defined(__CSSOUNDLISTENERNULL_H__)
#define __CSSOUNDLISTENERNULL_H__

#include "cssfxldr/common/snddata.h"
#include "isndrdr.h"
#include "isndlstn.h"

class csSoundListenerNull : public iSoundListener
{
public:
  DECLARE_IBASE;

  csSoundListenerNull();
  virtual ~csSoundListenerNull();

  /// Set direction of listener (front and top 3d vectors)
  virtual void SetDirection (float fx, float fy, float fz, float tx, float ty, float tz);
  /// Set position of listener
  virtual void SetPosition (float x, float y, float z);
  /// Set velocity of listener
  virtual void SetVelocity (float x, float y, float z);
  /// Set a distance attenuator
  virtual void SetDistanceFactor (float factor);
  /// Set a RollOff factor
  virtual void SetRollOffFactor (float factor);
  /// Set the Doppler attenuator
  virtual void SetDopplerFactor (float factor);
  /// Set distance between the 2 'ears' of listener
  virtual void SetHeadSize (float size);
  /// set type of environment where 'live' listener
  virtual void SetEnvironment (SoundEnvironment env);

  /// Get direction of listener (front and top 3d vectors)
  virtual void GetDirection (float &fx, float &fy, float &fz, float &tx, float &ty, float &tz);
  /// Get position of listener
  virtual void GetPosition (float &x, float &y, float &z);
  /// Get velocity of listener
  virtual void GetVelocity (float &x, float &y, float &z);
  /// Get a distance attenuator
  virtual float GetDistanceFactor ();
  /// Get a RollOff factor
  virtual float GetRollOffFactor ();
  /// Get the Doppler attenuator
  virtual float GetDopplerFactor ();
  /// Get distance between the 2 'ears' of listener
  virtual float GetHeadSize ();
  /// Get type of environment where 'live' listener
  virtual SoundEnvironment GetEnvironment ();

public:
  // Position
  float fPosX, fPosY, fPosZ;
  // Velocity
  float fVelX, fVelY, fVelZ;
  // Direction
  float fDirTopX, fDirTopY, fDirTopZ, fDirFrontX, fDirFrontY, fDirFrontZ;
  // Doppler
  float fDoppler;
  // Distance
  float fDistance;
  // RollOff
  float fRollOff;
  // HeadSize
  float fHeadSize;
  // Environment
  SoundEnvironment Environment;
};

#endif // __CSSOUNDLISTENERNULL_H__
