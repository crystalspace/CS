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
#include "csutil/cscolor.h"
#include "iengine/mesh.h"
#include "imesh/object.h"
#include "imesh/terrfunc.h"
#include "isys/plugin.h"

struct iEngine;
struct iMaterialWrapper;
struct iSystem;
class csTerrainQuad;

#define LOD_LEVELS 4
#define CS_HORIZON_SIZE 100

#define ALL_FEATURES (CS_OBJECT_FEATURE_LIGHTING)

/**
 * This is one block in the terrain.
 */
class csTerrBlock
{
public:
  G3DTriangleMesh mesh[LOD_LEVELS];	// Mesh with four LOD levels.
  csVector3* normals[LOD_LEVELS];	// Array of normals for the LOD levels.
  iMaterialWrapper* material;		// Material for this block.
  csVector3 center;			// Center for LOD.
  // Numbers of all meshes. Here we can see if we need to update the lighting
  // for a mesh and LOD.
  int dirlight_numbers[LOD_LEVELS];
  csBox3 bbox;				// Bounding box in 3D of this block.
  csTerrainQuad* node;			// Pointer to corresponding node in
  					// quadtree.

public:
  csTerrBlock ();
  ~csTerrBlock ();
};

class csTerrFuncObject : public iMeshObject
{
public:
  csTerrainHeightFunction* height_func;
  csTerrainNormalFunction* normal_func;
  void* height_func_data;
  void* normal_func_data;
  int blockxy;
  int gridx, gridy;
  csVector3 topleft;
  csVector3 scale;
  csTerrBlock* blocks;
  bool block_dim_invalid;
  csBox3 global_bbox;

private:
  iSystem* pSystem;
  iMeshObjectFactory* pFactory;
  iMeshObjectDrawCallback* vis_cb;
  float current_lod;
  uint32 current_features;
  csColor base_color;
  // Squared distances at which to change LOD (0..2).
  float lod_sqdist[LOD_LEVELS-1];
  // Maximum cost for every lod transition.
  float max_cost[LOD_LEVELS-1];

  // For directional lighting.
  bool do_dirlight;
  csVector3 dirlight;
  csColor dirlight_color;
  // This number is increased whenever there is a lighting change
  long dirlight_number;

  // For shadows. This grid is like a z-buffer indicating the closeness
  // of that part of the landscape to the light.
  float* shadow_map;

  bool initialized;

  // For correcting texture seams.
  int correct_tw, correct_th;
  float correct_du, correct_su;
  float correct_dv, correct_sv;

  // For visibility.
  int quad_depth;
  int block_depth;	// Depth of the quadtree in one block.
  csTerrainQuad* quadtree;
  float horizon[CS_HORIZON_SIZE];
  bool do_vis_test;

  /**
   * Clear a mesh and initialize it for new usage (call before
   * SetupBaseMesh() or ComputeLODLevel() (as dest)).
   */
  void InitMesh (G3DTriangleMesh& mesh);

  /**
   * Setup the base mesh (lod level 0). This will basically
   * initialize the mesh by sampling the height function at regular
   * intervals (gridx/gridy resolution).
   */
  void SetupBaseMesh (G3DTriangleMesh& mesh, int bx, int by);

  /**
   * Setup the visibility tree.
   */
  void SetupVisibilityTree (csTerrainQuad* quad,
    int x1, int y1, int x2, int y2, int depth);

  /**
   * Setup the visibility tree.
   */
  void SetupVisibilityTree ();

  /**
   * Enable/disable visibility testing.
   */
  void SetVisTesting (bool en)
  {
    do_vis_test = en;
  }

  /**
   * Is vis testing enabled?
   */
  bool IsVisTestingEnabled ()
  {
    return do_vis_test;
  }

  /**
   * Compute a destination mesh from a given source mesh
   * by reducing triangles.
   */
  void ComputeLODLevel (
	const G3DTriangleMesh& source, G3DTriangleMesh& dest,
	float maxcost, int& del_tri, int& tot_tri);

  /**
   * Compute the bounding box of a triangle mesh.
   */
  void ComputeBBox (const G3DTriangleMesh& mesh, csBox3& bbox);

  /**
   * Compute all bounding boxes.
   */
  void ComputeBBoxes ();

  /**
   * Allocate normal array and compute normals for one mesh and put
   * result in pNormals.
   */
  void ComputeNormals (const G3DTriangleMesh& mesh, csVector3** pNormals);

  /**
   * Compute all normal arrays (all lod levels and all blocks).
   */
  void ComputeNormals ();

  /**
   * Do the setup of the entire terrain. This will compute the base
   * mesh, the LOD meshes, normals, ...
   */
  void SetupObject ();

  /**
   * Recompute the shadow map if the light changes.
   */
  void RecomputeShadowMap ();

  /**
   * Recompute lighting for one block.
   */
  void RecomputeLighting (int lod, int bx, int by);

  /**
   * Test if this bounding box is visible in the given clipper.
   * Computed flags are for DrawTriangleMesh.
   */
  bool BBoxVisible (const csBox3& bbox, iRenderView* rview, iCamera* camera,
	int& clip_portal, int& clip_plane, int& clip_z_plane);

public:
  /// Constructor.
  csTerrFuncObject (iSystem* pSys, iMeshObjectFactory* factory);
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
  void SetHeightMap (iImage* im, float hscale, float hshift);

  /// Setup the number of blocks in the terrain.
  void SetResolution (int x, int y)
  {
    // @@@: x and y should be equal!!!
    CS_ASSERT (x == y);
    block_dim_invalid = blockxy != x;
    blockxy = x;
    initialized = false;
  }

  /// Get the x resolution.
  int GetXResolution () { return blockxy; }
  /// Get the y resolution.
  int GetYResolution () { return blockxy; }
  /**
   * Setup the number of grid points in every block for the base mesh.
   */
  void SetGridResolution (int x, int y)
  {
    gridx = x;
    gridy = y;
    initialized = false;
  }
  /// Get the x resolution for a block.
  int GetXGridResolution () { return gridx; }
  /// Get the y resolution for a block.
  int GetYGridResolution () { return gridy; }

  /// Set the top-left corner of the terrain.
  void SetTopLeftCorner (const csVector3& topleft)
  {
    csTerrFuncObject::topleft = topleft;
    initialized = false;
  }
  // Get the top-left corner.
  csVector3 GetTopLeftCorner ()
  {
    return topleft;
  }
  /// Set the scale of the terrain.
  void SetScale (const csVector3& scale)
  {
    csTerrFuncObject::scale = scale;
    initialized = false;
  }
  /// Get the scale of the terrain.
  csVector3 GetScale ()
  {
    return scale;
  }
  /**
   * Set the distance at which to switch to the given lod level
   * (lod from 1 to 3).
   */
  void SetLODDistance (int lod, float dist)
  {
    lod_sqdist[lod-1] = dist*dist;
  }
  /// Get the distance at which lod will switch to that level.
  float GetLODDistance (int lod)
  {
    return sqrt (lod_sqdist[lod-1]);
  }
  /// Set the maximum cost for LOD level (1..3).
  void SetMaximumLODCost (int lod, float maxcost)
  {
    max_cost[lod-1] = maxcost;
    initialized = false;
  }
  /// Get the maximum cost for LOD level (1..3).
  float GetMaximumLODCost (int lod)
  {
    return max_cost[lod-1];
  }

  /**
   * Correct texture mapping so that no seams will appear with textures
   * of the given size. By default this is 0,0 so no correction will happen.
   */
  void CorrectSeams (int tw, int th);

  /// Get texture size for which seams will be corrected.
  void GetCorrectSeams (int& tw, int& th) const
  {
    tw = correct_tw;
    th = correct_th;
  }

  void SetQuadDepth (int qd)
  {
    quad_depth = qd;
    initialized = false;
  }

  int GetQuadDepth () const
  {
    return quad_depth;
  }

  /**
   * Test visibility from a given position.
   * This will call MarkVisible() for all quad nodes that are visible.
   */
  void TestVisibility (iRenderView* rview);
  //  RDS NOTE: is this the same as DrawTest()????

  void SetDirLight (const csVector3& pos, const csColor& col)
  {
    csVector3 dp = dirlight - pos;
    if (do_dirlight &&
    	ABS (dp.x) < .00001 &&
	ABS (dp.y) < .00001 &&
	ABS (dp.z) < .00001 &&
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
  csVector3 GetDirLightPosition () const { return dirlight; }
  csColor GetDirLightColor () const { return dirlight_color; }
  void DisableDirLight () { do_dirlight = false; }
  bool IsDirLightEnabled () const { return do_dirlight; }
  void SetMaterial (int i, iMaterialWrapper* mat)
  {
    if (!blocks || block_dim_invalid)
    {
      blocks = new csTerrBlock [blockxy*blockxy];
      block_dim_invalid = false;
    }
    blocks[i].material = mat;
  }
  int GetMaterialCount () { return blockxy*blockxy; }

  int CollisionDetect (csTransform *p);

  ///--------------------- iMeshObject implementation ---------------------
  SCF_DECLARE_IBASE;

  virtual iMeshObjectFactory* GetFactory () const { return pFactory; }

  virtual bool DrawTest (iRenderView* rview, iMovable* movable);

  virtual void UpdateLighting (iLight** lights, int num_lights,
      	iMovable* movable);

  virtual bool Draw (iRenderView* rview, iMovable* movable,
  	csZBufMode zbufMode);

  virtual void SetVisibleCallback (iMeshObjectDrawCallback* cb)
  {
    SCF_SET_REF (vis_cb, cb);
  }
  virtual iMeshObjectDrawCallback* GetVisibleCallback () const
  {
    return vis_cb;
  }

  virtual void GetObjectBoundingBox (csBox3& bbox, int type = CS_BBOX_NORMAL);
  virtual csVector3 GetRadius ();

  virtual void NextFrame (csTime) { }
  virtual bool WantToDie () const { return false; }

  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }

  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr);

  virtual long GetShapeNumber () const { return 1; }

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
    // @@@ IMPLEMENT ME!
    return 1;
  }

  /**  RDS NOTE: this is from iTerrainObject, what matches???  **/
  //------------------------- iTerrFuncState implementation ----------------
  class TerrFuncState : public iTerrFuncState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csTerrFuncObject);
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
    virtual void SetGridResolution (int x, int y)
    {
      scfParent->SetGridResolution (x, y);
    }
    virtual int GetXGridResolution ()
    {
      return scfParent->GetXGridResolution ();
    }
    virtual int GetYGridResolution ()
    {
      return scfParent->GetYGridResolution ();
    }
    virtual void SetColor (const csColor& col)
    {
      scfParent->SetColor (col);
    }
    virtual csColor GetColor () const
    {
      return scfParent->GetColor ();
    }
    virtual void SetHeightFunction (csTerrainHeightFunction* func, void* d)
    {
      scfParent->SetHeightFunction (func, d);
    }
    virtual void SetNormalFunction (csTerrainNormalFunction* func, void* d)
    {
      scfParent->SetNormalFunction (func, d);
    }
    virtual void SetHeightMap (iImage* im, float hscale, float hshift)
    {
      scfParent->SetHeightMap (im, hscale, hshift);
    }
    virtual void SetLODDistance (int lod, float dist)
    {
      scfParent->SetLODDistance (lod, dist);
    }
    virtual float GetLODDistance (int lod)
    {
      return scfParent->GetLODDistance (lod);
    }
    virtual void SetMaximumLODCost (int lod, float maxcost)
    {
      scfParent->SetMaximumLODCost (lod, maxcost);
    }
    virtual float GetMaximumLODCost (int lod)
    {
      return scfParent->GetMaximumLODCost (lod);
    }
    virtual void CorrectSeams (int tw, int th)
    {
      scfParent->CorrectSeams (tw, th);
    }
    virtual void GetCorrectSeams (int& tw, int& th) const
    {
      scfParent->GetCorrectSeams (tw, th);
    }
    virtual void SetQuadDepth (int qd)
    {
      scfParent->SetQuadDepth (qd);
    }
    virtual int GetQuadDepth () const
    {
      return scfParent->GetQuadDepth ();
    }
    virtual void SetVisTesting (bool en)
    {
      scfParent->SetVisTesting (en);
    }
    virtual bool IsVisTestingEnabled ()
    {
      return scfParent->IsVisTestingEnabled ();
    }
    virtual void SetDirLight (const csVector3& pos, const csColor& col)
    {
      scfParent->SetDirLight (pos, col);
    }
    virtual csVector3 GetDirLightPosition () const
    {
      return scfParent->GetDirLightPosition ();
    }
    virtual csColor GetDirLightColor () const
    {
      return scfParent->GetDirLightColor ();
    }
    virtual void DisableDirLight ()
    {
      scfParent->DisableDirLight ();
    }
    virtual bool IsDirLightEnabled () const
    {
      return scfParent->IsDirLightEnabled ();
    }
    virtual void SetMaterial (int i, iMaterialWrapper* mat)
    {
      scfParent->SetMaterial (i, mat);
    }
    virtual int GetMaterialCount () const
    {
      return scfParent->GetMaterialCount ();
    }

    virtual int CollisionDetect (csTransform *p)
    {
      return scfParent->CollisionDetect (p);
    }

  } scfiTerrFuncState;
  friend class TerrFuncState;
};

/**
 * Factory for terrain.
 */
class csTerrFuncObjectFactory : public iMeshObjectFactory
{
private:
  iSystem *pSystem;

public:
  /// Constructor.
  csTerrFuncObjectFactory (iSystem* pSys);

  /// Destructor.
  virtual ~csTerrFuncObjectFactory ();

  SCF_DECLARE_IBASE;

  virtual iMeshObject* NewInstance ();

  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
};

/**
 * TerrFunc type. This is the plugin you have to use to create instances
 * of csTerrFuncObjectFactory.
 */
class csTerrFuncObjectType : public iMeshObjectType
{
private:
  iSystem *pSystem;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csTerrFuncObjectType (iBase*);
  /// Destructor.
  virtual ~csTerrFuncObjectType ();

  /// Draw.
  virtual iMeshObjectFactory* NewFactory ();

  /// get supported object features
  virtual uint32 GetFeatures () const;

  struct eiPlugIn : public iPlugIn
  {
    SCF_DECLARE_EMBEDDED_IBASE(csTerrFuncObjectType);
    virtual bool Initialize (iSystem* p)
    { scfParent->pSystem = p; return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugIn;
  friend struct eiPlugIn;
};

#endif // __CS_TERRFUNC_H__

