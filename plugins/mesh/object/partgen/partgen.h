/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein
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

#ifndef __CS_PARTGEN_H__
#define __CS_PARTGEN_H__

#include "csgeom/vector3.h"
#include "csgeom/box.h"
#include "csutil/cscolor.h"
#include "csutil/csvector.h"
#include "ivideo/graph3d.h"
#include "imesh/object.h"
#include "imesh/partsys.h"
#include "imesh/particle.h"

struct iMeshObjectFactory;
struct iMaterialWrapper;
struct iMovable;
struct iRenderView;

/**
 * This class represents a particle system. It is a set of iParticles.
 * Subclasses of this class may be of more interest to users.
 * More specialised particle systems can be found below.
 */
class csParticleSystem : public iMeshObject
{
protected:
  iMeshObjectFactory* factory;
  /// Object space radius.
  csVector3 radius;
  /// iParticle ptrs to the particles.
  csVector particles;
  /// Self destruct and when.
  bool self_destruct;
  cs_time time_to_live; // msec
  /// If this system should be deleted.
  bool to_delete;
  /// Color of all particles.
  csColor color;
  /// Material for all particles.
  iMaterialWrapper* mat;
  /// MixMode for all particles.
  UInt MixMode;
  /// Color change
  bool change_color; csColor colorpersecond;
  /// Size change
  bool change_size; float scalepersecond;
  /// Alpha change
  bool change_alpha; float alphapersecond; float alpha_now;
  /// Rotate particles, angle in radians.
  bool change_rotation; float anglepersecond;
  /**
   * bounding box in 3d of all particles in this system.
   * the particle system subclass has to give this a reasonable value.
   * no particle may exceed the bbox. 
   */
  csBox3 bbox;
  csMeshCallback* vis_cb;
  void* vis_cbData;

  /// Pointer to a mesh object factory for 2D sprites.
  iMeshObjectFactory* spr_factory;
  /// Previous time.
  cs_time prev_time;
  long shapenr;

  bool initialized;
  /// Set up this object.
  virtual void SetupObject () = 0;

protected:
  /// Helping func. Returns vector of with -1..+1 members. Varying length!
  static csVector3& GetRandomDirection ();
  /// Helping func. Returns vector of with -1..+1 members. Varying length!
  static csVector3& GetRandomDirection (const csVector3& magnitude,
	const csVector3& offset);
  static csVector3& GetRandomPosition (const csBox3& box);

public:
  /**
   * Make a new system. 
   * Also adds the particle system to the list of the current engine.
   */
  csParticleSystem (iSystem* system, iMeshObjectFactory* factory);

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
  /// Remove all particles.
  void RemoveParticles ();

  /// Add a new particle, increases num_particles. Do a DecRef yourself.
  inline void AppendParticle (iParticle *part) 
  { particles.Push(part); part->IncRef(); }
  
  /** 
   * Add an rectangle shaped sprite2d particle. Pass along half w and h.
   * adds sprite to engine list.
   */
  void AppendRectSprite (float width, float height, iMaterialWrapper* mat,
    bool lighted);

  /** 
   * Add a sprite2d n-gon with material, and given radius.
   * adds sprite to engine list.
   */
  void AppendRegularSprite (int n, float radius, iMaterialWrapper* mat,
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
  /// see if change color is enabled, and get a copy if so.
  inline bool GetChangeColor (csColor& col) const
  { if(!change_color) return false; col = colorpersecond; return true; }

  /// Change size of all particles, by factor per second.
  inline void SetChangeSize(float factor) 
  {change_size = true; scalepersecond = factor;}
  /// Stop change of size
  inline void UnsetChangeSize() {change_size=false;}
  /// see if change size is enabled, and get the value if so.
  inline bool GetChangeSize (float& factor) const
  { if(!change_size) return false; factor = scalepersecond; return true; }

  /// Set the alpha of particles.
  inline void SetAlpha(float alpha) 
  {alpha_now = alpha; MixMode = CS_FX_SETALPHA (alpha); SetupMixMode (); }
  /// Get the probable alpha of the particles
  inline float GetAlpha() const {return alpha_now;}
  /// Change alpha of all particles, by factor per second.
  inline void SetChangeAlpha(float factor) 
  {change_alpha = true; alphapersecond = factor;}
  /// Stop change of alpha
  inline void UnsetChangeAlpha() {change_alpha=false;}
  /// see if change alpha is enabled, and get the value if so.
  inline bool GetChangeAlpha (float& factor) const
  { if(!change_alpha) return false; factor = alphapersecond; return true; }

  /// Change rotation of all particles, by angle in radians per second.
  inline void SetChangeRotation(float angle) 
  {change_rotation = true; anglepersecond = angle;}
  /// Stop change of rotation
  inline void UnsetChangeRotation() {change_rotation=false;}
  /// see if change rotation is enabled, and get the angle if so.
  inline bool GetChangeRotation (float& angle) const
  { if(!change_rotation) return false; angle = anglepersecond; return true; }

  /// Get the bounding box for this particle system.
  inline const csBox3& GetBoundingBox() const {return bbox;}

  /// Set particle colors, convenience function.
  virtual void SetupColor ();
  /// Add particle colors, convenience function.
  virtual void AddColor (const csColor& col);
  /// Scale all particles.
  virtual void ScaleBy(float factor);
  /// Set particle mixmodes, convenience function.
  virtual void SetupMixMode ();
  /// Rotate all particles
  virtual void Rotate(float angle);

  /**
   * Update the state of the particles as time has passed.
   * i.e. move the particles, retexture, recolor ...
   * this member function will set to_delete if self_destruct is
   * enabled and time is up.
   */
  virtual void Update (cs_time elapsed_time);

  //------------------------ iMeshObject implementation ------------------------
  DECLARE_IBASE;

  virtual iMeshObjectFactory* GetFactory () { return factory; }
  virtual bool DrawTest (iRenderView* rview, iMovable* movable);
  virtual void UpdateLighting (iLight** lights, int num_lights,
      	iMovable* movable);
  virtual bool Draw (iRenderView* rview, iMovable* movable, csZBufMode mode);
  virtual void SetVisibleCallback (csMeshCallback* cb, void* cbData)
  {
    vis_cb = cb;
    vis_cbData = cbData;
  }
  virtual csMeshCallback* GetVisibleCallback ()
  {
    return vis_cb;
  }
  virtual void GetObjectBoundingBox (csBox3& bbox, int type = CS_BBOX_NORMAL)
  {
    (void)type;
    SetupObject ();
    bbox = csParticleSystem::bbox;
  }
  virtual csVector3 GetRadius () { return radius; }
  virtual void NextFrame (cs_time current_time)
  {
    cs_time elaps = 0;
    if (prev_time != 0) elaps = current_time-prev_time;
    prev_time = current_time;
    Update (elaps);
  }
  virtual bool WantToDie () { return to_delete; }
  virtual bool HitBeamObject (const csVector3&, const csVector3&,
  	csVector3&, float*) { return false; }
  virtual long GetShapeNumber () { return shapenr; }

  //------------------------- iParticleState implementation ----------------
  class ParticleState : public iParticleState
  {
    DECLARE_EMBEDDED_IBASE (csParticleSystem);
    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    {
      scfParent->initialized = false;
      scfParent->mat = material;
    }
    virtual iMaterialWrapper* GetMaterialWrapper ()
    {
      return scfParent->mat;
    }
    virtual void SetMixMode (UInt mode)
    {
      scfParent->MixMode = mode;
      scfParent->SetupMixMode ();
    }
    virtual UInt GetMixMode () { return scfParent->MixMode; }
    virtual void SetColor (const csColor& color)
    {
      scfParent->color = color;
      scfParent->SetupColor ();
    }
    virtual const csColor& GetColor () const
    {
      return scfParent->color;
    }
    virtual void SetAlpha(float alpha) {scfParent->SetAlpha(alpha);}
    virtual float GetAlpha() const {return scfParent->GetAlpha ();}
    virtual void SetChangeColor (const csColor& color)
    {
      scfParent->SetChangeColor (color);
    }
    virtual void UnsetChangeColor ()
    {
      scfParent->UnsetChangeColor ();
    }
    virtual bool GetChangeColor (csColor& col) const
    { 
      return scfParent->GetChangeColor(col); }
    virtual void SetChangeSize (float factor)
    {
      scfParent->SetChangeSize (factor);
    }
    virtual void UnsetChangeSize ()
    {
      scfParent->UnsetChangeSize ();
    }
    virtual bool GetChangeSize (float& factor) const
    { 
      return scfParent->GetChangeSize(factor); 
    }
    virtual void SetChangeRotation (float angle)
    {
      scfParent->SetChangeRotation (angle);
    }
    virtual void UnsetChangeRotation ()
    {
      scfParent->UnsetChangeRotation ();
    }
    virtual bool GetChangeRotation (float& angle) const
    { 
      return scfParent->GetChangeRotation(angle); 
    }
    virtual void SetChangeAlpha (float factor)
    {
      scfParent->SetChangeAlpha (factor);
    }
    virtual void UnsetChangeAlpha ()
    {
      scfParent->UnsetChangeAlpha ();
    }
    virtual bool GetChangeAlpha (float& factor) const
    { 
      return scfParent->GetChangeAlpha(factor); 
    }
    virtual void SetSelfDestruct (cs_time t)
    {
      scfParent->SetSelfDestruct (t);
    }
    virtual void UnSetSelfDestruct ()
    {
      scfParent->UnSetSelfDestruct ();
    }
  } scfiParticleState;
  friend class ParticleState;
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
  csNewtonianParticleSystem (iSystem* system, iMeshObjectFactory* factory);
  virtual ~csNewtonianParticleSystem ();

  void SetNumber (int max);

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
};

#endif // __CS_PARTGEN_H__
