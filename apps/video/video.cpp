/*
    Copyright (C) 2001 by Norman Krämer

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
#include "apps/video/video.h"
#include "csengine/sector.h"
#include "csengine/engine.h"
#include "csengine/csview.h"
#include "csengine/camera.h"
#include "csengine/light.h"
#include "csengine/polygon.h"
#include "csengine/meshobj.h"
#include "csengine/texture.h"
#include "csengine/thing.h"
#include "csparser/csloader.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivaria/conout.h"

//------------------------------------------------- We need the 3D engine -----

REGISTER_STATIC_LIBRARY (engine)

//-----------------------------------------------------------------------------

Video::Video ()
{
  view = NULL;
  engine = NULL;
  pVStream = NULL;
  pVideoFormat = NULL;
}

Video::~Video ()
{
  if (pVStream) pVStream->DecRef ();
  if (pVideoFormat)
  {
    pVideoFormat->Unload ();
    pVideoFormat->DecRef ();
  }
  delete view;
}

void cleanup ()
{
  System->console_out ("Cleaning up...\n");
  delete System;
}

bool Video::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  if (!superclass::Initialize (argc, argv, iConfigName))
    return false;

  // Find the pointer to engine plugin
  iEngine *Engine = QUERY_PLUGIN (this, iEngine);
  if (!Engine)
  {
    CsPrintf (MSG_FATAL_ERROR, "No iEngine plugin!\n");
    abort ();
  }
  engine = Engine->GetCsEngine ();
  Engine->DecRef ();

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!Open ("Video Crystal Space Application"))
  {
    Printf (MSG_FATAL_ERROR, "Error opening system!\n");
    cleanup ();
    exit (1);
  }

  // Setup the texture manager
  iTextureManager* txtmgr = G3D->GetTextureManager ();
  txtmgr->SetVerbose (true);

  // Initialize the texture manager
  txtmgr->ResetPalette ();
  
  // Allocate a uniformly distributed in R,G,B space palette for console
  // The console will crash on some platforms if this isn't initialize properly
  int r,g,b;
  for (r = 0; r < 8; r++)
    for (g = 0; g < 8; g++)
      for (b = 0; b < 4; b++)
	txtmgr->ReserveColor (r * 32, g * 32, b * 64);
  txtmgr->SetPalette ();

  // Some commercials...
  Printf (MSG_INITIALIZATION,
    "Video Crystal Space Application version 0.1.\n");

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->EnableLightingCache (false);

  // Create our world.
  Printf (MSG_INITIALIZATION, "Creating world!...\n");

  csLoader::LoadTexture (engine, "stone", "/lib/std/stone4.gif");
  csMaterialWrapper* tm = engine->GetMaterials ()->FindByName ("stone");

  room = engine->CreateCsSector ("room");
  csThing* walls = engine->CreateSectorWalls (room, "walls");
  csPolygon3D* p;
  p = walls->NewPolygon (tm);
  p->AddVertex (-5, 0, 5);
  p->AddVertex (5, 0, 5);
  p->AddVertex (5, 0, -5);
  p->AddVertex (-5, 0, -5);
  p->SetTextureSpace (p->Vobj (0), p->Vobj (1), 3);

  p = walls->NewPolygon (tm);
  p->AddVertex (-5, 20, -5);
  p->AddVertex (5, 20, -5);
  p->AddVertex (5, 20, 5);
  p->AddVertex (-5, 20, 5);
  p->SetTextureSpace (p->Vobj (0), p->Vobj (1), 3);

  p = walls->NewPolygon (tm);
  p->AddVertex (-5, 20, 5);
  p->AddVertex (5, 20, 5);
  p->AddVertex (5, 0, 5);
  p->AddVertex (-5, 0, 5);
  p->SetTextureSpace (p->Vobj (0), p->Vobj (1), 3);

  p = walls->NewPolygon (tm);
  p->AddVertex (5, 20, 5);
  p->AddVertex (5, 20, -5);
  p->AddVertex (5, 0, -5);
  p->AddVertex (5, 0, 5);
  p->SetTextureSpace (p->Vobj (0), p->Vobj (1), 3);

  p = walls->NewPolygon (tm);
  p->AddVertex (-5, 20, -5);
  p->AddVertex (-5, 20, 5);
  p->AddVertex (-5, 0, 5);
  p->AddVertex (-5, 0, -5);
  p->SetTextureSpace (p->Vobj (0), p->Vobj (1), 3);

  p = walls->NewPolygon (tm);
  p->AddVertex (5, 20, -5);
  p->AddVertex (-5, 20, -5);
  p->AddVertex (-5, 0, -5);
  p->AddVertex (5, 0, -5);
  p->SetTextureSpace (p->Vobj (0), p->Vobj (1), 3);

  csStatLight* light;
  light = new csStatLight (-3, 5, 0, 10, 1, 0, 0, false);
  room->AddLight (light);
  light = new csStatLight (3, 5, 0, 10, 0, 0, 1, false);
  room->AddLight (light);
  light = new csStatLight (0, 5, -3, 10, 0, 1, 0, false);
  room->AddLight (light);

  engine->Prepare ();

  Printf (MSG_INITIALIZATION, "--------------------------------------\n");

  // csView is a view encapsulating both a camera and a clipper.
  // You don't have to use csView as you can do the same by
  // manually creating a camera and a clipper but it makes things a little
  // easier.
  view = new csView (engine, G3D);
  view->SetSector (room);
  view->GetCamera ()->SetPosition (csVector3 (0, 5, -3));
  view->SetRectangle (0, 0, FrameWidth, FrameHeight);

  txtmgr->SetPalette ();

  // load the videoformat plugin
  iSCF *pSCF = QUERY_INTERFACE (this, iSCF);
  Printf (MSG_INITIALIZATION, "Loading an iVideoFormat.\n");
  pVideoFormat = (iStreamFormat*)pSCF->scfCreateInstance ("crystalspace.video.format.avi", 
							  "iStreamFormat", 0);
  pSCF->DecRef ();
  Printf (MSG_INITIALIZATION, "initializing iVideoFormat.\n");
  if (pVideoFormat && pVideoFormat->Initialize (this))
  {
    iVFS *pVFS = QUERY_PLUGIN (this, iVFS);
    if (pVFS)
    {
      Printf (MSG_INITIALIZATION, "Opening the video file.\n");
      iFile *pFile = pVFS->Open ("/this/data/video.avi", VFS_FILE_READ);
      pVFS->DecRef ();
      Printf (MSG_INITIALIZATION, "Scanning the video file.\n");
      if (pFile && pVideoFormat->Load (pFile))
      {
	pFile->DecRef ();
	// get a iterator to enumerate all streams found
	iStreamIterator *it = pVideoFormat->GetStreamIterator ();
	// look up an video stream
	csStreamDescription desc;
	iStream *pStream=NULL, *pS;
	Printf (MSG_INITIALIZATION, "Looking for video stream.\n");
	while (it->HasNext ())
	{
	   pS= it->GetNext ();
	   pS->GetStreamDescription (desc);
	   if (desc.type == CS_STREAMTYPE_VIDEO)
	   {
	     Printf (MSG_INITIALIZATION, "found video stream.\n");
	     pStream = pS;
	     break;
	   }
	}
	it->DecRef ();
	if (pStream)
	{
	  pVStream = QUERY_INTERFACE (pStream, iVideoStream);
	  // show some data we gathered
	  csVideoStreamDescription desc;
	  pVStream->GetStreamDescription (desc);
	  Printf (MSG_INITIALIZATION, "======= video stream data ======\n");
	  Printf (MSG_INITIALIZATION, "Colordepth     : %d\n", desc.colordepth);
	  Printf (MSG_INITIALIZATION, "Framecount     : %ld\n", desc.framecount);
	  Printf (MSG_INITIALIZATION, "Width x Height : %d x %d\n", desc.width, desc.height);
	  Printf (MSG_INITIALIZATION, "Framerate      : %g\n", desc.framerate);
	  Printf (MSG_INITIALIZATION, "Duration       : %ld\n", desc.duration);
	  Printf (MSG_INITIALIZATION, "CODEC          : %s\n", desc.codec);
	  Printf (MSG_INITIALIZATION, "================================\n");

	  // show the video in the center of the window
	  int vw = desc.width/2, vh = desc.height/2;
	  //	  vw = 750, vh =580;
	  int x = (FrameWidth  - vw)/2;
	  int y = (FrameHeight  - vh)/2;
	  //pVStream->SetRect (x, y, desc.width, desc.height);
	  pVStream->SetRect (x, y, vw, vh);
 	}
	else
	  Printf (MSG_DEBUG_0, "No video stream found in video file.\n");
      }
      else
	Printf (MSG_DEBUG_0, "Could not load the video file.\n");
    }
    else
      Printf (MSG_DEBUG_0, "Could not query VFS plugin.\n");
  }
  else
    Printf (MSG_DEBUG_0, "Could not create or initialize an instance of crystalspace.video.format.avi.\n");
  // @@@ DEBUG: IF THIS IS REMOVED THE SPRITE CRASHES!
  engine->Prepare ();

  return true;
}

void Video::NextFrame ()
{
  SysSystemDriver::NextFrame ();
  cs_time elapsed_time, current_time;
  GetElapsedTime (elapsed_time, current_time);

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
    view->GetCamera ()->Move (VEC_FORWARD * 4.0f * speed);
  if (GetKeyState (CSKEY_DOWN))
    view->GetCamera ()->Move (VEC_BACKWARD * 4.0f * speed);

  // Tell 3D driver we're going to display 3D things.
  if (!G3D->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
    return;

  view->Draw ();

  // Start drawing 2D graphics.
  if (!G3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;
  if (pVStream)
    pVStream->NextFrame ();

  // Drawing code ends here.
  G3D->FinishDraw ();
  // Print the final output.
  G3D->Print (NULL);
}

bool Video::HandleEvent (iEvent &Event)
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
  System = new Video ();

  // We want at least the minimal set of plugins
  System->RequestPlugin ("crystalspace.kernel.vfs:VFS");
  System->RequestPlugin ("crystalspace.font.server.default:FontServer");
  System->RequestPlugin ("crystalspace.image.loader:ImageLoader");
  System->RequestPlugin ("crystalspace.graphics3d.software:VideoDriver");
  System->RequestPlugin ("crystalspace.engine.core:Engine");
  System->RequestPlugin ("crystalspace.console.output.standard:Console.Output");

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
