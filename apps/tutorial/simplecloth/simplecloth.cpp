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
#include "cssys/sysfunc.h"
#include "iutil/vfs.h"
#include "csutil/cscolor.h"
#include "cstool/csview.h"
#include "cstool/initapp.h"
#include "simplecloth.h"
#include "iutil/eventq.h"
#include "iutil/event.h"
#include "iutil/objreg.h"
#include "iutil/csinput.h"
#include "iutil/virtclk.h"
#include "iengine/sector.h"
#include "iengine/engine.h"
#include "iengine/camera.h"
#include "iengine/light.h"
#include "iengine/statlght.h"
#include "iengine/texture.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/material.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/thing.h"
#include "imesh/clothmesh.h"
#include "imesh/object.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "ivideo/fontserv.h"
#include "igraphic/imageio.h"
#include "imap/parser.h"
#include "ivaria/reporter.h"
#include "ivaria/stdrep.h"
#include "csutil/cmdhelp.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// The global pointer to simple
Simple *simple;

Simple::Simple (iObjectRegistry* object_reg)
{
  Simple::object_reg = object_reg;
}

Simple::~Simple ()
{
}

void Simple::SetupFrame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();

  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.03 * 20);

  iCamera* c = view->GetCamera();
  if (kbd->GetKeyState (CSKEY_RIGHT))
    c->GetTransform ().RotateThis (CS_VEC_ROT_RIGHT, speed);
  if (kbd->GetKeyState (CSKEY_LEFT))
    c->GetTransform ().RotateThis (CS_VEC_ROT_LEFT, speed);
  if (kbd->GetKeyState (CSKEY_PGUP))
    c->GetTransform ().RotateThis (CS_VEC_TILT_UP, speed);
  if (kbd->GetKeyState (CSKEY_PGDN))
    c->GetTransform ().RotateThis (CS_VEC_TILT_DOWN, speed);
  if (kbd->GetKeyState (CSKEY_UP))
    c->Move (CS_VEC_FORWARD * 4 * speed);
  if (kbd->GetKeyState (CSKEY_DOWN))
    c->Move (CS_VEC_BACKWARD * 4 * speed);

  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
    return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();
}

void Simple::FinishFrame ()
{
  g3d->FinishDraw ();
  g3d->Print (NULL);
}

bool Simple::HandleEvent (iEvent& ev)
{
  if (ev.Type == csevBroadcast && ev.Command.Code == cscmdProcess)
  {
    simple->SetupFrame ();
    return true;
  }
  else if (ev.Type == csevBroadcast && ev.Command.Code == cscmdFinalProcess)
  {
    simple->FinishFrame ();
    return true;
  }
  else if (ev.Type == csevKeyDown && ev.Key.Code == CSKEY_ESC)
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
    if (q) q->GetEventOutlet()->Broadcast (cscmdQuit);
    return true;
  }

  return false;
}

bool Simple::SimpleEventHandler (iEvent& ev)
{
  return simple->HandleEvent (ev);
}

bool Simple::Initialize ()
{
  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_SOFTWARE3D,
	CS_REQUEST_ENGINE,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_LEVELLOADER,
	CS_REQUEST_REPORTER,
	CS_REQUEST_REPORTERLISTENER,
	CS_REQUEST_END))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simple2",
	"Can't initialize plugins!");
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, SimpleEventHandler))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simple2",
	"Can't initialize event handler!");
    return false;
  }

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    csCommandLineHelper::Help (object_reg);
    return false;
  }

  // The virtual clock.
  vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);
  if (vc == NULL)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simple2",
	"Can't find the virtual clock!");
    return false;
  }

  // Find the pointer to engine plugin
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (engine == NULL)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simple2",
	"No iEngine plugin!");
    return false;
  }

  loader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (loader == NULL)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simple2",
    	"No iLoader plugin!");
    return false;
  }

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (g3d == NULL)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simple2",
    	"No iGraphics3D plugin!");
    return false;
  }

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (kbd == NULL)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simple2",
    	"No iKeyboardDriver plugin!");
    return false;
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!csInitializer::OpenApplication (object_reg))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simple2",
    	"Error opening system!");
    return false;
  }

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simple2",
    	"Error loading 'stone4' texture!");
    return false;
  }
  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");

  room = engine->CreateSector ("room");
  csRef<iMeshWrapper> walls (engine->CreateSectorWallsMesh (room, "walls"));
  csRef<iThingState> walls_state (
  	SCF_QUERY_INTERFACE (walls->GetMeshObject (), iThingState));
  iPolygon3D* p;
  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (-10, 0, 10));
  p->CreateVertex (csVector3 (10, 0, 10));
  p->CreateVertex (csVector3 (10, 0, -10));
  p->CreateVertex (csVector3 (-10, 0, -10));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (-10, 20, -10));
  p->CreateVertex (csVector3 (10, 20, -10));
  p->CreateVertex (csVector3 (10, 20, 10));
  p->CreateVertex (csVector3 (-10, 20, 10));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (-10, 20, 10));
  p->CreateVertex (csVector3 (10, 20, 10));
  p->CreateVertex (csVector3 (10, 0,10));
  p->CreateVertex (csVector3 (-10, 0, 10));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (10, 20, 10));
  p->CreateVertex (csVector3 (10, 20, -10));
  p->CreateVertex (csVector3 (10, 0, -10));
  p->CreateVertex (csVector3 (10, 0, 10));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (-10, 20, -10));
  p->CreateVertex (csVector3 (-10, 20, 10));
  p->CreateVertex (csVector3 (-10, 0, 10));
  p->CreateVertex (csVector3 (-10, 0, -10));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (10, 20, -10));
  p->CreateVertex (csVector3 (-10, 20, -10));
  p->CreateVertex (csVector3 (-10, 0, -10));
  p->CreateVertex (csVector3 (10, 0, -10));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  csRef<iStatLight> light;
  iLightList* ll = room->GetLights ();

  light = engine->CreateLight (NULL, csVector3 (-3, 5, 0), 20,
  	csColor (1, 0, 0), false);
  ll->Add (light->QueryLight ());

  light = engine->CreateLight (NULL, csVector3 (3, 5,  0), 20,
  	csColor (0, 0, 1), false);
  ll->Add (light->QueryLight ());

  light = engine->CreateLight (NULL, csVector3 (0, 5, -3), 20,
  	csColor (0, 1, 0), false);
  ll->Add (light->QueryLight ());

  engine->Prepare ();

  view = csPtr<iView> (new csView (engine, g3d));
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 5, -3));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  iTextureManager* txtmgr = g3d->GetTextureManager ();
  txtmgr->SetPalette ();

  // Load a texture for our sprite.
  iTextureWrapper* txt = loader->LoadTexture ("spark",
  	"/lib/std/spark.png", CS_TEXTURE_3D, txtmgr, true);
  if (txt == NULL)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simple2",
    	"Error loading texture!");
    return false;
  }

  // Load a sprite template from disk.
  printf("bla1 \n");
  csRef<iMeshFactoryWrapper> imeshfact ( engine->CreateMeshFactory("crystalspace.mesh.object.cloth","StuffFactory") );

  
  if (imeshfact == NULL)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simple2",
    	"Error loading mesh object factory!");
    return false;
  }
  
  csRef<iMaterialWrapper> mat ( engine->GetMaterialList()->FindByName("stone") );

  if (mat == NULL)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simple2",
    	"Error loading material!!!");
    return false;
  }
  printf("bla2 \n");
  csRef<iClothFactoryState> spstate (SCF_QUERY_INTERFACE (imeshfact->GetMeshObjectFactory(), iClothFactoryState));
  
  spstate->SetMaterialWrapper( mat );
  printf("bla3 \n");
  spstate->GenerateFabric(25,25);
  printf("blu \n");

  // Create the sprite and add it to the engine.
  csRef<iMeshWrapper> sprite (engine->CreateMeshWrapper (
  	imeshfact, "MySprite", room,
	csVector3 (-2, 4, 0)));
  csMatrix3 m; m.Identity (); // m *= 5.;
  sprite->GetMovable ()->SetTransform (m);
  sprite->GetMovable ()->UpdateMove ();
  
  //spstate->SetMixMode( CS_FX_SETALPHA(0.9));
  
  //spstate->SetAction ("default");

  // The following two calls are not needed since CS_ZBUF_USE and
  // Object render priority are the default but they show how you
  // can do this.
  //sprite->SetZBufMode (CS_ZBUF_NONE);
  //sprite->SetRenderPriority (engine->GetRenderPriority ("alpha"));
  
  

  //sprite->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
  return true;
}

void Simple::Start ()
{
  csDefaultRunLoop (object_reg);
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);

  simple = new Simple (object_reg);
  if (simple->Initialize ())
    simple->Start ();
  delete simple;

  csInitializer::DestroyApplication (object_reg);
  return 0;
}

