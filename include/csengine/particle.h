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
#include "csgeom/box.h"
#include "csobject/csobject.h"
#include "csutil/cscolor.h"
#include "iparticl.h"

class csSprite2D;
class csTextureHandle;
class csWorld;
class csSector;
class csDynLight;

/**
 * This class represents a particle system. It is a set of iParticles.
 * Subclasses of this class may be of more interest to users.
 * More specialised particle systems can be found below.
 */
class csParticleSystem : public csObject, public iParticle
{
protected:
  /// iParticle ptrs to the particles.
  csVector particles;
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
  bool change_alpha; float alphapersecond; float alpha_now;
  /// Rotate particles, angle in radians.
  bool change_rotation; float anglepersecond;

public:
  /**
   * Make a new system. 
   * Also adds the particle system to the list of the current world.
   */
  csParticleSystem ();

  /**
   * Destroy particle system, and all particles.
   * Perhaps SetDelete(true) is easier, which will delete the part.sys.
   * at the next world->UpdateParticleSystems, and also remove the
   * particle system from the world list for you.
   */
  virtual ~csParticleSystem ();

  /// How many particles the system currently has.
  inline int GetNumParticles () { return particles.Length();}
  /// Get a particle.
  inline iParticle* GetParticle (int idx) 
  { return (iParticle*)particles[idx]; }

  /// Add a new particle, increases num_particles. Do a DecRef yourself.
  inline void AppendParticle (iParticle *part) 
  {particles.Push(part); part->IncRef();}
  
  /** 
   * Add an rectangle shaped csSprite2d particle. Pass along half w and h.
   * adds sprite to world list.
   */
  void AppendRectSprite (float width, float height, csTextureHandle* txt,
    bool lighted);

  /** 
   *Add a csSprite2d n-gon with texture, and given radius.
   *  adds sprite to world list.
   */
  void AppendRegularSprite (int n, float radius, csTextureHandle* txt,
    bool lighted);

  /// Set selfdestruct mode on, and msec to live.
  inline void SetSelfDestruct (time_t t) 
  { self_destruct=true; time_to_live = t; };
  /// system will no longer self destruct
  inline void UnSetSelfDestruct () { self_destruct=false; }
  /// returns whether the system will self destruct
  inline bool GetSelfDestruct () { return self_destruct; }
  /// if the system will self destruct, returns the time to live in msec.
  inline time_t GetTimeToLive () { return time_to_live; }

  /// Whether this system should be deleted when possible.
  inline void SetDelete (bool b) { to_delete = b; }
  /// Whether this system should be deleted when possible.
  inline bool GetDelete () { return to_delete; }

  /// Change color of all particles, by col per second.
  inline void SetChangeColor(const csColor& col) 
  {change_color = true; colorpersecond = col;}
  /// Stop change of color
  inline void UnsetChangeColor() {change_color=false;}

  /// Change size of all particles, by factor per second.
  inline void SetChangeSize(float factor) 
  {change_size = true; scalepersecond = factor;}
  /// Stop change of size
  inline void UnsetChangeSize() {change_size=false;}

  /// Set the alpha of particles.
  inline void SetAlpha(float alpha) 
  {alpha_now = alpha; SetMixmode( CS_FX_SETALPHA(alpha) ); }
  /// Get the probable alpha of the particles
  inline float GetAlpha() {return alpha_now;}
  /// Change alpha of all particles, by factor per second.
  inline void SetChangeAlpha(float factor) 
  {change_alpha = true; alphapersecond = factor;}
  /// Stop change of alpha
  inline void UnsetChangeAlpha() {change_alpha=false;}

  /// Change rotation of all particles, by angle in radians per second.
  inline void SetChangeRotation(float angle) 
  {change_rotation = true; anglepersecond = angle;}
  /// Stop change of rotation
  inline void UnsetChangeRotation() {change_rotation=false;}

  /**
   *  Get an iParticle for this particle system, thus you can add
   *  the particle system as particle to another particle system,
   *  making particle systems of particle systems.
   *  Do not add particle systems to themselves, you'll get infinite loops.
   */
  iParticle* GetAsParticle() {return this;}

  /// Move all particles to a sector, virtual so subclass can move more.
  virtual void MoveToSector (csSector *sector);
  /// Move all particles to this position
  virtual void SetPosition(const csVector3& pos);
  /// Move all particles by given delta.
  virtual void MovePosition(const csVector3& move);
  /// Set particle colors, convenience function.
  virtual void SetColor (const csColor& col);
  /// Add particle colors, convenience function.
  virtual void AddColor (const csColor& col);
  /// Scale all particles.
  virtual void ScaleBy(float factor);
  /// Set particle mixmodes, convenience function.
  virtual void SetMixmode (UInt mode);
  /// Rotate all particles
  virtual void Rotate(float angle);

  /**
   * Update the state of the particles as time has passed.
   * i.e. move the particles, retexture, recolor ...
   * this member function will set to_delete if self_destruct is
   * enabled and time is up.
   */
  virtual void Update(time_t elapsed_time);

  DECLARE_IBASE;
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

  /// Get a particles speed. speeds are in metres/second.
  inline csVector3& GetSpeed (int idx) { return part_speed[idx]; }
  /// Set a particles speed. speeds are in metres/second.
  inline void SetSpeed (int idx, const csVector3& spd) 
  { part_speed[idx] = spd; }

  /// Get a particles acceleration. accelerations are in metres/second.
  inline csVector3& GetAccel (int idx) { return part_accel[idx]; }
  /// Set a particles acceleration. accelerations are in metres/second.
  inline void SetAccel (int idx, const csVector3& acl) 
  { part_accel[idx] = acl; }

  CSOBJTYPE;
};

/**
 * This class has a set of particles that act like a spiraling
 * particle fountain.
 */
class csSpiralParticleSystem : public csNewtonianParticleSystem
{
protected:
  int max;
  time_t time_before_new_particle;
  csVector3 source;
  int last_reuse;
  csTextureHandle* txt;
  csSector* this_sector;

public:
  /// Specify max number of particles.
  csSpiralParticleSystem (int max, const csVector3& source,
  	csTextureHandle* txt);
  virtual ~csSpiralParticleSystem ();

  /// Moves the particles depending on their acceleration and speed.
  virtual void Update (time_t elapsed_time);

  /// Move all particles to a sector, virtual so subclass can move more.
  virtual void MoveToSector (csSector *sector);

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
  /// scaling of particles.
  bool scale_particles;
  time_t fade_particles;

public:
  /**
   * Give number of particles and center. 
   * push is speed added to all particles (i.e. speed of object being 
   * destroyed for example, or a wind pushing all particles in a direction),
   * give a texture to use as well,
   * nr_sides is the number of sides of every particle polygon.
   * part_radius is the radius of every particle,
   * spreading multipliers: a random number (1.0..+1.0) * spread is added.
   */
  csParSysExplosion (int number_p, const csVector3& explode_center,
  	const csVector3& push, csTextureHandle *txt, int nr_sides = 6,
	float part_radius = 0.25, bool lighted_particles = false,
	float spread_pos = 0.6, 
	float spread_speed = 0.5, float spread_accel = 1.5);
  ///
  virtual ~csParSysExplosion ();

  /**
   * Update and light is flickered as well. particles will be scaled.
   */
  virtual void Update (time_t elapsed_time);

  /// Move particles and light(if any) to a sector.
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

  /// Are particles scaled to nothing at end?
  bool GetFadeSprites() { return scale_particles;}
  /**
   * Set particles to be scaled to nothing starting at fade_particles msec 
   * before self-destruct.
   */
  void SetFadeSprites(time_t fade_time) 
  {scale_particles=true; fade_particles = fade_time; }

  CSOBJTYPE;
};


/**
 * A rain particle system. Particles start falling down.
 * Since speed if fixed due to air friction, this is not a NewtonianPartSys.
 */
class csRainParticleSystem : public csParticleSystem {
protected:
  csBox3 rainbox;
  csVector3 rain_dir;
  csVector3 *part_pos;

public:
  /** creates a rain particle system given parameters.
    * number : number of raindrops visible at one time
    * txt: texture of raindrops. mixmode = mixmode used.
    * lighted: the particles will be lighted if true.
    * drop_width, drop_height: size of rectangular raindrops.
    * rainbox_min and max: give the box in the world where it will rain.
    *   raindrops will start ...
    *   and when they exit this box they will disappear.
    * fall_speed: the direction and speed of the falling raindrops.
    *   You can make slanted rain this way. Although you would also want to
    *   slant the particles in that case...
    */
  csRainParticleSystem(int number, csTextureHandle* txt, UInt mixmode,
    bool lighted_particles, float drop_width, float drop_height, 
    const csVector3& rainbox_min, const csVector3& rainbox_max,
    const csVector3& fall_speed
    );
  virtual ~csRainParticleSystem();

  /**
   * Update and light is flickered as well. particles will be scaled.
   */
  virtual void Update (time_t elapsed_time);

  CSOBJTYPE;
};

#endif //CSPARTIC_H

