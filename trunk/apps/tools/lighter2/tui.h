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

#ifndef __TUI_H__
#define __TUI_H__

#include "csutil/scf_implementation.h"
#include "csutil/threadmanager.h"

#include "ivaria/reporter.h"

namespace lighter
{
  /// Text-mode User Interface
  class TUI : public ThreadedCallable<TUI>,
              public scfImplementation1<TUI, iReporterListener>
  {
  public:
    TUI ();

    // Setup
    void Initialize (iObjectRegistry* objReg);

    enum RedrawFlag
    {
      TUI_DRAW_CLEAR = (1<<0),
      TUI_DRAW_STATIC = (1<<2),
      TUI_DRAW_PROGRESS = (1<<3),
      TUI_DRAW_MESSAGES = (1<<4),
      TUI_DRAW_RAYCORE = (1<<5),
      TUI_DRAW_SETTINGS = (1<<6),
      TUI_DRAW_STATS = (1<<7),
      TUI_DRAW_SWAPCACHE = (1<<8),
      TUI_DRAW_ALL = 0xFFFFFFFF
    };


    /// Redraw the TUI
    void Redraw (int drawFlags = TUI_DRAW_ALL);

    /// Finish drawing
    void FinishDraw ();

    /// iReporterListener
    THREADED_CALLABLE_DECL4(TUI, Report, csThreadReturn, iReporter*, reporter,
      int, severity, const char*, msgId, const char*, description, HIGH, true, false);
  private:
    csString GetProgressBar (uint percent) const;

    //Draw parts of the TUI
    void DrawStatic () const;

    //Draw current state
    void DrawProgress () const;

    //Draw message buffers
    void DrawMessage () const;

    //Draw raytracer stats
    void DrawRayCore () const;

    //Draw settings
    void DrawSettings () const;

    //Draw stats
    void DrawStats () const;

    //Draw swap cache stats
    static csString FormatByteSize (uint64 size);
    void DrawSwapCacheStats () const;

    //Update simple output, if anything to update
    void DrawSimple ();
    void DrawSimpleEnd ();

    // Reporting stuff
    csString messageBuffer[4];
    uint messageBufferEnd;

    // Setting determining simple output mode instead of full TUI
    bool simpleMode, prevWasReporter;

    // Last drawn stuff in simple-mode
    size_t kdLastNumNodes;

    // Last printed task
    csString lastTask; float lastTaskProgress;

    iObjectRegistry* GetObjectRegistry() const { return object_reg; }
    iObjectRegistry* object_reg;
  };

  extern TUI globalTUI;
}

#endif
