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
#include "object_genmesh.h"
#include "config.h"
#include "scene.h"

namespace lighter
{

  ObjectFactory_Genmesh::ObjectFactory_Genmesh()
    : normals (0)
  {
    saverPluginName = "crystalspace.mesh.saver.factory.genmesh";
  }

  Object* ObjectFactory_Genmesh::CreateObject ()
  {
    return new Object_Genmesh (this);
  }

  void ObjectFactory_Genmesh::AddPrimitive (size_t a, size_t b, size_t c, 
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

    PrimitiveArray* primArray;
    if (submeshIndex != csArrayItemNotFound)
      primArray = &unlayoutedPrimitives[submeshIndex];
    else
    {
      submeshIndex = unlayoutedPrimitives.GetSize();
      primArray = &unlayoutedPrimitives.GetExtend (submeshIndex);
      Submesh newSubmesh;
      newSubmesh.sourceSubmesh = submesh;
      submeshes.Put (newSubmesh, submeshIndex);
    }

    Primitive newPrim (vertexData);
    Primitive::TriangleType t (a, b, c);
    newPrim.SetTriangle (t);

    newPrim.ComputePlane ();
    
    primArray->Push (newPrim);
  }

  void ObjectFactory_Genmesh::AddPrimitive (size_t a, size_t b, size_t c, 
                                               iMaterialWrapper* material)
  {
    PrimitiveArray* primArray;
    Submesh sm;
    sm.material = material;
    size_t submesh = submeshes.Get (sm, csArrayItemNotFound);
    if (submesh != csArrayItemNotFound)
      primArray = &unlayoutedPrimitives[submesh];
    else
    {
      submesh = unlayoutedPrimitives.GetSize();
      primArray = &unlayoutedPrimitives.GetExtend (submesh);
      submeshes.Put (sm, submesh);
    }

    Primitive newPrim (vertexData);
    Primitive::TriangleType t (a, b, c);
    newPrim.SetTriangle (t);

    newPrim.ComputePlane ();
    
    primArray->Push (newPrim);
  }

  void ObjectFactory_Genmesh::ParseFactory (iMeshFactoryWrapper *factory)
  {
    ObjectFactory::ParseFactory (factory);

    // Very dumb parser, just disconnect all triangles etc
    csRef<iGeneralFactoryState> genFact = 
      scfQueryInterface<iGeneralFactoryState> (factory->GetMeshObjectFactory ());
    
    if (!genFact) return; // bail

    genFact->RemoveRenderBuffer ("texture coordinate lightmap");
    genFact->Compress ();
    genFact->DisableAutoNormals();

    csVector3 *verts = genFact->GetVertices ();
    csVector2 *uv = genFact->GetTexels ();
    csVector3 *factNormals = genFact->GetNormals ();

    int i = 0;

    // Here we should save extra per-vertex stuff!
    vertexData.vertexArray.SetSize (genFact->GetVertexCount ());
    
    for (i = 0; i < genFact->GetVertexCount (); i++)
    {
      ObjectVertexData::Vertex &vertex = vertexData.vertexArray[i];
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

        CS::TriangleIndicesStream<size_t> tris (indices, 
          CS_MESHTYPE_TRIANGLES);

        while (tris.HasNext())
        {
          CS::TriangleT<size_t> tri (tris.Next ());

          AddPrimitive (tri.a, tri.b, tri.c, subMesh);
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

  void ObjectFactory_Genmesh::SaveFactory (iDocumentNode *node)
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
        const ObjectVertexData::Vertex &vertex = vertexData.vertexArray[i];
        verts[i] = vertex.position;
        textureUV[i] = vertex.textureUV;
        factNormals[i] = vertex.normal;
      }
    }

    genFact->ClearSubMeshes();
    SubmeshFindHelper findHelper (this);

    // Save primitives, trianglate on the fly
    for (uint i = 0; i < layoutedPrimitives.GetSize (); ++i)
    {
      const PrimitiveArray& meshPrims = layoutedPrimitives[i].primitives;
      IntDArray* indexArray = findHelper.FindSubmesh (i);
      indexArray->SetCapacity (meshPrims.GetSize()*3);
      for (size_t p = 0; p < meshPrims.GetSize(); p++)
      {
        const Primitive::TriangleType& t = meshPrims[p].GetTriangle ();
        for (int i = 0; i < 3; i++)
        {
          size_t idx = t[i];
          indexArray->Push ((int)idx);
        }
      }
    }

    findHelper.CommitSubmeshes (genFact);

    ObjectFactory::SaveFactory (node);
  }

  bool ObjectFactory_Genmesh::SubmeshesMergeable (iGeneralMeshSubMesh* sm1,
                                                     iGeneralMeshSubMesh* sm2)
  {
    if (sm1 == sm2) return true;

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

  void ObjectFactory_Genmesh::BeginSubmeshRemap ()
  {
  }

  void ObjectFactory_Genmesh::AddSubmeshRemap (size_t oldIndex, size_t newIndex)
  {
    const ObjectFactory_Genmesh::Submesh* smInfo = 
      submeshes.GetKeyPointer (oldIndex);

    tempSubmeshes.Put (*smInfo, newIndex);
  }

  void ObjectFactory_Genmesh::FinishSubmeshRemap ()
  {
    submeshes = tempSubmeshes;
    tempSubmeshes.Empty ();
  }

  IntDArray* ObjectFactory_Genmesh::SubmeshFindHelper::FindSubmesh (
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
      const ObjectFactory_Genmesh::Submesh* smInfo = 
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

  void ObjectFactory_Genmesh::SubmeshFindHelper::CommitSubmeshes (
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

      const ObjectFactory_Genmesh::Submesh* srcSubmesh = 
        factory->submeshes.GetKeyPointer (allocatedSubmeshes[i].submeshIndex);
      iMaterialWrapper* material = srcSubmesh->sourceSubmesh ? 
        srcSubmesh->sourceSubmesh->GetMaterial() : srcSubmesh->material;
      genFact->AddSubMesh (indices, material, 
        allocatedSubmeshes[i].name);

      factory->submeshNames.GetExtend (allocatedSubmeshes[i].submeshIndex) =
        allocatedSubmeshes[i].name;
    }
  }

  int ObjectFactory_Genmesh::SubmeshFindHelper::CompareAllocSubmesh (
    ObjectFactory_Genmesh::SubmeshFindHelper::AllocatedSubmesh const& item, 
    ObjectFactory_Genmesh::SubmeshFindHelper::AllocatedSubmeshKey const& key)
  {
    if (item.submeshIndex < key.submeshIndex)
      return -1;
    else if (item.submeshIndex > key.submeshIndex)
      return 1;
    else
      return 0;
  }

  //-------------------------------------------------------------------------

  Object_Genmesh::Object_Genmesh (ObjectFactory* factory) : Object (factory)
  {
    saverPluginName = "crystalspace.mesh.saver.genmesh";
  }

  void Object_Genmesh::SaveMesh (Scene* scene, iDocumentNode *node)
  {
    csRef<iGeneralMeshState> genMesh = 
      scfQueryInterface<iGeneralMeshState> (
      meshWrapper->GetMeshObject());
    if (!genMesh) return; // bail

    ObjectFactory_Genmesh* factory = 
      static_cast<ObjectFactory_Genmesh*> (this->factory);
    
    genMesh->SetShadowReceiving (false);
    genMesh->SetManualColors (true);
    genMesh->SetLighting (false);
    genMesh->RemoveRenderBuffer ("colors");
    genMesh->RemoveRenderBuffer ("texture coordinate lightmap");

   // Still may need to fix up submesh materials...
    CS::ShaderVarName lightmapName (globalLighter->strings, "tex lightmap");

    for (uint i = 0; i < allPrimitives.GetSize (); ++i)
    {
      csString submeshName;
      submeshName = factory->submeshNames[i];

      iGeneralMeshSubMesh* subMesh = genMesh->FindSubMesh (submeshName);
      if (!subMesh) continue;

      /* Fix up material (factory may not have a material set, but mesh object
       * material does not "propagate" to submeshes) */
      // FIXME: Should detect when this is necessary
      // FIXME: also can remove mesh object material then
      if (subMesh->GetMaterial() == 0)
      {
        csRef<iMeshObject> mo = 
          scfQueryInterface<iMeshObject> (genMesh);
        subMesh->SetMaterial (mo->GetMaterialWrapper());
      }

      if (!lightPerVertex)
      {
        csRef<iShaderVariableContext> svc = 
          scfQueryInterface<iShaderVariableContext> (subMesh);

        uint lmID = uint (lightmapIDs[i]);
        Lightmap* lm = scene->GetLightmaps()[lmID];
        csRef<csShaderVariable> svLightmap;
        svLightmap.AttachNew (new csShaderVariable (lightmapName));
        svLightmap->SetValue (lm->GetTexture());
        svc->AddVariable (svLightmap);
      }
    }

    if (!lightPerVertex)
    {
      csRef<csRenderBuffer> lightmapBuffer = csRenderBuffer::CreateRenderBuffer (
        vertexData.vertexArray.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 2);
      genMesh->AddRenderBuffer ("texture coordinate lightmap", lightmapBuffer);
      {
        csRenderBufferLock<csVector2> bufferLock (lightmapBuffer);
        // Save vertex-data
        RenormalizeLightmapUVs (scene->GetLightmaps(), bufferLock);
      }
    }

    Object::SaveMesh (scene, node);

    if (lightPerVertex)
    {
      /* For per-vertex lit objects we need to store the params in an external
         file as the complete data is only available after lighting. */
      csRef<iDocumentNode> srcParams = node->GetNode ("params");
      if (srcParams.IsValid())
      {
        csRef<iDocument> paramsDoc = globalLighter->docSystem->CreateDocument();
        csRef<iDocumentNode> root = paramsDoc->CreateRoot ();
        csRef<iDocumentNode> params = root->CreateNodeBefore (CS_NODE_ELEMENT);
        params->SetValue ("params");
        CS::DocumentHelper::CloneNode (srcParams, params);

        csString paramsFile;
        paramsFile.Format ("%s.params", GetFileName().GetData());
        paramsDoc->Write (globalLighter->vfs, paramsFile);

        csRef<iDocumentNode> srcParamsFile = node->GetNode ("paramsfile");
        csRef<iDocumentNode> srcParamsFileContent;
        if (!srcParamsFile.IsValid())
        {
          srcParamsFile = node->CreateNodeBefore (CS_NODE_ELEMENT, srcParams);
          srcParamsFile->SetValue ("paramsfile");
        }
        else
        {
          csRef<iDocumentNodeIterator> nodes = srcParamsFile->GetNodes();
          while (nodes->HasNext())
          {
            csRef<iDocumentNode> child = nodes->Next();
            if (child->GetType() != CS_NODE_TEXT) continue;
            srcParamsFileContent = child;
            break;
          }
        }
        if (!srcParamsFileContent.IsValid())
          srcParamsFileContent =
            srcParamsFile->CreateNodeBefore (CS_NODE_TEXT);
        srcParamsFileContent->SetValue (paramsFile);

        node->RemoveNode (srcParams);
      }
    }
  }

  void Object_Genmesh::SaveMeshPostLighting (Scene* scene)
  {
    // Tack on animation control for PD lights
    if (lightPerVertex)
    {
      scene->lightmapPostProc.ApplyAmbient (litColors->GetArray(),
        vertexData.vertexArray.GetSize()); 
      scene->lightmapPostProc.ApplyExposure (litColors->GetArray(),
        vertexData.vertexArray.GetSize()); 

      csRef<iRenderBuffer> staticColorsBuf = 
        csRenderBuffer::CreateRenderBuffer (litColors->GetSize(), 
          CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3, false);
      staticColorsBuf->CopyInto (litColors->GetArray(), litColors->GetSize());

      csRef<iSyntaxService> synsrv = 
        csQueryRegistry<iSyntaxService> (globalLighter->objectRegistry);

      csString paramsFile;
      paramsFile.Format ("%s.params", GetFileName().GetData());
      csRef<iFile> paramsData = globalLighter->vfs->Open (paramsFile, 
        VFS_FILE_READ);
      if (!paramsData.IsValid())
      {
        globalLighter->Report ("Error reading '%s'", 
          paramsFile.GetData());
        return;
      }
      csRef<iDocument> paramsDoc = globalLighter->docSystem->CreateDocument();
      const char* err = paramsDoc->Parse (paramsData);
      if (err != 0)
      {
        globalLighter->Report ("Error reading '%s': %s", 
          paramsFile.GetData(), err);
        return;
      }

      csRef<iDocumentNode> paramChild = paramsDoc->GetRoot()->GetNode ("params");

      if (litColorsPD && litColorsPD->GetSize() > 0)
      {
        csRef<iDocumentNode> animcontrolChild = 
          paramChild->CreateNodeBefore (CS_NODE_ELEMENT, 0);
        animcontrolChild->SetValue ("animcontrol");
        animcontrolChild->SetAttribute ("plugin", "crystalspace.mesh.anim.pdlight");

        {
          csRef<iDocumentNode> staticColorsChild =
            animcontrolChild->CreateNodeBefore (CS_NODE_ELEMENT, 0);
          staticColorsChild->SetValue ("staticcolors");

          synsrv->WriteRenderBuffer (staticColorsChild, staticColorsBuf);
        }

        LitColorsPDHash::GlobalIterator pdIter (litColorsPD->GetIterator ());
        while (pdIter.HasNext ())
        {
          csPtrKey<Light> light;
          LitColorArray& colors = pdIter.Next (light);

          csRef<iDocumentNode> lightChild =
            animcontrolChild->CreateNodeBefore (CS_NODE_ELEMENT, 0);
          lightChild->SetValue ("light");
          lightChild->SetAttribute ("lightid", light->GetLightID().HexString());

          scene->lightmapPostProc.ApplyExposure (colors.GetArray(),
            colors.GetSize()); 

          csRef<iRenderBuffer> colorsBuf = 
            csRenderBuffer::CreateRenderBuffer (colors.GetSize(), 
              CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3, false);
          colorsBuf->CopyInto (colors.GetArray(), colors.GetSize());

          synsrv->WriteRenderBuffer (lightChild, colorsBuf);
        }
      }
      else
      {
        csRef<iDocumentNode> renderbufferChild = 
          paramChild->CreateNodeBefore (CS_NODE_ELEMENT, 0);
        renderbufferChild->SetValue ("renderbuffer");

        renderbufferChild->SetAttribute ("name", "colors");

        synsrv->WriteRenderBuffer (renderbufferChild, staticColorsBuf);
      }

      err = paramsDoc->Write (globalLighter->vfs, paramsFile);
      if (err != 0)
      {
        globalLighter->Report ("Error writing '%s': %s", 
          paramsFile.GetData(), err);
      }
    }
  }

  void Object_Genmesh::StripLightmaps (csSet<csString>& lms)
  {
    Object::StripLightmaps (lms);

    ObjectFactory_Genmesh* factory = 
      static_cast<ObjectFactory_Genmesh*> (this->factory);

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

    /* FIXME: Also return external color buffer/params file.
     * -> That then needs a mechanism to tell the scene that these file 
     *    should not always be cleaned up. */
  }

}
