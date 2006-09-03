/*
  Copyright (C) 2005 by Jorrit Tyberghein
  Copyright (C) 2006 by Piotr Obrzut

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

#ifndef __CS_INSTMESH_H__
#define __CS_INSTMESH_H__

#include "cstool/objmodel.h"
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
#include "imesh/lighting.h"
#include "imesh/object.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/virtclk.h"
#include "ivideo/graph3d.h"
#include "ivideo/rendermesh.h"
#include "ivideo/rndbuf.h"

#include "imesh/instmesh.h"

class csBSPTree;
class csColor;
class csColor4;
class csInstmeshMeshObject;
class csInstmeshMeshObjectFactory;
class csInstmeshMeshObjectType;
class csPolygonMesh;
struct iCacheManager;
struct iEngine;
struct iMaterialWrapper;
struct iMovable;
struct iObjectRegistry;
struct iShadowBlockList;
class csInstmeshMeshObject;
class csInstmeshMeshObjectFactory;
class csInstmeshMeshObjectType;

CS_PLUGIN_NAMESPACE_BEGIN(InstMesh)
{

class csInstmeshMeshObject;
class csInstmeshMeshObjectFactory;
class csInstmeshMeshObjectType;

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
/**
* Instmesh version of mesh object.
*/
class csInstmeshMeshObject : 
  public scfImplementationExt5<csInstmeshMeshObject, 
                               csObjectModel,
                               iMeshObject,
                               iLightingInfo,
                               iShadowCaster,
                               iShadowReceiver,
                               iInstancingMeshState>
{
private:
  csRenderMeshHolder rmHolder;
  csRef<csRenderBufferHolder> bufferHolder;
  csWeakRef<iGraphics3D> g3d;

  csHash<csInstance*> instances;
  csRef<csShaderVariable> instance_template;
  csArray<size_t> var_vect_indices;
  static size_t max_instance_id;

  iMovable* lighting_movable;

  csDirtyAccessArray<csRenderMesh*> renderMeshes;

  csInstmeshMeshObjectFactory* factory;
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
  bool autobb;

  bool do_shadows;
  bool do_shadow_rec;

  csColor4* lit_fact_colors;
  // Should be equal to factory number times the number of instances.
  size_t num_lit_fact_colors;
  csColor4* static_fact_colors;

  float radius;
  csBox3 object_bbox;
  bool object_bbox_valid;

  void SetupShaderParams ();

  csArray<csVector4> Variable2Vectors (csShaderVariable& parameter);

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
  * Make sure the 'lit_fact_colors' array has the right size.
  * Also clears the pseudo-dynamic light hash if the vertex count
  * changed!
  */
  void CheckLitColors ();

  /**
  * Process one light and add the values to the instmesh light table.
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
  csInstmeshMeshObject (csInstmeshMeshObjectFactory* factory);

  CS_LEAKGUARD_DECLARE (csInstmeshMeshObject);

  /// Destructor.
  virtual ~csInstmeshMeshObject ();

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
  void GetRadius (float& rad, csVector3& cent);
  void SetShadowCasting (bool m) { do_shadows = m; }
  void SetShadowReceiving (bool m) { do_shadow_rec = m; }
  bool IsShadowCasting () const { return do_shadows; }
  bool IsShadowReceiving () const { return do_shadow_rec; }

  size_t GetInstancesCount (){return instances.GetSize ();}

  iVirtualClock* vc;

  // Instancing functions.
  size_t AddInstance ();
  void RemoveInstance (size_t id);
  void RemoveAllInstances ();
  size_t AddInstancesVariable (const csShaderVariable& parameter);
  const csShaderVariable& GetInstanceVariable (size_t instance_id, size_t variable_id);
  void SetInstanceVariable (size_t instance_id, size_t variable_id, 
    const csVector3 &variable);
  void SetInstanceVariable (size_t instance_id, csShaderVariable variable);
  void SetAutomaticBoundingBoxCreation (bool flag){autobb = flag;} 
  void SetupShaderVariableContext ();

  void SetBoundingBox (const csBox3& box);

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
  void DisconnectAllLights ();

  //----------------------- iMeshObject implementation ----------------------
  virtual iMeshObjectFactory* GetFactory () const {return (iMeshObjectFactory*)factory;}
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
    const csVector3& /*pos*/, uint /*framenr*/) { }
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

  virtual bool SetColor (const csColor& col)
  {
    base_color.Set (col);
    lighting_dirty = true;
    return true;
  }
  virtual bool GetColor (csColor& col) const { col = base_color; return true; }
  bool SetMaterialWrapper (iMaterialWrapper* mat);
  virtual iMaterialWrapper* GetMaterialWrapper () const { return material; }
  virtual void InvalidateMaterialHandles () { }
  /**
  * see imesh/object.h for specification. The default implementation
  * does nothing.
  */
  virtual void PositionChild (iMeshObject* /*child*/, csTicks /*current_time*/) { }

  /// Calculate bounding box and radius.
  void CalculateBBoxRadius ();
  void SetObjectBoundingBox (const csBox3& bbox);
  float GetRadius ();
  /**\name iObjectModel implementation
   * @{ */
  virtual void GetObjectBoundingBox (csBox3& bbox)
  {
    bbox = GetObjectBoundingBox ();
  }
  virtual const csBox3& GetObjectBoundingBox ();

  virtual iObjectModel* GetObjectModel () { return this; }

  //------------------ iPolygonMesh interface implementation ----------------//
  struct PolyMesh : public scfImplementation1<PolyMesh, iPolygonMesh>
  {
  private:
    csFlags flags;
    csInstmeshMeshObject* parent;

  public:

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

    PolyMesh (csInstmeshMeshObject* parent) : scfImplementationType (this),
      parent (parent)
    {
      flags.Set (CS_POLYMESH_TRIANGLEMESH);
    }
    virtual ~PolyMesh () { }
    };
  csRef<PolyMesh> polygonMesh;
  friend struct PolyMesh;
};

/**
* Factory for general meshes.
*/
class csInstmeshMeshObjectFactory : 
  public scfImplementation2<csInstmeshMeshObjectFactory,
                            iMeshObjectFactory,
                            iInstancingFactoryState>
{
private:
  friend class csInstmeshMeshObject;	//@@@ FIXME: remove

  csRef<iMaterialWrapper> material;
  csDirtyAccessArray<csVector3> fact_vertices;
  csDirtyAccessArray<csVector2> fact_texels;
  csDirtyAccessArray<csVector3> fact_normals;
  csDirtyAccessArray<csColor4> fact_colors;
  csDirtyAccessArray<csTriangle> fact_triangles;

  bool vertices_dirty_flag;
  bool texels_dirty_flag;
  bool normals_dirty_flag;
  bool colors_dirty_flag;
  bool triangle_dirty_flag;
  bool tangents_dirty_flag;

  csRef<iRenderBuffer> vertex_buffer;
  csRef<iRenderBuffer> texel_buffer;
  csRef<iRenderBuffer> normal_buffer;
  //csRef<iRenderBuffer> color_buffer;
  csRef<iRenderBuffer> index_buffer;
  csRef<iRenderBuffer> binormal_buffer;
  csRef<iRenderBuffer> tangent_buffer;

  csBox3 factory_bbox;
  float factory_radius;

  bool autonormals;
  bool autonormals_compress;
  bool do_fullbright;

  csWeakRef<iGraphics3D> g3d;
  csRef<iStringSet> strings;

  uint default_mixmode;
  bool default_lighting;
  csColor default_color;
  bool default_manualcolors;
  bool default_shadowcasting;
  bool default_shadowreceiving;

public:
  CS_LEAKGUARD_DECLARE (csInstmeshMeshObjectFactory);

  csRef<iVirtualClock> vc;

  iObjectRegistry* object_reg;
  iMeshFactoryWrapper* logparent;
  iMeshObjectType* instmesh_type;
  csRef<iLightManager> light_mgr;
  csFlags flags;

  iEngine* engine;

  /// Constructor.
  csInstmeshMeshObjectFactory (iMeshObjectType *pParent,
    iObjectRegistry* object_reg);

  /// Destructor.
  virtual ~csInstmeshMeshObjectFactory ();

  csArray<csVector4> Variable2Vectors (const csShaderVariable& parameter);

  /// Get the bounding box.
  const csBox3& GetFactoryBox () const { return factory_bbox; }
  /// Get the bounding radius.
  const float GetFactoryRadius () const { return factory_radius; }

  /// Calculate the factory bounding box and sphere.
  void CalculateBoundingVolumes ();

  /// Do full bright.
  bool DoFullBright () const { return do_fullbright; }

  bool SetMaterialWrapper (iMaterialWrapper* material)
  {
    csInstmeshMeshObjectFactory::material = material;
    return true;
  }
  iMaterialWrapper* GetMaterialWrapper () const { return material; }
  void AddVertex (const csVector3& v,
    const csVector2& uv, const csVector3& normal,
    const csColor4& color);
  size_t GetVertexCount () const { return fact_vertices.Length (); }
  const csVector3* GetVertices ()
  {
    return fact_vertices.GetArray ();
  }
  const csVector2* GetTexels ()
  {
    return fact_texels.GetArray ();
  }
  const csVector3* GetNormals ()
  {
    return fact_normals.GetArray ();
  }
  const csColor4* GetColors ()
  {
    return fact_colors.GetArray ();
  }

  void AddTriangle (const csTriangle& tri)
  {
    fact_triangles.Push (tri);
  }
  size_t GetTriangleCount () const { return fact_triangles.Length (); }
  const csTriangle* GetTriangles () { return fact_triangles.GetArray (); }

  void CalculateNormals (bool compress);
  void Compress ();
  void GenerateBox (const csBox3& box);
  void GenerateQuad (const csVector3& v1, const csVector3& v2, 
    const csVector3& v3, const csVector3& v4);
  void GenerateSphere (const csEllipsoid& sphere, int rim_vertices,
    bool cyl_mapping = false, bool toponly = false,
    bool reversed = false);

  iStringSet* GetStrings()
  { return strings; }

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
  virtual iMeshObjectType* GetMeshObjectType () const { return instmesh_type; }

  virtual iObjectModel* GetObjectModel () { return 0; }

  class RenderBufferAccessor : 
    public scfImplementation1<RenderBufferAccessor, iRenderBufferAccessor>
  {
  public:
    CS_LEAKGUARD_DECLARE (eiRenderBufferAccessor);
    csWeakRef<csInstmeshMeshObjectFactory> parent;
    virtual ~RenderBufferAccessor ()
    {
    }
    RenderBufferAccessor (csInstmeshMeshObjectFactory* parent) : 
      scfImplementationType (this), parent (parent)
    {
    }
    virtual void PreGetBuffer (csRenderBufferHolder* holder,
    	csRenderBufferName buffer)
    {
      if (parent) parent->PreGetBuffer (holder, buffer);
    }
  };
  csRef<RenderBufferAccessor> renderBufferAccessor;

  void PreGetBuffer (csRenderBufferHolder* holder, csRenderBufferName buffer);
};

/**
* Instmesh type. This is the plugin you have to use to create instances
* of csInstmeshMeshObjectFactory.
*/
class csInstmeshMeshObjectType : 
  public scfImplementation2<csInstmeshMeshObjectType,
                            iMeshObjectType,
                            iComponent>
{
public:
  iObjectRegistry* object_reg;
  bool do_verbose;

  /// Constructor.
  csInstmeshMeshObjectType (iBase*);
  /// Destructor.
  virtual ~csInstmeshMeshObjectType ();
  /// Draw.
  virtual csPtr<iMeshObjectFactory> NewFactory ();
  /// Initialize.
  bool Initialize (iObjectRegistry* object_reg);
};

}
CS_PLUGIN_NAMESPACE_END(InstMesh)

#endif // __CS_INSTMESH_H__
