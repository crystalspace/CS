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


    video - A sample app demonstrating the usage of the CS video decoders.
	    The video stream is mapped on a 3d cube as proctexture
*/

#include "cssysdef.h"
#include "csutil/sysfunc.h"
#include "apps/video/video.h"
#include "csutil/cscolor.h"
#include "cstool/initapp.h"
#include "cstool/csview.h"
#include "csutil/cmdhelp.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/fontserv.h"
#include "ivaria/conout.h"
#include "imap/loader.h"
#include "iengine/sector.h"
#include "iengine/engine.h"
#include "iengine/camera.h"
#include "iengine/light.h"
#include "iengine/mesh.h"
#include "iengine/texture.h"
#include "iengine/material.h"
#include "igraphic/imageio.h"
#include "imesh/thing.h"
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
#include "csutil/event.h"

//------------------------------------------------- We need the 3D engine -----

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// The global system driver
Video *System;

Video::Video ()
{
}

Video::~Video ()
{
  if (pVideoFormat)
  {
    pVideoFormat->Unload ();
  }
}

void Video::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (System->object_reg, iReporter));
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
  delete System; System = 0;
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
	CS_REQUEST_CONSOLEOUT,
	CS_REQUEST_END))
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
    exit (-1);
  }

  // Find the pointer to level loader plugin
  LevelLoader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (!LevelLoader)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iLoader plugin!");
    exit (-1);
  }

  myG3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!myG3D)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iGraphics3D plugin!");
    exit (-1);
  }

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (!kbd)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iKeyboardDriver plugin!");
    exit (-1);
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!csInitializer::OpenApplication (object_reg))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error opening system!");
    Cleanup ();
    exit (1);
  }

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

  csRef<iMaterialWrapper> iMW (SCF_QUERY_INTERFACE (tm, iMaterialWrapper));

  room = engine->CreateSector ("room");
  csRef<iMeshWrapper> wallmesh (engine->CreateSectorWallsMesh (room, "walls"));
  csRef<iThingState> ws =
  	SCF_QUERY_INTERFACE (wallmesh->GetMeshObject (), iThingState);
  csRef<iThingFactoryState> walls = ws->GetFactory ();

  walls->AddInsideBox (csVector3 (-5, 0, -5), csVector3 (5, 20, 5));
  walls->SetPolygonMaterial (CS_POLYRANGE_LAST, iMW);
  walls->SetPolygonTextureMapping (CS_POLYRANGE_LAST, 3);

  csRef<iLight> light;
  iLightList* ll = room->GetLights ();
  light = engine->CreateLight (0, csVector3(-3, 5, 0), 10, csColor(1, 0, 0));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3(3, 5, 0), 10, csColor(0, 0, 1));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3(0, 5, -3), 10, csColor(0, 1, 0));
  ll->Add (light);

  engine->Prepare ();

  Report (CS_REPORTER_SEVERITY_NOTIFY, "--------------------------------------");

  // csView is a view encapsulating both a camera and a clipper.
  // You don't have to use csView as you can do the same by
  // manually creating a camera and a clipper but it makes things a little
  // easier.
  view = csPtr<iView> (new csView (engine, myG3D));
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 5, -3));
  iGraphics2D* g2d = myG3D->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  // load the videoformat plugin
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Loading an iVideoFormat.");
  pVideoFormat = CS_LOAD_PLUGIN (plugin_mgr,
    "crystalspace.video.format.avi", iStreamFormat);

  if (pVideoFormat)
  {
    csRef<iVFS> pVFS (CS_QUERY_REGISTRY (object_reg, iVFS));
    if (pVFS)
    {
      Report (CS_REPORTER_SEVERITY_NOTIFY, "Opening the video file.");
      csRef<iFile> pFile (pVFS->Open ("/varia/video.avi", VFS_FILE_READ));
      Report (CS_REPORTER_SEVERITY_NOTIFY, "Scanning the video file.");
      if (pFile && pVideoFormat->Load (pFile))
      {
	// get a iterator to enumerate all streams found
	iStreamIterator *it = pVideoFormat->GetStreamIterator ();
	// look up an video stream
	csStreamDescription desc;
	iStream *pStream=0, *pS;
	Report (CS_REPORTER_SEVERITY_NOTIFY, "Looking for video stream.");
	while (it->HasNext ())
	{
	   pS= it->Next ();
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
	  Report (CS_REPORTER_SEVERITY_NOTIFY, "Colordepth     : %" PRId8, desc.colordepth);
	  Report (CS_REPORTER_SEVERITY_NOTIFY, "Framecount     : %" PRId32, desc.framecount);
	  Report (CS_REPORTER_SEVERITY_NOTIFY, "Width x Height : %d x %d", desc.width, desc.height);
	  Report (CS_REPORTER_SEVERITY_NOTIFY, "Framerate      : %g", desc.framerate);
	  Report (CS_REPORTER_SEVERITY_NOTIFY, "Duration       : %" PRId32, desc.duration);
	  Report (CS_REPORTER_SEVERITY_NOTIFY, "CODEC          : %s", desc.codec);
	  Report (CS_REPORTER_SEVERITY_NOTIFY, "================================");

	  // show the video in the center of the window
	  iGraphics2D* g2d = myG3D->GetDriver2D ();
	  int x = (g2d->GetWidth ()  - desc.width) / 2;
	  int y = (g2d->GetHeight ()  - desc.height) / 2;
	  pVStream->SetRect (x, y, desc.width, desc.height);
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
  float speed = (elapsed_time / 1000.0f) * (0.03f * 20.0f);

  if (kbd->GetKeyState (CSKEY_RIGHT))
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_ROT_RIGHT, speed);
  if (kbd->GetKeyState (CSKEY_LEFT))
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_ROT_LEFT, speed);
  if (kbd->GetKeyState (CSKEY_PGUP))
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_TILT_UP, speed);
  if (kbd->GetKeyState (CSKEY_PGDN))
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_TILT_DOWN, speed);
  if (kbd->GetKeyState (CSKEY_UP))
    view->GetCamera ()->Move (CS_VEC_FORWARD * 4.0f * speed);
  if (kbd->GetKeyState (CSKEY_DOWN))
    view->GetCamera ()->Move (CS_VEC_BACKWARD * 4.0f * speed);

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
  myG3D->Print (0);
}

bool Video::HandleEvent (iEvent &Event)
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
  System = new Video ();

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
