/*
  Copyright (C) 2006 by Marten Svanfeldt

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

#ifndef __STATISTICS_H__
#define __STATISTICS_H__

#include "config.h"
#include "tui.h"

namespace lighter
{
  /// Global statistics object
  class Statistics
  {
  public:
    class GlobalProgress;

    /// Progress for a single task.
    class Progress : public CS::Memory::CustomAllocated
    {
    protected:
      friend class GlobalProgress;

      Progress* parent;
      /// Task description
      csString taskName;

      /// Total amount of all sub-progress objects
      float totalAmount;
      float subProgressStart, subProgressAmount;
      float progress;

      Progress (const char* name, float progStart, float progAmount, 
        Progress* parent) : parent (parent), taskName (name), 
        totalAmount (progAmount), subProgressStart (progStart), 
        subProgressAmount (progAmount), progress (0) {}

      void SetProgress (float progress, const char* task);

      float GetFractionFromTaskProgress ();
    public:
      /**
       * Create a "sub-progress" object for a task. \a name is the 
       * description of the task and \a amount a relavive value for the
       * amount of work this task needs. It doesn't have a unit and doesn't
       * need to sum up to some special value; it can be completely arbitrary,
       * but the amounts of different tasks should be roughly in relation to
       * the time usually takes to complete them.
       */
      Progress (const char* name, float amount, Progress* parent = 0);

      /// Set complete progress, normalized.
      void SetProgress (float progress)
      { SetProgress (progress, 0); }

      /// Increment progress
      inline void IncProgress (float inc)
      {
        SetProgress (progress + inc);
      }

      /// Set description
      void SetTaskName (const char* taskName);

      /// Create a sub-progress object. \a amount is normalized.
      Progress* CreateProgress (float amount, const char* name = 0);

      /**
       * Given \a n items returns after how many items the progress can be
       * updated to get a visible difference in the percentage. 
       */
      size_t GetUpdateFrequency (size_t n)
      {
        size_t f = size_t ((float(n) / (GetFractionFromTaskProgress() * 100.0f)) + 0.5f);
        if (f == 0) return 1;
        if (f > n) return n;
        return f;
      }
    };

    class GlobalProgress : public Progress
    {
      friend class Progress;

      int lastUpdatePercentGlobal, lastUpdatePercentTask;

      void UpdateProgressDisplay (const char* taskName);
      void SetTaskProgress (Progress* prog)
      {
        lastUpdatePercentTask = int (prog->progress * 100.0f);
      }
    public:
      GlobalProgress () : Progress (0, 0, 1.0f, 0), 
        lastUpdatePercentGlobal (-1), lastUpdatePercentTask (-1) {}

      const char* GetTaskName() const { return taskName; }
      int GetOverallProgress() const { return lastUpdatePercentGlobal; }
      int GetTaskProgress() const { return lastUpdatePercentTask; }
    };
    GlobalProgress progress;

    class ProgressState
    {
      Statistics::Progress& progress;
      size_t updateFreq;
      size_t u;
      float progressStep;

    public:
      ProgressState (Statistics::Progress& progress, size_t total) : 
        progress (progress), 
        updateFreq (progress.GetUpdateFrequency (total)), u (updateFreq),
        progressStep (float (updateFreq) / total) {}

      CS_FORCEINLINE void Advance ()
      {
        if (--u == 0)
        {
          progress.IncProgress (progressStep);
          u = updateFreq;
          globalTUI.Redraw (TUI::TUI_DRAW_RAYCORE | TUI::TUI_DRAW_PMCORE);
        }
      }
    };

    struct Raytracer
    {
      Raytracer ()
        : numRays (0), numEyeRays (0), numLightRays (0),
          numReflectionRays (0), numRefractionRays (0),
          numShadowRays (0)
      {}

      /// Total number of rays traced
      uint64 numRays;

      /// Number of rays shot from eye for direct lighting
      uint64 numEyeRays;

      /// Number of rays shot from lights for photon mapping
      uint64 numLightRays;

      /// Number of rays generated as the result of reflection
      uint64 numReflectionRays;

      /// Number of rays generated as the results of refraction
      uint64 numRefractionRays;

      /// Number of rays generated to test for occlusion of light sources
      uint64 numShadowRays;

      /// Number of rays generated to gather photons for indirect lighting
      uint64 numFinalGatherRays;
    } raytracer;

    struct Photonmapping
    {
      Photonmapping ()
        : numStoredPhotons(0), numKDLookups(0),
          KDTreeDepth(0), irCachePrimary(0),
          irCacheSecondary(0), irCacheSplits(0),
          irCacheLookups(0)
      {}

      /// Total number of photons stored (different from traced)
      uint64 numStoredPhotons;

      /// Number of KD tree lookups
      uint64 numKDLookups;

      /// Depth of the photon map KD tree (after balancing
      uint64 KDTreeDepth;

      /// Primary samples in the IR Cache
      uint64 irCachePrimary;

      /// Secondary evaluations from the IR cache
      uint64 irCacheSecondary;

      /// Number of nodes splits in the IR cache octree
      uint64 irCacheSplits;

      /// Number of lookups into the IR cache
      uint64 irCacheLookups;
    } photonmapping;

    struct Scene
    {
      Scene ()
        : numSectors (0), numObjects (0), numLights (0)
      {}

      /// Number of sectors in world
      size_t numSectors;

      /// Number of objects in world
      size_t numObjects;

      /// Number of lights in world
      size_t numLights;
    } scene;

    struct KDTree
    {
      KDTree ()
        : numNodes (0), leafNodes (0), maxDepth (0), sumDepth (0),
        numPrimitives (0)
      {}

      /// Number of inner nodes
      size_t numNodes;

      /// Number of leaf nodes
      size_t leafNodes;

      /// Max depth of leaf node
      size_t maxDepth;

      /// Sum depth of leaf-nodes
      size_t sumDepth;

      /// Total number of primitives in leafs
      size_t numPrimitives;     
    } kdtree;
  };

  extern Statistics globalStats;


  
}

#endif
