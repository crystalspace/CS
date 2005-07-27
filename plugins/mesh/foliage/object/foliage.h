/*
    Copyright (C) 2004 by Jorrit Tyberghein

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

#ifndef __CS_FOLIAGEMESH_H__
#define __CS_FOLIAGEMESH_H__

#include "csgeom/objmodel.h"
#include "csgeom/vector3.h"
#include "cstool/rendermeshholder.h"
#include "csutil/leakguard.h"
#include "csutil/weakref.h"
#include "iengine/lod.h"
#include "iengine/sharevar.h"
#include "igeom/objmodel.h"
#include "imesh/foliage.h"
#include "imesh/object.h"
#include "iutil/comp.h"
#include "ivaria/terraform.h"
#include "ivideo/graph3d.h"
#include "csutil/flags.h"

class csFoliageMeshObjectFactory;

class csShaderVariableContext;

struct iObjectRegistry;


#define PROTO_TRIS 12
#define PROTO_VERTS 8

class csFoliageGeometry : public iFoliageGeometry
{
private:
  csRef<iMaterialWrapper> material;
  csDirtyAccessArray<csFoliageVertex> vertices;
  csDirtyAccessArray<csTriangle> triangles;

public:
  csFoliageGeometry ();
  virtual ~csFoliageGeometry ();

  //-------------------- iFoliageGeometry implementation -------------------

  SCF_DECLARE_IBASE;

  virtual size_t AddVertex (const csVector3& pos, const csVector2& texel,
      	const csColor& color, const csVector3& normal);
  virtual const csDirtyAccessArray<csFoliageVertex>& GetVertices () const
  {
    return vertices;
  }
  virtual size_t AddTriangle (const csTriangle& tri);
  virtual const csDirtyAccessArray<csTriangle>& GetTriangles () const
  {
    return triangles;
  }
  virtual void SetMaterialWrapper (iMaterialWrapper* material);
  virtual iMaterialWrapper* GetMaterialWrapper () const
  {
    return material;
  }
};

// Listen to LOD variable changes.
// @@@ This class is also used in meshlod.cpp. Put somewhere
// global?
class csFoliageLODListener : public iSharedVariableListener
{
private:
  float* variable;
public:
  SCF_DECLARE_IBASE;
  csFoliageLODListener (float* variable)
  {
    SCF_CONSTRUCT_IBASE (0);
    csFoliageLODListener::variable = variable;
  }

  virtual ~csFoliageLODListener() { SCF_DESTRUCT_IBASE(); }

  virtual void VariableChanged (iSharedVariable* var)
  {
    *variable = var->Get ();
  }
};

class csFoliageObject : public iFoliageObject, public iLODControl
{
private:
  char* name;
  csRefArray<iFoliageGeometry> geometry;

  /// Function for lod.
  float lod_m, lod_a;
  /// Or using variables.
  csRef<iSharedVariable> lod_varm;
  csRef<iSharedVariable> lod_vara;
  csRef<csFoliageLODListener> lod_varm_listener;
  csRef<csFoliageLODListener> lod_vara_listener;
  void ClearLODListeners ();

public:
  csFoliageObject (const char* name);
  virtual ~csFoliageObject ();

  //-------------------- iFoliageObject implementation -------------------

  SCF_DECLARE_IBASE;

  virtual const char* GetName () const { return name; }
  virtual csPtr<iFoliageGeometry> CreateGeometry (size_t lodslot);
  virtual csPtr<iFoliageGeometry> CreateGeometryLOD (size_t fromslot,
      size_t toslot, float factory);
  virtual iFoliageGeometry* GetGeometry (size_t lodslot);
  virtual size_t GetMaxLodSlot () const;
  virtual iLODControl* GetLODControl () { return (iLODControl*)this; }

  //-------------------- iLODControl implementation -------------------

  virtual void SetLOD (float m, float a);
  virtual void GetLOD (float& m, float& a) const;
  virtual void SetLOD (iSharedVariable* varm, iSharedVariable* vara);
  virtual void GetLOD (iSharedVariable*& varm, iSharedVariable*& vara)
  	const
  {
    varm = lod_varm;
    vara = lod_vara;
  }
  virtual int GetLODPolygonCount (float lod) const;
};


/**
 * A piece of geometry with an associated transform.
 */
struct csFoliageGeometryInstance
{
  csFoliageGeometry* geometry;
  csReversibleTransform transform;
};

/**
 * An array of geometry instances all using the same material.
 */
struct csGeomInstances
{
  csArray<csFoliageGeometryInstance> instances;
  iMaterialWrapper* material;
  csRef<iRenderBuffer> index_buffer;
  size_t total_triangles;
  size_t total_vertices;
};

/**
 * Foliage mesh block. This represents a subset of the foliage
 * geometry depending on where the camera is.
 */
class csFoliageMeshBlock
{
private:
  // 3D bounding box for all geometry in this block.
  csBox3 bbox;

  // Interleaved render buffer for all foliage geometry in this
  // block.
  csRef<iRenderBuffer> interleaved_buffer;
  csRef<iRenderBuffer> vertex_buffer;
  csRef<iRenderBuffer> texel_buffer;
  csRef<iRenderBuffer> color_buffer;
  csRef<iRenderBuffer> normal_buffer;

  // One set of geometry instances for every material.
  csArray<csGeomInstances> geom_instances;

  // If true the we need to setup the render meshes and buffers of this block.
  bool setup;

  // Create all the buffers.
  void CreateBuffers ();

public:
  /**
   * Construct a new mesh block.
   */
  csFoliageMeshBlock ();

  /**
   * Add a geometry instance.
   */
  void AddGeometryInstance (csFoliageGeometry* geom,
  	const csReversibleTransform& trans);

  /**
   * Draw this block (add to the render mesh holder).
   */
  void Draw (iGraphics3D* g3d, iRenderView* rview,
      uint32 frustum_mask, const csReversibleTransform& transform,
      csRenderMeshHolder& rmHolder);
};

/**
 * Foliage version of mesh object.
 */
class csFoliageMeshObject : public iMeshObject
{
private:
  // The render mesh holder is used by GetRenderMeshes() to supply
  // render meshes that can be returned by that function.
  csRenderMeshHolder rmHolder;

  // The standard render buffer holder. It takes care of giving
  // the renderer all required renderbuffers.
  csRef<csRenderBufferHolder> bufferHolder;

  // The shader variable context. Holds shader variables like the
  // object to world transform.
  csRef<csShaderVariableContext> variableContext;

  // Since every mesh can have a different color we need to have
  // the color buffer here. But we will use the basic colors
  // from the factory.
  csRef<iRenderBuffer> color_buffer;

  // Setup the bufferholder to get the buffers and accessors
  // for all types of data we need.
  void SetupBufferHolder ();

  // Admin stuff.
  csWeakRef<iGraphics3D> g3d;
  csFoliageMeshObjectFactory* factory;
  iBase* logparent;

  // Callback when object is rendered (in GetRenderMeshes()).
  csRef<iMeshObjectDrawCallback> vis_cb;

  // Material and mixmode for rendering.
  csRef<iMaterialWrapper> material;
  uint MixMode;

  // Base color. The mesh_colors_dirty_flag is set to true if
  // the color changes.
  csColor color;
  bool mesh_colors_dirty_flag;
  // factory_color_nr is used to compare with the last color number
  // in the factory. That way we can update the color buffer (set
  // mesh_colors_dirty_flag to true) as soon as the colors in the
  // factory change.
  uint factory_color_nr;

  // Currently unused.
  float current_lod;
  uint32 current_features;
  csFlags flags;

  // This flag is set to false initially and will be set to true
  // by SetupObject() after object is initialized. Some functions
  // can set this to false again to force reinit.
  bool initialized;

  // 

  /**
   * Setup this object. This function will check if setup is needed.
   */
  void SetupObject ();

public:
  /// Constructor.
  csFoliageMeshObject (csFoliageMeshObjectFactory* factory);

  /// Destructor.
  virtual ~csFoliageMeshObject ();

  void SetMixMode (uint mode) { MixMode = mode; }
  uint GetMixMode () const { return MixMode; }
  const csColor& GetColor () const { return color; }
  void GetObjectBoundingBox (csBox3& bbox);
  void SetObjectBoundingBox (const csBox3& bbox);

  //----------------------- iMeshObject implementation ----------------------
  SCF_DECLARE_IBASE;

  virtual iMeshObjectFactory* GetFactory () const
  {
    return (iMeshObjectFactory*)factory;
  }
  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> Clone ()
  {
    // We don't support making clones.
    return 0;
  }
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
  virtual void NextFrame (csTicks, const csVector3&)
  {
    // We don't support animation.
  }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const
  {
    // We don't support hard transform.
    return false;
  }
  virtual bool HitBeamOutline (const csVector3& start, const csVector3& end,
    csVector3& isect, float *pr);
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr, int* polygon_idx = 0);
  virtual void SetLogicalParent (iBase* lp)
  {
    logparent = lp;
    CS_ASSERT (logparent != 0);
  }
  virtual iBase* GetLogicalParent () const { return logparent; }

  virtual iObjectModel* GetObjectModel ();
  virtual bool SetColor (const csColor& col)
  {
    mesh_colors_dirty_flag = true;
    color = col;
    return true;
  }
  virtual bool GetColor (csColor& col) const { col = color; return true; }
  virtual bool SetMaterialWrapper (iMaterialWrapper* mat);
  virtual iMaterialWrapper* GetMaterialWrapper () const { return material; }
  virtual void InvalidateMaterialHandles ()
  {
    // We visit our material all the time so this is not needed here.
  }
  virtual void PositionChild (iMeshObject* child,csTicks current_time)
  {
    // We don't support sockets.
  }

  //------------------------- iFoliageMeshState implementation ----------------
  class FoliageMeshState : public iFoliageMeshState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csFoliageMeshObject);
  } scfiFoliageMeshState;

  //------------------ iRenderBufferAccessor implementation ------------
  class RenderBufferAccessor : public iRenderBufferAccessor
  {
  private:
    csFoliageMeshObject* parent;

  public:
    SCF_DECLARE_IBASE;
    RenderBufferAccessor (csFoliageMeshObject* parent)
    {
      SCF_CONSTRUCT_IBASE (0);
      RenderBufferAccessor::parent = parent;
    }
    virtual ~RenderBufferAccessor ()
    {
      SCF_DESTRUCT_IBASE ();
    }
    virtual void PreGetBuffer (csRenderBufferHolder* holder, csRenderBufferName buffer)
    {
      parent->PreGetBuffer (holder, buffer);
    }
  } *scfiRenderBufferAccessor;
  friend class RenderBufferAccessor;

  void PreGetBuffer (csRenderBufferHolder* holder, csRenderBufferName buffer);
};

/// Palette type.
struct csFoliagePalette
{
  // @@@ TODO
};

/// A piece of generated foliage on the map.
struct csGeneratedFoliage
{
  int type;		// Index into 'foliage_objects'.
  float yrot;		// Rotation along the y-axis.
  csVector3 pos;	// Position on the terrain.
};

/**
 * Factory for foliage meshes.
 */
class csFoliageMeshObjectFactory : public iMeshObjectFactory
{
  friend class csFoliageMeshObject;

private:
  // The actual data.
  csVector3 vertices[PROTO_VERTS];
  csVector3 normals[PROTO_VERTS];
  csVector2 texels[PROTO_VERTS];
  csColor colors[PROTO_VERTS];
  csTriangle triangles[PROTO_TRIS];

  // If the colors change we increase this number
  // so that the meshes know that they have to update
  // color buffers.
  uint color_nr;

  // If one of the flags below is dirty then the
  // buffer must be created or filled.
  bool mesh_vertices_dirty_flag;
  bool mesh_texels_dirty_flag;
  bool mesh_normals_dirty_flag;
  bool mesh_triangle_dirty_flag;

  // Admin.
  bool initialized;
  csWeakRef<iGraphics3D> g3d;

  // Buffers for the renderers.
  csRef<iRenderBuffer> vertex_buffer;
  csRef<iRenderBuffer> texel_buffer;
  csRef<iRenderBuffer> index_buffer;
  csRef<iRenderBuffer> normal_buffer;

  // Prepare the buffers (check if they are dirty).
  // Mesh objects will call this before rendering.
  void PrepareBuffers ();

  // Bounding box/sphere.
  csVector3 radius;
  csBox3 object_bbox;
  bool object_bbox_valid;
  /// Calculate bounding box and radius.
  void CalculateBBoxRadius ();

  // For polygon mesh.
  csMeshedPolygon* polygons;

  // Foliage data.
  csRefArray<iFoliageObject> foliage_objects;
  csRef<iTerraFormer> terraformer;
  csBox2 samplerRegion;
  csArray<csFoliagePalette> foliage_types;

  // Generated foliage.
  int genfoliage_res;	// Resolution at which we generate foliage.
  csArray<csGeneratedFoliage>* genfoliage;
  void ClearGeneratedFoliage ();
  void GenerateFoliage ();

  /**
   * Setup this factory. This function will check if setup is needed.
   */
  void SetupFactory ();

public:
  static csStringID heights_name, foliage_density_name, foliage_types_name;

  iObjectRegistry* object_reg;
  iBase* logparent;
  iMeshObjectType* foliage_type;
  csFlags flags;

  /// Constructor.
  csFoliageMeshObjectFactory (iMeshObjectType *pParent,
  	iObjectRegistry* object_reg);

  /// Destructor.
  virtual ~csFoliageMeshObjectFactory ();

  csVector3* GetVertices () { return vertices; }
  csVector2* GetTexels () { return texels; }
  csVector3* GetNormals () { return normals; }
  csColor* GetColors () { return colors; }
  csTriangle* GetTriangles () { return triangles; }
  void Invalidate ();

  const csBox3& GetObjectBoundingBox ();
  void SetObjectBoundingBox (const csBox3& bbox);
  const csVector3& GetRadius ();

  /**
   * Calculate polygons for iPolygonMesh.
   */
  csMeshedPolygon* GetPolygons ();

  csPtr<iFoliageObject> CreateObject (const char* name);
  iFoliageObject* FindObject (const char* name) const;
  const csRefArray<iFoliageObject>& GetObjects () const
  {
    return foliage_objects;
  }
  void SetTerraFormer (iTerraFormer *form) { terraformer = form; }
  iTerraFormer *GetTerraFormer () { return terraformer; }
  void SetSamplerRegion (const csBox2& region);
  const csBox2& GetSamplerRegion () const
  {
    return samplerRegion;
  }
  void AddPaletteEntry (size_t typeidx, const char* objectname,
      float relative_density) { }
  void ClearPaletteType (size_t typeidx) { }
  size_t GetPaletteTypeCount () const { return foliage_types.Length (); }
  size_t GetPaletteEntryCount (size_t typeidx) const { return 0; }
  const char* GetPaletteEntry (size_t typeidx, size_t entryidx,
      float& relative_density) { return 0; }

  //------------------------ iMeshObjectFactory implementation --------------
  SCF_DECLARE_IBASE;

  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> NewInstance ();
  virtual csPtr<iMeshObjectFactory> Clone () { return 0; }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual void SetLogicalParent (iBase* lp)
  {
    logparent = lp;
    CS_ASSERT (logparent != 0);
  }
  virtual iBase* GetLogicalParent () const { return logparent; }
  virtual iMeshObjectType* GetMeshObjectType () const { return foliage_type; }

  //----------------------- iFoliageFactoryState implementation -------------
  class FoliageFactoryState : public iFoliageFactoryState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csFoliageMeshObjectFactory);
    virtual csPtr<iFoliageObject> CreateObject (const char* name)
    {
      return scfParent->CreateObject (name);
    }
    virtual iFoliageObject* FindObject (const char* name) const
    {
      return scfParent->FindObject (name);
    }
    virtual const csRefArray<iFoliageObject>& GetObjects () const
    {
      return scfParent->GetObjects ();
    }
    virtual void SetTerraFormer (iTerraFormer* form)
    {
      scfParent->SetTerraFormer (form);
    }
    virtual iTerraFormer* GetTerraFormer ()
    {
      return scfParent->GetTerraFormer ();
    }

    virtual void AddPaletteEntry (size_t typeidx, const char* objectname,
      float relative_density)
    {
      scfParent->AddPaletteEntry (typeidx, objectname, relative_density);
    }
    virtual void ClearPaletteType (size_t typeidx)
    {
      scfParent->ClearPaletteType (typeidx);
    }
    virtual size_t GetPaletteTypeCount () const
    {
      return scfParent->GetPaletteTypeCount ();
    }
    virtual size_t GetPaletteEntryCount (size_t typeidx) const
    {
      return scfParent->GetPaletteEntryCount (typeidx);
    }
    virtual const char* GetPaletteEntry (size_t typeidx, size_t entryidx,
      float& relative_density)
    {
      return scfParent->GetPaletteEntry (typeidx, entryidx, relative_density);
    }

    virtual void SetSamplerRegion (const csBox2& region)
    {
      scfParent->SetSamplerRegion (region);
    }
    virtual const csBox2& GetSamplerRegion () const
    {
      return scfParent->GetSamplerRegion ();
    }
  } scfiFoliageFactoryState;

  //------------------ iPolygonMesh interface implementation ----------------//
  struct PolyMesh : public iPolygonMesh
  {
  private:
    csFoliageMeshObjectFactory* factory;
    csFlags flags;

  public:
    SCF_DECLARE_IBASE;

    void SetFactory (csFoliageMeshObjectFactory* Factory)
    {
      factory = Factory;
    }

    virtual int GetVertexCount () { return PROTO_VERTS; }
    virtual csVector3* GetVertices () { return factory->GetVertices (); }
    virtual int GetPolygonCount () { return PROTO_TRIS; }
    virtual csMeshedPolygon* GetPolygons ();
    virtual int GetTriangleCount () { return PROTO_TRIS; }
    virtual csTriangle* GetTriangles () { return factory->GetTriangles (); }
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
    SCF_DECLARE_EMBEDDED_IBASE (csFoliageMeshObjectFactory);
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

 
  void PreGetBuffer (csRenderBufferHolder* holder, csRenderBufferName buffer);
};

/**
 * Foliage type. This is the plugin you have to use to create instances
 * of csFoliageMeshObjectFactory.
 */
class csFoliageMeshObjectType : public iMeshObjectType
{
public:
  iObjectRegistry* object_reg;

  SCF_DECLARE_IBASE;

  /// Constructor.
  csFoliageMeshObjectType (iBase*);
  /// Destructor.
  virtual ~csFoliageMeshObjectType ();
  /// Draw.
  virtual csPtr<iMeshObjectFactory> NewFactory ();
  /// Initialize.
  bool Initialize (iObjectRegistry* object_reg);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csFoliageMeshObjectType);
    virtual bool Initialize (iObjectRegistry* object_reg)
    {
      return scfParent->Initialize (object_reg);
    }
  } scfiComponent;
};

#endif // __CS_FOLIAGEMESH_H__
