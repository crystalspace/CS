/*
    Copyright (C) 2002 by Jorrit Tyberghein and Ryan Surkamp

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

#ifndef __CS_BCTERR_H__
#define __CS_BCTERR_H__

#include "csgeom/transfrm.h"
#include "csgeom/vector3.h"
#include "igeom/polymesh.h"
#include "csutil/cscolor.h"
#include "iengine/mesh.h"
#include "imesh/object.h"
#include "imesh/terrfunc.h"
#include "imesh/bcterr.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "ivideo/vbufmgr.h"
#include "igeom/objmodel.h"
#include "qsqrt.h"
#include "quadtree.h"

struct iEngine;
struct iMaterialWrapper;
struct iObjectRegistry;
class csBCTerrObject;
class csBCTerrFactoryObject;

#define user_lod_start 2
#define cpu_time_limit 500
#define max_lod_distance 5000000000000000000.0

/*
 * csSharedLODMesh
 * only created and set once by Bezier Curve Factory
 */
class csSharedLODMesh
{
public:
  iVertexBuffer* buf;
  csVector3* verts;
  csVector3* normals;
  csVector2* texels;
  G3DTriangleMesh* mesh;
  csColor* color;
  // bool free;
  unsigned char level;
  // end = start of shared vertices = x_verts * z_verts
  int x_verts, z_verts; // end * 2 = triangle end
  int num_verts;
  void CreateMesh (int x_verts, int z_verts, int edge_res, unsigned int level );
  csSharedLODMesh ();
  ~csSharedLODMesh ();
};


class csBCTerrBlock
{
public:
  csVector3* controlpoint; // control point upper left
  iMaterialWrapper* material;
  csBox3 bbox;
  csSharedLODMesh* current_lod;
  csSharedLODMesh* default_lod;
  csBCTerrObject* owner;

  G3DTriangleMesh draw_mesh;
  csTriangle small_tri[2];
  csTriangle* large_tri;
  iVertexBuffer* buf;
  csVector3* verts;
  csVector3* normals;
  csVector2* texels;
  csColor* color;

  //bool vis;
  char max_LOD; // max user lod
  //unsigned char lod_level;
  // shared edges in Vertex Buffers
  // starts at LOD->end and then defines each end values for each side
  // control points are included in the mesh
  // todo: start them at 0!!!!
  int x1_end,  z1_end, x2_end, z2_end;
  void FreeLOD (); // got deleted / deleting self
  bool FreeSharedLOD (); // outside influence is taking LOD away
  /*
    Draw
    0 = current
    1 = default
    2 = largepolygons
    3 = small polygon
  */
  void Draw (iRenderView *rview, iCamera* camera, int level);
  void SetInfo (csBCTerrObject* nowner, csVector3* cntrl,
  	csBCTerrBlock* up_neighbor, csBCTerrBlock* left_neighbor);
  void Build (csVector3* cntrl,
  	csBCTerrBlock* up_neighbor, csBCTerrBlock* left_neighbor);
  void CreateNewMesh (int level);
  void AddEdgeTriangles ( csSharedLODMesh *lod);
  void AddEdgesToCurrent ();
  void ManagerClosed ();
  void SetupBaseMesh ();
  void AddMaterial (iMaterialWrapper* mat);
  void RebuildBlock (csBCTerrBlock* up_neighbor, csBCTerrBlock* left_neighbor);
  csBCTerrBlock ();
  ~csBCTerrBlock ();
};

struct BCPolyMesh : public iPolygonMesh
{
public:
  csMeshedPolygon* culling_mesh;
  bool culling;
  csVector3* square_verts;

  SCF_DECLARE_IBASE;
  BCPolyMesh ();
  virtual ~BCPolyMesh ();
  
  virtual int GetVertexCount ()
  {
    if (culling)
      return 4;
    else
      return 0;
  }
  virtual csVector3* GetVertices ()
  {
    if (culling)
      return square_verts;
    else
      return NULL;
  }
  virtual int GetPolygonCount ()
  {
    if (culling)
      return 1;
    else
      return 0;
  }
  virtual csMeshedPolygon* GetPolygons ()
  {
    if (culling)
      return culling_mesh;
    else
      return NULL;
  }
  virtual void Cleanup () 
  {
    return;
  }
};

/**
 *  csBCTerrObject : Bezier Curve Terrain Object
 */

class csBCTerrObject : public iMeshObject
{
private:
  void SetupControlPoints (iImage* im);
  int GetHeightFromImage (iImage* im, float x, float z);
  void FlattenSides ();
  void BuildCullMesh ();
public:
  // todo: clean this up?
  iObjectRegistry* object_reg;
  iBase* logparent;
  iMeshObjectFactory* pFactory;
  iMeshObjectDrawCallback* vis_cb;
  iBCTerrFactoryState* factory_state;
  iVertexBufferManager *vbufmgr;
  csBox3 bbox;
  csVector3 radius;

  // flatten info
  bool btop, bright, bdown, bleft, initheight;
  // flatten to this height if not colinearally
  float  toph, righth, downh, lefth;
  
  // correctseams info
  int correct_tw, correct_th;
  float correct_du, correct_su, correct_dv, correct_sv;

  BCPolyMesh culling_mesh;
  bool vis;
  int sys_inc;

  csVector3* control_points;
  csBCCollisionQuad *collision;
  csBCTerrBlock* blocks;

  csVector3 topleft;
  int x_blocks, z_blocks, hor_length;
  bool initialized, prebuilt;

  csBCTerrObject (iObjectRegistry* object_reg, iMeshObjectFactory* factory);
  virtual ~csBCTerrObject ();

  void SetHeightMap (iImage* im);
  void SetupMesh ();
  void SetupCollisionQuads ();
  bool ComputeSharedMesh (csSharedLODMesh* mesh, csVector3* cntrl_pts);
  void SetupVertexBuffer (iVertexBuffer *&vbuf1, iVertexBuffer *&vbuf2);
  void GetRadius (csVector3& rad, csVector3& cent);
  int HeightTest (csVector3 *point);
  int HeightTestExt (csVector3 *point);
  void FreeSharedLOD (const csVector3 point);
  void PreBuild ();
  void SetControlPoint (const csVector3 point, const int iter);
  void SetControlPointHeight (const float height, const int iter);
  void CorrectSeams (int tw, int th);
  void Build ();
  void RebuildBlocks ();


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

  virtual void NextFrame (csTicks ticks)
  {
    if (factory_state) factory_state->AddTime (ticks);
  }
  virtual bool WantToDie () const { return false; }

  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }
  virtual iBase* GetLogicalParent () const { return logparent; }

  virtual bool HitBeamOutline (const csVector3& start, const csVector3& end,
    csVector3& isect, float* pr);
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
    csVector3& isect, float* pr);


  //------------------------- iBCTerrState implementation ----------------
  class BCTerrState : public iBCTerrState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csBCTerrObject);
    virtual void SetSize ( int x, int z)
    {
      if (!scfParent->initialized && !scfParent->prebuilt)
      {
        scfParent->x_blocks = x;
        scfParent->z_blocks = z;
      }
    }
    virtual void SetTopLeftCorner (const csVector3& topleft)
    {
      if (!scfParent->initialized)
      {
        scfParent->topleft = topleft;
      }
    }
    virtual void SetBlockMaterial (int x_block, int z_block,
    	iMaterialWrapper* mat )
    {
      if (scfParent->initialized)
      {
        if ( (x_block > 0) && (x_block <= scfParent->x_blocks) )
        {
          if ( (z_block > 0) && (z_block <= scfParent->z_blocks) )
          {
            int size;
            size = scfParent->x_blocks * (z_block - 1) + x_block - 1;
            if (size < (scfParent->x_blocks * scfParent->z_blocks))
              scfParent->blocks[size].AddMaterial(mat);
          }
        }
      }
    }
    virtual void SetBlockMaterialNum (int num, iMaterialWrapper* mat)
    {
      if (scfParent->initialized)
      {
        int size;
        size = scfParent->x_blocks * scfParent->z_blocks;
        if ( (num < size) && (num > -1) )
        {
          scfParent->blocks[num].AddMaterial(mat);
        }
      }
    }
    virtual void SetHeightMap (iImage* im)
    {
      if (!scfParent->initialized)
      {
        scfParent->SetHeightMap (im);
      }
    }
    virtual int HeightTest (csVector3 *point)
    {
      if (!scfParent->initialized) return 0;
      return scfParent->HeightTest (point);
    }
    virtual int CameraHeightTest (csVector3 *point)
    {
      return 0;
    }
    virtual void SetControlPoint (const csVector3 point, const int iter)
    {
      scfParent->SetControlPoint (point, iter);
    }
    virtual void SetControlPoint (const csVector3 point, const int x,
          const int z) 
    {
      int size;
      size = ((z - 1)* scfParent->hor_length) + (x - 1); 
      scfParent->SetControlPoint (point, size);
    }
    virtual void SetControlPointHeight (const float height, const int iter) 
    {
      scfParent->SetControlPointHeight (height, iter);
    }
    virtual void SetControlPointHeight (const float height, const int x,
          const int z)
    {
      int size;
      size = ((z - 1)* scfParent->hor_length) + (x - 1);
      scfParent->SetControlPointHeight (height, size);
    }
    virtual void SetFlattenHeight (const float up, const float down,
          const float left, const float right)
    {
      scfParent->initheight = true;
      scfParent->toph = up;
      scfParent->righth = right;
      scfParent->downh = down;
      scfParent->lefth = left;
    }
    virtual void DoFlatten (const bool up, const bool down,
          const bool left, const bool right) 
    {
      scfParent->btop = up;
      scfParent->bdown = down;
      scfParent->bleft = left;
      scfParent->bright = right;
    }
    virtual void SetSystemInc (const int inc) 
    {
      if (inc > 4)
        scfParent->sys_inc = inc; 
    }
    virtual void PreBuild ()
    {
      scfParent->PreBuild ();
    }
    virtual void Build ()
    {
      scfParent->Build ();
    }
    virtual int GetControlLength ()
    {
      return (scfParent->x_blocks * scfParent->z_blocks);
    }
    virtual bool GetControlPoint (int iter, csVector3 &point)
    {
      int end;
      end = scfParent->x_blocks * scfParent->z_blocks;
      if ( (iter > -1) && (iter < end) )
      {
        point = scfParent->control_points[iter];
        return true;
      } else
        return false;
    }
  } scfiBCTerrState;
  
  //------------------------- iObjectModel implementation ----------------
  class BCTerrModel : public iObjectModel
  {
    SCF_DECLARE_EMBEDDED_IBASE (csBCTerrObject);
    virtual long GetShapeNumber () const { return 1; }
    virtual iPolygonMesh* GetPolygonMesh ()
    {
      return (iPolygonMesh*)&scfParent->culling_mesh;
    }
    virtual iPolygonMesh* GetSmallerPolygonMesh ()
    {
      return (iPolygonMesh*)&scfParent->culling_mesh;
    }
    virtual csPtr<iPolygonMesh> CreateLowerDetailPolygonMesh (float detail)
    { return csPtr<iPolygonMesh> (NULL); }
    virtual void GetObjectBoundingBox (csBox3& bbox,
        int type = CS_BBOX_NORMAL)
    {
      bbox = scfParent->bbox;
    }
    virtual void GetRadius (csVector3& radius, csVector3& center)
    {
      scfParent->GetRadius (radius, center);
    }
  } scfiObjectModel;
  
  ///--------------------- iMeshObject implementation ---------------------
  virtual iObjectModel* GetObjectModel () { return &scfiObjectModel;}

  //------------------------- iTerrFuncState implementation ----------------
  // only here for walktest use
  class TerrFuncState : public iTerrFuncState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csBCTerrObject);
    virtual void LoadMaterialGroup (iLoaderContext* ldr_context,
    	const char *pName, int iStart, int iEnd)
    {
      return;
    }
    virtual void SetTopLeftCorner (const csVector3& topleft)
    {
      return;
    }
    virtual csVector3 GetTopLeftCorner ()
    {
      return scfParent->topleft;
    }
    virtual void SetScale (const csVector3& scale)
    {
      return;
    }
    virtual csVector3 GetScale ()
    {
      csVector3 nreturn;
      return nreturn;
    }
    virtual void SetResolution (int x, int y)
    {
      return;
    }
    virtual int GetXResolution ()
    {
      return 0;
    }
    virtual int GetYResolution ()
    {
      return 0;
    }
    virtual void SetGridResolution (int x, int y)
    {
      return;
    }
    virtual int GetXGridResolution ()
    {
      return 0;
    }
    virtual int GetYGridResolution ()
    {
      return 0;
    }
    virtual void SetColor (const csColor& col)
    {
      return;
    }
    virtual csColor GetColor () const
    {
      csColor ncolor;
      return ncolor;
    }
    virtual void SetHeightFunction (iTerrainHeightFunction* func)
    {
      return;;
    }
    virtual void SetNormalFunction (iTerrainNormalFunction* func)
    {
      return;;
    }
    virtual void SetHeightMap (iImage* im, float hscale, float hshift)
    {
      return;
    }
    virtual iTerrainHeightFunction* GetHeightFunction () const
    {
      return NULL;
    }
    virtual iTerrainNormalFunction* GetNormalFunction () const
    {
      return NULL;
    }
    virtual void SetLODDistance (int lod, float dist)
    {
      return;
    }
    virtual float GetLODDistance (int lod)
    {
      return 0;
    }
    virtual void SetMaximumLODCost (int lod, float maxcost)
    {
      return;
    }
    virtual float GetMaximumLODCost (int lod)
    {
      return 0;
    }
    virtual void CorrectSeams (int tw, int th)
    {
      scfParent->CorrectSeams (tw, th);
      return;
    }
    virtual void GetCorrectSeams (int& tw, int& th) const
    {
      return;
    }
    virtual void SetQuadDepth (int qd)
    {
      return;
    }
    virtual int GetQuadDepth () const
    {
      return 0;
    }
    virtual void SetVisTesting (bool en)
    {
      return;
    }
    virtual bool IsVisTestingEnabled ()
    {
      return false;
    }
    virtual void SetDirLight (const csVector3& pos, const csColor& col)
    {
      return;
    }
    virtual csVector3 GetDirLightPosition () const
    {
      csVector3 dir;
      return dir;
    }
    virtual csColor GetDirLightColor () const
    {
      csColor dir;
      return dir;
    }
    virtual void DisableDirLight ()
    {
      return;
    }
    virtual bool IsDirLightEnabled () const
    {
      return false;
    }
    virtual void SetMaterial (int i, iMaterialWrapper* mat)
    {
      return;
    }
    virtual int GetMaterialCount () const
    {
      return 0;
    }
    virtual int CollisionDetect (csTransform *p)
    {
      int hits;
      csVector3 new_point = p->GetOrigin ();
      hits = scfParent->HeightTestExt (&new_point);
      p->SetOrigin (new_point);
      return hits;
    }

  } scfiTerrFuncState;
  friend class TerrFuncState;

 
  ///--------- interface to receive state of vertexbuffermanager-----------
  struct eiVertexBufferManagerClient : public iVertexBufferManagerClient
  {
    SCF_DECLARE_EMBEDDED_IBASE (csBCTerrObject);
    virtual void ManagerClosing ();
  } scfiVertexBufferManagerClient;

private:
  /*void SetAllVisible ()
  {
    if (blocks)
    {
      int n = x_blocks * z_blocks;
      for (int i = 0; i < n; i++)
      {
        blocks[i].vis = true;
      }
    }
  }*/
};

class csBCLODOwner
{
public:
  csBCTerrBlock** owners;
  int size;
  csBCLODOwner (int size);
  ~csBCLODOwner ();
};


/**
 * csBCTerrObjectFactory : Factory to create Bezier Curve Terrain Objects
 */
class csBCTerrObjectFactory : public iMeshObjectFactory
{
private:
  iBase* logparent;
  void GetXZFromLOD ( int level, int &x_vert, int &z_vert);
  void ResetOwner ()
  {
    last_level = 0;
    last_owner = 0;
  }
  void AddTerrObject (csBCTerrObject* obj);

public:
  iObjectRegistry *object_reg;
  int edge_res; // # of shared vertices per block edge, includes control points
  csVector2 blocksize;
  float height_multiplier;
  csBCTerrObject** object_list;
  int num_objects;
  bool free_lods;

  // a pointer to an array of pointers to shared_mesh type
  // this is used to share resources and save memory
  csSharedLODMesh** Shared_Meshes;
  csVector2* LOD_UV;
  float* LOD_Distance;
  float sys_distance, start_sys;
  int LOD_Levels;
  int* LOD_Mesh_Numbers; // # of shared meshes per LOD_Level
  bool initialized; // true after all variables are set
  iMaterialWrapper* default_mat;

  csBCLODOwner** owners;
  int last_level, last_owner;
  csVector3 focus;
  csTicks time;
  int time_count;

  csBCTerrObjectFactory (iObjectRegistry* object_reg);
  virtual ~csBCTerrObjectFactory ();

  // AddLOD sorts LOD levels by distance, sets all LOD info except shared meshes
  void AddLOD (float distance, int inc);
  csSharedLODMesh* GetSharedMesh ( int level, csBCTerrBlock *owner);
  void FreeShared (csBCTerrBlock *owner, int level  );
  void CheckShared ();
  csSharedLODMesh* CreateFreeMesh ();

  SCF_DECLARE_IBASE;

  virtual csPtr<iMeshObject> NewInstance ();

  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }
  virtual iBase* GetLogicalParent () const { return logparent; }
  //------------------------- iBCTerrFactoryState implementation ----------------
  class BCTerrFactoryState : public iBCTerrFactoryState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csBCTerrObjectFactory);
    virtual void SetMaxEdgeResolution ( int res)
    {
      if (scfParent->initialized) return;
      if (res > 0) scfParent->edge_res = res;
    }
    virtual int GetMaxEdgeResolution ()
    {
      return scfParent->edge_res;
    }
    virtual void SetBlockSize ( float x, float z)
    {
      if (scfParent->initialized) return;
      scfParent->blocksize.x = x;
      scfParent->blocksize.y = z;
    }
    virtual void AddLOD ( float distance, int increments)
    {
      if (!scfParent->initialized) scfParent->AddLOD (distance, increments);
    }
    virtual void SetLODDistance ( int lod_level, float distance)
    {
      if ( (lod_level <= scfParent->LOD_Levels) && (lod_level > 0) )
      {
        lod_level -= 1;
        scfParent->LOD_Distance[lod_level] = distance * distance;
      }
    }
    virtual void SetMultiplier ( float m )
    {
      scfParent->height_multiplier = m;
    }
    virtual float GetMultiplier ()
    {
      return scfParent->height_multiplier;
    }
    virtual csSharedLODMesh* GetSharedMesh (int level, csBCTerrBlock *owner)
    {
      return scfParent->GetSharedMesh (level, owner);
    }
    virtual void FreeShared (csBCTerrBlock *owner, int level )
    {
      scfParent->FreeShared (owner, level);
    }
    virtual void AddTime (csTicks addtime)
    {
      scfParent->time_count += 1;
      if (scfParent->time_count == scfParent->num_objects)
      { 
        scfParent->time += addtime;
        scfParent->time_count = 0;
        scfParent->free_lods = true;
      }
      //scfParent->CheckShared ();
    }
    virtual csVector2* GetSize ()
    {
      return &scfParent->blocksize;
    }
    virtual csSharedLODMesh* CreateFreeMesh (bool wavy)
    {
      return scfParent->CreateFreeMesh ();
    }
    virtual void SetFocusPoint (const csVector3 nfocus)
    {
      scfParent->focus = nfocus;
    }
    virtual csVector2* GetLODUV (int lod_level)
    {
      if ((lod_level >= scfParent->LOD_Levels) || (lod_level < 0) ) return NULL;
      return &scfParent->LOD_UV[lod_level];
    }
    virtual void SetDefaultMaterial (iMaterialWrapper* mat)
    {
      scfParent->default_mat = mat;
    }
    virtual iMaterialWrapper* GetDefaultMaterial ()
    {
      return scfParent->default_mat;
    }
    virtual int GetUserLOD ()
    {
      return scfParent->LOD_Levels;
    }
    virtual float* GetLODDistances ()
    {
      return scfParent->LOD_Distance;
    }
    // start_sys
    virtual void GetSystemDistance (float &start, float &dist)
    {
      start = scfParent->start_sys;
      dist = scfParent->sys_distance;
    }
    virtual void SetSystemDistance (float start, float new_dist)
    {
      scfParent->sys_distance = new_dist * new_dist;
      scfParent->start_sys = start * start;
    }
  } scfiBCTerrFactoryState;
  friend class BCTerrFactoryState;
};


/**
 * TerrFunc type. This is the plugin you have to use to create instances
 * of csBCTerrObjectFactory.
 */
class csBCTerrObjectType : public iMeshObjectType
{
private:
  iObjectRegistry *object_reg;

public:
  SCF_DECLARE_IBASE;

  csBCTerrObjectType (iBase*);

  virtual ~csBCTerrObjectType ();

  virtual csPtr<iMeshObjectFactory> NewFactory ();

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csBCTerrObjectType);
    virtual bool Initialize (iObjectRegistry* p)
    { scfParent->object_reg = p; return true; }
  } scfiComponent;
  friend struct eiComponent;
};

extern csVector3 BezierCompute ( float u, csVector3* temp);
extern csVector3 BezierControlCompute (float u, csVector3* cntrl, int width);

#endif

