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

#ifndef __RADOBJECT_H__
#define __RADOBJECT_H__

#include "radprimitive.h"
#include "lightmap.h"
#include "csgeom/transfrm.h"

namespace lighter
{
  class LightmapUVLayouter;
  class RadObject;

  /**
   * Hold per object vertex data
   */
  struct RadObjectVertexData 
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
   * Baseclass for RadObject factories.
   * Needs to be subclassed to provide an interface to a specific mesh
   * object type to be able to import/export geometry data.
   */
  class RadObjectFactory : public csRefCount
  {
  public:
    RadObjectFactory ();

    // Compute lightmap coordinates for this factory. Returns true on success
    virtual bool ComputeLightmapUV (LightmapUVLayouter* layoutEngine);

    // Renormalize lightmap UVs
    virtual void RenormalizeLightmapUVs ();

    // Get a new object
    virtual RadObject* CreateObject ();

    // Parse data
    virtual void ParseFactory (iMeshFactoryWrapper *factory);
  
    // Write out the data again
    virtual void SaveFactory (iDocumentNode *node);

    // Name of the factory
    csString factoryName;
  
  protected:

    // All faces, untransformed
    RadPrimitiveArray allPrimitives;

    // Vertex data for above faces
    RadObjectVertexData vertexData;

    // Size of the required lightmaps
    LightmapPtrDelArray lightmapTemplates;

    // Lightmap masks
    LightmapMaskArray lightmapMasks;
    bool lightmapMaskArrayValid;

    // Factory created from
    iMeshFactoryWrapper *factoryWrapper;

    // String identifying the saver plugin. Should be set from derived
    // classes
    csString saverPluginName;

    friend class RadObject;
  };
  typedef csRefArray<RadObjectFactory> RadObjectFactoryRefArray;
  typedef csHash<csRef<RadObjectFactory>, csString> RadObjectFactoryHash;

  /**
  * A single object in the computational scene.
  * Each object exists in a single spot in a single sector, but there
  * might be more instances created from a single factory in different places.
  */
  class RadObject : public csRefCount
  {
  public:
    // Construct a new RadObject from a RadObjectFactory and transform
    RadObject (RadObjectFactory* object);

    // Initialize the RadObject from factory and wrapper. Call only after
    // constructor and ParseMesh have been called
    virtual void Initialize ();

    // Parse data
    virtual void ParseMesh (iMeshWrapper *wrapper);

    // Write out the data again
    virtual void SaveMesh (iDocumentNode *node);

    // Fixup the lightmap borders
    virtual void FixupLightmaps ();

    // Accessor 
    const LightmapPtrDelArray& GetLightmaps () const 
    { return lightmaps; }

    LightmapPtrDelArray& GetLightmaps () 
    { return lightmaps; }

    const RadPrimitiveArray& GetPrimitives () const
    { return allPrimitives; }

    RadPrimitiveArray& GetPrimitives ()
    { return allPrimitives; }
    
    const RadObjectVertexData& GetVertexData () const
    { return vertexData; }

    RadObjectVertexData& GetVertexData ()
    { return vertexData; }

    // Name
    csString meshName;

  protected:
    // All faces, already transformed
    RadPrimitiveArray allPrimitives;

    // Vertex data for above, transformed
    RadObjectVertexData vertexData;

    // Lightmaps associated with this mesh
    LightmapPtrDelArray lightmaps;

    // Factory we where created from
    RadObjectFactory* factory;

    // Reference to the mesh
    iMeshWrapper *meshWrapper;

    friend class  RadObjectFactory;
  };
  typedef csRefArray<RadObject> RadObjectRefArray;
  typedef csHash<csRef<RadObject>, csString> RadObjectHash;

  //////////////////////////////////////////////////////////////////////////
  // SPECIFIC VERISONS
  //////////////////////////////////////////////////////////////////////////
  
  //////////////   Genmesh     /////////////////////////////////////////////
  class RadObjectFactory_Genmesh : public RadObjectFactory
  {
  public:
    RadObjectFactory_Genmesh ();

    // Parse data
    virtual void ParseFactory (iMeshFactoryWrapper *factory);

    // Write out the data again
    virtual void SaveFactory (iDocumentNode *node);
  protected:

    // Extra data saved
    csVector3 *normals;
  };

}

#endif
