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
*/

#include "cssysdef.h"
#include "cssys/system.h"
#include "csutil/cscolor.h"
#include "cstool/csview.h"
#include "simpmap.h"
#include "iengine/sector.h"
#include "iengine/engine.h"
#include "iengine/camera.h"
#include "iengine/light.h"
#include "iengine/texture.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/material.h"
#include "iengine/campos.h"
#include "imesh/object.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/thing.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "imap/parser.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// The global system driver
Simple *System;

Simple::Simple ()
{
  view = NULL;
  engine = NULL;
  loader = NULL;
  g3d = NULL;
}

Simple::~Simple ()
{
  if (view) view->DecRef ();
  if (engine) engine->DecRef ();
  if (loader) loader->DecRef();
  if (g3d) g3d->DecRef ();
}

void Cleanup ()
{
  System->ConsoleOut ("Cleaning up...\n");
  delete System;
}

bool Simple::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  if (!superclass::Initialize (argc, argv, iConfigName))
    return false;

  // Find the pointer to engine plugin
  engine = CS_QUERY_PLUGIN (this, iEngine);
  if (!engine)
  {
    Printf (CS_MSG_FATAL_ERROR, "No iEngine plugin!\n");
    abort ();
  }

  loader = CS_QUERY_PLUGIN_ID (this, CS_FUNCID_LVLLOADER, iLoader);
  if (!loader)
  {
    Printf (CS_MSG_FATAL_ERROR, "No iLoader plugin!\n");
    abort ();
  }

  g3d = CS_QUERY_PLUGIN_ID (this, CS_FUNCID_VIDEO, iGraphics3D);
  if (!g3d)
  {
    Printf (CS_MSG_FATAL_ERROR, "No iGraphics3D plugin!\n");
    abort ();
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!Open ("Simple Crystal Space Application"))
  {
    Printf (CS_MSG_FATAL_ERROR, "Error opening system!\n");
    Cleanup ();
    exit (1);
  }

  // Setup the texture manager
  iTextureManager* txtmgr = G3D->GetTextureManager ();
  txtmgr->SetVerbose (true);

  // Initialize the texture manager
  txtmgr->ResetPalette ();

  Printf (CS_MSG_INITIALIZATION,
    "Simple Crystal Space Application version 0.1.\n");

  // Create our world.
  Printf (CS_MSG_INITIALIZATION, "Loading world!...\n");

  // Set VFS current directory to the level we want to load.
  VFS->ChDir ("/lev/flarge");
  // Load the level file which is called 'world'.
  if (!loader->LoadMapFile ("world"))
  {
    Printf (CS_MSG_FATAL_ERROR, "Couldn't load level!\n");
    Cleanup ();
    exit (1);
  }

  engine->Prepare ();
  Printf (CS_MSG_INITIALIZATION, "Loaded.\n");

  // Find the starting position in this level.
  csVector3 pos (0, 0, 0);
  if (engine->GetCameraPositionCount () > 0)
  {
    // There is a valid starting position defined in the level file.
    iCameraPosition* campos = engine->GetCameraPosition (0);
    room = engine->FindSector (campos->GetSector ());
    pos = campos->GetPosition ();
  }
  else
  {
    // We didn't find a valid starting position. So we default
    // to going to room called 'room' at position (0,0,0).
    room = engine->FindSector ("room");
  }
  if (!room)
  {
    Printf (CS_MSG_FATAL_ERROR, "Can't find a valid starting position!\n");
    Cleanup ();
    exit (1);
  }

  view = new csView (engine, g3d);
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (pos);
  view->SetRectangle (0, 0, FrameWidth, FrameHeight);

  txtmgr->SetPalette ();
  return true;
}

bool Simple::HandleEvent (iEvent& Event)
{
  if (superclass::HandleEvent (Event))
    return true;

  if (Event.Type == csevKeyDown && Event.Key.Code == CSKEY_ESC)
  {
    Shutdown = true;
    return true;
  }

  return false;
}

void Simple::NextFrame ()
{
  superclass::NextFrame ();
  // First get elapsed time from the system driver.
  cs_time elapsed_time, current_time;
  GetElapsedTime (elapsed_time, current_time);
  
  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.03 * 20);

  iCamera* c = view->GetCamera();
  if (GetKeyState (CSKEY_RIGHT))
    c->GetTransform ().RotateThis (VEC_ROT_RIGHT, speed);
  if (GetKeyState (CSKEY_LEFT))
    c->GetTransform ().RotateThis (VEC_ROT_LEFT, speed);
  if (GetKeyState (CSKEY_PGUP))
    c->GetTransform ().RotateThis (VEC_TILT_UP, speed);
  if (GetKeyState (CSKEY_PGDN))
    c->GetTransform ().RotateThis (VEC_TILT_DOWN, speed);
  if (GetKeyState (CSKEY_UP))
    c->Move (VEC_FORWARD * 4 * speed);
  if (GetKeyState (CSKEY_DOWN))
    c->Move (VEC_BACKWARD * 4 * speed);

  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (
      engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
      return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();

  // Drawing code ends here.
  g3d->FinishDraw ();
  // Display the final output.
  g3d->Print (NULL);
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  srand (time (NULL));

  // Create our main class.
  System = new Simple ();

  // We want at least the minimal set of plugins
  System->RequestPlugin ("crystalspace.kernel.vfs:VFS");
  //@@@ WHY IS THE FONTSERVER NEEDED FOR OPENGL AND NOT FOR SOFTWARE???
  System->RequestPlugin ("crystalspace.font.server.default:FontServer");
  System->RequestPlugin ("crystalspace.graphic.image.io.multiplex:ImageLoader");
  System->RequestPlugin ("crystalspace.graphics3d.software:VideoDriver");
  System->RequestPlugin ("crystalspace.engine.3d:Engine");
  System->RequestPlugin ("crystalspace.level.loader:LevelLoader");

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
  if (!System->Initialize (argc, argv, NULL))
  {
    System->Printf (CS_MSG_FATAL_ERROR, "Error initializing system!\n");
    Cleanup ();
    exit (1);
  }

  // Main loop.
  System->Loop ();

  // Cleanup.
  Cleanup ();

  return 0;
}
