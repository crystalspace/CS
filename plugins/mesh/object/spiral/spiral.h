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
#include "plugins/mesh/object/partgen/partgen.h"
#include "imspiral.h"

/**
 * This class has a set of particles that act like a spiraling
 * particle fountain.
 */
class csSpiralMeshObject : public csNewtonianParticleSystem
{
protected:
  int max;
  int time_before_new_particle; // needs to be signed.
  csVector3 source;
  int last_reuse;

  void SetupObject ();

public:
  /// Constructor.
  csSpiralMeshObject (iSystem* system, iMeshObjectFactory* factory);
  /// Destructor.
  virtual ~csSpiralMeshObject ();

  /// Set the number of particles to use.
  void SetNumberParticles (int num) { initialized = false; max = num; }

  /// Set the source.
  void SetSource (const csVector3& source)
  {
    initialized = false;
    csSpiralMeshObject::source = source;
  }

  /// Update the particle system.
  virtual void Update (cs_time elapsed_time);

  /// For iMeshObject.
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () { return true; }

  DECLARE_IBASE_EXT (csParticleSystem);

  //------------------------- iSpiralState implementation ----------------
  class SpiralState : public iSpiralState
  {
    DECLARE_EMBEDDED_IBASE (csSpiralMeshObject);
    virtual void SetNumberParticles (int num)
    {
      scfParent->SetNumberParticles (num);
    }
    virtual void SetSource (const csVector3& source)
    {
      scfParent->SetSource (source);
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
  iSystem* system;

public:
  /// Constructor.
  csSpiralMeshObjectFactory (iBase *pParent, iSystem* system);

  /// Destructor.
  virtual ~csSpiralMeshObjectFactory ();

  //------------------------ iMeshObjectFactory implementation --------------
  DECLARE_IBASE;

  virtual iMeshObject* NewInstance ();
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () { return false; }
};
 
/**
 * Spiral type. This is the plugin you have to use to create instances
 * of csSpiralMeshObjectFactory.
 */
class csSpiralMeshObjectType : public iMeshObjectType
{
private:
  iSystem* system;

public:
  /// Constructor.
  csSpiralMeshObjectType (iBase*);

  /// Destructor.
  virtual ~csSpiralMeshObjectType ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

  //------------------------ iMeshObjectType implementation --------------
  DECLARE_IBASE;

  virtual iMeshObjectFactory* NewFactory ();
};


#endif // __CS_SPIRAL_H__

