/*
    Copyright (C) 2000 Samuel Humphreys

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

#include "iplugin.h"
#include "csutil/scf.h"

SCF_VERSION (iPerfStats, 0, 0, 1);

/**
 * The performance and statistics plugin.
 * This plugin tracks and records all kinds of useful information
 * while the program is running.
 */
struct iPerfStats : public iPlugIn
{
  /// Set whether paused. Returns previous status
  virtual bool Pause (bool pause) = 0;
  ///
  virtual void FinishSection () = 0;
  /// Reset all statistics back to zero
  virtual void ResetStats () = 0;
  /**
   * The frames per second is really the resolution number of frames
   * divided by the time taken to complete them. Set to -1 to disable
   * (the default). The resolution is the time interval in milliseconds
   * in which a frames-per-second value is computed.
   */
  virtual void SetResolution (int iMilSecs) = 0;
  /**
   * Set the name of this section, which will be utilised if the stats are
   * printed to file.
   */
  virtual void SetName (const char *Name) = 0;
  /// Output stats to named file. If summary is true, you'll get a summary.
  virtual void SetOutputFile (const char *Name, bool summary) = 0;

  /// Get the current fps
  virtual float GetFPS () = 0;
  /**
   * Start a new set of performance statistics as a subsection, with an
   * optional name. If you intend to print all the stats to file then this
   * name will identify the subsection.
   */
  virtual iPerfStats *StartNewSubsection (const char *name) = 0;

  /// Finish the subsection. This will DecRef () the subsection.
  virtual void FinishSubsection () = 0;
  /// Is there currently a subsection?
  virtual bool IsSubsection () = 0;
  /**
   * Print this sections current summary stats, where sysflags is MSG_STDOUT etc 
   * as defined in iSystem.h
   */
  virtual void PrintSectionStats (int sysflags) = 0;

  /**
   * Print the subsections current stats, where sysflags is MSG_STDOUT etc as 
   * defined in iSystem.h
   */
  virtual void PrintSubsectionStats (int sysflags) = 0;

  /**
   * When ran with a debugger, this should cause it to break when starting this
   * frame number, if compiled without CS_DEBUG it will have no effect. If you
   * load this plugin last (like in walktest) then set frame_num to one less.
   */
  virtual void DebugSetBreak (int frame_num) = 0;
};
