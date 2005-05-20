/*
    movierecorder.h - Crystal Space plugin implementation for a
                      realtime movie recorder.

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

#include "cssysdef.h"
#include "movierecorder.h"
#include "iutil/vfs.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "ivaria/reporter.h"
#include "ivideo/graph2d.h"
#include "iutil/virtclk.h"
#include "ivaria/reporter.h"
#include "iengine/engine.h"
#include "igraphic/image.h"
#include "csutil/event.h"
#include "csgfx/imagemanipulate.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csMovieRecorder)
  SCF_IMPLEMENTS_INTERFACE (iMovieRecorder)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csMovieRecorder::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csMovieRecorder::VirtualClock)
  SCF_IMPLEMENTS_INTERFACE (iVirtualClock)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csMovieRecorder)



void csMovieRecorder::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (object_reg, iReporter));
  if (rep)
    rep->ReportV (severity, "crystalspace.movierecorder", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

csMovieRecorder::csMovieRecorder (iBase* parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  scfiEventHandler = 0;
  scfiVirtualClock = 0;
  object_reg = 0;
  initialized = false;
  writer = 0;
  ffakeClockTicks = 0;
  fakeClockTicks = 0;
  fakeClockElapsed = 0;
  paused = false;
}

csMovieRecorder::~csMovieRecorder ()
{
  Stop();
    
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
    if (q)
      q->RemoveListener (scfiEventHandler);
    scfiEventHandler->DecRef ();
  }

  if (scfiVirtualClock)
  {
    // Put back the real virtual clock :)
    object_reg->Unregister(scfiVirtualClock, "iVirtualClock");
    object_reg->Register(realVirtualClock, "iVirtualClock");
    scfiVirtualClock->DecRef ();
  }

  SCF_DESTRUCT_IBASE();
}

bool csMovieRecorder::Initialize (iObjectRegistry* iobject_reg)
{
  object_reg = iobject_reg;

  // We need to receive keyboard events for our hotkeys, and the nothing
  // event for the actual movie recording.
  if (!scfiEventHandler)
  {
    scfiEventHandler = new EventHandler (this);
  }
  csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
  if (q != 0)
    q->RegisterListener (scfiEventHandler,
			 CSMASK_Nothing | CSMASK_Keyboard);

  // Unregister the normal virtual clock and register our own
  // replacement, hoping nobody will notice :)
  //
  // FIXME: This will work for the application when this plugin is
  //        loaded in initialization, but some modules might see the
  //        real VC. Additionally, this won't work at all if the app
  //        loads us after the app gets a VC reference.
  if (!scfiVirtualClock)
  {
    scfiVirtualClock = new VirtualClock (this);
  }
  realVirtualClock = CS_QUERY_REGISTRY (object_reg, iVirtualClock);
  object_reg->Unregister(realVirtualClock, "iVirtualClock");
  object_reg->Register(scfiVirtualClock, "iVirtualClock");

  return true;
}

void csMovieRecorder::SetupPlugin()
{
  if (initialized) return;

  if (!Engine) Engine = CS_QUERY_REGISTRY (object_reg, iEngine);

  if (!G2D) G2D = CS_QUERY_REGISTRY (object_reg, iGraphics2D);
  if (!G2D)
  {
    csPrintf ("No G2D!\n");
    return;
  }

  if (!VFS) VFS = CS_QUERY_REGISTRY (object_reg, iVFS);
  if (!VFS)
  {
    csPrintf ("No VFS!\n");
    return;
  }

  config.AddConfig (object_reg, "/config/movierecorder.cfg");

  frameRate = config->GetFloat("MovieRecorder.Capture.FPS", 30.0);
  rtjQuality = config->GetFloat("MovieRecorder.Capture.RTJpegQuality", 1.0);
  recordWidth = config->GetInt("MovieRecorder.Capture.Width", 0);
  recordHeight = config->GetInt("MovieRecorder.Capture.Height", 0);
  useLZO = config->GetBool("MovieRecorder.Capture.UseLZO", true);
  useRTJpeg = config->GetBool("MovieRecorder.Capture.UseRTJpeg", false);
  useRGB = config->GetBool("MovieRecorder.Capture.UseRGB", false);
  throttle = config->GetBool("MovieRecorder.Capture.Throttle", true);

  GetKeyCode(config->GetStr("MovieRecorder.Keys.Record", "alt-r"), keyRecord);
  GetKeyCode(config->GetStr("MovieRecorder.Keys.Pause", "alt-p"), keyPause);

  // Filename formatting code ripped from Bugplug's screenshotter
  captureFormat = csStrNew (config->GetStr ("MovieRecorder.Capture.FilenameFormat",
    "/tmp/crystal000.nuv"));
  // since this string is passed to format later,
  // replace all '%' with '%%'
  {
    char* newstr = new char[strlen(captureFormat)*2 + 1];
    memset (newstr, 0, strlen(captureFormat)*2 + 1);

    char* pos = captureFormat;
    while (pos)
    {
      char* percent = strchr (pos, '%');
      if (percent)
      {
	strncat (newstr, pos, percent-pos);
	strcat (newstr, "%%");
	pos = percent + 1;
      }
      else
      {
	strcat (newstr, pos);
	pos = 0;
      }
    }
    delete[] captureFormat;
    captureFormat = newstr;
  }
  // scan for the rightmost string of digits
  // and create an appropriate format string
  {
    captureFormatNumberMax = 1;
    int captureFormatNumberDigits = 0;

    char* end = strrchr (captureFormat, 0);
    if (end != captureFormat)
    {
      do
      {
	end--;
      }
      while ((end >= captureFormat) && (!isdigit (*end)));
      if (end >= captureFormat)
      {
	do
	{
	  captureFormatNumberMax *= 10;
	  captureFormatNumberDigits++; 
	  end--;
	}
	while ((end >= captureFormat) && (isdigit (*end)));
	
	csString nameForm;
	nameForm.Format ("%%0%dd", captureFormatNumberDigits);

        csString newCapForm;
        newCapForm.Append (captureFormat, end-captureFormat+1);
        newCapForm.Append (nameForm);
        newCapForm.Append (end+captureFormatNumberDigits+1);

	delete[] captureFormat;
	captureFormat = csStrNew (newCapForm);
      }
    }
  }

  initialized = true;
}

bool csMovieRecorder::HandleEvent (iEvent &event)
{
  if (event.Type == csevKeyboard)
  {
    return EatKey (event);
  }
  else if (event.Type == csevBroadcast)
  {
    if (csCommandEventHelper::GetCode(&event) == cscmdPreProcess)
    {
      return HandleStartFrame (event);
    }
    if (csCommandEventHelper::GetCode(&event) == cscmdPostProcess)
    {
      return HandleEndFrame (event);
    }
  }

  return false;
}

bool csMovieRecorder::EatKey (iEvent& event)
{
  SetupPlugin();
  bool down = csKeyEventHelper::GetEventType (&event);
  csKeyModifiers m;
  csKeyEventHelper::GetModifiers (&event, m);
  bool alt = m.modifiers[csKeyModifierTypeAlt] != 0;
  bool ctrl = m.modifiers[csKeyModifierTypeCtrl] != 0;
  bool shift = m.modifiers[csKeyModifierTypeShift] != 0;
  utf32_char key = csKeyEventHelper::GetCookedCode (&event);

  if (down && (key == keyRecord.code) && (alt == keyRecord.alt) && 
      (ctrl == keyRecord.ctrl) && (shift == keyRecord.shift)) 
  {
    if (IsRecording())
	Stop();
    else
	Start();
    return true;
  }

  if (down && key==keyPause.code && alt==keyPause.alt && 
      ctrl==keyPause.ctrl && shift==keyPause.shift) 
  {
    if (IsPaused())
	UnPause();
    else
	Pause();
    return true;
  }

  return false;
}

bool csMovieRecorder::HandleStartFrame (iEvent& /*event*/)
{
  SetupPlugin();
  // don't use VC here - we need 'real' ticks.
  frameStartTime = csGetTicks();
  return false;
}

bool csMovieRecorder::HandleEndFrame (iEvent& /*event*/)
{
  if (IsRecording() && !IsPaused()) {
    csRef<iImage> img (csPtr<iImage> (G2D->ScreenShot ()));

    csTicks ticks = csGetTicks();
    csTicks thisFrameTime = ticks - frameStartTime;

    if (!img) {
      Report (CS_REPORTER_SEVERITY_ERROR, "This video driver doesn't support screen capture.");
      Stop();
      return false;
    }

    // If we're recording to a different resolution, try to scale the image
    if (img->GetWidth() != writer->width || img->GetHeight() != writer->height) {
      img = csImageManipulate::Rescale (img, writer->width, writer->height);
    }

    numFrames++;

    csTicks encodeTime, writeTime;
    unsigned char *buffer = (unsigned char *) img->GetImageData();
    writer->writeFrame(buffer, encodeTime, writeTime);

    totalFrameTime += thisFrameTime;
    minFrameTime = MIN (minFrameTime, thisFrameTime);
    maxFrameTime = MAX (maxFrameTime, thisFrameTime);

    totalFrameEncodeTime += encodeTime;
    minFrameEncodeTime = MIN (minFrameEncodeTime, encodeTime);
    maxFrameEncodeTime = MAX (maxFrameEncodeTime, encodeTime);

    totalWriteToDiskTime += writeTime;
    minWriteToDiskTime = MIN (minWriteToDiskTime, writeTime);
    maxWriteToDiskTime = MAX (maxWriteToDiskTime, writeTime);
  }

  return false;
}

bool csMovieRecorder::IsRecording(void) const
{
  return writer != 0;
}

void csMovieRecorder::Start(void) 
{
  if (IsPaused()) {
    UnPause();
    return;
  }
  if (IsRecording())
    Stop();

  // Filename forming code, ripped from Bugplug's screenshotter
  int i = 0;
  bool exists = false;
  do
  {
    cs_snprintf (movieFileName, CS_MAXPATHLEN, captureFormat, i);
    if (exists = VFS->Exists (movieFileName)) i++;
  }
  while ((i < captureFormatNumberMax) && (exists));

  if (i >= captureFormatNumberMax)
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY,
    	"Too many video recording files in current directory");
    return;
  }

  // If the config specified 0x0, that means we use the current resolution unscaled
  int w = recordWidth  ? recordWidth  : G2D->GetWidth();
  int h = recordHeight ? recordHeight : G2D->GetHeight();

  numFrames = 0;
  totalFrameEncodeTime = totalFrameTime = totalWriteToDiskTime = 0;
  minFrameEncodeTime = minFrameTime = minWriteToDiskTime = (csTicks)-1;
  maxFrameEncodeTime = maxFrameTime = maxWriteToDiskTime = 0;

  movieFile = VFS->Open (movieFileName, VFS_FILE_WRITE | VFS_FILE_UNCOMPRESSED);
  if (!movieFile)
  {
    Report (CS_REPORTER_SEVERITY_WARNING,
    	"Couldn't open file '%s' for recording", movieFileName);
    return;
  }
  fakeTicksPerFrame = (1000 / frameRate);
  ffakeClockTicks = fakeClockTicks;

  frameStartTime = csGetTicks();

  writer = new NuppelWriter(w, h, &WriterCallback, this, frameRate,
			    rtjQuality, useRTJpeg, useLZO, useRGB);

  Report (CS_REPORTER_SEVERITY_NOTIFY, "Video recorder started - %s", movieFileName);
}

void csMovieRecorder::Stop(void)
{
  if (IsRecording()) {
    delete writer;
    writer = 0;
    movieFile = 0;
    Report (CS_REPORTER_SEVERITY_NOTIFY, "Video recorder stopped - %s", movieFileName);

    if (numFrames != 0)
    {
      float avgFrameEncodeTime = ((float)totalFrameEncodeTime / (float)numFrames);
      float avgWriteToDiskTime = ((float)totalWriteToDiskTime / (float)numFrames);
      float avgFrameTime = ((float)totalFrameTime / (float)numFrames);
      
      Report (CS_REPORTER_SEVERITY_NOTIFY, 
	"Video recording statistics for %s:\n"
	" Number of frames: %d\n"
	" Time spent for:\n"
	"  encoding image data - total: %.3fs, per frame: %zu min/%g avg/%zu max ms\n"
	"  writing encoded data - total: %.3fs, per frame: %zu min/%g avg/%zu max ms\n"
	"  drawing frame - total: %.3fs, per frame: %zu min/%g avg/%zu max ms\n"
	"\n"
	" Frame time in relation to real time: x%.4f\n"
	" Theoretical video FPS recordable in real-time: %.2f\n",
	movieFileName, 
	numFrames,
	((float)totalFrameEncodeTime / 1000.0f),
	  minFrameEncodeTime, avgFrameEncodeTime, maxFrameEncodeTime,
	((float)totalWriteToDiskTime / 1000.0f),
	  minWriteToDiskTime, avgWriteToDiskTime, maxWriteToDiskTime,
	((float)totalFrameTime / 1000.0f),
	  minFrameTime, avgFrameTime, maxFrameTime,
	((avgFrameEncodeTime + avgWriteToDiskTime + avgFrameTime) * 
	  frameRate) / 1000.0f,
	1000.0f / (avgFrameEncodeTime + avgWriteToDiskTime + avgFrameTime)
      );
    }
  }
}

bool csMovieRecorder::IsPaused(void) const
{
  return paused;
}

void csMovieRecorder::Pause(void) 
{
  if (!IsRecording())
    return;
  paused = true;
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Video recorder paused - %s", movieFileName);
}

void csMovieRecorder::UnPause(void)
{
  if (!IsRecording())
    return;
  paused = false;
  ffakeClockTicks = fakeClockTicks;
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Video recorder unpaused - %s", movieFileName);
}

void csMovieRecorder::WriterCallback(const void *data, long bytes, void *extra)
{
  csMovieRecorder *rec = (csMovieRecorder *) extra;
  rec->movieFile->Write((const char*) data, bytes);
}

void csMovieRecorder::ClockAdvance ()
{
  csTicks lastFakeClockTicks = fakeClockTicks;
  realVirtualClock->Advance();
  csTicks realTicksPerFrame = realVirtualClock->GetElapsedTicks();

  /*
    To avoid 'jumps' in time when the clock is throttled/unthrottled
    we keep our own tick counter, which is either increased by the
    real elapsed time (normal mode) or the required frame time (recording).
   */
  if (!IsRecording() || IsPaused()) {
    fakeClockElapsed = realTicksPerFrame;
    fakeClockTicks += realTicksPerFrame;
  }
  else {
    ffakeClockTicks += fakeTicksPerFrame;
    fakeClockTicks = (csTicks)ffakeClockTicks;

    fakeClockElapsed = fakeClockTicks - 
      lastFakeClockTicks;
    // If we're rendering slower than real time, there's nothing we can do about it.
    // If we're rendering faster, put in a little delay here.
    if (throttle && ((fakeClockElapsed > realTicksPerFrame)))
    {
      csSleep(fakeClockElapsed - realTicksPerFrame);
    }
  } 
}

void csMovieRecorder::ClockSuspend ()
{
  if (!IsRecording() || IsPaused())
    realVirtualClock->Suspend();
}

void csMovieRecorder::ClockResume ()
{
  if (!IsRecording() || IsPaused())
    realVirtualClock->Resume();
}

csTicks csMovieRecorder::ClockGetElapsedTicks () const
{
  return fakeClockElapsed;
}

csTicks csMovieRecorder::ClockGetCurrentTicks () const
{
  return fakeClockTicks;
}

/// From Bugplug, for decoding keys in the config file
void csMovieRecorder::GetKeyCode (const char* keystring, struct keyBinding &key)
{
  key.shift = key.alt = key.ctrl = false;
  char const* dash = strchr (keystring, '-');
  while (dash)
  {
    if (!strncmp (keystring, "shift", int (dash-keystring))) key.shift = true;
    else if (!strncmp (keystring, "alt", int (dash-keystring))) key.alt = true;
    else if (!strncmp (keystring, "ctrl", int (dash-keystring))) key.ctrl = true;
    keystring = dash+1;
    dash = strchr (keystring, '-');
  }

  key.code = 0;
  if (!strcmp (keystring, "tab")) key.code = CSKEY_TAB;
  else if (!strcmp (keystring, "space")) key.code = ' ';
  else if (!strcmp (keystring, "esc")) key.code = CSKEY_ESC;
  else if (!strcmp (keystring, "enter")) key.code = CSKEY_ENTER;
  else if (!strcmp (keystring, "bs")) key.code = CSKEY_BACKSPACE;
  else if (!strcmp (keystring, "up")) key.code = CSKEY_UP;
  else if (!strcmp (keystring, "down")) key.code = CSKEY_DOWN;
  else if (!strcmp (keystring, "right")) key.code = CSKEY_RIGHT;
  else if (!strcmp (keystring, "left")) key.code = CSKEY_LEFT;
  else if (!strcmp (keystring, "pgup")) key.code = CSKEY_PGUP;
  else if (!strcmp (keystring, "pgdn")) key.code = CSKEY_PGDN;
  else if (!strcmp (keystring, "home")) key.code = CSKEY_HOME;
  else if (!strcmp (keystring, "end")) key.code = CSKEY_END;
  else if (!strcmp (keystring, "ins")) key.code = CSKEY_INS;
  else if (!strcmp (keystring, "del")) key.code = CSKEY_DEL;
  else if (!strcmp (keystring, "f1")) key.code = CSKEY_F1;
  else if (!strcmp (keystring, "f2")) key.code = CSKEY_F2;
  else if (!strcmp (keystring, "f3")) key.code = CSKEY_F3;
  else if (!strcmp (keystring, "f4")) key.code = CSKEY_F4;
  else if (!strcmp (keystring, "f5")) key.code = CSKEY_F5;
  else if (!strcmp (keystring, "f6")) key.code = CSKEY_F6;
  else if (!strcmp (keystring, "f7")) key.code = CSKEY_F7;
  else if (!strcmp (keystring, "f8")) key.code = CSKEY_F8;
  else if (!strcmp (keystring, "f9")) key.code = CSKEY_F9;
  else if (!strcmp (keystring, "f10")) key.code = CSKEY_F10;
  else if (!strcmp (keystring, "f11")) key.code = CSKEY_F11;
  else if (!strcmp (keystring, "f12")) key.code = CSKEY_F12;
  else if (*(keystring+1) != 0) key.code = 0;
  else if ((*keystring >= 'A' && *keystring <= 'Z')
  	|| strchr ("!@#$%^&*()_+", *keystring))
  {
    key.shift = 1;
    key.code = *keystring;
  }
  else
    key.code = *keystring;
}

//---------------------------------------------------------------------------

