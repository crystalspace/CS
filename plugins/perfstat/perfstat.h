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

#ifndef __CS_PERFSTAT_H__
#define __CS_PERFSTAT_H__

#include "csutil/util.h"
#include "csutil/csvector.h"
#include "ivaria/perfstat.h"
#include "isys/plugin.h"

struct iSystem;
struct iEngine;

// For more complete comments see iperfstat.h For usage see walktest

class csPerfStats : iPerfStats
{
protected:
  //------------------------------------------------------------------------
  /// Structure which holds frame specific stats (currently only fps)
  struct FrameEntry
  {
    float fps;
  };
  // Define how many frame entries there are, this will be used in calculating
  // string lengths
#define FRAME_ENTRIES 1

  class FrameVector : public csVector
  {
  public:
    FrameVector (int ilimit = 0, int ithreshold = 0)
      : csVector (ilimit, ithreshold) {};
    /// Free a single element of the array
    virtual bool FreeItem (csSome Item)
    { delete (FrameEntry*)Item; return true; }
  };

  //-----------------------------------------------------------------------
  /// Structure which holds general stat info
  struct StatEntry
  {
    char *buf;
    int len;
    int frame_num;
    ~StatEntry ()
    { delete [] buf; }
  };

  class StatVector : public csVector
  {
  public:
    StatVector (int ilimit = 0, int ithreshold = 0)
      : csVector (ilimit, ithreshold) {};
    /// Free a single element of the array
    virtual bool FreeItem (csSome Item)
    { delete (StatEntry*)Item; return true; }
  };
  //------------------------------------------------------------------------

  iSystem *System;
  iEngine *Engine;
  /// The name of the sub/section utilised within an output file
  char *name;
  /// The name of the file to be outputted
  char *file_name;
  /**
   * Multiple concurrent subsections of subsections have increasingly wide
   * margins
   */
  char *margin;
  int indent;
  /// whether paused
  bool paused;
  /// Number of frames per statistic entry
  int resolution;
  /// Frame specified for debugger to break on.
  int break_frame;
  /// Only relevant to head section as it records per resolution frame stats
  bool frame_by_frame;

  /// Organised in a doubly linked list with a reference to the head section
  csPerfStats *head_section;
  csPerfStats *super_section;
  csPerfStats *sub_section;

  /// The sub/section from which stats are to be outputted to file
  csPerfStats *statlog_section;
  /**
   * Vector to hold summary information from all subsections and
   * statlog_section
   */
  StatVector *statvec;
  /// The vector which records stats from every resolution number of frames
  FrameVector *framevec;
  /// Frame specific data
  FrameEntry *frame;

  /// self explanatory
  csTime total_time;
  int frame_num;
  float lowest_fps;
  float highest_fps;
  float mean_fps;

  // Unimplemented.
/*  int total_polygons_considered; */
/*  int total_polygons_rejected; */
/*  int total_polygons_accepted; */
/*  int total_polygons_drawn; */
/*  int total_portals_drawn; */

  csTime frame_start;
  int frame_count;

  // Convenience functions.
  void AccumulateTotals (csTime elapsed_time);
  void CalculateFpsStats ();
  void SubsectionNextFrame (csTime elapsed_time, float fps);
  void SaveStats ();
  void WriteSummaryStats ();
  void WriteMainHeader ();
  void WriteFrameHeader ();
  void WriteSubBegin ();
  void WriteSubSummary ();
  bool WriteFile ();

 public:
  SCF_DECLARE_IBASE;

  csPerfStats (iBase *iParent);
  virtual ~csPerfStats ();
  ///
  virtual bool Initialize (iSystem *system);
  /// This is set to receive the once per frame nothing  event
  virtual bool HandleEvent (iEvent &event);

  /// Set whether paused. Returns previous status
  virtual bool Pause (bool pause);
  ///
  virtual void FinishSection ();
  /// Reset all accumulated statistics back to zero
  virtual void ResetStats ();
  virtual void SetResolution (int iMilSecs);

  virtual void SetName (const char *Name)
  { name = csStrNew (Name); }
  /// Output stats to named file. If summary is true, you'll get a summary.
  virtual void SetOutputFile (const char *Name, bool summary);

  virtual float GetFPS ()
  { return frame->fps; }
  /**
   * Start a new set of performance statistics as a subsection, with an
   * optional name.  If you intend to print all the stats to file then this
   * name will identify the subsection.
   */
  virtual iPerfStats *StartNewSubsection (const char *name);
  /// Finish the subsection. This will DecRef () the subsection.
  virtual void FinishSubsection ();

  virtual bool IsSubsection ()
  { return (sub_section != NULL); }

  /**
   * Print this sections current summary stats, where sysflags is CS_MSG_STDOUT
   * etc as defined in iSystem.h
   */
  virtual void PrintSectionStats (int sysflags);
  virtual void PrintSubsectionStats (int sysflags);
  /**
   * When ran with a debugger, this should cause it to break when starting this
   * frame number, if compiled without CS_DEBUG it will have no effect.  If you
   * load this plugin last (like in walktest) then set frame_num to one less.
   */
  virtual void DebugSetBreak (int frame_num)
  { break_frame = frame_num; }

  struct eiPlugIn : public iPlugIn
  {
    SCF_DECLARE_EMBEDDED_IBASE(csPerfStats);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize(p); }
    virtual bool HandleEvent (iEvent& e) { return scfParent->HandleEvent(e); }
  } scfiPlugIn;
};

#endif // __CS_PERFSTAT_H__
