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
  csTriangle large_tri[18];
  iVertexBuffer* buf;
  csVector3 verts[16];
  csVector3 normals[16];
  csVector2 texels[16];
  csColor color[16];

  bool vis;
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
  void CreateNewMesh (int level);
  void AddEdgeTriangles ( csSharedLODMesh *lod);
  void AddEdgesToCurrent ();
  void ManagerClosed ();
  void SetupBaseMesh ();
  csBCTerrBlock ();
  ~csBCTerrBlock ();
};

/**
 *  csBCTerrObject : Bezier Curve Terrain Object
 */

class csBCTerrObject : public iMeshObject
{
private:
  void SetupControlPoints (iImage* im);
  int GetHeightFromImage (iImage* im, float x, float z);
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

  csVector3* control_points;
  csBCTerrBlock* blocks;

  csVector3 topleft;
  int x_blocks, z_blocks, hor_length;
  bool initialized;

  csBCTerrObject (iObjectRegistry* object_reg, iMeshObjectFactory* factory);
  virtual ~csBCTerrObject ();

  void SetHeightMap (iImage* im);
  void SetupMesh ();
  bool ComputeSharedMesh (csSharedLODMesh* mesh, csVector3* cntrl_pts);
  void SetupVertexBuffer (iVertexBuffer *&vbuf1, iVertexBuffer *&vbuf2);
  void GetRadius (csVector3& rad, csVector3& cent);

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
      if (!scfParent->initialized)
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
              scfParent->blocks[size].material = mat;
          }
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
  } scfiBCTerrState;
  //------------------------- iObjectModel implementation ----------------
  class BCTerrModel : public iObjectModel
  {
    SCF_DECLARE_EMBEDDED_IBASE (csBCTerrObject);
    virtual long GetShapeNumber () const { return 1; }
    virtual iPolygonMesh* GetPolygonMesh ()
    {
      return NULL;
    }
    virtual iPolygonMesh* GetSmallerPolygonMesh ()
    {
      return NULL;
    }
    virtual iPolygonMesh* CreateLowerDetailPolygonMesh (float detail)
    {
      return NULL;
    }
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

  virtual iObjectModel* GetObjectModel () { return &scfiObjectModel;}

  /// interface to receive state of vertexbuffermanager
  struct eiVertexBufferManagerClient : public iVertexBufferManagerClient
  {
    SCF_DECLARE_EMBEDDED_IBASE (csBCTerrObject);
    virtual void ManagerClosing ();
  } scfiVertexBufferManagerClient;

private:
  void SetAllVisible ()
  {
    if (blocks)
    {
      int n = x_blocks * z_blocks;
      for (int i = 0; i < n; i++)
      {
        blocks[i].vis = true;
      }
    }
  }
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

public:
  iObjectRegistry *object_reg;
  int edge_res; // # of shared vertices per block edge, includes control points
  csVector2 blocksize;
  float height_multiplier;

  // a pointer to an array of pointers to shared_mesh type
  // this is used to share resources and save memory
  csSharedLODMesh** Shared_Meshes;
  csVector2* LOD_UV;
  float* LOD_Distance;
  float sys_distance;
  int LOD_Levels;
  int* LOD_Mesh_Numbers; // # of shared meshes per LOD_Level
  bool initialized; // true after all variables are set
  iMaterialWrapper* default_mat;

  csBCLODOwner** owners;
  int last_level, last_owner;
  csTicks time;
  csVector3* focus;

  csBCTerrObjectFactory (iObjectRegistry* object_reg);
  virtual ~csBCTerrObjectFactory ();

  // AddLOD sorts LOD levels by distance, sets all LOD info except shared meshes
  void AddLOD (float distance, int inc);
  csSharedLODMesh* GetSharedMesh ( int level, csBCTerrBlock *owner);
  void FreeShared (csBCTerrBlock *owner, int level  );
  void CheckShared ();
  csSharedLODMesh* CreateFreeMesh ();

  SCF_DECLARE_IBASE;

  virtual iMeshObject* NewInstance ();

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
      scfParent->time += addtime;
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
    virtual void SetFocusPoint (csVector3* nfocus)
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
    virtual float GetSystemDistance ()
    {
      return scfParent->sys_distance;
    }
    virtual void SetSystemDistance (float new_dist)
    {
      scfParent->sys_distance = new_dist * new_dist;
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

  virtual iMeshObjectFactory* NewFactory ();

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csBCTerrObjectType);
    virtual bool Initialize (iObjectRegistry* p)
    { scfParent->object_reg = p; return true; }
  } scfiComponent;
  friend struct eiComponent;
};


#endif

