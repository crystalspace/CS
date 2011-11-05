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
    ReportError ("Can't make genmesh factory!");
    return false;
  }
  factstate = scfQueryInterface<iGeneralFactoryState> (
      genmesh_fact->GetMeshObjectFactory ());
  if (!factstate)
  {
    ReportError ("Strange, genmesh_fact doesn't implement iGeneralFactoryState!");
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
    ReportError ("Can't make genmesh object!");
    return false;
  }
  csRef<iGeneralMeshState> state (
  	
  	scfQueryInterface<iGeneralMeshState> (genmesh->GetMeshObject ()));
  if (!state)
  {
    ReportError ("Strange, genmesh doesn't implement iGeneralMeshState!");
    return false;
  }

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
  factstate->Invalidate ();
}

struct TargetToUse
{
  csRenderTargetAttachment attachment;
  const char* format;
};
static const TargetToUse targetsToUse[] = {
  {rtaColor0, "rgb8"},
  {rtaColor0, "rgb16_f"},
  //{rtaDepth,  "d32"} // FIXME: Requires attachment support in iRenderManagerTargets
};

static const char* AttachmentToStr (csRenderTargetAttachment a)
{
  switch(a)
  {
  case rtaColor0: return "color0";
  case rtaDepth:  return "depth";
  default: return 0;
  }
}

void Simple::CreateTextures ()
{
  numAvailableformats = 0;
  for (size_t n = 0; n < sizeof(targetsToUse)/sizeof(targetsToUse[0]); n++)
  {
    if (!g3d->CanSetRenderTarget (targetsToUse[n].format,
      targetsToUse[n].attachment))
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	  "crystalspace.application.simplept",
	  "Format unsupported: %s(%s)", targetsToUse[n].format,
          AttachmentToStr (targetsToUse[n].attachment));
      continue;
    }
      
    csRef<iTextureHandle> texHandle = 
      g3d->GetTextureManager()->CreateTexture (256, 256, csimg2D, 
        targetsToUse[n].format, CS_TEXTURE_3D);
    if (!texHandle) continue;
    
    Target target;
    target.texh = texHandle;
    target.format = targetsToUse[n].format;
    target.attachment = targetsToUse[n].attachment;
    targetTextures.Push (target);
    
    availableFormatsStr.AppendFmt ("%s ", target.format);
    numAvailableformats++;
    
    if (!targetTex)
    {
      targetTex = texHandle;
      currentTargetStr.Format ("%s(%s)", AttachmentToStr (target.attachment),
        target.format);
    }
  }
}

void Simple::CycleTarget()
{
  csRef<iRenderManagerTargets> targets =
    scfQueryInterface<iRenderManagerTargets> (rm);
  if (!targets) return;
      
  targets->UnregisterRenderTarget (targetTex);
  
  currentTarget = (currentTarget + 1) % targetTextures.GetSize();
  const Target& target = targetTextures[currentTarget];
  currentTargetStr.Format ("%s(%s)", AttachmentToStr (target.attachment),
    target.format);
  
  targetTex = target.texh;
  targets->RegisterRenderTarget (targetTex, targetView);

  // Also need to set new texture handle on material
  targetMat->GetMaterial()->GetVariableAdd (svTexDiffuse)->SetValue (targetTex);
}

//-----------------------------------------------------------------------------

static const char appID[] = "CrystalSpace.SimplePT";

Simple::Simple () : currentTarget (0)
{
  SetApplicationName (appID);
}

Simple::~Simple ()
{
}

void Simple::Frame ()
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

  // Tell the camera to render into the frame buffer.
  rm->RenderView (view);

  g3d->BeginDraw(CSDRAW_2DGRAPHICS);
  int fontHeight = font->GetTextHeight();
  int y = g3d->GetDriver2D()->GetHeight() - fontHeight;
  int white = g3d->GetDriver2D()->FindRGB (255, 255, 255);
  if (numAvailableformats > 1)
  {
    g3d->GetDriver2D()->Write (font, 0, y, white, -1,
      csString().Format ("SPACE to cycle formats: %s",
      availableFormatsStr.GetData()));
  }
  y -= fontHeight;
  g3d->GetDriver2D()->Write (font, 0, y, white, -1,
    csString().Format ("current target: %s",
    currentTargetStr.GetData()));
  g3d->FinishDraw ();
}

bool Simple::OnKeyboard (iEvent& ev)
{
  if ((ev.Name == csevKeyboardDown(object_reg)) && 
    (csKeyEventHelper::GetCookedCode (&ev) == CSKEY_ESC))
  {
    csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (object_reg));
    if (q)
      q->GetEventOutlet()->Broadcast (csevQuit(object_reg));
    return true;
  }
  else if ((ev.Name == csevKeyboardUp(object_reg)) && 
    (csKeyEventHelper::GetCookedCode (&ev) == CSKEY_SPACE))
  {
    CycleTarget ();
    return true;
  }

  return false;
}

bool Simple::OnInitialize (int /*argc*/, char* /*argv*/ [])
{
  if (!csInitializer::RequestPlugins (GetObjectRegistry (),
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
    ReportError ("Can't initialize plugins!");
    return false;
  }

  // "Warm up" the event handler so it can interact with the world
  csBaseEventHandler::Initialize (GetObjectRegistry ());

  // Now we need to register the event handler for our application.
  // Crystal Space is fully event-driven. Everything (except for this
  // initialization) happens in an event.
  // Rather than simply handling all events, we subscribe to the
  // particular events we're interested in.
  csEventID events[] = {
    csevFrame (GetObjectRegistry ()),
    csevKeyboardEvent (GetObjectRegistry ()),
    CS_EVENTLIST_END
  };

  if (!RegisterQueue (GetObjectRegistry (), events))
    return ReportError ("Failed to set up event handler!");

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

  return true;
}

void Simple::OnExit ()
{
  // Shut down the event handlers we spawned earlier.
  printer.Invalidate ();
}

bool Simple::Application ()
{
  // Open the main system. This will open all the previously loaded plug-ins.
  // i.e. all windows will be opened.
  if (!OpenApplication (GetObjectRegistry ()))
    return ReportError ("Error opening system!");

  if (SetupModules ())
  {
    // This calls the default runloop. This will basically just keep
    // broadcasting process events to keep the game going.
    Run ();
  }

  return true;
}

bool Simple::SetupModules ()
{
  // The virtual clock.
  vc = csQueryRegistry<iVirtualClock> (object_reg);

  // Find the pointer to engine plugin
  engine = csQueryRegistry<iEngine> (object_reg);
  if (!engine)
  {
    ReportError ("No iEngine plugin!");
    return false;
  }

  rm = engine->GetRenderManager ();

  loader = csQueryRegistry<iLoader> (object_reg);
  if (!loader)
  {
    ReportError ("No iLoader plugin!");
    return false;
  }

  g3d = csQueryRegistry<iGraphics3D> (object_reg);
  if (!g3d)
  {
    ReportError ("No iGraphics3D pluginn");
    return false;
  }
  font = g3d->GetDriver2D()->GetFontServer()->LoadFont (CSFONT_LARGE, 10);

  kbd = csQueryRegistry<iKeyboardDriver> (object_reg);
  if (!kbd)
  {
    ReportError ("No iKeyboardDriver pluginn");
    return false;
  }

  // We need a View to the virtual world.
  view.AttachNew(new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  // We use the full window to draw the world.
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());
  
  // Grab string ID for "tex diffuse" SV name
  csRef<iShaderVarStringSet> svstrings =
    csQueryRegistryTagInterface<iShaderVarStringSet> (
      GetObjectRegistry (), "crystalspace.shader.variablenameset");
  svTexDiffuse = svstrings->Request ("tex diffuse");

  // Here we create our world.
  CreateRoom ();

  // Let the engine prepare the meshes and textures.
  engine->Prepare ();

  // Now calculate static lighting for our geometry.
  using namespace CS::Lighting;
  SimpleStaticLighter::ShineLights (room, engine, 4);

  // Now we need to position the camera in our world.
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, 0));

  // We use some other "helper" event handlers to handle 
  // pushing our work into the 3D engine and rendering it
  // to the screen.
  printer.AttachNew (new FramePrinter (GetObjectRegistry ()));
  
  return true;
}

bool Simple::CreateRoom ()
{
  // Create our world.
  ReportInfo ("Creating world!...");

  // Create the procedural texture and a material for it
  //ProcTexture = new csEngineProcTex ();
  // Find the pointer to VFS.
  csRef<iVFS> VFS (csQueryRegistry<iVFS> (object_reg));
  if (!VFS)
  {
    ReportError ("No iVFS plugin!");
    return false;
  }

  VFS->PushDir ();
  VFS->ChDir ("/lev/partsys/");
  bool Success = (loader->LoadMapFile ("world", false));
  VFS->PopDir ();

  targetMat = engine->CreateMaterial ("rendertarget", nullptr);
  CreateTextures ();
  targetMat->GetMaterial()->GetVariableAdd (svTexDiffuse)->SetValue (targetTex);
  {
    iSector *room = engine->GetSectors ()->FindByName ("room");
    targetView = csPtr<iView> (new csView (engine, g3d));
    targetView->GetCamera ()->SetViewportSize (256, 256);
    targetView->GetCamera ()->GetTransform ().SetOrigin (csVector3 (-0.5,0,0));
    targetView->GetCamera ()->SetSector (room);
    targetView->SetRectangle (0, 0, 256, 256);
    iPerspectiveCamera* pcam = targetView->GetPerspectiveCamera ();
    pcam->SetPerspectiveCenter (0.5f, 0.5f);
    pcam->SetFOVAngle (pcam->GetFOVAngle (), 1.0f);

    csRef<iRenderManagerTargets> targets =
      scfQueryInterface<iRenderManagerTargets> (rm);
    if (targets)
      targets->RegisterRenderTarget (targetTex, targetView);
  }
  
  // Load the texture from the standard library.  This is located in
  // CS/data/standard.zip and mounted as /lib/std using the Virtual
  // File System (VFS) plugin.
  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
    ReportError ("Error loading %s texture!",
		 CS::Quote::Single ("stone4"));
  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");
  
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
  CreateGenMesh (/*ProcMat*/targetMat);

  csRef<iLight> light;
  light = engine->CreateLight (0, csVector3 (0, 0, 0), 20,
  	csColor (1, 1, 1));
  room->GetLights ()->Add (light);

  ReportInfo ("Created.");

  return Success;
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  /* Runs the application.
   *
   * csApplicationRunner<> is a small wrapper to support "restartable"
   * applications (ie where CS needs to be completely shut down and loaded
   * again). Simple1 does not use that functionality itself, however, it
   * allows you to later use "Simple.Restart();" and it'll just work.
   */
  return csApplicationRunner<Simple>::Run (argc, argv);
}
