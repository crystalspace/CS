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

#ifndef __CS_SNOW_H__
#define __CS_SNOW_H__

#include "csgeom/vector3.h"
#include "csgeom/box.h"
#include "csutil/cscolor.h"
#include "plugins/mesh/object/partgen/partgen.h"
#include "imsnow.h"

struct iMaterialWrapper;

/**
 * A snow particle system. Particles start falling down.
 * the snow swirls around a bit, otherwise much like rain.
 */
class csSnowMeshObject : public csParticleSystem
{
protected:
  csBox3 rainbox;
  csVector3 rain_dir;
  csVector3 *part_pos;
  csVector3 *part_speed;
  float swirl_amount;
  float drop_width, drop_height;
  int number;

  bool initialized;
  bool lighted_particles;

  void SetupObject ();

public:
  /**
   * creates a snow particle system.
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
  csSnowMeshObject (iSystem* system, iMeshObjectFactory* factory);
  /// Destructor.
  virtual ~csSnowMeshObject ();

  /// Set the number of particles to use.
  void SetNumberParticles (int num) { initialized = false; number = num; }
  /// Set the size of the drops.
  void SetDropSize (float dropwidth, float dropheight)
  {
    initialized = false;
    drop_width = dropwidth;
    drop_height = dropheight;
  }
  /// Set box.
  void SetBox (const csVector3& minbox, const csVector3& maxbox)
  {
    initialized = false;
    rainbox.Set (minbox, maxbox);
  }
  /// Enable or disable lighting.
  void SetLighting (bool l)
  {
    initialized = false;
    lighted_particles = l;
  }
  /// Set fall speed.
  void SetFallSpeed (const csVector3& fspeed)
  {
    initialized = false;
    rain_dir = fspeed;
  }
  /// Set swirl.
  void SetSwirl (float sw)
  {
    initialized = false;
    swirl_amount = sw;
  }

  /// Update the particle system.
  virtual void Update (cs_time elapsed_time);

  /// For iMeshObject.
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () { return false; }

  DECLARE_IBASE_EXT (csParticleSystem);

  //------------------------- iSnowState implementation ----------------
  class SnowState : public iSnowState
  {
    DECLARE_EMBEDDED_IBASE (csSnowMeshObject);
    virtual void SetNumberParticles (int num)
    {
      scfParent->SetNumberParticles (num);
    }
    virtual void SetDropSize (float dropwidth, float dropheight)
    {
      scfParent->SetDropSize (dropwidth, dropheight);
    }
    virtual void SetBox (const csVector3& minbox, const csVector3& maxbox)
    {
      scfParent->SetBox (minbox, maxbox);
    }
    virtual void SetLighting (bool l)
    {
      scfParent->SetLighting (l);
    }
    virtual void SetFallSpeed (const csVector3& fspeed)
    {
      scfParent->SetFallSpeed (fspeed);
    }
    virtual void SetSwirl (float sw)
    {
      scfParent->SetSwirl (sw);
    }
  } scfiSnowState;
  friend class SnowState;
};

/**
 * Factory for snow.
 */
class csSnowMeshObjectFactory : public iMeshObjectFactory
{
private:
  iSystem* system;

public:
  /// Constructor.
  csSnowMeshObjectFactory (iBase *pParent, iSystem* system);

  /// Destructor.
  virtual ~csSnowMeshObjectFactory ();

  //------------------------ iMeshObjectFactory implementation --------------
  DECLARE_IBASE;

  virtual iMeshObject* NewInstance ();
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () { return false; }
};
 
/**
 * Snow type. This is the plugin you have to use to create instances
 * of csSnowMeshObjectFactory.
 */
class csSnowMeshObjectType : public iMeshObjectType
{
private:
  iSystem* system;

public:
  /// Constructor.
  csSnowMeshObjectType (iBase*);

  /// Destructor.
  virtual ~csSnowMeshObjectType ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

  //------------------------ iMeshObjectType implementation --------------
  DECLARE_IBASE;

  /// Draw.
  virtual iMeshObjectFactory* NewFactory ();
};


#endif // __CS_SNOW_H__

