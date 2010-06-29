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
#include "csutil/scfarray.h"
#include "csutil/weakref.h"
#include "iengine/light.h"
#include "imesh/genmesh.h"
#include "imesh/object.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/virtclk.h"
#include "iutil/strset.h"
#include "ivideo/graph3d.h"
#include "ivideo/rendermesh.h"
#include "ivideo/rndbuf.h"

#include "submeshes.h"

class csBSPTree;
class csColor;
class csColor4;
struct iCacheManager;
struct iEngine;
struct iMaterialWrapper;
struct iMovable;
struct iObjectRegistry;

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
  csShaderVariable* GetVariable (CS::ShaderVarStringID name) const
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
  void PushVariables (csShaderVariableStack& stack) const
  { 
    context2->PushVariables (stack);
    context1->PushVariables (stack);
  }

  bool IsEmpty () const { return context1->IsEmpty() && context2->IsEmpty(); }

  void ReplaceVariable (csShaderVariable *variable) { }
  void Clear () { }
  bool RemoveVariable  (csShaderVariable *) { return false; }
  bool RemoveVariable  (CS::ShaderVarStringID) { return false; }
};

/**
 * Genmesh version of mesh object.
 */
class csGenmeshMeshObject : public scfImplementation2<csGenmeshMeshObject, 
						      iMeshObject,
						      iGeneralMeshState>
{
private:

  csRenderMeshHolder rmHolder;
  csRef<csShaderVariableContext> svcontext;
  csRef<csRenderBufferHolder> bufferHolder;
  csWeakRef<iGraphics3D> g3d;
  bool mesh_colors_dirty_flag;
  bool mesh_user_rb_dirty_flag;

  uint buffers_version;

  size_t factory_user_rb_state;

  csDirtyAccessArray<csRenderMesh*> renderMeshes;
  mutable SubMeshProxiesContainer subMeshes;
  mutable uint factorySubMeshesChangeNum;
  void UpdateSubMeshProxies () const;

  csUserRenderBufferManager userBuffers;
  csArray<CS::ShaderVarStringID> user_buffer_names;

  csGenmeshMeshObjectFactory* factory;
  iMeshWrapper* logparent;
  csRef<iMeshObjectDrawCallback> vis_cb;
  bool do_manual_colors;
  csColor4 base_color;
  float current_lod;
  uint32 current_features;
  csFlags flags;
  int forced_prog_lod_level;

  struct LegacyLightingData
  {
    csColor4* lit_mesh_colors;
    int num_lit_mesh_colors;	// Should be equal to factory number.
    
    csRef<iRenderBuffer> color_buffer;
    
    LegacyLightingData();
    ~LegacyLightingData();
    
    void SetColorNum (int num);
    void Free();
    void Clear(const csColor4& base_color);
  };
  LegacyLightingData legacyLighting;

  bool initialized;

  /// Current movable number.
  long cur_movablenr;

  /**
   * Setup this object. This function will check if setup is needed.
   */
  void SetupObject ();

  /// Get positions buffer
  iRenderBuffer* GetPositions();
  
  int ComputeProgLODLevel();
  
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
    subMeshes.GetDefaultSubmesh()->SubMeshProxy::SetMixmode (mode);
  }
  uint GetMixMode () const
  { return subMeshes.GetDefaultSubmesh()->SubMeshProxy::GetMixmode(); }
  const csColor& GetColor () const { return base_color; }
  /** @} */
  
  /**\name iGeneralMeshState implementation
   * @{ */
  void SetLighting (bool l) { }
  bool IsLighting () const { return false; }
  void SetManualColors (bool m) { do_manual_colors = m; }
  bool IsManualColors () const { return do_manual_colors; }
  const csBox3& GetObjectBoundingBox ();
  void SetObjectBoundingBox (const csBox3& bbox);
  void GetRadius (float& rad, csVector3& cent);
  void SetShadowCasting (bool m) { }
  bool IsShadowCasting () const { return true; }
  void SetShadowReceiving (bool m) { }
  bool IsShadowReceiving () const { return false; }
  iGeneralMeshSubMesh* FindSubMesh (const char* name) const; 
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
  struct AnimBuffers
  {
    csRef<iRenderBuffer> position;
    csRef<iRenderBuffer> texcoord;
    csRef<iRenderBuffer> normal;
  };
  AnimBuffers animBuffers;

  // This function sets up the shader variable context depending on
  // the existance of an animation control. If there is no animation control
  // then the factory will provide the vertex and other data. Otherwise the
  // mesh itself will do it.
  void SetupShaderVariableContext ();

  bool AddRenderBuffer (const char *name, iRenderBuffer* buffer);
  bool AddRenderBuffer (csRenderBufferName name, iRenderBuffer* buffer);
  bool RemoveRenderBuffer (const char *name);
  bool RemoveRenderBuffer (csRenderBufferName name);
  int GetRenderBufferCount () const
  {
    return (int)this->user_buffer_names.GetSize ();
  }
  iRenderBuffer* GetRenderBuffer (int index); 
  csRef<iString> GetRenderBufferName (int index) const;
  iRenderBuffer* GetRenderBuffer (const char* name);
  iRenderBuffer* GetRenderBuffer (csRenderBufferName name);

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
	iMaterialWrapper** material = 0, iMaterialArray* materials = 0);
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
    return true;
  }
  virtual bool GetColor (csColor& col) const { col = base_color; return true; }
  virtual bool SetMaterialWrapper (iMaterialWrapper* mat);
  virtual iMaterialWrapper* GetMaterialWrapper () const
  { return subMeshes.GetMaterialWrapper(); }

  /**
   * see imesh/object.h for specification. The default implementation
   * does nothing.
   */
  virtual void PositionChild (iMeshObject* /*child*/, csTicks /*current_time*/) { }

  virtual void BuildDecal(const csVector3* pos, float decalRadius,
          iDecalBuilder* decalBuilder);
  /** @} */

  class RenderBufferAccessor : 
    public scfImplementation1<RenderBufferAccessor, iRenderBufferAccessor>
  {
  public:
    CS_LEAKGUARD_DECLARE (RenderBufferAccessor);
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
  friend class RenderBufferAccessor;

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
  friend class ShaderVariableAccessor;

  void PreGetShaderVariableValue (csShaderVariable* variable);
  
  virtual void ForceProgLODLevel(int level)
  {
    forced_prog_lod_level = level;
  }
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
public:
  bool autonormals;
  bool autonormals_compress;
  bool do_fullbright;

  csWeakRef<iGraphics3D> g3d;
  csRef<iShaderVarStringSet> svstrings;

  struct KnownBuffers
  {
    csRef<iRenderBuffer> position;
    csRef<iRenderBuffer> texcoord;
    csRef<iRenderBuffer> normal;
    csRef<iRenderBuffer> color;
    
    csRef<iRenderBuffer> tangent;
    csRef<iRenderBuffer> bitangent;
  };
  KnownBuffers knownBuffers;
    
  csUserRenderBufferManager userBuffers;
  csArray<CS::ShaderVarStringID> user_buffer_names;
   
  struct LegacyBuffers
  {
    uint buffersSetup;
    bool mesh_vertices_dirty_flag;
    bool mesh_texels_dirty_flag;
    bool mesh_normals_dirty_flag;
    bool mesh_colors_dirty_flag;
   
    csDirtyAccessArray<csVector3> mesh_vertices;
    csDirtyAccessArray<csVector2> mesh_texels;
    csDirtyAccessArray<csVector3> mesh_normals;
    csDirtyAccessArray<csColor4> mesh_colors;
     
    LegacyBuffers();
  };
  LegacyBuffers legacyBuffers;
  void CreateLegacyBuffers();
  void ClearLegacyBuffers (uint mask = (uint)CS_BUFFER_ALL_MASK);
  void UpdateFromLegacyBuffers();
  
  struct SlidingWindow
  {
    int start_index;
    int end_index;
    SlidingWindow() {}
    SlidingWindow(int s, int e): start_index(s), end_index(e) {}
  };
  csArray<SlidingWindow> sliding_windows;
  
  SubMeshesContainer subMeshes;

  bool default_lighting;
  csColor default_color;
  bool default_manualcolors;
  bool default_shadowcasting;
  bool default_shadowreceiving;

  size_t user_buffer_change;

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

  /// Calculate bounding box and radius.
  void CalculateBBoxRadius ();

  /**
   * Setup this factory. This function will check if setup is needed.
   */
  void SetupFactory ();

  /// Update tangent and bitangent buffers
  void UpdateTangentsBitangents ();
  
  /// Get positions buffer
  iRenderBuffer* GetPositions();
public:
  CS_LEAKGUARD_DECLARE (csGenmeshMeshObjectFactory);

  csRef<iVirtualClock> vc;

  uint buffers_version;

  bool back2front;
  csBSPTree* back2front_tree;

  iObjectRegistry* object_reg;
  iMeshFactoryWrapper* logparent;
  csRef<csGenmeshMeshObjectType> genmesh_type;
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
    subMeshes.SetMaterialWrapper (material);
    return true;
  }
  iMaterialWrapper* GetMaterialWrapper () const
  { return subMeshes.GetMaterialWrapper(); }
  void AddVertex (const csVector3& v,
      const csVector2& uv, const csVector3& normal,
      const csColor4& color);
  void SetVertexCount (int n);
  int GetVertexCount () const;
  csVector3* GetVertices ();
  csVector2* GetTexels ();
  csVector3* GetNormals ();
  csColor4* GetColors (bool ensureValid);
  csColor4* GetColors ();

  void AddTriangle (const csTriangle& tri);
  void SetTriangleCount (int n);

  int GetTriangleCount () const;
  csTriangle* GetTriangles ();

  void Invalidate ();
  void CalculateNormals (bool compress);
  void DisableAutoNormals ()
  { autonormals = false; }
  void Compress ();
  void GenerateBox (const csBox3& box);
  void GenerateCylinder (float l, float r, uint sides);
  void GenerateCapsule (float l, float r, uint sides);
  void GenerateSphere (const csEllipsoid& ellips, int rim_vertices,
      	bool cyl_mapping = false, bool toponly = false,
	bool reversed = false);
  //void GeneratePlane (const csPlane3& plane);
  void SetBack2Front (bool b2f);
  bool IsBack2Front () const { return back2front; }
  void BuildBack2FrontTree ();

  bool InternalSetBuffer (csRenderBufferName name, iRenderBuffer* buffer);
  
  bool AddRenderBuffer (const char *name, iRenderBuffer* buffer);
  bool AddRenderBuffer (csRenderBufferName name, iRenderBuffer* buffer);
  bool RemoveRenderBuffer (const char *name);
  bool RemoveRenderBuffer (csRenderBufferName name);
  int GetRenderBufferCount () const
  {
    return (int)this->user_buffer_names.GetSize ();
  }
  iRenderBuffer* GetRenderBuffer (int index); 
  csRef<iString> GetRenderBufferName (int index) const;
  iRenderBuffer* GetRenderBuffer (const char* name);
  iRenderBuffer* GetRenderBuffer (csRenderBufferName name);

  /**
   * Get the string ID's for the anonymous buffers
   */
  const csArray<CS::ShaderVarStringID>& GetUserBufferNames ()
  { return user_buffer_names; }
  const csUserRenderBufferManager& GetUserBuffers()
  { return userBuffers; }
  iShaderVarStringSet* GetSVStrings()
  { return svstrings; }

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
  virtual const csBox3& GetObjectBoundingBox ();
  virtual void GetRadius (float& rad, csVector3& cent)
  {
    rad = GetRadius ();
    cent = object_bbox.GetCenter ();
  }
  /** @} */

  void SetMixMode (uint mode)
  {
    subMeshes.GetDefaultSubmesh()->SubMesh::SetMixmode (mode);
  }
  uint GetMixMode () const
  {
    return subMeshes.GetDefaultSubmesh()->SubMesh::GetMixmode();
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
  
  virtual void ClearSlidingWindows()
  {
    sliding_windows.SetSize(0);
  }
  
  virtual int GetSlidingWindowSize() const
  {
    return sliding_windows.GetSize();
  }
  
  virtual void AddSlidingWindow(int start_index, int end_index)
  {
    sliding_windows.Push(SlidingWindow(start_index, end_index));
  }
  
  virtual void GetSlidingWindow(int index, int& out_start_index, int& out_end_index) const
  {
    CS_ASSERT(index >= 0 && index < sliding_windows.GetSize());
    out_start_index = sliding_windows[index].start_index;
    out_end_index = sliding_windows[index].end_index;
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

  enum { Standard, Submeshes } polyMeshType;

  void SetPolyMeshStandard ();
  void SetPolyMeshSubmeshes ();
  virtual iObjectModel* GetObjectModel () { return this; }

  /// Genmesh factory shader variable accessor
  class ShaderVariableAccessor : 
    public scfImplementation1<ShaderVariableAccessor, iShaderVariableAccessor>
  {
  public:
    csWeakRef<csGenmeshMeshObjectFactory> parent;
    ShaderVariableAccessor (csGenmeshMeshObjectFactory* parent) :
      scfImplementationType (this), parent (parent) {}
    virtual void PreGetValue (csShaderVariable* variable)
    {
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
    CS_LEAKGUARD_DECLARE (RenderBufferAccessor);
    csWeakRef<csGenmeshMeshObjectFactory> parent;
    RenderBufferAccessor (csGenmeshMeshObjectFactory* parent) :
      scfImplementationType (this), parent (parent) {}
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
  csStringID base_id;

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
