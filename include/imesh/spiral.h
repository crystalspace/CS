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

#ifndef __CS_IMESH_SPIRAL_H__
#define __CS_IMESH_SPIRAL_H__

/**\file
 * Spiral particle mesh object
 */ 

#include "csutil/scf.h"

/**\addtogroup meshplugins
 * @{ */

class csColor;
class csVector3;

SCF_VERSION (iSpiralState, 0, 0, 2);

/**
 * This interface describes the API for the spiral mesh object.
 */
struct iSpiralState : public iBase
{
  /// Set the number of particles to use.
  virtual void SetParticleCount (int num) = 0;
  /// Get the number of particles.
  virtual int GetParticleCount () const = 0;
  /// Set the size of the spiral particles.
  virtual void SetParticleSize (float partwidth, float partheight) = 0;
  /// Get the size of the spiral particles.
  virtual void GetParticleSize (float& partwidth, float& partheight) const = 0;
  /// Set the source for the particles.
  virtual void SetSource (const csVector3& source) = 0;
  /// Get the source for the particles.
  virtual const csVector3& GetSource () const = 0;
  /// Set the time to live for all particles, in msec.
  virtual void SetParticleTime (csTicks ttl) = 0;
  /// Get the time to live of particles, in msec.
  virtual csTicks GetParticleTime () const = 0;
  /// Set particle radial speed in spiral
  virtual void SetRadialSpeed (float speed) = 0;
  /// Get particle radial speed in spiral
  virtual float GetRadialSpeed () const = 0;
  /// Set particle rotation speed in spiral
  virtual void SetRotationSpeed (float speed) = 0;
  /// Get particle rotation speed in spiral
  virtual float GetRotationSpeed () const = 0;
  /// Set particle climb speed in spiral
  virtual void SetClimbSpeed (float speed) = 0;
  /// Get particle climb speed in spiral
  virtual float GetClimbSpeed () const = 0;
};

/** @} */

#endif // __CS_IMESH_SPIRAL_H__

