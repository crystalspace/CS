/*
  Copyright (C) 2007 by Frank Richter

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

#include "object.h"
#include "raydebug.h"
#include "scene.h"

#include "imesh/genmesh.h"

namespace lighter
{
  RayDebugHelper::LightObjectPair::LightObjectPair (Light* light, 
    Object* obj) : light (light), obj (obj) {}

  RayDebugHelper::RayDebugHelper () : reMatcher (0) 
  {
  }

  RayDebugHelper::~RayDebugHelper ()
  { 
    delete reMatcher; 
  }

  RayDebugHelper::RayAndHits& RayDebugHelper::GetHits (Light* light, Object* obj, 
                                                       const Ray &ray)
  {
    SectorData& sectorData = allData.GetOrCreate (obj->GetSector ());
    LightObjectPair key (light->GetOriginalLight(), obj);
    ObjectData& objData = sectorData.objectData.GetOrCreate (key);
    RayAndHits* rayAndHits = objData.rays.GetElementPointer (ray.rayID);
    if (rayAndHits == 0)
    {
      rayAndHits = &objData.rays.Put (ray.rayID, RayAndHits ());
      rayAndHits->rayStart = ray.origin;
      rayAndHits->rayEnd = ray.origin + ray.direction * ray.maxLength;
    }
    return *rayAndHits;
  }

  void RayDebugHelper::RealRegisterHit (Light* light, Object* obj,
                                        const Ray &ray, const HitPoint &hit)
  {
    if (!obj->GetFlags().Check (OBJECT_FLAG_RAYDEBUG)) return;
    RayAndHits& hits = GetHits (light, obj, ray);
    hits.hits.Push (hit.distance);
  }

  void RayDebugHelper::RealRegisterUnhit (Light* light, Object* obj, 
                                          const Ray &ray)
  {
    if (!obj->GetFlags().Check (OBJECT_FLAG_RAYDEBUG)) return;
    GetHits (light, obj, ray);
  }

  void RayDebugHelper::SetFilterExpression (const csString& expr)
  {
    delete reMatcher;
    if (expr.IsEmpty ())
      reMatcher = 0;
    else
      reMatcher = new csRegExpMatcher (expr);
  }

  bool RayDebugHelper::EnableForMesh (const char* name)
  {
    return (reMatcher != 0) && (reMatcher->Match (name) == csrxNoError);
  }

  static const size_t maxRaysPerMesh = 250000;

  class FactoryWriter
  {
    csRef<iDocumentNode> node;
    int numTri;
    uint numVt;
    
    csDirtyAccessArray<csVector3> positions;
    csDirtyAccessArray<csColor> colors;
    csDirtyAccessArray<uint> indices;
    
    uint WriteVertex (const csVector3& v, const csColor& color)
    {
      positions.Push (v);
      colors.Push (color);
      return numVt++;
    }
    void WriteTri (const csTriangle& tri)
    {
      indices.Push (tri.a);
      indices.Push (tri.b);
      indices.Push (tri.c);
      numTri++;
    }
  public:
    FactoryWriter (iDocumentNode* node) : node (node), numTri (0), numVt (0)
    {
      positions.SetCapacity (maxRaysPerMesh * 3);
      colors.SetCapacity (maxRaysPerMesh * 3);
      indices.SetCapacity (maxRaysPerMesh * 2 * 3);
    }

    void AddLine (csVector3& v1, const csVector3& v2, const csColor& color)
    {
      csVector3 vd = v1-v2;
      vd.Normalize ();
      csVector3 up (0, 1, 0);
      if (fabsf (vd * up) < SMALL_EPSILON) up.Set (0, 0, 1);
      csVector3 n (vd % up);
      n *= EPSILON;

      int idx1 = WriteVertex (v1, color);
      WriteVertex (v2, color);
      WriteVertex (v2+n, color);

      // Generate thin quads.
      // @@@ Make it a real line once genmesh supports these.
      csTriangle tri;
      tri.a = idx1;
      tri.b = idx1+1;
      tri.c = idx1+2;
      WriteTri (tri);
      CS::Swap (tri.a, tri.c);
      WriteTri (tri);
    }

    void Finish (const char* filename, iSyntaxService* synldr)
    {
      {
        csRef<iRenderBuffer> buf (csRenderBuffer::CreateRenderBuffer (
          numVt, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3));
        buf->SetData (positions.GetArray ());
        
        csString fn;
        fn.Format ("%s_v", filename);
        
        csRef<CS::RenderBufferPersistent> bufPersist;
        bufPersist.AttachNew (new CS::RenderBufferPersistent (
          buf));
        bufPersist->SetFileName (fn);
        
	csRef<iDocumentNode> bufNode = 
	  node->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	bufNode->SetValue ("renderbuffer");
        bufNode->SetAttribute ("name", "position");
        bufNode->SetAttribute ("checkelementcount", "no");
        synldr->WriteRenderBuffer (bufNode, bufPersist);
      }
      
      {
        csRef<iRenderBuffer> buf (
          csRenderBuffer::CreateIndexRenderBuffer (
            numTri * 3, CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_INT, 
            0, numVt-1));
        buf->SetData (indices.GetArray ());
        
        csString fn;
        fn.Format ("%s_i", filename);
        
        csRef<CS::RenderBufferPersistent> bufPersist;
        bufPersist.AttachNew (new CS::RenderBufferPersistent (
          buf));
        bufPersist->SetFileName (fn);
        
	csRef<iDocumentNode> bufNode = 
	  node->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	bufNode->SetValue ("renderbuffer");
        bufNode->SetAttribute ("name", "index");
        bufNode->SetAttribute ("checkelementcount", "no");
        synldr->WriteRenderBuffer (bufNode, bufPersist);
      }
      
      {
        csRef<iRenderBuffer> buf (csRenderBuffer::CreateRenderBuffer (
          numVt, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3));
        buf->SetData (colors.GetArray ());
        
        csString fn;
        fn.Format ("%s_c", filename);
        
        csRef<CS::RenderBufferPersistent> bufPersist;
        bufPersist.AttachNew (new CS::RenderBufferPersistent (
          buf));
        bufPersist->SetFileName (fn);
        
	csRef<iDocumentNode> bufNode = 
	  node->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	bufNode->SetValue ("renderbuffer");
        bufNode->SetAttribute ("name", "color");
        synldr->WriteRenderBuffer (bufNode, bufPersist);
      }
    }
  };

  void RayDebugHelper::AppendMeshFactories (Sector* sector, 
                                            iDocumentNode* node, 
                                            Statistics::Progress& progress)
  {
    SectorData* sectorData = allData.GetElementPointer (sector);
    if (!sectorData) return;

    progress.SetProgress (0);
    float pairProgress = 1.0f / sectorData->objectData.GetSize();

    const char genmeshClassID[] = "crystalspace.mesh.object.genmesh";
    const char genmeshFactoryLoaderID[] = 
      "crystalspace.mesh.loader.factory.genmesh";

    csRef<iMeshObjectType> genmeshType = csLoadPluginCheck<iMeshObjectType> (
      globalLighter->objectRegistry, genmeshClassID);
    if (!genmeshType.IsValid()) return;

    SectorData::ObjectDataHash::GlobalIterator sectorDataIt (
      sectorData->objectData.GetIterator ());
    while (sectorDataIt.HasNext ())
    {
      LightObjectPair lightAndObj (0, 0);
      ObjectData& objData = sectorDataIt.Next (lightAndObj);

      size_t numFactories = 
        (objData.rays.GetSize() + maxRaysPerMesh - 1) / maxRaysPerMesh;
      ObjectData::RayHash::GlobalIterator rayIt (objData.rays.GetIterator ());

      for (size_t f = 0; f < numFactories; f++)
      {
        csRef<iDocumentSystem> docSys;
        docSys.AttachNew (new csTinyDocumentSystem);
        csRef<iDocument> doc (docSys->CreateDocument ());
        csRef<iDocumentNode> rootNode (doc->CreateRoot ());

        csRef<iDocumentNode> factNode = 
          rootNode->CreateNodeBefore (CS_NODE_ELEMENT);
        factNode->SetValue ("meshfact");

        csRef<iDocumentNode> pluginNode = 
          factNode->CreateNodeBefore (CS_NODE_ELEMENT);
        pluginNode->SetValue ("plugin");
        csRef<iDocumentNode> pluginContentNode = 
          pluginNode->CreateNodeBefore (CS_NODE_TEXT);
        pluginContentNode->SetValue (genmeshFactoryLoaderID);

        csRef<iDocumentNode> paramsNode = 
          factNode->CreateNodeBefore (CS_NODE_ELEMENT);
        paramsNode->SetValue ("params");

        csRef<iDocumentNode> lightingNode = 
          paramsNode->CreateNodeBefore (CS_NODE_ELEMENT);
        lightingNode->SetValue ("lighting");
        csRef<iDocumentNode> lightingContentNode = 
          lightingNode->CreateNodeBefore (CS_NODE_TEXT);
        lightingContentNode->SetValue ("no");

        csRef<iDocumentNode> manualcolorsNode = 
          paramsNode->CreateNodeBefore (CS_NODE_ELEMENT);
        manualcolorsNode->SetValue ("manualcolors");

        FactoryWriter writer (paramsNode);

        size_t nRays = 0;
        while (rayIt.HasNext () && (nRays++ < maxRaysPerMesh))
        {
          RayAndHits& rayAndHits = rayIt.Next ();
          if (rayAndHits.hits.GetSize() == 0)
          {
            const csColor col (0, 1, 0);
            writer.AddLine (rayAndHits.rayStart, rayAndHits.rayEnd, col);
          }
          else
          {
            const csColor col[3] = 
            { csColor (1, 0, 0), csColor (1, 1, 0), csColor (0.5, 0, 0) };
            rayAndHits.hits.Sort ();
            csVector3 lastPoint = rayAndHits.rayStart;
            csVector3 normDir (rayAndHits.rayEnd - rayAndHits.rayStart);
            normDir.Normalize ();
            for (size_t h = 0; h < rayAndHits.hits.GetSize(); h++)
            {
              csVector3 p (rayAndHits.rayStart + normDir * rayAndHits.hits[h]);
              writer.AddLine (lastPoint, p, col[h & 1]);
              lastPoint = p;
            }
            writer.AddLine (lastPoint, rayAndHits.rayEnd, col[2]);
          }
        }

        csString nStr ("");
        if (f > 0) nStr.Format ("_%zu", f);
        csString lightName (lightAndObj.light->GetName ());
        if (lightName.IsEmpty()) 
          lightName = lightAndObj.light->GetLightID().HexString();
        csString factoryName;
        factoryName.Format ("__lighter2_debug__factory_ray_%s_%s_%s%s_", 
          sector->sectorName.GetData(),
          lightAndObj.obj->meshName.GetData(),
          lightName.GetData(), nStr.GetData());

        writer.Finish (factoryName, globalLighter->syntaxService);

        csString filename (factoryName);
        filename += ".xml";
        csRef<iFile> factFile = globalLighter->vfs->Open (filename, VFS_FILE_WRITE);
        doc->Write (factFile);

        factNode = node->CreateNodeBefore (CS_NODE_ELEMENT);
        factNode->SetValue ("meshfact");
        factNode->SetAttribute ("name", factoryName);
        factNode->SetAttribute ("file", filename);
      }

      progress.IncProgress (pairProgress);
    }
    progress.SetProgress (1);
  }

  void RayDebugHelper::AppendMeshObjects (Sector* sector, 
                                          iDocumentNode* node, 
                                          Statistics::Progress& progress)
  {
    SectorData* sectorData = allData.GetElementPointer (sector);
    if (!sectorData) return;

    progress.SetProgress (0);
    float pairProgress = 1.0f / sectorData->objectData.GetSize();

    const char genmeshSaverID[] = "crystalspace.mesh.saver.genmesh";
    const char genmeshLoaderID[] = "crystalspace.mesh.loader.genmesh";

    csRef<iSaverPlugin> saver = csLoadPluginCheck<iSaverPlugin> (
      globalLighter->objectRegistry, genmeshSaverID);
    if (!saver.IsValid()) return;

    SectorData::ObjectDataHash::GlobalIterator sectorDataIt (
      sectorData->objectData.GetIterator ());
    while (sectorDataIt.HasNext ())
    {
      LightObjectPair lightAndObj (0, 0);
      ObjectData& objData = sectorDataIt.Next (lightAndObj);

      size_t numFactories = 
        (objData.rays.GetSize() + maxRaysPerMesh - 1) / maxRaysPerMesh;

      for (size_t f = 0; f < numFactories; f++)
      {
        csString nStr ("");
        if (f > 0) nStr.Format ("_%zu", f);
        csString lightName (lightAndObj.light->GetName ());
        if (lightName.IsEmpty()) 
          lightName = lightAndObj.light->GetLightID().HexString();
        csString factoryName, meshName;
        factoryName.Format ("__lighter2_debug__factory_ray_%s_%s_%s%s_", 
          sector->sectorName.GetData(),
          lightAndObj.obj->meshName.GetData(),
          lightName.GetData(), nStr.GetData());
        meshName.Format ("__lighter2_debug__ray_%s_%s_%s%s_", 
          sector->sectorName.GetData(),
          lightAndObj.obj->meshName.GetData(),
          lightName.GetData(), nStr.GetData());

        csRef<iDocumentNode> meshNode = node->CreateNodeBefore (CS_NODE_ELEMENT);
        meshNode->SetValue ("meshobj");
        meshNode->SetAttribute ("name", meshName);

        csRef<iDocumentNode> pluginNode = 
          meshNode->CreateNodeBefore (CS_NODE_ELEMENT);
        pluginNode->SetValue ("plugin");
        csRef<iDocumentNode> pluginContentNode = 
          pluginNode->CreateNodeBefore (CS_NODE_TEXT);
        pluginContentNode->SetValue (genmeshLoaderID);

        csRef<iDocumentNode> paramsNode = 
          meshNode->CreateNodeBefore (CS_NODE_ELEMENT);
        paramsNode->SetValue ("params");

        csRef<iDocumentNode> factoryNode = 
          paramsNode->CreateNodeBefore (CS_NODE_ELEMENT);
        factoryNode->SetValue ("factory");
        csRef<iDocumentNode> factoryContentNode = 
          factoryNode->CreateNodeBefore (CS_NODE_TEXT);
        factoryContentNode->SetValue (factoryName);

        csRef<iDocumentNode> materialNode = 
          paramsNode->CreateNodeBefore (CS_NODE_ELEMENT);
        materialNode->SetValue ("material");
        csRef<iDocumentNode> materialContentNode = 
          materialNode->CreateNodeBefore (CS_NODE_TEXT);
        materialContentNode->SetValue ("white");

        csRef<iDocumentNode> trimeshNode = 
          meshNode->CreateNodeBefore (CS_NODE_ELEMENT);
        trimeshNode->SetValue ("trimesh");
        csRef<iDocumentNode> idNode = 
          trimeshNode->CreateNodeBefore (CS_NODE_ELEMENT);
        idNode->SetValue ("id");
        csRef<iDocumentNode> idContentNode = 
          idNode->CreateNodeBefore (CS_NODE_TEXT);
        idContentNode->SetValue ("colldet");
      }

      progress.IncProgress (pairProgress);
    }
    progress.SetProgress (1);
  }

  void RayDebugHelper::FreeInfo (Sector* sector)
  {
    allData.DeleteAll (sector);
  }
} // namespace lighter
