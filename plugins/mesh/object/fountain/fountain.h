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

#ifndef __CS_FOUNTAIN_H__
#define __CS_FOUNTAIN_H__

#include "csgeom/vector3.h"
#include "csgeom/box.h"
#include "csutil/cscolor.h"
#include "plugins/mesh/object/partgen/partgen.h"
#include "imfount.h"

struct iMaterialWrapper;

/**
 * A Fountain particle system. Each x msec n particles shoot out of a spout, 
 * falling down after that. Thus some particles may not reach the floor if
 * x is too small. If n is too small you will see not many particles.
 * Note that the 'spout' means the spot where the fountain originates.
 * Also you know that after fall_time, a particle has reached
 * sin(elevation)*speed*fall_time + accel.y*fall_time*fall_time + spot.y 
 * i.e. the world y height of the pool of the fountain.
 */
class csFountainMeshObject : public csParticleSystem
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
  int number;
  float drop_width, drop_height;
  bool initialized;
  bool lighted_particles;

  int FindOldest();
  void RestartParticle (int index, float pre_move);
  void SetupObject ();

public:
  /**
   * Creates a fountain particle system.
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
  csFountainMeshObject (iSystem* system);
  /// Destructor.
  virtual ~csFountainMeshObject ();

  /// Set the number of particles to use.
  void SetNumberParticles (int num) { initialized = false; number = num; }
  /// Set the size of the fountain drops.
  void SetDropSize (float dropwidth, float dropheight)
  {
    initialized = false;
    drop_width = dropwidth;
    drop_height = dropheight;
  }
  /// Set origin of the fountain.
  void SetOrigin (const csVector3& origin)
  {
    initialized = false;
    csFountainMeshObject::origin = origin;
  }
  /// Enable or disable lighting.
  void SetLighting (bool l)
  {
    initialized = false;
    lighted_particles = l;
  }
  /// Set acceleration.
  void SetAcceleration (const csVector3& accel)
  {
    initialized = false;
    csFountainMeshObject::accel = accel;
  }
  /// Set elevation.
  void SetElevation (float elev)
  {
    initialized = false;
    elevation = elev;
  }
  /// Set azimuth.
  void SetAzimuth (float azi)
  {
    initialized = false;
    azimuth = azi;
  }
  /// Set opening.
  void SetOpening (float open)
  {
    initialized = false;
    opening = open;
  }
  /// Set speed.
  void SetSpeed (float spd)
  {
    initialized = false;
    speed = spd;
  }
  /// Set fall time.
  void SetFallTime (float ftime)
  {
    initialized = false;
    fall_time = ftime;
  }

  /// Update the particle system.
  virtual void Update (cs_time elapsed_time);

  /// For iMeshObject.
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () { return true; }

  DECLARE_IBASE_EXT (csParticleSystem);

  //------------------------- iFountainState implementation ----------------
  class FountainState : public iFountainState
  {
    DECLARE_EMBEDDED_IBASE (csFountainMeshObject);
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
    virtual void SetAcceleration (const csVector3& accel)
    {
      scfParent->SetAcceleration (accel);
    }
    virtual void SetElevation (float elev)
    {
      scfParent->SetElevation (elev);
    }
    virtual void SetAzimuth (float azi)
    {
      scfParent->SetAzimuth (azi);
    }
    virtual void SetOpening (float open)
    {
      scfParent->SetOpening (open);
    }
    virtual void SetSpeed (float spd)
    {
      scfParent->SetSpeed (spd);
    }
    virtual void SetFallTime (float ftime)
    {
      scfParent->SetFallTime (ftime);
    }
  } scfiFountainState;
  friend class FountainState;
};

/**
 * Factory for fountains.
 */
class csFountainMeshObjectFactory : public iMeshObjectFactory
{
private:
  iSystem* system;

public:
  /// Constructor.
  csFountainMeshObjectFactory (iSystem* system);

  /// Destructor.
  virtual ~csFountainMeshObjectFactory ();

  //------------------------ iMeshObjectFactory implementation --------------
  DECLARE_IBASE;

  virtual iMeshObject* NewInstance ();
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () { return false; }
};
 
/**
 * Fountain type. This is the plugin you have to use to create instances
 * of csFountainMeshObjectFactory.
 */
class csFountainMeshObjectType : public iMeshObjectType
{
private:
  iSystem* system;

public:
  /// Constructor.
  csFountainMeshObjectType (iBase*);

  /// Destructor.
  virtual ~csFountainMeshObjectType ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

  //------------------------ iMeshObjectType implementation --------------
  DECLARE_IBASE;

  /// Draw.
  virtual iMeshObjectFactory* NewFactory ();
};


#endif // __CS_FOUNTAIN_H__

