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

#ifndef __CS_PARTICLE_H__
#define __CS_PARTICLE_H__

#include "csgeom/vector3.h"
#include "csgeom/box.h"
#include "csobject/csobject.h"
#include "csutil/cscolor.h"
#include "csengine/cssprite.h"
#include "iparticl.h"

class csSprite2D;
class csMaterialWrapper;
class csEngine;
class csSector;
class csDynLight;
class csLight;
class csRenderView;

/**
 * This class represents a particle system. It is a set of iParticles.
 * Subclasses of this class may be of more interest to users.
 * More specialised particle systems can be found below.
 */
class csParticleSystem : public csSprite
{
protected:
  /// iParticle ptrs to the particles.
  csVector particles;
  /// Self destruct and when.
  bool self_destruct;
  cs_time time_to_live; // msec
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
  /** bounding box in 3d of all particles in this system.
   * the particle system subclass has to give this a reasonable value.
   * no particle may exceed the bbox. 
   */
  csBox3 bbox;

  /// Bounding box for polygon trees.
  csPolyTreeBBox ptree_bbox;

protected:
  /// Update this sprite in the polygon trees.
  virtual void UpdateInPolygonTrees ();
  /// Move all particles to a sector, virtual so subclass can move more.
  virtual void MoveToSector (csSector *sector);

public:
  /**
   * Make a new system. 
   * Also adds the particle system to the list of the current engine.
   */
  csParticleSystem (csObject* theParent);

  /**
   * Destroy particle system, and all particles.
   * Perhaps SetDelete(true) is easier, which will delete the part.sys.
   * at the next engine->UpdateParticleSystems, and also remove the
   * particle system from the engine list for you.
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
   * adds sprite to engine list.
   */
  void AppendRectSprite (float width, float height, csMaterialWrapper* mat,
    bool lighted);

  /** 
   * Add a csSprite2d n-gon with material, and given radius.
   * adds sprite to engine list.
   */
  void AppendRegularSprite (int n, float radius, csMaterialWrapper* mat,
    bool lighted);

  /// Set selfdestruct mode on, and msec to live.
  inline void SetSelfDestruct (cs_time t) 
  { self_destruct=true; time_to_live = t; };
  /// system will no longer self destruct
  inline void UnSetSelfDestruct () { self_destruct=false; }
  /// returns whether the system will self destruct
  inline bool GetSelfDestruct () { return self_destruct; }
  /// if the system will self destruct, returns the time to live in msec.
  inline cs_time GetTimeToLive () { return time_to_live; }

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

  /// Get the bounding box for this particle system.
  inline const csBox3& GetBoundingBox() const {return bbox;}

  /**
   * Get an iParticle for this particle system, thus you can add
   * the particle system as particle to another particle system,
   * making particle systems of particle systems.
   * Do not add particle systems to themselves, you'll get infinite loops.
   */
  iParticle* GetAsParticle() { return &scfiParticle; }

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
  virtual void Update (cs_time elapsed_time);

  /// Draw the particle system.
  virtual void Draw (csRenderView& rview);

  /// Light part sys according to the given array of lights.
  virtual void UpdateLighting (csLight** lights, int num_lights);

  /// Get the location of the part sys.
  virtual const csVector3& GetPosition () const;

  /// Update lighting as soon as the part sys becomes visible.
  virtual void DeferUpdateLighting (int flags, int num_lights);

  /**
   * Check if this sprite is hit by this object space vector.
   * Return the collision point in object space coordinates.
   * @@@ TO BE IMPLEMENTED!
   */
  virtual bool HitBeamObject (const csVector3& /*start*/,
    const csVector3& /*end*/, csVector3& /*isect*/, float* /*pr*/)
    { return false; }

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
  csNewtonianParticleSystem (csObject* theParent, int max);
  virtual ~csNewtonianParticleSystem ();

  /// Moves the particles depending on their acceleration and speed.
  virtual void Update (cs_time elapsed_time);

  /// Get a particles speed. speeds are in metres/second.
  csVector3& GetSpeed (int idx) { return part_speed[idx]; }
  /// Set a particles speed. speeds are in metres/second.
  void SetSpeed (int idx, const csVector3& spd) 
  { part_speed[idx] = spd; }

  /// Get a particles acceleration. accelerations are in metres/second.
  csVector3& GetAccel (int idx) { return part_accel[idx]; }
  /// Set a particles acceleration. accelerations are in metres/second.
  void SetAccel (int idx, const csVector3& acl) 
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
  int time_before_new_particle; // needs to be signed.
  csVector3 source;
  int last_reuse;
  csMaterialWrapper* mat;
  csSector* this_sector;

  /// Move all particles to a sector, virtual so subclass can move more.
  virtual void MoveToSector (csSector *sector);

public:
  /// Specify max number of particles.
  csSpiralParticleSystem (csObject* theParent, int max,
    const csVector3& source, csMaterialWrapper* mat);
  /// Destructor.
  virtual ~csSpiralParticleSystem ();

  /// Moves the particles depending on their acceleration and speed.
  virtual void Update (cs_time elapsed_time);

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


/**
 * A rain particle system. Particles start falling down.
 * Since speed if fixed due to air friction, this is not a NewtonianPartSys.
 */
class csRainParticleSystem : public csParticleSystem
{
protected:
  csBox3 rainbox;
  csVector3 rain_dir;
  csVector3 *part_pos;

public:
  /**
   * creates a rain particle system given parameters.
   * number : number of raindrops visible at one time
   * mat: material of raindrops. mixmode = mixmode used.
   * lighted: the particles will be lighted if true.
   * drop_width, drop_height: size of rectangular raindrops.
   * rainbox_min and max: give the box in the world where it will rain.
   *   raindrops will start ...
   *   and when they exit this box they will disappear.
   * fall_speed: the direction and speed of the falling raindrops.
   *   You can make slanted rain this way. Although you would also want to
   *   slant the particles in that case...
   */
  csRainParticleSystem(csObject* theParent, int number,
    csMaterialWrapper* mat, UInt mixmode,
    bool lighted_particles, float drop_width, float drop_height, 
    const csVector3& rainbox_min, const csVector3& rainbox_max,
    const csVector3& fall_speed
    );
  /// Destructor.
  virtual ~csRainParticleSystem();

  /// Update and light is flickered as well. particles will be scaled.
  virtual void Update (cs_time elapsed_time);

  CSOBJTYPE;
};


/**
 * A snow particle system. Particles start falling down.
 * the snow swirls around a bit, otherwise much like rain.
 */
class csSnowParticleSystem : public csParticleSystem
{
protected:
  csBox3 rainbox;
  csVector3 rain_dir;
  csVector3 *part_pos;
  csVector3 *part_speed;
  float swirl_amount;

public:
  /**
   * creates a snow particle system given parameters.
   * number : number of raindrops visible at one time
   * mat: material of raindrops. mixmode = mixmode used.
   * lighted: the particles will be lighted if true.
   * drop_width, drop_height: size of rectangular raindrops.
   * rainbox_min and max: give the box in the world where it will snow.
   *   snow flakes will start ...
   *   and when they exit this box they will disappear.
   * fall_speed: the direction and speed of the falling snow flakes.
   *   You can make slanted slow this way. Although you would also want to
   *   slant the particles in that case...
   * swirl: is the amount of swirl for a flake, 0.0 is like rain.
   */
  csSnowParticleSystem(csObject* theParent, int number,
    csMaterialWrapper* mat, UInt mixmode,
    bool lighted_particles, float drop_width, float drop_height, 
    const csVector3& rainbox_min, const csVector3& rainbox_max,
    const csVector3& fall_speed, float swirl
    );
  /// Destructor.
  virtual ~csSnowParticleSystem();

  /// Update the particle system.
  virtual void Update (cs_time elapsed_time);

  CSOBJTYPE;
};


/**
 * A Fountain particle system. Each x msec n particles shoot out of a spout, 
 * falling down after that. Thus some particles may not reach the floor if
 * x is too small. If n is too small you will see not many particles.
 * Note that the 'spout' means the spot where the fountain originates.
 * Also you know that after fall_time, a particle has reached
 * sin(elevation)*speed*fall_time + accel.y*fall_time*fall_time + spot.y 
 * i.e. the world y height of the pool of the fountain.
 */
class csFountainParticleSystem : public csParticleSystem
{
protected:
  int amt;
  csVector3 origin;
  csVector3 accel;
  csVector3* part_pos;
  csVector3* part_speed;
  float *part_age;
  float speed, opening, azimuth, elevation, fall_time;
  float time_left; // from previous update
  int next_oldest;

  int FindOldest();
  void RestartParticle(int index, float pre_move);

public:
  /**
   * creates a fountain particle system given parameters.
   * number : number of raindrops visible at one time
   * mat: material of raindrops. mixmode = mixmode used.
   * lighted: the particles will be lighted if true.
   * drop_width, drop_height: size of rectangular waterdrops.
   * spot is the origin of the fountain
   * accel is the particle acceleration, in m/s^2, the gravity.
   * fall_time is the seconds a particle gets to fall.
   * speed in m/s of the drops on exiting the spout.
   * opening is the angle controlling the width of the stream.
   * azimuth is the angle of the direction (horizontally) of the stream.
   * elevation is the angle of the direction (up/down) of the stream.
   *   angles in radians (2*PI is 360 degrees)
   */
  csFountainParticleSystem(csObject* theParent, int number, 
    csMaterialWrapper* mat, UInt mixmode,
    bool lighted_particles, float drop_width, float drop_height, 
    const csVector3& spot, const csVector3& accel, float fall_time,
    float speed, float opening, float azimuth, float elevation
    );
  /// Destructor.
  virtual ~csFountainParticleSystem();

  /// Update the particle system.
  virtual void Update (cs_time elapsed_time);

  CSOBJTYPE;
};

/**
 * A Fire particle system. Each x msec n particles shoot out of the fire, 
 */
class csFireParticleSystem : public csParticleSystem
{
protected:
  int amt;
  csVector3 direction;
  csVector3 origin;
  float swirl;
  float color_scale;
  csVector3* part_pos;
  csVector3* part_speed;
  float *part_age;
  float total_time;
  float time_left; // from previous update
  int next_oldest;

  csLight *light;
  int light_time;
  bool delete_light;
  csEngine *light_engine;

  int FindOldest();
  void RestartParticle(int index, float pre_move);
  void MoveAndAge(int index, float delta_t);

public:
  /**
   * creates a fire particle system given parameters.
   * number : number of raindrops visible at one time
   * mat: material of raindrops. mixmode = mixmode used.
   * lighted: the particles will be lighted if true.
   * drop_width, drop_height: size of rectangular particles.
   * total_time is the seconds a particle gets to burn.
   * dir is direction of fire.
   * origin is the starting point of the flame
   * swirl is the amount of swirling of particles.
   * color_scale scales the colour the particles are set to.
   */
  csFireParticleSystem(csObject* theParent, int number, 
    csMaterialWrapper* mat, UInt mixmode,
    bool lighted_particles, float drop_width, float drop_height, 
    float total_time, const csVector3& dir, const csVector3& origin,
    float swirl, float color_scale
    );
  /// Destructor.
  virtual ~csFireParticleSystem();

  /// Update the particle system.
  virtual void Update (cs_time elapsed_time);

  /// You can set a pseudo-static light here
  void SetControlledLight(csLight *l) {light = l;}
  /**
   * Add a new dynamic light (no need to call SetControlledLight). 
   * NB Will not move upon SetSector.
   */
  void AddLight(csEngine*, csSector*);

  CSOBJTYPE;
};

#endif // __CS_PARTICLE_H__
