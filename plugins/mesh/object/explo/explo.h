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

#ifndef __CS_EXPLOSION_H__
#define __CS_EXPLOSION_H__

#include "csgeom/vector3.h"
#include "csgeom/box.h"
#include "csutil/cscolor.h"
#include "plugins/mesh/object/partgen/partgen.h"
#include "imesh/explode.h"

struct iMaterialWrapper;
struct iSector;
struct iEngine;
struct iDynLight;

/**
 * An explosive particle system.
 * particles explode outward from defined center.
 */
class csExploMeshObject : public csNewtonianParticleSystem
{
protected:
  /// Center of explosion.
  csVector3 center;
  /// Dynamic light at center.
  bool has_light;
  iSector* light_sector;
  iEngine* light_engine;
  // Both pointers represent the same light but ilight points to the
  // iLight interface.
  iDynLight* explight;
  iLight* ilight;
  cs_time light_fade;
  /// scaling of particles.
  bool scale_particles;
  cs_time fade_particles;
  /// starting bbox.
  csBox3 startbox;
  float maxspeed, maxaccel, radiusnow;
  int amt;

  csVector3 push;
  int number, nr_sides;
  float part_radius;
  bool lighted_particles;
  float spread_pos;
  float spread_accel;
  float spread_speed;

  void SetupObject ();

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
  csExploMeshObject (iSystem* system, iMeshObjectFactory* factory);
  /// Destructor.
  virtual ~csExploMeshObject ();

  /// Set the number of particles to use.
  void SetNumberParticles (int num)
  {
    initialized = false;
    number = num;
    SetNumber (number);
  }
  /// Get the number of particles
  int GetNumberParticles () const {return number;}
  /// Set the explosion center.
  void SetCenter (const csVector3& center)
  {
    initialized = false;
    csExploMeshObject::center = center;
  }
  /// Get the explosion center
  const csVector3 &GetCenter () const {return center;}
  /// Set the push vector.
  void SetPush (const csVector3& push)
  {
    initialized = false;
    csExploMeshObject::push = push;
  }
  /// Get the push vector.
  const csVector3& GetPush () const {return push;}
  /// Set the number of sides.
  void SetNrSides (int nr_sides)
  {
    initialized = false;
    csExploMeshObject::nr_sides = nr_sides;
  }
  /// Get the number of sides.
  int GetNrSides () const {return nr_sides;}
  /// Set the radius of all particles.
  void SetPartRadius (float part_radius)
  {
    initialized = false;
    csExploMeshObject::part_radius = part_radius;
  }
  /// Get the radius of all particles.
  float GetPartRadius () const {return part_radius;}
  /// Enable or disable lighting.
  void SetLighting (bool l)
  {
    initialized = false;
    lighted_particles = l;
  }
  /// See if lighting is enabled or disabled.
  bool GetLighting () const {return lighted_particles;}
  /// Set the spread position.
  void SetSpreadPos (float spread_pos)
  {
    initialized = false;
    csExploMeshObject::spread_pos = spread_pos;
  }
  /// Get the spread position.
  float GetSpreadPos () const {return spread_pos;}
  /// Set the spread speed.
  void SetSpreadSpeed (float spread_speed)
  {
    initialized = false;
    csExploMeshObject::spread_speed = spread_speed;
  }
  /// Get the spread speed.
  float GetSpreadSpeed () const {return spread_speed;}
  /// Set the spread acceleration.
  void SetSpreadAcceleration (float spread_accel)
  {
    initialized = false;
    csExploMeshObject::spread_accel = spread_accel;
  }
  /// Get the spread acceleration.
  float GetSpreadAcceleration () const {return spread_accel;}
  /**
   * Set particles to be scaled to nothing starting at fade_particles msec 
   * before self-destruct.
   */
  void SetFadeSprites (cs_time fade_time) 
  { scale_particles = true; fade_particles = fade_time; }
  /// See if particles are faded (returns true), and returns fade time too.
  bool GetFadeSprites (cs_time& fade_time) const
  {
    if(!scale_particles) return false;
    fade_time = fade_particles; return true;
  }

  /// Explosion has a dynamic light at the center?
  bool HasLight () const { return has_light; }
  /**
   * Add a light at explosion center. add msec when light starts fading,
   * which is used when time_to_live is set / SelfDestruct is used.
   */
  void AddLight (iEngine*, iSector*, cs_time fade = 200);
  /// Remove the light.
  void RemoveLight ();

  /// Update the particle system.
  virtual void Update (cs_time elapsed_time);

  /// For iMeshObject.
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () const { return true; }

  DECLARE_IBASE_EXT (csParticleSystem);

  //------------------------- iExplosionState implementation ----------------
  class ExplosionState : public iExplosionState
  {
    DECLARE_EMBEDDED_IBASE (csExploMeshObject);
    virtual void SetNumberParticles (int num)
    {
      scfParent->SetNumberParticles (num);
    }
    virtual int GetNumberParticles () const 
    {
      return scfParent->GetNumberParticles();
    }
    virtual void SetCenter (const csVector3& center)
    {
      scfParent->SetCenter (center);
    }
    virtual const csVector3 &GetCenter () const 
    {
      return scfParent->GetCenter ();
    }
    virtual void SetLighting (bool l)
    {
      scfParent->SetLighting (l);
    }
    virtual bool GetLighting () const 
    {
      return scfParent->GetLighting ();
    }
    virtual void SetPush (const csVector3& push)
    {
      scfParent->SetPush (push);
    }
    virtual const csVector3 &GetPush () const 
    {
      return scfParent->GetPush ();
    }
    virtual void SetNrSides (int nr_sides)
    {
      scfParent->SetNrSides (nr_sides);
    }
    virtual int GetNrSides () const 
    {
      return scfParent->GetNrSides();
    }
    virtual void SetPartRadius (float part_radius)
    {
      scfParent->SetPartRadius (part_radius);
    }
    virtual float GetPartRadius () const 
    {
      return scfParent->GetPartRadius ();
    }
    virtual void SetSpreadPos (float spread_pos)
    {
      scfParent->SetSpreadPos (spread_pos);
    }
    virtual float GetSpreadPos () const 
    {
      return scfParent->GetSpreadPos ();
    }
    virtual void SetSpreadSpeed (float spread_speed)
    {
      scfParent->SetSpreadSpeed (spread_speed);
    }
    virtual float GetSpreadSpeed () const 
    {
      return scfParent->GetSpreadSpeed ();
    }
    virtual void SetSpreadAcceleration (float spread_accel)
    {
      scfParent->SetSpreadAcceleration (spread_accel);
    }
    virtual float GetSpreadAcceleration () const 
    {
      return scfParent->GetSpreadAcceleration ();
    }
    virtual void SetFadeSprites (cs_time fade_time)
    {
      scfParent->SetFadeSprites (fade_time);
    }
    virtual bool GetFadeSprites (cs_time& fade_time) const 
    {
      return scfParent->GetFadeSprites (fade_time);
    }
    virtual void AddLight (iEngine* engine, iSector* sector, cs_time fade = 200)
    {
      scfParent->AddLight (engine, sector, fade);
    }
  } scfiExplosionState;
  friend class ExplosionState;
};

/**
 * Factory for explosions.
 */
class csExploMeshObjectFactory : public iMeshObjectFactory
{
private:
  iSystem* system;

public:
  /// Constructor.
  csExploMeshObjectFactory (iBase *pParent, iSystem* system);

  /// Destructor.
  virtual ~csExploMeshObjectFactory ();

  //------------------------ iMeshObjectFactory implementation --------------
  DECLARE_IBASE;

  virtual iMeshObject* NewInstance ();
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
};
 
/**
 * Explosion type. This is the plugin you have to use to create instances
 * of csExploMeshObjectFactory.
 */
class csExploMeshObjectType : public iMeshObjectType
{
private:
  iSystem* system;

public:
  /// Constructor.
  csExploMeshObjectType (iBase*);

  /// Destructor.
  virtual ~csExploMeshObjectType ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

  //------------------------ iMeshObjectType implementation --------------
  DECLARE_IBASE;

  /// Draw.
  virtual iMeshObjectFactory* NewFactory ();
};


#endif // __CS_EXPLOSION_H__

