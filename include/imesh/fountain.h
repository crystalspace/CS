/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Copyright (C) 2001 by W.C.A. Wijngaards

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

#ifndef __CS_IMESH_FOUNTAIN_H__
#define __CS_IMESH_FOUNTAIN_H__

#include "csutil/scf.h"

class csColor;
class csVector3;

SCF_VERSION (iFountainState, 0, 0, 1);

/**
 * This interface describes the API for the fountain mesh object.
 */
struct iFountainState : public iBase
{
  /// Set the number of particles to use.
  virtual void SetParticleCount (int num) = 0;
  /// Get the number of particles used.
  virtual int GetParticleCount () const = 0;
  /// Set the size of the fountain drops.
  virtual void SetDropSize (float dropwidth, float dropheight) = 0;
  /// Get the size of the fountain drops.
  virtual void GetDropSize (float& dropwidth, float& dropheight) const = 0;
  /// Set origin of the fountain.
  virtual void SetOrigin (const csVector3& origin) = 0;
  /// Get origin of the fountain.
  virtual const csVector3& GetOrigin () const = 0;
  /// Enable or disable lighting.
  virtual void SetLighting (bool l) = 0;
  /// See if lighting is enabled.
  virtual bool GetLighting () const = 0;
  /// Set acceleration.
  virtual void SetAcceleration (const csVector3& accel) = 0;
  /// Get acceleration.
  virtual const csVector3& GetAcceleration () const = 0;
  /// Set elevation.
  virtual void SetElevation (float elev) = 0;
  /// Get elevation.
  virtual float GetElevation () const = 0;
  /// Set azimuth.
  virtual void SetAzimuth (float azi) = 0;
  /// Get azimuth.
  virtual float GetAzimuth () const = 0;
  /// Set opening.
  virtual void SetOpening (float open) = 0;
  /// Get opening.
  virtual float GetOpening () const = 0;
  /// Set speed.
  virtual void SetSpeed (float spd) = 0;
  /// Get speed.
  virtual float GetSpeed () const = 0;
  /// Set fall time.
  virtual void SetFallTime (float ftime) = 0;
  /// Get fall time.
  virtual float GetFallTime () const = 0;
};

#endif // __CS_IMESH_FOUNTAIN_H__

