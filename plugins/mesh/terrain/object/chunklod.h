/*
    Copyright (C) 2003 by Jorrit Tyberghein, Daniel Duhprey

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

#ifndef __CS_CHUNKLOD_H__
#define __CS_CHUNKLOD_H__

#include "csutil/cscolor.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/weakref.h"

#include "csgeom/transfrm.h"
#include "csgeom/objmodel.h"

#include "imesh/object.h"
#include "imesh/terrain.h"
#include "imesh/lighting.h"

#include "iutil/comp.h"

#include "ivideo/rendermesh.h"

#include "iengine/lightmgr.h"
#include "iengine/shadcast.h"

#include "csgfx/shadervarcontext.h"

#include "cstool/rendermeshholder.h"

struct iMaterialWrapper;
struct iImage;
struct iGraphics3D;

class csChunkLodTerrainType;
class csChunkLodTerrainFactory;
class csChunkLodTerrainObject;

/**
 * ChunkLod terrain type, instantiates factories which create meshes
 */
class csChunkLodTerrainType : public iMeshObjectType
{
private:
  iObjectRegistry *object_reg;
  iBase* parent;

public:
  SCF_DECLARE_IBASE;	

  csChunkLodTerrainType (iBase* p);
  virtual ~csChunkLodTerrainType ();

  csPtr<iMeshObjectFactory> NewFactory();

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csChunkLodTerrainType);
    bool Initialize (iObjectRegistry* p)
    { scfParent->object_reg = p; return true; }
  } scfiComponent;
  friend struct eiComponent;
};


/**
 * The factory will keep track of the precomputed quad-tree hierarchy
 * each individual instance of the terrain (if there are more than one)
 * will determine which nodes in the tree are meant to be rendered 
 * (that determination is made during drawtest)
 */
class csChunkLodTerrainFactory : public iMeshObjectFactory
{
  friend class csChunkLodTerrainObject;
public:
  iBase* parent;
  iMeshObjectType* chunklod_type;
  iObjectRegistry *object_reg;
  csFlags flags;
  csWeakRef<iEngine> engine;
  csRef<iLightManager> light_mgr;

  csVector3 scale;
  struct Data
  {
    csVector3 pos;
    csVector3 norm;
    csVector3 tan;
    csVector3 bin;
    csVector2 tex;
    int col;
    float error;
    Data ()
    {
      pos.Set (0.0f);
      norm.Set (0.0f);
      tan.Set (0.0f);
      bin.Set (0.0f);
      tex.Set (0, 0);
      col = 0;
      error = 0.0;
    }
  };
  csArray<Data> datamap;
  int hm_x, hm_y;
  
  csRef<iTerraFormer> terraform;
  csRef<iTerraSampler> fullsample;

  csStringID vertex_name, compressed_vertex_name; 
  csStringID normal_name, compressed_normal_name;
  csStringID tangent_name, compressed_tangent_name;
  csStringID binormal_name, compressed_binormal_name;
  csStringID texcors_name, compressed_texcors_name;
  csStringID color_name, compressed_color_name;
  csStringID index_name;
  csWeakRef<iGraphics3D> r3d;
  csRef<iShaderManager> shmgr;

  class MeshTreeNode
  {
  private:
    csChunkLodTerrainFactory *pFactory;
    MeshTreeNode *children[4];

    csVector3 center;
    csBox3 box;
    float radius;
    float error;

    csDirtyAccessArray<csVector3> vertices;
    csRef<iRenderBuffer> vertex_buffer;
    csRef<iRenderBuffer> compressed_vertex_buffer;
    csDirtyAccessArray<csVector3> normals;
    csRef<iRenderBuffer> normal_buffer;
    csRef<iRenderBuffer> compressed_normal_buffer;
    csDirtyAccessArray<csVector3> tangents;
    csRef<iRenderBuffer> tangent_buffer;
    csRef<iRenderBuffer> compressed_tangent_buffer;
    csDirtyAccessArray<csVector3> binormals;
    csRef<iRenderBuffer> binormal_buffer;
    csRef<iRenderBuffer> compressed_binormal_buffer;
    csDirtyAccessArray<csVector2> texcors;
    csRef<iRenderBuffer> texcors_buffer;
    csRef<iRenderBuffer> compressed_texcors_buffer;
    csRef<iRenderBuffer> texcoords_norm_buffer;
    csRef<iRenderBuffer> color_buffer;
    csRef<iRenderBuffer> compressed_color_buffer;
    csRef<iRenderBuffer> index_buffer;
    
    int parity;
    int max_levels;

    void InitBuffer (const Data &d, int p);
    void AddVertex (const Data &d, int p);
    void EndBuffer (const Data &d, int p);
    void AddEdgeVertex (const Data& d, const Data& mod);
    void AddSkirtVertex (const Data& d, const Data& mod);
    void ProcessMap (int l, int i, int j, int k);
    void ProcessEdge (int start, int end, int move, const Data& mod);
    
  public:
    csArray<int> colors;
  
    MeshTreeNode (csChunkLodTerrainFactory* p, int x, int y, int w, int h,
    	float error);
    ~MeshTreeNode ();

    MeshTreeNode *GetChild (int i) 
    { CS_ASSERT (i >= 0 && i < 4); return (error > 0) ? children[i] : 0; }

    iRenderBuffer *GetRenderBuffer (csRenderBufferName name);

    const csVector3 &Center () { return center; }
    const csBox3 &BBox () { return box; }
    float Radius () { return radius; }
    float Error () { return error; }

    int Count () { return (int)vertices.Length(); }

    const csVector3* GetVertices() const { return vertices.GetArray(); }
    const csVector3* GetNormals() const { return normals.GetArray(); }
  } *root;
  friend class MeshTreeNode;

  void ComputeError (int i, int j, int di, int dj, int n, int w);

public:
  SCF_DECLARE_IBASE;

  csChunkLodTerrainFactory (csChunkLodTerrainType* p, iObjectRegistry* objreg);
  virtual ~csChunkLodTerrainFactory ();

  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> NewInstance ();
  virtual csPtr<iMeshObjectFactory> Clone () { return 0; }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual void SetLogicalParent (iBase* lp) { parent = lp; }
  virtual iBase* GetLogicalParent () const { return parent; }
  virtual iMeshObjectType* GetMeshObjectType () const { return chunklod_type; }
  virtual iObjectModel* GetObjectModel () { return &scfiObjectModel; }

  void GetObjectBoundingBox (csBox3& bbox);
  void SetObjectBoundingBox (const csBox3& bbox);
  void GetRadius (csVector3& rad, csVector3& c);

  struct eiObjectModel : public csObjectModel
  {
    SCF_DECLARE_EMBEDDED_IBASE (csChunkLodTerrainFactory);
    virtual void GetObjectBoundingBox (csBox3& b)
    { scfParent->GetObjectBoundingBox (b); }
    virtual void SetObjectBoundingBox (const csBox3& b)
    { scfParent->SetObjectBoundingBox (b); }
    virtual void GetRadius (csVector3& r, csVector3& c)
    { scfParent->GetRadius (r, c); }
  } scfiObjectModel;
  friend struct eiObjectModel;

  void SetTerraFormer (iTerraFormer *form);
  iTerraFormer *GetTerraFormer ();
  void SetSamplerRegion (const csBox2& region);
  const csBox2& GetSamplerRegion ();
  bool SaveState (const char *filename);
  bool RestoreState (const char *filename);

  struct eiTerrainFactoryState : public iTerrainFactoryState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csChunkLodTerrainFactory);
    void SetTerraFormer (iTerraFormer* form)
    { scfParent->SetTerraFormer (form); }
    iTerraFormer* GetTerraFormer ()
    { return scfParent->GetTerraFormer (); }
    void SetSamplerRegion (const csBox2& region)
    { scfParent->SetSamplerRegion (region); }
    const csBox2& GetSamplerRegion ()
    { return scfParent->GetSamplerRegion (); }
    virtual bool SaveState (const char *filename)
    { return scfParent->SaveState (filename); }
    virtual bool RestoreState (const char *filename) 
    { return scfParent->RestoreState (filename); }
  } scfiTerrainFactoryState;
  friend struct eiTerrainFactoryState;

  csVector3 CollisionDetect (const csVector3 &p);
};


/**
 * An array giving shadow information for a pseudo-dynamic light.
 */
class csShadowArray
{
public:
  iLight* light;
  // For every vertex of the mesh a value.
  float* shadowmap;

  csShadowArray () : shadowmap (0) { }
  ~csShadowArray ()
  {
    delete[] shadowmap;
  }
};

/**
 * Instance of an implementation of Thatcher Ulritch's Chunked LOD algorithm
 * for terrain rendering.  http://www.tulrich.com/geekstuff/chunklod.html
 * The factory is responsible for the preprocessing step on the quad-tree
 * hierarchy and each instance (this class) is responsible for determining
 * which of those nodes in the tree should be rendered (draw_test) and render
 * them (GetRenderMeshes)
 */
class csChunkLodTerrainObject : public iMeshObject
{
private:
  iBase* logparent;
  csChunkLodTerrainFactory* pFactory;
  iMeshObjectDrawCallback* vis_cb;

  csColor basecolor;
  csRef<iMaterialWrapper> matwrap;
  csArray<iMaterialWrapper*> palette;	// TODO@@@ Use csRefArray!!!
  csRefArray<iImage> alphas;

  csDirtyAccessArray<csColor> staticLights;
  uint colorVersion;
  csColor dynamic_ambient;
  // If we are using the iLightingInfo lighting system then this
  // is an array of lights that affect us right now.
  csSet<iLight*> affecting_lights;
  csHash<csShadowArray*, iLight*> pseudoDynInfo;

  float error_tolerance;
  float lod_distance;

  csRenderMeshHolderSingle rmHolder;
  csRenderMeshHolderMultiple returnMeshesHolder;
  csDirtyAccessArray<csRenderMesh*>* returnMeshes;
  csReversibleTransform tr_o2c;

  // Use for clipping during rendering.
  csPlane3 planes[10];

  int tricount;
  csFlags flags;
  bool staticLighting;
  bool castShadows;

  // Data for the colldet polygon mesh.
  bool polymesh_valid;
  csVector3* polymesh_vertices;
  int polymesh_vertex_count;
  csTriangle* polymesh_triangles;
  int polymesh_tri_count;
  csMeshedPolygon* polymesh_polygons;
  void SetupPolyMeshData ();

  class MeshTreeNodeWrapper : public iBase
  {
    csRef<MeshTreeNodeWrapper> children[4];
  public:
    csWeakRef<csChunkLodTerrainObject> obj;
    csChunkLodTerrainFactory::MeshTreeNode* factoryNode;
    csRef<csRenderBufferHolder> bufferHolder;

    SCF_DECLARE_IBASE;

    MeshTreeNodeWrapper (csChunkLodTerrainObject* obj,
      csChunkLodTerrainFactory::MeshTreeNode* node);
    virtual ~MeshTreeNodeWrapper();
    MeshTreeNodeWrapper* GetChild (int n);
  };
  friend class MeshTreeNodeWrapper;

  class MeshTreeNodeRBA : public iRenderBufferAccessor
  {
    csWeakRef<MeshTreeNodeWrapper> wrapper;
    csRef<iRenderBuffer> colorBuffer;
    uint colorVersion;
  public:
    SCF_DECLARE_IBASE;

    MeshTreeNodeRBA (MeshTreeNodeWrapper* wrapper);
    virtual ~MeshTreeNodeRBA ();

    virtual void PreGetBuffer (csRenderBufferHolder* holder, csRenderBufferName buffer);
  };    
  friend class MeshTreeNodeRBA;

  
  csRef<MeshTreeNodeWrapper> rootNode;
  iMovable* light_movable; 
  void UpdateColors (const csArray<int>& colors, const csVector3* vertices, 
    const csVector3* normals, const csBox3& box, csColor* staticColors);
public: 
  SCF_DECLARE_IBASE;

  csChunkLodTerrainObject (csChunkLodTerrainFactory* f);
  virtual ~csChunkLodTerrainObject ();

  /// Returns a point to the factory that made this
  iMeshObjectFactory* GetFactory () const
  {
    return (iMeshObjectFactory*)pFactory;
  }

  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> Clone () { return 0; }

  /**
   * Does all pre-render calculation.  Determines which LOD children in the 
   * tree should be drawn
   */
  bool DrawTestQuad (iRenderView* rv, MeshTreeNodeWrapper* node, float kappa,
	uint32 frustum_mask);
  bool DrawTest (iRenderView* rview, iMovable* movable, uint32 frustum_mask);

  /// Returns the mesh, ready for rendering
  virtual csRenderMesh** GetRenderMeshes (int &n, iRenderView* rview,
  	iMovable* movable, uint32 frustum_mask);

  virtual void SetVisibleCallback (iMeshObjectDrawCallback* cb) { vis_cb = cb; }
  virtual iMeshObjectDrawCallback* GetVisibleCallback () const
  { return vis_cb; }

  /// For animation ... ha ha
  virtual void NextFrame (csTicks, const csVector3&) { }

  /// Unsupported
  virtual void HardTransform (const csReversibleTransform&) { }

  /// Shows that HardTransform is not supported by this mesh
  virtual bool SupportsHardTransform () const { return false; }

  /// Check if the terrain is hit by the beam
  virtual bool HitBeamOutline (const csVector3& start, const csVector3& end, 
	csVector3& isect, float* pr);
  /// Find exact position of a beam hit
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end, 
	csVector3& isect, float* pr, int* polygon_idx = 0);

  /// Set/Get logical parent
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }
  virtual iBase* GetLogicalParent () const { return logparent; }

  /// Gets the objects model, not sure what this means yet
  virtual iObjectModel *GetObjectModel () { return &scfiObjectModel; }

  /// Set (Get) the terrain to a constant base color 
  virtual bool SetColor (const csColor& c) { basecolor = c; return true; }
  virtual bool GetColor (csColor &c) const { c = basecolor; return true; }

  /** 
   * Set (Get) the terrain to a single material, useful only with 
   * large textures or small terrains (or terrains in the distance)
   * See TerrainState for better texture settings
   */
  virtual bool SetMaterialWrapper (iMaterialWrapper* m)
  { matwrap = m; return true; }
  virtual iMaterialWrapper* GetMaterialWrapper () const { return matwrap; }
  virtual void InvalidateMaterialHandles () { }
  /**
   * see imesh/object.h for specification. The default implementation
   * does nothing.
   */
  virtual void PositionChild (iMeshObject* child,csTicks current_time) { }

  bool SetMaterialPalette (const csArray<iMaterialWrapper*>& pal);
  const csArray<iMaterialWrapper*>& GetMaterialPalette () const
  {
    return palette;
  }
  bool SetMaterialMap (const csArray<char>& data, int x, int y);
  bool SetMaterialMap (iImage* map);
  bool SetLODValue (const char* parameter, float value);
  float GetLODValue (const char* parameter) const;
  /// Saves the texture quad-tree into the file specified
  bool SaveState (const char *filename);
  bool RestoreState (const char *filename);
  int CollisionDetect (iMovable *m, csTransform *p);
  void SetStaticLighting (bool enable)
  { staticLighting = enable; }
  bool GetStaticLighting ()
  { return staticLighting; }
  void SetCastShadows (bool enable)
  { castShadows = enable; }
  bool GetCastShadows ()
  { return castShadows; }

  char* GenerateCacheName ();
  void InitializeDefault (bool clear);
  bool ReadFromCache (iCacheManager* cache_mgr);
  bool WriteToCache (iCacheManager* cache_mgr);
  void PrepareLighting ();
  void SetDynamicAmbientLight (const csColor& color);
  const csColor& GetDynamicAmbientLight ();
  void LightChanged (iLight* light);
  void LightDisconnect (iLight* light);
  void CastShadows (iMovable* movable, iFrustumView* fview);
  void AppendShadowTri (const csVector3& a, const csVector3& b, 
    const csVector3& c, iShadowBlock* list, const csVector3& origin);
  void AppendShadows (iMovable* movable, iShadowBlockList* shadows,
    const csVector3& origin);

  struct eiTerrainObjectState : public iTerrainObjectState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csChunkLodTerrainObject);
    virtual bool SetMaterialPalette (const csArray<iMaterialWrapper*>& pal) 
    { return scfParent->SetMaterialPalette (pal); }
    virtual const csArray<iMaterialWrapper*>& GetMaterialPalette () const
    { return scfParent->GetMaterialPalette (); }
    virtual bool SetMaterialMap (const csArray<char>& data, int x, int y)
    { return scfParent->SetMaterialMap (data, x, y); }
    virtual bool SetMaterialMap (iImage* map)
    { return scfParent->SetMaterialMap (map); }
    virtual bool SetLODValue (const char* parameter, float value)
    { return scfParent->SetLODValue (parameter, value); }
    virtual float GetLODValue (const char* parameter) const
    { return scfParent->GetLODValue (parameter); }
    virtual bool SaveState (const char *filename)
    { return scfParent->SaveState (filename); }
    virtual bool RestoreState (const char *filename) 
    { return scfParent->RestoreState (filename); }
    virtual int CollisionDetect (iMovable *m, csTransform *p)
    { return scfParent->CollisionDetect (m, p); }
    virtual void SetStaticLighting (bool enable)
    { scfParent->SetStaticLighting (enable); }
    virtual bool GetStaticLighting ()
    { return scfParent->GetStaticLighting (); }
    virtual void SetCastShadows (bool enable)
    { scfParent->SetCastShadows (enable); }
    virtual bool GetCastShadows ()
    { return scfParent->GetCastShadows (); }
  } scfiTerrainObjectState;
  friend struct eiTerrainObjectState;

  //------------------ iPolygonMesh interface implementation ----------------//
  struct PolyMesh : public iPolygonMesh
  {
  private:
    csChunkLodTerrainObject* terrain;
    csFlags flags;
  public:
    SCF_DECLARE_IBASE;

    void SetTerrain (csChunkLodTerrainObject* t)
    {
      terrain = t;
      flags.SetAll (CS_POLYMESH_TRIANGLEMESH);
    }
    void Cleanup ();

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
    { SCF_CONSTRUCT_IBASE (0); }
    virtual ~PolyMesh ()
    { SCF_DESTRUCT_IBASE (); }
  } scfiPolygonMesh;
  friend class PolyMesh;

  //------------------ iObjectModel interface implementation ----------------//
  struct eiObjectModel : public csObjectModel
  {
    SCF_DECLARE_EMBEDDED_IBASE (csChunkLodTerrainObject);
    virtual void GetObjectBoundingBox (csBox3& b)
    { scfParent->pFactory->GetObjectBoundingBox (b); }
    virtual void SetObjectBoundingBox (const csBox3& b)
    { scfParent->pFactory->SetObjectBoundingBox (b); }
    virtual void GetRadius (csVector3& r, csVector3& c)
    { scfParent->pFactory->GetRadius (r, c); }
  } scfiObjectModel;
  friend struct eiObjectModel;

  //-------------------- iShadowReceiver interface implementation ----------
  struct ShadowReceiver : public iShadowReceiver
  {
    SCF_DECLARE_EMBEDDED_IBASE (csChunkLodTerrainObject);
    virtual void CastShadows (iMovable* movable, iFrustumView* fview)
    {
      scfParent->CastShadows (movable, fview);
    }
  } scfiShadowReceiver;
  friend struct ShadowReceiver;

  //-------------------- iShadowCaster interface implementation ----------
  struct ShadowCaster : public iShadowCaster
  {
    SCF_DECLARE_EMBEDDED_IBASE (csChunkLodTerrainObject);
    virtual void AppendShadows (iMovable* movable, iShadowBlockList* shadows,
    	const csVector3& origin)
    {
      scfParent->AppendShadows (movable, shadows, origin);
    }
  } scfiShadowCaster;
  friend struct ShadowCaster;

  //------------------------- iLightingInfo interface -------------------------
  struct LightingInfo : public iLightingInfo
  {
    SCF_DECLARE_EMBEDDED_IBASE (csChunkLodTerrainObject);
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
    virtual void SetDynamicAmbientLight (const csColor& color)
    {
      scfParent->SetDynamicAmbientLight (color);
    }
    virtual const csColor& GetDynamicAmbientLight ()
    {
      return scfParent->dynamic_ambient;
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
};

#endif // __CS_CHNKLOD_H__
