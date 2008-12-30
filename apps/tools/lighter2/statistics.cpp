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

#include "statistics.h"

namespace lighter
{
  Statistics globalStats;

  Statistics::Progress::Progress (const char* name, float amount, 
    Progress* parent) : parent (parent ? parent : &globalStats.progress),
    taskName (name), totalAmount (0),
    subProgressStart (this->parent->totalAmount), 
    subProgressAmount (amount), progress (0)
  {
    this->parent->totalAmount += amount;
  }

  void Statistics::Progress::SetProgress (float progress, const char* task)
  {
    this->progress = progress;

    if (parent != 0)
    {
      csString displayTask (taskName);
      if (task != 0) 
      {
        displayTask.Append (": ");
        displayTask.Append (task);
      }
      if (parent == &globalStats.progress)
      {
        // Bit hacky.
        globalStats.progress.SetTaskProgress (this);
      }
      float parentAmount = parent->totalAmount;
      if (parentAmount == 0) parentAmount = 1.0f;
      parent->SetProgress ((subProgressStart + progress * subProgressAmount)
        / parentAmount, displayTask);
    }
    else
      globalStats.progress.UpdateProgressDisplay (task);
  }

  float Statistics::Progress::GetFractionFromTaskProgress ()
  {
    float parentFrac, parentAmount;
    if (parent == &globalStats.progress)
    {
      parentFrac = 1.0f;
      parentAmount = subProgressAmount;
    }
    else
    {
      parentFrac = parent->GetFractionFromTaskProgress ();
      parentAmount = parent->totalAmount;
    }
    if (parentAmount == 0) parentAmount = 1.0f;
    return (subProgressAmount / parentAmount) * parentFrac;
  }

  void Statistics::Progress::SetTaskName (const char* taskName)
  {
    this->taskName = taskName;    
  }

  Statistics::Progress* Statistics::Progress::CreateProgress (float amount, 
    const char* name)
  {
    return new Progress (name, progress, amount, this);
  }

  //-------------------------------------------------------------------------

  void Statistics::GlobalProgress::UpdateProgressDisplay (
    const char* taskName)
  {
    if (taskName == 0) return;

    lastUpdatePercentGlobal = int (100.0f * progress);
    int redrawFlags;
    if (this->taskName != taskName)
    {
      this->taskName = taskName;
      redrawFlags = TUI::TUI_DRAW_ALL;
    }
    else
      redrawFlags = TUI::TUI_DRAW_PROGRESS | TUI::TUI_DRAW_SWAPCACHE;

    globalTUI.Redraw (redrawFlags);
  }
}
