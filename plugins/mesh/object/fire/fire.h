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

#ifndef __CS_FIRE_H__
#define __CS_FIRE_H__

#include "csgeom/vector3.h"
#include "csgeom/box.h"
#include "csutil/cscolor.h"
#include "plugins/mesh/object/partgen/partgen.h"
#include "imfire.h"

struct iMaterialWrapper;
struct iLight;
struct iDynLight;
struct iEngine;
struct iSector;

/**
 * A Fire particle system. Each x msec n particles shoot out of the fire, 
 */
class csFireMeshObject : public csParticleSystem
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

  int number;
  float drop_width, drop_height;
  bool initialized;
  bool lighted_particles;

  iLight* light;
  iDynLight* dynlight;
  int light_time;
  bool delete_light;
  iEngine* light_engine;

  int FindOldest ();
  void RestartParticle (int index, float pre_move);
  void MoveAndAge (int index, float delta_t);

  void SetupObject ();

public:
  /**
   * creates a fire particle system.
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
  csFireMeshObject (iSystem* system);
  /// Destructor.
  virtual ~csFireMeshObject ();

  /// You can set a pseudo-static light here
  void SetControlledLight (iLight *l);
  /**
   * Add a new dynamic light (no need to call SetControlledLight). 
   */
  void AddLight (iEngine*, iSector*);

  /// Set the number of particles to use.
  void SetNumberParticles (int num) { initialized = false; number = num; }
  /// Set the size of the fountain drops.
  void SetDropSize (float dropwidth, float dropheight)
  {
    initialized = false;
    drop_width = dropwidth;
    drop_height = dropheight;
  }
  /// Set origin of the fire.
  void SetOrigin (const csVector3& origin)
  {
    initialized = false;
    csFireMeshObject::origin = origin;
  }
  /// Set direction of the fire.
  void SetDirection (const csVector3& direction)
  {
    initialized = false;
    csFireMeshObject::direction = direction;
  }
  /// Enable or disable lighting.
  void SetLighting (bool l)
  {
    initialized = false;
    lighted_particles = l;
  }
  /// Set swirl.
  void SetSwirl (float swirl)
  {
    initialized = false;
    csFireMeshObject::swirl = swirl;
  }
  /// Set color scale.
  void SetColorScale (float colscale)
  {
    initialized = false;
    color_scale = colscale;
  }
  /// Set total time.
  void SetTotalTime (float tottime)
  {
    initialized = false;
    total_time = tottime;
  }

  /// Update the particle system.
  virtual void Update (cs_time elapsed_time);

  DECLARE_IBASE_EXT (csParticleSystem);

  //------------------------- iFireState implementation ----------------
  class FireState : public iFireState
  {
    DECLARE_EMBEDDED_IBASE (csFireMeshObject);
    virtual void SetNumberParticles (int num)
    {
      scfParent->SetNumberParticles (num);
    }
    virtual void SetDropSize (float dropwidth, float dropheight)
    {
      scfParent->SetDropSize (dropwidth, dropheight);
    }
    virtual void SetOrigin (const csVector3& origin)
    {
      scfParent->SetOrigin (origin);
    }
    virtual void SetLighting (bool l)
    {
      scfParent->SetLighting (l);
    }
    virtual void SetDirection (const csVector3& dir)
    {
      scfParent->SetDirection (dir);
    }
    virtual void SetSwirl (float swirl)
    {
      scfParent->SetSwirl (swirl);
    }
    virtual void SetColorScale (float colscale)
    {
      scfParent->SetColorScale (colscale);
    }
    virtual void SetTotalTime (float ttime)
    {
      scfParent->SetTotalTime (ttime);
    }
  } scfiFireState;
  friend class FireState;
};

/**
 * Factory for fire.
 */
class csFireMeshObjectFactory : public iMeshObjectFactory
{
private:
  iSystem* system;

public:
  /// Constructor.
  csFireMeshObjectFactory (iSystem* system);

  /// Destructor.
  virtual ~csFireMeshObjectFactory ();

  //------------------------ iMeshObjectFactory implementation --------------
  DECLARE_IBASE;

  /// Draw.
  virtual iMeshObject* NewInstance ();
};
 
/**
 * Fire type. This is the plugin you have to use to create instances
 * of csFireMeshObjectFactory.
 */
class csFireMeshObjectType : public iMeshObjectType
{
private:
  iSystem* system;

public:
  /// Constructor.
  csFireMeshObjectType (iBase*);

  /// Destructor.
  virtual ~csFireMeshObjectType ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

  //------------------------ iMeshObjectType implementation --------------
  DECLARE_IBASE;

  /// Draw.
  virtual iMeshObjectFactory* NewFactory ();
};


#endif // __CS_FIRE_H__

