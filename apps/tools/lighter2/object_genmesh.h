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

#ifndef __OBJECT_GENMESH_H__
#define __OBJECT_GENMESH_H__

#include "csgeom/transfrm.h"

#include "lightmap.h"
#include "object.h"
#include "primitive.h"

namespace lighter
{
  struct SubmeshNameArray : public csArray<csString>,
                            public csRefCount
  {
  };

  class ObjectFactory_Genmesh;

  class Object_Genmesh : public Object
  {
    csRef<SubmeshNameArray> submeshNames;
  public:
    Object_Genmesh (ObjectFactory_Genmesh* factory);

    virtual bool Initialize (Sector* sector);
    
    virtual void SaveMesh (iDocumentNode *node);

    virtual void FreeNotNeededForLighting ();

    virtual void SaveMeshPostLighting (Scene* scene);

    virtual void StripLightmaps (csSet<csString>& lms);
  };

  class ObjectFactory_Genmesh : public ObjectFactory
  {
  public:
    ObjectFactory_Genmesh (const Configuration& config);

    // Get a new object
    virtual csPtr<Object> CreateObject ();

    // Parse data
    virtual void ParseFactory (iMeshFactoryWrapper *factory);

    // Write out the data again
    virtual void SaveFactory (iDocumentNode *node);

    struct Submesh
    {
      csRef<iGeneralMeshSubMesh> sourceSubmesh;
      iMaterialWrapper* material;

      Submesh() : material (0) {}
    };
  protected:
    friend class Object_Genmesh;

    void SetupTangents (iGeneralFactoryState* genFact);

    // Begin remapping of submeshes
    virtual void BeginSubmeshRemap ();

    // Add a new mapping between old index and new index
    virtual void AddSubmeshRemap (size_t oldIndex, size_t newIndex);

    // Finish remapping of submeshes
    virtual void FinishSubmeshRemap ();

    typedef csHashReversible<size_t, Submesh> SubmeshHash;
    SubmeshHash submeshes;
    SubmeshHash tempSubmeshes;
    csRef<SubmeshNameArray> submeshNames;

    void AddPrimitive (size_t a, size_t b, size_t c, 
      iGeneralMeshSubMesh* submesh);
    void AddPrimitive (size_t a, size_t b, size_t c, 
      iMaterialWrapper* material);

    static bool SubmeshesMergeable (iGeneralMeshSubMesh* sm1,
      iGeneralMeshSubMesh* sm2);

    struct SubmeshFindHelper
    {
      SubmeshFindHelper (ObjectFactory_Genmesh* factory) : 
        factory (factory)
      { }

      IntDArray* FindSubmesh (size_t submeshIndex);
      void CommitSubmeshes (iGeneralFactoryState* genFact,
        const char* factFN);
      void CommitSubmeshNames ();
    protected:
      ObjectFactory_Genmesh* factory;

      struct AllocatedSubmeshKey
      {
        size_t submeshIndex;
      };
      struct AllocatedSubmesh : public AllocatedSubmeshKey
      {
        IntDArray indices;
        csString name;

        AllocatedSubmesh (const AllocatedSubmeshKey& other) :
          AllocatedSubmeshKey (other) {}
      };
      csArray<AllocatedSubmesh> allocatedSubmeshes;
      csSet<csString> usedNames;
      static int CompareAllocSubmesh (AllocatedSubmesh const& item, 
        AllocatedSubmeshKey const& key);
    };
  };
}

template<>
class csHashComputer<lighter::ObjectFactory_Genmesh::Submesh> :
  public csHashComputerStruct<lighter::ObjectFactory_Genmesh::Submesh> { };

template<>
class csComparator<lighter::ObjectFactory_Genmesh::Submesh, 
  lighter::ObjectFactory_Genmesh::Submesh> : 
public csComparatorStruct<lighter::ObjectFactory_Genmesh::Submesh> { };

#endif // __OBJECT_GENMESH_H__
