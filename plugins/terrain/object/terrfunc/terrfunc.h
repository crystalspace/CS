/*
    Copyright (C) 2001 by Jorrit Tyberghein
  
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

#ifndef __CS_TERRFUNC_H__
#define __CS_TERRFUNC_H__

#include "csgeom/transfrm.h"
#include "csgeom/vector3.h"
#include "csobject/csobject.h"
#include "csutil/cscolor.h"
#include "iterrain/object.h"
#include "iengine/terrain.h"
#include "iterrain/terrfunc.h"

struct iEngine;
struct iMaterialWrapper;
struct iSystem;

class csTerrFuncObject : public iTerrainObject
{
private:
  iSystem* pSystem;
  iTerrainObjectFactory* pFactory;
  iMaterialWrapper** materials;
  int blockx, blocky;
  int gridx[4], gridy[4];
  G3DTriangleMesh* trimesh[4];
  csVector3 topleft;
  csVector3 scale;
  csVector3* normals[4];
  csColor base_color;
  csTerrainHeightFunction* height_func;
  csTerrainNormalFunction* normal_func;
  void* height_func_data;
  void* normal_func_data;

  // For directional lighting.
  bool do_dirlight;
  csVector3 dirlight;
  csColor dirlight_color;
  // This number is increased whenever there is a lighting change
  long dirlight_number;
  // Numbers of all meshes. Here we can see if we need to update the lighting
  // for a mesh.
  long* dirlight_numbers[4];

  bool initialized;
  void SetupObject ();
  void ComputeNormals ();
  void RecomputeLighting (int lod, int bx, int by);

public:
  /// Constructor.
  csTerrFuncObject (iSystem* pSys, iTerrainObjectFactory* factory);
  virtual ~csTerrFuncObject ();

  void LoadMaterialGroup (iEngine* engine, const char *pName,
    int iStart, int iEnd);
  /// Set the base color.
  void SetColor (const csColor& col) { base_color = col; dirlight_number++; }
  /// Get the base color.
  csColor GetColor () const { return base_color; }
  /// Set the function to use for the terrain.
  void SetHeightFunction (csTerrainHeightFunction* func, void* d)
  { height_func = func; height_func_data = d; initialized = false; }
  /// Set the normal function to use for the terrain.
  void SetNormalFunction (csTerrainNormalFunction* func, void* d)
  { normal_func = func; normal_func_data = d; initialized = false; }

  ///--------------------- iTerrainObject implementation ---------------------
  DECLARE_IBASE;

  virtual void SetDirLight (const csVector3& pos, const csColor& col)
  {
    if (do_dirlight &&
    	((dirlight - pos) < .00001) &&
    	ABS (col.red-dirlight_color.red) < .0001 &&
    	ABS (col.green-dirlight_color.green) < .0001 &&
	ABS (col.blue-dirlight_color.blue) < .0001)
      return;	// Nothing changed.
    do_dirlight = true;
    dirlight = pos;
    dirlight.Normalize ();
    dirlight_color = col;
    dirlight_number++;
  }
  virtual csVector3 GetDirLightPosition () const { return dirlight; }
  virtual csColor GetDirLightColor () const { return dirlight_color; }
  virtual void DisableDirLight () { do_dirlight = false; }
  virtual bool IsDirLightEnabled () const { return do_dirlight; }
  virtual void Draw (iRenderView *rview, bool use_z_buf = true);
  virtual void SetMaterial (int i, iMaterialWrapper* mat)
  {
    if (!materials) materials = new iMaterialWrapper* [blockx * blocky];
    materials[i] = mat;
  }
  virtual int GetNumMaterials () { return blockx*blocky; }
  virtual void SetLOD (unsigned int detail) { }

  virtual int CollisionDetect (csTransform *p);

  /// Setup the number of blocks in the terrain.
  void SetResolution (int x, int y)
  {
    blockx = x;
    blocky = y;
    initialized = false;
  }

  /// Get the x resolution.
  int GetXResolution () { return blockx; }
  /// Get the y resolution.
  int GetYResolution () { return blocky; }
  /**
   * Setup the number of grid points in every block for a given
   * LOD level. There are four LOD levels (0..3).
   */
  void SetGridResolution (int lod, int x, int y)
  {
    gridx[lod] = x;
    gridy[lod] = y;
    initialized = false;
  }
  /// Get the x resolution for a block given some LOD (0..3).
  int GetXGridResolution (int lod) { return gridx[lod]; }
  /// Get the y resolution for a block given some LOD (0..3).
  int GetYGridResolution (int lod) { return gridy[lod]; }

  /// Set the top-left corner of the terrain.
  virtual void SetTopLeftCorner (const csVector3& topleft)
  {
    csTerrFuncObject::topleft = topleft;
    initialized = false;
  }
  // Get the top-left corner.
  virtual csVector3 GetTopLeftCorner ()
  {
    return topleft;
  }
  /// Set the scale of the terrain.
  virtual void SetScale (const csVector3& scale)
  {
    csTerrFuncObject::scale = scale;
    initialized = false;
  }
  /// Get the scale of the terrain.
  virtual csVector3 GetScale ()
  {
    return scale;
  }

  //------------------------- iTerrFuncState implementation ----------------
  class TerrFuncState : public iTerrFuncState
  {
    DECLARE_EMBEDDED_IBASE (csTerrFuncObject);
    virtual void LoadMaterialGroup (iEngine* engine, const char *pName,
      int iStart, int iEnd)
    {
      scfParent->LoadMaterialGroup (engine, pName, iStart, iEnd);
    }
    virtual void SetTopLeftCorner (const csVector3& topleft)
    {
      scfParent->SetTopLeftCorner (topleft);
    }
    virtual csVector3 GetTopLeftCorner ()
    {
      return scfParent->GetTopLeftCorner ();
    }
    virtual void SetScale (const csVector3& scale)
    {
      scfParent->SetScale (scale);
    }
    virtual csVector3 GetScale ()
    {
      return scfParent->GetScale ();
    }
    virtual void SetResolution (int x, int y)
    {
      scfParent->SetResolution (x, y);
    }
    virtual int GetXResolution ()
    {
      return scfParent->GetXResolution ();
    }
    virtual int GetYResolution ()
    {
      return scfParent->GetYResolution ();
    }
    virtual void SetGridResolution (int lod, int x, int y)
    {
      scfParent->SetGridResolution (lod, x, y);
    }
    virtual int GetXGridResolution (int lod)
    {
      return scfParent->GetXGridResolution (lod);
    }
    virtual int GetYGridResolution (int lod)
    {
      return scfParent->GetYGridResolution (lod);
    }
    virtual void SetColor (const csColor& col)
    {
      scfParent->SetColor (col);
    }
    virtual csColor GetColor () const
    {
      return scfParent->GetColor ();
    }
    virtual void SetHeightFunction (csTerrainHeightFunction* func,
    	void* d)
    {
      scfParent->SetHeightFunction (func, d);
    }
    virtual void SetNormalFunction (csTerrainNormalFunction* func,
    	void* d)
    {
      scfParent->SetNormalFunction (func, d);
    }
  } scfiTerrFuncState;
  friend class TerrFuncState;
};

/**
 * Factory for terrain. 
 */
class csTerrFuncObjectFactory : public iTerrainObjectFactory
{
private:
  iSystem *pSystem;

public:
  /// Constructor.
  csTerrFuncObjectFactory (iSystem* pSys);

  /// Destructor.
  virtual ~csTerrFuncObjectFactory ();

  DECLARE_IBASE;

  virtual iTerrainObject* NewInstance ();
};

/**
 * TerrFunc type. This is the plugin you have to use to create instances
 * of csTerrFuncObjectFactory.
 */
class csTerrFuncObjectType : public iTerrainObjectType
{
private:
  iSystem *pSystem;

public:
  /// Constructor.
  csTerrFuncObjectType (iBase*);

  /// Destructor.
  virtual ~csTerrFuncObjectType ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSys);

  //---------------------- iTerrainObjectType implementation --------------
  DECLARE_IBASE;

  /// Draw.
  virtual iTerrainObjectFactory* NewFactory ();
};

#endif // __CS_TERRFUNC_H__

