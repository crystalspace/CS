/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef _CUBE_H_
#define _CUBE_H_

#include "imeshobj.h"
#include "imcube.h"

struct iMaterialWrapper;
class csCubeMeshObjectFactory;

/**
 * Cube version of mesh object.
 */
class csCubeMeshObject : public iMeshObject
{
private:
  csCubeMeshObjectFactory* factory;

public:
  /// Constructor.
  csCubeMeshObject (csCubeMeshObjectFactory* factory);

  /// Destructor.
  virtual ~csCubeMeshObject ();

public:
  ///------------------------ iMeshObject implementation ------------------------
  DECLARE_IBASE;

  /// Draw.
  virtual bool Draw (iRenderView* rview, iMovable* movable);
};

/**
 * Factory for cubes. This factory also implements iCubeMeshObject
 * so that you can set the size of the cube and the material to use
 * for all instances that are created from this factory.
 */
class csCubeMeshObjectFactory : public iMeshObjectFactory
{
private:
  float size;
  iMaterialWrapper* material;

public:
  /// Constructor.
  csCubeMeshObjectFactory (iBase*);

  /// Destructor.
  virtual ~csCubeMeshObjectFactory ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

  /// Get the size of this cube.
  float GetSize () { return size; }
  /// Get the material for this cube.
  iMaterialWrapper* GetMaterialWrapper () { return material; }

public:
  //------------------------ iMeshObjectFactory implementation --------------
  DECLARE_IBASE;

  /// Draw.
  virtual iMeshObject* NewInstance ();

  //------------------------- iCubeMeshObject implementation ----------------
  struct CubeMeshObject : public iCubeMeshObject
  {
    DECLARE_EMBEDDED_IBASE (csCubeMeshObjectFactory);
    virtual void SetSize (float size) { scfParent->size = size; }
    virtual float GetSize () { return scfParent->size; }
    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    {
      scfParent->material = material;
    }
    virtual iMaterialWrapper* GetMaterialWrapper () { return scfParent->material; }
  } scfiCubeMeshObject;
  friend class CubeMeshObject;
};

#endif // _CUBE_H_

