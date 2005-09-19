/*
  Copyright (C) 2005 by Marten Svanfeldt

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
    bool res = layoutEngine->LayoutUVOnPrimitives (allPrimitives, 
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
    allPrimitives = factory->allPrimitives;
    unsigned int i = 0;
    for (i = 0; i < allPrimitives.GetSize (); i++)
    {
      RadPrimitive& prim = allPrimitives[i];

      prim.Transform (transform);
      //recompute the factors
      prim.ComputePlane ();
      prim.ComputeUVTransform ();
      prim.SetRadObject (this);
      prim.Prepare (globalSettings.uPatchResolution, globalSettings.vPatchResolution);
    }

    // Create and init lightmaps
    for (i = 0; i < factory->lightmapTemplates.GetSize (); i++)
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

  void RadObject::SaveMesh (iDocumentNode *node)
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
        if (*mmData < FLT_EPSILON || *mmData > totalArea) continue;

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
          if (mmData[idx]>FLT_EPSILON) continue;

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

 /*         if (curMmRow[u] > FLT_EPSILON) continue; //only fix non-masked

          // Average 8 neighbours
          uint count = 0;
          csColor newColor (0.0f, 0.0f, 0.0f);
          
          // Top row
          if (topMmRow)
          {
            if (u > 0 && topMmRow[u-1] > FLT_EPSILON) newColor += topLmRow[u-1], count++;
            if (topMmRow[u] > FLT_EPSILON) newColor += topLmRow[u], count++;
            if (u < (lmw-1) && topMmRow[u+1] > FLT_EPSILON) newColor += topLmRow[u+1], count++;
          }

          // current row
          if (u > 0 && curMmRow[u-1] > FLT_EPSILON) newColor += curLmRow[u-1], count++;
          if (u < (lmw-1) && curMmRow[u+1] > FLT_EPSILON) newColor += curLmRow[u+1], count++;

          // bottom row
          if (bottomMmRow)
          {
            if (u > 0 && bottomMmRow[u-1] > FLT_EPSILON) newColor += bottomLmRow[u-1], count++;
            if (bottomMmRow[u] > FLT_EPSILON) newColor += bottomLmRow[u], count++;
            if (u < (lmw-1) && bottomMmRow[u+1] > FLT_EPSILON) newColor += bottomLmRow[u+1], count++;
          }

          if (count > 0)
          {
            curLmRow[u] = newColor * (1.0f/count);
          }
          else
          {
            curLmRow[u].Set (0.0f,0.0f,0.0f);
          }*/
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
    csVector3 *factNormals = genFact->GetNormals ();

    int i = 0;

    // Here we should save extra per-vertex stuff!
    normals = new csVector3[genFact->GetVertexCount ()];
    for (i = 0; i < genFact->GetVertexCount (); i++)
    {
      normals[i] = factNormals[i];
    }

    for (i=0; i<genFact->GetTriangleCount ();i++)
    {
      RadPrimitive newPrim;
      Vector3DArray &arr = newPrim.GetVertices ();
      IntArray &extra = newPrim.GetExtraData ();

      arr.Push (verts[tris[i].a]);
      arr.Push (verts[tris[i].b]);
      arr.Push (verts[tris[i].c]);

      extra.Push (tris[i].a);
      extra.Push (tris[i].b);
      extra.Push (tris[i].c);

      newPrim.GetUVs ().SetSize (3);
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

    genFact->SetVertexCount ((int)allPrimitives.GetSize () * 3);
    genFact->SetTriangleCount ((int)allPrimitives.GetSize ());
    
    csTriangle *tris = genFact->GetTriangles ();
    csVector3 *verts = genFact->GetVertices ();
    csVector2 *tc = genFact->GetTexels ();
    csVector3 *factNormals = genFact->GetNormals ();

    int j = 0;

    for (int i=0; i< genFact->GetTriangleCount (); i++)
    {
      const Lightmap* lm = lightmapTemplates[allPrimitives[i].GetLightmapID ()];
      
      allPrimitives[i].RenormalizeUVs (lm->GetWidth (),
                                       lm->GetHeight ());

      const Vector3DArray& v = allPrimitives[i].GetVertices ();
      const Vector2DArray& uv = allPrimitives[i].GetUVs ();
      const IntArray& extra = allPrimitives[i].GetExtraData ();

      verts[j] = v[0];
      tc[j] = uv[0];
      factNormals[j] = normals[extra[0]];
      tris[i].a = j++;

      verts[j] = v[1];
      tc[j] = uv[1];
      factNormals[j] = normals[extra[1]];
      tris[i].b = j++;

      verts[j] = v[2];
      tc[j] = uv[2];
      factNormals[j] = normals[extra[2]];
      tris[i].c = j++;
    }

    RadObjectFactory::SaveFactory (node);
  }
}
