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
#include "pysimp.h"
#include "csengine/sector.h"
#include "csengine/engine.h"
#include "csengine/csview.h"
#include "csengine/camera.h"
#include "csengine/light.h"
#include "csengine/polygon.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivaria/script.h"
#include "imap/parser.h"

//------------------------------------------------- We need the 3D engine -----

CS_IMPLEMENT_APPLICATION

SCF_REGISTER_STATIC_LIBRARY (engine)

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
}

PySimple::~PySimple ()
{
  SCF_DEC_REF (myG3D);
  if(view)
    delete view;
  if (LevelLoader)
    LevelLoader->DecRef();
}

void cleanup ()
{
  System->console_out ("Cleaning up...\n");
  delete System;
}

bool PySimple::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  if (!superclass::Initialize (argc, argv, iConfigName))
    return false;

  // Find the pointer to engine plugin
  iEngine *Engine = CS_QUERY_PLUGIN (this, iEngine);
  if (!Engine)
  {
    CsPrintf (CS_MSG_FATAL_ERROR, "No iEngine plugin!\n");
    abort ();
  }
  engine = Engine->GetCsEngine ();
  Engine->DecRef ();

  myG3D = CS_QUERY_PLUGIN_ID(this, CS_FUNCID_VIDEO, iGraphics3D);
  if (!myG3D) {
    Printf (CS_MSG_FATAL_ERROR, "No iGraphics3D loader plugin!\n");
    return false;
  }

  LevelLoader = CS_QUERY_PLUGIN_ID (this, CS_FUNCID_LVLLOADER, iLoader);
  if (!LevelLoader)
  {
    CsPrintf (CS_MSG_FATAL_ERROR, "No iLoader plugin!\n");
    abort ();
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!Open ("Simple Crystal Space Python Application"))
  {
    Printf (CS_MSG_FATAL_ERROR, "Error opening system!\n");
    cleanup ();
    exit (1);
  }

  // Some commercials...
  Printf (CS_MSG_INITIALIZATION, "Simple Crystal Space Python Application version 0.1.\n");
  iTextureManager* txtmgr = myG3D->GetTextureManager ();
  txtmgr->SetVerbose (true);

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->EnableLightingCache (false);

  // Create our world.
  Printf (CS_MSG_INITIALIZATION, "Creating world!...\n");

  LevelLoader->LoadTexture ("stone", "/lib/std/stone4.gif");
  csSector *room = engine->CreateCsSector ("room");

  // Initialize the python plugin.
  iScript* is = CS_LOAD_PLUGIN (this, "crystalspace.script.python", "Python", iScript);
  if (is) {

  // Load a python module (scripts/python/pysimp.py).

    if (!is->LoadModule ("pysimp"))
      return 0;

  // Set up our room.

//@@@@@@@@@@@@@@@ PYTHON SCRIPT should create polygons on thing
  // Execute one method defined in pysimp.py
  // This will create the polygons in the room.
    if (!is->RunText ("pysimp.CreateRoom('stone')"))
      return 0;

    is->DecRef ();
  }

//Now try some lua scripting stuff
  is = CS_LOAD_PLUGIN (this, "crystalspace.script.lua", "Lua", iScript);
  if (is) {

    if (!is->LoadModule ("scripts/lua/pysimp.lua"))
      return 0;
   
    if (!is->RunText ("CreateRoom('stone')"))
      return 0;

    is->DecRef();
  }

  csStatLight* light;
  light = new csStatLight (0, 5, 0, 10, 1, 0, 0, false);
  room->AddLight(light);

  engine->Prepare ();

  Printf (CS_MSG_INITIALIZATION, "--------------------------------------\n");

  // csView is a view encapsulating both a camera and a clipper.
  // You don't have to use csView as you can do the same by
  // manually creating a camera and a clipper but it makes things a little
  // easier.
  view = new csView (engine, myG3D);
  view->GetCamera ()->SetSector(&((csSector*)engine->sectors[0])->scfiSector);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 2, 0));
  view->SetRectangle (2, 2, FrameWidth - 4, FrameHeight - 4);

  txtmgr->SetPalette ();
  return true;
}

void PySimple::NextFrame ()
{
  SysSystemDriver::NextFrame ();
  cs_time elapsed_time, current_time;
  GetElapsedTime (elapsed_time, current_time);

  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.) * (0.03 * 20);

  if (GetKeyState (CSKEY_RIGHT))
    view->GetCamera ()->GetTransform ().RotateThis (VEC_ROT_RIGHT, speed);
  if (GetKeyState (CSKEY_LEFT))
    view->GetCamera ()->GetTransform ().RotateThis (VEC_ROT_LEFT, speed);
  if (GetKeyState (CSKEY_PGUP))
    view->GetCamera ()->GetTransform ().RotateThis (VEC_TILT_UP, speed);
  if (GetKeyState (CSKEY_PGDN))
    view->GetCamera ()->GetTransform ().RotateThis (VEC_TILT_DOWN, speed);
  if (GetKeyState (CSKEY_UP))
    view->GetCamera ()->Move (VEC_FORWARD * 4 * speed);
  if (GetKeyState (CSKEY_DOWN))
    view->GetCamera ()->Move (VEC_BACKWARD * 4 * speed);

  // Tell 3D driver we're going to display 3D things.
  if (!myG3D->BeginDraw (CSDRAW_3DGRAPHICS)) return;

  if(view)
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
    Shutdown = true;
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
    System->Printf (CS_MSG_FATAL_ERROR, "Error initializing system!\n");
    cleanup ();
    exit (1);
  }

  // Main loop.
  System->Loop ();

  // Cleanup.
  cleanup ();

  return 0;
}
