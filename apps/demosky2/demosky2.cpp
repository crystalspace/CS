/*
    Copyright (C) 2001 by W.C.A. Wijngaards

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
#include "apps/demosky2/demosky2.h"
#include "cstool/csview.h"
#include "cstool/initapp.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/natwin.h"
#include "ivideo/txtmgr.h"
#include "ivideo/fontserv.h"
#include "ivaria/conout.h"
#include "imesh/sprite2d.h"
#include "imesh/ball.h"
#include "imesh/object.h"
#include "imap/parser.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iengine/sector.h"
#include "iengine/camera.h"
#include "iengine/movable.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/thing.h"
#include "ivaria/reporter.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/event.h"
#include "iutil/objreg.h"
#include "iutil/csinput.h"

//------------------------------------------------- We need the 3D engine -----

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// the global system driver variable
DemoSky *System;

void DemoSky::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  iReporter* rep = CS_QUERY_REGISTRY (System->GetObjectRegistry (), iReporter);
  if (rep)
    rep->ReportV (severity, "crystalspace.application.demosky", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

DemoSky::DemoSky ()
{
  view = NULL;
  engine = NULL;
  myG2D = NULL;
  myG3D = NULL;
  LevelLoader = NULL;
  kbd = NULL;
  skydome = NULL;
  skytime = 0.0;
}

DemoSky::~DemoSky ()
{
  if(skydome) skydome->DecRef();
  if (view) view->DecRef ();;
  if(font) font->DecRef ();
  if (LevelLoader) LevelLoader->DecRef ();
  if (engine) engine->DecRef ();
  if (myG2D) myG2D->DecRef ();
  if (myG3D) myG3D->DecRef ();
  if (kbd) kbd->DecRef ();
}

void Cleanup ()
{
  csPrintf ("Cleaning up...\n");
  delete System;
}


bool DemoSky::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  if (!superclass::Initialize (argc, argv, iConfigName))
    return false;

  iObjectRegistry* object_reg = GetObjectRegistry ();
  
  if (!csInitializeApplication (object_reg))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "couldn't init app! (perhaps plugins missing?)");
    return false;
  }

  // Find the pointer to engine plugin
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!engine)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iEngine plugin!");
    abort ();
  }
  engine->IncRef ();

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

  myG2D = CS_QUERY_REGISTRY (object_reg, iGraphics2D);
  if (!myG2D)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iGraphics2D plugin!");
    abort ();
  }
  myG2D->IncRef ();

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (!kbd)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iKeyboardDriver!");
    abort ();
  }
  kbd->IncRef();

  // Open the main system. This will open all the previously loaded plug-ins.
  iNativeWindow* nw = myG2D->GetNativeWindow ();
  if (nw) nw->SetTitle ("Crystal Space Procedural Sky Demo 2");
  if (!Open ())
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

  font = myG2D->GetFontServer()->LoadFont(CSFONT_LARGE);

  // Some commercials...
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Crystal Space Procedural Sky Demo 2.");

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  // Create our world.
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Creating world!...");
  room = engine->CreateSector ("room");

  /// ball mesh
  const char* classId = "crystalspace.mesh.object.ball";
  iMeshFactoryWrapper *mesh_fact = engine->CreateMeshFactory(classId,
    "ballFact");

  if (!LevelLoader->LoadTexture ("white", "/lib/std/white.gif"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.application.demosky2",
      "Error loading 'white' texture!");
    Cleanup ();
    exit (1);
  }
  iMaterialWrapper* wh = engine->GetMaterialList ()->FindByName ("white");

  csVector3 meshposition(0,0,0);
  skydome = engine->CreateMeshWrapper(mesh_fact,
    "skydome", room, meshposition);
  //skydome->SetRenderPriority(); for skyboxes...
  skydome->SetZBufMode(CS_ZBUF_FILL);
  skydome->GetFlags().Set(CS_ENTITY_CAMERA | CS_ENTITY_NOSHADOWS | 
    CS_ENTITY_NOLIGHTING);

  csVector3 meshradius(100.,100.,100.);
  iBallState *ballstate = SCF_QUERY_INTERFACE( skydome->GetMeshObject(), 
    iBallState);
  ballstate->SetRadius( meshradius.x, meshradius.y, meshradius.z );
  ballstate->SetShift( 0,0,0 );
  //ballstate->SetRimVertices( 12 );
  ballstate->SetRimVertices( 24 );
  ballstate->SetMaterialWrapper( wh ); 
  ballstate->SetMixMode( CS_FX_COPY );
  ballstate->SetReversed(true);
  ballstate->SetTopOnly(false);
  ballstate->SetLighting(false);

  /*  Test stuff
  ballstate->SetColor( csColor(0,0,1) );
  float start[] = {0.0, 1,0,1};
  float end[] = {1.0, 0,0,0};
  float* testgrad[] = { start, end, NULL};
  //float* testgrad[] = { {0.0, 1,0,1}, {1.0, 0,0,0}, NULL};
  ballstate->ApplyVertGradient( meshposition.y, meshposition.y+meshradius.y,
    testgrad);

  /// day
  float sky0[] = {-0.01, 0.5,0.6,0.3};
  float sky1[] = {0.0, .5, .6, 1.};
  float sky2[] = {1.0, .1, .3, .8};
  float* testgrad2[] = {sky0, sky1, sky2, NULL};
  /// night
  //float night0[] = {-0.01, 0.5,0.6,0.3};
  //float night1[] = {0.0, 0.1,0.1,0.1};
  //float night2[] = {1.0, 0,0,0};
  //float* testgrad3[] = {night0, night1, night2, NULL};
  ballstate->ApplyVertGradient( meshposition.y, meshposition.y+meshradius.y,
    testgrad2);

  ballstate->ApplyLightSpot( csVector3(20,30,100), 1.0, NULL);
  
  float sunset0[] = {0.0, 0.9,0.9,-0.9};
  float sunset1[] = {0.5, 0.1,-0.6,-0.8};
  float sunset2[] = {1.0, 1,-0.9,1};
  float* testgrad4[] = {sunset0, sunset1, sunset2, NULL};
  ballstate->ApplyLightSpot( csVector3(-30,-20,-20), 2.0, testgrad4);
  */

  ballstate->PaintSky(skytime, NULL, NULL, NULL, NULL);

  ballstate->DecRef();

  engine->Prepare ();

  Report (CS_REPORTER_SEVERITY_NOTIFY, "--------------------------------------");

  // csView is a view encapsulating both a camera and a clipper.
  // You don't have to use csView as you can do the same by
  // manually creating a camera and a clipper but it makes things a little
  // easier.
  view = new csView (engine, myG3D);
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, 0));
  view->SetRectangle (0, 0, myG2D->GetWidth (), myG2D->GetHeight ());

  txtmgr->SetPalette ();

  return true;
}

void DemoSky::NextFrame ()
{
  SysSystemDriver::NextFrame ();
  csTicks elapsed_time, current_time;
  GetElapsedTime (elapsed_time, current_time);

  //printf("elapsed %d\n", (int)elapsed_time);

  float speed = (elapsed_time / 1000.) * (0.03 * 20);
  // animate sky
  float secsperday = 30.;
  skytime += (elapsed_time / ( 1000. * secsperday ));
  while(skytime > 1.0) skytime -= 1.0;
  iBallState *ballstate = SCF_QUERY_INTERFACE( skydome->GetMeshObject(), 
    iBallState);
  ballstate->PaintSky(skytime, NULL, NULL, NULL, NULL);
  ballstate->DecRef();

  // Now rotate the camera according to keyboard state

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

  char buf[255];
  const char *text = "Escape to quit. Arrow keys/pgup/pgdown to move.";
  float hour = skytime * 24. + 8;
  sprintf(buf, "%2dhr. %s", int(hour)%24, text);
  int txtx = 10;
  int txty = myG2D->GetHeight() - 20;
  myG2D->Write(font, txtx+1, txty+1, 
    myG3D->GetTextureManager()->FindRGB(0,0,0), -1, buf);
  myG2D->Write(font, txtx, txty, 
    myG3D->GetTextureManager()->FindRGB(192,192,192), -1, buf);

  // Drawing code ends here.
  myG3D->FinishDraw ();
  // Print the final output.
  myG3D->Print (NULL);
}

bool DemoSky::HandleEvent (iEvent &Event)
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
  System = new DemoSky ();

  // We want at least the minimal set of plugins
  System->RequestPlugin ("crystalspace.kernel.vfs:VFS");
  System->RequestPlugin ("crystalspace.font.server.default:FontServer");
  System->RequestPlugin ("crystalspace.graphic.image.io.multiplex:ImageLoader");
  System->RequestPlugin ("crystalspace.graphics3d.software:VideoDriver");
  System->RequestPlugin ("crystalspace.engine.3d:Engine");
  System->RequestPlugin ("crystalspace.console.output.standard:Console.Output");
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

  Cleanup ();

  return 0;
}


