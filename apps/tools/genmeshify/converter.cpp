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
  }

  bool Converter::ConvertMeshFact (iDocumentNode* from, iDocumentNode* to)
  {
    if (!thingFactLoader.IsValid()) return false;

    csRef<iDocumentNode> paramsNode = from->GetNode ("params");
    if (!paramsNode) return false;

    return false;
  }

  bool Converter::ConvertMeshObj (const char* name,
                                  iDocumentNode* from, iDocumentNode* to,
                                  iDocumentNode* sectorNode)
  {
    if (!thingObjLoader.IsValid()) return false;

    csRef<iDocumentNode> paramsNode = from->GetNode ("params");
    if (!paramsNode) return false;

    csRef<iMeshWrapper> meshWrap = app->engine->CreateMeshWrapper (name);
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

    csRef<iThingFactoryState> thingfact = 
      scfQueryInterface<iThingFactoryState> (mobj->GetFactory ());
    if (!thingfact)
    {
      app->Report ("Loaded mesh object does not supply an iThingFactoryState");
      return false;
    }

    csString factoryName;
    if (name)
      factoryName.Format ("factory_%s_%x", name, rng.Get((uint32)~0));
    else
      factoryName.Format ("factory_%x", rng.Get((uint32)~0));

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
    if (!CopyThingToGM (thingfact, gmfact)) return false;

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
    csRef<iMeshObject> gmObj = mof->NewInstance ();
    if (!gmObj.IsValid()) return false;
    gmObj->SetMixMode (mobj->GetMixMode ());
    if (!gmSaver->WriteDown (gmObj, to, 0)) return false;

    if (!ExtractPortals (meshWrap, sectorNode)) return false;

    return true;
  }

  bool Converter::CopyThingToGM (iThingFactoryState* from, 
                                 iGeneralFactoryState* to)
  {
    typedef csHash<csArray<Poly>, csPtrKey<iMaterialWrapper> > MatPolyHash;
    // Step 1: collect all polygons, sorted by materials
    MatPolyHash polies;
    const csVector3* vertices = from->GetVertices();
    const csVector3* normals = from->GetNormals();

    int polycount = from->GetPolygonCount();
    for (int p = 0; p < polycount; p++)
    {
      int vc = from->GetPolygonVertexCount (p);
      const int* vtIndex = from->GetPolygonVertexIndices (p);
      csMatrix3 texM; 
      csVector3 texV;
      from->GetPolygonTextureMapping (p, texM, texV);
      csTransform object2texture (texM, texV);
      csVector3 polynormal (-from->GetPolygonObjectPlane (p).Normal());
      
      Poly newPoly;
      for (int v = 0; v < vc; v++)
      {
        int vt = vtIndex[v];
        PolyVertex newVertex;
        newVertex.pos = vertices[vt];
        newVertex.normal = normals ? normals[vt] : polynormal;
        csVector3 t = object2texture.Other2This (newVertex.pos);
        newVertex.tc.Set (t.x, t.y);
        newPoly.vertices.Push (newVertex);
      }
      iMaterialWrapper* polyMat = from->GetPolygonMaterial (p);
      csArray<Poly>* matPolies = polies.GetElementPointer (polyMat);
      if (matPolies == 0)
      {
        csArray<Poly> newArray;
        newArray.Push (newPoly);
        polies.Put (polyMat, newArray);
      }
      else
        matPolies->Push (newPoly);
    }

    // Step 2: layout lightmaps

    // Step 3: create GM submeshes
    {
      size_t vertexTotal = 0;
      MatPolyHash::GlobalIterator it = polies.GetIterator ();
      while (it.HasNext())
      {
        csPtrKey<iMaterialWrapper> mat;
        const csArray<Poly>& polies = it.Next (mat);

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
        csVector3* vertPtr = to->GetVertices() + vertexNum;
        csVector3* normPtr = to->GetNormals() + vertexNum;
        csVector2* tcPtr = to->GetTexels() + vertexNum;

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
        to->AddSubMesh (indexBuffer, mat, 0);
      }
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
}
