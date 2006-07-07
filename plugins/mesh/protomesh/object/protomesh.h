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

#ifndef __CS_PROTOMESH_H__
#define __CS_PROTOMESH_H__

#include "csgeom/vector3.h"
#include "cstool/objmodel.h"
#include "csutil/leakguard.h"
#include "csutil/weakref.h"
#include "cstool/rendermeshholder.h"
#include "imesh/objmodel.h"
#include "iutil/comp.h"
#include "imesh/protomesh.h"
#include "imesh/object.h"
#include "ivideo/graph3d.h"
#include "csutil/flags.h"
#include "csutil/cscolor.h"

struct iObjectRegistry;

class csShaderVariableContext;

namespace CS
{
namespace Plugins
{
namespace ProtoMesh
{


class csProtoMeshObjectFactory;

#define PROTO_TRIS 12
#define PROTO_VERTS 8

/**
 * Protomesh version of mesh object.
 */
class csProtoMeshObject : 
  public scfImplementation2<csProtoMeshObject, 
                            iMeshObject,
                            iProtoMeshState>
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
  csRef<csProtoMeshObjectFactory> factory;
  iMeshWrapper* logparent;

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

  float fuzz;

  // This flag is set to false initially and will be set to true
  // by SetupObject() after object is initialized. Some functions
  // can set this to false again to force reinit.
  bool initialized;

  /**
   * Setup this object. This function will check if setup is needed.
   */
  void SetupObject ();

public:
  /// Constructor.
  csProtoMeshObject (csProtoMeshObjectFactory* factory);

  /// Destructor.
  virtual ~csProtoMeshObject ();

  virtual void SetFuzzFactor (float factor) { fuzz = factor; }
  virtual float GetFuzzFactor () { return fuzz; }

  /**\name iMeshObject implementation
   * @{ */
  void SetMixMode (uint mode) { MixMode = mode; }
  uint GetMixMode () const { return MixMode; }

  virtual iMeshObjectFactory* GetFactory () const;
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
  virtual void NextFrame (csTicks, const csVector3&, uint)
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
  virtual void PositionChild (iMeshObject* /*child*/, csTicks /*current_time*/)
  {
    // We don't support sockets.
  }
  /** @} */

  /**\name iRenderBufferAccessor implementation
   * @{ */
  class RenderBufferAccessor : 
    public scfImplementation1<RenderBufferAccessor, 
                              iRenderBufferAccessor>
  {
  private:
    csWeakRef<csProtoMeshObject> parent;

  public:
    RenderBufferAccessor (csProtoMeshObject* parent) : 
      scfImplementationType (0)
    {
      RenderBufferAccessor::parent = parent;
    }
    virtual ~RenderBufferAccessor () { }
    virtual void PreGetBuffer (csRenderBufferHolder* holder,
    	csRenderBufferName buffer)
    {
      if (parent) parent->PreGetBuffer (holder, buffer);
    }
  };
  csRef<RenderBufferAccessor> myRenderBufferAccessor;
  friend class RenderBufferAccessor;

  void PreGetBuffer (csRenderBufferHolder* holder, csRenderBufferName buffer);
  /** @} */
};

/**
 * Factory for proto meshes.
 */
class csProtoMeshObjectFactory : 
  public scfImplementationExt2<csProtoMeshObjectFactory, 
                               csObjectModel,
                              iMeshObjectFactory,
                              iProtoFactoryState>
{
  friend class csProtoMeshObject;

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
  float radius;
  csBox3 object_bbox;
  bool object_bbox_valid;

  // For polygon mesh.
  csMeshedPolygon* polygons;

  /// Calculate bounding box and radius.
  void CalculateBBoxRadius ();

  /**
   * Setup this factory. This function will check if setup is needed.
   */
  void SetupFactory ();

public:
  iObjectRegistry* object_reg;
  iMeshFactoryWrapper* logparent;
  csRef<iMeshObjectType> proto_type;
  csFlags flags;

  /// Constructor.
  csProtoMeshObjectFactory (iMeshObjectType *pParent,
  	iObjectRegistry* object_reg);

  /// Destructor.
  virtual ~csProtoMeshObjectFactory ();

  /**\name iProtoFactoryState implementation
   * @{ */
  csVector3* GetVertices () { return vertices; }
  csVector2* GetTexels () { return texels; }
  csVector3* GetNormals () { return normals; }
  csColor* GetColors () { return colors; }
  csTriangle* GetTriangles () { return triangles; }
  void Invalidate ();
  /** @} */

  void GetObjectBoundingBox (csBox3& bbox);
  void SetObjectBoundingBox (const csBox3& b);
  void GetRadius (float& radius, csVector3& center);

  /**
   * Calculate polygons for iPolygonMesh.
   */
  csMeshedPolygon* GetPolygons ();

  /**\name iMeshObjectFactory implementation
   * @{ */
  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> NewInstance ();
  virtual csPtr<iMeshObjectFactory> Clone () { return 0; }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual void SetMeshFactoryWrapper (iMeshFactoryWrapper* lp)
  {
    logparent = lp;
    CS_ASSERT (logparent != 0);
  }
  virtual iMeshFactoryWrapper* GetMeshFactoryWrapper () const
  { return logparent; }
  virtual iMeshObjectType* GetMeshObjectType () const { return proto_type; }
  virtual bool SetMaterialWrapper (iMaterialWrapper*) { return false; }
  virtual iMaterialWrapper* GetMaterialWrapper () const { return 0; }
  virtual void SetMixMode (uint) { }
  virtual uint GetMixMode () const { return 0; }
  /** @} */

  //------------------ iPolygonMesh interface implementation ----------------//
  struct PolyMesh : public scfImplementation1<PolyMesh, 
                                              iPolygonMesh>
  {
  private:
    csWeakRef<csProtoMeshObjectFactory> factory;
    csFlags flags;

  public:
    virtual int GetVertexCount () { return PROTO_VERTS; }
    virtual csVector3* GetVertices () 
    { return factory ? factory->GetVertices () : 0; }
    virtual int GetPolygonCount () { return PROTO_TRIS; }
    virtual csMeshedPolygon* GetPolygons ();
    virtual int GetTriangleCount () { return PROTO_TRIS; }
    virtual csTriangle* GetTriangles () 
    { return factory ? factory->GetTriangles () : 0; }
    virtual void Lock () { }
    virtual void Unlock () { }
    
    virtual csFlags& GetFlags () { return flags;  }
    virtual uint32 GetChangeNumber() const { return 0; }

    PolyMesh (csProtoMeshObjectFactory* Factory) : 
      scfImplementationType (this), factory (Factory)
    {
      flags.Set (CS_POLYMESH_TRIANGLEMESH);
    }
    virtual ~PolyMesh () { }
  };
  friend struct PolyMesh;

  virtual iTerraFormer* GetTerraFormerColldet () { return 0; }
  virtual iTerrainSystem* GetTerrainColldet () { return 0; }
  virtual iObjectModel* GetObjectModel () { return this; }

  void PreGetBuffer (csRenderBufferHolder* holder, csRenderBufferName buffer);
};

/**
 * Protomesh type. This is the plugin you have to use to create instances
 * of csProtoMeshObjectFactory.
 */
class csProtoMeshObjectType : 
  public scfImplementation2<csProtoMeshObjectType, 
                            iMeshObjectType,
                            iComponent>
{
public:
  iObjectRegistry* object_reg;

  /// Constructor.
  csProtoMeshObjectType (iBase*);
  /// Destructor.
  virtual ~csProtoMeshObjectType ();
  /// Draw.
  virtual csPtr<iMeshObjectFactory> NewFactory ();
  /// Initialize.
  bool Initialize (iObjectRegistry* object_reg);
};

} // namespace ProtoMesh
} // namespace Plugins
} // namespace CS

#endif // __CS_PROTOMESH_H__
