/*
  Copyright (C) 2005-2006 by Marten Svanfeldt

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

#include "crystalspace.h"

#include <algorithm>

#include "common.h"
#include "lighter.h"
#include "lightmap.h"
#include "lightmapuv.h"
#include "radobject_genmesh.h"
#include "config.h"
#include "scene.h"

namespace lighter
{

  RadObjectFactory_Genmesh::RadObjectFactory_Genmesh()
    : normals (0)
  {
    saverPluginName = "crystalspace.mesh.saver.factory.genmesh";
  }

  RadObject* RadObjectFactory_Genmesh::CreateObject ()
  {
    return new RadObject_Genmesh (this);
  }

  void RadObjectFactory_Genmesh::AddPrimitive (size_t a, size_t b, size_t c, 
                                               iGeneralMeshSubMesh* submesh)
  {
    size_t submeshIndex = csArrayItemNotFound;
    SubmeshHash::GlobalIterator smIt (submeshes.GetIterator ());
    while (smIt.HasNext())
    {
      Submesh currentSM;
      size_t newIndex = smIt.Next (currentSM);
      if (SubmeshesMergeable (currentSM.sourceSubmesh, submesh))
      {
        submeshIndex = newIndex;
        break;
      }
    }

    RadPrimitiveArray* primArray;
    if (submeshIndex != csArrayItemNotFound)
      primArray = &allPrimitives[submeshIndex];
    else
    {
      submeshIndex = allPrimitives.GetSize();
      primArray = &allPrimitives.GetExtend (submeshIndex);
      Submesh newSubmesh;
      newSubmesh.sourceSubmesh = submesh;
      submeshes.Put (newSubmesh, submeshIndex);
    }

    RadPrimitive newPrim (vertexData);
    newPrim.GetIndexArray ().Push (a);
    newPrim.GetIndexArray ().Push (b);
    newPrim.GetIndexArray ().Push (c);

    newPrim.ComputePlane ();
    
    primArray->Push (newPrim);
  }

  void RadObjectFactory_Genmesh::AddPrimitive (size_t a, size_t b, size_t c, 
                                               iMaterialWrapper* material)
  {
    RadPrimitiveArray* primArray;
    Submesh sm;
    sm.material = material;
    size_t submesh = submeshes.Get (sm, csArrayItemNotFound);
    if (submesh != csArrayItemNotFound)
      primArray = &allPrimitives[submesh];
    else
    {
      submesh = allPrimitives.GetSize();
      primArray = &allPrimitives.GetExtend (submesh);
      submeshes.Put (sm, submesh);
    }

    RadPrimitive newPrim (vertexData);
    newPrim.GetIndexArray ().Push (a);
    newPrim.GetIndexArray ().Push (b);
    newPrim.GetIndexArray ().Push (c);

    newPrim.ComputePlane ();
    
    primArray->Push (newPrim);
  }

  void RadObjectFactory_Genmesh::ParseFactory (iMeshFactoryWrapper *factory)
  {
    RadObjectFactory::ParseFactory (factory);

    // Very dumb parser, just disconnect all triangles etc
    csRef<iGeneralFactoryState> genFact = 
      scfQueryInterface<iGeneralFactoryState> (factory->GetMeshObjectFactory ());
    
    if (!genFact) return; // bail

    genFact->RemoveRenderBuffer ("texture coordinate lightmap");
    genFact->Compress ();

    bool keepSubMeshes = globalLighter->settings.keepGenmeshSubmeshes;

    csVector3 *verts = genFact->GetVertices ();
    csVector2 *uv = genFact->GetTexels ();
    csVector3 *factNormals = genFact->GetNormals ();

    int i = 0;

    // Here we should save extra per-vertex stuff!
    vertexData.vertexArray.SetSize (genFact->GetVertexCount ());
    
    for (i = 0; i < genFact->GetVertexCount (); i++)
    {
      RadObjectVertexData::Vertex &vertex = vertexData.vertexArray[i];
      vertex.position = verts[i];
      vertex.normal = factNormals[i];
      vertex.textureUV = uv[i];
    }

    if (genFact->GetSubMeshCount() > 0)
    {
      for (size_t s = 0; s < genFact->GetSubMeshCount(); s++)
      {
        iGeneralMeshSubMesh* subMesh = genFact->GetSubMesh (s);
        iRenderBuffer* indices = subMesh->GetIndices();
        iMaterialWrapper* material = subMesh->GetMaterial();

        CS::TriangleIndicesStream<size_t> tris;
        csRenderBufferLock<uint8> indexLock (indices);
        const uint8* indexEnd = indexLock + indices->GetSize();
        tris.BeginTriangulate (indexLock, indexEnd, indices->GetElementDistance(),
          indices->GetComponentType(), CS_MESHTYPE_TRIANGLES);

        while (tris.HasNextTri())
        {
          size_t a, b, c;
          tris.NextTriangle (a, b, c);

          AddPrimitive (a, b, c, subMesh);
        }
      }
    }
    else
    {
      csTriangle *tris = genFact->GetTriangles ();
      iMaterialWrapper* material = 
        factory->GetMeshObjectFactory()->GetMaterialWrapper();
      for (i=0; i < genFact->GetTriangleCount ();i++)
      {
        AddPrimitive (tris[i].a, tris[i].b, tris[i].c, material);
      }
    }
  }

  void RadObjectFactory_Genmesh::SaveFactory (iDocumentNode *node)
  {
    csRef<iGeneralFactoryState> genFact = 
      scfQueryInterface<iGeneralFactoryState> (
      factoryWrapper->GetMeshObjectFactory ());
    
    if (!genFact) return; // bail

    // For now, just dump.. later we should preserve extra attributes etc :)
    
    genFact->SetVertexCount ((int)vertexData.vertexArray.GetSize ());

    csVector3 *verts = genFact->GetVertices ();
    csVector2 *textureUV = genFact->GetTexels ();
    
    csVector3 *factNormals = genFact->GetNormals ();

    {
      // Save vertex-data
      for (int i = 0; i < genFact->GetVertexCount (); ++i)
      {
        const RadObjectVertexData::Vertex &vertex = vertexData.vertexArray[i];
        verts[i] = vertex.position;
        textureUV[i] = vertex.textureUV;
        factNormals[i] = vertex.normal;
      }
    }

    genFact->ClearSubMeshes();
    SubmeshFindHelper findHelper (this);

    // Save primitives, trianglate on the fly
    for (uint i = 0; i < allPrimitives.GetSize (); ++i)
    {
      const RadPrimitiveArray& meshPrims = allPrimitives[i];
      IntDArray* indexArray = findHelper.FindSubmesh (i);
      indexArray->SetCapacity (meshPrims.GetSize()*3);
      for (size_t p = 0; p < meshPrims.GetSize(); p++)
      {
        const SizeTDArray& indices = meshPrims[p].GetIndexArray ();
        if (indices.GetSize () == 3)
        {
          //Triangle, easy case
          for (int i = 0; i < 3; i++)
          {
            size_t idx = indices[i];
            indexArray->Push ((int)idx);
          }
        }
        else
        {
          //TODO: Implement this case, use a triangulator
          // @@@ RadObject_Genmesh atm only delivers triangles
        }
      }
    }

    findHelper.CommitSubmeshes (genFact);

    RadObjectFactory::SaveFactory (node);
  }

  bool RadObjectFactory_Genmesh::SubmeshesMergeable (iGeneralMeshSubMesh* sm1,
                                                     iGeneralMeshSubMesh* sm2)
  {
    if (sm1->GetMixmode() != sm2->GetMixmode()) return false;
    if (sm1->GetMaterial() != sm2->GetMaterial()) return false;

    csRef<iShaderVariableContext> svc1 = 
      scfQueryInterface<iShaderVariableContext> (sm1);
    csRef<iShaderVariableContext> svc2 = 
      scfQueryInterface<iShaderVariableContext> (sm2);
    // If we were more elaborate we'd compare all SVs.
    if (!svc1->IsEmpty() || !svc2->IsEmpty()) return false;

    return true;
  }

  IntDArray* RadObjectFactory_Genmesh::SubmeshFindHelper::FindSubmesh (
    size_t submeshIndex)
  {
    AllocatedSubmeshKey key;
    key.submeshIndex = submeshIndex;
    size_t index = allocatedSubmeshes.FindSortedKey (
      csArrayCmp<AllocatedSubmesh, AllocatedSubmeshKey> (key,
      CompareAllocSubmesh));
    if (index == csArrayItemNotFound)
    {
      AllocatedSubmesh newEntry (key);
      const RadObjectFactory_Genmesh::Submesh* smInfo = 
        factory->submeshes.GetKeyPointer (submeshIndex);
      
      if (smInfo->sourceSubmesh)
      {
        newEntry.name = smInfo->sourceSubmesh->GetName();
        int n = 0;
        while (usedNames.Contains (newEntry.name))
        {
          newEntry.name = smInfo->sourceSubmesh->GetName();
          newEntry.name += "_";
          newEntry.name += n++;
        }
        usedNames.Add (newEntry.name);
      }
      else
        newEntry.name = uint (allocatedSubmeshes.GetSize());

      index = allocatedSubmeshes.Push (newEntry);
    }
    return &allocatedSubmeshes[index].indices;
  }

  void RadObjectFactory_Genmesh::SubmeshFindHelper::CommitSubmeshes (
    iGeneralFactoryState* genFact)
  {
    for (size_t i = 0; i < allocatedSubmeshes.GetSize(); i++)
    {
      IntDArray& indexArray = allocatedSubmeshes[i].indices;

      size_t minIndex, maxIndex;
      size_t n = indexArray.GetSize();
      if (n & 1)
      {
        minIndex = maxIndex = indexArray[--n];
      }
      else
      {
        minIndex = (size_t)~0;
        maxIndex = 0;
      }
      while (n > 0)
      {
        size_t a = indexArray[--n];
        size_t b = indexArray[--n];
        if (a > b)
        {
          if (a > maxIndex) maxIndex = a;
          if (b < minIndex) minIndex = b;
        }
        else
        {
          if (b > maxIndex) maxIndex = b;
          if (a < minIndex) minIndex = a;
        }
      }

      csRef<iRenderBuffer> indices = 
        csRenderBuffer::CreateIndexRenderBuffer (indexArray.GetSize(),
          CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_INT, 
          minIndex, maxIndex);
      indices->CopyInto (indexArray.GetArray (), indexArray.GetSize());

      const RadObjectFactory_Genmesh::Submesh* srcSubmesh = 
        factory->submeshes.GetKeyPointer (allocatedSubmeshes[i].submeshIndex);
      iMaterialWrapper* material = srcSubmesh->sourceSubmesh ? 
        srcSubmesh->sourceSubmesh->GetMaterial() : srcSubmesh->material;
      genFact->AddSubMesh (indices, material, 
        allocatedSubmeshes[i].name);

      factory->submeshNames.GetExtend (allocatedSubmeshes[i].submeshIndex) =
        allocatedSubmeshes[i].name;
    }
  }

  int RadObjectFactory_Genmesh::SubmeshFindHelper::CompareAllocSubmesh (
    RadObjectFactory_Genmesh::SubmeshFindHelper::AllocatedSubmesh const& item, 
    RadObjectFactory_Genmesh::SubmeshFindHelper::AllocatedSubmeshKey const& key)
  {
    if (item.submeshIndex < key.submeshIndex)
      return -1;
    else if (item.submeshIndex > key.submeshIndex)
      return 1;
    else
      return 0;
  }

  //-------------------------------------------------------------------------

  RadObject_Genmesh::RadObject_Genmesh (RadObjectFactory* factory) : RadObject (factory)
  {
    saverPluginName = "crystalspace.mesh.saver.genmesh";
  }

  void RadObject_Genmesh::SaveMesh (Scene* scene, iDocumentNode *node)
  {
    csRef<iGeneralMeshState> genMesh = 
      scfQueryInterface<iGeneralMeshState> (
      meshWrapper->GetMeshObject());
    if (!genMesh) return; // bail

    RadObjectFactory_Genmesh* factory = 
      static_cast<RadObjectFactory_Genmesh*> (this->factory);

    CS::ShaderVarName lightmapName (globalLighter->strings, "tex lightmap");

    for (uint i = 0; i < allPrimitives.GetSize (); ++i)
    {
      csString submeshName;
      submeshName = factory->submeshNames[i];

      iGeneralMeshSubMesh* subMesh = genMesh->FindSubMesh (submeshName);
      if (!subMesh) continue;

      /* Fix up material (factory may not have a material set, but mesh object
       * material does not "propagate" to submeshes) */
      if (subMesh->GetMaterial() == 0)
      {
        csRef<iMeshObject> mo = 
          scfQueryInterface<iMeshObject> (genMesh);
        subMesh->SetMaterial (mo->GetMaterialWrapper());
      }

      csRef<iShaderVariableContext> svc = 
        scfQueryInterface<iShaderVariableContext> (subMesh);

      uint lmID = uint (lightmapIDs[i]);
      Lightmap* lm = scene->GetLightmaps()[lmID];
      csRef<csShaderVariable> svLightmap;
      svLightmap.AttachNew (new csShaderVariable (lightmapName));
      svLightmap->SetValue (lm->GetTexture());
      svc->AddVariable (svLightmap);
    }

    csRef<csRenderBuffer> lightmapBuffer = csRenderBuffer::CreateRenderBuffer (
      vertexData.vertexArray.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 2);
    genMesh->RemoveRenderBuffer ("texture coordinate lightmap");
    genMesh->AddRenderBuffer ("texture coordinate lightmap", lightmapBuffer);
    {
      csRenderBufferLock<csVector2> bufferLock(lightmapBuffer);
      csVector2 *lightmapUV = bufferLock.Lock ();
      // Save vertex-data
      for (size_t i = 0; i < vertexData.vertexArray.GetSize(); ++i)
      {
        const RadObjectVertexData::Vertex &vertex = vertexData.vertexArray[i];
        lightmapUV[i] = vertex.lightmapUV;
      }
    }

    RadObject::SaveMesh (scene, node);
  }

  void RadObject_Genmesh::StripLightmaps (csSet<csString>& lms)
  {
    RadObject::StripLightmaps (lms);

    RadObjectFactory_Genmesh* factory = 
      static_cast<RadObjectFactory_Genmesh*> (this->factory);

    csRef<iGeneralFactoryState> genFact = 
      scfQueryInterface<iGeneralFactoryState> (
      factory->factoryWrapper->GetMeshObjectFactory ());
    if (!genFact) return; // bail

    csRef<iGeneralMeshState> genMesh = 
      scfQueryInterface<iGeneralMeshState> (
      meshWrapper->GetMeshObject());
    if (!genMesh) return; // bail

    if (genFact->GetSubMeshCount() > 0)
    {
      for (size_t s = 0; s < genFact->GetSubMeshCount(); s++)
      {
        iGeneralMeshSubMesh* subMesh = genMesh->FindSubMesh (
          genFact->GetSubMesh (s)->GetName());
        if (!subMesh) continue;
        csRef<iShaderVariableContext> svc = 
          scfQueryInterface<iShaderVariableContext> (subMesh);
        csShaderVariable* sv = svc->GetVariable (
          globalLighter->strings->Request ("tex lightmap"));
        if (sv != 0)
        {
          iTextureWrapper* tex;
          sv->GetValue (tex);
          if (tex != 0)
            lms.Add (tex->QueryObject()->GetName());
          svc->RemoveVariable (sv);
        }
      }
    }
  }

}
