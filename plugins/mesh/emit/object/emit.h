/*
    Copyright (C) 2001 by W.C.A. Wijngaards

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

#ifndef __CS_EMIT_H__
#define __CS_EMIT_H__

#include "csgeom/vector3.h"
#include "csgeom/box.h"
#include "csgeom/transfrm.h"
#include "csutil/cscolor.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"
#include "imesh/emit.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iengine/movable.h"

#include "partgen.h"

struct iMaterialWrapper;
class csEmitMeshObject;

/** fixed value emitter */
class csEmitFixed :
  public scfImplementation2<csEmitFixed,
    iEmitFixed, scfFakeInterface<iEmitGen3D> >
{
private:
  csVector3 val;
public:
  csEmitFixed (iBase *parent);
  virtual ~csEmitFixed ();
  virtual void GetValue (csVector3& value, csVector3 &given);
  virtual void SetValue (const csVector3& value);
};

/** box value emitter */
class csEmitBox :
  public scfImplementation2<csEmitBox,
    iEmitBox, scfFakeInterface<iEmitGen3D> >
{
private:
  csVector3 min, max;
  csVector3 mult;
  csRandomFloatGen randgen;
public:
  csEmitBox (iBase *parent);
  virtual ~csEmitBox ();
  virtual void GetValue (csVector3& value, csVector3 &given);
  virtual void SetContent (const csVector3& min, const csVector3& max);
  virtual void GetContent (csVector3& min, csVector3& max);
};

/** Sphere value emitter */
class csEmitSphere :
  public scfImplementation2<csEmitSphere,
    iEmitSphere, scfFakeInterface<iEmitGen3D> >
{
private:
  csVector3 center;
  float min, max;
  float rand_min, rand_mult;
  csRandomFloatGen randgen;
public:
  csEmitSphere (iBase *parent);
  virtual ~csEmitSphere ();
  virtual void GetValue (csVector3& value, csVector3 &given);
  virtual void SetContent (const csVector3& center, float min, float max);
  virtual void GetContent (csVector3& center, float& min, float& max);
};

/** Cone value emitter */
class csEmitCone :
  public scfImplementation2<csEmitCone,
    iEmitCone, scfFakeInterface<iEmitGen3D> >
{
private:
  csVector3 origin;
  float elevation, azimuth, aperture, min, max;
  csRandomFloatGen randgen;
public:
  csEmitCone (iBase *parent);
  virtual ~csEmitCone ();
  virtual void GetValue (csVector3& value, csVector3 &given);
  virtual void SetContent (const csVector3& origin, float elevation,
      float azimuth, float aperture, float min, float max);
  virtual void GetContent (csVector3& origin, float& elevation,
      float& azimuth, float& aperture, float& min, float& max);
};

/** Mix value emitter */
class csEmitMix :
  public scfImplementation2<csEmitMix,
    iEmitMix, scfFakeInterface<iEmitGen3D> >
{
private:
  struct part
  {
    csRef<iEmitGen3D> emit;
    float weight;
    struct part* next;
  } *list;
  float totalweight;
  int nr;
  csRandomFloatGen randgen;
public:
  csEmitMix (iBase *parent);
  virtual ~csEmitMix ();
  virtual void GetValue (csVector3& value, csVector3 &given);
  virtual void AddEmitter (float weight, iEmitGen3D* emit);
  virtual void RemoveEmitter(int num);
  virtual float GetTotalWeight () {return totalweight;}
  virtual int GetEmitterCount () {return nr;}
  virtual void AdjustEmitterWeight(int num,float weight);
  virtual void GetContent (int num, float& weight, iEmitGen3D*& emit);
};

/** Line value emitter */
class csEmitLine :
  public scfImplementation2<csEmitLine,
    iEmitLine, scfFakeInterface<iEmitGen3D> >
{
private:
  csVector3 start, end;
  csRandomFloatGen randgen;
public:
  csEmitLine (iBase *parent);
  virtual ~csEmitLine ();
  virtual void GetValue (csVector3& value, csVector3 &given);
  virtual void SetContent (const csVector3& start, const csVector3& end);
  virtual void GetContent (csVector3& start, csVector3& end);
};

/** Cylinder value emitter */
class csEmitCylinder :
  public scfImplementation2<csEmitCylinder,
    iEmitCylinder, scfFakeInterface<iEmitGen3D> >
{
private:
  csVector3 start, end;
  float min, max;
  csRandomFloatGen randgen;
public:
  csEmitCylinder (iBase *parent);
  virtual ~csEmitCylinder ();
  virtual void GetValue (csVector3& value, csVector3 &given);
  virtual void SetContent (const csVector3& start, const csVector3& end,
      float min, float max);
  virtual void GetContent (csVector3& start, csVector3& end,
    float& min, float& max);
};

/** SphereTangent value emitter */
class csEmitSphereTangent :
  public scfImplementation2<csEmitSphereTangent,
    iEmitSphereTangent, scfFakeInterface<iEmitGen3D> >
{
private:
  csVector3 center;
  float min, max;
  csRandomFloatGen randgen;
public:
  csEmitSphereTangent (iBase *parent);
  virtual ~csEmitSphereTangent ();
  virtual void GetValue (csVector3& value, csVector3 &given);
  virtual void SetContent (const csVector3& center, float min, float max);
  virtual void GetContent (csVector3& center, float& min, float& max);
};

/** CylinderTangent value emitter */
class csEmitCylinderTangent :
  public scfImplementation2<csEmitCylinderTangent,
    iEmitCylinderTangent, scfFakeInterface<iEmitGen3D> >
{
private:
  csVector3 start, end;
  float min, max;
  csRandomFloatGen randgen;
public:
  csEmitCylinderTangent (iBase *parent);
  virtual ~csEmitCylinderTangent ();
  virtual void GetValue (csVector3& value, csVector3 &given);
  virtual void SetContent (const csVector3& start, const csVector3& end,
      float min, float max);
  virtual void GetContent (csVector3& start, csVector3& end,
      float& min, float& max);
};

/** emit ages structure */
class csEmitAge
{
public:
  /// moment of age (in msec)
  int time;
  /// color at the age
  csColor color;
  /// alpha value at the age (0=opaque, 1.=invisible)
  float alpha;
  /// swirl at the age
  float swirl;
  /// rotationspeed at the age
  float rotspeed;
  /// scale at the age
  float scale;
  /// next age
  csEmitAge *next;
};

/**
 * A generic emitter particle system.
 * The particles behave in a newtonian manner.
 * You can construct iEmitGen3D structures that will generate xyz values.
 * These values are used to set particle start position, start velocity,
 * and start acceleration.
 * Depending on the age of each particle, it can be coloured, scaled, faded...
 */
class csEmitMeshObject :
  public scfImplementationExt1<csEmitMeshObject, csParticleSystem, iEmitState>
{
protected:
  /// true if particles should be lighted
  bool lighted_particles;
  /// the start position generator
  csRef<iEmitGen3D> startpos;
  /// the start speed generator
  csRef<iEmitGen3D> startspeed;
  /// the start accel generator
  csRef<iEmitGen3D> startaccel;
  /// attractor position generator (can be 0)
  csRef<iEmitGen3D> attractor;
  /// the field speed
  csRef<iEmitGen3D> fieldspeed;
  /// the field acceleration
  csRef<iEmitGen3D> fieldaccel;
  /// attractor force
  float attractor_force;
  /// the time to live for particles in msec
  int timetolive;
  /// the aging table
  csEmitAge *aging;
  /// size of aging table
  int nr_aging_els;
  /// is using Rect Sprites as particles
  bool using_rect_sprites;
  /// size for rect sprites
  float drop_width, drop_height;
  /// shape for regular sprites
  int drop_sides; float drop_radius;
  /// the min and max container box, if enabled
  bool has_container_box; csVector3 container_min, container_max;

  /// the ages (time they lived) of the particles, in msec.
  int* ages;
  /// particle speed m/s
  csVector3 *part_speed;
  /// particle acceleration m/s*s
  csVector3 *part_accel;
  /// attractor position per particle
  csVector3 *part_attract;

  // random number generator
  csRandomFloatGen randgen;

  /// give particle i new start values
  void StartParticle (int i);
  /// age particle i elapsed msec. delta_t is elapsed/1000.
  void MoveAgeParticle (int i, int elapsed, float delta_t);

  void SetupObject ();

public:
  /// Constructor.
  csEmitMeshObject (iObjectRegistry* object_reg, iMeshObjectFactory* factory);
  /// Destructor.
  virtual ~csEmitMeshObject ();

  /// Enable or disable lighting.
  void SetLighting (bool l)
  {
    lighted_particles = l;
    ChangeObject ();
  }
  /// see if lighting is enabled
  bool GetLighting () const
  { return lighted_particles; }
  /// set ttl
  void SetParticleTime (int ttl) { timetolive = ttl; }
  /// get ttl
  int GetParticleTime () const { return timetolive; }
  /// set startposemit
  void SetStartPosEmit (iEmitGen3D *emit)
  {
    startpos = emit;
    ChangeObject ();
  }
  /// get startposemit
  iEmitGen3D* GetStartPosEmit () const { return startpos; }
  /// set startspeedemit
  void SetStartSpeedEmit (iEmitGen3D *emit)
  {
    startspeed = emit;
    ChangeObject ();
  }
  /// get startspeedemit
  iEmitGen3D* GetStartSpeedEmit () const { return startspeed; }
  /// set startaccelemit
  void SetStartAccelEmit (iEmitGen3D *emit)
  {
    startaccel = emit;
    ChangeObject ();
  }
  /// get startemit
  iEmitGen3D* GetStartAccelEmit () const { return startaccel; }
  /// set startaccelemit
  void SetAttractorEmit (iEmitGen3D *emit)
  {
    attractor = emit;
    ChangeObject ();
  }
  /// get startemit
  iEmitGen3D* GetAttractorEmit () const { return attractor; }
  /// Set attractor force
  void SetAttractorForce (float f)
  {
    attractor_force = f;
  }
  /// Get attractor force
  float GetAttractorForce () const { return attractor_force; }
  /// set field speed emitter object
  void SetFieldSpeedEmit (iEmitGen3D *emit)
  {
    fieldspeed = emit;
    ChangeObject ();
  }
  /// get field speed emitter
  iEmitGen3D* GetFieldSpeedEmit () const { return fieldspeed; }
  /// set field accel emitter object
  void SetFieldAccelEmit (iEmitGen3D *emit)
  {
    fieldaccel = emit;
    ChangeObject ();
  }
  /// get field accel emitter
  iEmitGen3D* GetFieldAccelEmit () const { return fieldaccel; }

  /// get the number of ageing moments
  int GetAgingCount () const { return nr_aging_els; }
  /// add an age
  void AddAge (int time, const csColor& color, float alpha,
        float swirl, float rotspeed, float scale);
  /// remove an aging moment
  void RemoveAge(int time, const csColor& color, float alpha,
        float swirl, float rotspeed, float scale);
  /// get aging data
  void GetAgingMoment (int i, int& time, csColor& color, float &alpha,
        float& swirl, float& rotspeed, float& scale);
  /// replace an age
  void ReplaceAge (int time, const csColor& color, float alpha,
        float swirl, float rotspeed, float scale);

  /**
   * Compensate for movement of the particle systems, to keep
   * particles sitting in the same spot, if needed.
   */
  void CompensateForTransform (const csReversibleTransform& oldtrans,
    const csReversibleTransform& newtrans);

  /// set rectangular particles
  void SetRectParticles (float w, float h)
  {
    using_rect_sprites = true;
    drop_width = w;
    drop_height = h;
    ChangeObject ();
  }
  /// set regular polygon particles
  void SetRegularParticles (int n, float radius)
  {
    using_rect_sprites = false;
    drop_sides = n;
    drop_radius = radius;
    ChangeObject ();
  }
  /// is using rects?
  bool UsingRectParticles () const { return using_rect_sprites; }
  /// get rect size
  void GetRectParticles (float &w, float &h) const
  {
    w = drop_width;
    h = drop_height;
  }
  /// get regular shape
  void GetRegularParticles (int& n, float& radius) const
  {
    n = drop_sides;
    radius = drop_radius;
  }

  /// set container box, enabled to false disables the container box
  void SetContainerBox (bool enabled, const csVector3& min,
		      const csVector3& max)
  {
    has_container_box=enabled;
    container_min=min;
    container_max=max;
    ChangeObject ();
  }
  /// get the container box coords, returns true if enabled.
  bool GetContainerBox (csVector3& min, csVector3& max) const
  {
    if (!has_container_box)
      return false;
    min=container_min;
    max=container_max;
    return has_container_box;
  }

  /// Update the particle system.
  virtual void Update (csTicks elapsed_time);

  /// For iMeshObject.
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () const { return false; }

  // Redirect some iEmitState functions to the csParticleSystem implemenation
  using csParticleSystem::SetParticleCount;
  virtual void SetParticleCount (int num)
  { csParticleSystem::SetParticleCount (num); }
  virtual size_t GetParticleCount () const
  { return csParticleSystem::GetParticleCount (); }
};

/**
 * Factory for emitter.
 */
class csEmitMeshObjectFactory :
  public scfImplementation2<csEmitMeshObjectFactory,
    iMeshObjectFactory, iEmitFactoryState>
{
private:
  iObjectRegistry* object_reg;
  iMeshFactoryWrapper* logparent;
  iMeshObjectType* emit_type;
  csFlags flags;

public:
  /// Constructor.
  csEmitMeshObjectFactory (iMeshObjectType *pParent, iObjectRegistry*);

  /// Destructor.
  virtual ~csEmitMeshObjectFactory ();

  //------------------------ iMeshObjectFactory implementation --------------
  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> NewInstance ();
  virtual csPtr<iMeshObjectFactory> Clone () { return 0; }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual void SetMeshFactoryWrapper (iMeshFactoryWrapper* lp)
  { logparent = lp; }
  virtual iMeshFactoryWrapper* GetMeshFactoryWrapper () const
  { return logparent; }
  virtual iMeshObjectType* GetMeshObjectType () const { return emit_type; }
  virtual iObjectModel* GetObjectModel () { return 0; }
  virtual bool SetMaterialWrapper (iMaterialWrapper*) { return false; }
  virtual iMaterialWrapper* GetMaterialWrapper () const { return 0; }
  virtual void SetMixMode (uint) { }
  virtual uint GetMixMode () const { return 0; }

  //------------------------- iEmitFactoryState implementation ----------------
  virtual csRef<iEmitFixed> CreateFixed ()
  { return csPtr<iEmitFixed>(new csEmitFixed (0)); }
  virtual csRef<iEmitBox> CreateBox ()
  { return csPtr<iEmitBox>(new csEmitBox (0)); }
  virtual csRef<iEmitSphere> CreateSphere ()
  { return csPtr<iEmitSphere>(new csEmitSphere (0)); }
  virtual csRef<iEmitCone> CreateCone ()
  { return csPtr<iEmitCone>(new csEmitCone (0)); }
  virtual csRef<iEmitMix> CreateMix ()
  { return csPtr<iEmitMix>(new csEmitMix (0)); }
  virtual csRef<iEmitLine> CreateLine ()
  { return csPtr<iEmitLine>(new csEmitLine (0)); }
  virtual csRef<iEmitCylinder> CreateCylinder ()
  { return csPtr<iEmitCylinder>(new csEmitCylinder (0)); }
  virtual csRef<iEmitSphereTangent> CreateSphereTangent ()
  { return csPtr<iEmitSphereTangent>(new csEmitSphereTangent (0)); }
  virtual csRef<iEmitCylinderTangent> CreateCylinderTangent ()
  { return csPtr<iEmitCylinderTangent>(new csEmitCylinderTangent (0)); }
};

/**
 * Emit type. This is the plugin you have to use to create instances
 * of csEmitMeshObjectFactory.
 */
class csEmitMeshObjectType :
  public scfImplementation2<csEmitMeshObjectType,
    iMeshObjectType, iComponent>
{
private:
  iObjectRegistry* object_reg;

public:
  /// Constructor.
  csEmitMeshObjectType (iBase*);
  /// Destructor.
  virtual ~csEmitMeshObjectType ();
  /// Draw.
  virtual csPtr<iMeshObjectFactory> NewFactory ();

  virtual bool Initialize (iObjectRegistry* p)
  { this->object_reg = p; return true; }
};

#endif // __CS_EMIT_H__
