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

#include "pysimp.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

PySimple::PySimple ()
{
  SetApplicationName ("CrystalSpace.PySimp");
  motion_flags = 0;
}

PySimple::~PySimple ()
{
}

bool PySimple::OnInitialize (int argc, char* argv[])
{
  if (!csInitializer::RequestPlugins (GetObjectRegistry(),
  	CS_REQUEST_VFS,
	CS_REQUEST_OPENGL3D,
	CS_REQUEST_ENGINE,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_LEVELLOADER,
	CS_REQUEST_END))
    return ReportError ("Couldn't init app!");

  csBaseEventHandler::Initialize (GetObjectRegistry());

  if (!RegisterQueue (GetObjectRegistry(), 
    csevAllEvents (GetObjectRegistry())))
    return ReportError("Failed to set up event handler!");

  Process = csevProcess (GetObjectRegistry());
  FinalProcess = csevFinalProcess (GetObjectRegistry());
  KeyboardDown = csevKeyboardDown (GetObjectRegistry());

  return true;
}

bool PySimple::Application()
{
  vc = CS_QUERY_REGISTRY (GetObjectRegistry(), iVirtualClock);

  // Find the pointer to engine plugin
  engine = CS_QUERY_REGISTRY (GetObjectRegistry(), iEngine);
  if (!engine)
    return ReportError ("No iEngine plugin!");

  myG3D = CS_QUERY_REGISTRY (GetObjectRegistry(), iGraphics3D);
  if (!myG3D)
    return ReportError ("No iGraphics3D loader plugin!");

  LevelLoader = CS_QUERY_REGISTRY (GetObjectRegistry(), iLoader);
  if (!LevelLoader)
    return ReportError ("No iLoader plugin!");

  kbd = CS_QUERY_REGISTRY (GetObjectRegistry(), iKeyboardDriver);
  if (!kbd)
    return ReportError ("No iKeyboardDriver!");

  // Open the main system. This will open all the previously loaded plug-ins.
  iNativeWindow* nw = myG3D->GetDriver2D ()->GetNativeWindow ();
  if (nw) nw->SetTitle ("Simple Crystal Space Python Application");
  if (!csInitializer::OpenApplication (GetObjectRegistry()))
    return ReportError ("Error opening system!");

  // Some commercials...
  ReportInfo ("Simple Crystal Space Python Application version 0.1.");

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  // Create our world.
  ReportInfo ("Creating world!...");

  LevelLoader->LoadTexture ("stone", "/lib/std/stone4.gif");
  iSector *room = engine->CreateSector ("room");

  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (GetObjectRegistry(), iPluginManager));
  // Initialize the python plugin.
  csRef<iScript> is (CS_LOAD_PLUGIN (plugin_mgr,
      "crystalspace.script.python", iScript));
  if (is)
  {
    char const* module = "pysimp";
    csRef<iCommandLineParser> cmd =
	CS_QUERY_REGISTRY(GetObjectRegistry(), iCommandLineParser);
    if (cmd.IsValid())
    {
      char const* file = cmd->GetName(0);
      if (file != 0)
	module = file;
    }

    // Load a python module.
    ReportInfo ("Loading script file `%s'...", module);
    if (!is->LoadModule (module))
      return false;

    // Set up our room.
    // Execute one method defined in pysimp.py
    // This will create the polygons in the room.
    csString run;
    run << module << ".CreateRoom('stone')";
    if (!is->RunText(run))
      return false;
  }
  else
    ReportError ("Could not load Python plugin");

  csRef<iLight> light;
  light = engine->CreateLight (0, csVector3 (0, 5, 0), 10,
  	csColor (1, 0, 0));
  room->GetLights ()->Add (light);

  engine->Prepare ();

  // csView is a view encapsulating both a camera and a clipper.
  // You don't have to use csView as you can do the same by
  // manually creating a camera and a clipper but it makes things a little
  // easier.
  view = csPtr<iView> (new csView (engine, myG3D));
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 2, 0));
  iGraphics2D* g2d = myG3D->GetDriver2D ();
  view->SetRectangle (2, 2, g2d->GetWidth () - 4, g2d->GetHeight () - 4);

  Run ();

  return true;
}

void PySimple::OnCommandLineHelp ()
{
  csPrintf("\nTo load a Python script other than the default `pysimp.py',\n"
	   "specify its name (without the .py extension) as the one and only\n"
	   "argument to pysimp. The script must define a Python function\n"
	   "named CreateRoom() which accepts a material name as its only\n"
	   "argument, and which sets up the geometry for a `room' in the\n"
	   "sector named \"room\". The specified script will be `imported',\n"
	   "so it must be found in Python's search path (possibly augmented\n"
	   "by PYTHONPATH).\n\n");
}

void PySimple::ProcessFrame ()
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

bool PySimple::OnKeyboard (iEvent& Event)
{
  if ((Event.Name == KeyboardDown) &&
      (csKeyEventHelper::GetCookedCode (&Event) == CSKEY_ESC))
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY (GetObjectRegistry(), iEventQueue));
    if (q)
      q->GetEventOutlet()->Broadcast (csevQuit (GetObjectRegistry()));
    return true;
  }

  return false;
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  return csApplicationRunner<PySimple>::Run (argc, argv);
}
