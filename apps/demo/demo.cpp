/*
    Copyright (C) 2001 by Jorrit Tyberghein

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


    csdemo - This is the demo application for crystal space, it reads worlds
    and scenes with sequences and animation paths from data files allowing
    to produce generic demos.
*/

#include "cssysdef.h"
#include "csutil/sysfunc.h"
#include "demo.h"
#include "demoseq.h"
#include "csutil/cscolor.h"
#include "csutil/cmdhelp.h"
#include "csgeom/path.h"
#include "cstool/csfxscr.h"
#include "cstool/csview.h"
#include "cstool/initapp.h"
#include "ivideo/graph3d.h"
#include "ivideo/natwin.h"
#include "ivideo/fontserv.h"
#include "ivideo/graph2d.h"
#include "ivaria/conout.h"
#include "iengine/engine.h"
#include "iengine/sector.h"
#include "iengine/light.h"
#include "iengine/camera.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/halo.h"
#include "iengine/material.h"
#include "imesh/thing.h"
#include "imesh/particle.h"
#include "imesh/sprite2d.h"
#include "imesh/sprite3d.h"
#include "imesh/ball.h"
#include "imesh/stars.h"
#include "imesh/object.h"
#include "imap/reader.h"
#include "imap/parser.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/virtclk.h"
#include "iutil/csinput.h"
#include "iutil/cmdline.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "csutil/stringarray.h"
#include "igraphic/imageio.h"
#include "ivaria/reporter.h"
#include "csqsqrt.h"
#include "csutil/event.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// The global system driver
Demo *System;

void Demo::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (System->object_reg, iReporter));
  if (rep)
  {
    rep->ReportV (severity, "crystalspace.application.demo", msg, arg);
  }
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

Demo::Demo ()
{
  seqmgr = 0;
}

Demo::~Demo ()
{
  delete seqmgr;
}

void Cleanup ()
{
  csPrintf ("Cleaning up...\n");
  iObjectRegistry* object_reg = System->object_reg;
  delete System; System = 0;
  csInitializer::DestroyApplication (object_reg);
}

static bool DemoEventHandler (iEvent& ev)
{
  if (ev.Type == csevBroadcast && ev.Command.Code == cscmdProcess)
  {
    System->SetupFrame ();
    return true;
  }
  else if (ev.Type == csevBroadcast && ev.Command.Code == cscmdFinalProcess)
  {
    System->FinishFrame ();
    return true;
  }
  else
  {
    return System ? System->DemoHandleEvent (ev) : false;
  }
}

static void TestDemoFile (const char* zip, iVFS* myVFS, csStringArray& demos)
{
  csRef<iDataBuffer> realpath_db (myVFS->GetRealPath (zip));
  char* realpath = (char*)(realpath_db->GetData ());
  char* testpath = new char [strlen (realpath)+3];
  strcpy (testpath, realpath);
  if (testpath[strlen (testpath)-1] == '/')
  {
    // We have a directory.
    size_t l = strlen (testpath);
    testpath[l-1] = '$';
    testpath[l] = '/';
    testpath[l+1] = 0;
  }
  else if (strstr (testpath, ".zip") == 0)
  {
    delete[] testpath;
    return;
  }

  myVFS->Mount ("/tmp/csdemo_temp", testpath);
  if (myVFS->Exists ("/tmp/csdemo_temp/sequences"))
    demos.Push (csStrNew (testpath));
  myVFS->Unmount ("/tmp/csdemo_temp", 0);
  delete[] testpath;
}

bool Demo::LoadDemoFile (const char* demofile)
{
  do_demo = 3;
  myVFS->Mount ("/data/demo", demofile);

  if (!myVFS->ChDir ("/data/demo"))
  {
    Report (CS_REPORTER_SEVERITY_ERROR,
	  "The directory on VFS for demo file does not exist!");
   return false;
  }

  // This can fail, but we don't care :-)
  loader->LoadLibraryFile ("library");

  if (!loader->LoadMapFile ("world", false, 0, true))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "There was an error loading world!");
    exit (0);
  }

  room = engine->GetSectors ()->FindByName ("room");
  seqmgr = new DemoSequenceManager (this);
  seqmgr->Setup ("sequences");

  engine->Prepare ();

  view = csPtr<iView> (new csView (engine, myG3D));
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (
  	  csVector3 (0.0f, 0.0f, -900.0f));
  view->GetCamera ()->GetTransform ().RotateThis (
  	  csVector3 (0.0f, 1.0f, 0.0f), 0.8f);
  view->SetRectangle (0, 0, myG2D->GetWidth (), myG2D->GetHeight ());
  return true;
}

bool Demo::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return false;

  if (!csInitializer::SetupConfigManager (object_reg, iConfigName))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't initialize app!");
    return false;
  }

  if (!csInitializer::RequestPlugins (object_reg, CS_REQUEST_END))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't initialize app!");
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, DemoEventHandler))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't initialize event handler!");
    return false;
  }

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    csCommandLineHelper::Help (object_reg);
    exit (0);
  }

  // The virtual clock.
  vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (!kbd)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No keyboard driver!");
    return false;
  }

  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!engine)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No engine!");
    return false;
  }

  loader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (!loader)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No loader!");
    return false;
  }

  myG3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!myG3D)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No 3D driver!");
    return false;
  }

  myG2D = CS_QUERY_REGISTRY (object_reg, iGraphics2D);
  if (!myG2D)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No 2D driver!");
    return false;
  }

  myVFS = CS_QUERY_REGISTRY (object_reg, iVFS);
  if (!myVFS)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No VFS!");
    return false;
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  iNativeWindow* nw = myG2D->GetNativeWindow ();
  if (nw) nw->SetTitle ("The Crystal Space Demo.");
  if (!csInitializer::OpenApplication (object_reg))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error opening system!");
    Cleanup ();
    exit (1);
  }

  font = myG2D->GetFontServer ()->LoadFont (CSFONT_LARGE);

  // Some commercials...
  Report (CS_REPORTER_SEVERITY_NOTIFY,
    "The Crystal Space Demo.");

  // Create our world.
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Creating world!...");

  // Check the demo file and mount it if required.
  csRef<iCommandLineParser> cmdline (CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser));
  const char *val;
  if ((val = cmdline->GetName ()) != 0)
  {
    if (!LoadDemoFile (val))
      return false;
  }
  else
  {
    do_demo = 0;
    selected_demo = (size_t)-1;
    // Here we can't do the demo because no data file was given.
    // However we scan for possible data files and present them to the
    // user.
    csRef<iStringArray> zips = myVFS->FindFiles ("/this/*");
    size_t i;
    for (i = 0 ; i < zips->Length () ; i++)
    {
      const char* zip = zips->Get (i);
      TestDemoFile (zip, myVFS, demos);
    }
    myVFS->Mount ("/tmp/csdemo_datadir", "$@data$/");
    zips = myVFS->FindFiles ("/tmp/csdemo_datadir/*");
    for (i = 0 ; i < zips->Length () ; i++)
    {
      const char* zip = zips->Get (i);
      TestDemoFile (zip, myVFS, demos);
    }
    myVFS->Unmount ("/tmp/csdemo_datadir", 0);
  }

  col_red = myG2D->FindRGB (255, 0, 0);
  col_blue = myG2D->FindRGB (0, 0, 255);
  col_white = myG2D->FindRGB (255, 255, 255);
  col_gray = myG2D->FindRGB (50, 50, 50);
  col_black = myG2D->FindRGB (0, 0, 0);
  col_yellow = myG2D->FindRGB (255, 255, 0);
  col_cyan = myG2D->FindRGB (0, 255, 255);
  col_green = myG2D->FindRGB (0, 255, 0);

  return true;
}

#define MAP_OFF 0
#define MAP_OVERLAY 1
#define MAP_EDIT 2
#define MAP_EDIT_FORWARD 3
static int map_enabled = MAP_OFF;
static csVector2 map_tl (-1000, 1000);
static csVector2 map_br (1000, -1000);
static int map_selpoint = 0;
static char map_selpath[255] = { 0 };

void Demo::GfxWrite (int x, int y, int fg, int bg, const char *str, ...)
{
  va_list arg;
  csString buf;

  va_start (arg, str);
  buf.FormatV (str, arg);
  va_end (arg);

  myG2D->Write (font, x, y, fg, bg, buf);
}

void Demo::FileWrite (iFile* file, char *str, ...)
{
  va_list arg;
  csString buf;

  va_start (arg, str);
  buf.FormatV (str, arg);
  va_end (arg);

  file->Write (buf.GetData(), buf.Length());
}

void Demo::ShowMessage (const char* msg, ...)
{
  message_error = false;
  va_list arg;
  va_start (arg, msg);
  message.Format (msg, arg);
  va_end (arg);
  message_timer = csGetTicks () + 1500;
}

void Demo::ShowError (const char* msg, ...)
{
  message_error = true;
  va_list arg;
  va_start (arg, msg);
  message.Format (msg, arg);
  va_end (arg);
  message_timer = csGetTicks () + 1500;
}

void Demo::SetupFrame ()
{
  if (do_demo == 0)
  {
    // Don't do the demo but print out information about
    // where to get all stuff.
    if (!myG3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;
    int col_bg = myG2D->FindRGB (200, 180, 180);
    int col_fgdata = myG2D->FindRGB (20, 70, 20);
    int col_bgsel = myG2D->FindRGB (255, 255, 255);
    int col_fgsel = myG2D->FindRGB (0, 0, 0);
    myG2D->Clear (col_bg);
    int tx = 10;
    int ty = 10;
    GfxWrite (tx, ty, col_black, col_bg,
    	"This is the Crystal Space Demo application"); ty += 10;
    GfxWrite (tx, ty, col_black, col_bg,
    	"To use this demo you need to give a data file."); ty += 10;
    GfxWrite (tx, ty, col_black, col_bg,
    	"Download 'demodata.zip' from"); ty += 10;
    GfxWrite (tx, ty, col_fgdata, col_bg,
    	"    ftp://sunsite.dk/projects/crystal/cs098/levels/demodata.zip"); ty += 10;

    ty += 10;

    GfxWrite (tx, ty, col_black, col_bg,
    	"After you downloaded demodata you can rerun this demo as follows:"); ty += 10;
    GfxWrite (tx, ty, col_fgdata, col_bg,
    	"    csdemo demodata.zip"); ty += 10;
    ty += 10;
    GfxWrite (tx, ty, col_black, col_bg,
    	"or you can run it with OpenGL and a higher resolution:"); ty += 10;
    GfxWrite (tx, ty, col_fgdata, col_bg,
    	"    csdemo demodata.zip -video=opengl -mode=800x600"); ty += 10;

    ty += 10;

    if (demos.Length () == 0)
    {
      GfxWrite (tx, ty, col_black, col_bg,
    	"I could not find any data files in this and the data directory!");
      ty += 10;
      GfxWrite (tx, ty, col_black, col_bg,
    	"Press ESC to exit this program.");
      ty += 10;
    }
    else
    {
      GfxWrite (tx, ty, col_black, col_bg,
    	"Here are all the demo data files that I could find in this and the data dir:"); ty += 10;
      GfxWrite (tx, ty, col_black, col_bg,
    	"You can select one to run that demo or press ESC to exit this program."); ty += 10;
      ty += 10;
      first_y = ty;
      size_t i;
      for (i = 0 ; i < demos.Length () ; i++)
      {
	int bg = col_bg;
	int fg = col_fgdata;
        if (selected_demo == i)
	{
	  bg = col_bgsel;
	  fg = col_fgsel;
	  myG2D->DrawBox (tx, ty, myG2D->GetWidth ()-2*tx, 10, bg);
	}
        GfxWrite (tx+30, ty, fg, bg, demos.Get (i)); ty += 10;
      }
    }

    return;
  }
  else if (do_demo == 1)
  {
    if (!myG3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;
    myG2D->Clear (0);
    do_demo++;
    return;
  }
  else if (do_demo == 2)
  {
    if (!myG3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;
    myG2D->Clear (0);
    if (!LoadDemoFile (demos.Get (selected_demo)))
      return;
    do_demo++;
    return;
  }

  csTicks elapsed_time, current_time;
  elapsed_time = vc->GetElapsedTicks ();
  current_time = vc->GetCurrentTicks ();

  // since no time has passed, the animated screen image stays the same.
  // avoid drawing this, it will only fill up queues and cause jerky
  // movement on some hardware/drivers.
  if (elapsed_time == 0) return;

  // Now rotate the camera according to keyboard state
  csReversibleTransform& camtrans = view->GetCamera ()->GetTransform ();
  if (map_enabled < MAP_EDIT)
  {
    float speed = (elapsed_time / 1000.0f) * (0.03f * 20.0f);
    if (kbd->GetKeyState (CSKEY_RIGHT))
      camtrans.RotateThis (CS_VEC_ROT_RIGHT, speed);
    if (kbd->GetKeyState (CSKEY_LEFT))
      camtrans.RotateThis (CS_VEC_ROT_LEFT, speed);
    if (kbd->GetKeyState (CSKEY_PGUP))
      camtrans.RotateThis (CS_VEC_TILT_UP, speed);
    if (kbd->GetKeyState (CSKEY_PGDN))
      camtrans.RotateThis (CS_VEC_TILT_DOWN, speed);
    if (kbd->GetKeyState (CSKEY_UP))
      view->GetCamera ()->Move (CS_VEC_FORWARD * 400.0f * speed);
    if (kbd->GetKeyState (CSKEY_DOWN))
      view->GetCamera ()->Move (CS_VEC_BACKWARD * 400.0f * speed);
  }

  if (map_enabled < MAP_EDIT_FORWARD)
    seqmgr->ControlPaths (view->GetCamera (), elapsed_time);
  else if (map_enabled == MAP_EDIT_FORWARD)
  {
    csTicks debug_time;
    csTicks start, total;
    csNamedPath* np = seqmgr->GetSelectedPath (map_selpath, start, total);
    if (np)
    {
      float r = np->GetTimeValue (map_selpoint);
      np->Calculate (r);
      debug_time = csTicks (start + total * r);
    }
    else
      debug_time = 0;	// Not possible!
    seqmgr->DebugPositionObjects (view->GetCamera (), debug_time);
  }

  if (map_enabled == MAP_EDIT_FORWARD)
  {
    csNamedPath* np = seqmgr->GetSelectedPath (map_selpath);
    if (np)
    {
      float r = np->GetTimeValue (map_selpoint);
      np->Calculate (r);
      csVector3 pos, up, forward;
      np->GetInterpolatedPosition (pos);
      np->GetInterpolatedUp (up);
      np->GetInterpolatedForward (forward);
      view->GetCamera ()->GetTransform ().SetOrigin (pos);
      view->GetCamera ()->GetTransform ().LookAt (forward.Unit (), up.Unit ());
    }
  }

  // Tell 3D driver we're going to display 3D things.
  if (!myG3D->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS
  	| CSDRAW_CLEARZBUFFER))
    return;

  if (map_enabled != MAP_EDIT)
  {
    view->Draw ();
    seqmgr->Draw3DEffects (myG3D);
    if (map_enabled == MAP_EDIT_FORWARD)
      csfxFadeToColor (myG3D, 0.3f, csColor (0.0f, 0.0f, 1.0f));
  }

  // Start drawing 2D graphics.
  if (!myG3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;

  if (map_enabled == MAP_EDIT)
    myG2D->Clear (0);
  else if (map_enabled < MAP_EDIT)
    seqmgr->Draw2DEffects (myG2D);
  if (map_enabled >= MAP_OVERLAY)
    seqmgr->DebugDrawPaths (view->GetCamera (), map_selpath,
    	map_tl, map_br, map_selpoint);
  if (map_enabled == MAP_EDIT)
    DrawEditInfo ();

  int fw, fh;
  font->GetMaxSize (fw, fh);
  int tx = 10;
  int ty = myG2D->GetHeight ()-fh-3;
  char messageLine[100];
  messageLine[0] = 0;
  switch (map_enabled)
  {
    case MAP_OFF:
      if (seqmgr->IsSuspended ())
        GfxWrite (tx, ty, col_black, col_white, "[PAUSED]");
      break;
    case MAP_OVERLAY:
      GfxWrite (tx, ty, col_black, col_white, "%sOverlay (%s)",
        seqmgr->IsSuspended () ? "[PAUSED] " : "",
      	map_selpath);
      break;
    case MAP_EDIT:
      GfxWrite (tx, ty, col_black, col_white, "Edit (%s)",
      	map_selpath);
      break;
    case MAP_EDIT_FORWARD:
      GfxWrite (tx, ty, col_black, col_white, "Forward/Up (%s)",
      	map_selpath);
      break;
  }

  if (!message.IsEmpty())
  {
    GfxWrite (10, 10, col_black, message_error ? col_red : col_white, message);
    if (current_time > message_timer) message.Empty();
  }
}

void Demo::FinishFrame ()
{
  myG3D->FinishDraw ();
  myG3D->Print (0);
}

void Demo::DrawEditInfo ()
{
  int fw, fh;
  font->GetMaxSize (fw, fh);
  fh += 2;
  int dim = myG2D->GetHeight ()-10;
  myG2D->DrawBox (dim+5, 0, myG2D->GetWidth ()-dim-5,
  	myG2D->GetHeight (), col_white);
  csTicks start, total;
  csNamedPath* np = seqmgr->GetSelectedPath (map_selpath, start, total);
  if (np)
  {
    int ww = dim+10;
    int hh = 10;
    GfxWrite (ww, hh, col_black, col_white, "Point %d", map_selpoint); hh += fh;
    csVector3 v, fwd, up;
    np->GetPositionVector (map_selpoint, v);
    np->GetForwardVector (map_selpoint, fwd);
    np->GetUpVector (map_selpoint, up);
    GfxWrite (ww, hh, col_black, col_white, "P(%g,%g,%g)",
    	v.x, v.y, v.z); hh += fh;
    GfxWrite (ww, hh, col_black, col_white, "F(%.2g,%.2g,%.2g)",
    	fwd.x, fwd.y, fwd.z); hh += fh;
    GfxWrite (ww, hh, col_black, col_white, "U(%.2g,%.2g,%.2g)",
    	up.x, up.y, up.z); hh += fh;
    float t = np->GetTimeValue (map_selpoint);
    csTicks tms = csTicks (t*total);
    GfxWrite (ww, hh, col_black, col_white, "tot time %d ms", total); hh += fh;
    GfxWrite (ww, hh, col_black, col_white, "rel time %d ms", tms); hh += fh;
    GfxWrite (ww, hh, col_black, col_white, "Left Path Info:"); hh += fh;
    if (map_selpoint > 0)
    {
      csVector3 v1;
      np->GetPositionVector (map_selpoint-1, v1);
      float d = csQsqrt (csSquaredDist::PointPoint (v, v1));
      float t1 = np->GetTimeValue (map_selpoint-1);
      float dr = t-t1;
      float speed = (float) fabs (dr) / d;
      csTicks tms1 = csTicks (t1*total);
      GfxWrite (ww+20, hh, col_black, col_white, "len %g", d); hh += fh;
      GfxWrite (ww+20, hh, col_black, col_white, "dr %g", dr); hh += fh;
      GfxWrite (ww+20, hh, col_black, col_white, "speed %g", speed); hh += fh;
      GfxWrite (ww+20, hh, col_black, col_white, "rel time %d ms",
      	tms-tms1); hh += fh;
    }
    GfxWrite (ww, hh, col_black, col_white, "Right Path Info:"); hh += fh;
    if (map_selpoint < np->GetPointCount ()-1)
    {
      csVector3 v1;
      np->GetPositionVector (map_selpoint+1, v1);
      float t1 = np->GetTimeValue (map_selpoint+1);
      float dr = t1-t;
      float d = csQsqrt (csSquaredDist::PointPoint (v, v1));
      float speed = (float) fabs (dr) / d;
      csTicks tms1 = csTicks (t1*total);
      GfxWrite (ww+20, hh, col_black, col_white, "len %g", d); hh += fh;
      GfxWrite (ww+20, hh, col_black, col_white, "dr %g", dr); hh += fh;
      GfxWrite (ww+20, hh, col_black, col_white, "speed %g", speed); hh += fh;
      GfxWrite (ww+20, hh, col_black, col_white, "rel time %d ms",
      	tms1-tms); hh += fh;
    }
  }
}

bool Demo::DemoHandleEvent (iEvent &Event)
{
  if ((Event.Type == csevKeyboard) &&
    (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown))
  {
    csTicks elapsed_time, current_time;
    elapsed_time = vc->GetElapsedTicks ();
    current_time = vc->GetCurrentTicks ();
    csKeyModifiers m;
    csKeyEventHelper::GetModifiers (&Event, m);
    bool shift = m.modifiers[csKeyModifierTypeShift] != 0;
    bool alt = m.modifiers[csKeyModifierTypeAlt] != 0;
    bool ctrl = m.modifiers[csKeyModifierTypeCtrl] != 0;
    utf32_char keyCode = csKeyEventHelper::GetCookedCode (&Event);

#if 0
    if (do_demo != 3)
    {
      if (keyCode == CSKEY_ESC)
      {
	csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
	if (q)
	  q->GetEventOutlet()->Broadcast (cscmdQuit);
        return true;
      }
    }
    else
#endif
    if (map_enabled == MAP_EDIT_FORWARD)
    {
      //==============================
      // Handle keys in path_edit_forward mode.
      //==============================
      csNamedPath* np = seqmgr->GetSelectedPath (map_selpath);
      if (np)
      {
        float dx = map_br.x - map_tl.x;
        float speed;
        if (shift) speed = dx / 20.0f;
        else if (ctrl) speed = dx / 600.0f;
        else speed = dx / 100.0f;
        if (keyCode == CSKEY_UP)
        {
          csVector3 v;
	  np->GetPositionVector (map_selpoint, v);
          v.y += speed;
	  np->SetPositionVector (map_selpoint, v);
	  ShowMessage ("Y location set at '%g'", v.y);
          return true;
        }
        if (keyCode == CSKEY_DOWN)
        {
          csVector3 v;
	  np->GetPositionVector (map_selpoint, v);
          v.y -= speed;
	  np->SetPositionVector (map_selpoint, v);
	  ShowMessage ("Y location set at '%g'", v.y);
          return true;
        }
        if (keyCode == CSKEY_LEFT)
        {
          csVector3 up, forward;
	  np->GetUpVector (map_selpoint, up);
	  np->GetForwardVector (map_selpoint, forward);
	  csReversibleTransform trans = view->GetCamera ()->GetTransform ();
          trans.LookAt (forward.Unit (), up.Unit ());
	  trans.RotateThis (csVector3 (0.0f, 0.0f, 1.0f), -0.1f);
	  up = trans.This2Other (csVector3 (0.0f, 1.0f, 0.0f)) - trans.GetOrigin ();
	  np->SetUpVector (map_selpoint, up);
	  ShowMessage ("Up vector set at '%.3g,%.3g,%.3g'", up.x, up.y, up.z);
          return true;
        }
        if (keyCode == CSKEY_RIGHT)
        {
          csVector3 up, forward;
	  np->GetUpVector (map_selpoint, up);
	  np->GetForwardVector (map_selpoint, forward);
	  csReversibleTransform trans = view->GetCamera ()->GetTransform ();
          trans.LookAt (forward.Unit (), up.Unit ());
	  trans.RotateThis (csVector3 (0.0f, 0.0f, 1.0f), 0.1f);
	  up = trans.This2Other (csVector3 (0.0f, 1.0f, 0.0f)) - trans.GetOrigin ();
	  np->SetUpVector (map_selpoint, up);
	  ShowMessage ("Up vector set at '%.3g,%.3g,%.3g'", up.x, up.y, up.z);
          return true;
        }
      }
      switch (keyCode)
      {
        case 'c':
          map_enabled = MAP_EDIT;
	  return true;
	case 'y':
	  // Average the 'y' of this point so that it is on a line
	  // with the previous and next point.
	  // Make the forward vector look along the path. i.e. let it look
	  // to an average direction as specified by next and previous point.
	  if (map_selpoint <= 0 || map_selpoint >= np->GetPointCount ()-1)
	  {
	    ShowMessage ("The 'y' operation can't work on this point!\n");
	  }
	  else
	  {
            csVector3 v1, v2, v3;
	    np->GetPositionVector (map_selpoint-1, v1);
	    np->GetPositionVector (map_selpoint, v2);
	    np->GetPositionVector (map_selpoint+1, v3);
	    if (ABS (v1.x-v3.x) > ABS (v1.z-v3.z))
	      v2.y = v1.y + (v3.y-v1.y) * (v1.x-v2.x) / (v1.x-v3.x);
	    else
	      v2.y = v1.y + (v3.y-v1.y) * (v1.z-v2.z) / (v1.z-v3.z);
	    ShowMessage ("Y location set at '%g'", v2.y);
	    np->SetPositionVector (map_selpoint, v2);
	  }
	  break;
	case '0':
	  // Let the up vector point really upwards.
	  {
	    csVector3 forward;
	    np->GetForwardVector (map_selpoint, forward);
            csVector3 up;
	    up = csVector3 (0, 1, 0) % forward;
	    up = - (up % forward);
	    np->SetUpVector (map_selpoint, up);
	  }
	  break;
	case CSKEY_BACKSPACE:
	  // Change direction of the forward vector.
	  {
	    csVector3 forward;
	    np->GetForwardVector (map_selpoint, forward);
	    np->SetForwardVector (map_selpoint, -forward);
	  }
	  break;
        case '=':
	  // Make the forward vector look along the path. i.e. let it look
	  // to an average direction as specified by next and previous point.
	  if (map_selpoint <= 0 || map_selpoint >= np->GetPointCount ()-1)
	  {
	    ShowMessage ("The '=' operation can't work on this point!\n");
	  }
	  else
	  {
            csVector3 v1, v2;
	    np->GetPositionVector (map_selpoint-1, v1);
	    np->GetPositionVector (map_selpoint+1, v2);
	    csVector3 forward = (v2-v1).Unit ();
	    np->SetForwardVector (map_selpoint, forward);
            csVector3 up;
	    np->GetUpVector (map_selpoint, up);
	    up = up % forward;
	    up = - (up % forward);
	    np->SetUpVector (map_selpoint, up);
	  }
	  break;
        case '-':
	  // Make the forward vector look along the path. i.e. let it look
	  // backward to the previous point in the path if there is one.
	  if (map_selpoint <= 0)
	  {
            csVector3 v1, v2;
	    np->GetPositionVector (map_selpoint+1, v1);
	    np->GetPositionVector (map_selpoint, v2);
	    csVector3 forward = (v2-v1).Unit ();
	    np->SetForwardVector (map_selpoint, forward);
            csVector3 up;
	    np->GetUpVector (map_selpoint, up);
	    up = up % forward;
	    up = - (up % forward);
	    np->SetUpVector (map_selpoint, up);
	  }
	  else
	  {
            csVector3 v1, v2;
	    np->GetPositionVector (map_selpoint, v1);
	    np->GetPositionVector (map_selpoint-1, v2);
	    csVector3 forward = (v2-v1).Unit ();
	    np->SetForwardVector (map_selpoint, forward);
            csVector3 up;
	    np->GetUpVector (map_selpoint, up);
	    up = up % forward;
	    up = - (up % forward);
	    np->SetUpVector (map_selpoint, up);
	  }
	  break;
        case '+':
	  // Make the forward vector look along the path. i.e. let it look
	  // to the next point in the path if there is one.
	  if (map_selpoint >= np->GetPointCount ()-1)
	  {
            csVector3 v1, v2;
	    np->GetPositionVector (map_selpoint-1, v1);
	    np->GetPositionVector (map_selpoint, v2);
	    csVector3 forward = (v2-v1).Unit ();
	    np->SetForwardVector (map_selpoint, forward);
            csVector3 up;
	    np->GetUpVector (map_selpoint, up);
	    up = up % forward;
	    up = - (up % forward);
	    np->SetUpVector (map_selpoint, up);
	  }
	  else
	  {
            csVector3 v1, v2;
	    np->GetPositionVector (map_selpoint, v1);
	    np->GetPositionVector (map_selpoint+1, v2);
	    csVector3 forward = (v2-v1).Unit ();
	    np->SetForwardVector (map_selpoint, forward);
            csVector3 up;
	    np->GetUpVector (map_selpoint, up);
	    up = up % forward;
	    up = - (up % forward);
	    np->SetUpVector (map_selpoint, up);
	  }
	  break;
      }
    }
    else if (map_enabled == MAP_EDIT)
    {
      //==============================
      // Handle keys in path editing mode.
      //==============================
      csNamedPath* np = seqmgr->GetSelectedPath (map_selpath);
      float dx = map_br.x - map_tl.x;
      float dy = map_br.y - map_tl.y;
      float speed;
      if (shift) speed = dx / 20.0f;
      else if (ctrl) speed = dx / 600.0f;
      else speed = dx / 100.0f;
      if (np)
      {
        if (keyCode == CSKEY_UP)
        {
	  if (alt)
	  {
	    map_tl.y -= dy / 10.0f;
	    map_br.y -= dy / 10.0f;
	  }
	  else
	  {
            csVector3 v;
	    np->GetPositionVector (map_selpoint, v);
            v.z += speed;
	    np->SetPositionVector (map_selpoint, v);
	  }
          return true;
        }
        if (keyCode == CSKEY_DOWN)
        {
	  if (alt)
	  {
	    map_tl.y += dy / 10.0f;
	    map_br.y += dy / 10.0f;
	  }
	  else
	  {
            csVector3 v;
	    np->GetPositionVector (map_selpoint, v);
            v.z -= speed;
	    np->SetPositionVector (map_selpoint, v);
	  }
          return true;
        }
        if (keyCode == CSKEY_LEFT)
        {
	  if (alt)
	  {
	    map_tl.x -= dx / 10.0f;
	    map_br.x -= dx / 10.0f;
	  }
	  else
	  {
            csVector3 v;
	    np->GetPositionVector (map_selpoint, v);
            v.x -= speed;
	    np->SetPositionVector (map_selpoint, v);
	  }
          return true;
        }
        if (keyCode == CSKEY_RIGHT)
        {
	  if (alt)
	  {
	    map_tl.x += dx / 10.0f;
	    map_br.x += dx / 10.0f;
	  }
	  else
	  {
            csVector3 v;
	    np->GetPositionVector (map_selpoint, v);
            v.x += speed;
	    np->SetPositionVector (map_selpoint, v);
	  }
          return true;
        }
      }
      switch (keyCode)
      {
	case 'm':
          map_enabled = MAP_OFF;
          return true;
	case 's':
	  if (np)
	  {
	    char buf[200], backup[200];
	    strcpy (buf, "/data/demo/paths/");
	    strcat (buf, np->GetName ());
	    // Make a backup of the original file.
	    strcpy (backup, buf);
	    strcat (backup, ".bak");
	    myVFS->DeleteFile (backup);
	    csRef<iDataBuffer> dbuf (myVFS->ReadFile (buf));
	    if (dbuf)
	    {
	      if (dbuf->GetSize ())
	        myVFS->WriteFile (backup, **dbuf, dbuf->GetSize ());
	    }

	    csRef<iFile> fp (myVFS->Open (buf, VFS_FILE_WRITE));
	    if (fp)
	    {
	      int i, num = np->GetPointCount ();
	      FileWrite (fp, "    NUM (%d)\n", num);
	      const float* t = np->GetTimeValues ();
	      FileWrite (fp, "    TIMES (%g", t[0]);
	      for (i = 1 ; i < num ; i++)
	        FileWrite (fp, ",%g", t[i]);
	      FileWrite (fp, ")\n");
	      FileWrite (fp, "    POS (\n");
	      for (i = 0 ; i < num ; i++)
	      {
	        csVector3 v;
		np->GetPositionVector (i, v);
	        FileWrite (fp, "      V (%g,%g,%g)\n", v.x, v.y, v.z);
	      }
	      FileWrite (fp, "    )\n");
	      FileWrite (fp, "    FORWARD (\n");
	      for (i = 0 ; i < num ; i++)
	      {
	        csVector3 v;
		np->GetForwardVector (i, v);
	        FileWrite (fp, "      V (%g,%g,%g)\n", v.x, v.y, v.z);
	      }
	      FileWrite (fp, "    )\n");
	      FileWrite (fp, "    UP (\n");
	      for (i = 0 ; i < num ; i++)
	      {
	        csVector3 v;
		np->GetUpVector (i, v);
	        FileWrite (fp, "      V (%g,%g,%g)\n", v.x, v.y, v.z);
	      }
	      FileWrite (fp, "    )\n");
	      ShowMessage ("Wrote path to file '%s'", buf);
	    }
	    else
	      ShowError ("Error writing to file '%s'!", buf);
	  }
	  break;
        case 'i':
	  if (np)
	  {
	    np->InsertPoint (map_selpoint);
	    map_selpoint++;
	    if (map_selpoint == np->GetPointCount ()-1)
	    {
	      csVector3 v;
	      np->GetPositionVector (map_selpoint-1, v);
	      np->SetPositionVector (map_selpoint, v);
	      np->GetUpVector (map_selpoint-1, v);
	      np->SetUpVector (map_selpoint, v);
	      np->GetForwardVector (map_selpoint-1, v);
	      np->SetForwardVector (map_selpoint, v);
	      np->SetTimeValue (map_selpoint,
	    	  np->GetTimeValue (map_selpoint-1));
	    }
	    else
	    {
	      csVector3 v1, v2;
	      np->GetPositionVector (map_selpoint-1, v1);
	      np->GetPositionVector (map_selpoint+1, v2);
	      np->SetPositionVector (map_selpoint, (v1+v2)/2.);
	      np->GetUpVector (map_selpoint-1, v1);
	      np->GetUpVector (map_selpoint+1, v2);
	      np->SetUpVector (map_selpoint, (v1+v2)/2.);
	      np->GetForwardVector (map_selpoint-1, v1);
	      np->GetForwardVector (map_selpoint+1, v2);
	      np->SetForwardVector (map_selpoint, (v1+v2)/2.);
	      np->SetTimeValue (map_selpoint,
	    	  (np->GetTimeValue (map_selpoint-1)+
		   np->GetTimeValue (map_selpoint+1)) / 2.0f);
	    }
	  }
          break;
        case 'd':
	  if (np)
	  {
	    np->RemovePoint (map_selpoint);
	    if (map_selpoint >= np->GetPointCount ())
	      map_selpoint--;
	  }
	  break;
	case ',':
	  if (np)
	  {
	    if (map_selpoint > 0 && map_selpoint < np->GetPointCount ()-1)
	    {
	      float t = np->GetTimeValue (map_selpoint);
	      float t1 = np->GetTimeValue (map_selpoint-1);
	      float t2 = np->GetTimeValue (map_selpoint+1);
	      float dt = (t2-t1);
	      if (shift) dt /= 5.;
	      else if (ctrl) dt /= 500.;
	      else dt /= 50.;
	      t -= dt;
	      if (t < t1) t = t1;
	      np->SetTimeValue (map_selpoint, t);
	    }
	  }
	  break;
	case '.':
	  if (np)
	  {
	    if (map_selpoint > 0 && map_selpoint < np->GetPointCount ()-1)
	    {
	      float t = np->GetTimeValue (map_selpoint);
	      float t1 = np->GetTimeValue (map_selpoint-1);
	      float t2 = np->GetTimeValue (map_selpoint+1);
	      float dt = (t2-t1);
	      if (shift) dt /= 5.;
	      else if (ctrl) dt /= 500.;
	      else dt /= 50.;
	      t += dt;
	      if (t > t2) t = t2;
	      np->SetTimeValue (map_selpoint, t);
	    }
	  }
	  break;
	case '/':
	  if (np && map_selpoint > 0 && map_selpoint < np->GetPointCount ()-1)
	  {
	    float t1 = np->GetTimeValue (map_selpoint - 1);
	    float t2 = np->GetTimeValue (map_selpoint + 1);
	    np->SetTimeValue (map_selpoint, (t1+t2) / 2.0f);
	  }
	  break;
	case '?':
	  if (np)
	  {
	    int num = np->GetPointCount ();
	    const float* xv, * yv, * zv;
	    xv = np->GetDimensionValues (0);
	    yv = np->GetDimensionValues (1);
	    zv = np->GetDimensionValues (2);
	    csVector3 v0, v1;
	    // Calculate the total length of the path.
	    float totlen = 0;
	    int i;
	    v0.Set (xv[0], yv[0], zv[0]);
	    for (i = 1 ; i < num ; i++)
	    {
	      v1.Set (xv[i], yv[i], zv[i]);
	      float d = csQsqrt (csSquaredDist::PointPoint (v0, v1));
	      totlen += d;
	      v0 = v1;
	    }
	    float list[10000];
	    // Calculate the time value for every path segment,
	    // given the total length of the path.
	    v0.Set (xv[0], yv[0], zv[0]);
	    list[0] = 0;
	    float tot = 0;
	    for (i = 1 ; i < num ; i++)
	    {
	      v1.Set (xv[i], yv[i], zv[i]);
	      float d = csQsqrt (csSquaredDist::PointPoint (v0, v1));
	      tot += d;
	      list[i] = tot / totlen;
	      v0 = v1;
	    }
	    np->SetTimeValues (list);
	  }
	  break;
        case '>':
	  if (np)
	  {
            map_selpoint++;
	    if (map_selpoint >= np->GetPointCount ())
	      map_selpoint = 0;
	  }
	  break;
        case '<':
	  if (np)
	  {
            map_selpoint--;
	    if (map_selpoint < 0)
	      map_selpoint = np->GetPointCount ()-1;
	  }
	  break;
        case 'c':
	  ShowMessage ("Edit forward and up vectors, press 'c' to exit");
	  map_enabled = MAP_EDIT_FORWARD;
	  break;
        case '+':
	  {
	    float dx = (map_br.x-map_tl.x) / 2.0f;
	    float dy = (map_br.y-map_tl.y) / 2.0f;
	    float cx = (map_br.x+map_tl.x) / 2.0f;
	    float cy = (map_br.y+map_tl.y) / 2.0f;
	    map_tl.x = cx-dx * 0.9f;
	    map_tl.y = cy-dy * 0.9f;
	    map_br.x = cx+dx * 0.9f;
	    map_br.y = cy+dy * 0.9f;
	  }
	  break;
        case '-':
	  {
	    float dx = (map_br.x-map_tl.x) / 2.0f;
	    float dy = (map_br.y-map_tl.y) / 2.0f;
	    float cx = (map_br.x+map_tl.x) / 2.0f;
	    float cy = (map_br.y+map_tl.y) / 2.0f;
	    map_tl.x = cx-dx * 1.1f;
	    map_tl.y = cy-dy * 1.1f;
	    map_br.x = cx+dx * 1.1f;
	    map_br.y = cy+dy * 1.1f;
	  }
	  break;
        case '=':
	  map_tl.Set (-1000.0f, 1000.0f);
	  map_br.Set (1000.0f, -1000.0f);
	  break;
        case '[':
	  seqmgr->SelectPreviousPath (map_selpath);
	  np = seqmgr->GetSelectedPath (map_selpath);
	  if (np)
	  {
	    if (map_selpoint >= np->GetPointCount ())
	      map_selpoint = np->GetPointCount ()-1;
	  }
	  break;
        case ']':
	  seqmgr->SelectNextPath (map_selpath);
	  np = seqmgr->GetSelectedPath (map_selpath);
	  if (np)
	  {
	    if (map_selpoint >= np->GetPointCount ())
	      map_selpoint = np->GetPointCount ()-1;
	  }
	  break;
      }
    }
    else
    {
      //==============================
      // Handle keys in demo or overlay mode.
      //==============================
      if (keyCode == CSKEY_ESC)
      {
	csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
	if (q)
	  q->GetEventOutlet()->Broadcast (cscmdQuit);
        return true;
      }
      switch (keyCode)
      {
      	case 'R':
	  ShowMessage ("Restarted sequence manager");
  	  seqmgr->Restart ("/data/demo/sequences");
	  break;
        case 'p':
          if (seqmgr->IsSuspended ()) seqmgr->Resume ();
          else seqmgr->Suspend ();
	  break;
        case '.':
          seqmgr->TimeWarp (20);
	  break;
        case ',':
          seqmgr->TimeWarp ((csTicks)-20);
	  break;
        case '>':
          seqmgr->TimeWarp (2500);
	  break;
        case '<':
          seqmgr->TimeWarp ((csTicks)-2500, true);
	  break;
        case '/':
          seqmgr->TimeWarp (0, true);
	  break;
        case 'm':
	  map_enabled++;
	  if (map_enabled == MAP_EDIT)
	  {
	    ShowMessage ("Map editing mode, press 'm' to exit");
            seqmgr->Suspend ();
	  }
	  break;
      }
    }
  }
  else if (Event.Type == csevMouseDown)
  {
    if (do_demo == 0)
    {
      selected_demo = (Event.Mouse.y - first_y) / 10;
      if ((selected_demo != (size_t)-1) && selected_demo < demos.Length ())
        do_demo = 1;
    }
    else if (do_demo < 3)
    {
    }
    else if (Event.Mouse.Button == 1)
    {
      csVector2 p (Event.Mouse.x, myG2D->GetHeight ()-Event.Mouse.y);
      csVector3 v;
      view->GetCamera ()->InvPerspective (p, 1, v);
      csVector3 vw = view->GetCamera ()->GetTransform ().This2Other (v);
      if (map_enabled == MAP_EDIT_FORWARD)
      {
        csNamedPath* np = seqmgr->GetSelectedPath (map_selpath);
	if (np)
	{
          vw -= view->GetCamera ()->GetTransform ().GetOrigin ();
          np->SetForwardVector (map_selpoint, vw);
	  csVector3 up;
	  np->GetUpVector (map_selpoint, up);
	  up = up % vw;
	  up = - (up % vw);
	  np->SetUpVector (map_selpoint, up);
        }
      }
      else if (map_enabled == MAP_EDIT)
      {
        p.y = Event.Mouse.y;
	int dim = myG2D->GetHeight ()-10;
	float dx = (map_br.x-map_tl.x) / 2.0f;
	float dy = (map_br.y-map_tl.y) / 2.0f;
	float cx = map_tl.x + (map_br.x-map_tl.x)*(1-(dim-p.x)/dim);
	float cy = map_tl.y + (map_br.y-map_tl.y)*(1-(dim-p.y)/dim);
	map_tl.x = cx-dx*.9;
	map_tl.y = cy-dy*.9;
	map_br.x = cx+dx*.9;
	map_br.y = cy+dy*.9;
      }
    }
  }
  else if (Event.Type == csevMouseMove)
  {
    if (do_demo == 0)
    {
      selected_demo = (Event.Mouse.y - first_y) / 10;
      if (!((selected_demo != (size_t)-1) && selected_demo < demos.Length ()))
        selected_demo = (size_t)-1;
    }
  }

  return false;
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  srand (time (0));

  // Create our main class.
  System = new Demo ();

  // Initialize the main system. This will load all needed plug-ins
  // and initialize them.
  if (!System->Initialize (argc, argv, "/config/csdemo.cfg"))
  {
    System->Report (CS_REPORTER_SEVERITY_ERROR, "Error initializing system!");
    Cleanup ();
    exit (1);
  }

  // Main loop.
  csDefaultRunLoop(System->object_reg);

  // Cleanup.
  Cleanup ();

  return 0;
}
