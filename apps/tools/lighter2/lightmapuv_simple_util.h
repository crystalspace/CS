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
    csArray<CS::SubRectangles::SubRect*>& outSubRects, 
    bool failDump = false)
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
        if (failDump)
        {
          csString str;
          str.Format ("MapPrimsToAlloc_fail_%d", MapPrimsToAlloc_counter++);
          alloc.Dump (str);
        }
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

  template<class Allocators>
  static void UndoAllocations (Allocators& allocs, const 
    AllocResultHash& result)
  {
    AllocResultHash::ConstGlobalIterator resultIt (result.GetIterator ());
    while (resultIt.HasNext())
    {
      const AllocResult& oneRes = resultIt.Next ();
      for (size_t i = 0; i < oneRes.subRects.GetSize(); i++)
        allocs.Get (oneRes.allocIndex).Reclaim (oneRes.subRects[i]);
    }
  }

  enum
  {
    resultFailure,
    resultAllocated,
    resultWithNew
  };

  template<class Arrays, class Allocators, class AllocMixin>
  static int AllocAllPrimsInner (const Arrays& arrays, Allocators& allocs,
    AllocResultHash& result, const csArray<SizeAndIndex>& testOrder,
    csArray<PrimToMap>& primsToMap, bool failDump = false)
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
    {
      allocs.Delete (allocator);
#if defined(DUMP_SUBRECTANGLES)
      if (failDump)
      {
        allMapped = MapPrimsToAlloc<AllocMixin> (primsToMap, 
          allocs.New (allocator), subRects, true);
        allocs.Delete (allocator);
      }
#endif
    }
    return resultFailure;
  }

  static void MergeResultHash (AllocResultHash& to, const AllocResultHash& from)
  {
    AllocResultHash::ConstGlobalIterator fromIt (from.GetIterator ());
    while (fromIt.HasNext())
    {
      size_t key;
      const AllocResult& fromRes = fromIt.Next (key);
#ifdef CS_DEBUG
      AllocResult* toRes = to.GetElementPointer (key);
      CS_ASSERT(toRes == 0);
#endif
      to.Put (key, fromRes);
    }
  }

  const uint allocTryNoGrow = 1;
  const uint allocTryNormal = 2;
  const uint allocDefault = allocTryNoGrow | allocTryNormal;

  /// Allocate all queued primitves onto allocators from \a allocs.
  template<class Arrays, class Allocators>
  static bool AllocAllPrims (const Arrays& arrays, Allocators& allocs,
    AllocResultHash& result, Statistics::Progress* progress, uint flags)
  {
    size_t u = 0, updateFreq = 0;
    float progressStep = 0;
    if (progress && (arrays.GetSize() > 0))
    {
      updateFreq = progress->GetUpdateFrequency (arrays.GetSize());
      u = 0;
      progressStep = updateFreq * (1.0f / arrays.GetSize());
      progress->SetProgress (0);
    }
    bool createTestOrder = true;
    csArray<SizeAndIndex> testOrder;

    result.DeleteAll();

    size_t arraysFirst = 0;
    while (arraysFirst < arrays.GetSize())
    {
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

      /* Try to layout as much primitive queues onto a single map at once as
         possible.
         The number of queues to be layouted is determined by a binary-search
         style algorithm: half the number of queues allocated until they all
         fit, then try halfway between that number and the largest last 
         non-working amount. Recurse until right number is found. 
       */
      size_t tryMin = arraysFirst;
      size_t tryMax = arrays.GetSize();
      size_t lastTryMax = tryMax+1;
      while (tryMin <= tryMax)
      {
        size_t tryCount = tryMax - arraysFirst;
        if (tryCount == 0)
          // If we get here none of the queues could be mapped...
          return false;

        csArray<PrimToMap> primsToMap;
        for (size_t a = 0; a < tryCount; a++)
        {
          typename Arrays::ArrayType queue = arrays.Get (a+arraysFirst);
          primsToMap.SetCapacity (primsToMap.GetSize() + queue.GetSize());
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

#if defined(DUMP_SUBRECTANGLES)
        const bool doFailDump = tryCount == 1;
#else
        const bool doFailDump = false;
#endif
        int res = resultFailure;
        AllocResultHash thisResult;
        if (flags & allocTryNoGrow)
        {
          res = AllocAllPrimsInner<Arrays, Allocators, MPTAAMAllocNoGrow> (
            arrays, allocs, thisResult, testOrder, primsToMap, doFailDump);
        }
        if (!res && (flags & allocTryNormal))
        {
          res = AllocAllPrimsInner<Arrays, Allocators, MPTAAMAlloc> (
            arrays, allocs, thisResult, testOrder, primsToMap, doFailDump);
        }

        if (res) 
        {
          if (res == resultWithNew) createTestOrder = true;

          tryMin = tryMax;
          tryMax = lastTryMax - 1;

          if (tryMin >= tryMax) 
          {
            MergeResultHash (result, thisResult);
            break;
          }

          UndoAllocations (allocs, thisResult);
          allocs.CleanEmpty ();
          thisResult.DeleteAll ();
        }
        else
        {
          lastTryMax = tryMax;
          tryMax = (tryMin + tryMax) / 2;
        }
      }
      if (progress)
      {
        u += tryMax - arraysFirst;
        progress->IncProgress (progressStep * (u / updateFreq));
        u = u % updateFreq;
      }
      arraysFirst = tryMax;
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
    void CleanEmpty ()
    {
      size_t i = 0;
      while (i < lightmaps.GetSize())
      {
        if (lightmaps[i]->GetAllocator().IsEmpty())
        {
          lightmaps.DeleteIndex (i);
        }
        else
          i++;
      }
    }
  };

}

#endif // __LIGHTMAPUV_SIMPLE_UTIL_H__
