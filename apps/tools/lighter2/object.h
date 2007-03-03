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
#include "csgeom/transfrm.h"

namespace lighter
{
  class LightmapUVFactoryLayouter;
  class LightmapUVObjectLayouter;
  class Object;
  class Scene;
  class Light;

  enum ObjectFlags
  {
    OBJECT_FLAG_NOLIGHT = 1,
    OBJECT_FLAG_NOSHADOW = 2
  };


  /**
   * Hold per object vertex data
   */
  struct ObjectVertexData 
  {
    struct Vertex
    {
      //Position
      csVector3 position;

      //Normal
      csVector3 normal;

      //Texture UV
      csVector2 textureUV;

      //Lightmap UV (not normalized.. pixel-coordinates)
      csVector2 lightmapUV;

      //Extra data
      csRef<csRefCount> extraData;
    };

    typedef csDirtyAccessArray<Vertex> VertexDelArray;
    VertexDelArray vertexArray;

    //Helper functions

    /// Split one vertex, duplicate it and return index for new one
    size_t SplitVertex (size_t oldIndex)
    {
      const Vertex& oldVertex = vertexArray[oldIndex];
      return vertexArray.Push (oldVertex);
    }

    /// Interpolate between two vertices with t
    Vertex InterpolateVertex (size_t i0, size_t i1, float t)
    {
      Vertex newVertex;

      const Vertex& v0 = vertexArray[i0];
      const Vertex& v1 = vertexArray[i1];

      newVertex.position = v0.position - (v1.position - v0.position) * t;
      newVertex.normal = v0.normal - (v1.normal - v0.normal) * t;
      newVertex.normal.Normalize ();

      newVertex.textureUV = v0.textureUV - (v1.textureUV - v0.textureUV) * t;
      newVertex.lightmapUV = v0.lightmapUV - (v1.lightmapUV - v0.lightmapUV) * t;

      return newVertex;
    }

    /// Transform all vertex positions and normal
    void Transform (const csReversibleTransform& transform)
    {
      for(size_t i = 0; i < vertexArray.GetSize (); ++i)
      {
        vertexArray[i].position = transform.This2Other (vertexArray[i].position);
        vertexArray[i].normal = transform.This2OtherRelative (vertexArray[i].normal);
      }
    }
  };

  /**
   * Baseclass for Object factories.
   * Needs to be subclassed to provide an interface to a specific mesh
   * object type to be able to import/export geometry data.
   */
  class ObjectFactory : public csRefCount
  {
  public:
    ObjectFactory ();

    virtual bool PrepareLightmapUV (LightmapUVFactoryLayouter* uvlayout);

    // Get a new object
    virtual Object* CreateObject ();

    // Parse data
    virtual void ParseFactory (iMeshFactoryWrapper *factory);
  
    // Write out the data again
    virtual void SaveFactory (iDocumentNode *node);

    // Getters
    inline float GetLMuTexelPerUnit () const
    { return lmuScale; }

    inline float GetLMvTexelPerUnit () const
    { return lmvScale; }


    // Name of the factory
    csString factoryName;

    /// Whether to light meshes of this factory per vertex
    bool lightPerVertex;
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
    csArray<PrimitiveArray> unlayoutedPrimitives;
    struct LayoutedPrimitives
    {
      PrimitiveArray primitives;
      csRef<LightmapUVObjectLayouter> factory;
      size_t group;

      LayoutedPrimitives (const PrimitiveArray& primitives, 
                          LightmapUVObjectLayouter* factory, size_t group) :
        primitives (primitives), factory (factory), group (group) {}
    };
    csArray<LayoutedPrimitives> layoutedPrimitives;

    // Vertex data for above faces
    ObjectVertexData vertexData;

    // Lightmap settings
    float lmuScale;
    float lmvScale;

    // Factory created from
    iMeshFactoryWrapper *factoryWrapper;

    // String identifying the saver plugin. Should be set from derived
    // classes
    const char* saverPluginName;

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
    virtual bool Initialize ();

    // Remove lightmap SVs. Add names of used lightmap textures to set
    virtual void StripLightmaps (csSet<csString>& lms);

    // Parse data
    virtual void ParseMesh (iMeshWrapper *wrapper);

    // Write out the data again
    virtual void SaveMesh (Scene* scene, iDocumentNode *node);

    // Write out data that must be written after lighting.
    virtual void SaveMeshPostLighting (Scene* scene) { }

    // Fill lightmap mask with primitive sub-pixel area coverage
    virtual void FillLightmapMask (LightmapMaskArray& masks);

    //-- Getters for data
    inline const csArray<PrimitiveArray>& GetPrimitives () const
    { return allPrimitives; }

    inline csArray<PrimitiveArray>& GetPrimitives ()
    { return allPrimitives; }
    
    inline const ObjectVertexData& GetVertexData () const
    { return vertexData; }

    inline ObjectVertexData& GetVertexData ()
    { return vertexData; }

    typedef csDirtyAccessArray<csColor> LitColorArray;
    inline LitColorArray* GetLitColors ()
    { return litColors; }

    typedef csHash<LitColorArray, csPtrKey<Light> > LitColorsPDHash;
    /// Return lit colors for all PD lights
    inline LitColorsPDHash* GetLitColorsPD ()
    { return litColorsPD; }
    /// Return lit colors for one PD light
    LitColorArray* GetLitColorsPD (Light* light);

    inline const csFlags& GetFlags () const
    { return objFlags; }

    // Name
    csString meshName;

    /// Whether to light mesh per vertex
    bool lightPerVertex;
  protected:
    // All faces, already transformed
    csArray<PrimitiveArray> allPrimitives;
    csArray<uint> lightmapIDs;

    // Vertex data for above, transformed
    ObjectVertexData vertexData;
    //@{
    /// Lit colors (if object is lit per-vertex)
    LitColorArray* litColors;
    LitColorsPDHash* litColorsPD;
    //@}

    // Factory we where created from
    ObjectFactory* factory;

    // Reference to the mesh
    iMeshWrapper *meshWrapper;

    // String identifying the saver plugin. Should be set from derived
    // classes
    const char* saverPluginName;

    // Internal flags
    csFlags objFlags;

    // Renormalize lightmap UVs into buffer \a lmcoords.
    virtual void RenormalizeLightmapUVs (const LightmapPtrDelArray& lightmaps,
      csVector2* lmcoords);

    // Helper function: get a filename prefix for this mesh
    csString GetFileName() const;

    friend class ObjectFactory;
  };
  typedef csRefArray<Object> ObjectRefArray;
  typedef csHash<csRef<Object>, csString> ObjectHash;
}

#endif // __OBJECT_H__
