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
    // Some helpers
    inline void SetTaskProgress (const char* task, float taskProgress)
    {
      progress.taskProgress = taskProgress;
      progress.taskName = task;

      globalTUI.Redraw ();
    }

    inline void IncTaskProgress (float inc)
    {
      progress.taskProgress += inc;
      globalTUI.Redraw (TUI::TUI_DRAW_PROGRESS);
    }

    inline void SetTotalProgress (float totalProgress)
    {
      progress.overallProgress = totalProgress;
      globalTUI.Redraw ();
    }

    struct Progress
    {
      Progress () 
        : overallProgress (0), taskProgress (0)
      {
      }

      /// Overall progress  0-1000
      float overallProgress;

      /// Current task progress 0-1000
      float taskProgress;

      /// Task description
      csString taskName;
    } progress;

    struct Raytracer
    {
      Raytracer ()
        : numRays (0), usRaytracing (0)
      {}

      /// Number of rays traced
      uint64 numRays;

      /// Number of uS spent raytracing
      uint64 usRaytracing;
    } raytracer;
  };

  extern Statistics globalStats;


  
}

#endif