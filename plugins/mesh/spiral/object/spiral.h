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

#ifndef __CS_SPIRAL_H__
#define __CS_SPIRAL_H__

#include "csgeom/vector3.h"
#include "csgeom/box.h"
#include "csutil/cscolor.h"
#include "csutil/refarr.h"
#include "csplugincommon/particlesys/partgen.h"
#include "imesh/spiral.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

/**
 * This class has a set of particles that act like a spiraling
 * particle fountain.
 */
class csSpiralMeshObject : public csParticleSystem
{
protected:
  float part_time;
  float time_left; // from previous update
  csVector3 source; // x,y,z
  csVector3 part_source; // cached source: radius, height, angle
  csVector3 part_random; // radius, height, angle
  int last_reuse;
  csVector3 part_speed; // speed: radius, height, angle
  float * part_age;
  float part_width, part_height;

  void SetPosition(int index);
  int FindOldest();
  void RestartParticle (int index, float pre_move);
  void SetupObject ();

public:
  /// Constructor.
  csSpiralMeshObject (iObjectRegistry* object_reg, iMeshObjectFactory* factory);
  /// Destructor.
  virtual ~csSpiralMeshObject ();

  void SetParticleSize (float partwidth, float partheight)
  {
    part_width = partwidth;
    part_height = partheight;
  }

  void GetParticleSize (float& partwidth, float& partheight) const
  {
    partwidth = part_width;
    partheight = part_height;
  }
  
  /// Set the source.
  void SetSource (const csVector3& source);

  /// Get the source.
  const csVector3& GetSource () const { return source; }

  /// Update the particle system.
  virtual void Update (csTicks elapsed_time);

  void SetParticleTime (csTicks ttl)
  {
      part_time = ttl/1000.0;
  }
  
  csTicks GetParticleTime () const
  {
      return (csTicks)(part_time*1000);
  }
  void SetRadialSpeed (float speed)
  {
    part_speed.x = speed;
  }
  float GetRadialSpeed () const
  {
    return part_speed.x;
  }
  void SetRotationSpeed (float speed)
  {
    part_speed.z = speed;
  }
  float GetRotationSpeed () const
  {
    return part_speed.z;
  }
  void SetClimbSpeed (float speed)
  {
    part_speed.y = speed;
  }
  float GetClimbSpeed () const
  {
    return part_speed.y;
  }

  /// For iMeshObject.
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () const { return true; }

  SCF_DECLARE_IBASE_EXT (csParticleSystem);

  //------------------------- iSpiralState implementation ----------------
  class SpiralState : public iSpiralState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpiralMeshObject);
    virtual void SetParticleCount (int num)
    {
      scfParent->SetParticleCount (num);
    }
    virtual int GetParticleCount () const
    {
      return (int)scfParent->GetParticleCount();
    }
    virtual void SetParticleSize (float partwidth, float partheight)
    {
        scfParent->SetParticleSize( partwidth, partheight );
    }
    virtual void GetParticleSize (float& partwidth, float& partheight) const
    {
        scfParent->GetParticleSize(partwidth, partheight);
    }
    virtual void SetSource (const csVector3& source)
    {
      scfParent->SetSource (source);
    }
    virtual const csVector3& GetSource () const
    {
      return scfParent->GetSource();
    }
    virtual void SetParticleTime (csTicks ttl)
    {
      scfParent->SetParticleTime (ttl);
    }
    virtual csTicks GetParticleTime () const
    {
      return scfParent->GetParticleTime();
    }
    virtual void SetRadialSpeed (float speed)
    {
      scfParent->SetRadialSpeed (speed);
    }
    virtual float GetRadialSpeed () const
    {
      return scfParent->GetRadialSpeed();
    }
    virtual void SetRotationSpeed (float speed)
    {
      scfParent->SetRotationSpeed (speed);
    }
    virtual float GetRotationSpeed () const
    {
      return scfParent->GetRotationSpeed();
    }
    virtual void SetClimbSpeed (float speed)
    {
      scfParent->SetClimbSpeed (speed);
    }
    virtual float GetClimbSpeed () const
    {
      return scfParent->GetClimbSpeed();
    }
  } scfiSpiralState;
  friend class SpiralState;
};

/**
 * Factory for spiral.
 */
class csSpiralMeshObjectFactory : public iMeshObjectFactory
{
private:
  iObjectRegistry* object_reg;
  iMeshFactoryWrapper* logparent;
  iMeshObjectType* spiral_type;
  csFlags flags;

public:
  /// Constructor.
  csSpiralMeshObjectFactory (iMeshObjectType* pParent, iObjectRegistry* object_reg);

  /// Destructor.
  virtual ~csSpiralMeshObjectFactory ();

  //------------------------ iMeshObjectFactory implementation --------------
  SCF_DECLARE_IBASE;

  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> NewInstance ();
  virtual csPtr<iMeshObjectFactory> Clone () { return 0; }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual void SetMeshFactoryWrapper (iMeshFactoryWrapper* lp)
  { logparent = lp; }
  virtual iMeshFactoryWrapper* GetMeshFactoryWrapper () const
  { return logparent; }
  virtual iMeshObjectType* GetMeshObjectType () const { return spiral_type; }
  virtual iObjectModel* GetObjectModel () { return 0; }
};

/**
 * Spiral type. This is the plugin you have to use to create instances
 * of csSpiralMeshObjectFactory.
 */
class csSpiralMeshObjectType : public iMeshObjectType
{
private:
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csSpiralMeshObjectType (iBase*);

  /// Destructor.
  virtual ~csSpiralMeshObjectType ();

  virtual csPtr<iMeshObjectFactory> NewFactory ();

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSpiralMeshObjectType);
    virtual bool Initialize(iObjectRegistry* p)
    { scfParent->object_reg = p; return true; }
  } scfiComponent;
  friend struct eiComponent;
};

#endif // __CS_SPIRAL_H__
