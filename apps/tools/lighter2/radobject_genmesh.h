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

#ifndef __RADOBJECT_GENMESH_H__
#define __RADOBJECT_GENMESH_H__

#include "csgeom/transfrm.h"

#include "lightmap.h"
#include "radobject.h"
#include "radprimitive.h"

namespace lighter
{
  class RadObject_Genmesh : public RadObject
  {
  public:
    RadObject_Genmesh (RadObjectFactory* factory);

    virtual void SaveMesh (Scene* scene, iDocumentNode *node);

    virtual void StripLightmaps (csSet<csString>& lms);
  };

  class RadObjectFactory_Genmesh : public RadObjectFactory
  {
  public:
    RadObjectFactory_Genmesh ();

    // Get a new object
    virtual RadObject* CreateObject ();

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
    friend class RadObject_Genmesh;

    // Extra data saved
    csVector3 *normals;

    typedef csHashReversible<size_t, Submesh> SubmeshHash;
    SubmeshHash submeshes;
    csArray<csString> submeshNames;

    void AddPrimitive (size_t a, size_t b, size_t c, 
      iGeneralMeshSubMesh* submesh);
    void AddPrimitive (size_t a, size_t b, size_t c, 
      iMaterialWrapper* material);

    static bool SubmeshesMergeable (iGeneralMeshSubMesh* sm1,
      iGeneralMeshSubMesh* sm2);

    struct SubmeshFindHelper
    {
      SubmeshFindHelper (RadObjectFactory_Genmesh* factory) : 
        factory (factory)
      { }

      IntDArray* FindSubmesh (size_t submeshIndex);
      void CommitSubmeshes (iGeneralFactoryState* genFact);
    protected:
      RadObjectFactory_Genmesh* factory;

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
struct csHashComputer<lighter::RadObjectFactory_Genmesh::Submesh> :
  public csHashComputerStruct<lighter::RadObjectFactory_Genmesh::Submesh> { };

template<>
struct csComparator<lighter::RadObjectFactory_Genmesh::Submesh, 
  lighter::RadObjectFactory_Genmesh::Submesh> : 
public csComparatorStruct<lighter::RadObjectFactory_Genmesh::Submesh> { };

#endif // __RADOBJECT_GENMESH_H__
