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

#include "csgeom/vector3.h"
#include "csgeom/transfrm.h"
#include "csutil/cscolor.h"
#include "imeshobj.h"
#include "imcube.h"
#include "igraph3d.h"
#include "itranman.h"

struct iMaterialWrapper;
class csCubeMeshObjectFactory;

/**
 * Cube version of mesh object.
 */
class csCubeMeshObject : public iMeshObject
{
private:
  csCubeMeshObjectFactory* factory;
  csVector3 vertices[8];
  csVector3 normals[8];
  csVector2 uv[8];
  csColor colors[8];
  csTriangle triangles[12];
  G3DFogInfo fog[8];
  float cur_size;
  G3DTriangleMesh mesh;

  /**
   * Camera space bounding box is cached here.
   * GetCameraBoundingBox() will check the current cookie from the
   * transformation manager to see if it needs to recalculate this.
   */
  csBox3 camera_bbox;

  /// Current cookie for camera_bbox.
  csTranCookie camera_cookie;

public:
  /// Constructor.
  csCubeMeshObject (csCubeMeshObjectFactory* factory);

  /// Destructor.
  virtual ~csCubeMeshObject ();

public:

  /// Get the bounding box in transformed space.
  void GetTransformedBoundingBox (iTransformationManager* tranman,
      const csReversibleTransform& trans, csBox3& cbox);
  /**
   * Get the coordinates of the cube in screen coordinates.
   * Fills in the boundingBox with the X and Y locations of the cube.
   * Returns the max Z location of the cube, or -1 if not
   * on-screen. If the cube is not on-screen, the X and Y values are not
   * valid.
   */
  float GetScreenBoundingBox (iTransformationManager* tranman, float fov,
      float shiftx, float shifty,
      const csReversibleTransform& trans, csBox2& sbox, csBox3& cbox);

  ///------------------------ iMeshObject implementation ------------------------
  DECLARE_IBASE;

  virtual bool DrawTest (iRenderView* rview, iMovable* movable);
  virtual void UpdateLighting (iLight** lights, int num_lights,
      	iMovable* movable);
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
  UInt MixMode;

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
  /// Get mixmode.
  UInt GetMixMode () { return MixMode; }

public:
  //------------------------ iMeshObjectFactory implementation --------------
  DECLARE_IBASE;

  /// Draw.
  virtual iMeshObject* NewInstance ();

  //------------------------- iCubeMeshObject implementation ----------------
  class CubeMeshObject : public iCubeMeshObject
  {
    DECLARE_EMBEDDED_IBASE (csCubeMeshObjectFactory);
    virtual void SetSize (float size) { scfParent->size = size; }
    virtual float GetSize () { return scfParent->size; }
    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    {
      scfParent->material = material;
    }
    virtual iMaterialWrapper* GetMaterialWrapper () { return scfParent->material; }
    virtual void SetMixMode (UInt mode) { scfParent->MixMode = mode; }
    virtual UInt GetMixMode () { return scfParent->MixMode; }
  } scfiCubeMeshObject;
  friend class CubeMeshObject;
};

#endif // _CUBE_H_

