/*
  Copyright (C) 2005 by Marten Svanfeldt
            (C) 2006 by Frank Richter

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

#include "converter.h"
#include "genmeshify.h"

namespace genmeshify
{
  Converter::Converter (App* app, iLoaderContext* context, iRegion* region) : 
    app (app), context (context), region (region)
  {
    gmfactSaver = csLoadPluginCheck<iSaverPlugin> (app->objectRegistry,
      "crystalspace.mesh.saver.factory.genmesh", true);
    gmSaver = csLoadPluginCheck<iSaverPlugin> (app->objectRegistry,
      "crystalspace.mesh.saver.genmesh", true);
    if (gmfactSaver.IsValid() && gmSaver.IsValid())
    {
      thingFactLoader = csLoadPluginCheck<iLoaderPlugin> (app->objectRegistry,
        "crystalspace.mesh.loader.factory.thing", true);
      thingObjLoader = csLoadPluginCheck<iLoaderPlugin> (app->objectRegistry,
        "crystalspace.mesh.loader.thing", true);
    }

    idTexLightmap = app->strings->Request ("tex lightmap");
  }

  bool Converter::ConvertMeshFact (iDocumentNode* from, iDocumentNode* to)
  {
    if (!thingFactLoader.IsValid()) return false;

    csRef<iDocumentNode> paramsNode = from->GetNode ("params");
    if (!paramsNode) return false;

    return false;
  }

  bool Converter::ConvertMeshObj (iSector* sector,
                                  const char* meshName, 
                                  iDocumentNode* from, iDocumentNode* to,
                                  iDocumentNode* sectorNode,
                                  iDocumentNode* textures)
  {
    if (!thingObjLoader.IsValid()) return false;

    csRef<iDocumentNode> paramsNode = from->GetNode ("params");
    if (!paramsNode) return false;

    csRef<iMeshWrapper> meshWrap = app->engine->CreateMeshWrapper (meshName);
    region->QueryObject ()->ObjAdd (meshWrap->QueryObject());

    csRef<iBase> obj = thingObjLoader->Parse (paramsNode, 0, context, meshWrap);
    if (!obj) return false;
    csRef<iMeshObject> mobj = scfQueryInterface<iMeshObject> (obj);
    if (!mobj)
    {
      app->Report ("Loaded mesh object does not implement iMeshObject");
      return false;
    }
    meshWrap->SetMeshObject (mobj);
    mobj->SetMeshWrapper (meshWrap);
    csRef<iThingState> thingobj = scfQueryInterface<iThingState> (obj);
    if (!thingobj)
    {
      app->Report ("Loaded mesh object does not implement iThingState");
      return false;
    }

    // Check for a hard transform
    {
      csRef<iDocumentNode> hardTFnode = from->GetNode ("hardmove");
      if (hardTFnode.IsValid())
      {
        csReversibleTransform tr;
	csRef<iDocumentNode> matrix_node = hardTFnode->GetNode ("matrix");
	if (matrix_node)
	{
	  csMatrix3 m;
          if (!app->synsrv->ParseMatrix (matrix_node, m))
	    return false;
          tr.SetT2O (m);
	}
	csRef<iDocumentNode> vector_node = hardTFnode->GetNode ("v");
	if (vector_node)
	{
	  csVector3 v;
	  if (!app->synsrv->ParseVector (vector_node, v))
	    return false;
          tr.SetOrigin (v);
	}
        meshWrap->HardTransform (tr);
      }
    }
    meshWrap->GetMovable()->SetSector (sector);
    iLightingInfo* linfo = meshWrap->GetLightingInfo ();
    if (linfo) linfo->ReadFromCache (app->engine->GetCacheManager());

    // @@@ FIXME: properly share factories, if possible
    csRef<iThingFactoryState> thingfact = 
      scfQueryInterface<iThingFactoryState> (mobj->GetFactory ());
    if (!thingfact)
    {
      app->Report ("Loaded mesh object does not supply an iThingFactoryState");
      return false;
    }

    csString uniqueName;
    uniqueName.Format ("%s_%s", sector->QueryObject()->GetName(), meshName);
    csString factoryName;
    factoryName.Format ("factory_%s", uniqueName.GetData());

    csRef<iMeshFactoryWrapper> mfw = app->engine->CreateMeshFactory (
      "crystalspace.mesh.object.genmesh", factoryName);
    if (!mfw)
    {
      app->Report ("Could not create genmesh factory");
      return false;
    }
    region->QueryObject ()->ObjAdd (mfw->QueryObject());
    csRef<iMeshObjectFactory> mof = mfw->GetMeshObjectFactory();
    csRef<iGeneralFactoryState> gmfact = 
      scfQueryInterface<iGeneralFactoryState> (mof);
    if (!gmfact)
    {
      app->Report ("Factory does not implement iGeneralFactoryState");
      return false;
    }
    LMLayout lmLayout;
    if (!CopyThingToGM (thingfact, gmfact, uniqueName, lmLayout)) return false;

    if (!ExtractLightmaps (lmLayout, thingobj, textures)) 
      return false;

    {
      csRef<iDocumentNode> factoryNode = 
        sectorNode->GetParent()->CreateNodeBefore (CS_NODE_ELEMENT,
          sectorNode);
      factoryNode->SetValue ("meshfact");
      factoryNode->SetAttribute ("name", factoryName);

      csRef<iDocumentNode> pluginNode = 
        factoryNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
      pluginNode->SetValue ("plugin");

      csRef<iDocumentNode> pluginIdNode = 
        pluginNode->CreateNodeBefore (CS_NODE_TEXT, 0);
      pluginIdNode->SetValue ("crystalspace.mesh.loader.factory.genmesh");

      if (!gmfactSaver->WriteDown (gmfact, factoryNode, 0)) return false;
    }
    csRef<iMeshObject> newObj = mof->NewInstance ();
    if (!newObj.IsValid()) return false;
    newObj->SetMixMode (mobj->GetMixMode ());
    csRef<iGeneralMeshState> gmObj = 
      scfQueryInterface<iGeneralMeshState> (newObj);

    // Add lightmap SVs
    for (size_t i = 0; i < lmLayout.subMeshes.GetSize(); i++)
    {
      const LMLayout::SubMesh& layoutSM = lmLayout.subMeshes[i];
      csRef<iTextureWrapper> dummyTex = 
        app->engine->FindTexture (lmLayout.slmNames[layoutSM.slm], region);
      if (!dummyTex.IsValid())
      {
        dummyTex = app->engine->CreateBlackTexture (
          lmLayout.slmNames[layoutSM.slm], 1, 1, 0, 0);
        region->Add (dummyTex->QueryObject());
      }

      iGeneralMeshSubMesh* submesh = 
        gmObj->FindSubMesh (layoutSM.name);
      if (!submesh) continue;
      csRef<iShaderVariableContext> svc = 
        scfQueryInterface<iShaderVariableContext> (submesh);
      csRef<csShaderVariable> sv;
      sv.AttachNew (new csShaderVariable (idTexLightmap));
      sv->SetValue (dummyTex);
      svc->AddVariable (sv);
    }

    if (!gmSaver->WriteDown (gmObj, to, 0)) return false;

    if (!ExtractPortals (meshWrap, sectorNode)) return false;

    return true;
  }

  struct PolyHashKey
  {
    iMaterialWrapper* material;
    size_t slm;

    uint GetHash () const { return (uintptr_t)material ^ (uint)slm; }
    inline friend bool operator < (const PolyHashKey& r1, const PolyHashKey& r2)
    { 
      if (r1.material < r2.material) return true;
      return r1.slm < r2.slm;
    }
  };

  bool Converter::CopyThingToGM (iThingFactoryState* from, 
                                 iGeneralFactoryState* to,
                                 const char* name, 
                                 LMLayout& layout)
  {
    typedef csHash<csArray<Poly>, PolyHashKey> MatPolyHash;

    int polycount = from->GetPolygonCount();

    csDirtyAccessArray<float> lmCoords;
    // Step 1: layout lightmaps
    for (int p = 0; p < polycount; p++)
    {
      size_t lmcOffs = lmCoords.GetSize();
      lmCoords.SetSize (lmcOffs + 2*from->GetPolygonVertexCount (p));

      LMLayout::Lightmap lm;
      lm.hasLM = from->GetLightmapLayout (p, lm.slm,
        lm.rectOnSLM, lmCoords.GetArray() + lmcOffs);
      layout.polyLightmaps.Push (lm);

      if (lm.hasLM)
      {
        LMLayout::Dim& dim = layout.slmDimensions.GetExtend (lm.slm);
        dim.GrowTo (lm.rectOnSLM.xmax, lm.rectOnSLM.ymax);
      }
    }
    // Generate super LM names
    for (size_t s = 0; s < layout.slmDimensions.GetSize(); s++)
    {
      csString lightmapName;
      lightmapName.Format ("lightmap_%s_%zu", name, s);
      layout.slmNames.Push (lightmapName);
    }

    // Step 2: collect all polygons, sorted by materials + superlightmap
    MatPolyHash polies;
    const csVector3* vertices = from->GetVertices();
    const csVector3* normals = from->GetNormals();

    float* lmcoordPtr = lmCoords.GetArray();
    for (int p = 0; p < polycount; p++)
    {
      int vc = from->GetPolygonVertexCount (p);
      const int* vtIndex = from->GetPolygonVertexIndices (p);
      csMatrix3 texM; 
      csVector3 texV;
      from->GetPolygonTextureMapping (p, texM, texV);
      csTransform object2texture (texM, texV);
      csVector3 polynormal (-from->GetPolygonObjectPlane (p).Normal());
      const LMLayout::Lightmap& lm = layout.polyLightmaps[p];
      
      Poly newPoly;
      newPoly.orgIndex = p;
      for (int v = 0; v < vc; v++)
      {
        int vt = vtIndex[v];
        PolyVertex newVertex;
        newVertex.pos = vertices[vt];
        newVertex.normal = normals ? normals[vt] : polynormal;
        csVector3 t = object2texture.Other2This (newVertex.pos);
        newVertex.tc.Set (t.x, t.y);
        if (lm.hasLM)
        {
          newVertex.tclm.x = *lmcoordPtr++;
          newVertex.tclm.y = *lmcoordPtr++;
        }
        else
          newVertex.tclm.Set (0, 0);
        newPoly.vertices.Push (newVertex);
      }
      iMaterialWrapper* polyMat = from->GetPolygonMaterial (p);
      PolyHashKey key;
      key.material = polyMat;
      key.slm = layout.polyLightmaps[p].hasLM ? layout.polyLightmaps[p].slm 
        : csArrayItemNotFound;
      csArray<Poly>* matPolies = polies.GetElementPointer (key);
      if (matPolies == 0)
      {
        csArray<Poly> newArray;
        newArray.Push (newPoly);
        polies.Put (key, newArray);
      }
      else
        matPolies->Push (newPoly);
    }

    // Step 3: create GM submeshes
    {
      size_t vertexTotal = 0;
      csDirtyAccessArray<csVector2> tclm;
      MatPolyHash::GlobalIterator it = polies.GetIterator ();
      while (it.HasNext())
      {
        PolyHashKey key;
        const csArray<Poly>& polies = it.Next (key);

        size_t numVerts = 0;
        size_t numTris = 0;
        for (size_t p = 0; p < polies.GetSize(); p++)
        {
          numVerts += polies[p].vertices.GetSize();
          numTris += polies[p].vertices.GetSize() - 2;
        }
        size_t vertexNum = vertexTotal;
        vertexTotal += numVerts;

        to->SetVertexCount ((int)vertexTotal);
        tclm.SetSize (vertexTotal);
        csVector3* vertPtr = to->GetVertices() + vertexNum;
        csVector3* normPtr = to->GetNormals() + vertexNum;
        csVector2* tcPtr = to->GetTexels() + vertexNum;
        csVector2* tclmPtr = tclm.GetArray() + vertexNum;

        csRef<iRenderBuffer> indexBuffer = 
          csRenderBuffer::CreateIndexRenderBuffer (numTris * 3, 
          CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_INT, 0, numVerts - 1);
        {
          csRenderBufferLock<uint, iRenderBuffer*> index (indexBuffer);

          for (size_t p = 0; p < polies.GetSize(); p++)
          {
            const csArray<PolyVertex>& verts = polies[p].vertices;
            size_t vertex0 = vertexNum;
            for (size_t v = 0; v < verts.GetSize(); v++)
            {
              *vertPtr++ = verts[v].pos;
              *normPtr++ = verts[v].normal;
              *tcPtr++ = verts[v].tc;
              *tclmPtr++ = verts[v].tclm;
              if (v >= 2)
              {
                *index++ = (uint)vertex0;
                *index++ = (uint)(vertexNum-1);
                *index++ = (uint)(vertexNum);
              }
              vertexNum++;
            }
          }
        }
        csString submeshName;
        if (key.slm != csArrayItemNotFound)
        {
          submeshName.Format ("%s_%zu", 
            key.material->QueryObject()->GetName(), key.slm);
          LMLayout::SubMesh layoutSM;
          layoutSM.name = submeshName;
          layoutSM.slm = key.slm;
          layout.subMeshes.Push (layoutSM);
        }
        iGeneralMeshSubMesh* submesh = 
          to->AddSubMesh (indexBuffer, key.material, submeshName);
      }
      csRef<csRenderBuffer> tclmBuffer = csRenderBuffer::CreateRenderBuffer (
        tclm.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 2);
      tclmBuffer->CopyInto (tclm.GetArray(), tclm.GetSize());
      to->AddRenderBuffer ("texture coordinate lightmap", tclmBuffer);
    }
    
    return true;
  }

  bool Converter::ExtractPortals (iMeshWrapper* mesh, iDocumentNode* to)
  {
    csRef<iDocumentNode> portalsNode;

    iSceneNode* sceneNode = mesh->QuerySceneNode ();
    csRef<iSceneNodeArray> nodeChildren = sceneNode->GetChildrenArray ();
    for (size_t c = 0; c < nodeChildren->GetSize(); c++)
    {
      iSceneNode* childNode = nodeChildren->Get (c);
      iMeshWrapper* childMesh = childNode->QueryMesh ();
      if (!childMesh) continue;
      iPortalContainer* portals = childMesh->GetPortalContainer ();
      if (!portals) continue;

      for (int p = 0; p < portals->GetPortalCount(); p++)
      {
        iPortal* portal = portals->GetPortal (p);
        if (!portalsNode.IsValid())
        {
          portalsNode = to->CreateNodeBefore (CS_NODE_ELEMENT, 0);
          portalsNode->SetValue ("portals");
        }
        if (!app->saver->SavePortal (portal, portalsNode)) return false;
      }
    }

    return true;
  }

  bool Converter::ExtractLightmaps (const LMLayout& layout, 
                                    iThingState* object, 
                                    iDocumentNode* textures)
  {
    csRefArray<csImageMemory> slmImages;
    for (size_t i = 0; i < layout.slmDimensions.GetSize(); i++)
    {
      const LMLayout::Dim& dim = layout.slmDimensions[i];
      csRef<csImageMemory> newImg;
      newImg.AttachNew (new csImageMemory (dim.w, dim.h));
      slmImages.Push (newImg);
    }
    for (size_t p = 0; p < layout.polyLightmaps.GetSize(); p++)
    {
      const LMLayout::Lightmap& lm = layout.polyLightmaps[p];
      if (!lm.hasLM) continue;
      csRef<iImage> lightmap = object->GetPolygonLightmap (int (p));
      if (!lightmap) continue;
      slmImages[lm.slm]->Copy (lightmap, lm.rectOnSLM.xmin, lm.rectOnSLM.ymin,
        lm.rectOnSLM.Width(), lm.rectOnSLM.Height());
    }
    for (size_t s = 0; s < slmImages.GetSize(); s++)
    {
      const char* lightmapName = layout.slmNames[s];
      csString fn;
      fn.Format ("lightmaps/%s.png", lightmapName);

      csRef<iDataBuffer> buf = app->imageIO->Save (slmImages[s], "image/png");
      if (!buf.IsValid())
      {
        app->Report ("Error saving lightmap to PNG");
        return false;
      }
      if (!app->vfs->WriteFile (fn, buf->GetData(), buf->GetSize()))
      {
        app->Report ("Error writing to file %s", fn.GetData());
        return false;
      }

      csRef<iDocumentNode> textureNode = 
        textures->CreateNodeBefore (CS_NODE_ELEMENT, 0);
      textureNode->SetValue ("texture");
      textureNode->SetAttribute ("name", lightmapName);

      csRef<iDocumentNode> textureFileNode = 
        textureNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
      textureFileNode->SetValue ("file");

      csRef<iDocumentNode> textureFileContent = 
        textureFileNode->CreateNodeBefore (CS_NODE_TEXT, 0);
      textureFileContent->SetValue (fn);

      csRef<iDocumentNode> textureClassNode = 
        textureNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
      textureClassNode->SetValue ("class");

      csRef<iDocumentNode> textureClassContent = 
        textureClassNode->CreateNodeBefore (CS_NODE_TEXT, 0);
      textureClassContent->SetValue ("lightmap");
    }
    return true;
  }
}
