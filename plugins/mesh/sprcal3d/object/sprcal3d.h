/*
    Copyright (C) 2003 by Keith Fulton

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

#ifndef __CS_SPRCAL3D_H__
#define __CS_SPRCAL3D_H__

#include "cssys/sysfunc.h"
#include "csutil/cscolor.h"
#include "csutil/parray.h"
#include "csutil/garray.h"
#include "csutil/refarr.h"
#include "csutil/hash.h"
#include "csgeom/math3d.h"
#include "csgeom/math2d.h"
#include "csgeom/poly2d.h"
#include "csgeom/poly3d.h"
#include "csgeom/box.h"
#include "csgeom/objmodel.h"
#include "csgfx/shadervarcontext.h"
#include "imesh/spritecal3d.h"
#include "imesh/object.h"
#include "imesh/lighting.h"
#include "iengine/dynlight.h"
#include "iengine/material.h"
#include "iengine/light.h"
#include "iengine/lod.h"
#include "iengine/statlght.h"
#include "iutil/config.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/virtclk.h"
#include "ivideo/graph3d.h"
#include "ivideo/vbufmgr.h"
#include "ivideo/rendermesh.h"
#include "ivideo/rndbuf.h"
#include "cstool/anonrndbuf.h"
#include "ivideo/material.h"
#include "qint.h"

#include <cal3d/cal3d.h>

struct iObjectRegistry;
struct iEngine;
struct iMaterialWrapper;

#define ALL_LOD_FEATURES (CS_LOD_TRIANGLE_REDUCTION|CS_LOD_DISTANCE_REDUCTION)

struct csCal3DAnimation
{
    int      index;
    csString name;
    int	     type;
    float    base_velocity;
    float    min_velocity;
    float    max_velocity;
    int      min_interval;
    int      max_interval;
    int      idle_pct;
};

struct csCal3DMesh
{
    int		      index;
    csString	      name;
    bool	      attach_by_default;
    iMaterialWrapper *default_material;
    csArray<csString> morph_target_name;
};

/**
 * A socket for specifying where sprites can plug into
 * other sprites.
 */
class csSpriteCal3DSocket : public iSpriteCal3DSocket
{
  private:
    char* name;
    int triangle_index;
    int submesh_index;
    int mesh_index;
    iMeshWrapper *attached_mesh;

  public:

    /// Default Constructor
    csSpriteCal3DSocket();

    virtual ~csSpriteCal3DSocket ();

    /// Set the name.
    virtual void SetName (char const*);
    /// Get the name.
    virtual char const* GetName () const { return name; }

    /// Set the attached sprite.
    virtual void SetMeshWrapper (iMeshWrapper* mesh) {attached_mesh = mesh;}
    /// Get the attached sprite.
    virtual iMeshWrapper* GetMeshWrapper () const {return attached_mesh;}

    /// Set the index of the triangle for the socket.
    virtual void SetTriangleIndex (int tri_index) { triangle_index = tri_index; }
    /// Get the index of the triangle for the socket.
    virtual int GetTriangleIndex () const { return triangle_index; }

    /// Set the index of the submesh for the socket.
    virtual void SetSubmeshIndex (int subm_index) { submesh_index = subm_index;}
    /// Get the index of the submesh for the socket.
    virtual int GetSubmeshIndex () const { return submesh_index;}

    /// Set the index of the mesh for the socket.
    virtual void SetMeshIndex (int m_index) { mesh_index = m_index;}
    /// Get the index of the mesh for the socket.
    virtual int GetMeshIndex () const {return mesh_index;}

    SCF_DECLARE_IBASE;
};

class csSpriteCal3DMeshObject;

/**
 * A Cal3D sprite based on a triangle mesh with a single texture.
 * Animation is done with frames.
 * This class represents a template from which a csSpriteCal3D
 * class can be made.
 */
class csSpriteCal3DMeshObjectFactory : public iMeshObjectFactory
{
private:
  friend class csSpriteCal3DMeshObject;

  /// Material handle as returned by iTextureManager.
  iBase* logparent;

  /// If true then this factory has been initialized.
  bool initialized;

  /// This is the factory equivalent class in cal3d.
  CalCoreModel calCoreModel;
  csArray<csCal3DAnimation*> anims;
  csArray<csCal3DMesh*>      submeshes;
  csArray<csString>          morph_animation_names;

  csString     basePath;
  float	       renderScale;

  /// The sockets.
  csPDelArray<csSpriteCal3DSocket> sockets;

public:
  iObjectRegistry* object_reg;
  iVirtualClock* vc;

  csRef<iGraphics3D> g3d;

  /**
   * Reference to the engine (optional because sprites can also be
   * used for the isometric engine).
   */
  iEngine* engine;
  
  static csStringID vertex_name, texel_name, normal_name, color_name, 
    index_name;

  /// Create the sprite template.
  csSpriteCal3DMeshObjectFactory (iBase *pParent, iObjectRegistry* object_reg);
  /// Destroy the template.
  virtual ~csSpriteCal3DMeshObjectFactory ();

  void Report (int severity, const char* msg, ...);

  //------------------------iSpriteCal3dFactoryState implementation --------
  /// Create a new core object.
  bool Create(const char *name);
  void ReportLastError ();
  void SetLoadFlags(int flags);
  void SetBasePath(const char *path);
  void SetRenderScale(float scale) { renderScale=scale; }
  float GetRenderScale() { return renderScale; }
  bool LoadCoreSkeleton(iVFS *vfs,const char *filename);
  int  LoadCoreAnimation(iVFS *vfs,const char *filename,const char *name,int type,float base_vel,
                         float min_vel,float max_vel,int min_interval,int max_interval,int idle_pct);
  int LoadCoreMesh(iVFS *vfs,const char *filename,const char *name,bool attach,iMaterialWrapper *defmat);
  int LoadCoreMorphTarget(iVFS *vfs,int mesh_index,const char *filename,const char *name);
  int AddMorphAnimation(const char *name);
  bool AddMorphTarget(int morphanimation_index,const char *mesh_name, const char *morphtarget_name);
  bool AddCoreMaterial(iMaterialWrapper *mat);
  void BindMaterials();

  int  GetMeshCount() { return submeshes.Length(); }
  int GetMorphAnimationCount() { return morph_animation_names.Length(); }
  int GetMorphTargetCount(int mesh_id);
  const char *GetMeshName(int idx);
  int  FindMeshName(const char *meshName);
  const char *GetMorphAnimationName(int idx);
  int  FindMorphAnimationName(const char *meshName);
  bool IsMeshDefault(int idx);

  /// Create and add a new socket to the sprite.
  csSpriteCal3DSocket* AddSocket ();
  /// find a named socket into the sprite.
  csSpriteCal3DSocket* FindSocket (const char * name) const;
  /// find a socked based on the sprite attached to it
  csSpriteCal3DSocket* FindSocket (iMeshWrapper *mesh) const;
  /// Query the number of sockets
  int GetSocketCount () const { return sockets.Length (); }
  /// Query the socket number f
  csSpriteCal3DSocket* GetSocket (int f) const
  {
    return (f < sockets.Length ())
      ? (csSpriteCal3DSocket *)sockets [f]
      : (csSpriteCal3DSocket*)0;
  }


  //------------------------ iMeshObjectFactory implementation --------------
  SCF_DECLARE_IBASE;

  virtual csPtr<iMeshObject> NewInstance ();
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () const { return true; }
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }
  virtual iBase* GetLogicalParent () const { return logparent; }


  //------------------ iPolygonMesh interface implementation ----------------//

  int GetVertexCount() { return 0; }
  csVector3* GetVertices () { return 0; }
  int GetTriangleCount () { return 0; }
  csMeshedPolygon* GetPolygons () { return 0; }
  csTriangle* GetTriangles () { return 0; }

  struct PolyMesh : public iPolygonMesh
  {
  private:
    csSpriteCal3DMeshObjectFactory* factory;
    csFlags flags;

  public:
    SCF_DECLARE_IBASE;

    void SetFactory (csSpriteCal3DMeshObjectFactory* Factory)
    {
      factory = Factory;
    }

    virtual int GetVertexCount ()
    {
      return factory->GetVertexCount ();
    }
    virtual csVector3* GetVertices ()
    {
      return factory->GetVertices ();
    }
    virtual int GetPolygonCount ()
    {
      return factory->GetTriangleCount ();
    }
    virtual csMeshedPolygon* GetPolygons ()
    {
      return factory->GetPolygons();
    }
    virtual int GetTriangleCount ()
    {
      return factory->GetTriangleCount ();
    }
    virtual csTriangle* GetTriangles ()
    {
      return factory->GetTriangles();
    }

    virtual void Lock () { }
    virtual void Unlock () { }

    virtual csFlags& GetFlags () { return flags;  }
    virtual uint32 GetChangeNumber() const { return 0; }

    PolyMesh () : polygons (0)
    {
      SCF_CONSTRUCT_IBASE (0);
      flags.Set (CS_POLYMESH_TRIANGLEMESH);
    }
    virtual ~PolyMesh ()
    {
      Cleanup ();
      SCF_DESTRUCT_IBASE ();
    }
    void Cleanup () { } //  delete[] polygons; polygons = 0; }

    csMeshedPolygon* polygons;
  } scfiPolygonMesh;
  friend struct PolyMesh;

  //------------------------- iObjectModel implementation ----------------
  void GetObjectBoundingBox (csBox3& bbox, int type, csVector3 *verts,int vertCount);
  void GetObjectBoundingBox (csBox3& bbox, int type = CS_BBOX_NORMAL);
  void GetRadius (csVector3& rad, csVector3& cent);

  class ObjectModel : public csObjectModel
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObjectFactory);
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

  //--------------------- iSpriteCal3DFactoryState implementation -------------//
  struct SpriteCal3DFactoryState : public iSpriteCal3DFactoryState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObjectFactory);

    virtual bool Create(const char *name)
    { return scfParent->Create(name); }

    virtual void SetLoadFlags(int flags)
    {
      scfParent->SetLoadFlags(flags);
    }

    virtual void ReportLastError ()
    { scfParent->ReportLastError(); }

    virtual void SetBasePath(const char *path)
    { scfParent->SetBasePath(path); }

    virtual void SetRenderScale(float scale)
    { scfParent->SetRenderScale(scale); }
    virtual float GetRenderScale()
    { return scfParent->GetRenderScale(); }

    virtual bool LoadCoreSkeleton(iVFS *vfs,const char *filename)
    { return scfParent->LoadCoreSkeleton(vfs,filename); }

    virtual int LoadCoreAnimation(iVFS *vfs,const char *filename,const char *name,int type,float base_vel,float min_vel,float max_vel,int min_interval,int max_interval,int idle_pct)
    { return scfParent->LoadCoreAnimation(vfs,filename,name,type,base_vel,min_vel,max_vel,min_interval,max_interval,idle_pct); }

    virtual int LoadCoreMesh(iVFS *vfs,const char *filename,const char *name,bool attach,iMaterialWrapper *defmat)
    { return scfParent->LoadCoreMesh(vfs,filename,name,attach,defmat); }

    virtual int AddMorphAnimation(const char *name)
    { return scfParent->AddMorphAnimation(name); }
    
    virtual bool AddMorphTarget(int morphanimation_index,const char *mesh_name, const char *morphtarget_name)
    { return scfParent->AddMorphTarget(morphanimation_index,mesh_name, morphtarget_name); }
 
    virtual int LoadCoreMorphTarget(iVFS *vfs,int mesh_index,const char *filename,const char *name)
    { return scfParent->LoadCoreMorphTarget(vfs,mesh_index,filename,name); }
	    
    virtual bool AddCoreMaterial(iMaterialWrapper *mat)
    { return scfParent->AddCoreMaterial(mat); }

    virtual void BindMaterials()
    { scfParent->BindMaterials(); }

    virtual int  GetMeshCount()
    { return scfParent->GetMeshCount(); }

    virtual int GetMorphAnimationCount()
    { return scfParent->GetMorphAnimationCount();}

    virtual int GetMorphTargetCount(int mesh_id)
    { return scfParent->GetMorphTargetCount(mesh_id);}

    virtual const char *GetMeshName(int idx)
    { return scfParent->GetMeshName(idx); }

    virtual int  FindMeshName(const char *meshName)
    { return scfParent->FindMeshName(meshName); }
    
    virtual const char *GetMorphAnimationName(int idx)
    { return scfParent->GetMorphAnimationName(idx); }

    virtual int  FindMorphAnimationName(const char *meshName)
    { return scfParent->FindMorphAnimationName(meshName); }

    virtual bool IsMeshDefault(int idx)
    { return scfParent->IsMeshDefault(idx); }

    virtual iSpriteCal3DSocket* AddSocket ()
    {
      csRef<iSpriteCal3DSocket> ifr (
	  SCF_QUERY_INTERFACE_SAFE (scfParent->AddSocket (),
	    iSpriteCal3DSocket));
      return ifr;       // DecRef is ok here.
    }
    virtual iSpriteCal3DSocket* FindSocket (const char* name) const
    {
      csRef<iSpriteCal3DSocket> ifr (SCF_QUERY_INTERFACE_SAFE (
	    scfParent->FindSocket (name), iSpriteCal3DSocket));
      return ifr;       // DecRef is ok here.
    }
    virtual iSpriteCal3DSocket* FindSocket (iMeshWrapper* mesh) const
    {
      csRef<iSpriteCal3DSocket> ifr (SCF_QUERY_INTERFACE_SAFE (
	    scfParent->FindSocket (mesh), iSpriteCal3DSocket));
      return ifr;       // DecRef is ok here.
    }
    virtual int GetSocketCount () const
    {
      return scfParent->GetSocketCount ();
    }
    virtual iSpriteCal3DSocket* GetSocket (int f) const
    {
      csRef<iSpriteCal3DSocket> ifr (
	  SCF_QUERY_INTERFACE_SAFE (scfParent->GetSocket (f),
	    iSpriteCal3DSocket));
      return ifr;       // DecRef is ok here.
    }

  } scfiSpriteCal3DFactoryState;
  struct LODControl : public iLODControl
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObjectFactory);
    virtual void SetLOD (float, float)
    {
    }
    virtual void GetLOD (float& m, float& a) const
    {
      m = 0;
      a = 1;
    }
    virtual void SetLOD (iSharedVariable*, iSharedVariable*)
    {
    }
    virtual void GetLOD (iSharedVariable*& varm, iSharedVariable* &vara)
    {
      varm = 0;
      vara = 0;
    }
    virtual int GetLODPolygonCount (float /*lod*/) const
    {
      return 0;
    }
    virtual void GetLOD(iSharedVariable *& first,iSharedVariable *& second) const
    {
      first=0; second=0;
    }
  } scfiLODControl;
  friend struct LODControl;

};

/**
 * A Cal3D sprite based on a triangle mesh with a single texture.
 * Animation is done with frames (a frame may be controlled by
 * a skeleton).
 */
class csSpriteCal3DMeshObject : public iMeshObject
{
private:
  csRef<iObjectRegistry> object_reg;
  iMeshObjectDrawCallback* vis_cb;
  uint32 current_features;  // LOD Control thing
  iBase* logparent;
  CalModel calModel;
  float last_update_time;
  csArray<csCal3DAnimation*> active_anims;
  csArray<float>             active_weights;
  bool is_idling;
  float idle_override_interval;
  int   idle_action;

  /**
   * Each mesh must have its own individual socket assignments,
   * but the vector must be copied down from the factory at create time.
   */
  csPDelArray<csSpriteCal3DSocket> sockets;


#ifndef CS_USE_NEW_RENDERER
  iVertexBufferManager* vbufmgr;
  csRef<iVertexBuffer> vbuf;
//  csArray<G3DTriangleMesh>  mesh;
//  csArray<bool>             initialized;
//  csArray<csColor*>         mesh_colors;
  csArray<G3DTriangleMesh>  *meshes;
#else
  class BaseAccessor : public iShaderVariableAccessor
  {
  protected:
    csSpriteCal3DMeshObject* meshobj;
    int mesh, submesh;
    uint meshVersion;

    csRef<iRenderBuffer> vertex_buffer;
    int vertex_size;
    csRef<iRenderBuffer> texel_buffer;
    int texel_size;
    csRef<iRenderBuffer> normal_buffer;
    int normal_size;
    csRef<iRenderBuffer> color_buffer;
    int color_size;
    csRef<iRenderBuffer> index_buffer;
    int index_size;
  public:
    SCF_DECLARE_IBASE;

    BaseAccessor (csSpriteCal3DMeshObject* meshobj, int mesh, 
      int submesh)
    {
      SCF_CONSTRUCT_IBASE (0);
      BaseAccessor::meshobj = meshobj;
      BaseAccessor::mesh = mesh;
      BaseAccessor::submesh = submesh;    
      meshVersion = (uint)-1;
    }
    virtual ~BaseAccessor() 
    {
      SCF_DESTRUCT_IBASE ();
    }
    virtual void PreGetValue (csShaderVariable *variable);
  };

  bool rmeshesSetup;
  uint meshVersion;
  csRef<iStringSet> strings;
  csDirtyAccessArray<csRenderMesh*> allRenderMeshes;
  csArray<csArray<csRenderMesh> > renderMeshes;
  csRef<iGraphics3D> G3D;
  iMovable* currentMovable;
#endif
  bool arrays_initialized;
  csBox3 object_bbox;
  uint bboxVersion;
  csArray<csColor*>         *meshes_colors;
  csArray<int>		     attached_ids;
  csArray<bool>             *is_initialized;
  csSet<iLight*> affecting_lights;
  bool lighting_dirty;
  csColor dynamic_ambient;

  void SetupObject ();
  void SetupObjectSubmesh(int index);
  void SetIdleOverrides(csRandomGen *rng,int which);
  void RecalcBoundingBox(csBox3& bbox);

#ifndef CS_USE_NEW_RENDERER
  void SetupVertexBuffer (int mesh, int submesh, int num_vertices,
    int num_triangles, csTriangle *triangles);
  bool DrawSubmesh (iGraphics3D* g3d, iRenderView* rview, 
    CalRenderer *pCalRenderer, int mesh, int submesh, 
    iMaterialWrapper *material);
#else
  void SetupRenderMeshes ();
#endif
  void InitSubmeshLighting (int mesh, int submesh, CalRenderer *pCalRenderer,
    iMovable* movable);
  void UpdateLightingSubmesh (iLight* lights, iMovable* movable, 
    CalRenderer *pCalRenderer, int mesh, int submesh);
  void UpdateLighting (iMovable* movable, CalRenderer *pCalRenderer);

public:
  SCF_DECLARE_IBASE;

  /// The parent.
  csSpriteCal3DMeshObjectFactory* factory;

  /// Constructor.
  csSpriteCal3DMeshObject (iBase *pParent, iObjectRegistry* object_reg,
    CalCoreModel& calCoreModel);
  /// Destructor.
  virtual ~csSpriteCal3DMeshObject ();

  /// Set the factory.
  void SetFactory (csSpriteCal3DMeshObjectFactory* factory);

  /// Get the factory.
  csSpriteCal3DMeshObjectFactory* GetFactory3D () const { return factory; }

  // ------------------------------- Lighting --------------------------------
  void DynamicLightChanged (iDynLight* dynlight);
  void DynamicLightDisconnect (iDynLight* dynlight);
  void StaticLightChanged (iStatLight* statlight);
  void StaticLightDisconnect (iStatLight* statlight);

  //------------------------- iLightingInfo interface -------------------------
  struct LightingInfo : public iLightingInfo
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObject);
    virtual void InitializeDefault (bool clear)
    {
    }
    virtual bool ReadFromCache (iCacheManager* cache_mgr)
    {
      return true;
    }
    virtual bool WriteToCache (iCacheManager* cache_mgr)
    {
      return true;
    }
    virtual void PrepareLighting ()
    {
    }
    virtual void SetDynamicAmbientLight (const csColor& color)
    {
      scfParent->dynamic_ambient = color;
    }
    virtual const csColor& GetDynamicAmbientLight ()
    {
      return scfParent->dynamic_ambient;;
    }
    virtual void DynamicLightChanged (iDynLight* dynlight)
    {
      scfParent->DynamicLightChanged (dynlight);
    }
    virtual void DynamicLightDisconnect (iDynLight* dynlight)
    {
      scfParent->DynamicLightDisconnect (dynlight);
    }
    virtual void StaticLightChanged (iStatLight* statlight)
    {
      scfParent->StaticLightChanged (statlight);
    }
    virtual void StaticLightDisconnect (iStatLight* statlight)
    {
      scfParent->StaticLightDisconnect (statlight);
    }
  } scfiLightingInfo;
  friend struct LightingInfo;

  ///------------------------ iMeshObject implementation ----------------------
  virtual bool HitBeamOutline (const csVector3& start, const csVector3& end,
      csVector3& intersect, float* pr) { return false; }
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
      csVector3& intersect, float* pr, int* = 0) { return false; }

  virtual bool SetColor (const csColor& col)
  {
    //SetBaseColor (col);
    return true;
  }
  virtual bool GetColor (csColor& col) const
  {
    //GetBaseColor (col);
    return true;
  }
  virtual bool SetMaterialWrapper (iMaterialWrapper* mat)
  {
    //SetMaterial (mat);
    return true;
  }
  virtual iMaterialWrapper* GetMaterialWrapper () const
  {
    return 0; //GetMaterial ();
  }
  virtual void InvalidateMaterialHandles () { }
  /**
   * see imesh/object.h for specification. The default implementation
   * does nothing.
   */
  virtual void PositionChild (iMeshObject* child,csTicks current_time);

  virtual iMeshObjectFactory* GetFactory () const
  {
    csRef<iMeshObjectFactory> ifact (SCF_QUERY_INTERFACE (factory,
    	iMeshObjectFactory));
    return ifact;	// DecRef is ok here.
  }
  virtual bool DrawTest (iRenderView* rview, iMovable* movable);
  virtual csRenderMesh **GetRenderMeshes (int &n);
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
  virtual bool Advance (csTicks current_time);

  virtual void NextFrame (csTicks current_time,const csVector3& new_pos)
  {   
    Advance (current_time);
  }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }
  virtual iBase* GetLogicalParent () const { return logparent; }

  //------------------ iPolygonMesh interface implementation ----------------//
  struct PolyMesh : public iPolygonMesh
  {
  private:
    csFlags flags;

  public:
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObject);

    /// Get the number of vertices for this mesh.
    virtual int GetVertexCount ()
    {
      csSpriteCal3DMeshObjectFactory* fact = scfParent->GetFactory3D ();
      return fact->GetVertexCount ();
    }
    /// Get the pointer to the array of vertices.
    virtual csVector3* GetVertices ()
    {
      csSpriteCal3DMeshObjectFactory* fact = scfParent->GetFactory3D ();
      return fact->GetVertices ();
    }
    /// Get the number of polygons for this mesh.
    virtual int GetPolygonCount ()
    {
      csSpriteCal3DMeshObjectFactory* fact = scfParent->GetFactory3D ();
      return fact->GetTriangleCount ();
    }
    virtual int GetTriangleCount ()
    {
      csSpriteCal3DMeshObjectFactory* fact = scfParent->GetFactory3D ();
      return fact->GetTriangleCount ();
    }
    virtual csTriangle* GetTriangles ()
    {
      csSpriteCal3DMeshObjectFactory* fact = scfParent->GetFactory3D ();
      return fact->GetTriangles();
    }

    /// Get the pointer to the array of polygons.
    virtual csMeshedPolygon* GetPolygons ();

    virtual void Lock () { }
    virtual void Unlock () { }
    
    virtual csFlags& GetFlags () { return flags;  }
    virtual uint32 GetChangeNumber() const { return 0; }

    PolyMesh () : polygons (0)
    {
      flags.Set (CS_POLYMESH_TRIANGLEMESH);
    }
    virtual ~PolyMesh () { Cleanup (); }
    void Cleanup () { delete[] polygons; polygons = 0; }

    csMeshedPolygon* polygons;
  } scfiPolygonMesh;
  friend struct PolyMesh;

  virtual iObjectModel* GetObjectModel () { return &scfiObjectModel; }

  //--------------------- iSpriteCal3DState implementation -------------//
  int GetAnimCount();
  const char *GetAnimName(int idx);
  int  GetAnimType(int idx);
  int  FindAnim(const char *name);
  void ClearAllAnims();
  bool SetAnimCycle(const char *name, float weight);
  bool AddAnimCycle(const char *name, float weight, float delay);
  bool AddAnimCycle(int idx, float weight, float delay);
  void ClearAnimCycle(int idx, float delay);
  bool ClearAnimCycle(const char *name, float delay);
  int  GetActiveAnimCount();
  int  GetActiveAnims(char *buffer,int max_length);
  void SetActiveAnims(const char *buffer,int anim_count);
  bool SetAnimAction(const char *name, float delayIn, float delayOut);
  bool SetVelocity(float vel,csRandomGen *rng=0);
  void SetLOD(float lod);
  
  bool AttachCoreMesh(const char *meshname);
  bool AttachCoreMesh(int mesh_id,int iMatWrapID);
  bool DetachCoreMesh(const char *meshname);
  bool DetachCoreMesh(int mesh_id);

  bool BlendMorphTarget(int morph_animation_id, float weight, float delay);
  bool ClearMorphTarget(int morph_animation_id, float delay);

  /// Create and add a new socket to the sprite.
  csSpriteCal3DSocket* AddSocket ();
  /// find a named socket into the sprite.
  csSpriteCal3DSocket* FindSocket (const char * name) const;
  /// find a socked based on the sprite attached to it
  csSpriteCal3DSocket* FindSocket (iMeshWrapper *mesh) const;
  /// Query the number of sockets
  int GetSocketCount () const { return sockets.Length (); }
  /// Query the socket number f
  csSpriteCal3DSocket* GetSocket (int f) const
  {
    return (f < sockets.Length ())
      ? (csSpriteCal3DSocket *)sockets [f]
      : (csSpriteCal3DSocket*)0;
  }


  struct SpriteCal3DState : public iSpriteCal3DState
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSpriteCal3DMeshObject);

    virtual int GetAnimCount()
    {
	return scfParent->GetAnimCount();
    }
    virtual const char *GetAnimName(int idx)
    {
	return scfParent->GetAnimName(idx);
    }
    virtual int GetAnimType(int idx)
    {
	return scfParent->GetAnimType(idx);
    }

    virtual void ClearAllAnims()
    {
	scfParent->ClearAllAnims();
    }

    virtual bool SetAnimCycle(const char *name, float weight)
    {
	return scfParent->SetAnimCycle(name,weight);
    }
    virtual bool AddAnimCycle(const char *name, float weight, float delay)
    {
	return scfParent->AddAnimCycle(name,weight,delay);
    }
    bool AddAnimCycle(int idx, float weight, float delay)
    {
      return scfParent->AddAnimCycle(idx,weight,delay);
    }
    virtual bool ClearAnimCycle(const char *name, float delay)
    {
	return scfParent->ClearAnimCycle(name,delay);
    }
    virtual int  GetActiveAnimCount()
    {
      return scfParent->GetActiveAnimCount();
    }
    virtual int  GetActiveAnims(char *buffer,int max_length)
    {
      return scfParent->GetActiveAnims(buffer,max_length);
    }
    virtual void SetActiveAnims(const char *buffer,int anim_count)
    {
      scfParent->SetActiveAnims(buffer,anim_count);
    }
    virtual bool SetAnimAction(const char *name, float delayIn, float delayOut)
    {
	return scfParent->SetAnimAction(name,delayIn,delayOut);
    }

    virtual bool SetVelocity(float vel,csRandomGen *rng=0)
    {
	return scfParent->SetVelocity(vel,rng);
    }

    virtual void SetLOD(float lod)
    {
	scfParent->SetLOD(lod);
    }
    virtual bool AttachCoreMesh(const char *meshname)
    {  return scfParent->AttachCoreMesh(meshname); }

    virtual bool AttachCoreMesh(int mesh_id,int iMatWrapID)
    {  return scfParent->AttachCoreMesh(mesh_id,iMatWrapID); }

    virtual bool DetachCoreMesh(const char *meshname)
    {  return scfParent->DetachCoreMesh(meshname); }

    virtual bool DetachCoreMesh(int mesh_id)
    {  return scfParent->DetachCoreMesh(mesh_id); }

    virtual bool BlendMorphTarget(int morph_animation_id, float weight, float delay)
    {  return scfParent->BlendMorphTarget(morph_animation_id, weight, delay); }

    virtual bool ClearMorphTarget(int morph_animation_id, float delay)
    { return scfParent->ClearMorphTarget(morph_animation_id, delay); }
    virtual iSpriteCal3DSocket* AddSocket ()
    {
      csRef<iSpriteCal3DSocket> ifr (
	  SCF_QUERY_INTERFACE_SAFE (scfParent->AddSocket (),
	    iSpriteCal3DSocket));
      return ifr;       // DecRef is ok here.
    }
    virtual iSpriteCal3DSocket* FindSocket (const char* name) const
    {
      csRef<iSpriteCal3DSocket> ifr (SCF_QUERY_INTERFACE_SAFE (
	    scfParent->FindSocket (name), iSpriteCal3DSocket));
      return ifr;       // DecRef is ok here.
    }
    virtual iSpriteCal3DSocket* FindSocket (iMeshWrapper* mesh) const
    {
      csRef<iSpriteCal3DSocket> ifr (SCF_QUERY_INTERFACE_SAFE (
	    scfParent->FindSocket (mesh), iSpriteCal3DSocket));
      return ifr;       // DecRef is ok here.
    }
    virtual int GetSocketCount () const
    {
      return scfParent->GetSocketCount ();
    }
    virtual iSpriteCal3DSocket* GetSocket (int f) const
    {
      csRef<iSpriteCal3DSocket> ifr (
	  SCF_QUERY_INTERFACE_SAFE (scfParent->GetSocket (f),
	    iSpriteCal3DSocket));
      return ifr;       // DecRef is ok here.
    }

  } scfiSpriteCal3DState;
  friend struct SpriteCal3DState;

  //--------------------- iLODControl implementation -------------//
  int GetLODPolygonCount (float lod)
  { return 0; }

  struct LODControl : public iLODControl
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObject);
    virtual void SetLOD (float, float) {  }
    virtual void GetLOD (float& m, float& a) const { m = 0; a = 1; }
    virtual void SetLOD (iSharedVariable*, iSharedVariable*) {  }
    virtual void GetLOD (iSharedVariable*& varm, iSharedVariable*& vara)
    {
      varm = 0;
      vara = 0;
    }
    virtual int GetLODPolygonCount (float lod) const
    {
      return scfParent->GetLODPolygonCount (lod);
    }
    virtual void GetLOD(iSharedVariable *& first,iSharedVariable *& second) const
    {
      first=0; second=0;
    }
  } scfiLODControl;
  friend struct LODControl;

#ifndef CS_USE_NEW_RENDERER
  /// interface to receive state of vertexbuffermanager
  struct eiVertexBufferManagerClient : public iVertexBufferManagerClient
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObject);
    virtual void ManagerClosing ();
  }scfiVertexBufferManagerClient;
  friend struct eiVertexBufferManagerClient;
#endif

  void GetObjectBoundingBox (csBox3& bbox, int type, csVector3 *verts,int vertCount);
  void GetObjectBoundingBox (csBox3& bbox, int type = CS_BBOX_NORMAL);
  void GetRadius (csVector3& rad, csVector3& cent);

  class ObjectModel : public csObjectModel
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObject);
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
};

/**
 * Sprite Cal3D type. This is the plugin you have to use to create instances
 * of csSpriteCal3DMeshObjectFactory.
 */
class csSpriteCal3DMeshObjectType : public iMeshObjectType
{
private:
  iObjectRegistry* object_reg;
  csRef<iVirtualClock> vc;
  iEngine* engine;

public:
  /// Constructor.
  csSpriteCal3DMeshObjectType (iBase*);

  /// Destructor.
  virtual ~csSpriteCal3DMeshObjectType ();

  bool Initialize (iObjectRegistry* p);

  //------------------------ iMeshObjectType implementation --------------
  SCF_DECLARE_IBASE;

  /// New Factory.
  virtual csPtr<iMeshObjectFactory> NewFactory ();

  //------------------- iConfig interface implementation -------------------
  struct csSpriteCal3DConfig : public iConfig
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObjectType);
    virtual bool GetOptionDescription (int idx, csOptionDescription *option) { return false; }
    virtual bool SetOption (int id, csVariant* value) { return false; }
    virtual bool GetOption (int id, csVariant* value) { return false; }
  } scfiConfig;
  friend struct csSpriteCal3DConfig;

  //--------------------- iComponent interface implementation
  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSpriteCal3DMeshObjectType);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
  friend struct eiComponent;

  //--------------------- iLODControl implementation -------------//
  struct LODControl : public iLODControl
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObjectType);
    virtual void SetLOD (float, float)
    {
    }
    virtual void GetLOD (float& m, float& a) const
    {
      m = 0;
      a = 1;
    }
    virtual void SetLOD (iSharedVariable*, iSharedVariable*)
    {
    }
    virtual void GetLOD (iSharedVariable*& varm, iSharedVariable*& vara)
    {
      varm = 0;
      vara = 0;
    }
    virtual int GetLODPolygonCount (float /*lod*/) const
    {
      return 0;
    }
    virtual void GetLOD(iSharedVariable *& first,iSharedVariable *& second) const
    {
      first=0; second=0;
    }
  } scfiLODControl;
  friend struct LODControl;
};

#endif // __CS_SPRCAL3D_H__
