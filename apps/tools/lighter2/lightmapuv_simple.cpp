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
#include "lightmapuv_simple.h"
#include "object.h"
#include "config.h"

namespace lighter
{

  // Very simple FactoryLayouter.. just map "flat" on the lightmap
  csPtr<LightmapUVObjectLayouter> SimpleUVFactoryLayouter::LayoutFactory (
    const PrimitiveArray& inPrims, ObjectVertexData& vertexData,
    const ObjectFactory* factory,
    csArray<PrimitiveArray>& outPrims)
  {
    if (inPrims.GetSize () == 0) return 0;

    SimpleUVObjectLayouter* newFactory = new SimpleUVObjectLayouter (this);

    outPrims.Empty();

    LightmapPtrDelArray localLightmaps;

    csArray<PrimitiveArray> coplanarPrims;
    DetermineNeighbouringPrims (inPrims, vertexData, coplanarPrims);

    // This is really dumb.. make sure every vertex have unqiue UV
    size_t i;

    BoolDArray vused;
    vused.SetSize (vertexData.vertexArray.GetSize (), false);

    //TODO Reimplement simple UV-FactoryLayouter
    
    // Layout every primitive by itself    
    for (i = 0; i < coplanarPrims.GetSize (); i++)
    {
      PrimitiveArray& prims = coplanarPrims[i];
      
      bool lmCoordsGood = false;
      int its = 0; //number of iterations
      
      csVector2 minuv, maxuv, uvSize;
      while (!lmCoordsGood && its < 5)
      {
        // Compute lightmapping
        ProjectPrimitives (prims, vused,
                           factory->GetLMuTexelPerUnit () / (1<<its), 
                           factory->GetLMvTexelPerUnit () / (1<<its));

        // Compute uv-size  
        prims[0].ComputeMinMaxUV (minuv, maxuv);
        for (size_t p = 1; p < prims.GetSize(); p++)
        {
          Primitive& prim (prims[p]);
          csVector2 pminuv, pmaxuv;
          prim.ComputeMinMaxUV (pminuv, pmaxuv);
          minuv.x = csMin (minuv.x, pminuv.x);
          minuv.y = csMin (minuv.y, pminuv.y);
          maxuv.x = csMax (maxuv.x, pmaxuv.x);
          maxuv.y = csMax (maxuv.y, pmaxuv.y);
        }
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

        PrimitiveArray& outArray = outPrims.GetExtend (lmID);
        csArray<csArray<size_t> >& coplanarGroup = 
          newFactory->coplanarGroups.GetExtend (lmID);
        csArray<size_t>& thisGroup = 
          coplanarGroup.GetExtend (coplanarGroup.GetSize());
        for (size_t p = 0; p < prims.GetSize(); p++)
        {
          size_t outIdx = outArray.Push (prims[p]);
          thisGroup.Push (outIdx);
        }
        thisGroup.ShrinkBestFit();
        //outPrims.Push (prims);
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

    return csPtr<LightmapUVObjectLayouter> (newFactory);
  }

  static int SortPrimByD (const Primitive& prim1, const Primitive& prim2)
  {
    const float D1 = prim1.GetPlane().DD;
    const float D2 = prim2.GetPlane().DD;
    if (D1 < D2)
      return -1;
    else if (D1 > D2)
      return 1;
    else
      return 0;
  }

  struct Edge
  {
    size_t a, b;

    Edge (size_t a, size_t b) : a (a), b (b) {}

    friend bool operator== (const Edge& e1, const Edge& e2)
    {
      return (e1.a == e2.b) && (e1.b == e2.a);
    }
  };

  struct UberPrimitive
  {
    PrimitiveArray prims;
    csArray<Edge> outsideEdges;

    UberPrimitive (const Primitive& startPrim);

    bool UsesEdge (const Edge& edge);
    void AddPrimitive (const Primitive& prim);
  };

  UberPrimitive::UberPrimitive (const Primitive& startPrim)
  {
    prims.Push (startPrim);

    const Primitive::TriangleType& t = startPrim.GetTriangle ();
    
    for (size_t e = 0; e < 3; e++)
    {
      Edge edge (t[e], t[(e+1) % 3]);
      outsideEdges.Push (edge);
    }
  }

  bool UberPrimitive::UsesEdge (const Edge& edge)
  {
    for (size_t o = 0; o < outsideEdges.GetSize(); o++)
    {
      if (outsideEdges[o] == edge) return true;
    }
    return false;
  }

  void UberPrimitive::AddPrimitive (const Primitive& prim)
  {
    prims.Push (prim);
    const Primitive::TriangleType& t = prim.GetTriangle ();
    for (size_t e = 0; e < 3; e++)
    {
      Edge edge (t[e], t[(e+1) % 3]);
      bool found = false;
      /* If edge is an "outside edge", remove from the outside list
       * Otherwise add it */
      for (size_t o = 0; o < outsideEdges.GetSize(); o++)
      {
        if (outsideEdges[o] == edge) 
        {
          found = true;
          outsideEdges.DeleteIndexFast (o);
          break;
        }
      }
      if (!found) outsideEdges.Push (edge);
    }
  }

  void SimpleUVFactoryLayouter::DetermineNeighbouringPrims (
    const PrimitiveArray& inPrims, ObjectVertexData& vertexData,
    csArray<PrimitiveArray>& outPrims)
  {
    PrimitiveArray primsByD;
    for (size_t i = 0; i < inPrims.GetSize (); i++)
    {
      const Primitive& prim (inPrims[i]);
      primsByD.InsertSorted (prim, SortPrimByD);
    }
    // Takes all neighbouring primitives
    csArray<UberPrimitive> uberPrims;
    while (primsByD.GetSize() > 0)
    {
      // Primitives are sorted by D, look for actual coplanar ones
      PrimitiveArray coplanarPrims;
      const Primitive& prim0 (primsByD[0]);
      float lastD = prim0.GetPlane().DD;
      const csVector3& lastNormal = prim0.GetPlane().GetNormal();
      
      for (size_t i = 1; i < primsByD.GetSize(); )
      {
        const Primitive& prim (primsByD[i]);
        if (fabsf (prim.GetPlane().DD - lastD) > EPSILON) break;
        if (((prim.GetPlane().GetNormal() * lastNormal)) > (1.0f - EPSILON))
        {
          coplanarPrims.Push (prim);
          primsByD.DeleteIndex (i);
        }
        else
          i++;
      }
      coplanarPrims.Push (prim0);
      primsByD.DeleteIndex (0);

      // In the coplanar ones, look for neighbouring ones
      while (coplanarPrims.GetSize() > 0)
      {
        const Primitive& prim (coplanarPrims[0]);
        UberPrimitive& ubp = uberPrims[uberPrims.Push (UberPrimitive (prim))];
        coplanarPrims.DeleteIndexFast (0);

        bool primAdded;
        do
        {
          primAdded = false;
          for (size_t p = 0; (p < coplanarPrims.GetSize()) && !primAdded; p++)
          {
            const Primitive& prim (coplanarPrims[p]);
            const Primitive::TriangleType& t = prim.GetTriangle ();
            for (size_t e = 0; e < 3; e++)
            {
              Edge edge (t[e], t[(e+1) % 3]);
              if (ubp.UsesEdge (edge))
              {
                ubp.AddPrimitive (prim);
                primAdded = true;
                coplanarPrims.DeleteIndex (p);
                break;
              }
            }
          }
        }
        while (primAdded);
      }
    }
    for (size_t u = 0; u < uberPrims.GetSize(); u++)
    {
      outPrims.Push (uberPrims[u].prims);
    }
  }

  bool SimpleUVFactoryLayouter::AllocLightmap (LightmapPtrDelArray& lightmaps, 
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

  bool SimpleUVFactoryLayouter::ProjectPrimitives (PrimitiveArray& prims, 
                                            BoolDArray &usedVerts,
                                            float uscale, float vscale)
  {
    size_t i;
    const Primitive& prim = prims[0];

    // Select projection dimension to be biggest component of plane normal
    //if (prim.GetPlane ().GetNormal ().SquaredNorm () == 0.0f)
      //prim.ComputePlane ();

    const csVector3& normal = prim.GetPlane ().GetNormal ();
    size_t projDimension = 0; //x

    if (fabsf (normal.y) > fabsf (normal.x) &&
        fabsf (normal.y) > fabsf (normal.z))
      projDimension = 1; //y biggest
    else if (fabsf (normal.z) > fabsf (normal.x))
      projDimension = 2; //z biggest

    size_t selX = (projDimension + 1) % 3;
    size_t selY = (projDimension + 2) % 3;

    csSet<size_t> primsUsedVerts;
    csArray<size_t> indexMap;

    for (size_t p = 0; p < prims.GetSize(); p++)
    {
      Primitive& prim = prims[p];

      ObjectVertexData &vdata = prim.GetVertexData ();

      Primitive::TriangleType& t = prim.GetTriangle ();

      for (i = 0; i < 3; ++i)
      {
        size_t index = t[i]; 
        if (usedVerts[index])
        {
          //need to duplicate
          if ((indexMap.GetSize() <= index) 
            || (indexMap[index] == (size_t)~0))
          {
            size_t newIndex = vdata.SplitVertex (index);
            usedVerts.SetSize (newIndex+1, false);
            indexMap.SetSize (newIndex+1, (size_t)~0);
            indexMap[index] = newIndex;
            index = newIndex;
          }
          else
            index = indexMap[index];
          t[i] = index;
        }
        primsUsedVerts.Add (index);
      }

      for (i = 0; i < 3; ++i)
      {
        size_t index = t[i];
        const csVector3 &position = vdata.vertexArray[index].position;
        csVector2 &lightmapUV = vdata.vertexArray[index].lightmapUV;

        lightmapUV.x = position[selX] * uscale;
        lightmapUV.y = position[selY] * vscale;
      
      }
    }
    csSet<size_t>::GlobalIterator it (primsUsedVerts.GetIterator());
    while (it.HasNext())
    {
      size_t index = it.Next ();
      usedVerts[index] = true;
    }
    return true;
  }

  //-------------------------------------------------------------------------

  bool SimpleUVObjectLayouter::LayoutUVOnPrimitives (PrimitiveArray &prims, 
    size_t groupNum, ObjectVertexData& vertexData, uint& lmID)
  {
    const csArray<csArray<size_t> >& coplanarGroup = coplanarGroups[groupNum];

    csArray<csVector2> uvsizes;
    csArray<csVector2> minuvs;
    for (size_t c = 0; c < coplanarGroup.GetSize(); c++)
    {
      const csArray<size_t>& coPrim = coplanarGroup[c];
      csVector2 minuv, maxuv, uvSize;

      // Compute uv-size  
      prims[coPrim[0]].ComputeMinMaxUV (minuv, maxuv);
      for (size_t p = 1; p < coPrim.GetSize(); p++)
      {
        Primitive& prim (prims[coPrim[p]]);
        csVector2 pminuv, pmaxuv;
        prim.ComputeMinMaxUV (pminuv, pmaxuv);
        minuv.x = csMin (minuv.x, pminuv.x);
        minuv.y = csMin (minuv.y, pminuv.y);
        maxuv.x = csMax (maxuv.x, pmaxuv.x);
        maxuv.y = csMax (maxuv.y, pmaxuv.y);
      }
      uvSize = (maxuv-minuv)+csVector2(2.0f,2.0f);
      uvsizes.Push (uvSize);
      minuvs.Push (minuv);
    }

    csArray<csVector2> remaps;
    csSet<size_t> remapped;
    MapComplete (uvsizes, minuvs, remaps, lmID);
    for (size_t c = 0; c < coplanarGroup.GetSize(); c++)
    {
      const csArray<size_t>& coPrim = coplanarGroup[c];
      for (size_t p = 0; p < coPrim.GetSize(); p++)
      {
        Primitive& prim (prims[coPrim[p]]);
        const Primitive::TriangleType& t = prim.GetTriangle ();
        // Be careful to remap each distinct vertex only once
        const csVector2& move = remaps[c];
        for (size_t v = 0; v < 3; v++)
        {
          size_t index = t [v];
          if (!remapped.Contains (index))
          {
            csVector2 &uv = vertexData.vertexArray[index].lightmapUV;
            uv += move;
            remapped.AddNoTest (index);
          }
        }
        //prim.RemapUVs (remaps[c]);
        prim.SetGlobalLightmapID (lmID);
      }
    }

    return true;
  }

  void SimpleUVObjectLayouter::MapComplete (const csArray<csVector2>& sizes, 
                                           const csArray<csVector2>& minuvs,  
                                           csArray<csVector2>& remaps, 
                                           uint& lmID)
  {
    LightmapPtrDelArray& globalLightmaps = parent->globalLightmaps;
    csArray<csSubRectangles::SubRect*> subRects;

    remaps.SetSize (sizes.GetSize());
    size_t lightmapIndex = 0;
    bool successful = false;
    while (!successful)
    {
      while (!successful && (lightmapIndex < globalLightmaps.GetSize()))
      {
        successful = true;
        Lightmap* lm = globalLightmaps[lightmapIndex];
        for (size_t i = 0; i < sizes.GetSize () && successful; i++)
        {

          const csVector2& uvSize = sizes[i];

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

            csVector2 uvRemap = 
              csVector2(lmArea.xmin, lmArea.ymin) - minuvs[i] 
              + csVector2(1.0f,1.0f);
            remaps[i] = uvRemap;
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
  }

}

