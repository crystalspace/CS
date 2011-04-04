/*
  Copyright (C) 2005 by Marten Svanfeldt

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

#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "csutil/hash.h"

#include "primitive.h"
#include "light.h"
#include "lightmap.h"
#include "lightmapuv.h"
#include "vertexdata.h"
#include "csgeom/transfrm.h"

namespace lighter
{
  csPtr<iRenderBuffer> WrapBuffer (iRenderBuffer* buffer, 
                                   const char* suffix,
                                   const char* basename);

  class LightmapUVFactoryLayouter;
  class LightmapUVObjectLayouter;
  class Object;
  class Scene;
  class Light;

  enum ObjectFlags
  {
    // Don't compute lighting
    OBJECT_FLAG_NOLIGHT = 1,
    // Don't cast shadows
    OBJECT_FLAG_NOSHADOW = 2,
    // Include in occlusion debugging
    OBJECT_FLAG_RAYDEBUG = 4,
    // Tangent space is available
    OBJECT_FLAG_TANGENTS = 8,
    // Don't cast shadows on itself
    OBJECT_FLAG_NOSELFSHADOW = 16
  };

  /**
   * Baseclass for Object factories.
   * Needs to be subclassed to provide an interface to a specific mesh
   * object type to be able to import/export geometry data.
   */
  class ObjectFactory : public csRefCount
  {
  public:
    ObjectFactory (const Configuration& config);

    virtual bool PrepareLightmapUV (LightmapUVFactoryLayouter* uvlayout);

    // Get a new object
    virtual csPtr<Object> CreateObject ();

    // Parse data
    virtual void ParseFactory (iMeshFactoryWrapper *factory);
  
    // Write out the data again
    virtual void SaveFactory (iDocumentNode *node);

    // Getters
    inline float GetLMDensity () const
    { return lmScale; }
    const ObjectFactoryVertexData& GetVertexData() const
    { return vertexData; }

    // Name of the factory
    csString factoryName;
  
    //@{
    /// Indices of tangent/bitangent in vertex data
    size_t vdataTangents;
    size_t vdataBitangents;
    //@}

    /// Whether to light meshes of this factory per vertex
    bool lightPerVertex : 1;
    
    /// Whether to avoid modifying this factory
    bool noModify : 1;

    /// Whether tangent space data is available
    bool hasTangents : 1;

    /**
     * Whether meshes of this factory should not cast shadows
     * on themselves
     */
    bool noSelfShadow : 1;
  protected:

    // Begin remapping of submeshes
    virtual void BeginSubmeshRemap ()
    {}

    // Add a new mapping between old index and new index
    virtual void AddSubmeshRemap (size_t oldIndex, size_t newIndex)
    {}

    // Finish remapping of submeshes
    virtual void FinishSubmeshRemap ()
    {}

    // All faces, untransformed
    csArray<FactoryPrimitiveArray> unlayoutedPrimitives;
    // A group of primitives that fit on a lightmap together.
    struct LayoutedPrimitives
    {
      FactoryPrimitiveArray primitives;
      csRef<LightmapUVObjectLayouter> factory;
      size_t group;

      LayoutedPrimitives (const FactoryPrimitiveArray& primitives, 
                          LightmapUVObjectLayouter* factory, size_t group) :
        primitives (primitives), factory (factory), group (group) {}
    };
    csArray<LayoutedPrimitives> layoutedPrimitives;

    // Vertex data for above faces
    ObjectFactoryVertexData vertexData;

    // Lightmap settings
    float lmScale;

    // Factory created from
    csRef<iMeshFactoryWrapper> factoryWrapper;

    // String identifying the saver plugin. Should be set from derived
    // classes
    const char* saverPluginName;

    // Helper function: get a filename prefix for this mesh
    csString GetFileName() const;
    
    /* Wrap a render buffer in a RenderBufferPersistent if binary buffers
       are enabled */
    csPtr<iRenderBuffer> WrapBuffer (iRenderBuffer* buffer, const char* suffix);

    friend class Object;
  };
  typedef csRefArray<ObjectFactory> ObjectFactoryRefArray;
  typedef csHash<csRef<ObjectFactory>, csString> ObjectFactoryHash;

  /**
  * A single object in the computational scene.
  * Each object exists in a single spot in a single sector, but there
  * might be more instances created from a single factory in different places.
  */
  class Object : public csRefCount
  {
    Object (const Object&); // Illegal
  public:
    // Construct a new Object from a ObjectFactory and transform
    Object (ObjectFactory* object);
    ~Object ();

    // Initialize the Object from factory and wrapper. Call only after
    // constructor and ParseMesh have been called
    virtual bool Initialize (Sector* sector);

    // Remove lightmap SVs. Add names of used lightmap textures to set
    virtual void StripLightmaps (csSet<csString>& lms);

    // Parse data
    virtual void ParseMesh (iMeshWrapper *wrapper);

    // Write out the data again
    virtual void SaveMesh (iDocumentNode *node);

    /* Conserve memory: free all object data that won't be needed for the 
     * actual lighting. */
    virtual void FreeNotNeededForLighting ();

    // Immediate preparations before object is being lit.
    virtual void PrepareLighting ();

    // Write out data that must be written after lighting.
    virtual void SaveMeshPostLighting (Scene* scene) { }

    // Fill lightmap mask with primitive sub-pixel area coverage
    virtual void FillLightmapMask (LightmapMaskPtrDelArray& masks);

    //-- Getters for data
    inline const csArray<PrimitiveArray>& GetPrimitives () const
    { return allPrimitives; }

    inline csArray<PrimitiveArray>& GetPrimitives ()
    { return allPrimitives; }
    
    inline const ObjectVertexData& GetVertexData () const
    { return vertexData; }

    inline ObjectVertexData& GetVertexData ()
    { return vertexData; }
    
    inline iMeshWrapper* GetMeshWrapper () { return meshWrapper; }

    inline const csSphere& GetBoundingSphere () const
    { return bsphere; }

    typedef csDirtyAccessArray<csColor> LitColorArray;
    inline LitColorArray* GetLitColors (size_t num)
    { return litColors + num; }

    typedef csHash<LitColorArray, csPtrKey<Light> > LitColorsPDHash;
    /// Return lit colors for all PD lights
    inline LitColorsPDHash* GetLitColorsPD (size_t num)
    { return litColorsPD + num; }
    /// Return lit colors for one PD light
    LitColorArray* GetLitColorsPD (Light* light, size_t num);

    inline const csFlags& GetFlags () const
    { return objFlags; }

    inline Sector* GetSector() const { return sector; }

    csMatrix3 ComputeTangentSpace (const Primitive* prim,
      const csVector3& pt) const;

    csMatrix3 GetTangentSpace (size_t vert) const;
    
    const csReversibleTransform& GetObjectToWorld () const
    { return objectToWorld; }

    // Name
    csString meshName;

    csString materialName;

    /// Whether to light mesh per vertex
    bool lightPerVertex;
    
    /// Get the light influences for a certain primitive group
    LightInfluences& GetLightInfluences (uint groupID, Light* light);
    const LightInfluences& GetLightInfluences (uint groupID, Light* light) const;
    
    size_t GetPrimitiveGroupNum() const { return allPrimitives.GetSize(); }
    csArray<Light*> GetLightsAffectingGroup (uint groupID) const;
    uint GetPrimitiveGroupLightmap (uint groupID) const
    { return allPrimitives[groupID].Get (0).GetGlobalLightmapID (); }
  protected:
    // Lightmap texture names (used for stripping)
    static const char* const lightmapTextures[];
    
    // All faces, already transformed
    csArray<PrimitiveArray> allPrimitives;
    csArray<uint> lightmapIDs;
    struct LMLayoutingInfo
    {
      csRef<LightmapUVObjectLayouter> layouter;
      size_t layoutID;
      size_t group;

      LMLayoutingInfo (LightmapUVObjectLayouter* layouter, size_t layoutID,
        size_t group) :
        layouter (layouter), layoutID (layoutID), group (group) {}
    };
    csArray<LMLayoutingInfo> lmLayouts;

    // Sector the object belongs in
    Sector* sector;

    // Bounding sphere
    csSphere bsphere;
    
    // Object to world transform
    csReversibleTransform objectToWorld;

    // Vertex data for above, transformed
    ObjectVertexData vertexData;
    //@{
    /// Lit colors (if object is lit per-vertex)
    LitColorArray* litColors;
    LitColorsPDHash* litColorsPD;
    //@}

    // Factory we where created from
    csRef<ObjectFactory> factory;

    // Reference to the mesh
    csRef<iMeshWrapper> meshWrapper;

    // String identifying the saver plugin. Should be set from derived
    // classes
    const char* saverPluginName;

    // Internal flags
    csFlags objFlags;

    //@{
    /// Indices of tangent/bitangent in vertex data
    size_t vdataTangents;
    size_t vdataBitangents;
    //@}

    // Renormalize lightmap UVs into buffer \a lmcoords.
    virtual void RenormalizeLightmapUVs (const LightmapPtrDelArray& lightmaps,
      csVector2* lmcoords);

    // Compute bounding sphere from vertex data
    void ComputeBoundingSphere ();

    // Helper function: get a filename prefix for this mesh
    csString GetFileName() const;
    
    /* Wrap a render buffer in a RenderBufferPersistent if binary buffers
       are enabled */
    csPtr<iRenderBuffer> WrapBuffer (iRenderBuffer* buffer, const char* suffix);
    
    struct GroupAndLight
    {
      Light* light;
      uint groupID;
      
      GroupAndLight (Light* light, uint groupID) : light (light),
        groupID (groupID) {}
        
      bool operator< (const GroupAndLight& other) const
      {
        if (light < other.light) return true;
        if (light > other.light) return false;
        return groupID < other.groupID;
      }
      uint GetHash() const
      {
        return uint (uintptr_t (light)) ^ groupID;
      }
    };
    struct LightInfluencesRC :
      public CS::Utility::FastRefCount<LightInfluencesRC>,
      public LightInfluences
    {
      LightInfluencesRC (uint w, uint h, uint xOffs, uint yOffs)
       : LightInfluences (w, h, xOffs, yOffs) {}
    };
    typedef csHash<csRef<LightInfluencesRC>, GroupAndLight> LightInfluencesHash;
    LightInfluencesHash* lightInfluences;
    CS::Threading::Mutex lightInfluencesMutex;

    friend class ObjectFactory;
  };
  typedef csRefArray<Object> ObjectRefArray;
  typedef csHash<csRef<Object>, csString> ObjectHash;
}

#endif // __OBJECT_H__
