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
#include "csutil/cscolor.h"

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
  /// Color change
  bool change_color; csColor colorpersecond;
  /// Size change
  bool change_size; float scalepersecond;
  /// Alpha change
  bool change_alpha; float alphapersecond;
  /// Rotate sprites, angle in radians.
  bool change_rotation; float anglepersecond;

public:
  /**
   * Make a new system, with maximum number of particles inside it.
   * Also adds the particle system to the list of the current world.
   */
  csParticleSystem (int max_part);

  /**
   * Destroy particle system, and all particles.
   * Perhaps SetDelete(true) is easier, which will delete the part.sys.
   * at the next world->UpdateParticleSystems, and also remove the
   * particle system from the world list for you.
   */
  virtual ~csParticleSystem ();

  /// Get the max particles this system can store.
  int GetMaxParticles () { return max_particles; }
  /// How many particles the system currently has.
  int GetNumParticles () { return num_particles; }

  /// Set selfdestruct mode on, and msec to live.
  void SetSelfDestruct (time_t t) { self_destruct=true; time_to_live = t; };
  /// system will no longer self destruct
  void UnSetSelfDestruct () { self_destruct=false; }
  /// returns whether the system will self destruct
  bool GetSelfDestruct () { return self_destruct; }
  /// if the system will self destruct, returns the time to live in msec.
  time_t GetTimeToLive () { return time_to_live; }

  /// Whether this system should be deleted when possible.
  void SetDelete (bool b) { to_delete = b; }
  /// Whether this system should be deleted when possible.
  bool GetDelete () { return to_delete; }

  /// Change color of all sprites, by col per second.
  void SetChangeColor(const csColor& col) 
  {change_color = true; colorpersecond = col;}
  /// Stop change of color
  void UnsetChangeColor() {change_color=false;}

  /// Change size of all sprites, by factor per second.
  void SetChangeSize(float factor) 
  {change_size = true; scalepersecond = factor;}
  /// Stop change of size
  void UnsetChangeSize() {change_size=false;}

  /// Change alpha of all sprites, by factor per second.
  void SetChangeAlpha(float factor) 
  {change_alpha = true; alphapersecond = factor;}
  /// Stop change of alpha
  void UnsetChangeAlpha() {change_alpha=false;}

  /// Change rotation of all sprites, by angle in radians per second.
  void SetChangeRotation(float angle) 
  {change_rotation = true; anglepersecond = angle;}
  /// Stop change of rotation
  void UnsetChangeRotation() {change_rotation=false;}

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
  /// Set sprite colors, convenience function.
  void SetColors (const csColor& col);
  /// Move all sprites to a sector, virtual so subclass can move more.
  virtual void MoveToSector (csSector *sector);

  /**
   * Update the state of the particles as time has passed.
   * i.e. move the particles, retexture, recolor ...
   * this member function will set to_delete if self_destruct is
   * enabled and time is up.
   */
  virtual void Update(time_t elapsed_time);

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

  /// Get a sprites speed. speeds are in metres/second.
  csVector3& GetSpeed (int idx) { return part_speed[idx]; }
  /// Set a sprites speed. speeds are in metres/second.
  void SetSpeed (int idx, const csVector3& spd) { part_speed[idx] = spd; }

  /// Get a sprites acceleration. accelerations are in metres/second.
  csVector3& GetAccel (int idx) { return part_accel[idx]; }
  /// Set a sprites acceleration. accelerations are in metres/second.
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
  /// scaling of sprites.
  bool scale_sprites;
  time_t sprites_fade;

public:
  /**
   * Give number of particles and center. 
   * push is speed added to all sprites (i.e. speed of object being 
   * destroyed for example, or a wind pushing all particles in a direction),
   * give a texture to use as well,
   * nr_sides is the number of sides of every particle polygon.
   * part_radius is the radius of every particle,
   * spreading multipliers: a random number (1.0..+1.0) * spread is added.
   */
  csParSysExplosion (int number_p, const csVector3& explode_center,
  	const csVector3& push, csTextureHandle *txt, int nr_sides = 6,
	float part_radius = 0.25, float spread_pos = 0.6, 
	float spread_speed = 0.5, float spread_accel = 1.5);
  ///
  virtual ~csParSysExplosion ();

  /**
   * Update and light is flickered as well. sprites will be scaled.
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

  /// Are sprites scaled to nothing at end?
  bool GetFadeSprites() { return scale_sprites;}
  /**
   * Set sprites to be scaled to nothing starting at fade_sprite msec 
   * before self-destruct.
   */
  void SetFadeSprites(time_t sprite_fade_time) 
  {scale_sprites=true; sprites_fade = sprite_fade_time; }

  CSOBJTYPE;
};


#endif //CSPARTIC_H

