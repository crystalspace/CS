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

#include "common.h"

#include "lightmapuv_simple.h"

// Uncomment to dump subrectangles used during PrepareLighting()
//#define DUMP_SUBRECTANGLES

namespace lighter
{

  /* SimpleUVFactoryLayouter::PrepareLighting() implementation.
     Needs a bunch of auxiliary stuff. Gets its one file to reduce
     clutter. */

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
    const size_t arraysFirst, const size_t tryCount)
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
        int res = resultFailure;
        if (flags & allocTryNoGrow)
        {
          res = AllocAllPrimsInner<Arrays, Allocators, MPTAAMAllocNoGrow> (
            arrays, allocs, result, testOrder, arraysFirst, tryCount);
        }
        if (!res && (flags & allocTryNormal))
        {
          res = AllocAllPrimsInner<Arrays, Allocators, MPTAAMAlloc> (
            arrays, allocs, result, testOrder, arraysFirst, tryCount);
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

  class AllocGlobalLM
  {
    LightmapPtrDelArray& globalLightmaps;
  public:
    AllocGlobalLM (LightmapPtrDelArray& globalLightmaps) : 
      globalLightmaps (globalLightmaps) {}

    size_t GetSize() const { return globalLightmaps.GetSize(); }
    CS::SubRectanglesCompact& Get (size_t n)
    { return globalLightmaps[n]->GetAllocator(); }
    CS::SubRectanglesCompact& New (size_t& index) 
    { 
      Lightmap *newL = new Lightmap (globalConfig.GetLMProperties ().maxLightmapU,
                                     globalConfig.GetLMProperties ().maxLightmapV);
      index = globalLightmaps.Push (newL);
      return newL->GetAllocator();
    }
    void Delete (size_t index)
    {
      globalLightmaps.DeleteIndex (index);
    }
  };

  class AllocNewGlobalLM
  {
    LightmapPtrDelArray& globalLightmaps;
  public:
    size_t glmOfs;

    AllocNewGlobalLM (LightmapPtrDelArray& globalLightmaps) : 
      globalLightmaps (globalLightmaps), glmOfs (globalLightmaps.GetSize()) {}

    size_t GetSize() const { return globalLightmaps.GetSize() - glmOfs; }
    CS::SubRectanglesCompact& Get (size_t n)
    { return globalLightmaps[glmOfs + n]->GetAllocator(); }
    CS::SubRectanglesCompact& New (size_t& index) 
    { 
      Lightmap *newL = new Lightmap (globalConfig.GetLMProperties ().maxLightmapU,
                                     globalConfig.GetLMProperties ().maxLightmapV);
      index = globalLightmaps.Push (newL) - glmOfs;
      return newL->GetAllocator();
    }
    void Delete (size_t index)
    {
      globalLightmaps.DeleteIndex (glmOfs + index);
    }
  };

  namespace
  {
    // Not nice, but simple...
    csArray<size_t>* totalAffectedPrimCount;
  };

  int SimpleUVFactoryLayouter::SortPDLQueues (const PDLQueue& p1,
                                              const PDLQueue& p2)
  {
    size_t nb1 = p1.pdBits.NumBitsSet();
    size_t nb2 = p2.pdBits.NumBitsSet();
    if (nb1 > nb2)
      return 1;
    else if (nb1 < nb2)
      return -1;

    size_t c1 = 0;
    size_t c2 = 0;

    for (size_t l = 0; l < p1.pdBits.GetSize(); l++)
    {
      if (p1.pdBits.IsBitSet (l)) 
        c1 += totalAffectedPrimCount->Get (l);
      if (p2.pdBits.IsBitSet (l)) 
        c2 += totalAffectedPrimCount->Get (l);
    }

    if (c1 > c2)
      return -1;
    else if (c1 < c2)
      return 1;

    return 0;
  }

  int SimpleUVFactoryLayouter::SortLQMaps (LayoutedQueue::Map const& s1, 
                                           LayoutedQueue::Map const& s2)
  {
    float dx = s1.alloc->GetRectangle().Width() 
      - s2.alloc->GetRectangle().Width();
    if (dx > 0)
      return -1;
    else if (dx < 0)
      return 1;
    float dy = s1.alloc->GetRectangle().Height() 
      - s2.alloc->GetRectangle().Height();
    if (dy > 0)
      return -1;
    else if (dy < 0)
      return 1;
    return 0;
  }

  /// Allocate a rect of dimension \a w and \h onto one of \a allocs.
  template<class Allocators>
  bool AllocOntoOne (int w, int h, Allocators& allocs,
    CS::SubRectangles::SubRect*& subRect, size_t& nAlloc,
    csRect& rect)
  {
    bool success = false;
    for (size_t a = 0; a < allocs.GetSize(); a++)
    {
      subRect = allocs.Get (a).Alloc (w, h, rect);
      if (subRect != 0)
      {
        success = true;
        nAlloc = a;
        break;
      }
    }
    if (!success)
    {
      CS::SubRectanglesCompact& alloc = allocs.New (nAlloc);
      subRect = alloc.Alloc (w, h, rect);
      success = subRect != 0;
    }
    return success;
  }

  struct GloballyAllocated
  {
    CS::SubRectanglesCompact* globalAlloc;
    CS::SubRectangles::SubRect* globalSR;
    CS::SubRectanglesCompact* srcAlloc;
  };

  void SimpleUVFactoryLayouter::PrepareLighting (Statistics::Progress& progress)
  {
    progress.SetProgress (0);

    Statistics::Progress progressPDLQueues (0, 90, &progress);
    Statistics::Progress progressOntoGlobal (0, 1, &progress);
    Statistics::Progress progressNonPDL (0, 3, &progress);
    size_t u, updateFreq;
    float progressStep;

    totalAffectedPrimCount = new csArray<size_t> ();
    // Sort queues by number of PD lights they're affected by.
    csArray<PDLQueue> allQueues;
    PDQueuesHash::GlobalIterator it (pdQueues.GetIterator());
    while (it.HasNext ())
    {
      PDLQueue& q = allQueues.GetExtend (allQueues.GetSize ());
      q.queue = &it.Next (q.pdBits);

      for (size_t l = 0; l < q.pdBits.GetSize(); l++)
      {
        totalAffectedPrimCount->GetExtend (l, 0) += q.queue->GetSize ();
      }
    }
    allQueues.Sort (SortPDLQueues);
    delete totalAffectedPrimCount;

    // Prims unaffected by PDLs are handled a bit specially.
    csArray<PDLQueue> queuesNoPDL;
    while ((allQueues.GetSize() > 0) && (allQueues[0].pdBits.AllBitsFalse()))
    {
      queuesNoPDL.Push (allQueues[0]);
      allQueues.DeleteIndex (0);
    }

    csArray<LayoutedQueue> layoutedQueues;

    progressPDLQueues.SetProgress (0);
    progressStep = 1.0f / allQueues.GetSize();
    for (size_t q = 0; q < allQueues.GetSize(); q++)
    {
      progressPDLQueues.SetProgress (q * progressStep);

      PDLQueue& currentQueue = allQueues[q];
      LayoutedQueue newEntry;
      newEntry.pdBits = currentQueue.pdBits;

      /* Look for layouted queues whose PD lights are a subset of this one.
         These layouted queues are then "merged" into the new layouted queue
         before the current primitives are laid out.
         The motivation is to get primitives that have similar PD light 
         affection closer together.
        */
      for (size_t l = layoutedQueues.GetSize(); l-- > 0; )
      {
        LayoutedQueue& currentLayouted = layoutedQueues[l];
        if ((currentLayouted.pdBits & ~newEntry.pdBits).AllBitsFalse ()
          && !((currentLayouted.pdBits & newEntry.pdBits).AllBitsFalse ()))
        {
          /**
           * Layouted queue matches, reserve space on one of the allocs on the
           * new layouted queue.
           */
          for (size_t a = 0; a < currentLayouted.maps.GetSize(); a++)
          {
            const LayoutedQueue::Map& map = currentLayouted.maps[a];
            const csRect& rect = map.alloc->GetRectangle ();
            csRect newRect;
            /// Allocated SR
            CS::SubRectangles::SubRect* sr;
            /// Position of rectangle
            csVector2 ofs;
            /**
             * Allocator of new queue on which the layouted queue rect was
             * allocated. */
            size_t nAlloc;
            AllocLQ allocLQ (newEntry);
            bool b = AllocOntoOne (rect.Width(), rect.Height(),
              allocLQ, sr, nAlloc, newRect);
            CS_ASSERT(b);
            (void)b;
            ofs.Set (newRect.xmin, newRect.ymin);

            newEntry.maps[nAlloc].alloc->PlaceInto (map.alloc, sr);

            /// Copy all the positions from the old LQ to the new
            for (size_t q = 0; q < map.queues.GetSize(); q++)
            {
              const LayoutedQueue::Map::Queue& oldMapQueue = map.queues[q];
              LayoutedQueue::Map& newMap = 
                newEntry.maps.GetExtend (nAlloc);
              LayoutedQueue::Map::Queue& newMapQueue = 
                newMap.queues.GetExtend (newMap.queues.GetSize());
              newMapQueue.srcQueue = oldMapQueue.srcQueue;

              const csArray<csVector2>& srcPos = oldMapQueue.positions;
              csArray<csVector2>& positions = newMapQueue.positions;
              for (size_t v = 0; v < srcPos.GetSize(); v++)
              {
                const csVector2& unmappedPos = srcPos[v];
                const csVector2 mappedPos = unmappedPos + ofs;
                positions.Push (mappedPos);
              }
            }
            delete map.alloc;
          }
          layoutedQueues.DeleteIndex (l);
        }
      }

      /* Allocate current prims.
         'Placement' above will result in a couple of small(er) rectangles
         from which the allocator tries to acquire space. 
         Doing the prims allocation before placement can lead to bigger areas
         be available. OTOH, doing allocation afterwards allows for 
         "filling gaps".
         The nicest way would be filling gaps + large rects to alloc from
         available ... unfortunately that's not trivial to achieve due the way
         CS::SubRectanglesCompact works.
       */
      AllocResultHash results;
      AllocLQ allocLQ (newEntry);
      Statistics::Progress* allocCurrentProg = 
        progressPDLQueues.CreateProgress (progressStep * 0.95f);
      bool b = AllocAllPrims (ArraysQPDPA (*currentQueue.queue), 
        allocLQ, results, allocCurrentProg, allocDefault);
      delete allocCurrentProg;
      CS_ASSERT(b);
      (void)b;

#if defined(DUMP_SUBRECTANGLES)
      static int counter = 0;
      for (size_t a = 0; a < newEntry.maps.GetSize(); a++)
      {
        csString str;
        str.Format ("newentry_%d_%zu", counter, a);
        newEntry.maps[a].alloc->Dump (str);
      }
      counter++;
#endif

      AllocResultHash::GlobalIterator it (results.GetIterator ());
      while (it.HasNext())
      {
        size_t queueIndex;
        AllocResult& result = it.Next (queueIndex);

        LayoutedQueue::Map& newMap = 
          newEntry.maps.GetExtend (result.allocIndex);
        LayoutedQueue::Map::Queue& mapQueue = 
          newMap.queues.GetExtend (newMap.queues.GetSize());

        mapQueue.srcQueue = &(currentQueue.queue->Get (queueIndex));
        mapQueue.positions = result.positions;
      }

      layoutedQueues.Insert (0, newEntry);
    }
    progressPDLQueues.SetProgress (1);

    progressOntoGlobal.SetProgress (0);
    if (layoutedQueues.GetSize() > 0)
    {
      u = updateFreq = progressOntoGlobal.GetUpdateFrequency (
        layoutedQueues.GetSize());
      progressStep = updateFreq * (1.0f / layoutedQueues.GetSize());
    }

    csArray<GloballyAllocated> queuesAllocated;
    // Finally allocate onto global lightmaps...
    for (size_t l = layoutedQueues.GetSize(); l-- > 0; )
    {
      LayoutedQueue& currentLayouted = layoutedQueues[l];

      for (size_t a = 0; a < currentLayouted.maps.GetSize(); a++)
      {
        CS::SubRectanglesCompact& allocator = *currentLayouted.maps[a].alloc;
        csRect minRect (allocator.GetMinimumRectangle());
        allocator.Shrink (minRect.Width(), minRect.Height());
      }

      currentLayouted.maps.Sort (SortLQMaps);

#if defined(DUMP_SUBRECTANGLES)
      for (size_t a = 0; a < currentLayouted.maps.GetSize(); a++)
      {
        csString str;
        str.Format ("l%zua%zu", l, a);
        currentLayouted.maps[a].alloc->Dump (str);
      }
#endif

      AllocResultHash results;
      ArraysOneLQ allocArrays (currentLayouted);
      AllocGlobalLM allocGLM (globalLightmaps);
      size_t glmOfs = 0;
      // Look if there is some space to spare somewhere anyway...
      bool b = AllocAllPrims (allocArrays, allocGLM, results, 0, allocTryNoGrow);
      if (!b)
      {
        // ...otherwise distribute onto to some new lightmaps.
        AllocNewGlobalLM allocNewGLM (globalLightmaps);
        b = AllocAllPrims (allocArrays, allocNewGLM, results, 0, allocTryNormal);
        glmOfs = allocNewGLM.glmOfs;
      }
      CS_ASSERT(b);
      (void)b;

      AllocResultHash::GlobalIterator it (results.GetIterator ());
      while (it.HasNext())
      {
        size_t queueIndex;
        AllocResult& result = it.Next (queueIndex);
        const LayoutedQueue::Map& currentMap = currentLayouted.maps[queueIndex];

        size_t glmIndex = result.allocIndex + glmOfs;
        GloballyAllocated ga;
        ga.globalAlloc = &globalLightmaps[glmIndex]->GetAllocator();
        ga.globalSR = result.subRects[0];
        ga.srcAlloc = currentMap.alloc;
        queuesAllocated.Push (ga);

        int x = int (result.positions[0].x);
        int y = int (result.positions[0].y);
        for (size_t q = 0; q < currentMap.queues.GetSize(); q++)
        {
          const LayoutedQueue::Map::Queue& mapQueue = currentMap.queues[q];
          const QueuedPDPrimitives* queue = mapQueue.srcQueue;
          queue->layouter->LayoutQueuedPrims (*queue->prims, queue->groupNum, 
            glmIndex, mapQueue.positions, x, y);
        }
      }
    }

    for (size_t g = 0; g < queuesAllocated.GetSize(); g++)
    {
      GloballyAllocated& ga = queuesAllocated[g];
      ga.globalAlloc->PlaceInto (ga.srcAlloc, ga.globalSR);
    }

    for (size_t l = 0; l < layoutedQueues.GetSize(); l++)
    {
      LayoutedQueue& currentLayouted = layoutedQueues[l];
      for (size_t m = 0; m < currentLayouted.maps.GetSize(); m++)
      {
        delete currentLayouted.maps[m].alloc;
      }
    }
    progressOntoGlobal.SetProgress (1);

    progressNonPDL.SetProgress (0);
    if (queuesNoPDL.GetSize() > 0)
    {
      u = updateFreq = progressNonPDL.GetUpdateFrequency (
        queuesNoPDL.GetSize());
      progressStep = updateFreq * (1.0f / queuesNoPDL.GetSize());
    }

    // Distribute unaffected prims to the space that's left
    for (size_t q = 0; q < queuesNoPDL.GetSize(); q++)
    {
      PDLQueue& currentQueue = queuesNoPDL[q];

      AllocResultHash results;
      AllocGlobalLM allocGLM (globalLightmaps);
      bool b = AllocAllPrims (ArraysQPDPA (*currentQueue.queue), 
        allocGLM, results, 0, allocDefault);
      CS_ASSERT(b);
      (void)b;

      AllocResultHash::GlobalIterator it (results.GetIterator ());
      while (it.HasNext())
      {
        size_t queueIndex;
        AllocResult& result = it.Next (queueIndex);
        const QueuedPDPrimitives& queue = currentQueue.queue->Get (queueIndex);
        queue.layouter->LayoutQueuedPrims (*queue.prims, queue.groupNum, 
          result.allocIndex, result.positions, 0, 0);
        if (--u == 0)
        {
          progressNonPDL.IncProgress (progressStep);
          u = updateFreq;
        }
      }
    }
    progressNonPDL.SetProgress (1);

    for (size_t g = 0; g < globalLightmaps.GetSize(); g++)
    {
      Lightmap* lm = globalLightmaps[g];
      lm->UpdateDimensions();

#if defined(DUMP_SUBRECTANGLES)
      csString str;
      str.Format ("glm%zu", g);
      lm->GetAllocator().Dump (str);
#endif
    }

    progress.SetProgress (1);
  }
 

}

