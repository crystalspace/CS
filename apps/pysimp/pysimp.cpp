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

#include "sysdef.h"
#include "cssys/system.h"
#include "csutil/inifile.h"
#include "csutil/scanstr.h"
#include "pysimp.h"
#include "csengine/sector.h"
#include "csengine/world.h"
#include "csengine/csview.h"
#include "csengine/camera.h"
#include "csengine/light.h"
#include "csengine/polygon.h"
#include "csparser/csloader.h"
#include "igraph3d.h"
#include "itxtmgr.h"
#include "iscript.h"

//------------------------------ We need the VFS plugin and the 3D engine -----

REGISTER_STATIC_LIBRARY (vfs)
REGISTER_STATIC_LIBRARY (engine)

//-----------------------------------------------------------------------------

PySimple::PySimple ()
{
  view = NULL;
  world = NULL;
  motion_flags = 0;
}

PySimple::~PySimple ()
{
  if(view)
    delete view;
}

void cleanup ()
{
  System->console_out ("Cleaning up...\n");
  delete System;
}

bool PySimple::Initialize (int argc, char *argv[], const char *iConfigName)
{
  if (!superclass::Initialize (argc, argv, iConfigName))
    return false;

  // Find the pointer to world plugin
  iWorld *World = QUERY_PLUGIN (this, iWorld);
  if (!World)
  {
    CsPrintf (MSG_FATAL_ERROR, "No iWorld plugin!\n");
    abort ();
  }
  CHK (world = World->GetCsWorld ());
  World->DecRef ();

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!Open ("Simple Crystal Space Python Application"))
  {
    Printf (MSG_FATAL_ERROR, "Error opening system!\n");
    cleanup ();
    exit (1);
  }

  // Some commercials...
  Printf (MSG_INITIALIZATION, "Simple Crystal Space Python Application version 0.1.\n");
  iTextureManager* txtmgr = G3D->GetTextureManager ();
  txtmgr->SetVerbose (true);

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  world->EnableLightingCache (false);

  // Create our world.
  Printf (MSG_INITIALIZATION, "Creating world!...\n");

  csTextureHandle* tm = csLoader::LoadTexture (world, "stone", "/lib/std/stone4.gif");

  iScript* is = CREATE_INSTANCE("crystalspace.script.python", iScript);
  is->Initialize(this);
  is->RunText("import sys");
  is->RunText("sys.path.append('./scripts/');");
  is->RunText("import cshelper");

  if(!is->RunText("import unrmap"))
    return 0;
    
	is->Store("csTextureHandle *", "unrmap.tmptr", tm);
	is->Store("csSector *", "unrmap.roomptr", world->NewSector());

  if(!is->RunText("unrmap.Load('data/entry')"))
    return 0;
    
<<<<<<< pysimp.cpp
/*  csStatLight* light;
  light = new csStatLight (-3, 5, 0, 10, 1, 0, 0, false);
  room->AddLight (light);
  light = new csStatLight (3, 5, 0, 10, 0, 0, 1, false);
  room->AddLight (light);
  light = new csStatLight (0, 5, -3, 10, 0, 1, 0, false);
  room->AddLight (light);*/

  world->Prepare();

  // Create a dynamic light.
//  angle = 0;
//  dynlight = new csDynLight (cos (angle)*3, 17, sin (angle)*3, 7, 1, 0, 0);
//  world->AddDynLight (dynlight);
//  dynlight->SetSector (room);
//  dynlight->Setup ();
=======
  world->Prepare ();
>>>>>>> 1.2

  Printf (MSG_INITIALIZATION, "--------------------------------------\n");

  // csView is a view encapsulating both a camera and a clipper.
  // You don't have to use csView as you can do the same by
  // manually creating a camera and a clipper but it makes things a little
  // easier.
  view = new csView (world, G3D);
  view->SetSector((csSector*)world->sectors[0]);
  view->GetCamera ()->SetPosition (csVector3 (0, 5, 0));
  view->SetRectangle (2, 2, FrameWidth - 4, FrameHeight - 4);
  
  txtmgr->AllocPalette ();
  return true;
}

void PySimple::NextFrame (time_t elapsed_time, time_t current_time)
{
  superclass::NextFrame (elapsed_time, current_time);

  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.) * (0.03 * 20);

  if (GetKeyState (CSKEY_RIGHT))
    view->GetCamera ()->Rotate (VEC_ROT_RIGHT, speed);
  if (GetKeyState (CSKEY_LEFT))
    view->GetCamera ()->Rotate (VEC_ROT_LEFT, speed);
  if (GetKeyState (CSKEY_PGUP))
    view->GetCamera ()->Rotate (VEC_TILT_UP, speed);
  if (GetKeyState (CSKEY_PGDN))
    view->GetCamera ()->Rotate (VEC_TILT_DOWN, speed);
  if (GetKeyState (CSKEY_UP))
    view->GetCamera ()->Move (VEC_FORWARD * 4 * speed);
  if (GetKeyState (CSKEY_DOWN))
    view->GetCamera ()->Move (VEC_BACKWARD * 4 * speed);

  // Tell 3D driver we're going to display 3D things.
  if (!G3D->BeginDraw (CSDRAW_3DGRAPHICS)) return;

  if(view)
    view->Draw ();

  // Drawing code ends here.
  G3D->FinishDraw ();
  // Print the final output.
  G3D->Print (NULL);
}

bool PySimple::HandleEvent (csEvent &Event)
{
  if (superclass::HandleEvent (Event))
    return true;

  if ((Event.Type == csevKeyDown) && (Event.Key.Code == CSKEY_ESC))
  {
    StartShutdown ();
    return true;
  }

  return false;
}

/*
 * This is a debug handler that is called when your
 * program crashes (only on Unix/Linux right now).
 * You can use this to perform some last minutes dumps
 */
void debug_dump ()
{
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  ScanStr("",""); // Force linker to include this since soft3d requires it.

  srand (time (NULL));

  // Create our main class.
  System = new PySimple ();
  // temp hack until we find a better way
  csWorld::System = System;

  // We want at least the minimal set of plugins
  System->RequestPlugin ("crystalspace.kernel.vfs");
  System->RequestPlugin ("crystalspace.graphics3d.software");
  System->RequestPlugin ("crystalspace.engine.core");

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
  if (!System->Initialize (argc, argv, NULL))
  {
    System->Printf (MSG_FATAL_ERROR, "Error initializing system!\n");
    cleanup ();
    exit (1);
  }

  // Main loop.
  System->Loop ();

  // Cleanup.
  cleanup ();

  return 0;
}
