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

#include "cssysdef.h"
#include "cssys/system.h"
#include "csutil/scanstr.h"
#include "csutil/cscolor.h"
#include "pysimp.h"
#include "iengine/sector.h"
#include "iengine/engine.h"
#include "iengine/camera.h"
#include "iengine/light.h"
#include "iengine/statlght.h"
#include "imesh/thing/polygon.h"
#include "cstool/csview.h"
#include "cstool/initapp.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/natwin.h"
#include "ivideo/txtmgr.h"
#include "ivaria/script.h"
#include "imap/parser.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/csinput.h"
#include "ivaria/reporter.h"
#include "isys/plugin.h"

//------------------------------------------------- We need the 3D engine -----

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// The global system driver
PySimple *System;

PySimple::PySimple ()
{
  view = NULL;
  engine = NULL;
  motion_flags = 0;
  LevelLoader = NULL;
  myG3D = NULL;
  kbd = NULL;
}

PySimple::~PySimple ()
{
  SCF_DEC_REF (myG3D);
  if(view)
    delete view;
  if (LevelLoader)
    LevelLoader->DecRef();
  if (kbd)
    kbd->DecRef();
}

void PySimple::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  iReporter* rep = CS_QUERY_REGISTRY (System->GetObjectRegistry (), iReporter);
  if (rep)
    rep->ReportV (severity, "crystalspace.application.pysimple", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

void PySimple::Help () {
  SysSystemDriver::Help ();
  printf ("  -python            Test the csPython plugin\n");
  printf ("  -lua               Test the csLua plugin\n");
}

void Cleanup ()
{
  csPrintf ("Cleaning up...\n");
  delete System;
}

bool PySimple::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  if (!superclass::Initialize (argc, argv, iConfigName))
    return false;

  iObjectRegistry* object_reg = GetObjectRegistry ();
  
  if (!csInitializeApplication (object_reg))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "couldn't init app! (perhaps some plugins are missing?");
    return false;
  }

  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);

  // Find the pointer to engine plugin
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!engine)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iEngine plugin!");
    abort ();
  }
  engine->IncRef ();

  myG3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!myG3D)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iGraphics3D loader plugin!");
    return false;
  }
  myG3D->IncRef ();

  LevelLoader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (!LevelLoader)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iLoader plugin!");
    abort ();
  }
  LevelLoader->IncRef ();

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (!kbd)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iKeyboardDriver!");
    return false;
  }
  kbd->IncRef();

  // Open the main system. This will open all the previously loaded plug-ins.
  iNativeWindow* nw = myG3D->GetDriver2D ()->GetNativeWindow ();
  if (nw) nw->SetTitle ("Simple Crystal Space Python Application");
  if (!Open ())
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error opening system!");
    Cleanup ();
    exit (1);
  }

  // Some commercials...
  Report (CS_REPORTER_SEVERITY_NOTIFY,
    "Simple Crystal Space Python Application version 0.1.");
  iTextureManager* txtmgr = myG3D->GetTextureManager ();
  txtmgr->SetVerbose (true);

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  // Create our world.
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Creating world!...");

  LevelLoader->LoadTexture ("stone", "/lib/std/stone4.gif");
  iSector *room = engine->CreateSector ("room");

  int testpython=0, testlua=0, i;
  for (i=1; i<argc; i++)
  {
    if (!strcasecmp(argv[i], "-python"))
    {
      testpython=1;
    }
    if (!strcasecmp(argv[i], "-lua"))
    {
      testlua=1;
    }
  }

  if (!testpython && !testlua)
  {
    Report (CS_REPORTER_SEVERITY_ERROR,
      "Please select -python or -lua to select which scripting engine to test!");
    return 0;
  }

  if(testpython)
  {
    // Initialize the python plugin.
    iScript* is = CS_LOAD_PLUGIN (plugin_mgr,
      "crystalspace.script.python", "Python", iScript);
    if (is)
    {
      // Load a python module (scripts/python/pysimp.py).
      if (!is->LoadModule ("pysimp"))
        return 0;

      // Set up our room.
      // Execute one method defined in pysimp.py
      // This will create the polygons in the room.
      if (!is->RunText ("pysimp.CreateRoom('stone')"))
        return 0;

      is->DecRef ();
    }
  }

  if (testlua)
  {
    //Now try some lua scripting stuff
    iScript* is = CS_LOAD_PLUGIN (plugin_mgr,
      "crystalspace.script.lua", "Lua", iScript);
    if (is)
    {
      if (!is->LoadModule ("scripts/lua/pysimp.lua"))
        return 0;
      if (!is->RunText ("CreateRoom('stone')"))
        return 0;
      is->DecRef();
    }
  }

  iStatLight* light;
  light = engine->CreateLight (NULL, csVector3 (0, 5, 0), 10,
  	csColor (1, 0, 0), false);
  room->GetLights ()->AddLight (light->QueryLight ());

  engine->Prepare ();

  Report (CS_REPORTER_SEVERITY_NOTIFY, "--------------------------------------");

  // csView is a view encapsulating both a camera and a clipper.
  // You don't have to use csView as you can do the same by
  // manually creating a camera and a clipper but it makes things a little
  // easier.
  view = new csView (engine, myG3D);
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 2, 0));
  iGraphics2D* g2d = myG3D->GetDriver2D ();
  view->SetRectangle (2, 2, g2d->GetWidth () - 4, g2d->GetHeight () - 4);

  txtmgr->SetPalette ();
  return true;
}

void PySimple::NextFrame ()
{
  SysSystemDriver::NextFrame ();
  csTicks elapsed_time, current_time;
  GetElapsedTime (elapsed_time, current_time);

  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.) * (0.03 * 20);

  if (kbd->GetKeyState (CSKEY_RIGHT))
    view->GetCamera ()->GetTransform ().RotateThis (VEC_ROT_RIGHT, speed);
  if (kbd->GetKeyState (CSKEY_LEFT))
    view->GetCamera ()->GetTransform ().RotateThis (VEC_ROT_LEFT, speed);
  if (kbd->GetKeyState (CSKEY_PGUP))
    view->GetCamera ()->GetTransform ().RotateThis (VEC_TILT_UP, speed);
  if (kbd->GetKeyState (CSKEY_PGDN))
    view->GetCamera ()->GetTransform ().RotateThis (VEC_TILT_DOWN, speed);
  if (kbd->GetKeyState (CSKEY_UP))
    view->GetCamera ()->Move (VEC_FORWARD * 4 * speed);
  if (kbd->GetKeyState (CSKEY_DOWN))
    view->GetCamera ()->Move (VEC_BACKWARD * 4 * speed);

  // Tell 3D driver we're going to display 3D things.
  if (!myG3D->BeginDraw (CSDRAW_3DGRAPHICS)) return;

  if (view)
    view->Draw ();

  // Drawing code ends here.
  myG3D->FinishDraw ();
  // Print the final output.
  myG3D->Print (NULL);
}

bool PySimple::HandleEvent (iEvent &Event)
{
  if (superclass::HandleEvent (Event))
    return true;

  if ((Event.Type == csevKeyDown) && (Event.Key.Code == CSKEY_ESC))
  {
    iEventQueue* q = CS_QUERY_REGISTRY (GetObjectRegistry (), iEventQueue);
    if (q) q->GetEventOutlet()->Broadcast (cscmdQuit);
    return true;
  }

  return false;
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  srand (time (NULL));

  // Create our main class.
  System = new PySimple ();

  // We want at least the minimal set of plugins
  System->RequestPlugin ("crystalspace.kernel.vfs:VFS");
  System->RequestPlugin ("crystalspace.graphic.image.io.multiplex:ImageLoader");
  System->RequestPlugin ("crystalspace.graphics3d.software:VideoDriver");
  System->RequestPlugin ("crystalspace.font.server.default:FontServer");
  System->RequestPlugin ("crystalspace.engine.3d:Engine");
  System->RequestPlugin ("crystalspace.level.loader:LevelLoader");

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
  if (!System->Initialize (argc, argv, NULL))
  {
    System->Report (CS_REPORTER_SEVERITY_ERROR, "Error initializing system!");
    Cleanup ();
    exit (1);
  }

  // Main loop.
  System->Loop ();

  // Cleanup.
  Cleanup ();

  return 0;
}
