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
#include "iconfig.h"

struct iMaterialWrapper;
class csCubeMeshObjectFactory;

/**
 * Cube version of mesh object.
 */
class csCubeMeshObject : public iMeshObject
{
private:
  csCubeMeshObjectFactory* factory;
  iMeshObjectFactory* ifactory;
  csVector3 vertices[8];
  csVector3 normals[8];
  csVector2 uv[8];
  csColor colors[8];
  csTriangle triangles[12];
  G3DFogInfo fog[8];
  G3DTriangleMesh mesh;
  bool initialized;
  csBox3 object_bbox;
  csMeshCallback* vis_cb;
  void* vis_cbData;
  float sizex, sizey, sizez;
  csVector3 radius;
  csVector3 shift;

  /**
   * Camera space bounding box is cached here.
   * GetCameraBoundingBox() will check the current cookie from the
   * transformation manager to see if it needs to recalculate this.
   */
  csBox3 camera_bbox;

  /// Current cookie for camera_bbox.
  csTranCookie camera_cookie;

  /**
   * Setup this object. This function will check if setup is needed.
   */
  void SetupObject ();

public:
  /// Constructor.
  csCubeMeshObject (csCubeMeshObjectFactory* factory);

  /// Destructor.
  virtual ~csCubeMeshObject ();

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
      float sx, float sy,
      const csReversibleTransform& trans, csBox2& sbox, csBox3& cbox);

  ///------------------------ iMeshObject implementation ------------------------
  DECLARE_IBASE;

  virtual iMeshObjectFactory* GetFactory () { return ifactory; }
  virtual bool DrawTest (iRenderView* rview, iMovable* movable);
  virtual void UpdateLighting (iLight** lights, int num_lights,
      	iMovable* movable);
  virtual bool Draw (iRenderView* rview, iMovable* movable);
  virtual void SetVisibleCallback (csMeshCallback* cb, void* cbData)
  {
    vis_cb = cb;
    vis_cbData = cbData;
  }
  virtual csMeshCallback* GetVisibleCallback ()
  {
    return vis_cb;
  }
  virtual void GetObjectBoundingBox (csBox3& bbox, int type = CS_BBOX_NORMAL);
  virtual csVector3 GetRadius () { return radius; }
  virtual void NextFrame (cs_time /*current_time*/) { }
  virtual bool WantToDie () { return false; }
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () { return true; }
  virtual bool HitBeamObject (const csVector3&, const csVector3&,
  	csVector3&, float*) { return false; }
};

/**
 * Factory for cubes. This factory also implements iCubeFactoryState
 * so that you can set the size of the cube and the material to use
 * for all instances that are created from this factory.
 */
class csCubeMeshObjectFactory : public iMeshObjectFactory
{
private:
  float sizex, sizey, sizez;
  csVector3 shift;
  iMaterialWrapper* material;
  UInt MixMode;

public:
  /// Constructor.
  csCubeMeshObjectFactory (iBase *pParent);

  /// Destructor.
  virtual ~csCubeMeshObjectFactory ();

  /// Get the x size of this cube.
  float GetSizeX () { return sizex; }
  /// Get the y size of this cube.
  float GetSizeY () { return sizey; }
  /// Get the z size of this cube.
  float GetSizeZ () { return sizez; }
  /// Get the shift.
  const csVector3& GetShift () { return shift; }
  /// Get the x shift of this cube.
  float GetShiftX () { return shift.x; }
  /// Get the y shift of this cube.
  float GetShiftY () { return shift.y; }
  /// Get the z shift of this cube.
  float GetShiftZ () { return shift.z; }
  /// Get the material for this cube.
  iMaterialWrapper* GetMaterialWrapper () { return material; }
  /// Get mixmode.
  UInt GetMixMode () { return MixMode; }

  //------------------------ iMeshObjectFactory implementation --------------
  DECLARE_IBASE;

  virtual iMeshObject* NewInstance ();
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () { return false; }

  //------------------------- iCubeFactoryState implementation ----------------
  class CubeFactoryState : public iCubeFactoryState
  {
    DECLARE_EMBEDDED_IBASE (csCubeMeshObjectFactory);
    virtual void SetSize (float sizex, float sizey, float sizez)
    {
      scfParent->sizex = sizex;
      scfParent->sizey = sizey;
      scfParent->sizez = sizez;
    }
    virtual float GetSizeX () { return scfParent->sizex; }
    virtual float GetSizeY () { return scfParent->sizey; }
    virtual float GetSizeZ () { return scfParent->sizez; }
    virtual void SetShift (float shiftx, float shifty, float shiftz)
    {
      scfParent->shift.Set (shiftx, shifty, shiftz);
    }
    virtual float GetShiftX () { return scfParent->shift.x; }
    virtual float GetShiftY () { return scfParent->shift.y; }
    virtual float GetShiftZ () { return scfParent->shift.z; }
    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    {
      scfParent->material = material;
    }
    virtual iMaterialWrapper* GetMaterialWrapper () { return scfParent->material; }
    virtual void SetMixMode (UInt mode) { scfParent->MixMode = mode; }
    virtual UInt GetMixMode () { return scfParent->MixMode; }
  } scfiCubeFactoryState;
  friend class CubeFactoryState;
};

/**
 * Cube type. This is the plugin you have to use to create instances
 * of csCubeMeshObjectFactory.
 */
class csCubeMeshObjectType : public iMeshObjectType
{
private:
  float default_sizex, default_sizey, default_sizez;
  csVector3 default_shift;
  UInt default_MixMode;

public:
  /// Constructor.
  csCubeMeshObjectType (iBase*);

  /// Destructor.
  virtual ~csCubeMeshObjectType ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

  //------------------------ iMeshObjectType implementation --------------
  DECLARE_IBASE;

  /// Draw.
  virtual iMeshObjectFactory* NewFactory ();

  ///------------------- iConfig interface implementation -------------------
  struct csCubeConfig : public iConfig
  {
    DECLARE_EMBEDDED_IBASE (csCubeMeshObjectType);
    virtual bool GetOptionDescription (int idx, csOptionDescription *option);
    virtual bool SetOption (int id, csVariant* value);
    virtual bool GetOption (int id, csVariant* value);
  } scfiConfig;
  friend struct csCubeConfig;
};

#endif // _CUBE_H_

