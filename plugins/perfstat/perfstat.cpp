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

#include <string.h>
#define CS_SYSDEF_PROVIDE_PATH
#include "cssysdef.h"
#include "cssys/system.h"
#include "csver.h"
#include "csutil/scf.h"
#include "perfstat.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "isys/system.h"
#include "isys/vfs.h"
#include "isys/event.h"
#include "isys/plugin.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "iengine/engine.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csPerfStats)

SCF_EXPORT_CLASS_TABLE (perfstat)
  SCF_EXPORT_CLASS (csPerfStats, "crystalspace.utilities.perfstat",
    "Performance statistics utility")
SCF_EXPORT_CLASS_TABLE_END

SCF_IMPLEMENT_IBASE (csPerfStats)
  SCF_IMPLEMENTS_INTERFACE (iPerfStats)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csPerfStats::eiPlugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csPerfStats::csPerfStats (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
  Engine = NULL;
  file_name = NULL;
  statlog_section = NULL;
  statvec = NULL;
  framevec = NULL;
  margin = NULL;
  indent = 0;
  frame = new FrameEntry ();
  frame_by_frame = false;
  break_frame = -1;
  paused = false;
  frame_start = 0;
  frame_count = 0;
  ResetStats ();
}

csPerfStats::~csPerfStats ()
{
  delete [] name;
  delete [] file_name;
  delete [] margin;
  delete frame;
}

bool csPerfStats::Initialize (iObjectRegistry *object_reg)
{
  csPerfStats::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  iSystem* sys = CS_GET_SYSTEM (object_reg);	//@@@
  if (!sys->CallOnEvents (&scfiPlugin, CSMASK_Nothing))
    return false;
  sub_section = super_section = NULL;
  // default resolution
  resolution = 500;
  name = NULL;
  head_section = this;

  return true;
}

bool csPerfStats::HandleEvent (iEvent &event)
{
  if (event.Type != csevBroadcast
   || event.Command.Code != cscmdPostProcess)
    return false;

  if (!paused)
  {
    frame_count++;

    csTicks current_time = csGetTicks ();

    if (!frame_start)
    {
      frame_start = current_time;
      frame_count = 0;
    }

    csTicks elapsed_time = current_time - frame_start;

    AccumulateTotals (current_time - frame_start);
    float new_fps = -1;
    if (elapsed_time > csTicks (resolution))
    {
      frame->fps = new_fps = frame_count ?
        frame_count * 1000.0f / elapsed_time :
        0;
      CalculateFpsStats ();
      frame_start = current_time;
      frame_count = 0;

      if (frame_by_frame)
      {
        framevec->Push (frame);
        FrameEntry *fe = frame;
        frame = new FrameEntry ();
        frame->fps = fe->fps;
      }
    }

    if (sub_section)
      sub_section->SubsectionNextFrame (elapsed_time, new_fps);
  }
  return true; // whatever
}

bool csPerfStats::Pause (bool pause)
{
  if (sub_section)
    sub_section->Pause (pause);
  bool ret = paused;
  paused = pause;
  if (!paused && ret)
  {
    frame_start = csGetTicks ();
    frame_count = 0;
  }
  return ret;
}

void csPerfStats::ResetStats ()
{
  frame_num = 0;
  total_time = 0;

//  total_polygons_considered = 0;
//  total_polygons_rejected = 0;
//  total_polygons_accepted = 0;
//  total_polygons_drawn = 0;
//  total_portals_drawn = 0;

  lowest_fps = 10000;
  highest_fps = 0;
  mean_fps = 0;
  frame->fps = 0;
}

void csPerfStats::SetResolution (int iMilSecs)
{ 
  resolution = iMilSecs;
  frame_start = csGetTicks ();
  frame_count = 0;
  if (sub_section)
    sub_section->SetResolution (iMilSecs);
}

void csPerfStats::SubsectionNextFrame (csTicks elapsed_time, float fps)
{
  AccumulateTotals (elapsed_time);
  if (fps != -1)
  {
    frame->fps = fps;
    CalculateFpsStats ();
  }
  if (sub_section)
    sub_section->SubsectionNextFrame (elapsed_time, fps);
}


void csPerfStats::AccumulateTotals (csTicks elapsed_time)
{
  frame_num++;
#ifdef CS_DEBUG
  if (break_frame == frame_num)
    DEBUG_BREAK;
#endif
  total_time += elapsed_time;
  mean_fps = total_time ? (frame_num * 1000.0f / total_time) : 0;

//  total_polygons_considered += Stats::polygons_considered;
//  total_polygons_rejected += Stats::polygons_rejected;
//  total_polygons_accepted += Stats::polygons_accepted;
//  total_polygons_drawn += Stats::polygons_drawn;
//  total_portals_drawn += Stats::portals_drawn;
}

void csPerfStats::CalculateFpsStats ()
{
  if (frame->fps > highest_fps)
    highest_fps = frame->fps;
  if (frame->fps < lowest_fps)
    lowest_fps = frame->fps;
}

void csPerfStats::PrintSubsectionStats (int severity)
{
  if (sub_section)
    sub_section->PrintSectionStats (severity);
}

void csPerfStats::PrintSectionStats (int severity)
{
  iReporter* rep = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (rep)
  {
    rep->Report (severity, "crystalspace.perfstats", "Total Time/s : %f", ((float)total_time)/1000.0f);
    rep->Report (severity, "crystalspace.perfstats", "Total Frames : %d", frame_num);
    rep->Report (severity, "crystalspace.perfstats", "Mean FPS     : %f", mean_fps);
    rep->Report (severity, "crystalspace.perfstats", "Lowest FPS   : %f", lowest_fps);
    rep->Report (severity, "crystalspace.perfstats", "Highest FPS  : %f", highest_fps);
  }
}

iPerfStats *csPerfStats::StartNewSubsection (const char *name)
{
  if (sub_section)
    return NULL;

  sub_section = new csPerfStats (this);
  sub_section->SetName (name);
  sub_section->object_reg = object_reg;
  sub_section->resolution = resolution;
  sub_section->Engine = Engine;
  sub_section->statlog_section = statlog_section;
  sub_section->super_section = this;
  sub_section->sub_section = NULL;
  sub_section->paused = false;
  sub_section->head_section = head_section;

  int ind = sub_section->indent = indent+2;
  sub_section->margin = new char[ind+1];
  int i;
  for (i=0; i < ind; i++)
     sub_section->margin[i] = ' ';
  sub_section->margin[ind] = 0;

  if (head_section->frame_by_frame)
    sub_section->WriteSubBegin ();

  return (iPerfStats*)sub_section;
}

void csPerfStats::FinishSubsection ()
{
  if (sub_section)
  {
    sub_section->FinishSection ();
    sub_section->DecRef ();
    sub_section = NULL;
  }
}

void csPerfStats::FinishSection ()
{
  paused = true;
  if (sub_section)
    sub_section->FinishSection ();
  if (statlog_section)
    SaveStats ();
}

void csPerfStats::SetOutputFile (const char *Name, bool summary)
{ 
  file_name = csStrNew (Name); 
  statlog_section = this; 
  statvec = new StatVector (30, 100);
  if (!summary)
  {
    head_section->frame_by_frame = true;
    head_section->framevec = new FrameVector (1000, 500);
    WriteFrameHeader ();
  }
}

void csPerfStats::SaveStats ()
{
  if  (statlog_section != this)
    WriteSubSummary ();

  WriteSummaryStats (); 

  if  (statlog_section == this)
  {
    WriteMainHeader ();
    if (!WriteFile ())
      printf ("Stats file output error\n");
  }
}


void csPerfStats::WriteSummaryStats ()
{

  StatEntry *entry = new StatEntry ();
  char buf [] = 
    "\n%sTotal Time   : %f"
    "\n%sTotal Frames : %d"
    "\n%sMean FPS     : %f"
    "\n%sHighest FPS  : %f"
    "\n%sLowest FPS   : %f\n";

  // Shouldnt get numbers as big as 20 chars. as sprintf format defaults 
  // to 6 decimal places (at least on linux)
  int len_guess = strlen (buf) + (20 + indent)*5;
  entry->buf = new char[len_guess];

  sprintf (entry->buf, buf,
	   margin, ((float)total_time)/1000.0f, 
	   margin, frame_num, 
	   margin, mean_fps, 
	   margin, highest_fps, 
	   margin, lowest_fps);

  entry->len = strlen (entry->buf) + 1;
  entry->frame_num = statlog_section->frame_num;

  CS_ASSERT (entry->len <= len_guess);

  statlog_section->statvec->Push (entry);
}

void csPerfStats::WriteMainHeader ()
{
  StatEntry *entry = new StatEntry ();
  iGraphics3D *g3d = CS_QUERY_PLUGIN_ID (plugin_mgr,
  	CS_FUNCID_VIDEO, iGraphics3D);
  if (!g3d) abort ();
  iGraphics2D *g2d = g3d->GetDriver2D ();
  csGraphics3DCaps *caps = g3d->GetCaps ();
  csPixelFormat *pfmt = g2d->GetPixelFormat ();

#if defined CS_DEBUG
  char exe_mode [] = "Debug";
#else
  char exe_mode [] = "Optimised";
#endif

#if defined CS_BIG_ENDIAN
  char endianness [] = "big";
#else
  char endianness [] = "little";
#endif
  char buf [] = 
"===========================================================================\n"
"Crystal Space Version %s (%s)\n"
"===========================================================================\n"
"csGfx Info\n"
"                  Video Card : ?\n"
"                Video Memory : ?\n"
"                Video Driver : ?\n"
"                      Screen : %dx%d\n"
"                     CanClip : %s\n"
"                      MinTex : %dx%d\n"
"                      MaxTex : %dx%d\n"
"             MaxAspectRation : %d\n"
"             Double Buffered : %s\n"
"                Pixel Format : R%dG%dB%d\n"
"                 Full Screen : %s\n\n"
"csSound Info\n"
"                  Sound Card : ?\n"
"                Sound Memory : ?\n"
"                Sound Driver : ?\n\n"
"csSys Info\n"
"                  Endianness : %s\n"
"               System Memory : ?\n"
"===========================================================================\n"
" %s Executable\n"
"---------------------------------------------------------------------------\n"
"Demo Section : %s\n"
"---------------------------------------------------------------------------\n"
"Summary:\n"
"--------\n"
"%sResolution   : %d frames per entry";

    // 16 unknown entries...
    int len_guess = strlen (buf) + 18*15;
    entry->buf = new char [len_guess];

    sprintf (entry->buf, buf, 
	     CS_VERSION, CS_RELEASE_DATE, 
	     g3d->GetWidth (), g3d->GetHeight (),
	     caps->CanClip ? "yes" : "no",
	     caps->minTexWidth, caps->minTexHeight,
	     caps->maxTexWidth, caps->maxTexHeight,
	     caps->MaxAspectRatio,
	     g2d->GetDoubleBufferState () ? "yes" : "no",
	     pfmt->RedBits, pfmt->GreenBits, pfmt->BlueBits,
	     g2d->GetFullScreen () ? "yes" : "no",
	     endianness,
	     exe_mode,
	     name,
	     margin, resolution);

    entry->len = strlen (entry->buf)+1;
    entry->frame_num = statlog_section->frame_num;
    CS_ASSERT (entry->len <= len_guess);

    statvec->Push (entry);
    g3d->DecRef ();
}

void csPerfStats::WriteSubSummary ()
{
  if (name)
  {
    StatEntry *entry = new StatEntry ();
    char buf [] = "\n%sSummary Subsection '%s'\n%s------------------";
    int len_guess = strlen(buf) + strlen (name) + indent*2;
    entry->buf = new char [len_guess];
    sprintf (entry->buf, buf, 
	     margin, name,
	     margin);
    entry->len = strlen (entry->buf)+1;
    entry->frame_num = statlog_section->frame_num;
    CS_ASSERT (entry->len <= len_guess);
    statlog_section->statvec->Push (entry);
  }
}

void csPerfStats::WriteSubBegin ()
{
  if (name)
  {
    StatEntry *entry = new StatEntry ();
    char buf [] = "\n\nBegin Subsection '%s'\n----------------";
    int len_guess = strlen(buf) + strlen (name);
    entry->buf = new char [len_guess];
    sprintf (entry->buf, buf, name);
    entry->len = strlen (entry->buf)+1;
    entry->frame_num = statlog_section->frame_num;
    CS_ASSERT (entry->len <= len_guess);
    statlog_section->statvec->Push (entry);
  }
}

void csPerfStats::WriteFrameHeader ()
{
  StatEntry *entry = new StatEntry ();
  char buf [] = 
"\n"
"---------------------------------------------------------------------------\n"
"Frame    FPS\n"
"-----    ---";
  entry->len = strlen(buf)+1;
  entry->buf = new char [entry->len];
  strcpy (entry->buf, buf);
  entry->frame_num = statlog_section->frame_num;
  statlog_section->statvec->Push (entry);
}

bool csPerfStats::WriteFile ()
{
  if (!statvec)
    return false;

  long total_len = 0;
  int statvec_num = statvec->Length ();
  int i;

  if (!statvec_num)
    return false;

  for (i = 0; i < statvec_num; i++)
    total_len += ((StatEntry*)statvec->Get (i))->len;
  // subtract end of file characters
  total_len -= statvec_num;

  int framevec_num=0;
  char *f_buf = NULL;
  int f_buf_len;

  // Here the reason for writing to a buffer first is to format the frame
  // number and fps's origins to the same between entries.  
  // Suggestions for a better way?
  if (head_section->frame_by_frame)
  {
    // 25 is the max length of a per frame entry
    framevec_num = head_section->framevec->Length ();

    if (!framevec_num)
      return false;

    f_buf_len = 25*framevec_num;
    total_len += f_buf_len;

    f_buf = new char[f_buf_len];
    CS_ASSERT (f_buf);

    // probably a better way to fill a string with spaces
    for (i = 0; i < f_buf_len; i++)
      f_buf [i] = ' ';

    char *buf = f_buf;
    char tbuf[15];
    for (i = 0; i < framevec_num; i++)
    {
      FrameEntry* e = (FrameEntry*)head_section->framevec->Get (i);
      sprintf (tbuf, "\n%d", resolution*(i+1));
      memcpy (buf, tbuf, strlen (tbuf) * sizeof(char));
      sprintf (tbuf, "%f", e->fps);
      buf += 10;
      memcpy (buf, tbuf, strlen (tbuf) * sizeof(char));
      buf += 15;
    }
  }

  char *buffer = new char [total_len];
  CS_ASSERT (buffer);

  char *buf = buffer;

  // Print out the main header and main summary stats first.
  StatEntry* e = (StatEntry*)statvec->Get (statvec_num-1);
  strncpy (buf, e->buf, e->len - 1);
  buf += e->len - 1;
  e = (StatEntry*)statvec->Get (statvec_num-2);
  strncpy (buf, e->buf, e->len - 1);
  buf += e->len - 1;

  if (head_section->frame_by_frame)
  {
    char *tbuf = f_buf;
    int j = 0, frame_count = resolution;
    StatEntry* se = NULL;
    if (j < (statvec_num - 2))
    {      
      se = (StatEntry*)statvec->Get (j);
      j++;
    }
    for (i = 0; i < framevec_num; i++)
    {
      while (se && se->frame_num < frame_count)
      {
	strncpy (buf, se->buf, se->len - 1);
	buf += se->len - 1;
	se = NULL;
	if (j < (statvec_num - 2))
	{
	  se = (StatEntry*)statvec->Get (j);
	  j++;
	}
      }
      memcpy (buf, tbuf, 25 * sizeof(char));
      buf += 25;
      tbuf += 25;
      frame_count += resolution;
    }
  }
  else
  {
    for (i = 0; i < statvec_num - 2; i++)
    {
      StatEntry* e = (StatEntry*)statvec->Get (i);
      strncpy (buf, e->buf, e->len - 1);
      buf += e->len - 1;
    }
  }

  delete statvec;
  delete head_section->framevec;
  head_section->frame_by_frame = false;
  statvec = NULL;
  head_section->framevec = NULL;

  iVFS *vfs = CS_QUERY_PLUGIN_ID (plugin_mgr, CS_FUNCID_VFS, iVFS);
  if (!vfs) 
    return false;

  // Is there a limit to the size of buffer which can be written at once?
  iFile* cf;
  cf = vfs->Open (file_name, VFS_FILE_WRITE);
  cf->Write (buffer, total_len);
  cf->DecRef ();

  delete [] buffer;
  delete [] f_buf;
  vfs->DecRef ();

  return true;
}
