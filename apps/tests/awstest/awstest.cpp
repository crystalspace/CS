/*
    Copyright (C) 2001 by Christopher Nelson

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
#include "csutil/cscolor.h"
#include "csutil/csevent.h"
#include "cstool/csfxscr.h"
#include "cstool/csview.h"
#include "cstool/initapp.h"
#include "csutil/cmdhelp.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "ivideo/graph2d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/natwin.h"
#include "ivideo/fontserv.h"
#include "ivaria/conout.h"
#include "iengine/engine.h"
#include "iengine/sector.h"
#include "iengine/light.h"
#include "iengine/camera.h"
#include "iengine/mesh.h"
#include "iengine/light.h"
#include "iengine/statlght.h"
#include "iengine/movable.h"
#include "iengine/halo.h"
#include "imesh/thing/thing.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/ptextype.h"
#include "imesh/particle.h"
#include "imesh/sprite2d.h"
#include "imesh/sprite3d.h"
#include "imesh/ball.h"
#include "imesh/surf.h"
#include "imesh/object.h"
#include "imap/reader.h"
#include "igraphic/imageio.h"
#include "imap/parser.h"
#include "iengine/material.h"
#include "ivaria/reporter.h"
#include "iutil/eventq.h"
#include "iutil/virtclk.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "qsqrt.h"

#include "csgeom/csrect.h"
#include "csgeom/csrectrg.h"



#include "awstest.h"
#include "plugins/aws/awswin.h"
#include <stdio.h>


//-----------------------------------------------------------------------------

#define  QUERY_PLUG(myPlug, iFace, errMsg) \
  myPlug = CS_QUERY_PLUGIN (plugin_mgr, iFace); \
  if (!myPlug) \
  { \
    Report (CS_REPORTER_SEVERITY_ERROR, errMsg); \
    return false; \
  }

#define  QUERY_PLUG_ID(myPlug, funcid, iFace, errMsg) \
  myPlug = CS_QUERY_PLUGIN_ID (plugin_mgr, funcid, iFace); \
  if (!myPlug) \
  { \
    Report (CS_REPORTER_SEVERITY_ERROR, errMsg); \
    return false; \
  }


extern awsTest *System;

awsTest::awsTest()
{
  engine = NULL;
  myG3D = NULL;
  myG2D = NULL;
  myVFS = NULL;
  myConsole = NULL;
  font = NULL;
  loader = NULL;
  aws = NULL;
  view = NULL;
  message[0] = 0;

//#ifdef 0

  // This code is used to perform dry runs on the csrectregion code.

  csRectRegion rgn;
  csRect r1(10,10,100,100);
  csRect r2(5,50,75,75);
  csRect r3(50,90,100,200);

  rgn.Include(r1);
  rgn.Exclude(r2);
  rgn.Include(r3);

  printf("awstest: number of regions in test: %d\n", rgn.Count());

  int i;
  for(i=0; i<rgn.Count(); ++i)
  {
    csRect &r=rgn.RectAt(i);
    printf("\t%d,%d,%d,%d\n", r.xmin, r.ymin, r.xmax, r.ymax);
  }

//#endif

}

awsTest::~awsTest()
{
  SCF_DEC_REF (view);
  SCF_DEC_REF (font);
  SCF_DEC_REF (aws);
  SCF_DEC_REF (loader);
  SCF_DEC_REF (engine);
  SCF_DEC_REF (myG3D);
  SCF_DEC_REF (myG2D);
  SCF_DEC_REF (myVFS);
  SCF_DEC_REF (myConsole);
}

static bool AwsEventHandler (iEvent& ev)
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


bool 
awsTest::Initialize(int argc, const char* const argv[], const char *iConfigName)
{
  object_reg = csInitializer::CreateEnvironment ();
  if (!object_reg) return false;

  if (!csInitializer::SetupConfigManager (object_reg, iConfigName))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Could not init app!\n");
    return false;
  }

  csInitializer::SetupCommandLineParser (object_reg, argc, argv);
  if (!csInitializer::RequestPlugins (object_reg, CS_REQUEST_END))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Could not init app!\n");
    return false;
  }

  if (!csInitializer::Initialize (object_reg))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Could not init app!\n");
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, AwsEventHandler))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Could not init app!\n");
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
  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);

  // Load the engine plugin.
  Report(CS_REPORTER_SEVERITY_NOTIFY, "Loading engine...\n");
  engine = CS_LOAD_PLUGIN(plugin_mgr, "crystalspace.engine.3d", CS_FUNCID_ENGINE, iEngine);
      
  if (!engine)
  {
    Report(CS_REPORTER_SEVERITY_ERROR, "Could not load the engine plugin!\n");
    return false;
  }
    
  QUERY_PLUG_ID(myG3D, CS_FUNCID_VIDEO, iGraphics3D, "Couldn't load iGraphics3D plugin !\n");
  QUERY_PLUG(myG2D, iGraphics2D, "Couldn't load  iGraphics2D plugin !\n");
  QUERY_PLUG_ID(myVFS, CS_FUNCID_VFS, iVFS, "Couldn't load  iVFS plugin !\n");
  QUERY_PLUG_ID(myConsole, CS_FUNCID_CONSOLE, iConsoleOutput, "Couldn't load iConsoleOutput plugin !\n");
  
  // Load AWS
  Report(CS_REPORTER_SEVERITY_NOTIFY, "Loading AWS...\n");
  aws = CS_LOAD_PLUGIN(plugin_mgr, "crystalspace.window.alternatemanager", "awsManager", iAws);
  
  if (!aws)
  {
    Report(CS_REPORTER_SEVERITY_ERROR, "Could not load the AWS plugin!\n");
    return false;
  }
    
  
  loader = CS_LOAD_PLUGIN(plugin_mgr, "crystalspace.level.loader", CS_FUNCID_LVLLOADER, iLoader);
  if (!loader)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iLoader plugin!\n");
    return false;
  }
 
  // Open the main system. This will open all the previously loaded plug-ins.
  iNativeWindow* nw = myG2D->GetNativeWindow ();
  if (nw) nw->SetTitle ("AWS Test Harness");

  if (!csInitializer::OpenApplication (object_reg))
  {
    Report(CS_REPORTER_SEVERITY_ERROR, "Error opening system!\n");
    return false;
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
	txtmgr->ReserveColor(r * 32, g * 32, b * 64);
	
  txtmgr->SetPalette();

  font = myG2D->GetFontServer()->LoadFont (CSFONT_LARGE);

  // Initialize the console
  if (myConsole != NULL)
    // Don't let messages before this one appear
    myConsole->Clear ();

  // Some commercials...
  Report (CS_REPORTER_SEVERITY_NOTIFY,
    "The Alternate Window System Test Harness.\n");
  
  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  // Create our world.
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Creating world!...\n");

  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error loading 'stone4' texture!\n");
    exit (1);
  }
  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");

  room = engine->CreateSector ("room");
  iMeshWrapper* walls = engine->CreateSectorWallsMesh (room, "walls");
  iThingState* walls_state = SCF_QUERY_INTERFACE (walls->GetMeshObject (),
  	iThingState);
  iPolygon3D* p;
  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (-5, 0, 5));
  p->CreateVertex (csVector3 (5, 0, 5));
  p->CreateVertex (csVector3 (5, 0, -5));
  p->CreateVertex (csVector3 (-5, 0, -5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (-5, 20, -5));
  p->CreateVertex (csVector3 (5, 20, -5));
  p->CreateVertex (csVector3 (5, 20, 5));
  p->CreateVertex (csVector3 (-5, 20, 5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (-5, 20, 5));
  p->CreateVertex (csVector3 (5, 20, 5));
  p->CreateVertex (csVector3 (5, 0, 5));
  p->CreateVertex (csVector3 (-5, 0, 5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (5, 20, 5));
  p->CreateVertex (csVector3 (5, 20, -5));
  p->CreateVertex (csVector3 (5, 0, -5));
  p->CreateVertex (csVector3 (5, 0, 5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (-5, 20, -5));
  p->CreateVertex (csVector3 (-5, 20, 5));
  p->CreateVertex (csVector3 (-5, 0, 5));
  p->CreateVertex (csVector3 (-5, 0, -5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (5, 20, -5));
  p->CreateVertex (csVector3 (-5, 20, -5));
  p->CreateVertex (csVector3 (-5, 0, -5));
  p->CreateVertex (csVector3 (5, 0, -5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  walls_state->DecRef ();
  walls->DecRef ();

  iStatLight* light;
  iLightList* ll = room->GetLights ();
  light = engine->CreateLight (NULL, csVector3 (-3, 5, 0), 10,
  	csColor (1, 0, 0), false);
  ll->Add (light->QueryLight ());
  light->DecRef ();

  light = engine->CreateLight (NULL, csVector3 (3, 5,  0), 10,
  	csColor (0, 0, 1), false);
  ll->Add (light->QueryLight ());
  light->DecRef ();

  light = engine->CreateLight (NULL, csVector3 (0, 5, -3), 10,
  	csColor (0, 1, 0), false);
  ll->Add (light->QueryLight ());
  light->DecRef ();

  engine->Prepare ();

  Report(CS_REPORTER_SEVERITY_NOTIFY, "--------------------------------------\n");
  
  view = new csView (engine, myG3D);
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 5, -3));
  view->SetRectangle (0, 0, myG2D->GetWidth (), myG2D->GetHeight ());

  txtmgr->SetPalette ();
  col_red = txtmgr->FindRGB (255, 0, 0);
  col_blue = txtmgr->FindRGB (0, 0, 255);
  col_white = txtmgr->FindRGB (255, 255, 255);
  col_gray = txtmgr->FindRGB (50, 50, 50);
  col_black = txtmgr->FindRGB (0, 0, 0);
  col_yellow = txtmgr->FindRGB (255, 255, 0);
  col_cyan = txtmgr->FindRGB (0, 255, 255);
  col_green = txtmgr->FindRGB (0, 255, 0);
  
  /// 
  //aws->SetContext(myG2D, myG3D);
  aws->SetDefaultContext(engine, myG3D->GetTextureManager());
  aws->GetPrefMgr()->Load("./data/temp/awstest.def");
  aws->GetPrefMgr()->SelectDefaultSkin("Normal Windows");
  
  printf("aws-debug: Creating splash window...\n");
  awsWindow *test = aws->CreateWindowFrom("Splash");
  awsWindow *test2 = aws->CreateWindowFrom("Another");


  if (test)  test->Show();
  if (test2) test2->Show();
  
  /////////
  
  Report(CS_REPORTER_SEVERITY_NOTIFY, "Init done.\n");

  return true;
}
    
void 
awsTest::SetupFrame()
{
  static int counter=0;
    
  iCamera* c = view->GetCamera();
  
  counter++;
  
  // First get elapsed time from the system driver.
  csTicks elapsed_time, current_time;
  elapsed_time = vc->GetElapsedTicks ();
  current_time = vc->GetCurrentTicks ();
  
  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.03 * 2);

  c->GetTransform ().RotateThis (VEC_ROT_RIGHT, speed);
  
  // Tell 3D driver we're going to display 3D things.
  if (!myG3D->BeginDraw (
      engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
      return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();
    
  // Start drawing 2D graphics.
  if (!myG3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;
    
  sprintf(message, "awsTest(%d)", counter);
  myG2D->Write(font, 5,5, col_black, col_green, message);
  
  aws->Redraw();
  aws->Print(myG3D);
}
  
void 
awsTest::FinishFrame ()
{
  myG3D->FinishDraw ();
  myG3D->Print (NULL);
}
  
bool 
awsTest::HandleEvent (iEvent &Event)
{
  if (Event.Type == csevKeyDown && Event.Key.Code == CSKEY_ESC)
  {
    iEventQueue* q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
    if (q) q->GetEventOutlet()->Broadcast (cscmdQuit);
    return true;
  }
  
  return aws->HandleEvent(Event);
}

void 
awsTest::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  iReporter* rep = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (rep)
    rep->ReportV (severity, "crystalspace.application.awstest", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}


