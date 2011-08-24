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
#include "csutil/eventnames.h"
#include "csutil/util.h"
#include "csutil/weakref.h"
#include "cstool/numberedfilenamehelper.h"
#include "nuppelwriter.h"

struct iObjectRegistry;
struct iEngine;
struct iGraphics2D;
struct iVFS;
struct iFile;

CS_PLUGIN_NAMESPACE_BEGIN(Movierecorder)
{

/**
 * Movie recorder plugin. After loading this plugin, it can be activated
 * either programmatically or from the keyboard. The files produced are
 * in NuppelVideo format using the RTJpeg lossy codec and/or the LZO
 * lossless codec. Note that this module is GPLed rather than LGPLed due to
 * licenses on the existing compression code.
 */
class csMovieRecorder : 
  public scfImplementation2<csMovieRecorder, 
                            iMovieRecorder, 
			    iComponent>
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
  /// now as csMicroTicks
  csMicroTicks fakeClockTicks;
  /// How many ticks in our fake clocks have elapsed since last advance.
  csMicroTicks fakeClockElapsed;
  bool paused;
  // some statistic
  int numFrames;
  csMicroTicks totalFrameEncodeTime, minFrameEncodeTime, maxFrameEncodeTime;
  csMicroTicks totalWriteToDiskTime, minWriteToDiskTime, maxWriteToDiskTime;
  csMicroTicks frameStartTime, totalFrameTime, minFrameTime, maxFrameTime;

  /// format of the movie filename (e.g. "/this/cryst%03d.nuv")
  CS::NumberedFilenameHelper captureFormat;
  csString filenameFormat;
  csString movieFileName;
  csString recordingFile;

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

  /// Called by the embedded iVirtualClock
  void ClockAdvance ();
  void ClockSuspend ();
  void ClockResume ();
  csTicks ClockGetElapsedTicks () const;
  csTicks ClockGetCurrentTicks () const;
  csMicroTicks ClockGetElapsedMicroTicks () const;
  csMicroTicks ClockGetCurrentMicroTicks () const;
  float ClockGetElapsedSeconds ();

  /// Start/stop the recorder
  virtual void Start(void);
  virtual void Stop(void);
  virtual bool IsRecording(void) const;
  virtual void Pause(void);
  virtual void UnPause(void);
  virtual bool IsPaused(void) const;
  virtual void SetRecordingFile (const char* filename);
  virtual void SetFilenameFormat (const char* format);

  /**
   * Embedded iEventHandler interface that handles keyboard events
   */
  class KeyEventHandler : 
    public scfImplementation1<KeyEventHandler, 
                              iEventHandler>
  {
  private:
    csWeakRef<csMovieRecorder> parent;
  public:
    KeyEventHandler (csMovieRecorder* parent) : scfImplementationType (this),
      parent (parent) { }
    virtual ~KeyEventHandler () { }
    virtual bool HandleEvent (iEvent& ev)
    {
      if (parent && (CS_IS_KEYBOARD_EVENT(parent->object_reg, ev)))
      {      
        return parent->EatKey (ev);
      }

      return false;
    }
    CS_EVENTHANDLER_NAMES("crystalspace.movierecorder.keyboard")
    CS_EVENTHANDLER_NIL_CONSTRAINTS
  };
  csRef<KeyEventHandler> keyEventHandler;

  /**
  * Embedded iEventHandler interface that handles frame events in the
  * logic phase.
  */
  class LogicEventHandler : 
    public scfImplementation1<LogicEventHandler, 
    iEventHandler>
  {
  private:
    csWeakRef<csMovieRecorder> parent;
  public:
    LogicEventHandler (csMovieRecorder* parent) :
        scfImplementationType (this), parent (parent) { }
    virtual ~LogicEventHandler () { }
    virtual bool HandleEvent (iEvent& ev)
    {
      if (parent && (ev.Name == parent->Frame))
      {      
        return parent->HandleStartFrame (ev);
      }

      return false;
    }
    CS_EVENTHANDLER_PHASE_LOGIC("crystalspace.movierecorder.frame.logic")
  };
  csRef<LogicEventHandler> logicEventHandler;

  /**
  * Embedded iEventHandler interface that handles frame events in the
  * frame phase.
  */
  class FrameEventHandler : 
    public scfImplementation1<FrameEventHandler, 
    iEventHandler>
  {
  private:
    csWeakRef<csMovieRecorder> parent;
  public:
    FrameEventHandler (csMovieRecorder* parent) :
        scfImplementationType (this), parent (parent) { }
    virtual ~FrameEventHandler () { }
    virtual bool HandleEvent (iEvent& ev)
    {
      if (parent && (ev.Name == parent->Frame))
      {      
        return parent->HandleEndFrame (ev);
      }

      return false;
    }
    CS_EVENTHANDLER_PHASE_FRAME("crystalspace.movierecorder.frame.frame")
  };
  csRef<FrameEventHandler> frameEventHandler;

  /**
   * Embedded iVirtualClock. We replace the system's default virtual clock
   * with this in order to tie the application's timing to the movie rather
   * than real time when we're recording.
   */
  class VirtualClock : 
    public scfImplementation1<VirtualClock,
			      iVirtualClock>
  {
  private:
    csWeakRef<csMovieRecorder> parent;
  public:
    VirtualClock (csMovieRecorder* parent) : scfImplementationType (this),
      parent (parent) { }
    virtual ~VirtualClock() { }
    virtual void Advance ()
    {
      if (parent) parent->ClockAdvance();
    }
    virtual void Suspend ()
    {
      if (parent) parent->ClockSuspend();
    }
    virtual void Resume ()
    {
      if (parent) parent->ClockResume();
    }
    virtual csTicks GetElapsedTicks () const
    {
      return parent ? parent->ClockGetElapsedTicks() : 0;
    }
    virtual csTicks GetCurrentTicks () const
    {
      return parent ? parent->ClockGetCurrentTicks() : 0;
    }
    virtual csMicroTicks GetElapsedMicroTicks () const
    {
      return parent ? parent->ClockGetElapsedMicroTicks() : 0;
    }
    virtual float GetElapsedSeconds ()
    {
      return parent ? parent->ClockGetElapsedSeconds() : 0.0f;
    }
    virtual csMicroTicks GetCurrentMicroTicks () const
    {
      return parent ? parent->ClockGetCurrentMicroTicks() : 0;
    }
  };
  csRef<VirtualClock> virtualClock;

  CS_DECLARE_EVENT_SHORTCUTS;
};

}
CS_PLUGIN_NAMESPACE_END(Movierecorder)

#endif // __CS_MOVIERECORDER_H__
