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

#ifndef __PERFSTAT_H__
#define __PERFSTAT_H__

#include "csutil/util.h"
#include "csutil/csvector.h"
#include "iperstat.h"

struct iSystem;
class csWorld;

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
  csWorld *World;
  char *name;
  char *file_name;
  char *margin;
  int indent;
  csPerfStats *head_section;

  bool paused;
  int resolution;
  int break_frame;

  csPerfStats *super_section;
  csPerfStats *sub_section;

  bool frame_by_frame;
  csPerfStats *statlog_section;
  StatVector *statvec;
  FrameVector *framevec;
  /// Frame specific data
  FrameEntry *frame;

  time_t total_time;
  int frame_num;
  float lowest_fps;
  float highest_fps;
  float mean_fps;

/*    int total_polygons_considered; */
/*    int total_polygons_rejected; */
/*    int total_polygons_accepted; */
/*    int total_polygons_drawn; */
/*    int total_portals_drawn; */

  int cnt;
  time_t time0;

  void AccumulateTotals (time_t elapsed_time);
  void CalculateFpsStats ();
  void SubsectionNextFrame (time_t elapsed_time, float fps);
  void SaveStats ();
  void WriteSummaryStats ();
  void WriteMainHeader ();
  void WriteFrameHeader ();
  void WriteSubBegin ();
  void WriteSubSummary ();
  bool WriteFile ();

 public:
  DECLARE_IBASE;

  csPerfStats (iBase *iParent);
  virtual ~csPerfStats ();

  virtual bool Initialize (iSystem *system);
  virtual bool HandleEvent (csEvent &event);

  virtual void StartSection ()
  { paused = false; }
  virtual bool Pause (bool pause);
  virtual void FinishSection ();
  virtual void ResetStats ();
  virtual void PrintSectionStats (int sysflags);

  virtual iPerfStats *StartNewSubsection (const char *name);
  virtual void PrintSubsectionStats (int sysflags);
  virtual void FinishSubsection ();

  virtual bool IsSubsection ()
  { return (sub_section != NULL); }

  virtual void SetResolution (int res);
  virtual void DebugSetBreak (int frame_num)
  { break_frame = frame_num; }
  virtual void SetName (const char *Name)
  { name = strnew (Name); }

  virtual void SetOutputFile (const char *Name, bool summary);

  virtual float GetFPS ()
  { return frame->fps; }
};


#endif // __PERFSTAT_H__
