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

#include "simplept.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

void Simple::CreatePolygon (iThingFactoryState *th,
	int v1, int v2, int v3, int v4, iMaterialWrapper *mat)
{
  th->AddPolygon (4, v1, v2, v3, v4);
  th->SetPolygonMaterial (CS_POLYRANGE_LAST, mat);
  th->SetPolygonTextureMapping (CS_POLYRANGE_LAST, 6);
}

static float frand (float range)
{
  float r = float ((rand () >> 2) % 1000);
  return r * range / 1000.0;
}

bool Simple::CreateGenMesh (iMaterialWrapper* mat)
{
  csRef<iMeshFactoryWrapper> genmesh_fact (
  	csPtr<iMeshFactoryWrapper> (engine->CreateMeshFactory (
  	"crystalspace.mesh.object.genmesh", "genmesh")));
  if (!genmesh_fact)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simplept",
	"Can't make genmesh factory!");
    return false;
  }
  factstate = SCF_QUERY_INTERFACE (genmesh_fact->GetMeshObjectFactory (),
  	iGeneralFactoryState);
  if (!factstate)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simplept",
	"Strange, genmesh_fact doesn't implement iGeneralFactoryState!");
    return false;
  }

  factstate->SetMaterialWrapper (mat);
  factstate->SetVertexCount ((genmesh_resolution+1)*(genmesh_resolution+1));
  factstate->SetTriangleCount (genmesh_resolution*genmesh_resolution*2);

  csVector3* verts = factstate->GetVertices ();
  csVector2* texels = factstate->GetTexels ();
  csTriangle* triangles = factstate->GetTriangles ();

  int x, y;
  int idx = 0;
  for (y = 0 ; y <= genmesh_resolution ; y++)
  {
    float dy = float (y) / float (genmesh_resolution);
    for (x = 0 ; x <= genmesh_resolution ; x++)
    {
      float dx = float (x) / float (genmesh_resolution);
      float z = 8;
      if (x > 0 && x < genmesh_resolution-1 &&
      	  y > 0 && y < genmesh_resolution-1)
        z += frand (genmesh_scale.z);
      verts[idx].Set (
      	dx * genmesh_scale.x - genmesh_scale.x/2,
	dy * genmesh_scale.y - genmesh_scale.y/2,
	z);
      texels[idx].Set (dx, 1.0f - dy);

      idx++;
    }
  }

  idx = 0;
  for (y = 0 ; y < genmesh_resolution ; y++)
    for (x = 0 ; x < genmesh_resolution ; x++)
    {
      int idxv = y*(genmesh_resolution+1)+x;
      triangles[idx].c = idxv;
      triangles[idx].b = idxv+1;
      triangles[idx].a = idxv+genmesh_resolution+1;
      idx++;
      triangles[idx].c = idxv+1;
      triangles[idx].b = idxv+genmesh_resolution+1+1;
      triangles[idx].a = idxv+genmesh_resolution+1;
      idx++;
    }

  factstate->CalculateNormals ();
  factstate->Invalidate ();

  genmesh = csPtr<iMeshWrapper> (engine->CreateMeshWrapper (genmesh_fact,
  	"genmesh", room, csVector3 (0, 0, 0)));
  if (!genmesh)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simplept",
	"Can't make genmesh object!");
    return false;
  }
  csRef<iGeneralMeshState> state (
  	SCF_QUERY_INTERFACE (genmesh->GetMeshObject (),
  	iGeneralMeshState));
  if (!state)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simplept",
	"Strange, genmesh doesn't implement iGeneralMeshState!");
    return false;
  }

  state->SetLighting (true);
  state->SetManualColors (false);
  genmesh->SetZBufMode (CS_ZBUF_FILL);
  genmesh->SetRenderPriority (engine->GetWallRenderPriority ());

  int tot_angle_table_size = (genmesh_resolution+1) * (genmesh_resolution+1);
  angle_table = new float [tot_angle_table_size];
  angle_speed = new float [tot_angle_table_size];
  start_verts = new csVector3 [tot_angle_table_size];
  int i;
  for (i = 0 ; i < tot_angle_table_size ; i++)
  {
    angle_table[i] = 0;
    angle_speed[i] = 2. * (frand (5) + 5) / 10.0;
    start_verts[i] = verts[i];
  }

  return true;
}

void Simple::AnimateGenMesh (csTicks elapsed)
{
  csVector3* verts = factstate->GetVertices ();
  int idx = 0;
  int x, y;
  for (y = 0 ; y <= genmesh_resolution ; y++)
  {
    for (x = 0 ; x <= genmesh_resolution ; x++)
    {
      if (x > 0 && x < genmesh_resolution-1 &&
      	  y > 0 && y < genmesh_resolution-1)
      {
	angle_table[idx] += float (elapsed) * angle_speed[idx] / 1000.0;
	verts[idx] = start_verts[idx];
	verts[idx].x += .1*cos (angle_table[idx]);
	verts[idx].y += .1*sin (angle_table[idx]);
      }
      idx++;
    }
  }
}

//-----------------------------------------------------------------------------

// The global system driver
Simple *simple;

Simple::Simple (iObjectRegistry* object_reg)
{
  Simple::object_reg = object_reg;
}

Simple::~Simple ()
{
}

static bool SimpleEventHandler (iEvent& ev)
{
  if (ev.Type == csevBroadcast && csCommandEventHelper::GetCode(&ev) == cscmdProcess)
  {
    simple->SetupFrame ();
    return true;
  }
  else if (ev.Type == csevBroadcast && csCommandEventHelper::GetCode(&ev) == cscmdFinalProcess)
  {
    simple->FinishFrame ();
    return true;
  }
  else
  {
    return simple ? simple->HandleEvent (ev) : false;
  }
}

bool Simple::Initialize ()
{
  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_OPENGL3D,
	CS_REQUEST_ENGINE,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_LEVELLOADER,
	CS_REQUEST_REPORTER,
	CS_REQUEST_REPORTERLISTENER,
	CS_REQUEST_END))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simplept",
	"Can't initialize plugins!");
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, SimpleEventHandler))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simplept",
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

  // Find the pointer to engine plugin
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!engine)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simplept",
    	"No iEngine plugin!");
    return false;
  }

  loader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (!loader)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simplept",
    	"No iLoader plugin!");
    return false;
  }

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!g3d)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simplept",
    	"No iGraphics3D pluginn");
    return false;
  }

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (!kbd)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simplept",
    	"No iKeyboardDriver pluginn");
    return false;
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!csInitializer::OpenApplication (object_reg))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simplept",
    	"Error opening system!");
    return false;
  }

  // Setup the texture manager
  iTextureManager* txtmgr = g3d->GetTextureManager ();

  csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
    	"crystalspace.application.simplept",
  	"Simple Procedural Texture Crystal Space Application version 0.1.");

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  // Create our world.
  csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
    	"crystalspace.application.simplept",
  	"Creating world!...");

  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simplept",
    	"Error loading 'stone4' texture!");
    return false;
  }
  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");
  // Create the procedural texture and a material for it
  ProcTexture = new csEngineProcTex ();
  // Find the pointer to VFS.
  csRef<iVFS> VFS (CS_QUERY_REGISTRY (object_reg, iVFS));
  if (!VFS)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simplept",
    	"No iVFS plugin!");
    return false;
  }

  iMaterialWrapper* ProcMat = ProcTexture->Initialize (object_reg, engine,
  	txtmgr, "procmat");
  if (!ProcMat)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simplept",
    	"Could not initialize procedural texture!");
    return false;
  }
  ProcMat->QueryObject ()->ObjAdd (ProcTexture);
  ProcTexture->LoadLevel ();
  ProcTexture->DecRef ();
  room = engine->CreateSector ("proctex-room");
  csRef<iMeshWrapper> walls (engine->CreateSectorWallsMesh (room, "walls"));
  csRef<iThingState> ws =
  	SCF_QUERY_INTERFACE (walls->GetMeshObject (), iThingState);
  csRef<iThingFactoryState> walls_state = ws->GetFactory ();

  walls_state->CreateVertex (csVector3 (-8, -8, -5));
  walls_state->CreateVertex (csVector3 (-3, -3, +8));
  walls_state->CreateVertex (csVector3 (+3, -3, +8));
  walls_state->CreateVertex (csVector3 (+8, -8, -5));
  walls_state->CreateVertex (csVector3 (-8, +8, -5));
  walls_state->CreateVertex (csVector3 (-3, +3, +8));
  walls_state->CreateVertex (csVector3 (+3, +3, +8));
  walls_state->CreateVertex (csVector3 (+8, +8, -5));

  CreatePolygon (walls_state, 0, 1, 2, 3, tm);
  CreatePolygon (walls_state, 1, 0, 4, 5, tm);
  genmesh_resolution = 15;
  genmesh_scale.Set (6, 6, 0);
  CreateGenMesh (ProcMat);
  CreatePolygon (walls_state, 3, 2, 6, 7, tm);
  CreatePolygon (walls_state, 0, 3, 7, 4, tm);
  CreatePolygon (walls_state, 7, 6, 5, 4, tm);

  csRef<iLight> light;
  light = engine->CreateLight (0, csVector3 (0, 0, 0), 20,
  	csColor (1, 1, 1));
  room->GetLights ()->Add (light);

  engine->Prepare ();
  csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
    	"crystalspace.application.simplept",
  	"Created.");

  view = csPtr<iView> (new csView (engine, g3d));
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, 0));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  return true;
}

bool Simple::HandleEvent (iEvent& Event)
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

void Simple::SetupFrame ()
{
  // First get elapsed time from the system driver.
  csTicks elapsed_time, current_time;
  elapsed_time = vc->GetElapsedTicks ();
  current_time = vc->GetCurrentTicks ();

  AnimateGenMesh (elapsed_time);

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
  if (!g3d->BeginDraw (
      engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
      return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();
}

void Simple::FinishFrame ()
{
  g3d->FinishDraw ();
  g3d->Print (0);
}

void Simple::Start ()
{
  csDefaultRunLoop(object_reg);
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  srand (time (0));
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return -1;

  // Create our main class.
  simple = new Simple (object_reg);

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
  if (simple->Initialize ())
    simple->Start ();

  delete simple;

  csInitializer::DestroyApplication (object_reg);

  return 0;
}

