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

#ifndef __CS_CSPLUGINCOMMON_PARTICLESYS_PARTGEN_H__
#define __CS_CSPLUGINCOMMON_PARTICLESYS_PARTGEN_H__

/**\file
 */

#include "csextern.h"
#include "csgeom/box.h"
#include "csgeom/objmodel.h"
#include "csgeom/vector3.h"
#include "csgfx/shadervarcontext.h"
#include "cstool/rendermeshholder.h"
#include "csutil/cscolor.h"
#include "csutil/floatrand.h"
#include "csutil/refarr.h"
#include "iengine/lightmgr.h"
#include "iengine/material.h"
#include "imesh/object.h"
#include "imesh/particle.h"
#include "imesh/partsys.h"
#include "imesh/sprite2d.h"
#include "ivideo/graph3d.h"

/**\addtogroup plugincommon
 * @{ */

struct iMeshObjectFactory;
struct iMaterialWrapper;
struct iMovable;
struct iRenderView;
struct iObjectRegistry;
struct iEngine;
struct iMeshWrapper;

/**
 * This class represents a particle system. It is a set of iParticles.
 * Subclasses of this class may be of more interest to users.
 * More specialised particle systems can be found below.
 */
class CS_CSPLUGINCOMMON_EXPORT csParticleSystem : public iMeshObject
{
protected:
  iObjectRegistry* object_reg;
  iMeshObjectFactory* factory;
  iBase* logparent;
  iEngine* engine;
  csRef<iLightManager> light_mgr;
  /// Object space radius.
  csVector3 radius;
  /// iParticle ptrs to the particles.
  csRefArray<iSprite2DState> sprite2ds;
  csRefArray<iParticle> particles;
  /// Self destruct and when.
  bool self_destruct;
  csTicks time_to_live; // msec
  /// Color of all particles.
  csColor color;
  /// Material for all particles.
  csRef<iMaterialWrapper> mat;
  /// MixMode for all particles.
  uint MixMode;
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
  iMeshObjectDrawCallback* vis_cb;

  /// Pointer to a mesh object factory for 2D sprites.
  csRef<iMeshObjectFactory> spr_factory;
  /// Previous time.
  csTicks prev_time;
  float current_lod;
  uint32 current_features;

  csFlags flags;

  // Number of particles.
  size_t number;
  /// particle position
  csVector3 *part_pos;

  csRenderMeshHolderSingle rmHolder;

  csRef<csRenderBufferHolder> bufferHolder;
  size_t VertexCount;
  size_t TriangleCount;
  csVector3* vertices;
  csVector2* texels;
  csColor* colors;

  csRef<iRenderBuffer> vertex_buffer;
  csRef<iRenderBuffer> texel_buffer;
  csRef<iRenderBuffer> normal_buffer;
  csRef<iRenderBuffer> color_buffer;
  csRef<iRenderBuffer> index_buffer;

  csRef<iGraphics3D> g3d;

  /// Setup the buffers for the particles.
  void SetupBuffers (size_t part_sides);
  size_t part_sides;	// Number of vertices per particle (for buffers).

  bool initialized;
  /// Setup this object.
  virtual void SetupObject ();

  // Call if object needs changing.
  void ChangeObject ()
  {
    initialized = false;
    scfiObjectModel.ShapeChanged ();
  }

private:
  csRandomFloatGen randgen;
protected:
  /// Return vector with -1..+1 members. Varying length.
  csVector3 GetRandomDirection();
  /// Return vector with -1..+1 members. Varying length.
  csVector3 GetRandomDirection(csVector3 const& magnitude,
    csVector3 const& offset);
  /// Return vector with random position within box.
  csVector3 GetRandomPosition(csBox3 const& box);

public:
  /**
   * Make a new system.
   * Also adds the particle system to the list of the current engine.
   */
  csParticleSystem (iObjectRegistry* object_reg, iMeshObjectFactory* factory);

  /**
   * Destroy particle system, and all particles.
   */
  virtual ~csParticleSystem ();

  /// Set the number of particles to use.
  virtual void SetParticleCount (size_t num)
  {
    number = num;
    ChangeObject ();
  }
  /// Get the number of particles
  size_t GetParticleCount () const { return number; }

  void UpdateLighting (const csArray<iLight*>& lights, iMovable* movable);

  /// How many particles the system currently has.
  inline size_t GetNumParticles () const { return particles.Length();}
  /// Get a particle.
  inline iParticle* GetParticle (size_t idx) const
  { return particles[idx]; }
  /// Remove all particles.
  void RemoveParticles ();

  /// Add a new particle, increases num_particles. Do a DecRef yourself.
  inline void AppendParticle (iParticle *part, iSprite2DState* spr2d)
  {
    sprite2ds.Push (spr2d);
    particles.Push (part);
  }

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
  inline void SetSelfDestruct (csTicks t)
  { self_destruct=true; time_to_live = t; };
  /// system will no longer self destruct
  inline void UnSetSelfDestruct () { self_destruct=false; }
  /// returns whether the system will self destruct
  inline bool GetSelfDestruct () const { return self_destruct; }
  /// if the system will self destruct, returns the time to live in msec.
  inline csTicks GetTimeToLive () const { return time_to_live; }

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
  virtual void Update (csTicks elapsed_time);

  void GetObjectBoundingBox (csBox3& bbox)
  {
    SetupObject ();
    bbox = csParticleSystem::bbox;
  }
  void SetObjectBoundingBox (const csBox3& bbox)
  {
    csParticleSystem::bbox = bbox;
    scfiObjectModel.ShapeChanged ();
  }
  void GetRadius (csVector3& rad, csVector3& cent)
  {
    SetupObject ();
    rad = radius;
    cent = bbox.GetCenter();
  }

  //----------------------- iMeshObject implementation ------------------------
  SCF_DECLARE_IBASE;

  virtual iMeshObjectFactory* GetFactory () const { return factory; }
  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> Clone () { return 0; }
  virtual bool PreGetRenderMeshes (iRenderView* rview, iMovable* movable,
  	uint32 frustum_mask);
  virtual csRenderMesh** GetRenderMeshes (int& n, iRenderView* rview,
    iMovable* movable, uint32 frustum_mask);
  virtual void SetVisibleCallback (iMeshObjectDrawCallback* cb)
  {
    if (cb) cb->IncRef ();
    if (vis_cb) vis_cb->DecRef ();
    vis_cb = cb;
  }
  virtual iMeshObjectDrawCallback* GetVisibleCallback () const
  {
    return vis_cb;
  }
  virtual void NextFrame (csTicks current_time, const csVector3& /*pos*/)
  {
    csTicks elaps = 0;
    if (prev_time != 0) elaps = current_time-prev_time;
    prev_time = current_time;
    Update (elaps);
  }
  virtual int HitBeamBBox (const csVector3&, const csVector3&,
        csVector3&, float*)
  { return -1; }
  virtual bool HitBeamOutline (const csVector3&, const csVector3&,
        csVector3&, float*)
  { return false; }
  virtual bool HitBeamObject (const csVector3&, const csVector3&,
  	csVector3&, float*, int* = 0) { return false; }
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }
  virtual iBase* GetLogicalParent () const { return logparent; }

  //------------------------- iObjectModel implementation ----------------
  class ObjectModel : public csObjectModel
  {
    SCF_DECLARE_EMBEDDED_IBASE (csParticleSystem);
    virtual void GetObjectBoundingBox (csBox3& bbox)
    {
      scfParent->GetObjectBoundingBox (bbox);
    }
    virtual void SetObjectBoundingBox (const csBox3& bbox)
    {
      scfParent->SetObjectBoundingBox (bbox);
    }
    virtual void GetRadius (csVector3& rad, csVector3& cent)
    {
      scfParent->GetRadius (rad, cent);
    }
  } scfiObjectModel;
  friend class ObjectModel;

  virtual iObjectModel* GetObjectModel () { return &scfiObjectModel; }
  virtual bool SetColor (const csColor& col)
  {
    color = col;
    SetupColor ();
    return true;
  }
  virtual bool GetColor (csColor& col) const
  {
    col = color;
    return true;
  }
  virtual bool SetMaterialWrapper (iMaterialWrapper* mat)
  {
    initialized = false;
    csParticleSystem::mat = mat;
    return true;
  }
  virtual iMaterialWrapper* GetMaterialWrapper () const { return mat; }
  virtual void InvalidateMaterialHandles () { }
  /**
   * see imesh/object.h for specification. The default implementation
   * does nothing.
   */
  virtual void PositionChild (iMeshObject* child,csTicks current_time) { }

  //------------------------- iParticleState implementation ----------------
  class ParticleState : public iParticleState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csParticleSystem);
    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    {
      scfParent->initialized = false;
      scfParent->mat = material;
    }
    virtual iMaterialWrapper* GetMaterialWrapper () const
    {
      return scfParent->mat;
    }
    virtual void SetMixMode (uint mode)
    {
      scfParent->MixMode = mode;
      scfParent->SetupMixMode ();
    }
    virtual uint GetMixMode () const { return scfParent->MixMode; }
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
    virtual void SetSelfDestruct (csTicks t)
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
class CS_CSPLUGINCOMMON_EXPORT csNewtonianParticleSystem : public csParticleSystem
{
protected:
  /// Particle speed, m/s.
  csVector3 *part_speed;
  /// Particle acceleration, m/s^2.
  csVector3 *part_accel;

public:
  /// Specify max number of particles.
  csNewtonianParticleSystem (iObjectRegistry* object_reg,
  	iMeshObjectFactory* factory);
  virtual ~csNewtonianParticleSystem ();

  void SetCount (int max);

  /// Moves the particles depending on their acceleration and speed.
  virtual void Update (csTicks elapsed_time);

  /// Get a particles speed. speeds are in metres/second.
  csVector3& GetSpeed (int idx) const { return part_speed[idx]; }
  /// Set a particles speed. speeds are in metres/second.
  void SetSpeed (int idx, const csVector3& spd)
  { part_speed[idx] = spd; }

  /// Get a particles acceleration. accelerations are in metres/second.
  csVector3& GetAccel (int idx) const { return part_accel[idx]; }
  /// Set a particles acceleration. accelerations are in metres/second.
  void SetAccel (int idx, const csVector3& acl)
  { part_accel[idx] = acl; }
};

/** @} */

#endif // __CS_CSPLUGINCOMMON_PARTICLESYS_PARTGEN_H__
