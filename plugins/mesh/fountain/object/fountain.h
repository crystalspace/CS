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
#include "plugins/mesh/partgen/partgen.h"
#include "imesh/fountain.h"

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
  csFountainMeshObject (iSystem* system, iMeshObjectFactory* factory);
  /// Destructor.
  virtual ~csFountainMeshObject ();

  /// Set the number of particles to use.
  void SetParticleCount (int num) { initialized = false; number = num; }
  /// Get the number of particles used.
  int GetParticleCount () const { return number; }
  /// Set the size of the fountain drops.
  void SetDropSize (float dropwidth, float dropheight)
  {
    initialized = false;
    drop_width = dropwidth;
    drop_height = dropheight;
  }
  /// Get the size of the fountain drops.
  void GetDropSize (float& dropwidth, float& dropheight) const
  {
    dropwidth = drop_width;
    dropheight = drop_height;
  }
  /// Set origin of the fountain.
  void SetOrigin (const csVector3& origin)
  {
    initialized = false;
    csFountainMeshObject::origin = origin;
  }
  /// Get origin of the fountain.
  const csVector3& GetOrigin () const { return origin; }
  /// Enable or disable lighting.
  void SetLighting (bool l)
  {
    initialized = false;
    lighted_particles = l;
  }
  /// See if lighting is enabled.
  bool GetLighting () const { return lighted_particles; }
  /// Set acceleration.
  void SetAcceleration (const csVector3& accel)
  {
    initialized = false;
    csFountainMeshObject::accel = accel;
  }
  /// Get acceleration.
  const csVector3& GetAcceleration () const { return accel; }
  /// Set elevation.
  void SetElevation (float elev)
  {
    initialized = false;
    elevation = elev;
  }
  /// Get elevation.
  float GetElevation () const { return elevation; }
  /// Set azimuth.
  void SetAzimuth (float azi)
  {
    initialized = false;
    azimuth = azi;
  }
  /// Get azimuth.
  float GetAzimuth () const { return azimuth; }
  /// Set opening.
  void SetOpening (float open)
  {
    initialized = false;
    opening = open;
  }
  /// Get opening.
  float GetOpening () const { return opening; }
  /// Set speed.
  void SetSpeed (float spd)
  {
    initialized = false;
    speed = spd;
  }
  /// Get speed.
  float GetSpeed () const { return speed; }
  /// Set fall time.
  void SetFallTime (float ftime)
  {
    initialized = false;
    fall_time = ftime;
  }
  /// Get fall time.
  float GetFallTime () const { return fall_time; }

  /// Update the particle system.
  virtual void Update (cs_time elapsed_time);

  /// For iMeshObject.
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () const { return true; }

  SCF_DECLARE_IBASE_EXT (csParticleSystem);

  //------------------------- iFountainState implementation ----------------
  class FountainState : public iFountainState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csFountainMeshObject);
    virtual void SetParticleCount (int num)
    {
      scfParent->SetParticleCount (num);
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
    virtual int GetParticleCount () const
    {
      return scfParent->GetParticleCount ();
    }
    virtual void GetDropSize (float& dropwidth, float& dropheight) const
    {
      scfParent->GetDropSize (dropwidth, dropheight);
    }
    virtual const csVector3& GetOrigin () const
    {
      return scfParent->GetOrigin ();
    }
    virtual bool GetLighting () const
    {
      return scfParent->GetLighting ();
    }
    virtual const csVector3& GetAcceleration () const
    {
      return scfParent->GetAcceleration ();
    }
    virtual float GetElevation () const
    {
      return scfParent->GetElevation ();
    }
    virtual float GetAzimuth () const
    {
      return scfParent->GetAzimuth ();
    }
    virtual float GetOpening () const
    {
      return scfParent->GetOpening ();
    }
    virtual float GetSpeed () const
    {
      return scfParent->GetSpeed ();
    }
    virtual float GetFallTime () const
    {
      return scfParent->GetFallTime ();
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
  csFountainMeshObjectFactory (iBase *pParent, iSystem* system);

  /// Destructor.
  virtual ~csFountainMeshObjectFactory ();

  //------------------------ iMeshObjectFactory implementation --------------
  SCF_DECLARE_IBASE;

  virtual iMeshObject* NewInstance ();
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
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
  SCF_DECLARE_IBASE;

  /// Draw.
  virtual iMeshObjectFactory* NewFactory ();
  virtual uint32 GetFeatures () const
  {
    return ALL_FEATURES;
  }
};


#endif // __CS_FOUNTAIN_H__

