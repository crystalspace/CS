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

#include "common.h"

#include <algorithm>

#include "lighter.h"
#include "lightmap.h"
#include "lightmapuv.h"
#include "object_genmesh.h"
#include "config.h"
#include "scene.h"

namespace lighter
{
  ObjectFactory_Genmesh::ObjectFactory_Genmesh (const Configuration& config) :
    ObjectFactory (config)
  {
    submeshNames.AttachNew (new SubmeshNameArray);
    saverPluginName = "crystalspace.mesh.saver.factory.genmesh";
  }

  csPtr<Object> ObjectFactory_Genmesh::CreateObject ()
  {
    return csPtr<Object> (new Object_Genmesh (this));
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
      if ((!noModify && (SubmeshesMergeable (currentSM.sourceSubmesh, submesh)))
        || (currentSM.sourceSubmesh == submesh))
      {
        submeshIndex = newIndex;
        break;
      }
    }

    FactoryPrimitiveArray* primArray;
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

    FactoryPrimitive newPrim (vertexData);
    FactoryPrimitive::TriangleType t (a, b, c);
    newPrim.SetTriangle (t);

    newPrim.ComputePlane ();
    
    primArray->Push (newPrim);
  }

  void ObjectFactory_Genmesh::AddPrimitive (size_t a, size_t b, size_t c, 
                                               iMaterialWrapper* material)
  {
    FactoryPrimitiveArray* primArray;
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

    FactoryPrimitive newPrim (vertexData);
    FactoryPrimitive::TriangleType t (a, b, c);
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

    if (!noModify)
    {
      genFact->RemoveRenderBuffer ("texture coordinate lightmap");
      genFact->Compress ();
      genFact->DisableAutoNormals();
    }

    if (globalConfig.GetLighterProperties().directionalLMs)
    {
      vdataTangents = vertexData.AddCustomData (3);
      vdataBitangents = vertexData.AddCustomData (3);
    }

    const size_t vertCount = genFact->GetVertexCount ();

    vertexData.positions.SetSize (vertCount);
    memcpy (vertexData.positions.GetArray(), genFact->GetVertices (),
      vertCount * sizeof (csVector3));

    vertexData.normals.SetSize (vertCount);
    memcpy (vertexData.normals.GetArray(), genFact->GetNormals (),
      vertCount * sizeof (csVector3));

    vertexData.uvs.SetSize (vertCount);
    memcpy (vertexData.uvs.GetArray(), genFact->GetTexels (),
      vertCount * sizeof (csVector2));

    vertexData.ResizeCustomData ();
    if (globalConfig.GetLighterProperties().directionalLMs)
      SetupTangents (genFact);

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
      for (int i=0; i < genFact->GetTriangleCount ();i++)
      {
        AddPrimitive (tris[i].a, tris[i].b, tris[i].c, 
          (iMaterialWrapper*)0);
      }
    }
  }

  void ObjectFactory_Genmesh::SaveFactory (iDocumentNode *node)
  {
    csRef<iGeneralFactoryState> genFact = 
      scfQueryInterface<iGeneralFactoryState> (
      factoryWrapper->GetMeshObjectFactory ());
    
    if (!genFact) return; // bail

    if (noModify)
    {
      SubmeshFindHelper findHelper (this);
  
      // Save primitives, trianglate on the fly
      for (uint i = 0; i < layoutedPrimitives.GetSize (); ++i)
      {
	findHelper.FindSubmesh (i);
      }
      findHelper.CommitSubmeshNames ();
      return;
    }
    
    // TODO: Apply 'vertexData.splits' to user buffers

    const size_t vertCount = vertexData.positions.GetSize ();
    genFact->SetVertexCount ((int)vertCount);

    {
      csRef<iRenderBuffer> vertBuf = csRenderBuffer::CreateRenderBuffer (
        vertexData.positions.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
      vertBuf->SetData (vertexData.positions.GetArray());
      vertBuf = WrapBuffer (vertBuf, "v");
      genFact->RemoveRenderBuffer ("position");
      genFact->AddRenderBuffer ("position", vertBuf);
    }
    {
      csRef<iRenderBuffer> normBuf = csRenderBuffer::CreateRenderBuffer (
        vertexData.normals.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
      normBuf->SetData (vertexData.normals.GetArray());
      normBuf = WrapBuffer (normBuf, "n");
      genFact->RemoveRenderBuffer ("normal");
      genFact->AddRenderBuffer ("normal", normBuf);
    }
    {
      csRef<iRenderBuffer> tcBuf = csRenderBuffer::CreateRenderBuffer (
        vertexData.uvs.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 2);
      tcBuf->SetData (vertexData.uvs.GetArray());
      tcBuf = WrapBuffer (tcBuf, "tc");
      genFact->RemoveRenderBuffer ("texture coordinate 0");
      genFact->AddRenderBuffer ("texture coordinate 0", tcBuf);
    }

    if (hasTangents)
    {
      // Save tangents/bitangents, if we have them anyway
      csRef<iRenderBuffer> tangentBuf = csRenderBuffer::CreateRenderBuffer (
        vertexData.positions.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
      csRef<iRenderBuffer> bitangentBuf = csRenderBuffer::CreateRenderBuffer (
        vertexData.positions.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);

      tangentBuf = WrapBuffer (tangentBuf, "tng");
      bitangentBuf = WrapBuffer (bitangentBuf, "btg");

      csRenderBufferLock<csVector3> tangents (tangentBuf);
      csRenderBufferLock<csVector3> bitangents (bitangentBuf);
      for (size_t v = 0; v < vertCount; v++)
      {
        const csVector3& t = 
          *((csVector3*)vertexData.GetCustomData (v, vdataTangents));
        *tangents++ = t;
        const csVector3& b = 
          *((csVector3*)vertexData.GetCustomData (v, vdataBitangents));
        *bitangents++ = b;
      }

      genFact->RemoveRenderBuffer ("tangent");
      genFact->AddRenderBuffer ("tangent", tangentBuf);
      genFact->RemoveRenderBuffer ("bitangent");
      genFact->AddRenderBuffer ("bitangent", bitangentBuf);
    }

    genFact->ClearSubMeshes();
    SubmeshFindHelper findHelper (this);

    // Save primitives, trianglate on the fly
    for (uint i = 0; i < layoutedPrimitives.GetSize (); ++i)
    {
      const FactoryPrimitiveArray& meshPrims = layoutedPrimitives[i].primitives;
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

    findHelper.CommitSubmeshes (genFact, GetFileName());

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

  void ObjectFactory_Genmesh::SetupTangents (iGeneralFactoryState* genFact)
  {
    csRef<iRenderBuffer> tangentBuf = genFact->GetRenderBuffer ("tangent");
    csRef<iRenderBuffer> bitangentBuf = genFact->GetRenderBuffer ("bitangent");

    csRenderBufferLock<csVector3> tangents (tangentBuf, CS_BUF_LOCK_READ);
    csRenderBufferLock<csVector3> bitangents (bitangentBuf, CS_BUF_LOCK_READ);
    const size_t vertCount = vertexData.positions.GetSize ();
    for (size_t v = 0; v < vertCount; v++)
    {
      csVector3& t = *((csVector3*)vertexData.GetCustomData (v, vdataTangents));
      t = *tangents++;
      csVector3& b = *((csVector3*)vertexData.GetCustomData (v, vdataBitangents));
      b = *bitangents++;
    }
    hasTangents = true;
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
    iGeneralFactoryState* genFact, const char* factFN)
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
      indices->SetData (indexArray.GetArray ());
      indices = lighter::WrapBuffer (indices, csString().Format ("i%zu", i),
        factFN);

      const ObjectFactory_Genmesh::Submesh* srcSubmesh = 
        factory->submeshes.GetKeyPointer (allocatedSubmeshes[i].submeshIndex);
      iMaterialWrapper* material = srcSubmesh->sourceSubmesh ? 
        srcSubmesh->sourceSubmesh->GetMaterial() : srcSubmesh->material;
      genFact->AddSubMesh (indices, material, 
        allocatedSubmeshes[i].name);

      factory->submeshNames->GetExtend (allocatedSubmeshes[i].submeshIndex) =
        allocatedSubmeshes[i].name;
    }
  }

  void ObjectFactory_Genmesh::SubmeshFindHelper::CommitSubmeshNames ()
  {
    for (size_t i = 0; i < allocatedSubmeshes.GetSize(); i++)
    {
      factory->submeshNames->GetExtend (allocatedSubmeshes[i].submeshIndex) =
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

  Object_Genmesh::Object_Genmesh (ObjectFactory_Genmesh* factory) : 
    Object (factory), submeshNames (factory->submeshNames)
  {
    saverPluginName = "crystalspace.mesh.saver.genmesh";
  }
  
  bool Object_Genmesh::Initialize (Sector* sector)
  {
    if (!Object::Initialize (sector)) return false;
    
    csRef<iGeneralMeshState> genMesh = 
      scfQueryInterface<iGeneralMeshState> (
      meshWrapper->GetMeshObject());
    if (!genMesh) return false; // bail

    bool noSubmeshes = allPrimitives.GetSize () == 1; // @@@ insufficient test
    for (uint i = 0; i < allPrimitives.GetSize (); ++i)
    {
      csString submeshName;
      submeshName = submeshNames->Get (i);

      iGeneralMeshSubMesh* subMesh = genMesh->FindSubMesh (submeshName);
      if (!subMesh && !noSubmeshes) continue;

      iMaterialWrapper* material = 
        subMesh ? subMesh->GetMaterial() : (iMaterialWrapper*)0;
      if (material == 0)
      {
        csRef<iMeshObject> mo = 
          scfQueryInterface<iMeshObject> (genMesh);
        material = mo->GetMaterialWrapper();
      }
      if (material == 0) // If the material is still 0, get it from the factory.
      {
        csRef<iMeshObjectFactory> mof = 
          meshWrapper->GetFactory()->GetMeshObjectFactory();
        material = mof->GetMaterialWrapper();
      }

      const RadMaterial* radMat = sector->scene->GetRadMaterial (material);
      if (radMat == 0) continue;
      
      PrimitiveArray& primitives = allPrimitives[i];
      for (size_t p = 0; p < primitives.GetSize(); p++)
        primitives[p].SetMaterial (radMat);
    }
    
    return true;
  }

  void Object_Genmesh::SaveMesh (iDocumentNode *node)
  {
    if (objFlags.Check (OBJECT_FLAG_NOLIGHT))
      // Assume object is unchanged
      return;
    
    csRef<iGeneralMeshState> genMesh = 
      scfQueryInterface<iGeneralMeshState> (
      meshWrapper->GetMeshObject());
    if (!genMesh) return; // bail

    genMesh->SetShadowReceiving (false);
    genMesh->SetManualColors (true);
    genMesh->SetLighting (false);
    genMesh->RemoveRenderBuffer ("color");
    genMesh->RemoveRenderBuffer ("static color");
    genMesh->RemoveRenderBuffer ("texture coordinate lightmap");
    for (int i = 1; i < 4; i++)
    {
      csString name;
      name.Format ("color dir %d", i);
      genMesh->RemoveRenderBuffer (name);
      name.Format ("static color dir %d", i);
      genMesh->RemoveRenderBuffer (name);
    }

   // Still may need to fix up submesh materials...
    CS::ShaderVarName lightmapName[4] =
    { 
      CS::ShaderVarName (globalLighter->svStrings, "tex lightmap"),
      CS::ShaderVarName (globalLighter->svStrings, "tex lightmap dir 1"),
      CS::ShaderVarName (globalLighter->svStrings, "tex lightmap dir 2"),
      CS::ShaderVarName (globalLighter->svStrings, "tex lightmap dir 3")
    };
    CS::ShaderVarName specDirName[Scene::specDirectionMapCount];
    for (int i = 0; i < Scene::specDirectionMapCount; i++)
    {
      specDirName[i] = CS::ShaderVarName (globalLighter->svStrings,
        csString().Format ("tex spec directions %d", i+1));
    }
    int numLMs = globalConfig.GetLighterProperties().directionalLMs ? 4 : 1;

    bool noSubmeshes = allPrimitives.GetSize () == 1; // @@@ insufficient test
    for (uint i = 0; i < allPrimitives.GetSize (); ++i)
    {
      csString submeshName;
      submeshName = submeshNames->Get (i);

      iGeneralMeshSubMesh* subMesh = genMesh->FindSubMesh (submeshName);
      if (!subMesh && !noSubmeshes) continue;

      /* Fix up material (factory may not have a material set, but mesh object
       * material does not "propagate" to submeshes) */
      // FIXME: Should detect when this is necessary
      // FIXME: also can remove mesh object material then
      if (subMesh && (subMesh->GetMaterial() == 0))
      {
        csRef<iMeshObject> mo = 
          scfQueryInterface<iMeshObject> (genMesh);
        subMesh->SetMaterial (mo->GetMaterialWrapper());
      }

      if (!lightPerVertex)
      {
        csRef<iShaderVariableContext> svc = 
          scfQueryInterface<iShaderVariableContext> (
            subMesh ? (iBase*)subMesh : (iBase*)meshWrapper);

	uint lmID = uint (lightmapIDs[i]);
        for (int l = 0; l < numLMs; l++)
        {
          Lightmap* lm = sector->scene->GetLightmap(lmID, l);
          csRef<csShaderVariable> svLightmap;
          svLightmap.AttachNew (new csShaderVariable (lightmapName[l]));
          svLightmap->SetValue (lm->GetTexture());
          svc->AddVariable (svLightmap);
        }
        
        if (globalConfig.GetLighterProperties().specularDirectionMaps)
        {
          csRef<csShaderVariable> svInfluence;
          for (int i = 0; i < Scene::specDirectionMapCount; i++)
          {
            svInfluence.AttachNew (new csShaderVariable (specDirName[i]));
            svInfluence->SetValue (sector->scene->GetSpecDirectionMapTexture (
              lmID, i));
            svc->AddVariable (svInfluence);
          }
        }
      }
    }

    if (!lightPerVertex)
    {
      csRef<iRenderBuffer> lightmapBuffer = csRenderBuffer::CreateRenderBuffer (
        vertexData.positions.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 2);
      lightmapBuffer = WrapBuffer (lightmapBuffer, "lm");
      genMesh->AddRenderBuffer ("texture coordinate lightmap", lightmapBuffer);
      {
        csRenderBufferLock<csVector2> bufferLock (lightmapBuffer);
        // Save vertex-data
        RenormalizeLightmapUVs (sector->scene->GetLightmaps(), bufferLock);
      }
    }

    Object::SaveMesh (node);

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
        CS::DocSystem::CloneNode (srcParams, params);

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

  void Object_Genmesh::FreeNotNeededForLighting ()
  {
    Object::FreeNotNeededForLighting ();
    submeshNames.Invalidate();
  }

  void Object_Genmesh::SaveMeshPostLighting (Scene* scene)
  {
    static const char* bufferNames[] = 
    { "static color", "static color dir 1", "static color dir 2", "static color dir 3" };
    const int numBufs = 
      (globalConfig.GetLighterProperties().directionalLMs) ? 4 : 1;

    // Tack on animation control for PD lights
    if (lightPerVertex)
    {
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

      csRef<iDocumentNode> animcontrolChild;
      if (litColorsPD)
      {
        animcontrolChild = paramChild->CreateNodeBefore (CS_NODE_ELEMENT, 0);
        animcontrolChild->SetValue ("animcontrol");
        animcontrolChild->SetAttribute ("plugin", "crystalspace.mesh.anim.pdlight");
      }

      for (int b = 0; b < numBufs; b++)
      {
        scene->lightmapPostProc.ApplyAmbient (litColors[b].GetArray(),
          vertexData.positions.GetSize()); 
        scene->lightmapPostProc.ApplyExposure (litColors[b].GetArray(),
          vertexData.positions.GetSize()); 

        csRef<iRenderBuffer> staticColorsBuf = 
          csRenderBuffer::CreateRenderBuffer (litColors[b].GetSize(), 
            CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
        staticColorsBuf = WrapBuffer (staticColorsBuf,
          csString().Format ("c%d", b));
        staticColorsBuf->SetData (litColors[b].GetArray());

        if (litColorsPD[b].GetSize() > 0)
        {
          csRef<iDocumentNode> bufferChild = 
            animcontrolChild->CreateNodeBefore (CS_NODE_ELEMENT, 0);
          bufferChild->SetValue ("buffer");
          bufferChild->SetAttribute ("name", bufferNames[b]);

          {
            csRef<iDocumentNode> staticColorsChild =
              bufferChild->CreateNodeBefore (CS_NODE_ELEMENT, 0);
            staticColorsChild->SetValue ("staticcolors");

            synsrv->WriteRenderBuffer (staticColorsChild, staticColorsBuf);
          }

          uint n = 0;
          LitColorsPDHash::GlobalIterator pdIter (litColorsPD[b].GetIterator ());
          while (pdIter.HasNext ())
          {
            csPtrKey<Light> light;
            LitColorArray& colors = pdIter.Next (light);

            csRef<iDocumentNode> lightChild =
              bufferChild->CreateNodeBefore (CS_NODE_ELEMENT, 0);
            lightChild->SetValue ("light");
            lightChild->SetAttribute ("lightsector", light->GetSector()->sectorName);
            lightChild->SetAttribute ("lightname", light->GetName());

            scene->lightmapPostProc.ApplyExposure (colors.GetArray(),
              colors.GetSize()); 

            csRef<iRenderBuffer> colorsBuf = 
              csRenderBuffer::CreateRenderBuffer (colors.GetSize(), 
                CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
	    colorsBuf = WrapBuffer (colorsBuf,
	      csString().Format ("c%d_pd%u", b, n++));
            colorsBuf->SetData (colors.GetArray());

            synsrv->WriteRenderBuffer (lightChild, colorsBuf);
          }
        }
        else
        {
          csRef<iDocumentNode> renderbufferChild = 
            paramChild->CreateNodeBefore (CS_NODE_ELEMENT, 0);
          renderbufferChild->SetValue ("renderbuffer");

          renderbufferChild->SetAttribute ("name", bufferNames[b]);

          synsrv->WriteRenderBuffer (renderbufferChild, staticColorsBuf);
        }
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

    csRef<iGeneralFactoryState> genFact = 
      scfQueryInterface<iGeneralFactoryState> (
        meshWrapper->GetFactory()->GetMeshObjectFactory ());
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
        for (int i = 0; i < 4; i++)
        {
          csString svName;
          if (i == 0)
            svName = "tex lightmap";
          else
            svName.Format ("tex lightmap dir %d", i);
          csShaderVariable* sv = svc->GetVariable (
            globalLighter->svStrings->Request (svName));
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

    /* FIXME: Also return external color buffer/params file.
     * -> That then needs a mechanism to tell the scene that these file 
     *    should not always be cleaned up. */
  }

}
