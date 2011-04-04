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

#include "common.h"

#include "lightmapuv_simple.h"
#include "object.h"
#include "config.h"

#include "lightmapuv_simple_util.h"

namespace lighter
{

  /* Idea for future:
     Take what PD lights affect the object's instances of a set of 
     neighbouring prims and take that into consideration when splitting
     the factory. E.g. if two sets of neighbouring prims are affected
     by different PD light sets on the object two submeshes are created.
     This could improve lightmap layouting when a lot of PD lights
     are used.
   */

  // Very simple FactoryLayouter.. just map "flat" on the lightmap
  csPtr<LightmapUVObjectLayouter> SimpleUVFactoryLayouter::LayoutFactory (
    const FactoryPrimitiveArray& inPrims, ObjectFactoryVertexData& vertexData,
    const ObjectFactory* factory,
    csArray<FactoryPrimitiveArray>& outPrims,
    csBitArray& usedVerts, bool noSplit)
  {
    if (inPrims.GetSize () == 0) return 0;

    SimpleUVObjectLayouter* newFactory = new SimpleUVObjectLayouter (this);

    csArray<FactoryPrimitiveArray> coplanarPrims;
    DetermineNeighbouringPrims (inPrims, vertexData, coplanarPrims);
    newFactory->lightmapUVs.SetSize (vertexData.positions.GetSize());

    //TODO Reimplement simple UV-FactoryLayouter

    LightmapPtrDelArray localLightmaps;

    bool hasUnprojected;
    const int maxIter = csMax (
      csLog2 (globalConfig.GetLMProperties ().maxLightmapU),
      csLog2 (globalConfig.GetLMProperties ().maxLightmapV));
    const int maxIts = noSplit ? maxIter : 5;
      
    hasUnprojected = false;
    outPrims.Empty();
    localLightmaps.Empty();
    newFactory->uvsizes.Empty ();
    newFactory->coplanarGroups.Empty ();

    csArray<SizeAndIndex> sizes;
    csArray<csVector2> minuvs;

    // Layout every primitive by itself    
    for (size_t i = 0; i < coplanarPrims.GetSize (); i++)
    {
      FactoryPrimitiveArray& prims = coplanarPrims[i];
      
      bool lmCoordsGood = false;
      int its = 0; //number of iterations

      csBitArray groupUsedVerts (usedVerts);
      // Compute lightmapping
      bool projected = ProjectPrimitives (prims, groupUsedVerts,
					  factory->GetLMDensity (), 
					  newFactory->lightmapUVs,
					  noSplit);
      hasUnprojected |= !projected;
      
      if (projected)
      {
	csVector2 minuv, maxuv, uvSize;
	// Compute uv-size  
	prims[0].ComputeMinMaxUV (newFactory->lightmapUVs, minuv, maxuv);
	for (size_t p = 1; p < prims.GetSize(); p++)
	{
	  FactoryPrimitive& prim (prims[p]);
	  csVector2 pminuv, pmaxuv;
	  prim.ComputeMinMaxUV (newFactory->lightmapUVs, pminuv, pmaxuv);
	  minuv.x = csMin (minuv.x, pminuv.x);
	  minuv.y = csMin (minuv.y, pminuv.y);
	  maxuv.x = csMax (maxuv.x, pmaxuv.x);
	  maxuv.y = csMax (maxuv.y, pmaxuv.y);
	}
	while (!lmCoordsGood && its < maxIts)
	{
	  float scale = 1.0f / (1<<its);
	  uvSize = (maxuv-minuv)*scale+csVector2(2.0f,2.0f);
	  if (uvSize.x < globalConfig.GetLMProperties ().maxLightmapU &&
	      uvSize.y < globalConfig.GetLMProperties ().maxLightmapV)
	  {
	    lmCoordsGood = true;
	    ScaleLightmapUVs (prims, newFactory->lightmapUVs, scale);
	  }
	  its++;
	} 
  
	if (lmCoordsGood)
	{
	  // Ok, reasonable size - find a LM for it in the next step
	  usedVerts.SetSize (groupUsedVerts.GetSize());
	  usedVerts |= groupUsedVerts;
  
	  SizeAndIndex newSize;
	  newSize.uvsize = uvSize;
	  newSize.index = i;
	  sizes.Push (newSize);
  
	  /* Subtle: causes lumels to be aligned on a world space grid.
	  * The intention is that the lightmap coordinates for vertices 
	  * for two adjacent faces are lined up nicely.
	  * @@@ Does not take object translation into account. */
	  minuv.x = floor (minuv.x);
	  minuv.y = floor (minuv.y);
	  minuvs.GetExtend (i) = minuv;
	}
      }
    }
    // The rectangle packer works better when the rects are sorted by size.
    sizes.Sort (SortByUVSize<SizeAndIndex>);

    for (size_t s = 0; s < sizes.GetSize(); s++)
    {
      FactoryPrimitiveArray& prims = coplanarPrims[sizes[s].index];

      csRect lmArea; 
      int lmID;
      bool res;
      res = AllocLightmap (localLightmaps, (int)ceilf (sizes[s].uvsize.x), 
	  (int)ceilf (sizes[s].uvsize.y), lmArea, lmID);
      if (!res) continue; 

      FactoryPrimitiveArray& outArray = outPrims.GetExtend (lmID);
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

      csArray<csVector2>& groupUVsizes = 
	newFactory->uvsizes.GetExtend (lmID);
      groupUVsizes.Push (sizes[s].uvsize);
    }
    
    if (noSplit && (localLightmaps.GetSize() > 1)) return 0;
    if (hasUnprojected) return 0;

#ifdef DUMP_SUBRECTANGLES
    static int counter = 0;
    for (size_t i = 0; i < localLightmaps.GetSize (); i++)
    {
      Lightmap *lm = localLightmaps[i];
      csString str;
      str.Format ("locallightmap_%i_%zu", counter, i);
      lm->GetAllocator().Dump (str);
    }
    counter++;
#endif

    return csPtr<LightmapUVObjectLayouter> (newFactory);
  }

  uint SimpleUVFactoryLayouter::AllocLightmap (uint lmW, uint lmH)
  {
    if (lmW == 0) lmW = globalConfig.GetLMProperties ().maxLightmapU;
    if (lmH == 0) lmH = globalConfig.GetLMProperties ().maxLightmapV;
    
    Lightmap *newL = new Lightmap (lmW, lmH);
    csRect r;
    newL->GetAllocator().Alloc (lmW, lmH, r);
    newL->UpdateDimensions();
    
    size_t index;
    if (prepared && globalConfig.GetLighterProperties().directionalLMs)
    {
      size_t numLMs = globalLightmaps.GetSize();
      numLMs /= 4;
      index = numLMs;
      for (size_t i = 4; i-- > 1; )
      {
	globalLightmaps.Insert (index + numLMs*i,
	  new Lightmap (*newL));
      }
      globalLightmaps.Insert (index, newL);
    }
    else
      index = globalLightmaps.Push (newL);

    return (uint)index;
  }

  bool SimpleUVFactoryLayouter::Edge::equals (VertexEquality veq, 
                                              const Edge& other)
  {
    switch (veq)
    {
      case Pedantic:
        return (a == other.b) && (b == other.a);
      case PositionAndNormal:
        {
          const csVector3& n1 = prim.GetVertexData().normals[a];
          const csVector3& n2 = 
            other.prim.GetVertexData().normals[other.b];
          if (!((n1-n2).IsZero (
            globalConfig.GetLMProperties().normalsTolerance))) return false;
        }
        {
          const csVector3& n1 = prim.GetVertexData().normals[other.a];
          const csVector3& n2 = 
            other.prim.GetVertexData().normals[b];
          if (!((n1-n2).IsZero (
            globalConfig.GetLMProperties().normalsTolerance))) return false;
        }
        // Fall through
      case Position:
        {
          const csVector3& p1 = prim.GetVertexData().positions[a];
          const csVector3& p2 = 
            other.prim.GetVertexData().positions[other.b];
          if (!((p1-p2).IsZero (SMALL_EPSILON))) return false;
        }
        {
          const csVector3& p1 = prim.GetVertexData().positions[other.a];
          const csVector3& p2 = 
            other.prim.GetVertexData().positions[b];
          if (!((p1-p2).IsZero (SMALL_EPSILON))) return false;
        }
        return true;
    }
    CS_ASSERT_MSG("Should not happen - someone forgot a 'case'", false);
    return false;
  }

  SimpleUVFactoryLayouter::UberPrimitive::UberPrimitive (
    VertexEquality equality, const FactoryPrimitive& startPrim) : 
    equality (equality)
  {
    prims.Push (startPrim);

    const Primitive::TriangleType& t = startPrim.GetTriangle ();
    
    for (size_t e = 0; e < 3; e++)
    {
      Edge edge (startPrim, t[e], t[CS::Math::NextModulo3(e)]);
      outsideEdges.Push (edge);
    }
  }

  bool SimpleUVFactoryLayouter::UberPrimitive::UsesEdge (
    const Edge& edge)
  {
    for (size_t o = 0; o < outsideEdges.GetSize(); o++)
    {
      if (outsideEdges[o].equals (equality, edge)) return true;
    }
    return false;
  }

  void SimpleUVFactoryLayouter::UberPrimitive::AddPrimitive (
    const FactoryPrimitive& prim)
  {
    prims.Push (prim);
    const Primitive::TriangleType& t = prim.GetTriangle ();
    for (size_t e = 0; e < 3; e++)
    {
      Edge edge (prim, t[e], t[CS::Math::NextModulo3 (e)]);
      bool found = false;
      /* If edge is an "outside edge", remove from the outside list
       * Otherwise add it */
      for (size_t o = 0; o < outsideEdges.GetSize(); o++)
      {
        if (outsideEdges[o].equals (equality, edge)) 
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
    const FactoryPrimitiveArray& inPrims, ObjectFactoryVertexData& vertexData,
    csArray<FactoryPrimitiveArray>& outPrims)
  {
    FactoryPrimitiveArray workPrims (inPrims);
    // Takes all neighbouring primitives
    UberPrimArray uberPrims;
    while (workPrims.GetSize() > 0)
    {
      // Primitives are sorted by D, look for actual coplanar ones
      FactoryPrimitiveArray coplanarPrims;
      const FactoryPrimitive& prim0 (workPrims[0]);
      csVector3 normalsSum = prim0.GetPlane().GetNormal();
      
      for (size_t i = 1; i < workPrims.GetSize(); )
      {
        const FactoryPrimitive& prim (workPrims[i]);
        if (((prim.GetPlane().GetNormal() * normalsSum.Unit())) > 
          (1.0f - globalConfig.GetLMProperties().normalsTolerance))
        {
          normalsSum += prim.GetPlane().GetNormal();
          coplanarPrims.Push (prim);
          workPrims.DeleteIndex (i);
        }
        else
        {
          i++;
        }
      }
      coplanarPrims.Push (prim0);
      workPrims.DeleteIndex (0);

      // In the coplanar ones, look for neighbouring ones
      while (coplanarPrims.GetSize() > 0)
      {
        const FactoryPrimitive& prim (coplanarPrims[0]);
        UberPrimitive& ubp = uberPrims[uberPrims.Push (UberPrimitive (Position, prim))];
        coplanarPrims.DeleteIndexFast (0);

        bool primAdded;
        do
        {
          primAdded = false;
          for (size_t p = 0; (p < coplanarPrims.GetSize()) && !primAdded; p++)
          {
            const FactoryPrimitive& prim (coplanarPrims[p]);
            const FactoryPrimitive::TriangleType& t = prim.GetTriangle ();
            for (size_t e = 0; e < 3; e++)
            {
              Edge edge (prim, t[e], t[CS::Math::NextModulo3(e)]);
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

  class ArraysOneUV
  {
  public:
    class ArrayType
    {
      int u, v;
    public:
      ArrayType (int u, int v) : u (u), v (v) {}

      size_t GetSize () const { return 1; }
      csVector2 GetUVSize (size_t n) const 
      { 
        return csVector2 (u, v);
      }
    };
    ArrayType pseudoArray;
    ArraysOneUV (int u, int v) : pseudoArray (u, v) {}

    size_t GetSize() const { return 1; }
    ArrayType Get (size_t index) const 
    { return pseudoArray; }
  };

  bool SimpleUVFactoryLayouter::AllocLightmap (LightmapPtrDelArray& lightmaps, 
    int u, int v, csRect &lightmapArea, int &lightmapID)
  {
    AllocLightmapArray<false> allocLM (lightmaps);
    AllocResultHash results;
    bool b = AllocAllPrims (ArraysOneUV (u, v), allocLM, results, 0, allocDefault);
    
    const AllocResult& result = *results.GetElementPointer (0);
    lightmapArea.xmin = int (result.positions[0].x);
    lightmapArea.ymin = int (result.positions[0].y);
    lightmapArea.xmax = lightmapArea.xmin + u;
    lightmapArea.ymax = lightmapArea.ymin + v;
    lightmapID = int (result.allocIndex);

    return b;
  }

  bool SimpleUVFactoryLayouter::ProjectPrimitives (FactoryPrimitiveArray& prims, 
                                                   csBitArray &usedVerts,
                                                   float lmscale, 
                                                   Vector2Array& lightmapUVs,
                                                   bool noSplit)
  {
    size_t i;
    //const FactoryPrimitive& prim = prims[0];

    // Select projection dimension to be biggest component of plane normal

    //const csVector3& normal = prim.GetPlane ().GetNormal ();
    csVector3 normal (prims[0].GetPlane ().GetNormal ());
    for (size_t p = 1; p < prims.GetSize(); p++)
      normal += prims[p].GetPlane ().GetNormal ();
      
    size_t projDimension = 0; //x

    if (fabsf (normal.y) > fabsf (normal.x) &&
        fabsf (normal.y) > fabsf (normal.z))
      projDimension = 1; //y biggest
    else if (fabsf (normal.z) > fabsf (normal.x))
      projDimension = 2; //z biggest

    size_t selX = CS::Math::NextModulo3(projDimension);
    size_t selY = CS::Math::NextModulo3(selX);

    csSet<size_t> primsUsedVerts;
    csArray<size_t> indexMap;

    csVector2 minUV (FLT_MAX, FLT_MAX);
    csVector2 maxUV (-FLT_MAX, -FLT_MAX);

    for (size_t p = 0; p < prims.GetSize(); p++)
    {
      FactoryPrimitive& prim = prims[p];

      ObjectFactoryVertexData &vdata = prim.GetVertexData ();

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
            if (noSplit) return false;
            size_t newIndex = vdata.SplitVertex (index);
            usedVerts.SetSize (newIndex+1);
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
        const csVector3 &position = vdata.positions[index];
        csVector2 &lightmapUV = lightmapUVs.GetExtend (index);

        lightmapUV.x = position[selX] * lmscale;
        lightmapUV.y = position[selY] * lmscale;

        minUV.x = csMin (minUV.x, lightmapUV.x);
        minUV.y = csMin (minUV.y, lightmapUV.y);
        maxUV.x = csMax (maxUV.x, lightmapUV.x);
        maxUV.y = csMax (maxUV.y, lightmapUV.y);
      }
    }
    if ((maxUV.x - minUV.x) > (maxUV.y - minUV.y))
    {
      csSet<size_t>::GlobalIterator it (primsUsedVerts.GetIterator());
      while (it.HasNext())
      {
        size_t index = it.Next();
        csVector2& lightmapUV = lightmapUVs[index];
        float t = lightmapUV.x;
        lightmapUV.x = lightmapUV.y;
        lightmapUV.y = t;
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

  void SimpleUVFactoryLayouter::ScaleLightmapUVs (FactoryPrimitiveArray& prims,
                                                  Vector2Array& lightmapUVs, 
                                                  float scale)
  {
    csBitArray scaled;
    scaled.SetSize (lightmapUVs.GetSize());

    for (size_t p = 0; p < prims.GetSize(); p++)
    {
      FactoryPrimitive& prim = prims[p];
      Primitive::TriangleType& t = prim.GetTriangle ();

      for (int i = 0; i < 3; ++i)
      {
        size_t index = t[i];
        if (scaled.IsBitSet (index)) continue;
        lightmapUVs[index].x *= scale;
        lightmapUVs[index].y *= scale;
        scaled.SetBit (index);
      }
    }
  }

  void SimpleUVFactoryLayouter::QueuePDPrimitives (
    SimpleUVObjectLayouter* layouter, PrimitiveArray &prims, 
    size_t layoutID, size_t groupNum, Sector* sector, 
    const csBitArray& pdBits, const csArray<csVector2>& uvsizes)
  {
    QueuedPDPrimitives queuedPrims;
    queuedPrims.layouter = layouter;
    queuedPrims.prims = &prims;
    queuedPrims.layoutID = layoutID;
    queuedPrims.groupNum = groupNum;
    queuedPrims.uvsizes = uvsizes;
    SectorAndPDBits s;
    s.pdBits = pdBits;
    /* The originating sector is only relevant for PD-affected queues
     * (since PD lights are stored in the sector PD bits can only be 
     * compared for queues originating in the same sector, but when no
     * bits are set, this doesn't matter much). 
     * But PD-unaffected queues can be merged here already.
     */
    if (pdBits.AllBitsFalse())
      s.sector = 0;
    else
      s.sector = sector;
    QueuedPDPArray* q = pdQueues.GetElementPointer (s);
    if (q == 0)
    {
      pdQueues.Put (s, QueuedPDPArray ());
      q = pdQueues.GetElementPointer (s);
    }
    q->Push (queuedPrims);
  }

  //-------------------------------------------------------------------------

  size_t SimpleUVObjectLayouter::LayoutUVOnPrimitives (PrimitiveArray &prims, 
    size_t groupNum, Sector* sector, const csBitArray& pdBits)
  {
    // Prims will be layouted later...
    size_t layoutID = minuvs.GetSize();
    // Update minimum UVs for this particular layout
    ComputeMinUVs (prims, groupNum, minuvs.GetExtend (layoutID));

    parent->QueuePDPrimitives (this, prims, layoutID, groupNum, sector, pdBits,
      this->uvsizes[groupNum]);

    return layoutID;
  }

  void SimpleUVObjectLayouter::FinalLightmapLayout (PrimitiveArray &prims, 
                                                    size_t layoutID,
                                                    size_t groupNum, 
                                                    ObjectVertexData& vertexData, 
                                                    uint& lmID)
  {
    /* Primitives were enqueued for PD layouting. layouted onto a PD lightmap. That lightmap itself was
       placed somewhere on a global LM. So when remapping this must be taken
       into consideration. */
    const PDLayoutedGroup* layouted = pdLayouts.GetElementPointer (layoutID);
    CS_ASSERT(layouted);
    lmID = uint (layouted->lmID);

    const csArray<csArray<size_t> >& coplanarGroup = coplanarGroups[groupNum];
    csSet<size_t> remapped;
    for (size_t c = 0; c < coplanarGroup.GetSize(); c++)
    {
      const csArray<size_t>& coPrim = coplanarGroup[c];
      for (size_t p = 0; p < coPrim.GetSize(); p++)
      {
        Primitive& prim = prims[coPrim[p]];
        const Primitive::TriangleType& t = prim.GetTriangle ();
        // Be careful to remap each distinct vertex only once
        const csVector2& move = layouted->remaps[c];
        for (size_t v = 0; v < 3; v++)
        {
          size_t index = t [v];
          if (!remapped.Contains (index))
          {
            csVector2 &uv = vertexData.lightmapUVs[index];
            uv = lightmapUVs[index] + move + csVector2(0.5f,0.5f);
            remapped.AddNoTest (index);
          }
        }
        prim.SetGlobalLightmapID (lmID);
      }
    }
  }

  void SimpleUVObjectLayouter::ComputeMinUVs (PrimitiveArray& prims, 
                                              size_t groupNum,
                                              csArray<csVector2>& minuvs)
  {
    const csArray<csArray<size_t> >& coplanarGroup = coplanarGroups[groupNum];

    for (size_t c = 0; c < coplanarGroup.GetSize(); c++)
    {
      const csArray<size_t>& coPrim = coplanarGroup[c];
      csVector2 minuv, maxuv;

      // Compute uv-size
      Primitive& prim0 = prims[coPrim[0]];
      prim0.ComputeMinMaxUV (lightmapUVs, minuv, maxuv);
      for (size_t p = 1; p < coPrim.GetSize(); p++)
      {
        Primitive& prim = prims[coPrim[p]];
        csVector2 pminuv, pmaxuv;
        prim.ComputeMinMaxUV (lightmapUVs, pminuv, pmaxuv);
        minuv.x = csMin (minuv.x, pminuv.x);
        minuv.y = csMin (minuv.y, pminuv.y);
        maxuv.x = csMax (maxuv.x, pmaxuv.x);
        maxuv.y = csMax (maxuv.y, pmaxuv.y);
      }
      
      /* Subtle: causes lumels to be aligned on a world space grid.
       * The intention is that the lightmap coordinates for vertices 
       * for two adjacent faces are lined up nicely.
       * @@@ Does not take object translation into account. */
      minuv.x = floor (minuv.x);
      minuv.y = floor (minuv.y);
      minuvs.Push (minuv);
    }
  }

  void SimpleUVObjectLayouter::LayoutQueuedPrims (PrimitiveArray &prims, 
    size_t layoutID, size_t groupNum, size_t lightmap, 
    const csArray<csVector2>& positions, int dx, int dy)
  {
    const csArray<csVector2>& minuvs = this->minuvs[/*groupNum*/layoutID];
    PDLayoutedGroup layouted;
    layouted.lmID = uint (lightmap);
    layouted.remaps = positions;
    csVector2 move (dx+1, dy+1);
    for (size_t v = 0; v < layouted.remaps.GetSize(); v++)
    {
      csVector2& remap = layouted.remaps[v];
      remap += move - minuvs[v];
    }
    pdLayouts.Put (layoutID, layouted);
  }

}

