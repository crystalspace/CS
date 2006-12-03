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

#include "common.h"
#include "lighter.h"
#include "lightmap.h"
#include "lightmapuv.h"
#include "object.h"
#include "config.h"

// Debugging: uncomment to disable border smoothing
//#define NOSMOOTH

namespace lighter
{

  ObjectFactory::ObjectFactory ()
    : lightmapMaskArrayValid (false), factoryWrapper (0), 
      lightPerVertex (false)
  {
  }

  bool ObjectFactory::PrepareLightmapUV (LightmapUVFactoryLayouter* uvlayout)
  {
    BeginSubmeshRemap ();
    size_t oldSize = unlayoutedPrimitives.GetSize();
    for (size_t i = 0; i < oldSize; i++)
    {
      csArray<PrimitiveArray> newPrims;
      csRef<LightmapUVObjectLayouter> lightmaplayout = 
        uvlayout->LayoutFactory (unlayoutedPrimitives[i], vertexData, newPrims);
      if (!lightmaplayout) return false;

      for (size_t n = 0; n < newPrims.GetSize(); n++)
      {
        layoutedPrimitives.Push (LayoutedPrimitives (newPrims[n],
          lightmaplayout, n));

        AddSubmeshRemap (i, layoutedPrimitives.GetSize () - 1);
      }
    }
    unlayoutedPrimitives.DeleteAll();
    FinishSubmeshRemap ();

    return true;
  }

  Object* ObjectFactory::CreateObject ()
  {
    return new Object (this);
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
      if (kvp.IsValid())
      {
        const char* vVertexlight = kvp->GetValue ("vertexlight");
        if (vVertexlight != 0)
          lightPerVertex = (strcmp (vVertexlight, "yes") == 0);
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
            CS::DocumentHelper::CloneNode (child, newNode);
            node->RemoveNode (child);
          }
        }
        node->RemoveNode (paramChild);
      }
    }
  }

  //-------------------------------------------------------------------------

  Object::Object (ObjectFactory* fact)
    : factory (fact), lightPerVertex (fact->lightPerVertex), litColors (0)
  {
  }
  
  Object::~Object ()
  {
    delete litColors;
  }

  bool Object::Initialize ()
  {
    if (!factory || !meshWrapper) return false;
    const csReversibleTransform transform = meshWrapper->GetMovable ()->
      GetFullTransform ();

    //Copy over data, transform the radprimitives..
    vertexData = factory->vertexData;
    
    vertexData.Transform (transform);

    unsigned int i = 0;
    for(size_t j = 0; j < factory->layoutedPrimitives.GetSize (); ++j)
    {
      PrimitiveArray& factPrims = factory->layoutedPrimitives[j].primitives;
      PrimitiveArray& allPrimitives =
        this->allPrimitives.GetExtend (j);
      for (i = 0; i < factPrims.GetSize(); i++)
      {
        Primitive newPrim (vertexData);
        
        Primitive& prim = allPrimitives[allPrimitives.Push (newPrim)];
        prim.SetOriginalPrimitive (&factPrims[i]);
        prim.SetTriangle (factPrims[i].GetTriangle ()); 
        prim.ComputePlane ();
      }

      if (!lightPerVertex)
      {
        // FIXME: probably separate out to allow for better progress display
        bool res = 
          factory->layoutedPrimitives[j].factory->LayoutUVOnPrimitives (
          allPrimitives, factory->layoutedPrimitives[j].group, vertexData, 
          lightmapIDs.GetExtend (j));
        if (!res) return false;
      }

      for (i = 0; i < allPrimitives.GetSize(); i++)
      {
        Primitive& prim = allPrimitives[i];
        prim.ComputeUVTransform ();
        prim.SetObject (this);
        prim.Prepare ();
      }
    }

    if (lightPerVertex)
    {
      litColors = new LitColorArray();
      litColors->SetSize (vertexData.vertexArray.GetSize(), 
        csColor (0.0f, 0.0f, 0.0f));
    }

    return true;
  }

  void Object::RenormalizeLightmapUVs (const LightmapPtrDelArray& lightmaps)
  {
    if (lightPerVertex) return;

    BoolArray vertexProcessed;
    vertexProcessed.SetSize (vertexData.vertexArray.GetSize (),false);

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
            csVector2 &lmUV = vertexData.vertexArray[index].lightmapUV;
            lmUV.x = (lmUV.x + 0.5f) * factorX;
            lmUV.y = (lmUV.y + 0.5f) * factorY;
            indicesRemapped.AddNoTest (index);
          }
        }
      }
    }
  }

  void Object::StripLightmaps (csSet<csString>& lms)
  {
    iShaderVariableContext* svc = meshWrapper->GetSVContext();
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

  void Object::ParseMesh (iMeshWrapper *wrapper)
  {
    this->meshWrapper = wrapper;
    this->meshName = wrapper->QueryObject ()->GetName ();
    csRef<iObjectIterator> objiter = 
      wrapper->QueryObject ()->GetIterator();
    while (objiter->HasNext())
    {
      iObject* obj = objiter->Next();
      csRef<iKeyValuePair> kvp = 
        scfQueryInterface<iKeyValuePair> (obj);
      if (kvp.IsValid())
      {
        const char* vVertexlight = kvp->GetValue ("vertexlight");
        if (vVertexlight != 0)
          lightPerVertex = (strcmp (vVertexlight, "yes") == 0);
      }
    }
  }

  void Object::SaveMesh (Scene* /*scene*/, iDocumentNode* node)
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
            CS::DocumentHelper::CloneNode (child, newNode);
            node->RemoveNode (child);
          }
        }
        node->RemoveNode (paramChild);
      }
    }
  }

  void Object::FixupLightmaps (csArray<LightmapPtrDelArray*>& lightmaps)
  {
    if (lightPerVertex) return;

    //Create one, @@TODO: optimise this
    LightmapMaskArray masks;
    LightmapPtrDelArray::Iterator lmIt = lightmaps[0]->GetIterator ();
    while (lmIt.HasNext ())
    {
      const Lightmap* lm = lmIt.Next ();
      masks.Push (LightmapMask (*lm));
    }

    float totalArea = 0;

    // And fill it with data
    for (size_t i = 0; i < allPrimitives.GetSize(); i++)
    {
      LightmapMask &mask = masks[lightmapIDs[i]];
      PrimitiveArray::Iterator primIt = allPrimitives[i].GetIterator ();
      while (primIt.HasNext ())
      {
        const Primitive &prim = primIt.Next ();
        totalArea = (prim.GetuFormVector ()%prim.GetvFormVector ()).Norm ();
        float area2pixel = 1.0f / totalArea;

        int minu,maxu,minv,maxv;
        prim.ComputeMinMaxUV (minu,maxu,minv,maxv);
        uint findex = 0;

        // Go through all lightmap cells and add their element areas to the mask
        for (uint v = minv; v <= (uint)maxv;v++)
        {
          uint vindex = v * mask.width;
          for (uint u = minu; u <= (uint)maxu; u++, findex++)
          {
            const float elemArea = prim.GetElementAreas ()[findex];
            if (elemArea < FLT_EPSILON) continue; // No area, skip

            mask.maskData[vindex+u] += elemArea * area2pixel; //Accumulate
          }
        } 
      }
    }

    // Ok, here we are sure to have a mask
    
    // Un-antialias
    uint i;
#ifndef DUMP_NORMALS
    for (size_t l = 0; l < lightmaps.GetSize (); l++)
    {
      for (i = 0; i < lightmaps[l]->GetSize(); i++)
      {
        csColor* lmData = lightmaps[l]->Get (i)->GetData ().GetArray ();
        const float* mmData = masks[i].maskData.GetArray ();
        const size_t size = lightmaps[l]->Get (i)->GetData ().GetSize ();

        for (uint j = 0; j < size; j++, lmData++, mmData++)
        {
          if (*mmData < FLT_EPSILON || *mmData >= 1.0f) continue;

          *lmData *= (1.0f / *mmData);
        }
      }
    }
#endif

#ifndef NOSMOOTH
    // Do the filtering
    for (size_t l = 0; l < lightmaps.GetSize (); l++)
    {
      for (i = 0; i < lightmaps[l]->GetSize(); i++)
      {
        csColor* lmData = lightmaps[l]->Get (i)->GetData ().GetArray ();
        const float* mmData = masks[i].maskData.GetArray ();
              
        uint lmw = lightmaps[l]->Get (i)->GetWidth ();
        uint lmh = lightmaps[l]->Get (i)->GetHeight ();
        
        for (uint v = 0; v < lmh; v++)
        {
          // now scan over the row
          for (uint u = 0; u < lmw; u++)
          {
            const uint idx = v*lmw+u;
            
            // Only try to fix non-masked
            if (mmData[idx]>0) continue;

            uint count = 0;
            csColor newColor (0.0f,0.0f,0.0f);

            // We have a row above to use
            if (v > 0)
            {
              // We have a column to the left
              if (u > 0 && mmData[(v-1)*lmw+(u-1)] > FLT_EPSILON) newColor += lmData[(v-1)*lmw+(u-1)], count++;
              if (mmData[(v-1)*lmw+(u)] > FLT_EPSILON) newColor += lmData[(v-1)*lmw+(u)], count++;
              if (u < lmw-1 && mmData[(v-1)*lmw+(u+1)] > FLT_EPSILON) newColor += lmData[(v-1)*lmw+(u+1)], count++;
            }

            //current row
            if (u > 0 && mmData[v*lmw+(u-1)] > FLT_EPSILON) newColor += lmData[v*lmw+(u-1)], count++;
            if (u < lmw-1 && mmData[v*lmw+(u+1)] > FLT_EPSILON) newColor += lmData[v*lmw+(u+1)], count++;

            // We have a row below
            if (v < (lmh-1))
            {
              if (u > 0 && mmData[(v+1)*lmw+(u-1)] > FLT_EPSILON) newColor += lmData[(v+1)*lmw+(u-1)], count++;
              if (mmData[(v+1)*lmw+(u)] > FLT_EPSILON) newColor += lmData[(v+1)*lmw+(u)], count++;
              if (u < lmw-1 && mmData[(v+1)*lmw+(u+1)] > FLT_EPSILON) newColor += lmData[(v+1)*lmw+(u+1)], count++;
            }

            if (count > 0) 
            {
#ifndef DUMP_NORMALS
              newColor *= (1.0f/count);
#else
              csVector3 v (
                newColor.red*2.0f-float (count), 
                newColor.green*2.0f-float (count), 
                newColor.blue*2.0f-float (count));
              v.Normalize();
              newColor.Set (v.x*0.5f+0.5f, v.y*0.5f+0.5f, v.z*0.5f+0.5f);
#endif
              lmData[idx] = newColor;
            }
          }
        }
      }
    }
#endif // NOSMOOTH
  }

}
