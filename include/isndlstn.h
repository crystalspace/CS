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
#include "cssndldr/common/sndbuf.h"	//@@@BAD

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

extern const GUID IID_ISoundListener;

typedef struct
{
  /// Position of listener object
  float fPosX, fPosY, fPosZ;
  /// Front Direction of listener object
  float fDirFrontX, fDirFrontY, fDirFrontZ;
  /// Top Direction of listener object
  float fDirTopX, fDirTopY, fDirTopZ;
  /// Velocity of listener object
  float fVelX, fVelY, fVelZ;
  /// Doppler factor
  float fDoppler;
  /// Distance factor
  float fDistance;
  /// RollOff factor
  float fRollOff;
  /// Hear size (ears distance)
  float fHeadSize;
  /// Type of environment where is listener
  SoundEnvironment Environment;
} csSoundListenerInfo;

interface ISoundListener : public IUnknown
{
public:
  /// Set direction of listener (front and top 3d vectors)
  STDMETHOD (SetDirection) (float fx, float fy, float fz, float tx, float ty, float tz) PURE;
  /// Set position of listener
  STDMETHOD (SetPosition) (float x, float y, float z) PURE;
  /// Set velocity of listener
  STDMETHOD (SetVelocity) (float x, float y, float z) PURE;
  /// Set a distance attenuator
  STDMETHOD (SetDistanceFactor) (float factor) PURE;
  /// Set a RollOff factor
  STDMETHOD (SetRollOffFactor) (float factor) PURE;
  /// Set the Doppler attenuator
  STDMETHOD (SetDopplerFactor) (float factor) PURE;
  /// Set distance between the 2 'ears' of listener
  STDMETHOD (SetHeadSize) (float size) PURE;
  /// set type of environment where 'live' listener
  STDMETHOD (SetEnvironment) (SoundEnvironment env) PURE;

  STDMETHOD (GetInfoListener) (csSoundListenerInfo *info) PURE;
};

#endif // __ISOUNDLISTENER_H__
