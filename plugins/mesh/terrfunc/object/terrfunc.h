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
#include "csgeom/objmodel.h"
#include "csutil/cscolor.h"
#include "csutil/refarr.h"
#include "iengine/mesh.h"
#include "imesh/object.h"
#include "imesh/terrfunc.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "ivideo/vbufmgr.h"
#include "qsqrt.h"

struct iEngine;
struct iMaterialWrapper;
struct iObjectRegistry;
class csTerrainQuad;
class csTerrainQuadDiv;
class csTerrFuncObject;

#define LOD_LEVELS 4
#define CS_HORIZON_SIZE 100

/**
 * This is one block in the terrain.
 */
class csTerrBlock
{
public:
  csRef<iVertexBuffer> vbuf[LOD_LEVELS];	// Vertex buffer for every LOD level.
  csVector3* mesh_vertices[LOD_LEVELS];
  csVector2* mesh_texels[LOD_LEVELS];
  csColor* mesh_colors[LOD_LEVELS];
  int num_mesh_vertices[LOD_LEVELS];
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


  /// the triangle divisor for this block (if enabled)
  csTerrainQuadDiv *quaddiv;
  /// quaddiv- block visible?
  bool quaddiv_visible;
  /// and then store these bools for later
  bool qd_portal, qd_plane, qd_z_plane;

public:
  csTerrBlock ();
  ~csTerrBlock ();

  /// prepare quad divisor, precalculations
  void PrepareQuadDiv(iTerrainHeightFunction *height_func,
    csTerrFuncObject *terr);
  /// compute LOD for block, prepare to render (for quaddiv use)
  void PrepareFrame(const csVector3& campos, int framenum,
    csTerrFuncObject *terr);
  /// Draw (for quaddiv use)
  void Draw(iRenderView *rview, bool clip_portal, bool clip_plane,
    bool clip_z_plane, float correct_du, float correct_su, float correct_dv, 
    float correct_sv, csTerrFuncObject *terr, int framenum);
};

class csTerrFuncObject : public iMeshObject
{
public:
  csRef<iTerrainHeightFunction> height_func;
  csRef<iTerrainNormalFunction> normal_func;
  int blockxy;
  int gridx, gridy;
  csVector3 topleft;
  csVector3 scale;
  csVector3 radius;
  csVector3 rad_center;
  csTerrBlock* blocks;
  bool block_dim_invalid;
  csBox3 global_bbox;
  float grid_stepx;
  float grid_stepy;
  float inv_block_stepx;
  float inv_block_stepy;
  float inv_grid_stepx;
  float inv_grid_stepy;

  /// quad divisor enabled?
  bool quaddiv_enabled;
  /// quad height function wrapper
  iTerrainHeightFunction *quad_height;
  /// quad normal function wrapper
  iTerrainNormalFunction *quad_normal;
  /// quaddiv framenumber to use
  int qd_framenum;

  // A function which will get the height from the height function
  // but first it will clamp the input to 0,0-1,1.
  float GetClampedHeight (float x, float y)
  {
    if (x < 0.0f) x = 0.0f;
    if (x > 1.0f) x = 1.0f;
    if (y < 0.0f) y = 0.0f;
    if (y > 1.0f) y = 1.0f;
    return height_func->GetHeight (x, y);
  }

//private: //@@@
public:
  iObjectRegistry* object_reg;
  iBase* logparent;
  iMeshObjectFactory* pFactory;
  csRef<iMeshObjectDrawCallback> vis_cb;
  iVertexBufferManager *vbufmgr;
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

  csFlags flags;

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
  void InitMesh (G3DTriangleMesh& mesh, csVector3*& mesh_vertices,
  	csVector2*& mesh_texels, csColor*& mesh_colors);

  /**
   * Setup the base mesh (lod level 0). This will basically
   * initialize the mesh by sampling the height function at regular
   * intervals (gridx/gridy resolution).
   */
  void SetupBaseMesh (G3DTriangleMesh& mesh,
  	csVector3*& mesh_vertices, csVector2*& mesh_texels,
	csColor*& mesh_colors, int& num_mesh_vertices, int bx, int by);

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
	const G3DTriangleMesh& source, csVector3* source_vertices,
	csVector2* source_texels, csColor* source_colors,
	int num_source_vertices,
	G3DTriangleMesh& dest, csVector3*& dest_vertices,
	csVector2*& dest_texels, csColor*& dest_colors,
	int& num_dest_vertices,
	float maxcost, int& del_tri, int& tot_tri);

  /**
   * Compute the bounding box of a triangle mesh.
   */
  void ComputeBBox (const G3DTriangleMesh& mesh,
  	csVector3* mesh_vertices, int num_mesh_vertices, csBox3& bbox);

  /**
   * Compute all bounding boxes.
   */
  void ComputeBBoxes ();

  /**
   * Allocate normal array and compute normals for one mesh and put
   * result in pNormals.
   */
  void ComputeNormals (const G3DTriangleMesh& mesh,
  	csVector3* mesh_vertices, int num_mesh_vertices, csVector3** pNormals);

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
  bool BBoxVisible (iRenderView* rview, const csBox3& bbox,
	int& clip_portal, int& clip_plane, int& clip_z_plane,
	csPlane3* planes, uint32 frustum_mask);

  /// retrieve a vertexbuffer from the manager if not done already
  void SetupVertexBuffer (csRef<iVertexBuffer> &vbuf1, iVertexBuffer** vbuf2);

  /// interface to receive state of vertexbuffermanager
  struct eiVertexBufferManagerClient : public iVertexBufferManagerClient
  {
    SCF_DECLARE_EMBEDDED_IBASE (csTerrFuncObject);
    virtual void ManagerClosing ();
  }scfiVertexBufferManagerClient;
  friend struct eiVertexBufferManagerClient;


public:
  /// Constructor.
  csTerrFuncObject (iObjectRegistry* object_reg, iMeshObjectFactory* factory);
  virtual ~csTerrFuncObject ();

  void LoadMaterialGroup (iLoaderContext* ldr_context, const char *pName,
    int iStart, int iEnd);
  /// Get the base color.
  csColor GetColor () const { return base_color; }
  /// Set the function to use for the terrain.
  void SetHeightFunction (iTerrainHeightFunction* func)
  {
    height_func = func;
    initialized = false;
  }
  /// Set the normal function to use for the terrain.
  void SetNormalFunction (iTerrainNormalFunction* func)
  {
    normal_func = func;
    initialized = false;
  }
  void SetHeightMap (iImage* im, float hscale, float hshift, bool flipx,
    bool flipy);

  /// Setup the number of blocks in the terrain.
  void SetResolution (int x, int y)
  {
    (void)y;
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
    return qsqrt (lod_sqdist[lod-1]);
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

  /**
   * Construct quad divisors for all blocks and interconnects them.
   */
  void InitQuadDiv();

  /** Draw the quaddivisor terrain */
  void QuadDivDraw (iRenderView* rview, csZBufMode zbufMode,
  	csPlane3* planes, uint32 frustum_mask);

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

  // Take a block index value and convert it to an x,y value
  inline void Index2Block( int index, int& x, int& y)
  { x = index % blockxy; y = index / blockxy; }
  // Take a block x,y vaue and convert it to an index
  inline void Block2Index( int x, int y, int& index )
  { index = y * blockxy + x; }
  // Find the nearest top left block to point p.
  void Object2Block( csVector3 p, int& x, int& y)
  {
    x = int ( inv_block_stepx * ( p.x - topleft.x));
    y = int ( inv_block_stepy * ( p.z - topleft.z));
  }
  // Find the nearest top left grid to point p.
  void Object2Grid( csVector3 p, int& block_index, int& x, int& y)
  {
    csVector3 block_corner;
    Object2Block(p, x, y);
    Block2Object(x, y, block_corner);
    Block2Index(x, y, block_index);
    x = int ( inv_grid_stepx * ( p.x - block_corner.x ));
    y = int ( inv_grid_stepy * ( p.z - block_corner.z ));
  }
  // Find the top left corner of grid x,y in block bx,by
  void Grid2Object( int bx, int by, int x, int y, csVector3& p)
  {
    Block2Object(bx, by, p);
    p.x += x * grid_stepx;
    p.z += y * grid_stepy;
  }
  // Find the top left corner of block x,y
  void Block2Object( int x, int y, csVector3& p)
  {
    p.x = x * scale.x + topleft.x;
    p.y = topleft.y;
    p.z = y * scale.z + topleft.z;
  }

  void GetObjectBoundingBox (csBox3& bbox, int type = CS_BBOX_NORMAL);
  void GetRadius (csVector3& rad, csVector3& cent);

  ///--------------------- iMeshObject implementation ---------------------
  SCF_DECLARE_IBASE;

  virtual iMeshObjectFactory* GetFactory () const { return pFactory; }

  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> Clone () { return 0; }
  virtual bool DrawTest (iRenderView* rview, iMovable* movable,
  	uint32 frustum_mask);
  virtual csRenderMesh **GetRenderMeshes (int &n, iRenderView*,
    iMovable*, uint32) { n = 0; return 0; }

  virtual bool Draw (iRenderView* rview, iMovable* movable,
  	csZBufMode zbufMode);

  virtual void SetVisibleCallback (iMeshObjectDrawCallback* cb)
  {
    vis_cb = cb;
  }
  virtual iMeshObjectDrawCallback* GetVisibleCallback () const
  {
    return vis_cb;
  }

  virtual void NextFrame (csTicks, const csVector3& /*pos*/) { }

  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }
  virtual iBase* GetLogicalParent () const { return logparent; }

  virtual bool HitBeamOutline (const csVector3& start, const csVector3& end,
        csVector3& isect, float* pr);
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr, int* polygon_idx = 0);

  //------------------------- iObjectModel implementation ----------------
  class ObjectModel : public csObjectModel
  {
    SCF_DECLARE_EMBEDDED_IBASE (csTerrFuncObject);
    virtual void GetObjectBoundingBox (csBox3& bbox, int type = CS_BBOX_NORMAL)
    {
      scfParent->GetObjectBoundingBox (bbox, type);
    }
    virtual void GetRadius (csVector3& rad, csVector3& cent)
    {
      scfParent->GetRadius (rad, cent);
    }
  } scfiObjectModel;
  friend class ObjectModel;

  virtual iObjectModel* GetObjectModel () { return &scfiObjectModel; }
  /// Set the base color.
  virtual bool SetColor (const csColor& col)
  {
    base_color = col;
    dirlight_number++;
    return true;
  }
  virtual bool GetColor (csColor& col) const { col = base_color; return true; }
  virtual bool SetMaterialWrapper (iMaterialWrapper*) { return false; }
  virtual iMaterialWrapper* GetMaterialWrapper () const { return 0; }
  virtual void InvalidateMaterialHandles () { }
  /**
   * see imesh/object.h for specification. The default implementation
   * does nothing.
   */
  virtual void PositionChild (iMeshObject* child,csTicks current_time) { }

  /**  RDS NOTE: this is from iTerrainObject, what matches???  **/
  //------------------------- iTerrFuncState implementation ----------------
  class TerrFuncState : public iTerrFuncState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csTerrFuncObject);
    virtual void LoadMaterialGroup (iLoaderContext* ldr_context,
    	const char *pName, int iStart, int iEnd)
    {
      scfParent->LoadMaterialGroup (ldr_context, pName, iStart, iEnd);
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
    virtual void SetHeightFunction (iTerrainHeightFunction* func)
    {
      scfParent->SetHeightFunction (func);
    }
    virtual void SetNormalFunction (iTerrainNormalFunction* func)
    {
      scfParent->SetNormalFunction (func);
    }
    virtual void SetHeightMap (iImage* im, float hscale, float hshift,
      bool flipx, bool flipy)
    {
      scfParent->SetHeightMap (im, hscale, hshift, flipx, flipy);
    }
    virtual iTerrainHeightFunction* GetHeightFunction () const
    {
      return scfParent->height_func;
    }
    virtual iTerrainNormalFunction* GetNormalFunction () const
    {
      return scfParent->normal_func;
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
  iBase* logparent;
  csFlags flags;

public:
  iObjectRegistry *object_reg;

  /// Constructor.
  csTerrFuncObjectFactory (iObjectRegistry* object_reg);

  /// Destructor.
  virtual ~csTerrFuncObjectFactory ();

  SCF_DECLARE_IBASE;

  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> NewInstance ();
  virtual csPtr<iMeshObjectFactory> Clone () { return 0; }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }
  virtual iBase* GetLogicalParent () const { return logparent; }
  virtual iObjectModel* GetObjectModel () { return 0; }
};

/**
 * TerrFunc type. This is the plugin you have to use to create instances
 * of csTerrFuncObjectFactory.
 */
class csTerrFuncObjectType : public iMeshObjectType
{
private:
  iObjectRegistry *object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csTerrFuncObjectType (iBase*);
  /// Destructor.
  virtual ~csTerrFuncObjectType ();

  /// create a new factory.
  virtual csPtr<iMeshObjectFactory> NewFactory ();

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csTerrFuncObjectType);
    virtual bool Initialize (iObjectRegistry* p)
    { scfParent->object_reg = p; return true; }
  } scfiComponent;
  friend struct eiComponent;
};

#endif // __CS_TERRFUNC_H__
