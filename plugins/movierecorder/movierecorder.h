/*
    movierecorder.h - Crystal Space plugin implementation header
                      for a realtime movie recorder.

    Copyright (C) 2003 by Micah Dowty <micah@navi.picogui.org>

    NOTE that this plugin is GPL, not LGPL.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.
   
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef __CS_MOVIERECORDER_H__
#define __CS_MOVIERECORDER_H__

#include "iutil/comp.h"
#include "ivaria/movierecorder.h"
#include "iutil/eventh.h"
#include "iutil/virtclk.h"
#include "csutil/cfgacc.h"
#include "csutil/util.h"
#include "nuppelwriter.h"

struct iObjectRegistry;
struct iEngine;
struct iGraphics2D;
struct iVFS;
struct iFile;

/**
 * Movie recorder plugin. After loading this plugin, it can be activated
 * either programmatically or from the keyboard. The files produced are
 * in NuppelVideo format using the RTJpeg lossy codec and/or the LZO
 * lossless codec. Note that this module is GPLed rather than LGPLed due to
 * licenses on the existing compression code.
 */
class csMovieRecorder : public iMovieRecorder, public iComponent
{
private:
  iObjectRegistry *object_reg;
  csRef<iEngine> Engine;
  csRef<iGraphics2D> G2D;
  csRef<iVFS> VFS;
  csRef<iVirtualClock> vc;
  csConfigAccess config;
  bool initialized;
  NuppelWriter *writer;
  csRef<iFile> movieFile;
  csRef<iVirtualClock> realVirtualClock;
  /// Number of current ticks since start fake clock started.
  /// Stored as float here to properly handle all frame rates w/o
  /// round off errors.
  float ffakeClockTicks;
  /// How many ticks per one fake frame
  float fakeTicksPerFrame;
  /// Number of current ticks since start fake clock started,
  /// now as csTicks
  csTicks fakeClockTicks;
  /// How many ticks in our fake clocks have elapsed since last advance.
  csTicks fakeClockElapsed;
  bool paused;
  // some statistic
  int numFrames;
  csTicks totalFrameEncodeTime, minFrameEncodeTime, maxFrameEncodeTime;
  csTicks totalWriteToDiskTime, minWriteToDiskTime, maxWriteToDiskTime;
  csTicks frameStartTime, totalFrameTime, minFrameTime, maxFrameTime;

  /// format of the movie filename (e.g. "/this/cryst%03d.nuv")
  char* captureFormat;
  int captureFormatNumberMax;
  char movieFileName[CS_MAXPATHLEN];

  /// Capture settings
  float frameRate, rtjQuality;
  int recordWidth, recordHeight;
  bool useLZO, useRTJpeg, useRGB;
  /// Throttle clock if frame is drawn faster than required
  bool throttle;

  /// Key bindings
  struct keyBinding {
    utf32_char code;
    bool shift, alt, ctrl;
  } keyRecord, keyPause;

  /// Eat a key and process it.
  bool EatKey (iEvent& event);
  /// Start of frame.
  bool HandleStartFrame (iEvent& event);
  /// End of frame.
  bool HandleEndFrame (iEvent& event);

  /// Perform initialization that depends on other plugins
  void SetupPlugin ();

  /// From Bugplug, for decoding keys in the config file
  void GetKeyCode (const char* keystring, struct keyBinding &key);

  /// Wrapper around the reporter plugin
  void Report (int severity, const char* msg, ...);

  /// Callback function used by the NuppelWriter
  static void WriterCallback (const void *data, long bytes, void *extra);

public:
  csMovieRecorder (iBase* parent);
  virtual ~csMovieRecorder ();
  virtual bool Initialize (iObjectRegistry *iobject_reg);  

  /// Called via the embedded iEventHandler
  bool HandleEvent (iEvent &event);

  /// Called by the embedded iVirtualClock
  void ClockAdvance ();
  void ClockSuspend ();
  void ClockResume ();
  csTicks ClockGetElapsedTicks () const;
  csTicks ClockGetCurrentTicks () const;

  /// Start/stop the recorder
  virtual void Start(void);
  virtual void Stop(void);
  virtual bool IsRecording(void) const;
  virtual void Pause(void);
  virtual void UnPause(void);
  virtual bool IsPaused(void) const;

  SCF_DECLARE_IBASE;

  /**
   * Embedded iEventHandler interface that forwards its events to
   * csMovieRecorder's event handler
   */
  class EventHandler : public iEventHandler
  {
  private:
    csMovieRecorder* parent;
  public:
    SCF_DECLARE_IBASE;
    EventHandler (csMovieRecorder* parent)
    {
      SCF_CONSTRUCT_IBASE (0);
      EventHandler::parent = parent;
    }
    virtual ~EventHandler ()
    {
      SCF_DESTRUCT_IBASE();
    }
    virtual bool HandleEvent (iEvent& ev)
    {
      return parent->HandleEvent (ev);
    }
  } *scfiEventHandler;

  /**
   * Embedded iVirtualClock. We replace the system's default virtual clock
   * with this in order to tie the application's timing to the movie rather
   * than real time when we're recording.
   */
  class VirtualClock : public iVirtualClock
  {
  private:
    csMovieRecorder* parent;
  public:
    SCF_DECLARE_IBASE;
    VirtualClock (csMovieRecorder* parent)
    {
      SCF_CONSTRUCT_IBASE (0);
      VirtualClock::parent = parent;
    }
    virtual ~VirtualClock()
    {
      SCF_DESTRUCT_IBASE();
    }
    virtual void Advance ()
    {
      parent->ClockAdvance();
    }
    virtual void Suspend ()
    {
      parent->ClockSuspend();
    }
    virtual void Resume ()
    {
      parent->ClockResume();
    }
    virtual csTicks GetElapsedTicks () const
    {
      return parent->ClockGetElapsedTicks();
    }
    virtual csTicks GetCurrentTicks () const
    {
      return parent->ClockGetCurrentTicks();
    }
  } *scfiVirtualClock;
};

#endif // __CS_MOVIERECORDER_H__
