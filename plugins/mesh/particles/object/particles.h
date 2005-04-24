/*
    Copyright (C) 2003 by Jorrit Tyberghein, John Harger, Daniel Duhprey

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

#ifndef __CS_PARTICLES_H__
#define __CS_PARTICLES_H__

#include "csutil/array.h"
#include "csutil/cscolor.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/ref.h"
#include "csutil/weakref.h"
#include "csutil/leakguard.h"

#include "csgeom/objmodel.h"
#include "csgeom/transfrm.h"

#include "iengine/material.h"

#include "imesh/object.h"
#include "imesh/particles.h"

#include "iutil/comp.h"

#include "ivideo/graph3d.h"
#include "ivideo/rendermesh.h"

#include "csgfx/shadervar.h"
#include "csgfx/shadervarcontext.h"

/**
 * Particles type, instantiates factories which create meshes
 */
class csParticlesType : public iMeshObjectType
{
private:
  iObjectRegistry *object_reg;
  iBase* parent;

public:
  SCF_DECLARE_IBASE;

  csParticlesType (iBase* p);
  virtual ~csParticlesType ();

  csPtr<iMeshObjectFactory> NewFactory();

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csParticlesType);
    bool Initialize (iObjectRegistry* p)
    { scfParent->object_reg = p; return true; }
  } scfiComponent;
  friend struct eiComponent;
};


/**
 * The factory only stores initial settings used for creating many
 * instances of the same particle system type
 */
class csParticlesFactory : public iMeshObjectFactory
{
  friend class csParticlesObject;
private:
  iBase* parent;
  csParticlesType* particles_type;
  iObjectRegistry *object_reg;

  csWeakRef<iGraphics3D> g3d;
  csWeakRef<iShaderManager> shmgr;
  csRef<iMaterialWrapper> material;

  csParticleEmitType emit_type;
  float emit_size_1;
  float emit_size_2;
  float emit_size_3;

  csParticleForceType force_type;

  csVector3 force_direction;
  float force_range;
  csParticleFalloffType force_falloff;
  float force_cone_radius;
  csParticleFalloffType force_cone_radius_falloff;

  float force_amount;
  float particle_mass;
  float mass_variation;
  float dampener;

  bool autostart;
  bool transform_mode;

  int particles_per_second;
  int initial_particles;

  csVector3 gravity;

  float emit_time;
  float time_to_live;
  float time_variation;

  float diffusion;

  float particle_radius;

  csString physics_plugin;

  csArray<csColor4> gradient_colors;

  float loop_time;
  float base_heat;
  csColor4 constant_color;
  csParticleColorMethod color_method;
  csRef<iParticlesColorCallback> color_callback;
  csFlags flags;

public:
  CS_LEAKGUARD_DECLARE (csParticlesFactory);

  SCF_DECLARE_IBASE;

  csParticlesFactory (csParticlesType* p, iObjectRegistry* objreg);
  virtual ~csParticlesFactory ();

  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> NewInstance ();
  virtual csPtr<iMeshObjectFactory> Clone () { return 0; }
  virtual void HardTransform (const csReversibleTransform&) {}
  virtual bool SupportsHardTransform () const { return false; }
  virtual void SetLogicalParent (iBase* lp) { parent = lp; }
  virtual iBase* GetLogicalParent () const { return parent; }
  virtual iMeshObjectType* GetMeshObjectType () const { return particles_type; }
  virtual iObjectModel* GetObjectModel () { return 0; }

  void SetMaterial (iMaterialWrapper *mat)
  { material = mat; }
  void SetParticlesPerSecond (int count)
  { particles_per_second = count; }
  void SetInitialParticleCount (int count)
  { initial_particles = count; }
  void SetPointEmitType ()
  {
    emit_type = CS_PART_EMIT_SPHERE;
    emit_size_1 = 0.0015f;
    emit_size_2 = 0.001f;
  }
  void SetSphereEmitType (float outer_radius, float inner_radius)
  {
    emit_type = CS_PART_EMIT_SPHERE;
    emit_size_1 = outer_radius;
    emit_size_2 = inner_radius;
  }
  void SetPlaneEmitType (float x_size, float y_size)
  {
    emit_type = CS_PART_EMIT_PLANE;
    emit_size_1 = x_size;
    emit_size_2 = y_size;
  }
  void SetBoxEmitType (float x_size, float y_size, float z_size)
  {
    emit_type = CS_PART_EMIT_BOX;
    emit_size_1 = x_size;
    emit_size_2 = y_size;
    emit_size_3 = z_size;
  }
  void SetCylinderEmitType (float radius, float height)
  {
    emit_type = CS_PART_EMIT_CYLINDER;
    emit_size_1 = radius;
    emit_size_2 = height;
  }
  void SetRadialForceType (float range, csParticleFalloffType falloff)
  {
    force_type = CS_PART_FORCE_RADIAL;
    force_range = range;
    force_falloff = falloff;
  }
  void SetLinearForceType (const csVector3 &direction, float range,
    csParticleFalloffType falloff)
  {
    force_type = CS_PART_FORCE_LINEAR;
    force_direction = direction;
    force_range = range;
    force_falloff = falloff;
  }
  void SetConeForceType (const csVector3 &direction, float range,
    csParticleFalloffType falloff, float radius,
    csParticleFalloffType radius_falloff)
  {
    force_type = CS_PART_FORCE_CONE;
    force_direction = direction;
    force_range = range;
    force_falloff = falloff;
    force_cone_radius = radius;
    force_cone_radius_falloff = radius_falloff;
  }
  void SetForce (float force)
  { force_amount = force; }
  void SetDiffusion (float size)
  { diffusion = size; }
  void SetGravity (const csVector3 &gravity)
  { csParticlesFactory::gravity = gravity; }
  void SetEmitTime (float time)
  { emit_time = time; }
  void SetTimeToLive (float time)
  { time_to_live = time; }
  void SetTimeVariation (float variation)
  { time_variation = variation; }

  void AddColor (csColor4 color)
  {
    gradient_colors.Push(color);
  }
  void ClearColors ()
  { gradient_colors.DeleteAll (); }
  void SetConstantColorMethod (csColor4 color)
  {
    color_method = CS_PART_COLOR_CONSTANT;
    constant_color = color;
  }
  void SetLinearColorMethod ()
  { color_method = CS_PART_COLOR_LINEAR; }
  void SetLoopingColorMethod (float seconds)
  {
    color_method = CS_PART_COLOR_LOOPING;
    loop_time = seconds;
  }
  void SetHeatColorMethod (int base_temp)
  {
    color_method = CS_PART_COLOR_HEAT;
    base_heat = base_temp;
  }
  void SetColorCallback (iParticlesColorCallback* callback)
  {
    CS_ASSERT(callback != 0);
    color_method = CS_PART_COLOR_CALLBACK;
    color_callback = callback;
  }
  iParticlesColorCallback* GetColorCallback ()
  {
    return color_callback;
  }
  void SetParticleRadius (float rad)
  { particle_radius = rad; }
  int GetParticlesPerSecond ()
  { return particles_per_second; }
  int GetInitialParticleCount ()
  { return initial_particles; }
  csParticleEmitType GetEmitType ()
  { return emit_type; }
  float GetEmitSize1 ()
  { return emit_size_1; }
  float GetEmitSize2 ()
  { return emit_size_2; }
  float GetEmitSize3 ()
  { return emit_size_3; }
  csParticleForceType GetForceType ()
  { return force_type; }
  void GetFalloffType(csParticleFalloffType &force,
    csParticleFalloffType &cone)
  {
    force = force_falloff;
    cone = force_cone_radius_falloff;
  }
  float GetForceRange ()
  { return force_range; }
  void GetForceDirection (csVector3 &dir)
  { dir = force_direction; }
  float GetForceConeRadius ()
  { return force_cone_radius; }
  float GetForce ()
  { return force_amount; }
  float GetDiffusion ()
  { return diffusion; }
  void GetGravity (csVector3 &gravity)
  { gravity = csParticlesFactory::gravity; }
  float GetEmitTime ()
  { return emit_time; }
  float GetTimeToLive ()
  { return time_to_live; }
  float GetTimeVariation ()
  { return time_variation; }
  float GetParticleRadius ()
  { return particle_radius; }
  csParticleColorMethod GetParticleColorMethod ()
  { return color_method; }
  void GetConstantColor (csColor4& color)
  { color = constant_color; }
  float GetColorLoopTime ()
  { return loop_time; }
  float GetBaseHeat ()
  { return base_heat; }
  const csArray<csColor4> &GetGradient ()
  { return gradient_colors; }
  void SetDampener (float damp)
  { dampener = damp; }
  float GetDampener ()
  { return dampener; }
  void SetMass(float mass)
  { particle_mass = mass; }
  void SetMassVariation (float variation)
  { mass_variation = variation; }
  float GetMass()
  { return particle_mass; }
  float GetMassVariation ()
  { return mass_variation; }
  void SetAutoStart (bool a)
  { autostart = a; }
  void SetTransformMode (bool transform)
  { transform_mode = transform; }
  bool GetTransformMode ()
  { return transform_mode; }
  void SetPhysicsPlugin (const char *plugin)
  { physics_plugin = plugin; }

  struct eiParticlesFactoryState : public iParticlesFactoryState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csParticlesFactory);
    virtual void SetMaterial (iMaterialWrapper *material)
    { scfParent->SetMaterial (material); }
    virtual void SetParticlesPerSecond (int count)
    { scfParent->SetParticlesPerSecond (count); }
    virtual void SetInitialParticleCount (int count)
    { scfParent->SetInitialParticleCount (count); }
    virtual void SetPointEmitType ()
    { scfParent->SetPointEmitType (); }
    virtual void SetSphereEmitType (float outer_radius, float inner_radius)
    { scfParent->SetSphereEmitType(outer_radius, inner_radius); }
    virtual void SetPlaneEmitType (float x_size, float y_size)
    { scfParent->SetPlaneEmitType (x_size, y_size); }
    virtual void SetBoxEmitType (float x_size, float y_size, float z_size)
    {scfParent->SetBoxEmitType (x_size, y_size, z_size); }
    virtual void SetCylinderEmitType (float radius, float height)
    {scfParent->SetCylinderEmitType (radius, height); }
    virtual void SetRadialForceType(float range, csParticleFalloffType falloff)
    { scfParent->SetRadialForceType(range, falloff); }
    virtual void SetLinearForceType(const csVector3 &direction, float range,
      csParticleFalloffType falloff)
    { scfParent->SetLinearForceType(direction, range, falloff); }
    virtual void SetConeForceType (const csVector3 &direction, float range,
      csParticleFalloffType falloff, float radius,
      csParticleFalloffType radius_falloff)
    { scfParent->SetConeForceType (direction, range, falloff, radius,
        radius_falloff); }
    virtual void SetForce (float force)
    { scfParent->SetForce (force); }
    virtual void SetDiffusion (float size)
    { scfParent->SetDiffusion (size); }
    virtual void SetGravity (const csVector3 &gravity)
    { scfParent->SetGravity (gravity); }
    virtual void SetEmitTime (float time)
    { scfParent->SetEmitTime (time); }
    virtual void SetTimeToLive (float time)
    { scfParent->SetTimeToLive (time); }
    virtual void SetTimeVariation (float variation)
    { scfParent->SetTimeVariation (variation); }
    virtual void AddColor (csColor4 color)
    { scfParent->AddColor (color); }
    virtual void ClearColors ()
    { scfParent->ClearColors (); }
    virtual void SetConstantColorMethod (csColor4 color)
    { scfParent->SetConstantColorMethod (color); }
    virtual void SetLinearColorMethod ()
    { scfParent->SetLinearColorMethod (); }
    virtual void SetLoopingColorMethod (float seconds)
    { scfParent->SetLoopingColorMethod (seconds); }
    virtual void SetHeatColorMethod (int base_temp)
    { scfParent->SetHeatColorMethod (base_temp); }
    virtual void SetColorCallback (iParticlesColorCallback* callback)
    { scfParent->SetColorCallback (callback); }
    virtual iParticlesColorCallback* GetColorCallback ()
    { return scfParent->GetColorCallback (); }
    virtual void SetParticleRadius (float radius)
    { scfParent->SetParticleRadius (radius); }
    virtual csParticleColorMethod GetParticleColorMethod ()
    { return scfParent->GetParticleColorMethod (); }
    virtual void GetConstantColor (csColor4& color )
    { scfParent->GetConstantColor (color); }
    virtual float GetColorLoopTime ()
    { return scfParent->GetColorLoopTime (); }
    virtual const csArray<csColor4> &GetGradient ()
    { return scfParent->GetGradient (); }
    virtual float GetBaseHeat ()
    { return scfParent->GetBaseHeat (); }
    virtual int GetParticlesPerSecond ()
    { return scfParent->GetParticlesPerSecond (); }
    virtual int GetInitialParticleCount ()
    { return scfParent->GetInitialParticleCount (); }
    virtual csParticleEmitType GetEmitType ()
    { return scfParent->GetEmitType (); }
    virtual float GetSphereEmitInnerRadius ()
    { return scfParent->GetEmitSize2 (); }
    virtual float GetSphereEmitOuterRadius ()
    { return scfParent->GetEmitSize1 (); }
    virtual float GetEmitXSize ()
    { return scfParent->GetEmitSize1 (); }
    virtual float GetEmitYSize ()
    { return scfParent->GetEmitSize2 (); }
    virtual float GetEmitZSize ()
    { return scfParent->GetEmitSize3 (); }
    virtual csParticleForceType GetForceType ()
    { return scfParent->GetForceType (); }
    virtual void GetFalloffType(csParticleFalloffType &force,
      csParticleFalloffType &cone)
    { scfParent->GetFalloffType (force, cone); }
    virtual float GetForceRange ()
    { return scfParent->GetForceRange (); }
    virtual void GetForceDirection (csVector3 &dir)
    { scfParent->GetForceDirection (dir); }
    virtual float GetForceConeRadius ()
    { return scfParent->GetForceConeRadius (); }
    virtual float GetForce ()
    { return scfParent->GetForce (); }
    virtual float GetDiffusion ()
    { return scfParent->GetDiffusion (); }
    virtual void GetGravity (csVector3 &gravity)
    { scfParent->GetGravity (gravity); }
    virtual float GetEmitTime ()
    { return scfParent->GetEmitTime (); }
    virtual float GetTimeToLive ()
    { return scfParent->GetTimeToLive (); }
    virtual float GetTimeVariation ()
    { return scfParent->GetTimeVariation (); }
    virtual float GetParticleRadius ()
    { return scfParent->GetParticleRadius (); }
    virtual void SetDampener (float damp)
    { scfParent->SetDampener (damp); }
    virtual float GetDampener ()
    { return scfParent->GetDampener (); }
    virtual void SetMass(float mass)
    { scfParent->SetMass (mass); }
    virtual void SetMassVariation (float variation)
    { scfParent->SetMassVariation (variation); }
    virtual float GetMass()
    { return scfParent->GetMass (); }
    virtual void SetAutoStart (bool autostart)
    { scfParent->SetAutoStart (autostart); }
    virtual void SetTransformMode (bool transform)
    { scfParent->SetTransformMode (transform); }
    virtual bool GetTransformMode ()
    { return scfParent->GetTransformMode (); }
    virtual float GetMassVariation ()
    { return scfParent->GetMassVariation (); }
    virtual void SetPhysicsPlugin (const char *plugin)
    { scfParent->SetPhysicsPlugin (plugin); }
  } scfiParticlesFactoryState;
  friend struct eiParticlesFactoryState;
};


/**
 * Particles object instance
 */
class csParticlesObject : public iMeshObject
{
private:
  iBase* logparent;
  csParticlesFactory* pFactory;
  iMeshObjectDrawCallback* vis_cb;
  csRef<csShaderVariableContext> svcontext;
  csRef<csRenderBufferHolder> bufferHolder;
  csRef<iParticlesPhysics> physics;

  csRef<iMaterialWrapper> matwrap;
  csRenderMesh *mesh;
  csRenderMesh **meshpp;
  int meshppsize;

  csReversibleTransform tr_o2c;
  csMatrix3 rotation_matrix;
  int tricount;

  csStringID vertex_name;
  csStringID color_name;
  csStringID texcoord_name;
  csStringID index_name;
  csStringID radius_name;
  csStringID scale_name;

  int camera_fov;
  int camera_pixels;

  csColor basecolor;

  csParticleEmitType emit_type;
  float emit_size_1;
  float emit_size_2;
  float emit_size_3;

  csParticleForceType force_type;

  csVector3 force_direction;
  float force_range;
  csParticleFalloffType force_falloff;
  float force_cone_radius;
  csParticleFalloffType force_cone_radius_falloff;

  float force_amount;

  int particles_per_second;
  int initial_particles;

  csVector3 gravity;

  float emit_time;
  float time_to_live;
  float time_variation;

  float particle_mass;
  float mass_variation;
  float dampener;

  bool autostart;
  bool running;
  bool transform_mode;

  float diffusion;

  float particle_radius;
  bool radius_changed;

  csArray<csColor4> gradient_colors;
  float loop_time;
  float base_heat;
  csColor4 constant_color;
  csParticleColorMethod color_method;
  csRef<iParticlesColorCallback> color_callback;

  const csArray<csParticlesData> *point_data;
  struct i_vertex
  {
    csVector3 position;
    csVector4 color;
  };
  csDirtyAccessArray<i_vertex> vertex_data;

  size_t buffer_length;

  bool point_sprites;

  csRef<iRenderBuffer> masterBuffer;
  csRef<iRenderBuffer> vertex_buffer;
  csRef<iRenderBuffer> color_buffer;
  csRef<iRenderBuffer> texcoord_buffer;
  csRef<iRenderBuffer> index_buffer;

  csVector3 corners[4];

  csRandomGen rng;
  csVector3 emitter;
  float radius;

  csFlags flags;

public:
  CS_LEAKGUARD_DECLARE (csParticlesObject);

  SCF_DECLARE_IBASE;

  csParticlesObject (csParticlesFactory* f);
  virtual ~csParticlesObject ();

  /// Returns a point to the factory that made this
  iMeshObjectFactory* GetFactory () const
  { return (iMeshObjectFactory*)pFactory; }

  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> Clone ();

  /// Updates the lighting
  void UpdateLighting (iLight** lights, int num_lights, iMovable* movable);

  /// Returns the mesh, ready for rendering
  csRenderMesh** GetRenderMeshes (int& n, iRenderView* rview,
    iMovable* movable, uint32 frustum_mask);

  void SetVisibleCallback (iMeshObjectDrawCallback* cb) { vis_cb = cb; }
  iMeshObjectDrawCallback* GetVisibleCallback () const { return vis_cb; }

  /// For creating the quads when necessary
  void NextFrame (csTicks ticks, const csVector3&);

  /// Unsupported
  void HardTransform (const csReversibleTransform&) {}

  /// Shows that HardTransform is not supported by this mesh
  bool SupportsHardTransform () const { return false; }

  /// Check if hit by the beam
  bool HitBeamOutline (const csVector3& start, const csVector3& end,
	csVector3& isect, float* pr);
  /// Find exact position of a beam hit
  bool HitBeamObject (const csVector3& start, const csVector3& end,
	csVector3& isect, float* pr, int* polygon_idx = 0);

  /// Set/Get logical parent
  void SetLogicalParent (iBase* lp) { logparent = lp; }
  iBase* GetLogicalParent () const { return logparent; }

  /// Gets the object model
  iObjectModel *GetObjectModel () { return &scfiObjectModel; }

  /// Set the constant base color
  bool SetColor (const csColor& c) { basecolor = c; return true; }
  /// Get the constant base color
  bool GetColor (csColor &c) const { c = basecolor; return true; }

  iRenderBuffer *GetRenderBuffer (csRenderBufferName name);

  /// Set the material wrapper
  bool SetMaterialWrapper (iMaterialWrapper* m)
  { matwrap = m; return true; }
  /// Get the material wrapper
  iMaterialWrapper* GetMaterialWrapper () const { return matwrap; }
  void InvalidateMaterialHandles () {}

  void GetObjectBoundingBox (csBox3& bbox);
  void SetObjectBoundingBox (const csBox3& bbox);
  void GetRadius (csVector3& rad, csVector3& c);

  bool LoadPhysicsPlugin (const char *plugin_id);

  void SetParticlesPerSecond (int count)
  { particles_per_second = count; }
  void SetInitialParticleCount (int count)
  { initial_particles = count; }
  void SetPointEmitType ()
  {
    emit_type = CS_PART_EMIT_SPHERE;
    emit_size_1 = 0.0015f;
    emit_size_2 = 0.001f;
  }
  void SetSphereEmitType (float outer_radius, float inner_radius)
  {
    emit_type = CS_PART_EMIT_SPHERE;
    emit_size_1 = outer_radius;
    emit_size_2 = inner_radius;
  }
  void SetPlaneEmitType (float x_size, float y_size)
  {
    emit_type = CS_PART_EMIT_PLANE;
    emit_size_1 = x_size;
    emit_size_2 = y_size;
  }
  void SetBoxEmitType (float x_size, float y_size, float z_size)
  {
    emit_type = CS_PART_EMIT_BOX;
    emit_size_1 = x_size;
    emit_size_2 = y_size;
    emit_size_3 = z_size;
  }
  void SetCylinderEmitType (float radius, float height)
  {
    emit_type = CS_PART_EMIT_CYLINDER;
    emit_size_1 = radius;
    emit_size_2 = height;
  }
  void SetRadialForceType (float range, csParticleFalloffType falloff)
  {
    force_type = CS_PART_FORCE_RADIAL;
    force_range = range;
    force_falloff = falloff;
  }
  void SetLinearForceType (const csVector3 &direction, float range,
    csParticleFalloffType falloff)
  {
    force_type = CS_PART_FORCE_LINEAR;
    force_direction = direction;
    force_range = range;
    force_falloff = falloff;
  }
  void SetConeForceType (const csVector3 &direction, float range,
    csParticleFalloffType falloff, float radius,
    csParticleFalloffType radius_falloff)
  {
    force_type = CS_PART_FORCE_CONE;
    force_direction = direction;
    force_range = range;
    force_falloff = falloff;
    force_cone_radius = radius;
    force_cone_radius_falloff = radius_falloff;
  }
  void SetForce (float force)
  { force_amount = force; }
  void SetDiffusion (float size)
  { diffusion = size; }
  void SetGravity (const csVector3 &gravity)
  { csParticlesObject::gravity = gravity; }
  void SetEmitTime (float time)
  { emit_time = time; }
  void SetTimeToLive (float time)
  { time_to_live = time; }
  void SetTimeVariation (float variation)
  { time_variation = variation; }
  void AddColor (csColor4 color)
  {
    gradient_colors.Push(color);
  }
  void ClearColors ()
  {
    gradient_colors.DeleteAll ();
  }
  void SetConstantColorMethod (csColor4 color)
  {
    color_method = CS_PART_COLOR_CONSTANT;
    constant_color = color;
  }
  void SetLinearColorMethod ()
  { color_method = CS_PART_COLOR_LINEAR; }
  void SetLoopingColorMethod (float seconds)
  {
    color_method = CS_PART_COLOR_LOOPING;
    loop_time = seconds;
  }
  void SetHeatColorMethod (int base_temp)
  {
    color_method = CS_PART_COLOR_HEAT;
    base_heat = base_temp;
  }
  void SetColorCallback (iParticlesColorCallback* callback)
  {
    CS_ASSERT(callback != 0);
    color_method = CS_PART_COLOR_CALLBACK;
    color_callback = callback;
  }
  iParticlesColorCallback* GetColorCallback ()
  {
    return color_callback;
  }

  void SetParticleRadius (float rad);

  int GetParticlesPerSecond ()
  { return particles_per_second; }
  int GetInitialParticleCount ()
  { return initial_particles; }
  void GetEmitPosition (csVector3 &position)
  { position = emitter; }
  csParticleEmitType GetEmitType ()
  { return emit_type; }
  float GetEmitSize1 ()
  { return emit_size_1; }
  float GetEmitSize2 ()
  { return emit_size_2; }
  float GetEmitSize3 ()
  { return emit_size_3; }
  csParticleForceType GetForceType ()
  { return force_type; }
  void GetFalloffType(csParticleFalloffType &force,
    csParticleFalloffType &cone)
  {
    force = force_falloff;
    cone = force_cone_radius_falloff;
  }
  float GetForceRange ()
  { return force_range; }
  void GetForceDirection (csVector3 &dir)
  { dir = force_direction; }
  float GetForceConeRadius ()
  { return force_cone_radius; }
  float GetForce ()
  { return force_amount; }
  float GetDiffusion ()
  { return diffusion; }
  void GetGravity (csVector3 &gravity)
  { gravity = csParticlesObject::gravity; }
  float GetEmitTime ()
  { return emit_time; }
  float GetTimeToLive ()
  { return time_to_live; }
  float GetTimeVariation ()
  { return time_variation; }
  float GetParticleRadius ()
  { return particle_radius; }
  void SetDampener (float damp)
  { dampener = damp; }
  float GetDampener ()
  { return dampener; }
  void SetMass(float mass)
  { particle_mass = mass; }
  void SetMassVariation (float variation)
  { mass_variation = variation; }
  float GetMassVariation ()
  { return mass_variation; }
  float GetMass ()
  { return particle_mass; }
  csParticleColorMethod GetParticleColorMethod ()
  { return color_method; }
  void GetConstantColor (csColor4& color)
  { color = constant_color; }
  float GetColorLoopTime ()
  { return loop_time; }
  float GetBaseHeat ()
  { return base_heat; }
  const csArray<csColor4> &GetGradient ()
  { return gradient_colors; }
  void SetTransformMode (bool transform)
  { transform_mode = transform; }
  bool GetTransformMode ()
  { return transform_mode; }
  csReversibleTransform GetCameraTranform ()
  { return tr_o2c; }
  const csMatrix3 &GetRotation ()
  { return rotation_matrix; }

  void Start ();
  void Stop ();
  bool IsRunning ()
  { return running; }

  virtual void PositionChild (iMeshObject* child,csTicks current_time) {}

  struct eiParticlesObjectState : public iParticlesObjectState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csParticlesObject);
    virtual void SetParticlesPerSecond (int count)
    { scfParent->SetParticlesPerSecond (count); }
    virtual void SetInitialParticleCount (int count)
    { scfParent->SetInitialParticleCount (count); }
    virtual void SetPointEmitType ()
    { scfParent->SetPointEmitType (); }
    virtual void SetSphereEmitType (float outer_radius, float inner_radius)
    { scfParent->SetSphereEmitType (outer_radius, inner_radius); }
    virtual void SetPlaneEmitType (float x_size, float y_size)
    { scfParent->SetPlaneEmitType (x_size, y_size); }
    virtual void SetBoxEmitType (float x_size, float y_size, float z_size)
    {scfParent->SetBoxEmitType (x_size, y_size, z_size); }
    virtual void SetCylinderEmitType (float radius, float height)
    {scfParent->SetCylinderEmitType (radius, height); }
    virtual void SetRadialForceType(float range, csParticleFalloffType falloff)
    { scfParent->SetRadialForceType(range, falloff); }
    virtual void SetLinearForceType(const csVector3 &direction, float range,
      csParticleFalloffType falloff)
    { scfParent->SetLinearForceType(direction, range, falloff); }
    virtual void SetConeForceType (const csVector3 &direction, float range,
      csParticleFalloffType falloff, float radius,
      csParticleFalloffType radius_falloff)
    { scfParent->SetConeForceType (direction, range, falloff, radius,
        radius_falloff); }
    virtual void SetForce (float force)
    { scfParent->SetForce (force); }
    virtual void SetDiffusion (float size)
    { scfParent->SetDiffusion (size); }
    virtual void SetGravity (const csVector3 &gravity)
    { scfParent->SetGravity (gravity); }
    virtual void SetEmitTime (float time)
    { scfParent->SetEmitTime (time); }
    virtual void SetTimeToLive (float time)
    { scfParent->SetTimeToLive (time); }
    virtual void SetTimeVariation (float variation)
    { scfParent->SetTimeVariation (variation); }
    virtual void AddColor (csColor4 color)
    { scfParent->AddColor (color); }
    virtual void ClearColors ()
    { scfParent->ClearColors (); }
    virtual void SetConstantColorMethod (csColor4 color)
    { scfParent->SetConstantColorMethod (color); }
    virtual void SetLinearColorMethod ()
    { scfParent->SetLinearColorMethod (); }
    virtual void SetLoopingColorMethod (float seconds)
    { scfParent->SetLoopingColorMethod (seconds); }
    virtual void SetHeatColorMethod (int base_temp)
    { scfParent->SetHeatColorMethod (base_temp); }
    virtual void SetColorCallback (iParticlesColorCallback* callback)
    { scfParent->SetColorCallback (callback); }
    virtual iParticlesColorCallback* GetColorCallback ()
    { return scfParent->GetColorCallback (); }
    virtual csParticleColorMethod GetParticleColorMethod ()
    { return scfParent->GetParticleColorMethod (); }
    virtual void GetConstantColor (csColor4& color)
    { scfParent->GetConstantColor (color); }
    virtual float GetColorLoopTime ()
    { return scfParent->GetColorLoopTime (); }
    virtual const csArray<csColor4> &GetGradient ()
    { return scfParent->GetGradient (); }
    virtual float GetBaseHeat ()
    { return scfParent->GetBaseHeat (); }
    virtual void SetParticleRadius (float radius)
    { scfParent->SetParticleRadius (radius); }
    virtual int GetParticlesPerSecond ()
    { return scfParent->GetParticlesPerSecond (); }
    virtual int GetInitialParticleCount ()
    { return scfParent->GetInitialParticleCount (); }
    virtual void GetEmitPosition (csVector3 &position)
    { scfParent->GetEmitPosition (position); }
    virtual csParticleEmitType GetEmitType ()
    { return scfParent->GetEmitType (); }
    virtual float GetSphereEmitInnerRadius ()
    { return scfParent->GetEmitSize2 (); }
    virtual float GetSphereEmitOuterRadius ()
    { return scfParent->GetEmitSize1 (); }
    virtual float GetEmitXSize ()
    { return scfParent->GetEmitSize1 (); }
    virtual float GetEmitYSize ()
    { return scfParent->GetEmitSize2 (); }
    virtual float GetEmitZSize ()
    { return scfParent->GetEmitSize3 (); }
    virtual csParticleForceType GetForceType ()
    { return scfParent->GetForceType (); }
    virtual void GetFalloffType(csParticleFalloffType &force,
      csParticleFalloffType &cone)
    { scfParent->GetFalloffType (force, cone); }
    virtual float GetForceRange ()
    { return scfParent->GetForceRange (); }
    virtual void GetForceDirection (csVector3 &dir)
    { scfParent->GetForceDirection (dir); }
    virtual float GetForceConeRadius ()
    { return scfParent->GetForceConeRadius (); }
    virtual float GetForce ()
    { return scfParent->GetForce (); }
    virtual float GetDiffusion ()
    { return scfParent->GetDiffusion (); }
    virtual void GetGravity (csVector3 &gravity)
    { scfParent->GetGravity (gravity); }
    virtual float GetEmitTime ()
    { return scfParent->GetEmitTime (); }
    virtual float GetTimeToLive ()
    { return scfParent->GetTimeToLive (); }
    virtual float GetTimeVariation ()
    { return scfParent->GetTimeVariation (); }
    virtual float GetParticleRadius ()
    { return scfParent->GetParticleRadius (); }
    virtual void SetDampener (float damp)
    { scfParent->SetDampener (damp); }
    virtual float GetDampener ()
    { return scfParent->GetDampener (); }
    virtual void SetMass(float mass)
    { scfParent->SetMass (mass); }
    virtual void SetMassVariation (float variation)
    { scfParent->SetMassVariation (variation); }
    virtual float GetMass()
    { return scfParent->GetMass (); }
    virtual float GetMassVariation ()
    { return scfParent->GetMassVariation (); }
    virtual void SetTransformMode (bool transform)
    { scfParent->SetTransformMode (transform); }
    virtual bool GetTransformMode ()
    { return scfParent->GetTransformMode (); }
    virtual csReversibleTransform GetObjectToCamera ()
    { return scfParent->GetCameraTranform (); }
    virtual const csMatrix3 &GetRotation ()
    { return scfParent->GetRotation (); }
    virtual void ChangePhysicsPlugin (const char *plugin)
    { scfParent->LoadPhysicsPlugin (plugin); }
    virtual void Start ()
    { scfParent->Start (); }
    virtual void Stop ()
    { scfParent->Stop (); }
    virtual bool IsRunning ()
    { return scfParent->IsRunning (); }
   
  } scfiParticlesObjectState;
  friend struct eiParticlesObjectState;

  struct eiObjectModel : public csObjectModel
  {
    SCF_DECLARE_EMBEDDED_IBASE (csParticlesObject);
    virtual void GetObjectBoundingBox (csBox3& b)
    { scfParent->GetObjectBoundingBox (b); }
    virtual void SetObjectBoundingBox (const csBox3& b)
    { scfParent->SetObjectBoundingBox (b); }
    virtual void GetRadius (csVector3& r, csVector3& c)
    { scfParent->GetRadius (r, c); }
  } scfiObjectModel;
  friend struct eiObjectModel;

  class eiRenderBufferAccessor : public iRenderBufferAccessor
  {
  private:
    csParticlesObject* parent;
  public:
    CS_LEAKGUARD_DECLARE (eiRenderBufferAccessor);
    SCF_DECLARE_IBASE;
    eiRenderBufferAccessor (csParticlesObject* parent)
    {
      SCF_CONSTRUCT_IBASE (0);
      eiRenderBufferAccessor::parent = parent;
    }
    virtual ~eiRenderBufferAccessor ()
    {
      SCF_DESTRUCT_IBASE ();
    }
    virtual void PreGetBuffer (csRenderBufferHolder* holder, csRenderBufferName buffer)
    {
      parent->PreGetBuffer (holder, buffer);
    }
  };
  csRef<eiRenderBufferAccessor> scfiRenderBufferAccessor;
  friend class eiRenderBufferAccessor;

  void PreGetBuffer (csRenderBufferHolder* holder, csRenderBufferName buffer);
};

#endif // __CS_PARTICLES_H__
