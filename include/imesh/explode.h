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

#ifndef __IMESH_EXPLODE_H__
#define __IMESH_EXPLODE_H__

#include "csutil/scf.h"

class csColor;
struct iEngine;
struct iSector;

SCF_VERSION (iExplosionState, 0, 0, 1);

/**
 * This interface describes the API for the explosion mesh object.
 */
struct iExplosionState : public iBase
{
  /// Set the number of particles to use.
  virtual void SetNumberParticles (int num) = 0;
  /// Set the explosion center.
  virtual void SetCenter (const csVector3& center) = 0;
  /// Set the push vector.
  virtual void SetPush (const csVector3& push) = 0;
  /// Set the number of sides.
  virtual void SetNrSides (int nr_sides) = 0;
  /// Set the radius of all particles.
  virtual void SetPartRadius (float part_radius) = 0;
  /// Enable or disable lighting.
  virtual void SetLighting (bool l) = 0;
  /// Set the spread position.
  virtual void SetSpreadPos (float spread_pos) = 0;
  /// Set the spread speed.
  virtual void SetSpreadSpeed (float spread_speed) = 0;
  /// Set the spread acceleration.
  virtual void SetSpreadAcceleration (float spread_accel) = 0;
  /**
   * Set particles to be scaled to nothing starting at fade_particles msec 
   * before self-destruct.
   */
  virtual void SetFadeSprites (cs_time fade_time) = 0;
  /**
   * Add a light at explosion center. add msec when light starts fading,
   * which is used when time_to_live is set / SelfDestruct is used.
   */
  virtual void AddLight (iEngine*, iSector*, cs_time fade = 200) = 0;
};

#endif

