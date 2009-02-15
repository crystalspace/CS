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

void Simple::CreatePolygon (iGeneralFactoryState *th,
	int v1, int v2, int v3, int v4)
{
  th->AddTriangle (csTriangle (v1, v2, v3));
  th->AddTriangle (csTriangle (v1, v3, v4));
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
  factstate = scfQueryInterface<iGeneralFactoryState> (
      genmesh_fact->GetMeshObjectFactory ());
  if (!factstate)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simplept",
	"Strange, genmesh_fact doesn't implement iGeneralFactoryState!");
    return false;
  }

  genmesh_fact->GetMeshObjectFactory ()->SetMaterialWrapper (mat);
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
  	
  	scfQueryInterface<iGeneralMeshState> (genmesh->GetMeshObject ()));
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
  if (simple)
  {
    if (ev.Name == csevProcess(simple->object_reg))
    {
      simple->SetupFrame ();
      return true;
    }
    else if (ev.Name == csevFinalProcess(simple->object_reg))
    {
      simple->FinishFrame ();
      return true;
    }
    else
    {
      return simple->HandleEvent (ev);
    }
  }
  else
    return false;
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

  Process = csevProcess (object_reg);
  FinalProcess = csevFinalProcess (object_reg);
  KeyboardDown = csevKeyboardDown (object_reg);

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
  vc = csQueryRegistry<iVirtualClock> (object_reg);

  // Find the pointer to engine plugin
  engine = csQueryRegistry<iEngine> (object_reg);
  if (!engine)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simplept",
    	"No iEngine plugin!");
    return false;
  }

  loader = csQueryRegistry<iLoader> (object_reg);
  if (!loader)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simplept",
    	"No iLoader plugin!");
    return false;
  }

  g3d = csQueryRegistry<iGraphics3D> (object_reg);
  if (!g3d)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simplept",
    	"No iGraphics3D pluginn");
    return false;
  }

  kbd = csQueryRegistry<iKeyboardDriver> (object_reg);
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

  rm = csQueryRegistryOrLoad<iRenderManager> (object_reg,
    "crystalspace.rendermanager.test1");

  // Setup the texture manager
  iTextureManager* txtmgr = g3d->GetTextureManager ();

  csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
    	"crystalspace.application.simplept",
  	"Simple Procedural Texture Crystal Space Application version 0.1.");
  	
  font = g3d->GetDriver2D()->GetFontServer()->LoadFont (CSFONT_LARGE, 10);

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
  //ProcTexture = new csEngineProcTex ();
  // Find the pointer to VFS.
  csRef<iVFS> VFS (csQueryRegistry<iVFS> (object_reg));
  if (!VFS)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simplept",
    	"No iVFS plugin!");
    return false;
  }

  VFS->PushDir ();
  VFS->ChDir ("/lev/partsys/");
  bool Success = (loader->LoadMapFile ("world", false));
  VFS->PopDir ();

  {
    csRef<iTextureHandle> texHandle = 
      g3d->GetTextureManager()->CreateTexture (256, 256, csimg2D, "rgb8",
        CS_TEXTURE_3D);
    targetTexture = engine->GetTextureList()->NewTexture (texHandle);
  }
  csRef<iMaterialWrapper> targetMat = engine->CreateMaterial ("rendertarget", targetTexture);
  {
    iSector *room = engine->GetSectors ()->FindByName ("room");
    targetView = csPtr<iView> (new csView (engine, g3d));
    targetView->GetCamera ()->GetTransform ().SetOrigin (csVector3 (-0.5,0,0));
    targetView->GetCamera ()->SetSector (room);
    targetView->SetRectangle (0, 0, 256, 256);
    targetView->GetCamera ()->SetPerspectiveCenter (128, 128);
    targetView->GetCamera ()->SetFOVAngle (targetView->GetCamera ()->GetFOVAngle(), 256);

    rm->RegisterRenderTarget (targetTexture->GetTextureHandle(), targetView);
  }

  /*iMaterialWrapper* ProcMat = ProcTexture->Initialize (object_reg, engine,
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
  ProcTexture->DecRef ();*/
  room = engine->CreateSector ("proctex-room");
  csRef<iMeshWrapper> walls = CS::Geometry::GeneralMeshBuilder
    ::CreateFactoryAndMesh (engine, room, "walls", "walls_factory");
  iMeshFactoryWrapper* walls_factory = walls->GetFactory ();
  csRef<iGeneralFactoryState> walls_state = 
    scfQueryInterface<iGeneralFactoryState> (
	walls_factory->GetMeshObjectFactory ());
  walls->GetMeshObject ()->SetMaterialWrapper (tm);

  csColor4 black (0, 0, 0);
  walls_state->AddVertex (csVector3 (-8, -8, -5), csVector2 (0, 0),
      csVector3 (0), black);
  walls_state->AddVertex (csVector3 (-3, -3, +8), csVector2 (1, 0),
      csVector3 (0), black);
  walls_state->AddVertex (csVector3 (+3, -3, +8), csVector2 (1, 0),
      csVector3 (0), black);
  walls_state->AddVertex (csVector3 (+8, -8, -5), csVector2 (0, 0),
      csVector3 (0), black);
  walls_state->AddVertex (csVector3 (-8, +8, -5), csVector2 (0, 1),
      csVector3 (0), black);
  walls_state->AddVertex (csVector3 (-3, +3, +8), csVector2 (1, 1),
      csVector3 (0), black);
  walls_state->AddVertex (csVector3 (+3, +3, +8), csVector2 (1, 1),
      csVector3 (0), black);
  walls_state->AddVertex (csVector3 (+8, +8, -5), csVector2 (0, 1),
      csVector3 (0), black);

  CreatePolygon (walls_state, 0, 1, 2, 3);
  CreatePolygon (walls_state, 1, 0, 4, 5);
  CreatePolygon (walls_state, 3, 2, 6, 7);
  CreatePolygon (walls_state, 0, 3, 7, 4);
  CreatePolygon (walls_state, 7, 6, 5, 4);
  walls_state->CalculateNormals ();

  genmesh_resolution = 15;
  genmesh_scale.Set (6, 6, 0);
  CreateGenMesh (ProcMat);

  csRef<iLight> light;
  light = engine->CreateLight (0, csVector3 (0, 0, 0), 20,
  	csColor (1, 1, 1));
  room->GetLights ()->Add (light);

  engine->Prepare ();

  using namespace CS::Lighting;
  SimpleStaticLighter::ShineLights (room, engine, 4);

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
  if ((Event.Name == csevKeyboardDown(object_reg)) && 
    (csKeyEventHelper::GetCookedCode (&Event) == CSKEY_ESC))
  {
    csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (object_reg));
    if (q)
      q->GetEventOutlet()->Broadcast (csevQuit(object_reg));
    return true;
  }
  else if ((Event.Name == csevKeyboardUp(object_reg)) && 
    (csKeyEventHelper::GetCookedCode (&Event) == CSKEY_SPACE))
  {
    ProcTexture->CycleTarget ();
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

  // move the r2t camera
  csVector3 Position (-0.5, 0, 3 + sin (current_time / (10*1000.0))*3);
  targetView->GetCamera ()->Move (Position
    - targetView->GetCamera ()->GetTransform ().GetOrigin ());

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
  /*if (!g3d->BeginDraw (
      engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
      return;*/

  // Tell the camera to render into the frame buffer.
  rm->RenderView (view);
}

void Simple::FinishFrame ()
{
  g3d->FinishDraw ();
  
  g3d->BeginDraw(CSDRAW_2DGRAPHICS);
  int fontHeight = font->GetTextHeight();
  int y = g3d->GetDriver2D()->GetHeight() - fontHeight;
  int white = g3d->GetDriver2D()->FindRGB (255, 255, 255);
  g3d->GetDriver2D()->Write (font, 0, y, white, -1,
    csString().Format ("SPACE to cycle formats: %s",
    ProcTexture->GetAvailableFormats()));
  y -= fontHeight;
  g3d->GetDriver2D()->Write (font, 0, y, white, -1,
    csString().Format ("current target: %s",
    ProcTexture->GetCurrentTarget()));
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

  delete simple; simple = 0;

  csInitializer::DestroyApplication (object_reg);

  return 0;
}

