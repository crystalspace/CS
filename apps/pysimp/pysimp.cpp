/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

/*
 * pysimp - A simple application demonstrating the usage of the python plugin.
 */

#include "cssysdef.h"
#include "cstool/csview.h"
#include "cstool/initapp.h"
#include "csutil/cmdhelp.h"
#include "csutil/cscolor.h"
#include "csutil/event.h"
#include "csutil/scanstr.h"
#include "csutil/sysfunc.h"
#include "iengine/camera.h"
#include "iengine/engine.h"
#include "iengine/light.h"
#include "iengine/sector.h"
#include "igraphic/imageio.h"
#include "imap/loader.h"
#include "iutil/cmdline.h"
#include "iutil/comp.h"
#include "iutil/csinput.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "iutil/virtclk.h"
#include "ivaria/reporter.h"
#include "ivaria/script.h"
#include "ivideo/fontserv.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/natwin.h"

#include "pysimp.h"

//------------------------------------------------- We need the 3D engine -----

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// The global system driver
PySimple *System;

PySimple::PySimple ()
{
  motion_flags = 0;
}

PySimple::~PySimple ()
{
}

void PySimple::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (System->object_reg, iReporter));
  if (rep)
    rep->ReportV (severity, "crystalspace.application.pysimple", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

void Cleanup ()
{
  csPrintf ("Cleaning up...\n");
  iObjectRegistry* object_reg = System->object_reg;
  delete System; System = 0;
  csInitializer::DestroyApplication (object_reg);
}

static bool PyEventHandler (iEvent& ev)
{
  if (ev.Type == csevBroadcast && csCommandEventHelper::GetCode(&ev) == cscmdProcess)
  {
    System->SetupFrame ();
    return true;
  }
  else if (ev.Type == csevBroadcast && csCommandEventHelper::GetCode(&ev) == cscmdFinalProcess)
  {
    System->FinishFrame ();
    return true;
  }
  else
  {
    return System->HandleEvent (ev);
  }
}

bool PySimple::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return false;

  if (!csInitializer::SetupConfigManager (object_reg, iConfigName))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't init app!");
    return false;
  }

  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_OPENGL3D,
	CS_REQUEST_ENGINE,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_LEVELLOADER,
	CS_REQUEST_END))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't init app!");
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, PyEventHandler))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Could not initialize event handler!");
    return false;
  }

  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    csCommandLineHelper::Help (object_reg);
    csPrintf("\nTo load a Python script other than the default `pysimp.py',\n"
	   "specify its name (without the .py extension) as the one and only\n"
	   "argument to pysimp. The script must define a Python function\n"
	   "named CreateRoom() which accepts a material name as its only\n"
	   "argument, and which sets up the geometry for a `room' in the\n"
	   "sector named \"room\". The specified script will be `imported',\n"
	   "so it must be found in Python's search path (possibly augmented\n"
	   "by PYTHONPATH).\n");
    exit (0);
  }

  vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);

  // Find the pointer to engine plugin
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!engine)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iEngine plugin!");
    exit (-1);
  }

  myG3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!myG3D)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iGraphics3D loader plugin!");
    return false;
  }

  LevelLoader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (!LevelLoader)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iLoader plugin!");
    exit (-1);
  }

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (!kbd)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iKeyboardDriver!");
    return false;
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  iNativeWindow* nw = myG3D->GetDriver2D ()->GetNativeWindow ();
  if (nw) nw->SetTitle ("Simple Crystal Space Python Application");
  if (!csInitializer::OpenApplication (object_reg))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error opening system!");
    Cleanup ();
    exit (1);
  }

  // Some commercials...
  Report (CS_REPORTER_SEVERITY_NOTIFY,
    "Simple Crystal Space Python Application version 0.1.");

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  // Create our world.
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Creating world!...");

  LevelLoader->LoadTexture ("stone", "/lib/std/stone4.gif");
  iSector *room = engine->CreateSector ("room");

  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (object_reg, iPluginManager));
  // Initialize the python plugin.
  csRef<iScript> is (CS_LOAD_PLUGIN (plugin_mgr,
      "crystalspace.script.python", iScript));
  if (is)
  {
    char const* module = "pysimp";
    csRef<iCommandLineParser> cmd =
	CS_QUERY_REGISTRY(object_reg, iCommandLineParser);
    if (cmd.IsValid())
    {
      char const* file = cmd->GetName(0);
      if (file != 0)
	module = file;
    }

    // Load a python module.
    Report (CS_REPORTER_SEVERITY_NOTIFY,
      "Loading script file `%s'...\n", module);
    if (!is->LoadModule (module))
      return 0;

    // Set up our room.
    // Execute one method defined in pysimp.py
    // This will create the polygons in the room.
    csString run;
    run << module << ".CreateRoom('stone')";
    if (!is->RunText(run))
      return 0;
  }

  csRef<iLight> light;
  light = engine->CreateLight (0, csVector3 (0, 5, 0), 10,
  	csColor (1, 0, 0));
  room->GetLights ()->Add (light);

  engine->Prepare ();

  Report (CS_REPORTER_SEVERITY_NOTIFY, "------------------------------------");

  // csView is a view encapsulating both a camera and a clipper.
  // You don't have to use csView as you can do the same by
  // manually creating a camera and a clipper but it makes things a little
  // easier.
  view = csPtr<iView> (new csView (engine, myG3D));
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 2, 0));
  iGraphics2D* g2d = myG3D->GetDriver2D ();
  view->SetRectangle (2, 2, g2d->GetWidth () - 4, g2d->GetHeight () - 4);

  return true;
}

void PySimple::SetupFrame ()
{
  csTicks elapsed_time, current_time;
  elapsed_time = vc->GetElapsedTicks ();
  current_time = vc->GetCurrentTicks ();

  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.) * (0.03 * 20);

  if (kbd->GetKeyState (CSKEY_RIGHT))
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_ROT_RIGHT, speed);
  if (kbd->GetKeyState (CSKEY_LEFT))
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_ROT_LEFT, speed);
  if (kbd->GetKeyState (CSKEY_PGUP))
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_TILT_UP, speed);
  if (kbd->GetKeyState (CSKEY_PGDN))
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_TILT_DOWN, speed);
  if (kbd->GetKeyState (CSKEY_UP))
    view->GetCamera ()->Move (CS_VEC_FORWARD * 4 * speed);
  if (kbd->GetKeyState (CSKEY_DOWN))
    view->GetCamera ()->Move (CS_VEC_BACKWARD * 4 * speed);

  // Tell 3D driver we're going to display 3D things.
  if (!myG3D->BeginDraw (CSDRAW_3DGRAPHICS)) return;

  if (view)
    view->Draw ();
}

void PySimple::FinishFrame ()
{
  // Drawing code ends here.
  myG3D->FinishDraw ();
  // Print the final output.
  myG3D->Print (0);
}

bool PySimple::HandleEvent (iEvent &Event)
{
  if ((Event.Type == csevKeyboard) && 
    (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown) &&
    (csKeyEventHelper::GetCookedCode (&Event) == CSKEY_ESC))
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
    if (q)
      q->GetEventOutlet()->Broadcast (cscmdQuit);
    return true;
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
  System = new PySimple ();

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
  if (!System->Initialize (argc, argv, 0))
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
