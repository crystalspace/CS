/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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
#include "imesh/object.h"
#include "imesh/cube.h"
#include "ivideo/graph3d.h"
#include "iutil/config.h"
#include "isys/plugin.h"

struct iMaterialWrapper;
class csCubeMeshObjectFactory;

#define ALL_FEATURES (CS_OBJECT_FEATURE_LIGHTING)

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
  iMeshObjectDrawCallback* vis_cb;
  float sizex, sizey, sizez;
  csVector3 radius;
  csVector3 shift;
  long shapenr;
  float current_lod;
  uint32 current_features;

  /**
   * Camera space bounding box is cached here.
   * GetCameraBoundingBox() will check the current camera number
   * to see if it needs to recalculate this.
   */
  csBox3 camera_bbox;

  /// Current camera number.
  long cur_cameranr;
  /// Current movable number.
  long cur_movablenr;

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
  void GetTransformedBoundingBox (long cameranr, long movablenr,
      const csReversibleTransform& trans, csBox3& cbox);
  /**
   * Get the coordinates of the cube in screen coordinates.
   * Fills in the boundingBox with the X and Y locations of the cube.
   * Returns the max Z location of the cube, or -1 if not
   * on-screen. If the cube is not on-screen, the X and Y values are not
   * valid.
   */
  float GetScreenBoundingBox (long cameranr, long movablenr,
  	float fov, float sx, float sy,
	const csReversibleTransform& trans, csBox2& sbox, csBox3& cbox);

  //----------------------- iMeshObject implementation ------------------------
  SCF_DECLARE_IBASE;

  virtual iMeshObjectFactory* GetFactory () const { return ifactory; }
  virtual bool DrawTest (iRenderView* rview, iMovable* movable);
  virtual void UpdateLighting (iLight** lights, int num_lights,
      	iMovable* movable);
  virtual bool Draw (iRenderView* rview, iMovable* movable, csZBufMode mode);
  virtual void SetVisibleCallback (iMeshObjectDrawCallback* cb)
  {
    if (cb) cb->IncRef ();
    if (vis_cb) vis_cb->DecRef ();
    vis_cb = cb;
  }
  virtual iMeshObjectDrawCallback* GetVisibleCallback () const
  {
    return vis_cb;
  }
  virtual void GetObjectBoundingBox (csBox3& bbox, int type = CS_BBOX_NORMAL);
  virtual void GetRadius (csVector3& rad, csVector3& cent) 
	{ rad = radius; cent = object_bbox.GetCenter(); }
  virtual void NextFrame (csTicks /*current_time*/) { }
  virtual bool WantToDie () const { return false; }
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () const { return true; }
  virtual int HitBeamBBox (const csVector3& start, const csVector3& end,
    csVector3& isect, float* pr);
  virtual bool HitBeamOutline (const csVector3& start, const csVector3& end,
    csVector3& isect, float* pr);
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr);
  virtual long GetShapeNumber () const { return shapenr; }
  virtual uint32 GetLODFeatures () const { return current_features; }
  virtual void SetLODFeatures (uint32 mask, uint32 value)
  {
    mask &= ALL_FEATURES;
    current_features = (current_features & ~mask) | (value & mask);
  }
  virtual void SetLOD (float lod) { current_lod = lod; }
  virtual float GetLOD () const { return current_lod; }
  virtual int GetLODPolygonCount (float /*lod*/) const
  {
    return 0;	// @@@ Implement me please!
  }
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
  float GetSizeX () const { return sizex; }
  /// Get the y size of this cube.
  float GetSizeY () const { return sizey; }
  /// Get the z size of this cube.
  float GetSizeZ () const { return sizez; }
  /// Get the shift.
  const csVector3& GetShift () const { return shift; }
  /// Get the x shift of this cube.
  float GetShiftX () const { return shift.x; }
  /// Get the y shift of this cube.
  float GetShiftY () const { return shift.y; }
  /// Get the z shift of this cube.
  float GetShiftZ () const { return shift.z; }
  /// Get the material for this cube.
  iMaterialWrapper* GetMaterialWrapper () const { return material; }
  /// Get mixmode.
  UInt GetMixMode () const { return MixMode; }

  //------------------------ iMeshObjectFactory implementation --------------
  SCF_DECLARE_IBASE;

  virtual iMeshObject* NewInstance ();
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }

  //------------------------- iCubeFactoryState implementation ----------------
  class CubeFactoryState : public iCubeFactoryState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csCubeMeshObjectFactory);
    virtual void SetSize (float sizex, float sizey, float sizez)
    {
      scfParent->sizex = sizex;
      scfParent->sizey = sizey;
      scfParent->sizez = sizez;
    }
    virtual float GetSizeX () const { return scfParent->sizex; }
    virtual float GetSizeY () const { return scfParent->sizey; }
    virtual float GetSizeZ () const { return scfParent->sizez; }
    virtual void SetShift (float shiftx, float shifty, float shiftz)
    {
      scfParent->shift.Set (shiftx, shifty, shiftz);
    }
    virtual float GetShiftX () const { return scfParent->shift.x; }
    virtual float GetShiftY () const { return scfParent->shift.y; }
    virtual float GetShiftZ () const { return scfParent->shift.z; }
    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    {
      scfParent->material = material;
    }
    virtual iMaterialWrapper* GetMaterialWrapper () const
    { return scfParent->material; }
    virtual void SetMixMode (UInt mode) { scfParent->MixMode = mode; }
    virtual UInt GetMixMode () const { return scfParent->MixMode; }
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
  SCF_DECLARE_IBASE;

  /// Constructor.
  csCubeMeshObjectType (iBase*);
  /// Destructor.
  virtual ~csCubeMeshObjectType ();
  /// Draw.
  virtual iMeshObjectFactory* NewFactory ();
  /// Get features.
  virtual uint32 GetFeatures () const
  {
    return ALL_FEATURES;
  }

  //------------------- iConfig interface implementation -------------------
  struct csCubeConfig : public iConfig
  {
    SCF_DECLARE_EMBEDDED_IBASE (csCubeMeshObjectType);
    virtual bool GetOptionDescription (int idx, csOptionDescription *option);
    virtual bool SetOption (int id, csVariant* value);
    virtual bool GetOption (int id, csVariant* value);
  } scfiConfig;
  friend struct csCubeConfig;

  //------------------- iPlugin interface implementation -------------------
  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csCubeMeshObjectType);
    virtual bool Initialize (iObjectRegistry*) { return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
};

#endif // _CUBE_H_
