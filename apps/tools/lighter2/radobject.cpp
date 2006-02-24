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

namespace lighter
{

  RadObjectFactory::RadObjectFactory ()
    : lightmapMaskArrayValid (false), factoryWrapper (0)
  {
  }

  RadObject* RadObjectFactory::CreateObject ()
  {
    return new RadObject (this);
  }

  bool RadObjectFactory::ComputeLightmapUV (LightmapUVLayouter* layoutEngine)
  {
    // Default implementation
    bool res = layoutEngine->LayoutUVOnPrimitives (allPrimitives, vertexData,
      lightmapTemplates);
    if (!res) return false;

    return true;
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

  void RadObjectFactory::RenormalizeLightmapUVs ()
  {
    BoolArray vertexProcessed;
    vertexProcessed.SetSize (vertexData.vertexArray.GetSize (),false);

    // Iterate over lightmaps and renormalize UVs
    for (size_t j = 0; j < allPrimitives.GetSize (); ++j)
    {
      //TODO make sure no vertex is used in several lightmaps.. 
      const RadPrimitive &prim = allPrimitives.Get (j);
      const SizeTDArray &indexArray = prim.GetIndexArray ();
      const Lightmap* lm= lightmapTemplates[prim.GetLightmapID ()];
      for (size_t i = 0; i < indexArray.GetSize (); ++i)
      {
        csVector2 &lmUV = vertexData.vertexArray[indexArray[i]].lightmapUV;
        lmUV.x /= lm->GetWidth ();
        lmUV.y /= lm->GetHeight ();
      }
    }
  }


  RadObject::RadObject (RadObjectFactory* fact)
    : factory (fact)
  {
  }
  
  void RadObject::Initialize ()
  {
    if (!factory || !meshWrapper) return;
    const csReversibleTransform transform = meshWrapper->GetMovable ()->
      GetFullTransform ();

    //Copy over data, transform the radprimitives..
    vertexData = factory->vertexData;
    
    vertexData.Transform (transform);

    unsigned int i = 0;
    for(i = 0; i < factory->allPrimitives.GetSize (); ++i)
    {
      RadPrimitive newPrim (vertexData);
      
      RadPrimitive& prim = allPrimitives[allPrimitives.Push (newPrim)];
      prim.SetOriginalPrimitive (&factory->allPrimitives[i]);
      prim.GetIndexArray () = factory->allPrimitives[i].GetIndexArray ();
      prim.ComputePlane ();
      prim.ComputeUVTransform ();
      prim.SetRadObject (this);
      prim.Prepare (globalSettings.uPatchResolution, globalSettings.vPatchResolution);
    }

    // Create and init lightmaps
    for (i = 0; i < factory->lightmapTemplates.GetSize (); ++i)
    {
      Lightmap *lm = new Lightmap (*(factory->lightmapTemplates.Get (i)));
      lm->Initialize ();
      lightmaps.Push (lm);
    }
  }

  void RadObject::ParseMesh (iMeshWrapper *wrapper)
  {
    this->meshWrapper = wrapper;
    this->meshName = wrapper->QueryObject ()->GetName ();
  }

  void RadObject::SaveMesh (iDocumentNode* /*node*/)
  {

  }

  void RadObject::FixupLightmaps ()
  {
    //Create one
    LightmapMaskArray masks;
    LightmapPtrDelArray::Iterator lmIt = lightmaps.GetIterator ();
    while (lmIt.HasNext ())
    {
      masks.Push (LightmapMask (*(lmIt.Next ())));
    }

    float totalArea = 0;

    // And fill it with data
    RadPrimitiveArray::Iterator primIt = allPrimitives.GetIterator ();
    while (primIt.HasNext ())
    {
      const RadPrimitive &prim = primIt.Next ();
      LightmapMask &mask = masks[prim.GetLightmapID ()];
      totalArea = (prim.GetuFormVector ()%prim.GetvFormVector ()).Norm ();

      int minu,maxu,minv,maxv;
      prim.ComputeMinMaxUV (minu,maxu,minv,maxv);
      uint findex = 0;

      // Go through all lightmap cells and add their element areas to the mask
      for (uint v = minv; v <= (uint)maxv;v++)
      {
        uint vindex = v * mask.width;
        for (uint u = minu; u <= (uint)maxu; u++, findex++)
        {
          if (prim.GetElementAreas ()[findex] < FLT_EPSILON) continue; // No area, skip

          mask.maskData[vindex+u] += prim.GetElementAreas ()[findex]; //Accumulate
        }

      }
    }

    // Ok, here we are sure to have a mask
    
    // Un-antialis
    uint i;
    for (i = 0; i < lightmaps.GetSize (); i++)
    {
      csColor* lmData = lightmaps[i]->GetData ().GetArray ();
      float* mmData = masks[i].maskData.GetArray ();
      const size_t size = lightmaps[i]->GetData ().GetSize ();

      for (uint j = 0; j < size; j++, lmData++, mmData++)
      {
        if (*mmData < FLT_EPSILON || *mmData >= totalArea) continue;

        *lmData *= (totalArea / *mmData);
      }
    }

    // Do the filtering
    for (i = 0; i < lightmaps.GetSize (); i++)
    {
      csColor* lmData = lightmaps[i]->GetData ().GetArray ();
      float* mmData = masks[i].maskData.GetArray ();
            
      uint lmw = lightmaps[i]->GetWidth ();
      uint lmh = lightmaps[i]->GetHeight ();

      
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

          if (count > 0) newColor *= (1.0f/count);
          else newColor.Set (0.0f, 0.0f, 0.0f);
          lmData[idx] = newColor;
        }
      }
    }
  }

  //////////////////////////////////////////////////////////////////////////
  // SPECIFIC VERISONS
  //////////////////////////////////////////////////////////////////////////
  
  //////////////   Genmesh     /////////////////////////////////////////////
  
  RadObjectFactory_Genmesh::RadObjectFactory_Genmesh()
    : normals (0)
  {
    saverPluginName = "crystalspace.mesh.saver.factory.genmesh";
  }

  void RadObjectFactory_Genmesh::ParseFactory (iMeshFactoryWrapper *factory)
  {
    RadObjectFactory::ParseFactory (factory);

    // Very dumb parser, just disconnect all triangles etc
    csRef<iGeneralFactoryState> genFact = 
      scfQueryInterface<iGeneralFactoryState> (factory->GetMeshObjectFactory ());
    
    if (!genFact) return; // bail

    csTriangle *tris = genFact->GetTriangles ();
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

    
    for (i=0; i<genFact->GetTriangleCount ();i++)
    {
      RadPrimitive newPrim (vertexData);
      newPrim.GetIndexArray ().Push (tris[i].a);
      newPrim.GetIndexArray ().Push (tris[i].b);
      newPrim.GetIndexArray ().Push (tris[i].c);

      newPrim.ComputePlane ();
      
      allPrimitives.Push (newPrim);
    }
  }

  void RadObjectFactory_Genmesh::SaveFactory (iDocumentNode *node)
  {
    csRef<iGeneralFactoryState> genFact = 
      scfQueryInterface<iGeneralFactoryState> (
      factoryWrapper->GetMeshObjectFactory ());
    
    if (!genFact) return; // bail

    // For now, just dump.. later we should preserve extra attributes etc :)
    
    RenormalizeLightmapUVs ();

    genFact->SetVertexCount ((int)vertexData.vertexArray.GetSize ());

    csTriangle *tris = genFact->GetTriangles ();
    csVector3 *verts = genFact->GetVertices ();
    csVector2 *textureUV = genFact->GetTexels ();
    csRef<csRenderBuffer> lightmapBuffer = csRenderBuffer::CreateRenderBuffer (
      genFact->GetVertexCount (), CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 2);
    genFact->RemoveRenderBuffer ("texture coordinate lightmap");
    genFact->AddRenderBuffer ("texture coordinate lightmap", lightmapBuffer);
    
    csVector3 *factNormals = genFact->GetNormals ();

    {
      csRenderBufferLock<csVector2> bufferLock(lightmapBuffer);
      csVector2 *lightmapUV = bufferLock.Lock ();
      // Save vertex-data
      for (int i = 0; i < genFact->GetVertexCount (); ++i)
      {
        const RadObjectVertexData::Vertex &vertex = vertexData.vertexArray[i];
        verts[i] = vertex.position;
        textureUV[i] = vertex.textureUV;
        lightmapUV[i] = vertex.lightmapUV;
        factNormals[i] = vertex.normal;
      }
    }

    // Save primitives, trianglate on the fly
    IntDArray indexArray;
    indexArray.SetCapacity (allPrimitives.GetSize ());
    for (uint i = 0; i < allPrimitives.GetSize (); ++i)
    {
      SizeTDArray& indices = allPrimitives[i].GetIndexArray ();
      if (indices.GetSize () == 3)
      {
        //Triangle, easy case
        indexArray.Push ((int)indices[0]);
        indexArray.Push ((int)indices[1]);
        indexArray.Push ((int)indices[2]);
      }
      else
      {
        //TODO: Implement this case, use a triangulator
      }
    }

    genFact->SetTriangleCount ((int)indexArray.GetSize ()/3);
    csTriangle *gentri = genFact->GetTriangles ();
    std::copy (indexArray.GetArray (), indexArray.GetArray () + indexArray.GetSize (),
      (int*)gentri);

    RadObjectFactory::SaveFactory (node);
  }
}
