/*
    Copyright (C) 2001 by Norman Kramer

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
#include "cssys/sysfunc.h"
#include "apps/video/video.h"
#include "csutil/cscolor.h"
#include "cstool/initapp.h"
#include "cstool/csview.h"
#include "csutil/cmdhelp.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/fontserv.h"
#include "ivaria/conout.h"
#include "imap/parser.h"
#include "iengine/sector.h"
#include "iengine/engine.h"
#include "iengine/camera.h"
#include "iengine/light.h"
#include "iengine/statlght.h"
#include "iengine/mesh.h"
#include "iengine/texture.h"
#include "iengine/material.h"
#include "igraphic/imageio.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/thing.h"
#include "imesh/object.h"
#include "iutil/eventq.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/event.h"
#include "iutil/objreg.h"
#include "iutil/csinput.h"
#include "iutil/virtclk.h"
#include "ivaria/reporter.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"

//------------------------------------------------- We need the 3D engine -----

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// The global system driver
Video *System;

Video::Video ()
{
  view = NULL;
  engine = NULL;
  pVStream = NULL;
  pVideoFormat = NULL;
  LevelLoader = NULL;
  myG3D = NULL;
  kbd = NULL;
}

Video::~Video ()
{
  if (pVStream) pVStream->DecRef ();
  if (pVideoFormat)
  {
    pVideoFormat->Unload ();
    pVideoFormat->DecRef ();
  }
  if (view) view->DecRef ();
  if (LevelLoader) LevelLoader->DecRef();
  if (myG3D) myG3D->DecRef ();
  if (kbd) kbd->DecRef ();
  if (engine) engine->DecRef ();
}

void Video::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  iReporter* rep = CS_QUERY_REGISTRY (System->object_reg, iReporter);
  if (rep)
    rep->ReportV (severity, "crystalspace.application.video", msg, arg);
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
  delete System; System = NULL;
  csInitializer::DestroyApplication (object_reg);
}

static bool VideoEventHandler (iEvent& ev)
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
    return System ? System->HandleEvent (ev) : false;
  }
}
bool Video::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  object_reg = csInitializer::CreateEnvironment ();
  if (!object_reg) return false;

  if (!csInitializer::SetupConfigManager (object_reg, iConfigName))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't init app!");
    return false;
  }

  csInitializer::SetupCommandLineParser (object_reg, argc, argv);
  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_SOFTWARE3D,
	CS_REQUEST_ENGINE,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_LEVELLOADER,
	CS_REQUEST_CONSOLEOUT,
	CS_REQUEST_END))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't init app!");
    return false;
  }

  if (!csInitializer::Initialize (object_reg))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't init app!");
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, VideoEventHandler))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't init app!");
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
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);

  // Find the pointer to engine plugin
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!engine)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iEngine plugin!");
    abort ();
  }
  engine->IncRef ();

  // Find the pointer to level loader plugin
  LevelLoader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (!LevelLoader)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iLoader plugin!");
    abort ();
  }
  LevelLoader->IncRef ();

  myG3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!myG3D)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iGraphics3D plugin!");
    abort ();
  }
  myG3D->IncRef ();

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (!kbd)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iKeyboardDriver plugin!");
    abort ();
  }
  kbd->IncRef();

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!csInitializer::OpenApplication (object_reg))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error opening system!");
    Cleanup ();
    exit (1);
  }

  // Setup the texture manager
  iTextureManager* txtmgr = myG3D->GetTextureManager ();
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
  Report (CS_REPORTER_SEVERITY_NOTIFY,
    "Video Crystal Space Application version 0.1.");

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  // Create our world.
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Creating world!...");

  LevelLoader->LoadTexture ("stone", "/lib/std/stone4.gif");
  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");

  iMaterialWrapper *iMW = SCF_QUERY_INTERFACE (tm, iMaterialWrapper);
 
  room = engine->CreateSector ("room");
iMeshWrapper *wallmesh = engine->CreateSectorWallsMesh (room, "walls");
  iThingState* walls = SCF_QUERY_INTERFACE (wallmesh->GetMeshObject (), iThingState);

  csVector3 
	   f1 (-5, 20, 5),
	   f2 ( 5, 20, 5), 
	   f3 ( 5, 0, 5), 
	   f4 (-5, 0, 5), 
	   b1 (-5, 20, -5),
	   b2 ( 5, 20, -5), 
	   b3 ( 5, 0, -5), 
	   b4 (-5, 0, -5);

  iPolygon3D* p = walls->CreatePolygon ("back");
  p->SetMaterial (iMW);
  p->CreateVertex (b4);
  p->CreateVertex (b3);
  p->CreateVertex (b2);
  p->CreateVertex (b1);
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls->CreatePolygon ("front");
  p->SetMaterial (iMW);
  p->CreateVertex (f1);
  p->CreateVertex (f2);
  p->CreateVertex (f3);
  p->CreateVertex (f4);
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls->CreatePolygon ("top");
  p->SetMaterial (iMW);
  p->CreateVertex (b1);
  p->CreateVertex (b2);
  p->CreateVertex (f2);
  p->CreateVertex (f1);
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls->CreatePolygon ("right");
  p->SetMaterial (iMW);
  p->CreateVertex (f2);
  p->CreateVertex (b2);
  p->CreateVertex (b3);
  p->CreateVertex (f3);
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls->CreatePolygon ("left");
  p->SetMaterial (iMW);
  p->CreateVertex (f1);
  p->CreateVertex (f4);
  p->CreateVertex (b4);
  p->CreateVertex (b1);
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls->CreatePolygon ("bottom");
  p->SetMaterial (iMW);
  p->CreateVertex (f4);
  p->CreateVertex (f3);
  p->CreateVertex (b3);
  p->CreateVertex (b4);
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  iMW->DecRef ();
  walls->DecRef ();
  wallmesh->DecRef ();
  
  iStatLight* light;
  iLightList* ll = room->GetLights ();
  light = engine->CreateLight (NULL, csVector3(-3, 5, 0), 10, csColor(1, 0, 0), false);
  ll->Add (light->QueryLight ());
  light->DecRef ();

  light = engine->CreateLight (NULL, csVector3(3, 5, 0), 10, csColor(0, 0, 1), false);
  ll->Add (light->QueryLight ());
  light->DecRef ();

  light = engine->CreateLight (NULL, csVector3(0, 5, -3), 10, csColor(0, 1, 0), false);
  ll->Add (light->QueryLight ());
  light->DecRef ();

  engine->Prepare ();

  Report (CS_REPORTER_SEVERITY_NOTIFY, "--------------------------------------");

  // csView is a view encapsulating both a camera and a clipper.
  // You don't have to use csView as you can do the same by
  // manually creating a camera and a clipper but it makes things a little
  // easier.
  view = new csView (engine, myG3D);
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 5, -3));
  iGraphics2D* g2d = myG3D->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  txtmgr->SetPalette ();

  // load the videoformat plugin
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Loading an iVideoFormat.");
  pVideoFormat = CS_LOAD_PLUGIN (plugin_mgr,
    "crystalspace.video.format.avi", iStreamFormat);

  if (pVideoFormat)
  {
    iVFS *pVFS = CS_QUERY_REGISTRY (object_reg, iVFS);
    if (pVFS)
    {
      Report (CS_REPORTER_SEVERITY_NOTIFY, "Opening the video file.");
      iFile *pFile = pVFS->Open ("/this/data/video.avi", VFS_FILE_READ);
      Report (CS_REPORTER_SEVERITY_NOTIFY, "Scanning the video file.");
      if (pFile && pVideoFormat->Load (pFile))
      {
	pFile->DecRef ();
	// get a iterator to enumerate all streams found
	iStreamIterator *it = pVideoFormat->GetStreamIterator ();
	// look up an video stream
	csStreamDescription desc;
	iStream *pStream=NULL, *pS;
	Report (CS_REPORTER_SEVERITY_NOTIFY, "Looking for video stream.");
	while (it->HasNext ())
	{
	   pS= it->GetNext ();
	   pS->GetStreamDescription (desc);
	   if (desc.type == CS_STREAMTYPE_VIDEO)
	   {
	     Report (CS_REPORTER_SEVERITY_NOTIFY, "found video stream.");
	     pStream = pS;
	     break;
	   }
	}
	it->DecRef ();
	if (pStream)
	{
	  pVStream = SCF_QUERY_INTERFACE (pStream, iVideoStream);
	  // show some data we gathered
	  csVideoStreamDescription desc;
	  pVStream->GetStreamDescription (desc);
	  Report (CS_REPORTER_SEVERITY_NOTIFY, "======= video stream data ======");
	  Report (CS_REPORTER_SEVERITY_NOTIFY, "Colordepth     : %d", desc.colordepth);
	  Report (CS_REPORTER_SEVERITY_NOTIFY, "Framecount     : %ld", desc.framecount);
	  Report (CS_REPORTER_SEVERITY_NOTIFY, "Width x Height : %d x %d", desc.width, desc.height);
	  Report (CS_REPORTER_SEVERITY_NOTIFY, "Framerate      : %g", desc.framerate);
	  Report (CS_REPORTER_SEVERITY_NOTIFY, "Duration       : %ld", desc.duration);
	  Report (CS_REPORTER_SEVERITY_NOTIFY, "CODEC          : %s", desc.codec);
	  Report (CS_REPORTER_SEVERITY_NOTIFY, "================================");

	  // show the video in the center of the window
	  int vw = desc.width/2, vh = desc.height/2;
	  //	  vw = 750, vh =580;
	  iGraphics2D* g2d = myG3D->GetDriver2D ();
	  int x = (g2d->GetWidth ()  - vw)/2;
	  int y = (g2d->GetHeight ()  - vh)/2;
	  //pVStream->SetRect (x, y, desc.width, desc.height);
	  pVStream->SetRect (x, y, vw, vh);
 	}
	else
	  Report (CS_REPORTER_SEVERITY_DEBUG, "No video stream found in video file.");
      }
      else
	Report (CS_REPORTER_SEVERITY_DEBUG, "Could not load the video file.");
    }
    else
      Report (CS_REPORTER_SEVERITY_DEBUG, "Could not query VFS plugin.");
  }
  else
    Report (CS_REPORTER_SEVERITY_DEBUG, "Could not create or initialize an instance of crystalspace.video.format.avi.");
  // @@@ DEBUG: IF THIS IS REMOVED THE SPRITE CRASHES!
  engine->Prepare ();

  return true;
}

void Video::SetupFrame ()
{
  csTicks elapsed_time, current_time;
  elapsed_time = vc->GetElapsedTicks ();
  current_time = vc->GetCurrentTicks ();

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
    view->GetCamera ()->Move (VEC_FORWARD * 4.0f * speed);
  if (kbd->GetKeyState (CSKEY_DOWN))
    view->GetCamera ()->Move (VEC_BACKWARD * 4.0f * speed);

  // Tell 3D driver we're going to display 3D things.
  if (!myG3D->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
    return;

  view->Draw ();

  // Start drawing 2D graphics.
  if (!myG3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;
  if (pVStream)
    pVStream->NextFrame ();
}

void Video::FinishFrame ()
{
  myG3D->FinishDraw ();
  myG3D->Print (NULL);
}

bool Video::HandleEvent (iEvent &Event)
{
  if ((Event.Type == csevKeyDown) && (Event.Key.Code == CSKEY_ESC))
  {
    iEventQueue* q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
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
  System = new Video ();

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
  if (!System->Initialize (argc, argv, NULL))
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
