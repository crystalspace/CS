/*
    Copyright (C) 2000 by W.C.A. Wijngaards

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

#ifndef __CS_PARTEXPLO_H__
#define __CS_PARTEXPLO_H__

#include "csgeom/vector3.h"
#include "csgeom/box.h"
#include "csobject/csobject.h"
#include "csutil/cscolor.h"
#include "csengine/cssprite.h"
#include "csengine/particle.h"
#include "iparticl.h"

class csSprite2D;
class csMaterialWrapper;
class csEngine;
class csSector;
class csDynLight;
class csLight;
class csRenderView;

/**
 * An explosive particle system.
 * particles explode outward from defined center.
 */
class csParSysExplosion : public csNewtonianParticleSystem
{
protected:
  /// Center of explosion.
  csVector3 center;
  /// Dynamic light at center.
  bool has_light;
  csSector *light_sector;
  csEngine *light_engine;
  csDynLight *explight;
  cs_time light_fade;
  /// scaling of particles.
  bool scale_particles;
  cs_time fade_particles;
  /// starting bbox.
  csBox3 startbox;
  float maxspeed, maxaccel, radiusnow;

  /// Move particles and light(if any) to a sector.
  virtual void MoveToSector (csSector *sector);

public:
  /**
   * Give number of particles and center. 
   * push is speed added to all particles (i.e. speed of object being 
   * destroyed for example, or a wind pushing all particles in a direction),
   * give a material to use as well,
   * nr_sides is the number of sides of every particle polygon.
   * part_radius is the radius of every particle,
   * spreading multipliers: a random number (1.0..+1.0) * spread is added.
   */
  csParSysExplosion (csObject* theParent, int number_p,
  	const csVector3& explode_center,
  	const csVector3& push, csMaterialWrapper *mat, int nr_sides = 6,
	float part_radius = 0.25, bool lighted_particles = false,
	float spread_pos = 0.6, 
	float spread_speed = 0.5, float spread_accel = 1.5);
  /// Destructor.
  virtual ~csParSysExplosion ();

  /// Update and light is flickered as well. particles will be scaled.
  virtual void Update (cs_time elapsed_time);

  /// Get the center of the explosion.
  csVector3& GetCenter () { return center; }

  /// Explosion has a dynamic light at the center?
  bool HasLight () { return has_light; }
  /**
   * Add a light at explosion center. add msec when light starts fading,
   * which is used when time_to_live is set / SelfDestruct is used.
   */
  void AddLight (csEngine*, csSector*, cs_time fade = 200);
  /// Remove the light.
  void RemoveLight ();

  /// Are particles scaled to nothing at end?
  bool GetFadeSprites() { return scale_particles;}
  /**
   * Set particles to be scaled to nothing starting at fade_particles msec 
   * before self-destruct.
   */
  void SetFadeSprites(cs_time fade_time) 
  {scale_particles=true; fade_particles = fade_time; }

  CSOBJTYPE;
};


#endif // __CS_PARTEXPLO_H__
