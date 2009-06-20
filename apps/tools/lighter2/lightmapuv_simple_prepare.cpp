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

#include "lightmapuv_simple_util.h"

namespace lighter
{

  /* SimpleUVFactoryLayouter::PrepareLighting() implementation.
     Needs a bunch of auxiliary stuff. Gets its one file to reduce
     clutter. */

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
    void CleanEmpty ()
    {
      size_t i = glmOfs;
      while (i < globalLightmaps.GetSize())
      {
        if (globalLightmaps[i]->GetAllocator().IsEmpty())
        {
          globalLightmaps.DeleteIndex (i);
        }
        else
          i++;
      }
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
    if (p1.sector != p2.sector)
      return csComparator<Sector*, Sector*>::Compare (p1.sector, p2.sector);

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
    CS_ASSERT_MSG("You can only prepare a UV layouter once", !prepared);
    prepared = true;
    progress.SetProgress (0);

    Statistics::Progress progressPDLQueues (0, 90, &progress);
    Statistics::Progress progressOntoGlobal (0, 1, &progress);
    Statistics::Progress progressNonPDL (0, 3, &progress);
    size_t u = 0, updateFreq = 0;
    float progressStep;

    totalAffectedPrimCount = new csArray<size_t> ();
    // Sort queues by number of PD lights they're affected by.
    csArray<PDLQueue> allQueues;
    PDQueuesHash::GlobalIterator it (pdQueues.GetIterator());
    while (it.HasNext ())
    {
      PDLQueue& q = allQueues.GetExtend (allQueues.GetSize ());
      SectorAndPDBits s;
      q.queue = &it.Next (s);
      q.sector = s.sector;
      q.pdBits = s.pdBits;

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
      newEntry.sector = currentQueue.sector;

      /* Look for layouted queues whose PD lights are a subset of this one.
         These layouted queues are then "merged" into the new layouted queue
         before the current primitives are laid out.
         The motivation is to get primitives that have similar PD light 
         affection closer together.

         Since PD lights are stored in the sector we can only compare PD bits
         when both queues originated in the same sector.
        */
      for (size_t l = layoutedQueues.GetSize(); l-- > 0; )
      {
        LayoutedQueue& currentLayouted = layoutedQueues[l];
        if ((currentLayouted.sector == newEntry.sector)
          && (currentLayouted.pdBits & ~newEntry.pdBits).AllBitsFalse ()
          && !((currentLayouted.pdBits & newEntry.pdBits).AllBitsFalse ()))
        {
          /**
           * Layouted queue matches, reserve space on one of the allocs on the
           * new layouted queue.
           */
          for (size_t a = 0; a < currentLayouted.maps.GetSize(); a++)
          {
      #if defined(DUMP_SUBRECTANGLES)
            static int counter = 0;
      #endif
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

      #if defined(DUMP_SUBRECTANGLES)
            {
              csString str;
              str.Format ("placed_before_%zu_%d", nAlloc, counter);
              newEntry.maps[nAlloc].alloc->Dump (str);
            }
      #endif
            newEntry.maps[nAlloc].alloc->PlaceInto (map.alloc, sr);
      #if defined(DUMP_SUBRECTANGLES)
            {
              csString str;
              str.Format ("placee_%zu_%d", nAlloc, counter);
              map.alloc->Dump (str);
              str.Format ("placed_after_%zu_%d", nAlloc, counter);
              newEntry.maps[nAlloc].alloc->Dump (str);
              counter++;
            }
      #endif

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

      CS_ASSERT(b);
      (void)b;

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
      AllocLightmapArray<> allocGLM (globalLightmaps);
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
          queue->layouter->LayoutQueuedPrims (*queue->prims, queue->layoutID,
            queue->groupNum, glmIndex, mapQueue.positions, x, y);
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
      AllocLightmapArray<> allocGLM (globalLightmaps);
      bool b = AllocAllPrims (ArraysQPDPA (*currentQueue.queue), 
        allocGLM, results, 0, allocDefault);
      if (!b)
      {
        /* It can happen that layouting onto a PO2-growing LM results
           in a different layout compared to non-PO2 growing. So try
           one more time with non-PO2 growing. */
        AllocLightmapArray<false> allocGLMnpo2 (globalLightmaps);
        b = AllocAllPrims (ArraysQPDPA (*currentQueue.queue), 
          allocGLMnpo2, results, 0, allocTryNormal);
      }
#if defined(DUMP_SUBRECTANGLES)
      if (!b)
      {
        for (size_t g = 0; g < globalLightmaps.GetSize(); g++)
        {
          Lightmap* lm = globalLightmaps[g];
          csString str;
          str.Format ("glm%zu", g);
          lm->GetAllocator().Dump (str);
        }
      }
#endif
      CS_ASSERT(b);
      (void)b;

      AllocResultHash::GlobalIterator it (results.GetIterator ());
      while (it.HasNext())
      {
        size_t queueIndex;
        AllocResult& result = it.Next (queueIndex);
        const QueuedPDPrimitives& queue = currentQueue.queue->Get (queueIndex);
        queue.layouter->LayoutQueuedPrims (*queue.prims, queue.layoutID,
          queue.groupNum, result.allocIndex, result.positions, 0, 0);
      }
      if (--u == 0)
      {
        progressNonPDL.IncProgress (progressStep);
        u = updateFreq;
      }
    }
    progressNonPDL.SetProgress (1);

    for (size_t g = 0; g < globalLightmaps.GetSize(); g++)
    {
      Lightmap* lm = globalLightmaps[g];
      lm->UpdateDimensions();
      // Fixup in case we did a non-PO2-grow distribution above
      lm->Grow (csFindNearestPowerOf2 (lm->GetWidth()), 
        csFindNearestPowerOf2 (lm->GetHeight()));

#if defined(DUMP_SUBRECTANGLES)
      csString str;
      str.Format ("glm%zu", g);
      lm->GetAllocator().Dump (str);
#endif
    }

    if (globalConfig.GetLighterProperties().directionalLMs)
    {
      size_t numLMs = globalLightmaps.GetSize();
      for (size_t i = 0; i < 3; i++)
      {
        for (size_t g = 0; g < numLMs; g++)
        {
          globalLightmaps.Push (new Lightmap (*(globalLightmaps[g])));
        }
      }
    }

    progress.SetProgress (1);
  }
 

}

