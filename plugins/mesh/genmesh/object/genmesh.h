/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS_GENMESH_H__
#define __CS_GENMESH_H__

#include "csgeom/objmodel.h"
#include "csgeom/transfrm.h"
#include "csgeom/vector3.h"
#include "csgeom/vector4.h"
#include "csgfx/shadervar.h"
#include "csgfx/shadervarcontext.h"
#include "cstool/rendermeshholder.h"
#include "cstool/userrndbuf.h"
#include "csutil/cscolor.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/flags.h"
#include "csutil/hash.h"
#include "csutil/leakguard.h"
#include "csutil/refarr.h"
#include "csutil/parray.h"
#include "csutil/weakref.h"
#include "iengine/light.h"
#include "iengine/lightmgr.h"
#include "iengine/shadcast.h"
#include "igeom/polymesh.h"
#include "imesh/genmesh.h"
#include "imesh/lighting.h"
#include "imesh/object.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/virtclk.h"
#include "ivideo/graph3d.h"
#include "ivideo/rendermesh.h"
#include "ivideo/rndbuf.h"

class csBSPTree;
class csColor;
class csColor4;
class csGenmeshMeshObject;
class csGenmeshMeshObjectFactory;
class csGenmeshMeshObjectType;
class csPolygonMesh;
struct iCacheManager;
struct iEngine;
struct iMaterialWrapper;
struct iMovable;
struct iObjectRegistry;
struct iShadowBlockList;

/**
 * An array giving shadow information for a pseudo-dynamic light.
 */
class csShadowArray
{
public:
  iLight* light;
  csShadowArray* next;
  // For every vertex of the mesh a value.
  float* shadowmap;

  csShadowArray () : shadowmap (0) { }
  ~csShadowArray ()
  {
    delete[] shadowmap;
  }
};

struct csGenmeshSubMesh
{
  csRef<iRenderBuffer> index_buffer;
  csRef<iMaterialWrapper> material;
  csRenderMeshHolder rmHolder;
  csRef<csRenderBufferHolder> bufferHolder;
  int tricount;

  // Override mixmode from parent.
  bool override_mixmode;
  uint MixMode;
};

/**
 * Genmesh version of mesh object.
 */
class csGenmeshMeshObject : public iMeshObject
{
private:
  csRenderMeshHolder rmHolder;
  csRef<csShaderVariableContext> svcontext;
  csRef<csRenderBufferHolder> bufferHolder;
  csWeakRef<iGraphics3D> g3d;
  bool mesh_colors_dirty_flag;

  uint buffers_version;
  csRef<iRenderBuffer> sorted_index_buffer;	// Only if factory back2front
  int num_sorted_mesh_triangles;
  csTriangle* sorted_mesh_triangles;

  // The following three are only used in case animation control is required.
  csRef<iRenderBuffer> vertex_buffer;
  csRef<iRenderBuffer> texel_buffer;
  csRef<iRenderBuffer> normal_buffer;

  csRef<iRenderBuffer> color_buffer;
  iMovable* lighting_movable;

  csPDelArray<csGenmeshSubMesh> subMeshes;
  csDirtyAccessArray<csRenderMesh*> renderMeshes;

  csUserRenderBufferManager userBuffers;
  csArray<csStringID> user_buffer_names;

  csGenmeshMeshObjectFactory* factory;
  iMeshWrapper* logparent;
  csRef<iMaterialWrapper> material;
  bool material_needs_visit;
  uint MixMode;
  iMeshObjectDrawCallback* vis_cb;
  bool do_lighting;
  bool do_manual_colors;
  csColor4 base_color;
  float current_lod;
  uint32 current_features;
  csFlags flags;

  bool do_shadows;
  bool do_shadow_rec;

  csColor4* lit_mesh_colors;
  int num_lit_mesh_colors;	// Should be equal to factory number.
  csColor4* static_mesh_colors;

  /**
   * Global sector wide dynamic ambient version.
   */
  uint32 dynamic_ambient_version;

  csHash<csShadowArray*, csPtrKey<iLight> > pseudoDynInfo;

  // If we are using the iLightingInfo lighting system then this
  // is an array of lights that affect us right now.
  csSet<csPtrKey<iLight> > affecting_lights;
  // In case we are not using the iLightingInfo system then we
  // GetRenderMeshes() will updated the following array:
  csArray<iLight*> relevant_lights;

  // If the following flag is dirty then some of the affecting lights
  // has changed and we need to recalculate.
  bool lighting_dirty;

  // choose whether to draw shadow caps or not
  bool shadow_caps;

  bool initialized;

  /**
   * Camera space bounding box is cached here.
   * GetCameraBoundingBox() will check the current camera number from the
   * camera to see if it needs to recalculate this.
   */
  csBox3 camera_bbox;
  csBox3 world_bbox;

  /// Current camera number.
  long cur_cameranr;
  /// Current movable number.
  long cur_movablenr;

  /**
   * Clears out the pseudoDynInfo hash and frees the memory allocated by the
   * shadow maps.
   */
  void ClearPseudoDynLights ();

  /**
   * Setup this object. This function will check if setup is needed.
   */
  void SetupObject ();

  /**
   * Make sure the 'lit_mesh_colors' array has the right size.
   * Also clears the pseudo-dynamic light hash if the vertex count
   * changed!
   */
  void CheckLitColors ();

  /**
   * Process one light and add the values to the genmesh light table.
   * The given transform is the full movable transform.
   */
  void UpdateLightingOne (const csReversibleTransform& trans, iLight* light);

  /**
   * Update lighting using the iLightingInfo system.
   */
  void UpdateLighting (const csArray<iLight*>& lights, iMovable* movable);

public:
  /// Constructor.
  csGenmeshMeshObject (csGenmeshMeshObjectFactory* factory);

  CS_LEAKGUARD_DECLARE (csGenmeshMeshObject);

  /// Destructor.
  virtual ~csGenmeshMeshObject ();

  void ClearSubMeshes ();
  void AddSubMesh (unsigned int *triangles,
    int tricount, iMaterialWrapper *material, bool do_mixmode, uint mixmode);

  void SetMixMode (uint mode)
  {
    MixMode = mode;
  }
  uint GetMixMode () const { return MixMode; }
  void SetLighting (bool l) { do_lighting = l; }
  bool IsLighting () const { return do_lighting; }
  const csColor& GetColor () const { return base_color; }
  void SetManualColors (bool m) { do_manual_colors = m; }
  bool IsManualColors () const { return do_manual_colors; }
  void GetObjectBoundingBox (csBox3& bbox);
  void SetObjectBoundingBox (const csBox3& bbox);
  void GetRadius (csVector3& rad, csVector3& cent);
  void SetShadowCasting (bool m) { do_shadows = m; }
  void SetShadowReceiving (bool m) { do_shadow_rec = m; }

  iVirtualClock* vc;
  csRef<iGenMeshAnimationControl> anim_ctrl;
  void SetAnimationControl (iGenMeshAnimationControl* anim_ctrl);
  iGenMeshAnimationControl* GetAnimationControl () const { return anim_ctrl; }
  const csVector3* AnimControlGetVertices ();
  const csVector2* AnimControlGetTexels ();
  const csVector3* AnimControlGetNormals ();
  const csColor4* AnimControlGetColors (csColor4* source);
  bool anim_ctrl_verts;
  bool anim_ctrl_texels;
  bool anim_ctrl_normals;
  bool anim_ctrl_colors;

  // This function sets up the shader variable context depending on
  // the existance of an animation control. If there is no animation control
  // then the factory will provide the vertex and other data. Otherwise the
  // mesh itself will do it.
  void SetupShaderVariableContext ();

  bool AddRenderBuffer (const char *name, iRenderBuffer* buffer);
  bool RemoveRenderBuffer (const char *name);

  //----------------------- Shadow and lighting system ----------------------
  char* GenerateCacheName ();
  void InitializeDefault (bool clear);
  bool ReadFromCache (iCacheManager* cache_mgr);
  bool WriteToCache (iCacheManager* cache_mgr);
  void PrepareLighting ();

  void AppendShadows (iMovable* movable, iShadowBlockList* shadows,
    	const csVector3& origin);
  void CastShadows (iMovable* movable, iFrustumView* fview);
  void LightChanged (iLight* light);
  void LightDisconnect (iLight* light);

  //----------------------- iMeshObject implementation ----------------------
  SCF_DECLARE_IBASE;

  virtual iMeshObjectFactory* GetFactory () const
  {
    return (iMeshObjectFactory*)factory;
  }
  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> Clone () { return 0; }
  virtual csRenderMesh** GetRenderMeshes (int &n, iRenderView* rview, 
    iMovable* movable, uint32 frustum_mask);
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
  virtual void NextFrame (csTicks /*current_time*/,
  	const csVector3& /*pos*/) { }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual bool HitBeamOutline (const csVector3& start, const csVector3& end,
    csVector3& isect, float *pr);
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr, int* polygon_idx = 0);
  virtual void SetMeshWrapper (iMeshWrapper* lp)
  {
    logparent = lp;
    CS_ASSERT (logparent != 0);
  }
  virtual iMeshWrapper* GetMeshWrapper () const { return logparent; }

  virtual iObjectModel* GetObjectModel ();
  virtual bool SetColor (const csColor& col)
  {
    base_color.Set (col);
    lighting_dirty = true;
    return true;
  }
  virtual bool GetColor (csColor& col) const { col = base_color; return true; }
  virtual bool SetMaterialWrapper (iMaterialWrapper* mat);
  virtual iMaterialWrapper* GetMaterialWrapper () const { return material; }
  virtual void InvalidateMaterialHandles () { }
  /**
   * see imesh/object.h for specification. The default implementation
   * does nothing.
   */
  virtual void PositionChild (iMeshObject* child,csTicks current_time) { }

  //------------------------- iLightingInfo interface -------------------------
  struct LightingInfo : public iLightingInfo
  {
    SCF_DECLARE_EMBEDDED_IBASE (csGenmeshMeshObject);
    virtual void InitializeDefault (bool clear)
    {
      scfParent->InitializeDefault (clear);
    }
    virtual bool ReadFromCache (iCacheManager* cache_mgr)
    {
      return scfParent->ReadFromCache (cache_mgr);
    }
    virtual bool WriteToCache (iCacheManager* cache_mgr)
    {
      return scfParent->WriteToCache (cache_mgr);
    }
    virtual void PrepareLighting ()
    {
      scfParent->PrepareLighting ();
    }
    virtual void LightChanged (iLight* light)
    {
      scfParent->LightChanged (light);
    }
    virtual void LightDisconnect (iLight* light)
    {
      scfParent->LightDisconnect (light);
    }
  } scfiLightingInfo;
  friend struct LightingInfo;

  //-------------------- iShadowCaster interface implementation ----------
  struct ShadowCaster : public iShadowCaster
  {
    SCF_DECLARE_EMBEDDED_IBASE (csGenmeshMeshObject);
    virtual void AppendShadows (iMovable* movable, iShadowBlockList* shadows,
    	const csVector3& origin)
    {
      scfParent->AppendShadows (movable, shadows, origin);
    }
  } scfiShadowCaster;
  friend struct ShadowCaster;

  //-------------------- iShadowReceiver interface implementation ----------
  struct ShadowReceiver : public iShadowReceiver
  {
    SCF_DECLARE_EMBEDDED_IBASE (csGenmeshMeshObject);
    virtual void CastShadows (iMovable* movable, iFrustumView* fview)
    {
      scfParent->CastShadows (movable, fview);
    }
  } scfiShadowReceiver;
  friend struct ShadowReceiver;

  //------------------------- iGeneralMeshState implementation ----------------
  class GeneralMeshState : public iGeneralMeshState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csGenmeshMeshObject);
    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    { scfParent->SetMaterialWrapper (material); }
    virtual iMaterialWrapper* GetMaterialWrapper () const
    { return scfParent->material; }
    virtual void SetMixMode (uint mode) { scfParent->MixMode = mode; }
    virtual uint GetMixMode () const { return scfParent->MixMode; }
    virtual void SetLighting (bool l) { scfParent->SetLighting (l); }
    virtual bool IsLighting () const { return scfParent->IsLighting (); }
    virtual void SetColor (const csColor& col) { scfParent->SetColor (col); }
    virtual const csColor& GetColor () const { return scfParent->GetColor (); }
    virtual void SetManualColors (bool m)
    {
      scfParent->SetManualColors (m);
    }
    virtual bool IsManualColors () const
    {
      return scfParent->IsManualColors ();
    }
    virtual void SetShadowCasting (bool m)
    {
      scfParent->do_shadows = m;
    }
    virtual bool IsShadowCasting () const
    {
      return scfParent->do_shadows;
    }
    virtual void SetShadowReceiving (bool m)
    {
      scfParent->do_shadow_rec = m;
    }
    virtual bool IsShadowReceiving () const
    {
      return scfParent->do_shadow_rec;
    }
    virtual void SetAnimationControl (iGenMeshAnimationControl* anim_ctrl)
    {
      scfParent->SetAnimationControl (anim_ctrl);
    }
    virtual iGenMeshAnimationControl* GetAnimationControl () const
    {
      return scfParent->GetAnimationControl ();
    }
    virtual void ClearSubMeshes ()
    {
      scfParent->ClearSubMeshes ();
    }
    virtual void AddSubMesh (unsigned int *triangles,
      int tricount,
      iMaterialWrapper *material)
    {
      scfParent->AddSubMesh (triangles, tricount, material, false, CS_FX_COPY);
    }
    virtual void AddSubMesh (unsigned int *triangles,
      int tricount, iMaterialWrapper *material, uint mixmode)
    {
      scfParent->AddSubMesh (triangles, tricount, material, true, mixmode);
    }
    virtual bool AddRenderBuffer (const char *name, iRenderBuffer* buffer)
    { return scfParent->AddRenderBuffer (name, buffer); }
    virtual bool RemoveRenderBuffer (const char *name)
    { return scfParent->RemoveRenderBuffer (name); }
  } scfiGeneralMeshState;
  friend class GeneralMeshState;

  //------------------ iPolygonMesh interface implementation ----------------//
  struct PolyMesh : public iPolygonMesh
  {
  private:
    csFlags flags;

  public:
    SCF_DECLARE_EMBEDDED_IBASE (csGenmeshMeshObject);

    virtual int GetVertexCount ();
    virtual csVector3* GetVertices ();
    virtual int GetPolygonCount ();
    virtual csMeshedPolygon* GetPolygons ();
    virtual int GetTriangleCount ();
    virtual csTriangle* GetTriangles ();
    virtual void Lock () { }
    virtual void Unlock () { }
    
    virtual csFlags& GetFlags () { return flags;  }
    virtual uint32 GetChangeNumber() const { return 0; }

    PolyMesh ()
    {
      flags.Set (CS_POLYMESH_TRIANGLEMESH);
    }
    virtual ~PolyMesh () { }
  } scfiPolygonMesh;
  friend struct PolyMesh;

  class eiRenderBufferAccessor : public iRenderBufferAccessor
  {
  public:
    CS_LEAKGUARD_DECLARE (eiRenderBufferAccessor);
    SCF_DECLARE_IBASE;
    csGenmeshMeshObject* parent;
    virtual ~eiRenderBufferAccessor ()
    {
      SCF_DESTRUCT_IBASE ();
    }
    eiRenderBufferAccessor (csGenmeshMeshObject* parent)
    {
      SCF_CONSTRUCT_IBASE (0);
      eiRenderBufferAccessor::parent = parent;
    }
    virtual void PreGetBuffer (csRenderBufferHolder* holder, csRenderBufferName buffer)
    {
      parent->PreGetBuffer (holder, buffer);
    }
  } *scfiRenderBufferAccessor;
  friend class eiRenderBufferAccessor;

  void PreGetBuffer (csRenderBufferHolder* holder, csRenderBufferName buffer);

  //------------------ iShaderVariableAccessor implementation ------------
  class eiShaderVariableAccessor : public iShaderVariableAccessor
  {
  public:
    SCF_DECLARE_IBASE;
    csGenmeshMeshObject* parent;
    virtual ~eiShaderVariableAccessor ()
    {
      SCF_DESTRUCT_IBASE ();
    }
    eiShaderVariableAccessor (csGenmeshMeshObject* parent)
    {
      SCF_CONSTRUCT_IBASE (0);
      eiShaderVariableAccessor::parent = parent;
    }
    virtual void PreGetValue (csShaderVariable* variable)
    {
      parent->PreGetShaderVariableValue (variable);
    }
  } *scfiShaderVariableAccessor;
  friend class eiShaderVariableAccessor;

  void PreGetShaderVariableValue (csShaderVariable* variable);
};

/**
 * Factory for general meshes.
 */
class csGenmeshMeshObjectFactory : public iMeshObjectFactory
{
private:
  csRef<iMaterialWrapper> material;
  csVector3* mesh_vertices;
  csVector2* mesh_texels;
  csVector3* mesh_normals;
  csColor4* mesh_colors;
  int num_mesh_vertices;
  csVector3* mesh_tri_normals;

  bool autonormals;

  bool mesh_vertices_dirty_flag;
  bool mesh_texels_dirty_flag;
  bool mesh_normals_dirty_flag;
  bool mesh_colors_dirty_flag;
  bool mesh_triangle_dirty_flag;
  bool mesh_tangents_dirty_flag;

  csTriangle* mesh_triangles;
  int num_mesh_triangles;

  csWeakRef<iGraphics3D> g3d;
  csRef<iStringSet> strings;

  csRef<iRenderBuffer> vertex_buffer;
  csRef<iRenderBuffer> texel_buffer;
  csRef<iRenderBuffer> normal_buffer;
  csRef<iRenderBuffer> color_buffer;
  csRef<iRenderBuffer> index_buffer;
  csRef<iRenderBuffer> binormal_buffer;
  csRef<iRenderBuffer> tangent_buffer;
  
  csUserRenderBufferManager userBuffers;
  csArray<csStringID> user_buffer_names;

  csPDelArray<csGenmeshSubMesh> subMeshes;

  uint default_mixmode;
  bool default_lighting;
  csColor default_color;
  bool default_manualcolors;
  bool default_shadowcasting;
  bool default_shadowreceiving;

  csVector3 radius;
  csBox3 object_bbox;
  bool object_bbox_valid;
  bool initialized;

  // For animation control.
  csRef<iGenMeshAnimationControlFactory> anim_ctrl_fact;
  void SetAnimationControlFactory (iGenMeshAnimationControlFactory*
  	anim_ctrl_fact);
  iGenMeshAnimationControlFactory* GetAnimationControlFactory () const
  {
    return anim_ctrl_fact;
  }

  csMeshedPolygon* polygons;

  /// Calculate bounding box and radius.
  void CalculateBBoxRadius ();

  /**
   * Compress vertices. This is for CalculateNormals().
   */
  bool CompressVertices (
	csVector3* orig_verts, int orig_num_vts,
	csVector3*& new_verts, int& new_num_vts,
	csTriangle* orig_tris, int num_tris,
	csTriangle*& new_tris,
	int*& mapping);

  /**
   * Setup this factory. This function will check if setup is needed.
   */
  void SetupFactory ();

public:
  CS_LEAKGUARD_DECLARE (csGenmeshMeshObjectFactory);

  csRef<iVirtualClock> vc;

  uint buffers_version;

  bool back2front;
  csBSPTree* back2front_tree;

  iObjectRegistry* object_reg;
  iMeshFactoryWrapper* logparent;
  iMeshObjectType* genmesh_type;
  csRef<iLightManager> light_mgr;
  csFlags flags;

  iEngine* engine;

  const csPDelArray<csGenmeshSubMesh>& GetSubMeshes () const
  {
    return subMeshes;
  }

  /// Constructor.
  csGenmeshMeshObjectFactory (iMeshObjectType *pParent,
  	iObjectRegistry* object_reg);

  /// Destructor.
  virtual ~csGenmeshMeshObjectFactory ();

  void SetMaterialWrapper (iMaterialWrapper* material)
  {
    csGenmeshMeshObjectFactory::material = material;
  }
  iMaterialWrapper* GetMaterialWrapper () const { return material; }
  void SetVertexCount (int n);
  int GetVertexCount () const { return num_mesh_vertices; }
  csVector3* GetVertices ()
  {
    SetupFactory ();
    return mesh_vertices;
  }
  csVector2* GetTexels ()
  {
    SetupFactory ();
    return mesh_texels;
  }
  csVector3* GetNormals ()
  {
    SetupFactory ();
    return mesh_normals;
  }
  csColor4* GetColors ()
  {
    SetupFactory ();
    return mesh_colors;
  }

  void SetTriangleCount (int n);

  int GetTriangleCount () const { return num_mesh_triangles; }
  csTriangle* GetTriangles () { SetupFactory (); return mesh_triangles; }

  void Invalidate ();
  void CalculateNormals ();
  void GenerateBox (const csBox3& box);
  void GenerateSphere (const csSphere& sphere, int rim_vertices);
  //void GeneratePlane (const csPlane3& plane);
  void SetBack2Front (bool b2f);
  bool IsBack2Front () const { return back2front; }
  void BuildBack2FrontTree ();

  bool AddRenderBuffer (const char *name, iRenderBuffer* buffer);
  bool RemoveRenderBuffer (const char *name);
  /**
   * Get the string ID's for the anonymous buffers
   */
  const csArray<csStringID>& GetUserBufferNames ()
  { return user_buffer_names; }
  const csUserRenderBufferManager& GetUserBuffers()
  { return userBuffers; }
  iStringSet* GetStrings()
  { return strings; }

  void ClearSubMeshes ();
  void AddSubMesh (unsigned int *triangles,
    int tricount, iMaterialWrapper *material, bool do_mixmode, uint mixmode);

  const csBox3& GetObjectBoundingBox ();
  void SetObjectBoundingBox (const csBox3& bbox);
  const csVector3& GetRadius ();

  /**
   * Calculate polygons for iPolygonMesh.
   */
  csMeshedPolygon* GetPolygons ();

  void SetMixMode (uint mode)
  {
    default_mixmode = mode;
  }
  uint GetMixMode () const
  {
    return default_mixmode;
  }
  void SetLighting (bool l)
  {
    default_lighting = l;
  }
  bool IsLighting () const
  {
    return default_lighting;
  }
  void SetColor (const csColor& col)
  {
    default_color = col;
  }
  const csColor& GetColor () const
  {
    return default_color;
  }
  void SetManualColors (bool m)
  {
    default_manualcolors = m;
  }
  bool IsManualColors () const
  {
    return default_manualcolors;
  }
  void SetShadowCasting (bool m)
  {
    default_shadowcasting = m;
  }
  bool IsShadowCasting () const
  {
    return default_shadowcasting;
  }
  void SetShadowReceiving (bool m)
  {
    default_shadowreceiving = m;
  }
  bool IsShadowReceiving () const
  {
    return default_shadowreceiving;
  }
  bool IsAutoNormals () const
  {
    return autonormals;
  }

  //------------------------ iMeshObjectFactory implementation --------------
  SCF_DECLARE_IBASE;

  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> NewInstance ();
  virtual csPtr<iMeshObjectFactory> Clone () { return 0; }
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () const { return true; }
  virtual void SetMeshFactoryWrapper (iMeshFactoryWrapper* lp)
  {
    logparent = lp;
    CS_ASSERT (logparent != 0);
  }
  virtual iMeshFactoryWrapper* GetMeshFactoryWrapper () const
  { return logparent; }
  virtual iMeshObjectType* GetMeshObjectType () const { return genmesh_type; }

  //----------------------- iGeneralFactoryState implementation -------------
  class GeneralFactoryState : public iGeneralFactoryState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csGenmeshMeshObjectFactory);

    virtual void SetMixMode (uint mode)
    {
      scfParent->SetMixMode (mode);
    }
    virtual uint GetMixMode () const
    {
      return scfParent->GetMixMode ();
    }
    virtual void SetLighting (bool l)
    {
      scfParent->SetLighting (l);
    }
    virtual bool IsLighting () const
    {
      return scfParent->IsLighting ();
    }
    virtual void SetColor (const csColor& col)
    {
      scfParent->SetColor (col);
    }
    virtual const csColor& GetColor () const
    {
      return scfParent->GetColor ();
    }
    virtual void SetManualColors (bool m)
    {
      scfParent->SetManualColors (m);
    }
    virtual bool IsManualColors () const
    {
      return scfParent->IsManualColors ();
    }
    virtual void SetShadowCasting (bool m)
    {
      scfParent->SetShadowCasting (m);
    }
    virtual bool IsShadowCasting () const
    {
      return scfParent->IsShadowCasting ();
    }
    virtual void SetShadowReceiving (bool m)
    {
      scfParent->SetShadowReceiving (m);
    }
    virtual bool IsShadowReceiving () const
    {
      return scfParent->IsShadowReceiving ();
    }

    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    {
      scfParent->SetMaterialWrapper (material);
    }
    virtual iMaterialWrapper* GetMaterialWrapper () const
    {
      return scfParent->GetMaterialWrapper ();
    }
    virtual void SetVertexCount (int n)
    {
      scfParent->SetVertexCount (n);
    }
    virtual int GetVertexCount () const
    {
      return scfParent->GetVertexCount ();
    }
    virtual csVector3* GetVertices ()
    {
      return scfParent->GetVertices ();
    }
    virtual csVector2* GetTexels ()
    {
      return scfParent->GetTexels ();
    }
    virtual csVector3* GetNormals ()
    {
      return scfParent->GetNormals ();
    }
    virtual void SetTriangleCount (int n)
    {
      scfParent->SetTriangleCount (n);
    }
    virtual int GetTriangleCount () const
    {
      return scfParent->GetTriangleCount ();
    }
    virtual csTriangle* GetTriangles ()
    {
      return scfParent->GetTriangles ();
    }
    virtual csColor4* GetColors ()
    {
      return scfParent->GetColors ();
    }
    virtual void Invalidate ()
    {
      scfParent->Invalidate ();
    }
    virtual void CalculateNormals ()
    {
      scfParent->CalculateNormals ();
    }
    virtual void GenerateBox (const csBox3& box)
    {
      scfParent->GenerateBox (box);
    }
    virtual void GenerateSphere (const csSphere& sphere, int rim_vertices)
    {
      scfParent->GenerateSphere (sphere, rim_vertices);
    }
    //virtual void GeneratePlane (const csPlane3& plane)
    //{
    //  scfParent->GeneratePlane (plane);
    //}
    virtual void SetBack2Front (bool b2f)
    {
      scfParent->SetBack2Front (b2f);
    }
    virtual bool IsAutoNormals () const
    {
      return scfParent->IsAutoNormals ();
    }
    virtual bool IsBack2Front () const
    {
      return scfParent->IsBack2Front ();
    }
    virtual void SetAnimationControlFactory (iGenMeshAnimationControlFactory*
    	anim_ctrl_fact)
    {
      scfParent->SetAnimationControlFactory (anim_ctrl_fact);
    }
    virtual iGenMeshAnimationControlFactory* GetAnimationControlFactory () const
    {
      return scfParent->GetAnimationControlFactory ();
    }
    virtual bool AddRenderBuffer (const char *name, iRenderBuffer* buffer)
    { return scfParent->AddRenderBuffer (name, buffer); }
    virtual bool RemoveRenderBuffer (const char *name)
    { return scfParent->RemoveRenderBuffer (name); }
    virtual void ClearSubMeshes ()
    {
      scfParent->ClearSubMeshes ();
    }
    virtual void AddSubMesh (unsigned int *triangles,
      int tricount,
      iMaterialWrapper *material)
    {
      scfParent->AddSubMesh (triangles, tricount, material, false, CS_FX_COPY);
    }
    virtual void AddSubMesh (unsigned int *triangles,
      int tricount, iMaterialWrapper *material, uint mixmode)
    {
      scfParent->AddSubMesh (triangles, tricount, material, true, mixmode);
    }
  } scfiGeneralFactoryState;
  friend class GeneralFactoryState;

  //------------------ iPolygonMesh interface implementation ----------------//
  struct PolyMesh : public iPolygonMesh
  {
  private:
    csGenmeshMeshObjectFactory* factory;
    csFlags flags;
  public:
    //SCF_DECLARE_EMBEDDED_IBASE (csGenmeshMeshObjectFactory);
    SCF_DECLARE_IBASE;

    void SetFactory (csGenmeshMeshObjectFactory* Factory)
    { factory = Factory; }

    virtual int GetVertexCount ();
    virtual csVector3* GetVertices ();
    virtual int GetPolygonCount ();
    virtual csMeshedPolygon* GetPolygons ();
    virtual int GetTriangleCount ();
    virtual csTriangle* GetTriangles ();
    virtual void Lock () { }
    virtual void Unlock () { }
    
    virtual csFlags& GetFlags () { return flags;  }
    virtual uint32 GetChangeNumber() const { return 0; }

    PolyMesh ()
    {
      SCF_CONSTRUCT_IBASE (0);
      flags.Set (CS_POLYMESH_TRIANGLEMESH);
    }
    virtual ~PolyMesh ()
    {
      SCF_DESTRUCT_IBASE ();
    }
  } scfiPolygonMesh;
  friend struct PolyMesh;

  //------------------------- iObjectModel implementation ----------------
  class ObjectModel : public csObjectModel
  {
    SCF_DECLARE_EMBEDDED_IBASE (csGenmeshMeshObjectFactory);
    virtual void GetObjectBoundingBox (csBox3& bbox)
    {
      bbox = scfParent->GetObjectBoundingBox ();
    }
    virtual void SetObjectBoundingBox (const csBox3& bbox)
    {
      scfParent->SetObjectBoundingBox (bbox);
    }
    virtual void GetRadius (csVector3& rad, csVector3& cent)
    {
      rad = scfParent->GetRadius ();
      cent.Set (0.0f);
    }
  } scfiObjectModel;
  friend class ObjectModel;

  virtual iObjectModel* GetObjectModel () { return &scfiObjectModel; }

  //------------------ iShaderVariableAccessor implementation ------------
  class eiShaderVariableAccessor : public iShaderVariableAccessor
  {
  public:
    //SCF_DECLARE_EMBEDDED_IBASE (csGenmeshMeshObjectFactory);
    SCF_DECLARE_IBASE;
    csGenmeshMeshObjectFactory* parent;
    virtual ~eiShaderVariableAccessor ()
    {
      SCF_DESTRUCT_IBASE ();
    }
    eiShaderVariableAccessor (csGenmeshMeshObjectFactory* parent)
    {
      SCF_CONSTRUCT_IBASE (0);
      eiShaderVariableAccessor::parent = parent;
    }
    virtual void PreGetValue (csShaderVariable* variable)
    {
      //scfParent->PreGetShaderVariableValue (variable);
      parent->PreGetShaderVariableValue (variable);
    }
  } *scfiShaderVariableAccessor;
  friend class eiShaderVariableAccessor;

  void PreGetShaderVariableValue (csShaderVariable* variable);


  class eiRenderBufferAccessor : public iRenderBufferAccessor
  {
  public:
    CS_LEAKGUARD_DECLARE (eiRenderBufferAccessor);
    SCF_DECLARE_IBASE;
    csGenmeshMeshObjectFactory* parent;
    virtual ~eiRenderBufferAccessor ()
    {
      SCF_DESTRUCT_IBASE ();
    }
    eiRenderBufferAccessor (csGenmeshMeshObjectFactory* parent)
    {
      SCF_CONSTRUCT_IBASE (0);
      eiRenderBufferAccessor::parent = parent;
    }
    virtual void PreGetBuffer (csRenderBufferHolder* holder, csRenderBufferName buffer)
    {
      parent->PreGetBuffer (holder, buffer);
    }
  } *scfiRenderBufferAccessor;
  friend class eiRenderBufferAccessor;

  void PreGetBuffer (csRenderBufferHolder* holder, csRenderBufferName buffer);
};

/**
 * Genmesh type. This is the plugin you have to use to create instances
 * of csGenmeshMeshObjectFactory.
 */
class csGenmeshMeshObjectType : public iMeshObjectType
{
public:
  iObjectRegistry* object_reg;
  bool do_verbose;

  SCF_DECLARE_IBASE;

  /// Constructor.
  csGenmeshMeshObjectType (iBase*);
  /// Destructor.
  virtual ~csGenmeshMeshObjectType ();
  /// Draw.
  virtual csPtr<iMeshObjectFactory> NewFactory ();
  /// Initialize.
  bool Initialize (iObjectRegistry* object_reg);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csGenmeshMeshObjectType);
    virtual bool Initialize (iObjectRegistry* object_reg)
    {
      return scfParent->Initialize (object_reg);
    }
  } scfiComponent;
};

#endif // __CS_GENMESH_H__
