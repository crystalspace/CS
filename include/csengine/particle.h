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
struct iMeshObjectFactory;

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

  /// Pointer to a mesh object factory for 2D sprites.
  iMeshObjectFactory* spr_factory;

protected:
  /// Update this sprite in the polygon trees.
  virtual void UpdateInPolygonTrees ();
  /// Move all particles to a sector, virtual so subclass can move more.
  virtual void MoveToSector (csSector *sector);
  /// Helping func. Returns vector of with -1..+1 members. Varying length!
  static csVector3& GetRandomDirection ();
  /// Helping func. Returns vector of with -1..+1 members. Varying length!
  static csVector3& GetRandomDirection (const csVector3& magnitude,
	const csVector3& offset);

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

#endif // __CS_PARTICLE_H__
