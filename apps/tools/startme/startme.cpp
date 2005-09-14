/*
    Copyright (C) 2005 by Jorrit Tyberghein

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

#include "startme.h"

CS_IMPLEMENT_APPLICATION

//---------------------------------------------------------------------------

StartMe::StartMe ()
{
  SetApplicationName ("CrystalSpace.StartMe");
  last_selected = -1;
  description_selected = -1;
}

StartMe::~StartMe ()
{
}

void StartMe::ProcessFrame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();
  if (elapsed_time > 200) elapsed_time = 200;
  // Now rotate the camera according to keyboard state
  float elapsed_seconds = float (elapsed_time) / 1000.0f;

  int mouse_x = mouse->GetLastX ();
  int mouse_y = mouse->GetLastY ();
  iCamera* camera = view->GetCamera ();
  csVector2 p (mouse_x, camera->GetShiftY() * 2 - mouse_y);

  csVector3 light_v, star_v;

  camera->InvPerspective (p, DEMO_MESH_Z-5, star_v);
  star_ticks += elapsed_time;
  while (star_ticks > STAR_NEWSTAR_TIMEOUT)
  {
    star_ticks -= STAR_NEWSTAR_TIMEOUT;
    csVector3 sv = star_v;
    star_v.x += (float (float (rand ()) / float (RAND_MAX)) - .5f) / 4.0f;
    star_v.y += (float (float (rand ()) / float (RAND_MAX)) - .5f) / 4.0f;
    star_v.z += (float (float (rand ()) / float (RAND_MAX)) - .5f) / 4.0f;
    int max = STAR_COUNT;
    while (stars[cur_star].inqueue)
    {
      cur_star++;
      if (cur_star >= STAR_COUNT) cur_star = 0;
      max--;
      if (max <= 0) break;
    }
    if (max <= 0) { printf ("MAX!\n"); fflush (stdout); break; }
    StarInfo& si = stars[cur_star];
    si.star->GetMovable ()->GetTransform ().SetOrigin (star_v);
    si.star->GetMovable ()->UpdateMove ();
    si.r = 0;
    si.stars_state->SetColor (csColor (0, 0, 0));
    si.star->GetFlags ().Reset (CS_ENTITY_INVISIBLE);
    si.inqueue = true;
    star_queue.Push (cur_star);
    cur_star++;
    if (cur_star >= STAR_COUNT) cur_star = 0;
  }

  float dr = elapsed_seconds / STAR_MAXAGE;
  size_t j = star_queue.Length ();
  while (j > 0)
  {
    j--;
    int star_idx = star_queue[j];
    StarInfo& si = stars[star_idx];
    si.r += dr;
    if (si.r >= 1)
    {
      si.star->GetFlags ().Set (CS_ENTITY_INVISIBLE);
      si.inqueue = false;
      star_queue.DeleteIndex (j);
    }
    else
    {
      float f = 1.0f;
      if (si.r < STAR_FADE_1)
      {
        f = 1.0f - (STAR_FADE_1-si.r) / STAR_FADE_1;
      }
      else if (si.r >= STAR_FADE_2)
      {
        f = 1.0f - (si.r - STAR_FADE_2) / (1.0f - STAR_FADE_2);
      }
      si.stars_state->SetColor (csColor (f+.2, f, f));
    }
  }

  camera->InvPerspective (p, DEMO_MESH_Z-3, light_v);
  pointer_light->SetCenter (light_v);
  pointer_light->Setup ();
  pointer_light->Setup ();

  csVector3 start_v, end_v;
  camera->InvPerspective (p, DEMO_MESH_Z-4, start_v);
  camera->InvPerspective (p, 100.0f, end_v);
  csVector3 start = camera->GetTransform ().This2Other (start_v);
  csVector3 end = camera->GetTransform ().This2Other (end_v);

  iSector* sector = camera->GetSector ();
  csVector3 isect;
  csIntersectingTriangle closest_tri;
  iMeshWrapper* sel_mesh;
  float sqdist = 1.0f;

  if (InDescriptionMode ())
    sel_mesh = meshes[description_selected];
  else
    sqdist = csColliderHelper::TraceBeam (cdsys, sector,
	start, end, true,
	closest_tri, isect, &sel_mesh);

  int i, sel = -1;
  if (sqdist >= 0 && sel_mesh)
  {
    const char* name = sel_mesh->QueryObject ()->GetName ();
    for (i = 0 ; i < DEMO_COUNT ; i++)
      if (!strcmp (demos[i].name, name))
      {
        spinning_speed[i] += elapsed_seconds / 80.0f;
	if (spinning_speed[i] > 0.05f) spinning_speed[i] = 0.05f;
	sel = i;
        break;
      }
  }
  last_selected = sel;

  for (i = 0 ; i < DEMO_COUNT ; i++)
  {
    if (sel != i)
    {
      if (spinning_speed[i] > 0)
      {
        spinning_speed[i] -= elapsed_seconds / 80.0f;
	if (spinning_speed[i] < 0)
	  spinning_speed[i] = 0;
      }
    }
    csYRotMatrix3 rot (spinning_speed[i]);
    meshes[i]->GetMovable ()->Transform (rot);
    meshes[i]->GetMovable ()->UpdateMove ();
  }

  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS |
  	CSDRAW_CLEARSCREEN | CSDRAW_CLEARZBUFFER))
    return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();

  if (InDescriptionMode ())
  {
    g3d->BeginDraw (CSDRAW_2DGRAPHICS);
    iGraphics2D* g2d = g3d->GetDriver2D ();
    csString desc = demos[description_selected].description;
    size_t idx = desc.FindFirst ('#');
    int y = 50;
    int fw, fh;
    font->GetMaxSize (fw, fh);
    while (idx != (size_t)-1)
    {
      csString start, remainder;
      desc.SubString (start, 0, idx);
      desc.SubString (remainder, idx+1);
      g2d->Write (font, 30, y, font_fg, font_bg, start);
      y += fh + 5;
      desc = remainder;
      idx = desc.FindFirst ('#');
    }
    g2d->Write (font, 30, y, font_fg, font_bg, desc);
  }
}

void StartMe::FinishFrame ()
{
  // Just tell the 3D renderer that everything has been rendered.
  g3d->FinishDraw ();
  g3d->Print (0);
}

void StartMe::EnterDescriptionMode ()
{
  description_selected = last_selected;
  main_light->SetColor (MAIN_LIGHT_OFF);
  pointer_light->SetColor (POINTER_LIGHT_OFF);
}

void StartMe::LeaveDescriptionMode ()
{
  description_selected = -1;
  main_light->SetColor (MAIN_LIGHT_ON);
  pointer_light->SetColor (POINTER_LIGHT_ON);
}

bool StartMe::OnKeyboard(iEvent& ev)
{
  // We got a keyboard event.
  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    // The user pressed a key (as opposed to releasing it).
    utf32_char code = csKeyEventHelper::GetCookedCode(&ev);
    if (code == CSKEY_ESC)
    {
      if (InDescriptionMode ())
        LeaveDescriptionMode ();
      else
      {
        // The user pressed escape to exit the application.
        // The proper way to quit a Crystal Space application
        // is by broadcasting a cscmdQuit event. That will cause the
        // main runloop to stop. To do that we get the event queue from
        // the object registry and then post the event.
        csRef<iEventQueue> q = 
          CS_QUERY_REGISTRY(GetObjectRegistry(), iEventQueue);
        if (q.IsValid()) q->GetEventOutlet()->Broadcast(cscmdQuit);
      }
    }
  }
  return false;
}

bool StartMe::OnMouseDown (iEvent &event)
{
  if (InDescriptionMode ())
  {
    system (demos[description_selected].command);
    LeaveDescriptionMode ();
    return true;
  }

  if (last_selected != -1)
  {
    EnterDescriptionMode ();
  }
  return true;
}

bool StartMe::LoadTextures ()
{
  if (!loader->LoadTexture ("spark", "/lib/std/spark.png"))
    return ReportError ("Error loading '%s' texture!", "spark");

  vfs->ChDir ("/lib/startme");
  int i;
  for (i = 0 ; i < DEMO_COUNT ; i++)
  {
    if (!loader->LoadTexture (demos[i].name, demos[i].image))
      return ReportError ("Error loading '%s' texture!", demos[i].image);
  }

  return true;
}

bool StartMe::OnInitialize(int argc, char* argv[])
{
  if (!csInitializer::SetupConfigManager (GetObjectRegistry (),
  	"/config/startme.cfg"))
    return ReportError ("Error reading config file 'startme.cfg'!");

  // RequestPlugins() will load all plugins we specify. In addition
  // it will also check if there are plugins that need to be loaded
  // from the config system (both the application config and CS or
  // global configs). In addition it also supports specifying plugins
  // on the commandline.
  if (!csInitializer::RequestPlugins(GetObjectRegistry(),
      CS_REQUEST_VFS,
      CS_REQUEST_OPENGL3D,
      CS_REQUEST_ENGINE,
      CS_REQUEST_FONTSERVER,
      CS_REQUEST_IMAGELOADER,
      CS_REQUEST_LEVELLOADER,
      CS_REQUEST_REPORTER,
      CS_REQUEST_REPORTERLISTENER,
      CS_REQUEST_PLUGIN("crystalspace.collisiondetection.opcode",
		iCollideSystem),
      CS_REQUEST_END))
    return ReportError ("Failed to initialize plugins!");

  // Now we need to setup an event handler for our application.
  // Crystal Space is fully event-driven. Everything (except for this
  // initialization) happens in an event.
  if (!RegisterQueue (GetObjectRegistry()))
    return ReportError ("Failed to set up event handler!");

  return true;
}

void StartMe::OnExit()
{
}

bool StartMe::Application()
{
  // Open the main system. This will open all the previously loaded plug-ins.
  // i.e. all windows will be opened.
  if (!OpenApplication(GetObjectRegistry()))
    return ReportError("Error opening system!");

  // Now get the pointer to various modules we need. We fetch them
  // from the object registry. The RequestPlugins() call we did earlier
  // registered all loaded plugins with the object registry.
  g3d = CS_QUERY_REGISTRY(GetObjectRegistry(), iGraphics3D);
  if (!g3d) return ReportError("Failed to locate 3D renderer!");

  engine = CS_QUERY_REGISTRY(GetObjectRegistry(), iEngine);
  if (!engine) return ReportError("Failed to locate 3D engine!");

  vc = CS_QUERY_REGISTRY(GetObjectRegistry(), iVirtualClock);
  if (!vc) return ReportError("Failed to locate Virtual Clock!");

  kbd = CS_QUERY_REGISTRY(GetObjectRegistry(), iKeyboardDriver);
  if (!kbd) return ReportError("Failed to locate Keyboard Driver!");

  mouse = CS_QUERY_REGISTRY(GetObjectRegistry(), iMouseDriver);
  if (!mouse) return ReportError("Failed to locate Mouse Driver!");

  cdsys = CS_QUERY_REGISTRY(GetObjectRegistry(), iCollideSystem);
  if (!cdsys) return ReportError("Failed to locate CollDet System!");

  loader = CS_QUERY_REGISTRY(GetObjectRegistry(), iLoader);
  if (!loader) return ReportError("Failed to locate Loader!");

  vfs = CS_QUERY_REGISTRY(GetObjectRegistry(), iVFS);
  if (!vfs) return ReportError("Failed to locate VFS!");

  // We need a View to the virtual world.
  view.AttachNew(new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  // We use the full window to draw the world.
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  // Load textures.
  if (!LoadTextures ())
    return false;

  // Here we create our world.
  CreateRoom ();

  // Let the engine prepare all lightmaps for use and also free all images 
  // that were loaded for the texture manager.
  engine->Prepare ();

  // Now we need to position the camera in our world.
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, 0));

  // Get our font.
  font = g2d->GetFontServer ()->LoadFont (CSFONT_LARGE);
  font_fg = g2d->FindRGB (255, 255, 255);
  font_bg = -1;

  // This calls the default runloop. This will basically just keep
  // broadcasting process events to keep the game going.
  Run();

  return true;
}

csPtr<iMeshWrapper> StartMe::CreateDemoMesh (const char* name,
	const csVector3& pos)
{
  csRef<iMeshWrapper> m;
  m = engine->CreateMeshWrapper (box_fact, name, room, pos);
  m->SetRenderPriority (engine->GetWallRenderPriority ());
  iMaterialWrapper* mat = engine->FindMaterial (name);
  m->GetMeshObject ()->SetMaterialWrapper (mat);
  return (csPtr<iMeshWrapper>)m;
}

void StartMe::CreateRoom ()
{
  // We create a new sector called "room".
  room = engine->CreateSector ("room");

  iMaterialWrapper* spark_mat = engine->FindMaterial ("spark");
  csRef<iMeshFactoryWrapper> spark_fact = engine->CreateMeshFactory (
  	"crystalspace.mesh.object.genmesh", "spark_fact");
  csRef<iGeneralFactoryState> spark_state = SCF_QUERY_INTERFACE (
  	spark_fact->GetMeshObjectFactory (), iGeneralFactoryState);
  spark_state->SetVertexCount (4);
  spark_state->GetVertices ()[0].Set (-.1, -.1, 0);
  spark_state->GetVertices ()[1].Set (.1, -.1, 0);
  spark_state->GetVertices ()[2].Set (.1, .1, 0);
  spark_state->GetVertices ()[3].Set (-.1, .1, 0);
  spark_state->GetTexels ()[0].Set (0, 0);
  spark_state->GetTexels ()[1].Set (1, 0);
  spark_state->GetTexels ()[2].Set (1, 1);
  spark_state->GetTexels ()[3].Set (0, 1);
  spark_state->GetNormals ()[0].Set (0, 0, 1);
  spark_state->GetNormals ()[1].Set (0, 0, 1);
  spark_state->GetNormals ()[2].Set (0, 0, 1);
  spark_state->GetNormals ()[3].Set (0, 0, 1);
  spark_state->SetTriangleCount (2);
  spark_state->GetTriangles ()[0].Set (2, 1, 0);
  spark_state->GetTriangles ()[1].Set (3, 2, 0);
  spark_state->SetLighting (false);
  spark_state->SetMixMode (CS_FX_ADD);
  spark_state->SetColor (csColor (1, 1, 1));
  spark_state->SetMaterialWrapper (spark_mat);
  int i;
  for (i = 0 ; i < STAR_COUNT ; i++)
  {
    stars[i].star = engine->CreateMeshWrapper (spark_fact, "star", room);
    stars[i].star->GetFlags ().Set (CS_ENTITY_INVISIBLE);
    stars[i].star->SetRenderPriority (engine->GetObjectRenderPriority ());
    stars[i].star->SetZBufMode (CS_ZBUF_NONE);
    stars[i].stars_state = SCF_QUERY_INTERFACE (stars[i].star->GetMeshObject (),
    	iGeneralMeshState);
  }
  cur_star = 0;
  star_ticks = 0;

  box_fact = engine->CreateMeshFactory (
  	"crystalspace.mesh.object.genmesh", "box_fact");
  csRef<iGeneralFactoryState> box_state = SCF_QUERY_INTERFACE (
  	box_fact->GetMeshObjectFactory (), iGeneralFactoryState);
  csBox3 b (-1, -1, -1, 1, 1, 1);
  box_state->GenerateBox (b);
  box_state->CalculateNormals ();

  int cols = 4;
  int rows = (DEMO_COUNT-1) / cols + 1;
  float dx = (DEMO_MESH_MAXX-DEMO_MESH_MINX) / float (cols-1);
  float dy = (DEMO_MESH_MAXY-DEMO_MESH_MINY) / float (rows-1);
  int x = 0, y = 0;
  for (i = 0 ; i < DEMO_COUNT ; i++)
  {
    meshes[i] = CreateDemoMesh (demos[i].name,
      	csVector3 (DEMO_MESH_MINX + dx * float (x),
		   DEMO_MESH_MINY + dy * float (y),
		   DEMO_MESH_Z));
    x++;
    if (x >= cols) { y++; x = 0; }
    spinning_speed[i] = 0;
  }

  // Now we need light to see something.
  iLightList* ll = room->GetLights ();

  main_light = engine->CreateLight(0, csVector3(0, 0, -5), 100,
  	MAIN_LIGHT_ON, CS_LIGHT_DYNAMICTYPE_DYNAMIC);
  ll->Add (main_light);

  pointer_light = engine->CreateLight(0, csVector3(0, 0, DEMO_MESH_Z-3), 5,
  	POINTER_LIGHT_ON, CS_LIGHT_DYNAMICTYPE_DYNAMIC);
  ll->Add (pointer_light);

  csColliderHelper::InitializeCollisionWrappers (cdsys, engine, 0);
}

/*-------------------------------------------------------------------------*
 * Main function
 *-------------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  /* Runs the application. 
   *
   * csApplicationRunner<> is a small wrapper to support "restartable" 
   * applications (ie where CS needs to be completely shut down and loaded 
   * again). StartMe1 does not use that functionality itself, however, it
   * allows you to later use "StartMe.Restart();" and it'll just work.
   */
  return csApplicationRunner<StartMe>::Run (argc, argv);
}
