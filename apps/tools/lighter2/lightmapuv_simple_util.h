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

#ifndef __LIGHTMAPUV_SIMPLE_UTIL_H__
#define __LIGHTMAPUV_SIMPLE_UTIL_H__

namespace lighter
{
  
  struct SizeAndIndex
  {
    csVector2 uvsize;
    size_t index;
  };

  template<typename T>
  static int SortByUVSize (T const& s1, T const& s2)
  {
    float dx = s1.uvsize.x - s2.uvsize.x;
    if (dx > 0)
      return -1;
    else if (dx < 0)
      return 1;
    float dy = s1.uvsize.y - s2.uvsize.y;
    if (dy > 0)
      return -1;
    else if (dy < 0)
      return 1;
    return 0;
  }

  /// Set of primitives to be mapped on one allocator
  struct PrimToMap
  {
    size_t srcArray;
    size_t sourceIndex;
    csVector2 uvsize;
    csRect rect;
  };

  struct MPTAAMAlloc
  {
    static CS::SubRectangles::SubRect* Alloc (CS::SubRectangles& allocator,
      int w, int h, csRect& rect)
    { return allocator.Alloc (w, h, rect);  }
  };

  struct MPTAAMAllocNoGrow
  {
    static CS::SubRectangles::SubRect* Alloc (
      CS::SubRectanglesCompact& allocator, int w, int h, csRect& rect)
    { return allocator.AllocNoGrow (w, h, rect);  }
  };

#if defined(DUMP_SUBRECTANGLES)
  namespace
  {
    static int MapPrimsToAlloc_counter = 0;
  }
#endif

  template<class AllocMixin>
  static bool MapPrimsToAlloc (csArray<PrimToMap>& primsToMap, 
    CS::SubRectanglesCompact& alloc, 
    csArray<CS::SubRectangles::SubRect*>& outSubRects)
  {
    bool success = true;
    csRect oldSize (alloc.GetRectangle());
    csArray<CS::SubRectanglesCompact::SubRect*> subRects;
    for (size_t p = 0; p < primsToMap.GetSize(); p++)
    {
      CS::SubRectanglesCompact::SubRect* sr = 
        AllocMixin::Alloc (alloc, int (ceilf (primsToMap[p].uvsize.x)),
          int (ceilf (primsToMap[p].uvsize.y)), primsToMap[p].rect);
      if (sr == 0)
      {
#if defined(DUMP_SUBRECTANGLES)
        csString str;
        str.Format ("MapPrimsToAlloc_fail_%d", MapPrimsToAlloc_counter++);
        alloc.Dump (str);
#endif
        for (size_t s = subRects.GetSize(); s-- > 0; )
          alloc.Reclaim (subRects[s]);
        success = false;
        alloc.Shrink (oldSize.Width(), oldSize.Height());
        break;
      }
      else
      {
        subRects.Push (sr);
      }
    }
    outSubRects = subRects;
    return success;
  }

  struct AllocResult
  {
    size_t allocIndex;
    csArray<csVector2> positions;
    csArray<CS::SubRectangles::SubRect*> subRects;
  };
  typedef csHash<AllocResult, size_t> AllocResultHash;

  enum
  {
    resultFailure,
    resultAllocated,
    resultWithNew
  };

  template<class Arrays, class Allocators, class AllocMixin>
  static int AllocAllPrimsInner (const Arrays& arrays, Allocators& allocs,
    AllocResultHash& result, const csArray<SizeAndIndex>& testOrder,
    csArray<PrimToMap>& primsToMap)
  {
    size_t allocator;
    bool allMapped = false;
    bool newCreated = false;
    csArray<CS::SubRectangles::SubRect*> subRects;
    for (size_t a = 0; a < allocs.GetSize(); a++)
    {
      size_t useAlloc = testOrder[a].index;
      allMapped = MapPrimsToAlloc<AllocMixin> (primsToMap, 
        allocs.Get (useAlloc), subRects);
      if (allMapped)
      {
        allocator = useAlloc;
        break;
      }
    }
    if (!allMapped)
    {
      allMapped = MapPrimsToAlloc<AllocMixin> (primsToMap, 
        allocs.New (allocator), subRects);
      newCreated = true;
    }
    if (allMapped) 
    {
      for (size_t p = 0; p < primsToMap.GetSize(); p++)
      {
        const PrimToMap& prim = primsToMap[p];
        AllocResult* res = result.GetElementPointer (prim.srcArray);
        if (res == 0)
        {
          AllocResult newResult;
          newResult.allocIndex = allocator;
          result.Put (prim.srcArray, newResult);
          res = result.GetElementPointer (prim.srcArray);
        }
        csVector2 uvRemap = csVector2(prim.rect.xmin, prim.rect.ymin);
        res->positions.GetExtend (prim.sourceIndex) = uvRemap;
        res->subRects.GetExtend (prim.sourceIndex, 0) = subRects[p];
      }
      return newCreated ? resultWithNew : resultAllocated;
    }
    else
      allocs.Delete (allocator);
    return resultFailure;
  }

  const uint allocTryNoGrow = 1;
  const uint allocTryNormal = 2;
  const uint allocDefault = allocTryNoGrow | allocTryNormal;

  /// Allocate all queued primitves onto allocators from \a allocs.
  template<class Arrays, class Allocators>
  static bool AllocAllPrims (const Arrays& arrays, Allocators& allocs,
    AllocResultHash& result, Statistics::Progress* progress, uint flags)
  {
    size_t u, updateFreq;
    float progressStep;
    if (progress && (arrays.GetSize() > 0))
    {
      updateFreq = progress->GetUpdateFrequency (arrays.GetSize());
      u = 0;
      progressStep = updateFreq * (1.0f / arrays.GetSize());
      progress->SetProgress (0);
    }
    bool createTestOrder = true;
    csArray<SizeAndIndex> testOrder;

    size_t arraysFirst = 0;
    while (arraysFirst < arrays.GetSize())
    {
      size_t tryCount = arrays.GetSize() - arraysFirst;

      if (createTestOrder)
      {
        testOrder.Empty ();
        for (size_t a = 0; a < allocs.GetSize(); a++)
        {
          SizeAndIndex si;
          si.index = a;
          const csRect& allocRec = allocs.Get (a).GetRectangle();
          si.uvsize.Set (allocRec.xmax, allocRec.ymax);
          testOrder.Push (si);
        }
        testOrder.Sort (SortByUVSize<SizeAndIndex>);
        createTestOrder = false;
      }

      while (tryCount > 0)
      {
        csArray<PrimToMap> primsToMap;
        for (size_t a = 0; a < tryCount; a++)
        {
          typename Arrays::ArrayType queue = arrays.Get (a+arraysFirst);
          for (size_t p = 0; p < queue.GetSize(); p++)
          {
            PrimToMap prim;
            prim.srcArray = a+arraysFirst;
            prim.sourceIndex = p;
            prim.uvsize = queue.GetUVSize (p);
            primsToMap.Push (prim);
          }
        }
        primsToMap.Sort (SortByUVSize<PrimToMap>);

        int res = resultFailure;
        if (flags & allocTryNoGrow)
        {
          res = AllocAllPrimsInner<Arrays, Allocators, MPTAAMAllocNoGrow> (
            arrays, allocs, result, testOrder, primsToMap);
        }
        if (!res && (flags & allocTryNormal))
        {
          res = AllocAllPrimsInner<Arrays, Allocators, MPTAAMAlloc> (
            arrays, allocs, result, testOrder, primsToMap);
        }

        if (res) 
        {
          arraysFirst += tryCount;
          if (progress)
          {
            u += tryCount;
            progress->IncProgress (progressStep * (u / updateFreq));
            u = u % updateFreq;
          }
          if (res == resultWithNew) createTestOrder = true;
          break;
        }
        tryCount--;
      }
      // None of the queues could be mapped...
      if (tryCount == 0) return false;
    }
    // All queues mapped successfully
    if (progress) progress->SetProgress (1);
    return true;
  }

  template<bool GrowPO2 = true>
  class AllocLightmapArray
  {
    LightmapPtrDelArray& lightmaps;
  public:
    AllocLightmapArray (LightmapPtrDelArray& lightmaps) : 
      lightmaps (lightmaps) {}

    size_t GetSize() const { return lightmaps.GetSize(); }
    CS::SubRectanglesCompact& Get (size_t n)
    { return lightmaps[n]->GetAllocator(); }
    CS::SubRectanglesCompact& New (size_t& index) 
    { 
      Lightmap *newL = new Lightmap (globalConfig.GetLMProperties ().maxLightmapU,
                                     globalConfig.GetLMProperties ().maxLightmapV);
      index = lightmaps.Push (newL);
      if (!GrowPO2) newL->GetAllocator().SetGrowPO2 (false);
      return newL->GetAllocator();
    }
    void Delete (size_t index)
    {
      lightmaps.DeleteIndex (index);
    }
  };

}

#endif // __LIGHTMAPUV_SIMPLE_UTIL_H__
