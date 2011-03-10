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

#include "lighter.h"
#include "lightmap.h"
#include "lightmapuv.h"
#include "scene.h"
#include "object.h"
#include "config.h"

namespace lighter
{

  csPtr<iRenderBuffer> WrapBuffer (iRenderBuffer* buffer, 
                                   const char* suffix,
                                   const char* basename)
  {
    csRef<iRenderBuffer> newBuffer;
    if (globalConfig.GetLighterProperties().saveBinaryBuffers)
    {
      csString newFn;
      newFn.Format ("bindata/%s_%s", basename, suffix);
      CS::RenderBufferPersistent* persistBuf =
        new CS::RenderBufferPersistent (buffer);
      persistBuf->SetFileName (newFn);
      newBuffer.AttachNew (persistBuf);
    }
    else
      newBuffer = buffer;
    return csPtr<iRenderBuffer> (newBuffer);
  }

  ObjectFactory::ObjectFactory (const Configuration& config)
    : lightPerVertex (false), noModify (false), hasTangents (false),
    noSelfShadow (false),
    lmScale (config.GetLMProperties ().lmDensity),
    factoryWrapper (0)
  {
  }

  bool ObjectFactory::PrepareLightmapUV (LightmapUVFactoryLayouter* uvlayout)
  {
    if (factoryWrapper->GetFlags().Check (CS_ENTITY_NOLIGHTING))
    {
      unlayoutedPrimitives.DeleteAll();
      noModify = true;
      return true;
    }
    
    BeginSubmeshRemap ();
    if (lightPerVertex)
    {
      size_t oldSize = unlayoutedPrimitives.GetSize();
      for (size_t i = 0; i < oldSize; i++)
      {
        layoutedPrimitives.Push (LayoutedPrimitives (unlayoutedPrimitives[i],
          0, 0));
        AddSubmeshRemap (i, i);
      }
      unlayoutedPrimitives.DeleteAll();
    }
    else
    {
      size_t oldSize = unlayoutedPrimitives.GetSize();
      csBitArray usedVertices;
      usedVertices.SetSize (vertexData.positions.GetSize ());

      for (size_t i = 0; i < oldSize; i++)
      {
        csArray<FactoryPrimitiveArray> newPrims;
        csRef<LightmapUVObjectLayouter> lightmaplayout = 
          uvlayout->LayoutFactory (unlayoutedPrimitives[i], vertexData, this, 
          newPrims, usedVertices, noModify);
        if (lightmaplayout)
        {
	  for (size_t n = 0; n < newPrims.GetSize(); n++)
	  {
	    layoutedPrimitives.Push (LayoutedPrimitives (newPrims[n],
	      lightmaplayout, n));
  
	    AddSubmeshRemap (i, layoutedPrimitives.GetSize () - 1);
	  }
	}
      }
      unlayoutedPrimitives.DeleteAll();
    }
    FinishSubmeshRemap ();

    return true;
  }

  csPtr<Object> ObjectFactory::CreateObject ()
  {
    return csPtr<Object> (new Object (this));
  }

  void ObjectFactory::ParseFactory (iMeshFactoryWrapper *factory)
  {
    this->factoryWrapper = factory;
    // Get the name
    this->factoryName = factoryWrapper->QueryObject ()->GetName ();   


    csRef<iObjectIterator> objiter = 
      factoryWrapper->QueryObject ()->GetIterator();
    while (objiter->HasNext())
    {
      iObject* obj = objiter->Next();
      csRef<iKeyValuePair> kvp = 
        scfQueryInterface<iKeyValuePair> (obj);
      if (kvp.IsValid() && (strcmp (kvp->GetKey(), "lighter2") == 0))
      {
        const char* vVertexlight = kvp->GetValue ("vertexlight");
        if (vVertexlight != 0)
          lightPerVertex = (strcmp (vVertexlight, "yes") == 0);

        const char* vNoSelfShadow = kvp->GetValue ("noselfshadow");
        if (vNoSelfShadow != 0)
        {
          noSelfShadow = (strcmp (vNoSelfShadow, "yes") == 0);
	}

        const char* vLMScale = kvp->GetValue ("lmscale");
        if (vLMScale)
        {
          float s=0;
          if (csScanStr (vLMScale, "%f", &s) == 1)
          {
            lmScale = s;
          }
        }
      }
    }
  }

  void ObjectFactory::SaveFactory (iDocumentNode *node)
  {
    // Save out the factory to the node
    csRef<iSaverPlugin> saver = 
      csQueryPluginClass<iSaverPlugin> (globalLighter->pluginManager,
      saverPluginName);      
    if (!saver) 
      saver = csLoadPlugin<iSaverPlugin> (globalLighter->pluginManager,
      saverPluginName);
    if (saver) 
    {
      // Write new mesh
      csRef<iDocumentNode> paramChild = node->GetNode ("params");
      saver->WriteDown(factoryWrapper->GetMeshObjectFactory (), node,
        0/*ssource*/);
      if (paramChild) 
      {
        // Move all nodes after the old params node to after the new params node
        csRef<iDocumentNodeIterator> nodes = node->GetNodes();
        while (nodes->HasNext())
        {
          csRef<iDocumentNode> child = nodes->Next();
          if (child->Equals (paramChild)) break;
        }
        // Skip <params>
        if (nodes->HasNext()) 
        {
          // Actual moving
          while (nodes->HasNext())
          {
            csRef<iDocumentNode> child = nodes->Next();
            if ((child->GetType() == CS_NODE_ELEMENT)
              && (strcmp (child->GetValue(), "params") == 0))
              break;
            csRef<iDocumentNode> newNode = node->CreateNodeBefore (
              child->GetType(), 0);
            CS::DocSystem::CloneNode (child, newNode);
            node->RemoveNode (child);
          }
        }
        node->RemoveNode (paramChild);
      }
    }
  }

  csString ObjectFactory::GetFileName() const
  {
    csString filename (factoryName);
    filename.ReplaceAll ("\\", "_");
    filename.ReplaceAll ("/", "_"); 
    filename.ReplaceAll (" ", "_"); 
    filename.ReplaceAll (".", "_"); 
    return filename;
  }
  
  csPtr<iRenderBuffer> ObjectFactory::WrapBuffer (iRenderBuffer* buffer, 
                                                  const char* suffix)
  {
    return lighter::WrapBuffer (buffer, suffix, GetFileName());
  }
  
  //-------------------------------------------------------------------------
  
  const char* const Object::lightmapTextures[] =
  {
    "tex lightmap",
    "tex lightmap dir 1",
    "tex lightmap dir 2",
    "tex lightmap dir 3",
    "tex spec directions 1",
    "tex spec directions 2",
    "tex spec directions 3",
    nullptr
  };

  Object::Object (ObjectFactory* fact)
    : lightPerVertex (fact->lightPerVertex), sector (0), litColors (0), 
      litColorsPD (0), factory (fact), lightInfluences (0)
  {
    if (factory->noSelfShadow)
      objFlags.Set (OBJECT_FLAG_NOSELFSHADOW);
  }
  
  Object::~Object ()
  {
    delete[] litColors;
    delete[] litColorsPD;
    delete lightInfluences;
  }

  bool Object::Initialize (Sector* sector)
  {
    if (!factory || !meshWrapper) return false;

    if (factory->hasTangents)
    {
      objFlags.Set (OBJECT_FLAG_TANGENTS);
      vdataBitangents = factory->vdataBitangents;
      vdataTangents = factory->vdataTangents;
    }

    this->sector = sector;

    const csReversibleTransform transform = meshWrapper->GetMovable ()->
      GetFullTransform ();
    objectToWorld = transform;

    //Copy over data, transform the radprimitives..
    vertexData = factory->vertexData;
    vertexData.Transform (transform);
    if (objFlags.Check (OBJECT_FLAG_TANGENTS))
    {
      for(size_t i = 0; i < vertexData.positions.GetSize (); ++i)
      {
        csVector3& tang = 
          *((csVector3*)vertexData.GetCustomData (i, vdataTangents));
        tang = transform.This2OtherRelative (tang);
        csVector3& bitang = 
          *((csVector3*)vertexData.GetCustomData (i, vdataBitangents));
        bitang = transform.This2OtherRelative (bitang);
      }
      
    }
    ComputeBoundingSphere ();

    if (!objFlags.Check (OBJECT_FLAG_NOLIGHT))
    {
      const LightRefArray& allPDLights = sector->allPDLights;
      csBitArray pdBits;
      pdBits.SetSize (allPDLights.GetSize());
      for (size_t i = 0; i < allPDLights.GetSize(); i++)
      {
	if (bsphere.TestIntersect (allPDLights[i]->GetBoundingSphere()))
	  pdBits.SetBit (i);
      }

      unsigned int i = 0;
      this->allPrimitives.SetCapacity (factory->layoutedPrimitives.GetSize ());
      for(size_t j = 0; j < factory->layoutedPrimitives.GetSize (); ++j)
      {
	FactoryPrimitiveArray& factPrims = factory->layoutedPrimitives[j].primitives;
	PrimitiveArray& allPrimitives =
	  this->allPrimitives.GetExtend (j);

	allPrimitives.SetCapacity (allPrimitives.GetSize() + factPrims.GetSize());
	for (i = 0; i < factPrims.GetSize(); i++)
	{
	  Primitive newPrim (vertexData, (uint)j);
	  
	  Primitive& prim = allPrimitives[allPrimitives.Push (newPrim)];
	  //prim.SetOriginalPrimitive (&factPrims[i]);
	  prim.SetTriangle (factPrims[i].GetTriangle ()); 
	  prim.ComputePlane ();
	}

	if (!lightPerVertex)
	{
	  // FIXME: probably separate out to allow for better progress display
	  LightmapUVObjectLayouter* layout = 
	    factory->layoutedPrimitives[j].factory;
	  const size_t group = factory->layoutedPrimitives[j].group;
	  size_t layoutID = layout->LayoutUVOnPrimitives (allPrimitives, 
	    group, sector, pdBits);
	  if (layoutID == (size_t)~0) return false;
	  lmLayouts.Push (LMLayoutingInfo (layout, layoutID, group));
	}

      }
    }

    factory.Invalidate();

    return true;
  }

  void Object::PrepareLighting ()
  {
    for (size_t j = 0; j < this->allPrimitives.GetSize(); j++)
    {
      PrimitiveArray& allPrimitives = this->allPrimitives[j];
      allPrimitives.ShrinkBestFit ();

      if (!lightPerVertex)
      {
        lmLayouts[j].layouter->FinalLightmapLayout (allPrimitives, 
          lmLayouts[j].layoutID, lmLayouts[j].group, vertexData, 
          lightmapIDs.GetExtend (j));
      }

      for (size_t i = 0; i < allPrimitives.GetSize(); i++)
      {
        Primitive& prim = allPrimitives[i];
        prim.ComputeUVTransform ();
        prim.SetObject (this);
        prim.Prepare ();
      }
    }
    lmLayouts.DeleteAll ();

    if (lightPerVertex)
    {
      size_t n = globalConfig.GetLighterProperties().directionalLMs ? 4 : 1;

      litColors = new LitColorArray[n];
      for (size_t i = 0; i < n; i++)
        litColors[i].SetSize (vertexData.positions.GetSize(), 
          csColor (0.0f, 0.0f, 0.0f));
      litColorsPD = new LitColorsPDHash[n];
    }
  }

  void Object::StripLightmaps (csSet<csString>& lms)
  {
    iShaderVariableContext* svc = meshWrapper->GetSVContext();
    for (const char* const* lmtn = lightmapTextures; *lmtn; lmtn++)
    {
      csShaderVariable* sv = svc->GetVariable (
        globalLighter->svStrings->Request (*lmtn));
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

  void Object::ParseMesh (iMeshWrapper *wrapper)
  {
    this->meshWrapper = wrapper;
    this->meshName = wrapper->QueryObject ()->GetName ();

    const csFlags& meshFlags = wrapper->GetFlags ();
    if (meshFlags.Check (CS_ENTITY_NOSHADOWS))
      objFlags.Set (OBJECT_FLAG_NOSHADOW);
    if (meshFlags.Check (CS_ENTITY_NOLIGHTING)
	|| factory->factoryWrapper->GetFlags().Check (CS_ENTITY_NOLIGHTING))
      objFlags.Set (OBJECT_FLAG_NOLIGHT);

    if (globalLighter->rayDebug.EnableForMesh (meshName))
      objFlags.Set (OBJECT_FLAG_RAYDEBUG);

    csRef<iObjectIterator> objiter = 
      wrapper->QueryObject ()->GetIterator();
    while (objiter->HasNext())
    {
      iObject* obj = objiter->Next();
      csRef<iKeyValuePair> kvp = 
        scfQueryInterface<iKeyValuePair> (obj);
      if (kvp.IsValid() && (strcmp (kvp->GetKey(), "lighter2") == 0))
      {
        const char* vNoSelfShadow = kvp->GetValue ("noselfshadow");
        if (vNoSelfShadow != 0)
        {
          objFlags.SetBool (OBJECT_FLAG_NOSELFSHADOW,
            strcmp (vNoSelfShadow, "yes") == 0);
	}

        if (!factory->lightPerVertex && !objFlags.Check (OBJECT_FLAG_NOLIGHT))
        {
          /* Disallow "disabling" of per-vertex lighting in an object when
           * it's enabled for the factory. */
          const char* vVertexlight = kvp->GetValue ("vertexlight");
          if (vVertexlight != 0)
            lightPerVertex = (strcmp (vVertexlight, "yes") == 0);
        }
      }
    }
  }

  void Object::SaveMesh (iDocumentNode* node)
  {
    // Save out the object to the node
    csRef<iSaverPlugin> saver = 
      csQueryPluginClass<iSaverPlugin> (globalLighter->pluginManager,
      saverPluginName);      
    if (!saver) 
      saver = csLoadPlugin<iSaverPlugin> (globalLighter->pluginManager,
      saverPluginName);
    if (saver) 
    {
      // Write new mesh
      csRef<iDocumentNode> paramChild = node->GetNode ("params");
      saver->WriteDown(meshWrapper->GetMeshObject (), node, 0/*ssource*/);
      if (paramChild) 
      {
        // Move all nodes after the old params node to after the new params node
        csRef<iDocumentNodeIterator> nodes = node->GetNodes();
        while (nodes->HasNext())
        {
          csRef<iDocumentNode> child = nodes->Next();
          if (child->Equals (paramChild)) break;
        }
        // Skip <params>
        if (nodes->HasNext()) 
        {
          // Actual moving
          while (nodes->HasNext())
          {
            csRef<iDocumentNode> child = nodes->Next();
            if ((child->GetType() == CS_NODE_ELEMENT)
              && (strcmp (child->GetValue(), "params") == 0))
              break;
            csRef<iDocumentNode> newNode = node->CreateNodeBefore (
              child->GetType(), 0);
            CS::DocSystem::CloneNode (child, newNode);
            node->RemoveNode (child);
          }
        }
        node->RemoveNode (paramChild);
      }
      // Add <staticlit> node
      bool hasStaticLit = false;
      {
        csRef<iDocumentNodeIterator> nodes = node->GetNodes();
        while (nodes->HasNext())
        {
	  csRef<iDocumentNode> child = nodes->Next();
	  if ((child->GetType() == CS_NODE_ELEMENT)
	    && (strcmp (child->GetValue(), "staticlit") == 0))
	  {
	    hasStaticLit = true;
	    break;
	  }
        }
        if (!hasStaticLit)
        {
	  csRef<iDocumentNode> newNode = node->CreateNodeBefore (
	    CS_NODE_ELEMENT, 0);
	  newNode->SetValue ("staticlit");
        }
      }
    }
  }

  void Object::FreeNotNeededForLighting ()
  {
    meshWrapper.Invalidate();
  }

  void Object::FillLightmapMask (LightmapMaskPtrDelArray& masks)
  {
    if (lightPerVertex || objFlags.Check (OBJECT_FLAG_NOLIGHT)) return;

    // And fill it with data
    for (size_t i = 0; i < allPrimitives.GetSize(); i++)
    {
      LightmapMask& mask = *(masks[lightmapIDs[i]]);
      ScopedSwapLock<LightmapMask> m (mask);
      float* maskData = mask.GetMaskData();

      PrimitiveArray::Iterator primIt = allPrimitives[i].GetIterator ();
      while (primIt.HasNext ())
      {
        const Primitive &prim = primIt.Next ();        

        int minu,maxu,minv,maxv;
        prim.ComputeMinMaxUV (minu,maxu,minv,maxv);
        uint findex = 0;

        // Go through all lightmap cells and add their element areas to the mask
        for (uint v = minv; v <= (uint)maxv;v++)
        {
          uint vindex = v * mask.GetWidth();
          for (uint u = minu; u <= (uint)maxu; u++, findex++)
          {            
            //@@TODO
            Primitive::ElementType type = prim.GetElementType (findex);
            if (type == Primitive::ELEMENT_EMPTY)
            {
              continue;
            }
            else if (type == Primitive::ELEMENT_BORDER)
            {
              maskData[vindex+u] += prim.ComputeElementFraction (findex);
            }
            else
            {
              maskData[vindex+u] += 1.0f;
            }
          }
        } 
      }
    }

  }

  Object::LitColorArray* Object::GetLitColorsPD (Light* light, size_t num)
  {
    LitColorArray* colors = litColorsPD[num].GetElementPointer (light);
    if (colors != 0) return colors;

    LitColorArray newArray;
    newArray.SetSize (litColors[num].GetSize(), csColor (0));
    return &(litColorsPD[num].Put (light, newArray));
  }

  csMatrix3 Object::ComputeTangentSpace (const Primitive* prim,
                                         const csVector3& pt) const
  {
    csVector3 normal (prim->ComputeNormal (pt));
    csVector3 tang, bitang;
    if (objFlags.Check (OBJECT_FLAG_TANGENTS))
    {
      tang = prim->ComputeCustomData<csVector3> (pt, vdataTangents).Unit();
      bitang = prim->ComputeCustomData<csVector3> (pt, vdataBitangents).Unit();
    }
    else
    {
      // Fake something up
      csVector3 right (1, 0, 0);
      if ((right * normal) > (1.0f - LITEPSILON))
        right = csVector3(0, 1, 0);
      bitang = normal % right;
      tang = normal % bitang;
    }
    return csMatrix3 (tang[0], bitang[0], normal[0],
                      tang[1], bitang[1], normal[1],
                      tang[2], bitang[2], normal[2]);
  }

  csMatrix3 Object::GetTangentSpace (size_t vert) const
  {
    csVector3 normal (vertexData.normals[vert]);
    csVector3 tang, bitang;
    if (objFlags.Check (OBJECT_FLAG_TANGENTS))
    {
      tang = *((csVector3*)vertexData.GetCustomData (vert, vdataTangents));
      bitang = *((csVector3*)vertexData.GetCustomData (vert, vdataBitangents));
    }
    else
    {
      // Fake something up
      csVector3 right (1, 0, 0);
      if ((right * normal) > (1.0f - LITEPSILON))
        right = csVector3(0, 1, 0);
      bitang = normal % right;
      tang = normal % bitang;
    }
    return csMatrix3 (tang[0], bitang[0], normal[0],
                      tang[1], bitang[1], normal[1],
                      tang[2], bitang[2], normal[2]);
  }
  
  LightInfluences& Object::GetLightInfluences (uint groupID, Light* light)
  {
    CS::Threading::ScopedLock<CS::Threading::Mutex> lock (lightInfluencesMutex);
    
    if (!lightInfluences)
      lightInfluences = new LightInfluencesHash;
  
    GroupAndLight key (light, groupID);
    csRef<LightInfluencesRC>& infl = lightInfluences->GetOrCreate (key);
    
    if (!infl.IsValid())
    {
      float minU = FLT_MAX, minV = FLT_MAX, maxU = -FLT_MAX, maxV = -FLT_MAX;
      const PrimitiveArray& groupPrims = allPrimitives[groupID];
      for (size_t p = 0; p < groupPrims.GetSize(); p++)
      {
        const Primitive& prim = groupPrims[p];
        CS_ASSERT(prim.GetGroupID() == groupID);
        
        const csVector2& primMinUV = prim.GetMinUV();
        if (primMinUV.x < minU) minU = primMinUV.x;
        if (primMinUV.y < minV) minV = primMinUV.y;
        
        const csVector2& primMaxUV = prim.GetMaxUV();
        if (primMaxUV.x > maxU) maxU = primMaxUV.x;
        if (primMaxUV.y > maxV) maxV = primMaxUV.y;
      }
      
      uint inflOffsX = uint (floorf (minU));
      uint inflOffsY = uint (floorf (minV));
      uint inflW = uint (ceilf (maxU)) - inflOffsX;
      uint inflH = uint (ceilf (maxV)) - inflOffsY;
      infl.AttachNew (
        new LightInfluencesRC (inflW, inflH, inflOffsX, inflOffsY));
    }
    
    return *infl;
  }

  csArray<Light*> Object::GetLightsAffectingGroup (uint groupID) const
  {
    csArray<Light*> lights;
    if (lightInfluences)
    {
      csSet<csPtrKey<Light> > lightsSeen;
      
      LightInfluencesHash::GlobalIterator inflIt = 
	lightInfluences->GetIterator();
      
      while (inflIt.HasNext())
      {
	GroupAndLight key (0, 0);
	inflIt.Next (key);
	if ((key.groupID == groupID) && !lightsSeen.Contains (key.light))
	{
	  lights.Push (key.light);
	  lightsSeen.AddNoTest (key.light);
	}
      }
    }
    
    return lights;
  }
  
  void Object::RenormalizeLightmapUVs (const LightmapPtrDelArray& lightmaps,
                                       csVector2* lmcoords)
  {
    if (lightPerVertex) return;

    BoolArray vertexProcessed;
    vertexProcessed.SetSize (vertexData.lightmapUVs.GetSize (), false);

    for (size_t p = 0; p < allPrimitives.GetSize (); p++)
    {
      const Lightmap* lm = lightmaps[lightmapIDs[p]];
      const float factorX = 1.0f / lm->GetWidth ();
      const float factorY = 1.0f / lm->GetHeight ();
      const PrimitiveArray& prims = allPrimitives[p];
      csSet<size_t> indicesRemapped;
      // Iterate over lightmaps and renormalize UVs
      for (size_t j = 0; j < prims.GetSize (); ++j)
      {
        //TODO make sure no vertex is used in several lightmaps.. 
        const Primitive &prim = prims.Get (j);
        const Primitive::TriangleType& t = prim.GetTriangle ();
        for (size_t i = 0; i < 3; ++i)
        {
          size_t index = t[i];
          if (!indicesRemapped.Contains (index))
          {
            const csVector2 &lmUV = vertexData.lightmapUVs[index];
            csVector2& outUV = lmcoords[index];
            outUV.x = (lmUV.x) * factorX;
            outUV.y = (lmUV.y) * factorY;
            indicesRemapped.AddNoTest (index);
          }
        }
      }
    }
  }

  void Object::ComputeBoundingSphere ()
  {
    if (vertexData.positions.GetSize() == 0)
    {
      bsphere.SetCenter (csVector3 (0));
      bsphere.SetRadius (0);
      return;
    }

    csBox3 bbox (vertexData.positions[0]);
    for (size_t p = 1; p < vertexData.positions.GetSize(); p++)
    {
      bbox.AddBoundingVertexSmart (vertexData.positions[p]);
    }

    bsphere.SetCenter (bbox.GetCenter());
    bsphere.SetRadius (sqrtf (bbox.SquaredPosMaxDist (bsphere.GetCenter())));
  }

  csString Object::GetFileName() const
  {
    csString filename (meshName);
    filename.ReplaceAll ("\\", "_");
    filename.ReplaceAll ("/", "_"); 
    filename.ReplaceAll (" ", "_"); 
    filename.ReplaceAll (".", "_"); 
    return filename;
  }
  
  csPtr<iRenderBuffer> Object::WrapBuffer (iRenderBuffer* buffer, 
                                           const char* suffix)
  {
    return lighter::WrapBuffer (buffer, suffix, GetFileName());
  }

}
