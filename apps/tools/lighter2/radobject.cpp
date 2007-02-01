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
#include "radobject.h"
#include "config.h"

namespace lighter
{

  RadObjectFactory::RadObjectFactory ()
    : lightmapMaskArrayValid (false), factoryWrapper (0)
  {
  }

  bool RadObjectFactory::PrepareLightmapUV (LightmapUVLayouter* uvlayout)
  {
    size_t oldSize = allPrimitives.GetSize();
    for (size_t i = 0; i < oldSize; i++)
    {
      csArray<RadPrimitiveArray> newPrims;
      LightmapUVLayoutFactory* lightmaplayout = 
        uvlayout->LayoutFactory (allPrimitives[i], vertexData, newPrims);
      if (!lightmaplayout) return false;

      for (size_t n = 0; n < newPrims.GetSize(); n++)
      {
        allPrimitives.Push (newPrims[n]);
        lightmaplayouts.Push (lightmaplayout);
        lightmaplayoutGroups.Push (n);
      }
    }
    while (oldSize-- > 0) allPrimitives.DeleteIndexFast (oldSize);

    return true;
  }

  RadObject* RadObjectFactory::CreateObject ()
  {
    return new RadObject (this);
  }

  void RadObjectFactory::ParseFactory (iMeshFactoryWrapper *factory)
  {
    this->factoryWrapper = factory;
    // Get the name
    this->factoryName = factoryWrapper->QueryObject ()->GetName ();
  }

  void RadObjectFactory::SaveFactory (iDocumentNode *node)
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
      // Make sure to remove old params node
      csRef<iDocumentNode> paramChild = node->GetNode ("params");
      if (paramChild) node->RemoveNode (paramChild);
      saver->WriteDown(factoryWrapper->GetMeshObjectFactory (), node,
      	0/*ssource*/);
    }
  }

  //-------------------------------------------------------------------------

  RadObject::RadObject (RadObjectFactory* fact)
    : factory (fact)
  {
  }
  
  bool RadObject::Initialize ()
  {
    if (!factory || !meshWrapper) return false;
    const csReversibleTransform transform = meshWrapper->GetMovable ()->
      GetFullTransform ();

    //Copy over data, transform the radprimitives..
    vertexData = factory->vertexData;
    
    vertexData.Transform (transform);

    unsigned int i = 0;
    for(size_t j = 0; j < factory->allPrimitives.GetSize (); ++j)
    {
      RadPrimitiveArray& factPrims = factory->allPrimitives[j];
      RadPrimitiveArray& allPrimitives =
        this->allPrimitives.GetExtend (j);
      for (i = 0; i < factPrims.GetSize(); i++)
      {
        RadPrimitive newPrim (vertexData);
        
        RadPrimitive& prim = allPrimitives[allPrimitives.Push (newPrim)];
        prim.SetOriginalPrimitive (&factPrims[i]);
        prim.GetIndexArray () = factPrims[i].GetIndexArray ();
        prim.ComputePlane ();
      }

      // FIXME: probably separate out to allow for better progress display
      bool res = factory->lightmaplayouts[j]->LayoutUVOnPrimitives (
        allPrimitives, factory->lightmaplayoutGroups[j], vertexData, 
        lightmapIDs.GetExtend (j));
      if (!res) return false;

      for (i = 0; i < allPrimitives.GetSize(); i++)
      {
        RadPrimitive& prim = allPrimitives[i];
        prim.ComputeUVTransform ();
        prim.SetRadObject (this);
        prim.Prepare (
          globalConfig.GetRadProperties ().uPatchResolution, 
          globalConfig.GetRadProperties ().vPatchResolution);
      }
    }

    return true;
  }

  void RadObject::RenormalizeLightmapUVs (const LightmapPtrDelArray& lightmaps)
  {
    BoolArray vertexProcessed;
    vertexProcessed.SetSize (vertexData.vertexArray.GetSize (),false);

    for (size_t p = 0; p < allPrimitives.GetSize (); p++)
    {
      const Lightmap* lm = lightmaps[lightmapIDs[p]];
      const RadPrimitiveArray& prims = allPrimitives[p];
      csSet<size_t> indicesRemapped;
      // Iterate over lightmaps and renormalize UVs
      for (size_t j = 0; j < prims.GetSize (); ++j)
      {
        //TODO make sure no vertex is used in several lightmaps.. 
        const RadPrimitive &prim = prims.Get (j);
        const SizeTDArray &indexArray = prim.GetIndexArray ();
        for (size_t i = 0; i < indexArray.GetSize (); ++i)
        {
          size_t index = indexArray[i];
          if (!indicesRemapped.Contains (index))
          {
            csVector2 &lmUV = vertexData.vertexArray[index].lightmapUV;
            lmUV.x /= lm->GetWidth ();
            lmUV.y /= lm->GetHeight ();
            indicesRemapped.AddNoTest (index);
          }
        }
      }
    }
  }

  void RadObject::StripLightmaps (csSet<csString>& lms)
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

  void RadObject::ParseMesh (iMeshWrapper *wrapper)
  {
    this->meshWrapper = wrapper;
    this->meshName = wrapper->QueryObject ()->GetName ();
  }

  static void CloneNode (iDocumentNode* from, iDocumentNode* to)
  {
    to->SetValue (from->GetValue ());
    csRef<iDocumentNodeIterator> it = from->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      csRef<iDocumentNode> child_clone = to->CreateNodeBefore (
    	  child->GetType (), 0);
      CloneNode (child, child_clone);
    }
    csRef<iDocumentAttributeIterator> atit = from->GetAttributes ();
    while (atit->HasNext ())
    {
      csRef<iDocumentAttribute> attr = atit->Next ();
      to->SetAttribute (attr->GetName (), attr->GetValue ());
    }
  }

  void RadObject::SaveMesh (Scene* /*scene*/, iDocumentNode* node)
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
            CloneNode (child, newNode);
            node->RemoveNode (child);
          }
        }
        node->RemoveNode (paramChild);
      }
    }
  }

  void RadObject::FixupLightmaps (csArray<LightmapPtrDelArray*>& lightmaps)
  {
    //Create one
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
      RadPrimitiveArray::Iterator primIt = allPrimitives[i].GetIterator ();
      while (primIt.HasNext ())
      {
        const RadPrimitive &prim = primIt.Next ();
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
              newColor *= (1.0f/count);
              lmData[idx] = newColor;
            }
          }
        }
      }
    }
  }

}
