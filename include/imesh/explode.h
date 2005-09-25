/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef __CS_IMESH_EXPLODE_H__
#define __CS_IMESH_EXPLODE_H__

/**\file
 * Explosion particle mesh object
 */ 

#include "csutil/scf.h"

class csColor;
class csVector3;

/**\addtogroup meshplugins
 * @{ */

SCF_VERSION (iExplosionState, 0, 0, 2);

/**
 * This interface describes the API for the explosion mesh object.
 */
struct iExplosionState : public iBase
{
  /// Set the number of particles to use.
  virtual void SetParticleCount (int num) = 0;
  /// Get the number of particles
  virtual int GetParticleCount () const = 0;
  /// Set the explosion center.
  virtual void SetCenter (const csVector3& center) = 0;
  /// Get the explosion center
  virtual const csVector3 &GetCenter () const = 0;
  /// Set the push vector.
  virtual void SetPush (const csVector3& push) = 0;
  /// Get the push vector.
  virtual const csVector3& GetPush () const = 0;
  /// Set the number of sides.
  virtual void SetNrSides (int nr_sides) = 0;
  /// Get the number of sides.
  virtual int GetNrSides () const = 0;
  /// Set the radius of all particles.
  virtual void SetPartRadius (float part_radius) = 0;
  /// Get the radius of all particles.
  virtual float GetPartRadius () const = 0;
  /// Enable or disable lighting.
  virtual void SetLighting (bool l) = 0;
  /// See if lighting is enabled or disabled.
  virtual bool GetLighting () const = 0;
  /// Set the spread position.
  virtual void SetSpreadPos (float spread_pos) = 0;
  /// Get the spread position.
  virtual float GetSpreadPos () const = 0;
  /// Set the spread speed.
  virtual void SetSpreadSpeed (float spread_speed) = 0;
  /// Get the spread speed.
  virtual float GetSpreadSpeed () const = 0;
  /// Set the spread acceleration.
  virtual void SetSpreadAcceleration (float spread_accel) = 0;
  /// Get the spread acceleration.
  virtual float GetSpreadAcceleration () const = 0;
  /**
   * Set particles to be scaled to nothing starting at fade_particles msec
   * before self-destruct.
   */
  virtual void SetFadeSprites (csTicks fade_time) = 0;
  /// See if particles are faded (returns true), and returns fade time too.
  virtual bool GetFadeSprites (csTicks& fade_time) const = 0;
};

/** @} */

#endif // __CS_IMESH_EXPLODE_H__

