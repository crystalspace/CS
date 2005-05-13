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
#include "csutil/sysfunc.h"
#include "csutil/cscolor.h"
#include "csutil/csevent.h"
#include "cstool/csfxscr.h"
#include "cstool/csview.h"
#include "cstool/initapp.h"
#include "csutil/cmdhelp.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "ivideo/graph2d.h"
#include "ivideo/natwin.h"
#include "ivaria/conout.h"
#include "iengine/engine.h"
#include "iengine/sector.h"
#include "iengine/light.h"
#include "iengine/camera.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/halo.h"
#include "imesh/thing.h"
#include "imesh/particle.h"
#include "imesh/sprite2d.h"
#include "imesh/sprite3d.h"
#include "imesh/ball.h"
#include "imesh/object.h"
#include "imap/reader.h"
#include "igraphic/imageio.h"
#include "imap/loader.h"
#include "iengine/material.h"
#include "ivaria/reporter.h"
#include "iutil/eventq.h"
#include "iutil/virtclk.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "csqsqrt.h"

#include "csgeom/csrect.h"
#include "csgeom/csrectrg.h"

#include "awstest.h"
#include "awssktst.h"
#include <stdio.h>

#include "cuscomp.h"

const int AWSTEST_MULTIPROC  = 0;
const int AWSTEST_SINGLEPROC = 1;
const int AWSTEST_SCREEN     = 2;

const int AWSTEST_CANVAS=AWSTEST_SCREEN;
// @@@@@ engine view as window background doesn't work with the proctex canvases
// @@@@@ yes it does! it just doesn't update right.
//-----------------------------------------------------------------------------

#define  QUERY_REG(myPlug, iFace, errMsg) \
  myPlug = CS_QUERY_REGISTRY (object_reg, iFace); \
  if (!myPlug) \
  { \
    Report (CS_REPORTER_SEVERITY_ERROR, errMsg); \
    return false; \
  }

extern awsTest *System;

awsTest::awsTest()
{
}

awsTest::~awsTest()
{
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
  object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return false;

  if (!csInitializer::SetupConfigManager (object_reg, iConfigName))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Could not init app!");
    return false;
  }

  if (!csInitializer::RequestPlugins (object_reg, CS_REQUEST_END))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Could not init app!");
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, AwsEventHandler))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Could not setup event handler!");
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
  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (object_reg, iPluginManager));

  // Load the engine plugin.
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Loading engine...");
  engine = CS_LOAD_PLUGIN(plugin_mgr, "crystalspace.engine.3d", iEngine);
  if (!engine)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Could not load the engine plugin!");
    return false;
  }
  if (!object_reg->Register (engine, "iEngine"))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Could not register engine!");
    return false;
  }

  QUERY_REG (myG3D, iGraphics3D, "Couldn't load iGraphics3D plugin!");
  QUERY_REG (myG2D, iGraphics2D, "Couldn't load  iGraphics2D plugin!");
  QUERY_REG (myVFS, iVFS, "Couldn't load  iVFS plugin!");
  QUERY_REG (myConsole, iConsoleOutput, "Couldn't load iConsoleOutput plugin!");

  // Load AWS
  Report(CS_REPORTER_SEVERITY_NOTIFY, "Loading AWS...");
  aws = CS_LOAD_PLUGIN(plugin_mgr,
  	"crystalspace.window.alternatemanager", iAws);

  if (!aws)
  {
    Report(CS_REPORTER_SEVERITY_ERROR, "Could not load the AWS plugin!");
    return false;
  }


  loader = CS_LOAD_PLUGIN(plugin_mgr, "crystalspace.level.loader", iLoader);
  if (!loader)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iLoader plugin!");
    return false;
  }
  if (!object_reg->Register (loader, "iLoader"))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Could not register loader!");
    return false;
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  iNativeWindow* nw = myG2D->GetNativeWindow ();
  if (nw) nw->SetTitle ("AWS Test Harness");

  if (!csInitializer::OpenApplication (object_reg))
  {
    Report(CS_REPORTER_SEVERITY_ERROR, "Error opening system!");
    return false;
  }

  font = myG2D->GetFontServer()->LoadFont (CSFONT_LARGE);

  // Initialize the console
  if (myConsole != 0)
    // Don't let messages before this one appear
    myConsole->Clear ();

  // Some commercials...
  Report (CS_REPORTER_SEVERITY_NOTIFY,
    "The Alternate Window System Test Harness.");

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  // Create our world.
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Creating world!...");

  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error loading 'stone4' texture!");
    exit (1);
  }
  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");

  room = engine->CreateSector ("room");
  csRef<iMeshWrapper> walls (engine->CreateSectorWallsMesh (room, "walls"));
  csRef<iThingState> ws =
  	SCF_QUERY_INTERFACE (walls->GetMeshObject (), iThingState);
  csRef<iThingFactoryState> walls_state = ws->GetFactory ();
  walls_state->AddInsideBox (csVector3 (-5, 0, -5), csVector3 (5, 20, 5));
  walls_state->SetPolygonMaterial (CS_POLYRANGE_LAST, tm);
  walls_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST, 3);

  csRef<iLight> light;
  iLightList* ll = room->GetLights ();
  light = engine->CreateLight (0, csVector3 (-3, 5, 0), 10,
  	csColor (1, 0, 0));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (3, 5,  0), 10,
  	csColor (0, 0, 1));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (0, 5, -3), 10,
  	csColor (0, 1, 0));
  ll->Add (light);

  engine->Prepare ();

  Report(CS_REPORTER_SEVERITY_NOTIFY, "--------------------------------------");

  view = csPtr<iView> (new csView (engine, myG3D));
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 5, -3));
  view->SetRectangle (0, 0, myG2D->GetWidth (), myG2D->GetHeight ());

  wview = csPtr<iView> (new csView (engine, myG3D));
  wview->GetCamera ()->SetSector (room);
  wview->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 5, -3));

  col_red = myG2D->FindRGB (255, 0, 0);
  col_blue = myG2D->FindRGB (0, 0, 255);
  col_white = myG2D->FindRGB (255, 255, 255);
  col_gray = myG2D->FindRGB (50, 50, 50);
  col_black = myG2D->FindRGB (0, 0, 0);
  col_yellow = myG2D->FindRGB (255, 255, 0);
  col_cyan = myG2D->FindRGB (0, 255, 255);
  col_green = myG2D->FindRGB (0, 255, 0);

/*  if (AWSTEST_CANVAS == AWSTEST_SINGLEPROC)
  {
    awsCanvas = aws->CreateDefaultCanvas(engine, myG3D->GetTextureManager(), 512, 512, 0);
    //aws->SetFlag(AWSF_AlwaysEraseWindows);  // Only set for surface or direct-drawing w/o engine
  }
  else if (AWSTEST_CANVAS == AWSTEST_MULTIPROC)
  {
    awsCanvas = aws->CreateDefaultCanvas(engine, myG3D->GetTextureManager());
  }
  else
  {
    awsCanvas = aws->CreateCustomCanvas(myG2D, myG3D);
    aws->SetFlag(AWSF_AlwaysRedrawWindows);     // Only set for direct-drawing with engine
  }*/

  aws->SetupCanvas(0, myG2D, myG3D);

  // next, setup sinks before loading any preferences
  awsTestSink *s = new awsTestSink();
  iAwsSink *sink =aws->GetSinkMgr()->CreateSink((intptr_t)s);

  s->SetSink(sink);
  s->SetWindowManager(aws);

  aws->GetSinkMgr()->RegisterSink("testButtonSink", sink);

  // make the manager aware of our custom component
  // the contructor automatically registers so we only need to create it
  (void) new CustomComponentFactory(aws);

  // now load preferences
  if (!aws->GetPrefMgr()->Load("/aws/awstest.def"))
      Report(CS_REPORTER_SEVERITY_ERROR, "couldn't load definition file!");
  aws->GetPrefMgr()->SelectDefaultSkin("Normal Windows");

  csPrintf("aws-debug: Creating splash window...\n");
  iAwsWindow *test = aws->CreateWindowFrom("Splash");
  iAwsWindow *test2 = aws->CreateWindowFrom("Another");
  iAwsWindow *test3 = aws->CreateWindowFrom("Engine View");
  iAwsWindow *test4 = aws->CreateWindowFrom("Layout Test");
  iAwsWindow *test5 = aws->CreateWindowFrom("Form1");

  if(test3)
  {
    iAwsComponent* engine_view = test3->FindChild("Engine View");
    if(engine_view)
      engine_view->SetProperty("view", (intptr_t)(iView*)wview);
  }

  if (test)  test->Show();
  if (test3) test3->Show();
  if (test4) test4->Show();
  if (test5) test5->Show();

  if (test2) 
  {
	  test2->Show();
	  test2->Execute(":MoveTo@Widget(x=500, y=0)");
  }
  //s->SetTestWin(test2);

  /////////

  Report(CS_REPORTER_SEVERITY_NOTIFY, "Init done.");

  return true;
}

void
awsTest::SetupFrame()
{
  static int counter=0;

  iCamera* c = view->GetCamera();
  iCamera* c2 = wview->GetCamera();

  counter++;

  // First get elapsed time from the system driver.
  csTicks elapsed_time, current_time;
  elapsed_time = vc->GetElapsedTicks ();
  current_time = vc->GetCurrentTicks ();

  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0f) * (0.03f * 2);

  c->GetTransform ().RotateThis (CS_VEC_ROT_RIGHT, speed);
  c2->GetTransform ().RotateThis (CS_VEC_ROT_LEFT, speed);

  // Tell 3D driver we're going to display 3D things.
  if (!myG3D->BeginDraw (
      engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
      return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();
  wview->Draw();

  // Start drawing 2D graphics.
  if (!myG3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;

  message.Format ("awsTest(%d)", counter);
  myG2D->Write(font, 5, 5, col_green, -1, message);

  aws->Redraw();
  aws->Print(myG3D, 64);
  /*if (AWSTEST_SINGLEPROCTEXCANVAS)
  {
    csRef<iTextureWrapper> tex (SCF_QUERY_INTERFACE(awsCanvas, iTextureWrapper));
    if (tex)
    {
      myG3D->DrawPixmap(tex->GetTextureHandle(),
                    0,0,512,512,
                    0,0,512,512,
                    0);
    }
  }*/
}

void
awsTest::FinishFrame ()
{
  myG3D->FinishDraw ();
  myG3D->Print (0);
}

bool
awsTest::HandleEvent (iEvent &Event)
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

  return aws->HandleEvent(Event);
}

void
awsTest::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (object_reg, iReporter));
  if (rep)
    rep->ReportV (severity, "crystalspace.application.awstest", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}


