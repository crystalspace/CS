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
#include "phystut.h"
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

#include "imesh/ball.h"
#include "ivaria/dynamics.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// The global pointer to simple
Simple *simple;

Simple::Simple ()
{
  engine = NULL;
  loader = NULL;
  g3d = NULL;
  kbd = NULL;
  vc = NULL;
  view = NULL;
  dyn = NULL;
  dynSys = NULL;
}

Simple::~Simple ()
{
  if (dyn) {
    if (dynSys) {
      dyn->RemoveSystem (dynSys);
    }
    dyn->DecRef ();
  }
  if (boxFact) boxFact->DecRef ();
  if (ballFact) ballFact->DecRef ();
  if (vc) vc->DecRef ();
  if (engine) engine->DecRef ();
  if (loader) loader->DecRef ();
  if (g3d) g3d->DecRef ();
  if (kbd) kbd->DecRef ();
  if (view) view->DecRef ();
  csInitializer::DestroyApplication (object_reg);
}

void Simple::SetupFrame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();

  // Now rotate the camera according to keyboard state
  float speed = elapsed_time / 1000.0;

  // Limit the speed to some stable (?) values.
  if (speed <= 0)
   speed = SMALL_EPSILON;
  else if (speed > 10)
   speed = 10;

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
    c->Move (CS_VEC_FORWARD * 5 * speed);
  if (kbd->GetKeyState (CSKEY_DOWN))
    c->Move (CS_VEC_BACKWARD * 5 * speed);

  dyn->Step (speed);

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

  else if (ev.Type == csevKeyDown && ev.Key.Code == CSKEY_SPACE)
  {
    if (rand()%2) CreateBox (); else CreateSphere ();
    return true;
  }
  else if (ev.Type == csevKeyDown && ev.Key.Code == 'b')
  {
    CreateBox ();
    return true;
  }
  else if (ev.Type == csevKeyDown && ev.Key.Code == 's')
  {
    CreateSphere ();
    return true;
  }
  else if (ev.Type == csevKeyDown && ev.Key.Code == 'g')
  { // Toggle gravity.
    dynSys->SetGravity (dynSys->GetGravity () == 0 ?
     csVector3 (0, -5, 0) : csVector3 (0));
    return true;
  }
  else if (ev.Type == csevKeyDown && ev.Key.Code == CSKEY_ESC)
  {
    iEventQueue* q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
    if (q)
    {
      q->GetEventOutlet()->Broadcast (cscmdQuit);
      q->DecRef ();
    }
    return true;
  }

  return false;
}

bool Simple::SimpleEventHandler (iEvent& ev)
{
  return simple->HandleEvent (ev);
}

bool Simple::Initialize (int argc, const char* const argv[])
{
  object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return false;

  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_SOFTWARE3D,
	CS_REQUEST_ENGINE,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_LEVELLOADER,
	CS_REQUEST_REPORTER,
	CS_REQUEST_REPORTERLISTENER,
	CS_REQUEST_PLUGIN ("crystalspace.dynamics.ode", iDynamics),
	CS_REQUEST_END))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.phystut",
	"Can't initialize plugins!");
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, SimpleEventHandler))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.phystut",
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
  if (!vc)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.phystut",
	"Can't find the virtual clock!");
    return false;
  }

  // Find the pointer to engine plugin
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!engine)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.phystut",
	"No iEngine plugin!");
    return false;
  }

  loader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (!loader)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.phystut",
    	"No iLoader plugin!");
    return false;
  }

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!g3d)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.phystut",
    	"No iGraphics3D plugin!");
    return false;
  }

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (!kbd)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.phystut",
    	"No iKeyboardDriver plugin!");
    return false;
  }

  dyn = CS_QUERY_REGISTRY (object_reg, iDynamics);
  if (!dyn)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.phystut",
    	"No iDynamics plugin!");
    return false;
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!csInitializer::OpenApplication (object_reg))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.phystut",
    	"Error opening system!");
    return false;
  }

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.phystut",
    	"Error loading 'stone4' texture!");
    return false;
  }
  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");

  room = engine->CreateSector ("room");
  iMeshWrapper* walls = engine->CreateSectorWallsMesh (room, "walls");
  iThingState* walls_state = SCF_QUERY_INTERFACE (walls->GetMeshObject (),
  	iThingState);
  iPolygon3D* p;
  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (-5, -5, 5));
  p->CreateVertex (csVector3 (5, -5, 5));
  p->CreateVertex (csVector3 (5, -5, -5));
  p->CreateVertex (csVector3 (-5, -5, -5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (-5, 5, -5));
  p->CreateVertex (csVector3 (5, 5, -5));
  p->CreateVertex (csVector3 (5, 5, 5));
  p->CreateVertex (csVector3 (-5, 5, 5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (-5, 5, 5));
  p->CreateVertex (csVector3 (5, 5, 5));
  p->CreateVertex (csVector3 (5, -5, 5));
  p->CreateVertex (csVector3 (-5, -5, 5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (5, 5, 5));
  p->CreateVertex (csVector3 (5, 5, -5));
  p->CreateVertex (csVector3 (5, -5, -5));
  p->CreateVertex (csVector3 (5, -5, 5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (-5, 5, -5));
  p->CreateVertex (csVector3 (-5, 5, 5));
  p->CreateVertex (csVector3 (-5, -5, 5));
  p->CreateVertex (csVector3 (-5, -5, -5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (5, 5, -5));
  p->CreateVertex (csVector3 (-5, 5, -5));
  p->CreateVertex (csVector3 (-5, -5, -5));
  p->CreateVertex (csVector3 (5, -5, -5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  walls_state->DecRef ();
  walls->DecRef ();

  iStatLight* light;
  iLightList* ll = room->GetLights ();

  light = engine->CreateLight (NULL, csVector3 (-3, 0, 0), 8,
  	csColor (1, 0, 0), false);
  ll->Add (light->QueryLight ());
  light->DecRef ();

  light = engine->CreateLight (NULL, csVector3 (3, 0,  0), 8,
  	csColor (0, 0, 1), false);
  ll->Add (light->QueryLight ());
  light->DecRef ();

  light = engine->CreateLight (NULL, csVector3 (0, 0, 3), 8,
  	csColor (0, 1, 0), false);
  ll->Add (light->QueryLight ());
  light->DecRef ();

  light = engine->CreateLight (NULL, csVector3 (0, -3, 0), 8,
  	csColor (1, 1, 0), false);
  ll->Add (light->QueryLight ());
  light->DecRef ();

  engine->Prepare ();

  view = new csView (engine, g3d);
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, -4.9));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  iTextureManager* txtmgr = g3d->GetTextureManager ();
  txtmgr->SetPalette ();

  iTextureWrapper* txt = loader->LoadTexture ("spark",
  	"/lib/std/spark.png", CS_TEXTURE_3D, txtmgr, true);
  if (txt == NULL)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.phystut",
    	"Error loading texture!");
    return false;
  }

  // Load the box mesh factory.
  boxFact = loader->LoadMeshObjectFactory ("/lib/std/sprite1");
  if (!boxFact)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.phystut",
    	"Error loading mesh object factory!");
    return false;
  }

  // Double the size.
  csMatrix3 m; m *= .5;
  csReversibleTransform t = csReversibleTransform (m, csVector3 (0));
  boxFact->HardTransform (t);


  // Create the ball mesh factory.
  ballFact = engine->CreateMeshFactory("crystalspace.mesh.object.ball",
   "ballFact");
  if (!ballFact)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.phystut",
    	"Error creating mesh object factory!");
    return false;
  }


  // Create the dynamic system.
  dynSys = dyn->CreateSystem ();
  if (!dynSys)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.phystut",
    	"Error creating dynamic system!");
    return false;
  }

  dynSys->SetGravity (csVector3 (0,-9.8,0));

  CreateRoomSolids (csVector3 (0), csVector3 (5), 1);

  return true;
}

void Simple::CreateBox (void)
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the mesh.
  iMeshWrapper* mesh = engine->CreateMeshWrapper (boxFact, "box", room);
  mesh->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);

  // Create a body and attach the mesh.
  iRigidBody* rb = dynSys->CreateBody ();
  rb->SetProperties (1, csVector3 (0), csMatrix3 ());
  rb->SetPosition (tc.GetOrigin ());
  rb->AttachMesh (mesh);
  mesh->DecRef ();

  // Create and attach a box collider.
  const csMatrix3 tm;
  const csVector3 tv (0);
  csOrthoTransform t (tm, tv);
  csVector3 size (.5, 1, .5); // This should be the same size as the mesh.
  rb->AttachColliderBox (size, t, 10, 1, 2);

  // Fling the body.
  rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 5));
  rb->SetAngularVelocity (tc.GetT2O () * csVector3 (0, 0, 1));
}

void Simple::CreateSphere (void)
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();


  // Create the mesh.
  iMeshWrapper* mesh = engine->CreateMeshWrapper (ballFact, "ball", room);
  mesh->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);

  iMaterialWrapper* mat = engine->GetMaterialList ()->FindByName ("spark");

  // Set the ball mesh properties.
  iBallState *s = SCF_QUERY_INTERFACE (mesh->GetMeshObject (), iBallState);
  const float r (rand()%10/10.);
  const csVector3 radius (r, r, r); // This should be the same size as the mesh.
  s->SetRadius (radius.x, radius.y, radius.z);
  s->SetRimVertices (16);
  s->SetMaterialWrapper (mat);
  s->DecRef ();


  // Create a body and attach the mesh.
  iRigidBody* rb = dynSys->CreateBody ();
  rb->SetProperties (radius.Norm()/2, csVector3 (0), csMatrix3 ());
  rb->SetPosition (tc.GetOrigin ());
  rb->AttachMesh (mesh);
  mesh->DecRef ();

  // Create and attach a sphere collider.
  rb->AttachColliderSphere (radius.Norm()/2, csVector3 (0), 10, 1, 2);

  // Fling the body.
  rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 5));
  rb->SetAngularVelocity (tc.GetT2O () * csVector3 (0, 0, 1));
}

void Simple::CreateRoomSolids (const csVector3& center,
 const csVector3& radius, float thickness)
{
  // Create a body for the room.
  iRigidBody* rb = dynSys->CreateBody ();
  rb->SetPosition (center);
  rb->MakeStatic ();

  const csMatrix3 tm;
  const csVector3 tv (0);
  csOrthoTransform t (tm, tv);

  // Attach colliders with offsets.

  // down
  t.SetOrigin (center + csVector3 (0, -radius.y - thickness/2, 0));
  rb->AttachColliderBox (
   csVector3 (radius.x*2, thickness, radius.z*2), t, 50, 1000, 0);

  // up
  t.SetOrigin (center + csVector3 (0, radius.y + thickness/2, 0));
  rb->AttachColliderBox (
   csVector3 (radius.x*2, thickness, radius.z*2), t, 50, 1000, 0);

  // forward
  t.SetOrigin (center + csVector3 (0, 0, radius.z + thickness/2));
  rb->AttachColliderBox (
   csVector3 (radius.x*2, radius.y*2, thickness), t, 50, 1000, 0);

  // right
  t.SetOrigin (center + csVector3 (radius.x + thickness/2, 0, 0));
  rb->AttachColliderBox (
   csVector3 (thickness, radius.y*2, radius.z*2), t, 50, 1000, 0);

  // left
  t.SetOrigin (center + csVector3 (-radius.x - thickness/2, 0, 0));
  rb->AttachColliderBox (
   csVector3 (thickness, radius.y*2, radius.z*2), t, 50, 1000, 0);

  // back
  t.SetOrigin (center + csVector3 (0, 0, -radius.z - thickness/2));
  rb->AttachColliderBox (
   csVector3 (radius.x*2, radius.y*2, thickness), t, 50, 1000, 0);

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
  simple = new Simple ();

  if (simple->Initialize (argc, argv))
    simple->Start ();

  delete simple;
  return 0;
}
