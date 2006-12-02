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

#include "primitive.h"
#include "lightmap.h"
#include "lightmapuv.h"
#include "csgeom/transfrm.h"

namespace lighter
{
  class LightmapUVFactoryLayouter;
  class LightmapUVObjectLayouter;
  class Object;
  class Scene;

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

    /// Transform all vertex positions
    void Transform (const csReversibleTransform& transform)
    {
      for(size_t i = 0; i < vertexArray.GetSize (); ++i)
      {
        vertexArray[i].position = transform.This2Other (vertexArray[i].position);
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

    // Name of the factory
    csString factoryName;

    /// Whether to light meshes of this factory per vertex
    bool lightPerVertex;
  protected:

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

    // Lightmap masks
    LightmapMaskArray lightmapMasks;
    bool lightmapMaskArrayValid;

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

    // Renormalize lightmap UVs
    virtual void RenormalizeLightmapUVs (const LightmapPtrDelArray& lightmaps);

    // Remove lightmap SVs. Add names of used lightmap textures to set
    virtual void StripLightmaps (csSet<csString>& lms);

    // Parse data
    virtual void ParseMesh (iMeshWrapper *wrapper);

    // Write out the data again
    virtual void SaveMesh (Scene* scene, iDocumentNode *node);

    // Fixup the lightmap borders
    virtual void FixupLightmaps (csArray<LightmapPtrDelArray*>& lightmaps);

    const csArray<PrimitiveArray>& GetPrimitives () const
    { return allPrimitives; }

    csArray<PrimitiveArray>& GetPrimitives ()
    { return allPrimitives; }
    
    const ObjectVertexData& GetVertexData () const
    { return vertexData; }

    ObjectVertexData& GetVertexData ()
    { return vertexData; }

    typedef csDirtyAccessArray<csColor> LitColorArray;
    LitColorArray* GetLitColors ()
    { return litColors; }

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
    /// Lit colors (if object is lit per-vertex)
    LitColorArray* litColors;

    // Factory we where created from
    ObjectFactory* factory;

    // Reference to the mesh
    iMeshWrapper *meshWrapper;

    // String identifying the saver plugin. Should be set from derived
    // classes
    const char* saverPluginName;

    friend class  ObjectFactory;
  };
  typedef csRefArray<Object> ObjectRefArray;
  typedef csHash<csRef<Object>, csString> ObjectHash;
}

#endif // __OBJECT_H__
