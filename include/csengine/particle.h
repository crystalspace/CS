/*
    Copyright (C) 2000 by Jorrit Tyberghein
    (C) W.C.A. Wijngaards, 2000

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

#ifndef CSPARTIC_H
#define CSPARTIC_H

#include "csgeom/vector3.h"
#include "csobject/csobject.h"

class csSprite2D;
class csTextureHandle;
class csWorld;
class csSector;
class csDynLight;

/**
 * This class represents a particle system. It is a set of 2d sprites.
 * Subclasses of this class may be of more interest to users.
 * More specialised particle systems can be found below.
 */
class csParticleSystem : public csObject
{
protected:
  /// Max particles in current arrays.
  int max_particles;
  /// The number of particles in use ( <= max_particles ).
  int num_particles;
  /// Array of ptrs to the 2d sprites of the particles.
  csSprite2D **part_2d;
  /// Self destruct and when.
  bool self_destruct;
  time_t time_to_live; // msec
  /// If this system should be deleted.
  bool to_delete;

public:
  /// Make a new system, with maximum number of particles inside it.
  csParticleSystem (int max_part);
  /// Destroy particle system, and all particles.
  virtual ~csParticleSystem ();

  /// Get the max particles this system can store.
  int GetMaxParticles () { return max_particles; }
  /// How many particles the system currently has.
  int GetNumParticles () { return num_particles; }

  /// Set selfdestruct mode on, and msec to live.
  void SetSelfDestruct (time_t t) { self_destruct=true; time_to_live = t; };
  void UnSetSelfDestruct () { self_destruct=false; }
  bool GetSelfDestruct () { return self_destruct; }
  time_t GetTimeToLive () { return time_to_live; }
  /// Whether this system should be deleted when possible.
  void SetDelete (bool b) { to_delete = b; }
  bool GetDelete () { return to_delete; }

  /// Get a particle sprite.
  csSprite2D* GetSprite2D (int idx) { return part_2d[idx]; }
  /// Add a new sprite, increases num_particles.
  void AppendSprite ();
  /// Add an rectangle shaped sprite. Pass along half w and h.
  void AppendRectSprite (float width, float height, csTextureHandle* txt);
  /// Add a n-gon with texture. with radius.
  void AppendRegularSprite (int n, float radius, csTextureHandle* txt);
  /// Set sprite mixmodes, convenience function.
  void SetMixmodes (UInt mode);
  /// Set sprite owner, convenience function.
  void SetOwner (csObject* owner); 
  /// Set sprite lighting, convenience function.
  void SetLighting (bool b);
  /// Move all sprites to a sector, virtual so subclass can move more.
  virtual void MoveToSector (csSector *sector);

  /**
   * Update the state of the particles as time has passed.
   * i.e. move the particles, retexture, recolor ...
   * this member function will set to_delete if self_destruct is
   * enabled and time is up.
   */
  virtual void Update(time_t elapsed_time);

  // Here follow some static members and functions to help managing.
protected:
  static csParticleSystem *first;
  csParticleSystem *next;
public:
  /// Update all particle systems. deletes systems if they SetDelete.
  static void UpdateAll (time_t elapsed_time);
  /// Delete all systems.
  static void DeleteAll ();

  CSOBJTYPE;
};



/**
 * This class has a set of particles that behave with phsyics.
 * They each have a speed and an acceleration.
 */
class csNewtonianParticleSystem : public csParticleSystem
{
protected:
  /// Particle speed, m/s.
  csVector3 *part_speed;
  /// Particle acceleration, m/s^2.
  csVector3 *part_accel;

public:
  /// Specify max number of particles.
  csNewtonianParticleSystem (int max);
  virtual ~csNewtonianParticleSystem ();

  /// Moves the particles depending on their acceleration and speed.
  virtual void Update (time_t elapsed_time);

  /// Get and set a sprites speed. speeds are in metres/second.
  csVector3& GetSpeed (int idx) { return part_speed[idx]; }
  void SetSpeed (int idx, const csVector3& spd) { part_speed[idx] = spd; }

  /// Get and set a sprites acceleration. accelerations are in metres/second.
  csVector3& GetAccel (int idx) { return part_accel[idx]; }
  void SetAccel (int idx, const csVector3& acl) { part_speed[idx] = acl; }

  CSOBJTYPE;
};



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
  csWorld *light_world;
  csDynLight *explight;
  time_t light_fade;

public:
  /**
   * Give number of particles and center. push is speed added to all
   * sprites (i.e. speed of object being destroyed for example),
   * give a texture to use as well, they are added as rects.
   * spreading multipliers: a random number (1.0..+1.0) * spread is added.
   */
  csParSysExplosion (int number_p, const csVector3& explode_center,
  	const csVector3& push, csTextureHandle *txt,
	float spread_pos = 0.6, float spread_speed = 0.5,
	float spread_accel = 1.5);
  ///
  virtual ~csParSysExplosion ();

  /**
   * Update and light is flickered as well.
   * If SelfDestruct is enabled, and a light has been added, the sprites 
   * will be scaled smaller when the light fades.
   */
  virtual void Update (time_t elapsed_time);

  /// Move sprites and light(if any) to a sector.
  virtual void MoveToSector (csSector *sector);

  /// Get the center of the explosion.
  csVector3& GetCenter () { return center; }
  /// Explosion has a dynamic light at the center?
  bool HasLight () { return has_light; }

  /**
   * Add a light at explosion center. add msec when light starts fading,
   * which is used when time_to_live is set / SelfDestruct is used.
   */
  void AddLight (csWorld *world, csSector *sec, time_t fade = 200);
  /// Remove the light.
  void RemoveLight ();

  CSOBJTYPE;
};


#endif //CSPARTIC_H

