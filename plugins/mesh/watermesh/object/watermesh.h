/*
Copyright (C) 2008 by Pavel Krajcevski

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

#ifndef __CS_WATERMESH_H__
#define __CS_WATERMESH_H__

#include "csgeom/vector3.h"
#include "cstool/objmodel.h"
#include "csutil/leakguard.h"
#include "csutil/weakref.h"
#include "cstool/rendermeshholder.h"
#include "iengine/engine.h"
#include "imesh/objmodel.h"
#include "iutil/comp.h"
#include "imesh/watermesh.h"
#include "imesh/object.h"
#include "ivideo/graph3d.h"
#include "csutil/flags.h"
#include "csutil/cscolor.h"

#include "oceancell.h"

struct iObjectRegistry;

class csShaderVariableContext;

namespace CS
{
namespace Plugins
{
namespace WaterMesh
{

class csWaterMeshObjectFactory;

#define WATER_SIZE  16
#define WATER_VERTS (WATER_SIZE * WATER_SIZE)
#define WATER_TRIS (2 * (WATER_SIZE - 1) * (WATER_SIZE - 1)) 

#define NUM_WAVE_FUNCS 3

/**
 * Watermesh version of mesh object.
 */
class csWaterMeshObject : 
  public scfImplementation2<csWaterMeshObject, 
                            iMeshObject,
                            iWaterMeshState>
{
private:
  // The render mesh holder is used by GetRenderMeshes() to supply
  // render meshes that can be returned by that function.
  csFrameDataHolder<csDirtyAccessArray<csRenderMesh*> > meshesHolder;
  csArray<csRenderCell> meshQueue;
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

  //Local buffers and variables
  csRef<iRenderBuffer> vertex_buffer;
  csRef<iRenderBuffer> normal_buffer;

  csDirtyAccessArray<csVector3> verts;
  csDirtyAccessArray<csVector3> norms;

  void SetupVertexBuffer();

  // Admin stuff.
  csWeakRef<iGraphics3D> g3d;
  csWeakRef<iEngine> engine;
  csRef<iStringSet> strings;
  csRef<iShaderVarStringSet> svStrings;

  csRef<csWaterMeshObjectFactory> factory;
  iMeshWrapper* logparent;

  // Normal Map
  csRef<iTextureWrapper> nMap;
  csShaderVariable *nMapVar;

  

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

  void updateLocal();

  // These are the recursive functions used for placing ocean cells
  void DrawFromNode(csOceanNode start, const csVector3 camPos, 
    csPlane3 *planes, uint32 frustum_mask);
  void DrawLeftFromNode(csOceanNode start, const csVector3 camPos, 
    csPlane3 *planes, uint32 frustum_mask);
  void DrawRightFromNode(csOceanNode start, const csVector3 camPos, 
    csPlane3 *planes, uint32 frustum_mask);
  void DrawTopFromNode(csOceanNode start, const csVector3 camPos, 
    csPlane3 *planes, uint32 frustum_mask);
  void DrawBottomFromNode(csOceanNode start, const csVector3 camPos, 
    csPlane3 *planes, uint32 frustum_mask);

  void AddNode(csOceanNode start, float dist);

public:
  /// Constructor.
  csWaterMeshObject (csWaterMeshObjectFactory* factory);

  /// Destructor.
  virtual ~csWaterMeshObject ();

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
  virtual void NextFrame (csTicks, const csVector3&, uint);
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

  virtual void SetNormalMap(iTextureWrapper *map);
  virtual iTextureWrapper* GetNormalMap();

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

  virtual void PositionChild (iMeshObject* /*child*/, csTicks /*current_time*/)
  {
    // We don't support sockets.
  }
  virtual void BuildDecal(const csVector3* pos, float decalRadius,
          iDecalBuilder* decalBuilder)
  {
  }

  bool vertsChanged;  
  void UpdateWater(iCamera *cam);
  /** @} */

  /**\name iRenderBufferAccessor implementation
   * @{ */
  class RenderBufferAccessor : 
    public scfImplementation1<RenderBufferAccessor, 
                              iRenderBufferAccessor>
  {
  private:
    csWeakRef<csWaterMeshObject> parent;

  public:
    RenderBufferAccessor (csWaterMeshObject* parent) : 
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

#include "csutil/deprecated_warn_off.h"

/**
 * Factory for water meshes.
 */
class csWaterMeshObjectFactory : 
  public scfImplementationExt2<csWaterMeshObjectFactory, 
                               csObjectModel,
                              iMeshObjectFactory,
                              iWaterFactoryState>
{
  friend class csWaterMeshObject;

private:
  // The actual data.
  //Near patch and local water
  csDirtyAccessArray<csVector3> verts;
  csDirtyAccessArray<csVector3> norms;
  csDirtyAccessArray<csVector2> texs;
  csDirtyAccessArray<csColor>  cols;
  csDirtyAccessArray<csTriangle> tris;
  
  //Ocean cells
  csArray<csOceanCell> cells;
  
  int numVerts, numTris;

  //size vars
  uint len, wid;
  uint gran;
  uint detail;
  bool size_changed;

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
  bool mesh_cells_dirty_flag;

  // Admin.
  bool initialized;
  csWeakRef<iGraphics3D> g3d;
  csWeakRef<iEngine> engine;

  // Water variables
  float waterAlpha;
  bool murkChanged;

  waterMeshType type;

  //Ocean Attributes
    //Amplitudes
    float amps[NUM_WAVE_FUNCS];
  
    //Frequencies
    float freqs[NUM_WAVE_FUNCS];
    
    //Phases
    float phases[NUM_WAVE_FUNCS];
    
    //Directional vectors
    csVector2 k1, k2, k3;
  

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

  /// Calculate bounding box and radius.
  void CalculateBBoxRadius ();

  /**
   * Setup this factory. This function will check if setup is needed.
   */
  void SetupFactory ();


  csArray<csWaterMeshObject *> children;

public:
  iObjectRegistry* object_reg;
  iMeshFactoryWrapper* logparent;
  csRef<iMeshObjectType> water_type;
  csFlags flags;

  bool changedVerts;  

  bool amplitudes_changed;
  bool frequencies_changed;
  bool phases_changed;
  bool directions_changed;

  bool far_patch_needs_update;

  /// Constructor.
  csWaterMeshObjectFactory (iMeshObjectType *pParent,
    iObjectRegistry* object_reg);

  /// Destructor.
  virtual ~csWaterMeshObjectFactory ();

  /**\name iWaterFactoryState implementation
   * @{ */
  void Invalidate ();

  void SetLength(uint length);
  uint GetLength() { return len; }

  void SetWidth(uint width);
  uint GetWidth() { return wid; }
  
  void SetGranularity(uint granularity);
  uint GetGranularity() { return gran; }
  
  void SetMurkiness(float murk);
  float GetMurkiness();
  
  void SetWaterType(waterMeshType waterType);
  
  void SetAmplitudes(float amp1, float amp2, float amp3);  
  void SetFrequencies(float freq1, float freq2, float freq3);  
  void SetPhases(float phase1, float phase2, float phase3);  
  void SetDirections(csVector2 dir1, csVector2 dir2, csVector2 dir3);
  
  inline bool isOcean() { return type == WATER_TYPE_OCEAN; }

  csRef<iTextureWrapper> MakeFresnelTex(int size);
  
  //Under the hood functions
  csVector3 GetFrequencies() { return csVector3(freqs[0], freqs[1], freqs[2]); }
  csVector3 GetPhases() { return csVector3(phases[0], phases[1], phases[2]); }
  csVector3 GetDirsX() { return csVector3(k1.x, k2.x, k3.x); }
  csVector3 GetDirsY() { return csVector3(k1.y, k2.y, k3.y); }
  csVector3 GetAmplitudes() { return csVector3(amps[0], amps[1], amps[2]); }
  
  /** @} */

  const csBox3& GetObjectBoundingBox ();
  void SetObjectBoundingBox (const csBox3& b);
  void GetRadius (float& radius, csVector3& center);

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
  virtual iMeshObjectType* GetMeshObjectType () const { return water_type; }
  virtual bool SetMaterialWrapper (iMaterialWrapper*) { return false; }
  virtual iMaterialWrapper* GetMaterialWrapper () const { return 0; }
  virtual void SetMixMode (uint) { }
  virtual uint GetMixMode () const { return 0; }
  /** @} */

  virtual iTerraFormer* GetTerraFormerColldet () { return 0; }
  virtual iTerrainSystem* GetTerrainColldet () { return 0; }
  virtual iObjectModel* GetObjectModel () { return this; }

  void PreGetBuffer (csRenderBufferHolder* holder, csRenderBufferName buffer);
  void AddMeshObject (csWaterMeshObject* meshObj);
  void RemoveMeshObject (csWaterMeshObject* meshObj);
};

#include "csutil/deprecated_warn_on.h"

/**
 * Watermesh type. This is the plugin you have to use to create instances
 * of csWaterMeshObjectFactory.
 */
class csWaterMeshObjectType : 
  public scfImplementation2<csWaterMeshObjectType, 
                            iMeshObjectType,
                            iComponent>
{
public:
  iObjectRegistry* object_reg;

  /// Constructor.
  csWaterMeshObjectType (iBase*);
  /// Destructor.
  virtual ~csWaterMeshObjectType ();
  /// Draw.
  virtual csPtr<iMeshObjectFactory> NewFactory ();
  /// Initialize.
  bool Initialize (iObjectRegistry* object_reg);
};

} // namespace WaterMesh
} // namespace Plugins
} // namespace CS

#endif // __CS_WATERMESH_H__
