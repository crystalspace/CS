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

#include "cstool/objmodel.h"
#include "csgeom/box.h"
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
#include "csutil/pooledscfclass.h"
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

#include "submeshes.h"

class csBSPTree;
class csColor;
class csColor4;
class csPolygonMesh;
struct iCacheManager;
struct iEngine;
struct iMaterialWrapper;
struct iMovable;
struct iObjectRegistry;
struct iShadowBlockList;

CS_PLUGIN_NAMESPACE_BEGIN(Genmesh)
{

class csGenmeshMeshObject;
class csGenmeshMeshObjectFactory;
class csGenmeshMeshObjectType;

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

class MergedSVContext : 
  public scfImplementationPooled<scfImplementation1<MergedSVContext,
                                                    iShaderVariableContext> >
{
  // context1 has precedence
  iShaderVariableContext* context1;
  iShaderVariableContext* context2;
public:
  MergedSVContext (iShaderVariableContext* context1, 
    iShaderVariableContext* context2) : scfPooledImplementationType (this),
    context1 (context1), context2 (context2)
  { }

  void AddVariable (csShaderVariable *variable)
  { }
  csShaderVariable* GetVariable (csStringID name) const
  { 
    csShaderVariable* sv = context1->GetVariable (name); 
    if (sv == 0)
      sv = context2->GetVariable (name);
    return sv;
  }
  const csRefArray<csShaderVariable>& GetShaderVariables () const
  { 
    return context1->GetShaderVariables();
  }
  void PushVariables (iShaderVarStack* stacks) const
  { 
    context2->PushVariables (stacks);
    context1->PushVariables (stacks);
  }

  bool IsEmpty () const { return context1->IsEmpty() && context2->IsEmpty(); }

  void ReplaceVariable (csShaderVariable *variable) { }
  void Clear () { }
};

#include "csutil/win32/msvc_deprecated_warn_off.h"

/**
 * Genmesh version of mesh object.
 */
class csGenmeshMeshObject : public scfImplementation5<csGenmeshMeshObject, 
						      iMeshObject,
						      iLightingInfo,
						      iShadowCaster,
						      iShadowReceiver,
						      iGeneralMeshState>
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

  csDirtyAccessArray<csRenderMesh*> renderMeshes;
  mutable SubMeshProxiesContainer subMeshes;
  mutable uint factorySubMeshesChangeNum;
  void UpdateSubMeshProxies () const;
  struct LegacySubmesh
  {
    csRef<iRenderBuffer> indexbuffer;
    csRef<iMaterialWrapper> material;
    uint mixmode;
    csRenderMeshHolder rmHolder;
    csRef<csRenderBufferHolder> bufferHolder;
  };
  csArray<LegacySubmesh> legacySubmeshes;

  csUserRenderBufferManager userBuffers;
  csArray<csStringID> user_buffer_names;

  csGenmeshMeshObjectFactory* factory;
  iMeshWrapper* logparent;
  csRef<iMaterialWrapper> material;
  bool material_needs_visit;
  uint MixMode;
  csRef<iMeshObjectDrawCallback> vis_cb;
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
  csArray<iLightSectorInfluence*> relevant_lights;

  // If the following flag is dirty then some of the affecting lights
  // has changed and we need to recalculate.
  bool lighting_dirty;

  // choose whether to draw shadow caps or not
  bool shadow_caps;

  bool initialized;

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
  void UpdateLighting (
      const csArray<iLightSectorInfluence*>& lights, iMovable* movable);

public:
  /// Constructor.
  csGenmeshMeshObject (csGenmeshMeshObjectFactory* factory);

  CS_LEAKGUARD_DECLARE (csGenmeshMeshObject);

  /// Destructor.
  virtual ~csGenmeshMeshObject ();

  /**\name iMeshObject implementation
   * @{ */
  void SetMixMode (uint mode)
  {
    MixMode = mode;
  }
  uint GetMixMode () const { return MixMode; }
  const csColor& GetColor () const { return base_color; }
  /** @} */
  
  /**\name iGeneralMeshState implementation
   * @{ */
  void SetLighting (bool l) { do_lighting = l; }
  bool IsLighting () const { return do_lighting; }
  void SetManualColors (bool m) { do_manual_colors = m; }
  bool IsManualColors () const { return do_manual_colors; }
  void GetObjectBoundingBox (csBox3& bbox);
  const csBox3& GetObjectBoundingBox ();
  void SetObjectBoundingBox (const csBox3& bbox);
  void GetRadius (float& rad, csVector3& cent);
  void SetShadowCasting (bool m) { do_shadows = m; }
  bool IsShadowCasting () const { return do_shadows; }
  void SetShadowReceiving (bool m) { do_shadow_rec = m; }
  bool IsShadowReceiving () const { return do_shadow_rec; }
  iGeneralMeshSubMesh* FindSubMesh (const char* name) const; 
  void AddSubMesh (unsigned int *triangles, int tricount, 
    iMaterialWrapper *material, uint mixmode);
  void AddSubMesh (unsigned int *triangles, int tricount, 
    iMaterialWrapper *material)
  {
    AddSubMesh (triangles, tricount, material, (uint)~0);
  }
  /** @} */

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
  int GetRenderBufferCount () const
  {
    return (int)this->user_buffer_names.GetSize ();
  }
  csRef<iRenderBuffer> GetRenderBuffer (int index); 
  csRef<iString> GetRenderBufferName (int index) const;


  /**\name Shadow and lighting system
   * @{ */
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
  void DisconnectAllLights ();
  /** @} */

  /**\name iMeshObject implementation
   * @{ */
  virtual iMeshObjectFactory* GetFactory () const;
  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> Clone () { return 0; }
  virtual csRenderMesh** GetRenderMeshes (int &n, iRenderView* rview, 
    iMovable* movable, uint32 frustum_mask);
  virtual void SetVisibleCallback (iMeshObjectDrawCallback* cb)
  {
    vis_cb = cb;
  }
  virtual iMeshObjectDrawCallback* GetVisibleCallback () const
  {
    return vis_cb;
  }
  virtual void NextFrame (csTicks /*current_time*/,
  	const csVector3& /*pos*/, uint /*currentFrame*/) { }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual bool HitBeamOutline (const csVector3& start, const csVector3& end,
    csVector3& isect, float *pr);
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr, int* polygon_idx = 0,
	iMaterialWrapper** material = 0);
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
  virtual void PositionChild (iMeshObject* /*child*/, csTicks /*current_time*/) { }
  /** @} */

  class RenderBufferAccessor : 
    public scfImplementation1<RenderBufferAccessor, iRenderBufferAccessor>
  {
  public:
    CS_LEAKGUARD_DECLARE (eiRenderBufferAccessor);
    csWeakRef<csGenmeshMeshObject> parent;
    virtual ~RenderBufferAccessor () { }
    RenderBufferAccessor (csGenmeshMeshObject* parent)
    	: scfImplementationType (this)
    {
      this->parent = parent;
    }
    virtual void PreGetBuffer (csRenderBufferHolder* holder,
    	csRenderBufferName buffer)
    {
      if (parent) parent->PreGetBuffer (holder, buffer);
    }
  };
  csRef<RenderBufferAccessor> renderBufferAccessor;
  friend class eiRenderBufferAccessor;

  void PreGetBuffer (csRenderBufferHolder* holder, csRenderBufferName buffer);

  //------------------ iShaderVariableAccessor implementation ------------
  class ShaderVariableAccessor : 
    public scfImplementation1<ShaderVariableAccessor, iShaderVariableAccessor>
  {
  public:
    csWeakRef<csGenmeshMeshObject> parent;
    virtual ~ShaderVariableAccessor () { }
    ShaderVariableAccessor (csGenmeshMeshObject* parent)
    	: scfImplementationType (this)
    {
      this->parent = parent;
    }
    virtual void PreGetValue (csShaderVariable* variable)
    {
      if (parent) parent->PreGetShaderVariableValue (variable);
    }
  };
  csRef<ShaderVariableAccessor> shaderVariableAccessor;
  friend class eiShaderVariableAccessor;

  void PreGetShaderVariableValue (csShaderVariable* variable);
};

/**
 * Factory for general meshes.
 */
class csGenmeshMeshObjectFactory : 
  public scfImplementationExt2<csGenmeshMeshObjectFactory,
                               csObjectModel,
                               iMeshObjectFactory,
                               iGeneralFactoryState>
{
private:
  csRef<iMaterialWrapper> material;
  csDirtyAccessArray<csVector3> mesh_vertices;
  csDirtyAccessArray<csVector2> mesh_texels;
  csDirtyAccessArray<csVector3> mesh_normals;
  csDirtyAccessArray<csColor4> mesh_colors;

  bool autonormals;
  bool autonormals_compress;
  bool do_fullbright;

  bool mesh_vertices_dirty_flag;
  bool mesh_texels_dirty_flag;
  bool mesh_normals_dirty_flag;
  bool mesh_colors_dirty_flag;
  bool mesh_triangle_dirty_flag;
  bool mesh_tangents_dirty_flag;

  csDirtyAccessArray<csTriangle> mesh_triangles;

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

  SubMeshesContainer subMeshes;

  uint default_mixmode;
  bool default_lighting;
  csColor default_color;
  bool default_manualcolors;
  bool default_shadowcasting;
  bool default_shadowreceiving;

  float radius;
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
  csRef<csGenmeshMeshObjectType> genmesh_type;
  csRef<iLightManager> light_mgr;
  csFlags flags;

  iEngine* engine;

  SubMeshesContainer& GetSubMeshes ()
  {
    return subMeshes;
  }

  /// Constructor.
  csGenmeshMeshObjectFactory (csGenmeshMeshObjectType* pParent,
  	iObjectRegistry* object_reg);

  /// Destructor.
  virtual ~csGenmeshMeshObjectFactory ();

  /// Do full bright.
  bool DoFullBright () const { return do_fullbright; }

  bool SetMaterialWrapper (iMaterialWrapper* material)
  {
    csGenmeshMeshObjectFactory::material = material;
    return true;
  }
  iMaterialWrapper* GetMaterialWrapper () const { return material; }
  void AddVertex (const csVector3& v,
      const csVector2& uv, const csVector3& normal,
      const csColor4& color);
  void SetVertexCount (int n);
  int GetVertexCount () const { return (int)mesh_vertices.Length (); }
  csVector3* GetVertices ()
  {
    SetupFactory ();
    return mesh_vertices.GetArray ();
  }
  csVector2* GetTexels ()
  {
    SetupFactory ();
    return mesh_texels.GetArray ();
  }
  csVector3* GetNormals ()
  {
    SetupFactory ();
    return mesh_normals.GetArray ();
  }
  csColor4* GetColors ()
  {
    SetupFactory ();
    return mesh_colors.GetArray ();
  }

  void AddTriangle (const csTriangle& tri);
  void SetTriangleCount (int n);

  int GetTriangleCount () const { return (int)mesh_triangles.Length (); }
  csTriangle* GetTriangles ()
  {
    SetupFactory ();
    return mesh_triangles.GetArray ();
  }

  void Invalidate ();
  void CalculateNormals (bool compress);
  void Compress ();
  void GenerateBox (const csBox3& box);
  void GenerateSphere (const csEllipsoid& ellips, int rim_vertices,
      	bool cyl_mapping = false, bool toponly = false,
	bool reversed = false);
  //void GeneratePlane (const csPlane3& plane);
  void SetBack2Front (bool b2f);
  bool IsBack2Front () const { return back2front; }
  void BuildBack2FrontTree ();

  bool AddRenderBuffer (const char *name, iRenderBuffer* buffer);
  bool RemoveRenderBuffer (const char *name);
  int GetRenderBufferCount () const
  {
    return (int)this->user_buffer_names.GetSize ();
  }
  csRef<iRenderBuffer> GetRenderBuffer (int index); 
  csRef<iString> GetRenderBufferName (int index) const;

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
    int tricount, iMaterialWrapper *material, uint mixmode);
  virtual void AddSubMesh (unsigned int *triangles, int tricount, 
    iMaterialWrapper *material)
  {
    if (polyMeshType != Submeshes) SetPolyMeshSubmeshes();
    AddSubMesh (triangles, tricount, material, (uint)~0);
  }
  iGeneralMeshSubMesh* AddSubMesh (iRenderBuffer* indices, 
    iMaterialWrapper *material, const char* name, uint mixmode);
  iGeneralMeshSubMesh* FindSubMesh (const char* name) const
  {
    return subMeshes.FindSubMesh (name);
  }
  void DeleteSubMesh (iGeneralMeshSubMesh* mesh)
  {
    subMeshes.DeleteSubMesh (mesh);
  }
  size_t GetSubMeshCount () const
  {
    return subMeshes.GetSubMeshCount ();
  }
  iGeneralMeshSubMesh* GetSubMesh (size_t index) const
  {
    return subMeshes.GetSubMesh (index);
  }

  void SetObjectBoundingBox (const csBox3& bbox);
  float GetRadius ();

  /**\name iObjectModel implementation
   * @{ */
  virtual void GetObjectBoundingBox (csBox3& bbox)
  {
    bbox = GetObjectBoundingBox ();
  }
  virtual const csBox3& GetObjectBoundingBox ();
  virtual void GetRadius (float& rad, csVector3& cent)
  {
    rad = GetRadius ();
    cent = object_bbox.GetCenter ();
  }
  /** @} */

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
  virtual iMeshObjectType* GetMeshObjectType () const; 

  //------------------ iPolygonMesh interface implementation ----------------//
  struct PolyMesh : public scfImplementation1<PolyMesh, iPolygonMesh>
  {
  private:
    csGenmeshMeshObjectFactory* factory;
    csFlags flags;
  public:
    //SCF_DECLARE_IBASE;

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

    PolyMesh () : scfImplementationType (this)
    {
      flags.Set (CS_POLYMESH_TRIANGLEMESH);
    }
    virtual ~PolyMesh ()
    {
    }
  };
  csRef<iPolygonMesh> polygonMesh;
  friend struct PolyMesh;
  enum { Standard, Submeshes } polyMeshType;

  void SetPolyMeshStandard ();
  void SetPolyMeshSubmeshes ();
  virtual iObjectModel* GetObjectModel () { return this; }

  /// Genmesh factory shader variable accessor
  class ShaderVariableAccessor : 
    public scfImplementation1<ShaderVariableAccessor, iShaderVariableAccessor>
  {
  public:
    //SCF_DECLARE_EMBEDDED_IBASE (csGenmeshMeshObjectFactory);
    csWeakRef<csGenmeshMeshObjectFactory> parent;
    virtual ~ShaderVariableAccessor ()
    {
    }
    ShaderVariableAccessor (csGenmeshMeshObjectFactory* parent) :
      scfImplementationType (this)
    {
      this->parent = parent;
    }
    virtual void PreGetValue (csShaderVariable* variable)
    {
      //scfParent->PreGetShaderVariableValue (variable);
      if (parent) parent->PreGetShaderVariableValue (variable);
    }
  };
  csRef<ShaderVariableAccessor> shaderVariableAccessor;
  friend class ShaderVariableAccessor;

  void PreGetShaderVariableValue (csShaderVariable* variable);

  /// Genmesh factory render buffer accessor
  class RenderBufferAccessor : 
    public scfImplementation1<RenderBufferAccessor, iRenderBufferAccessor>
  {
  public:
    CS_LEAKGUARD_DECLARE (eiRenderBufferAccessor);
    csWeakRef<csGenmeshMeshObjectFactory> parent;
    virtual ~RenderBufferAccessor ()
    {
    }
    RenderBufferAccessor (csGenmeshMeshObjectFactory* parent) :
      scfImplementationType (this)
    {
      this->parent = parent;
    }
    virtual void PreGetBuffer (csRenderBufferHolder* holder,
    	csRenderBufferName buffer)
    {
      parent->PreGetBuffer (holder, buffer);
    }
  };
  csRef<RenderBufferAccessor> renderBufferAccessor;
  friend class RenderBufferAccessor;

  void PreGetBuffer (csRenderBufferHolder* holder, csRenderBufferName buffer);
};

#include "csutil/win32/msvc_deprecated_warn_on.h"

/**
 * Genmesh type. This is the plugin you have to use to create instances
 * of csGenmeshMeshObjectFactory.
 */
class csGenmeshMeshObjectType : 
  public scfImplementation2<csGenmeshMeshObjectType, 
                            iMeshObjectType,
                            iComponent>
{
public:
  iObjectRegistry* object_reg;
  bool do_verbose;
  MergedSVContext::Pool mergedSVContextPool;
  csStringHash submeshNamePool;

  /// Constructor.
  csGenmeshMeshObjectType (iBase*);
  /// Destructor.
  virtual ~csGenmeshMeshObjectType ();
  /// Draw.
  virtual csPtr<iMeshObjectFactory> NewFactory ();
  /// Initialize.
  bool Initialize (iObjectRegistry* object_reg);
};

}
CS_PLUGIN_NAMESPACE_END(Genmesh)

#endif // __CS_GENMESH_H__
