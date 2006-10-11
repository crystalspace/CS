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
#include "lightmapuv.h"
#include "radobject.h"
#include "config.h"

namespace lighter
{

  // Very simple layouter.. just map "flat" on the lightmap
  LightmapUVLayoutFactory* SimpleUVLayouter::LayoutFactory (
    const RadPrimitiveArray& inPrims, RadObjectVertexData& vertexData,
    csArray<RadPrimitiveArray>& outPrims)
  {
    if (inPrims.GetSize () == 0) return 0;

    outPrims.Empty();

    LightmapPtrDelArray localLightmaps;

    // This is really dumb.. make sure every vertex have unqiue UV
    size_t i;

    BoolDArray vused;
    vused.SetSize (vertexData.vertexArray.GetSize (), false);

    //TODO Reimplement simple UV-layouter
    
    // Layout every primitive by itself    
    for (i = 0; i < inPrims.GetSize (); i++)
    {
      RadPrimitive prim (inPrims[i]);
      bool lmCoordsGood = false;
      int its = 0; //number of iterations
      
      csVector2 minuv, maxuv, uvSize;
      while (!lmCoordsGood && its < 5)
      {
        // Compute lightmapping

        /*prim.SetLightmapMapping (globalSettings.uTexelPerUnit / (1<<its), 
                                 globalSettings.vTexelPerUnit / (1<<its));*/


        ProjectPrimitive (prim, vused,
                          globalConfig.GetLMProperties ().uTexelPerUnit / (1<<its), 
                          globalConfig.GetLMProperties ().vTexelPerUnit / (1<<its));

        // Compute uv-size  
        prim.ComputeMinMaxUV (minuv, maxuv);
        uvSize = (maxuv-minuv)+csVector2(2.0f,2.0f);
        if (uvSize.x < globalConfig.GetLMProperties ().maxLightmapU &&
            uvSize.y < globalConfig.GetLMProperties ().maxLightmapV)
        {
          lmCoordsGood = true;
        }
        its++;
      } 

      if (lmCoordsGood)
      {
        // Ok, resonable size, lets try to find a LM for it
        csRect lmArea; 
        int lmID;
        bool res;
        res = AllocLightmap (localLightmaps, (int)ceilf (uvSize.x), 
	  (int)ceilf (uvSize.y), lmArea, lmID);
        if (!res) continue; 

        RadPrimitiveArray& outArray = outPrims.GetExtend (lmID);
        outArray.Push (prim);
        //prim.SetLightmapID (lmID);
      }
    }

    // Set the size of the lightmaps..
    /*for (i = 0; i < lightmaps.GetSize (); i++)
    {
      Lightmap *lm = lightmaps[i];
      // Calculate next pow2 size
      uint newWidth = csFindNearestPowerOf2 (lm->GetMaxUsedU ());
      uint newHeight = csFindNearestPowerOf2 (lm->GetMaxUsedV ());
      lm->SetSize (newWidth, newHeight);
    }*/

    return new SimpleUVLayoutFactory (this);
  }

  bool SimpleUVLayouter::AllocLightmap (LightmapPtrDelArray& lightmaps, 
    int u, int v, csRect &lightmapArea, int &lightmapID)
  {
    // Now see if we can get some space in the already allocated lightmaps
    csSubRect *rect;
    for (unsigned int i = 0; i < lightmaps.GetSize (); i++)
    {
      rect = lightmaps[i]->GetAllocator ().Alloc (u,v,lightmapArea);
      if (rect)
      {
        //We have a space, use it
        lightmapID = i;
        return true;
      }
    }

    // Still here, need a new lightmap
    Lightmap *newL = new Lightmap (globalConfig.GetLMProperties ().maxLightmapU,
                                   globalConfig.GetLMProperties ().maxLightmapV);
    lightmaps.Push (newL);

    rect = newL->GetAllocator ().Alloc (u,v,lightmapArea);
    if (rect)
    {
      //We have a space, use it
      lightmapID = (int)lightmaps.GetSize () - 1;
      return true;
    }

    return false;
  }

  bool SimpleUVLayouter::ProjectPrimitive (RadPrimitive &prim, BoolDArray &usedVerts,
    float uscale, float vscale)
  {
    size_t i;

    // Select projection dimension to be biggest component of plane normal
    if (prim.GetPlane ().GetNormal ().SquaredNorm () == 0.0f)
      prim.ComputePlane ();

    const csVector3& normal = prim.GetPlane ().GetNormal ();
    size_t projDimension = 0; //x

    if (fabsf (normal.y) > fabsf (normal.x) &&
        fabsf (normal.y) > fabsf (normal.z))
      projDimension = 1; //y biggest
    else if (fabsf (normal.z) > fabsf (normal.x))
      projDimension = 2; //z biggest

    size_t selX = (projDimension + 1) % 3;
    size_t selY = (projDimension + 2) % 3;

    RadObjectVertexData &vdata = prim.GetVertexData ();

    SizeTDArray &indexArray = prim.GetIndexArray ();

    size_t oldIndexSize = indexArray.GetSize ();

    for (i = 0; i < oldIndexSize; ++i)
    {
      if (usedVerts[indexArray[i]])
      {
        //need to duplicate        
        indexArray[i] = vdata.SplitVertex (indexArray[i]);
        usedVerts.SetSize (vdata.vertexArray.GetSize (), false);
      }
      usedVerts[indexArray[i]] = true;
    }

    for (i = 0; i < indexArray.GetSize (); ++i)
    {
      size_t index = indexArray[i];
      const csVector3 &position = vdata.vertexArray[index].position;
      csVector2 &lightmapUV = vdata.vertexArray[index].lightmapUV;

      lightmapUV.x = position[selX] * uscale;
      lightmapUV.y = position[selY] * vscale;
    
    }
    return true;
  }

  //-------------------------------------------------------------------------

  bool SimpleUVLayoutFactory::LayoutUVOnPrimitives (RadPrimitiveArray &prims, 
    RadObjectVertexData& vertexData, uint& lmID)
  {
    LightmapPtrDelArray& globalLightmaps = parent->globalLightmaps;

    csArray<csSubRectangles::SubRect*> subRects;

    size_t lightmapIndex = 0;
    bool successful = false;
    while (!successful)
    {
      while (!successful && (lightmapIndex < globalLightmaps.GetSize()))
      {
        successful = true;
        Lightmap* lm = globalLightmaps[lightmapIndex];
        for (size_t i = 0; i < prims.GetSize () && successful; i++)
        {
          RadPrimitive &prim = prims[i];

          csVector2 minuv, maxuv, uvSize;
          prim.ComputeMinMaxUV (minuv, maxuv);
          uvSize = (maxuv-minuv)+csVector2(2.0f,2.0f);

          csRect lmArea; 
          csSubRectangles::SubRect* res = 
            lm->GetAllocator().Alloc ((int)ceilf (uvSize.x), 
	        (int)ceilf (uvSize.y), lmArea);
          if (res == 0)
          {
            lightmapIndex++;
            successful = false;
            for (size_t r = 0; r < subRects.GetSize(); r++)
              lm->GetAllocator().Reclaim (subRects[r]);
            subRects.Empty();
          }
          else
          {
            subRects.Push (res);

            csVector2 uvRemap = csVector2(lmArea.xmin, lmArea.ymin) - minuv + csVector2(1.0f,1.0f);
            prim.RemapUVs (uvRemap);
            prim.SetGlobalLightmapID (uint (lightmapIndex));
          }
        }
      }
      if (!successful && lightmapIndex >= globalLightmaps.GetSize())
      {
        Lightmap *newL = new Lightmap (globalConfig.GetLMProperties ().maxLightmapU,
                                       globalConfig.GetLMProperties ().maxLightmapV);
        globalLightmaps.Push (newL);
      }
    }
    lmID = uint (lightmapIndex);
    return true;
  }

}

