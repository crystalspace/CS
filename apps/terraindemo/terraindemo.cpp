/*
    Copyright (C) 2006 by Kapoulkine Arseny

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

#include "terraindemo.h"

#include "plugins/engine/3d/rview.h"
#include "plugins/engine/3d/rview.cpp"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

TerrainDemo::TerrainDemo ()
{
  SetApplicationName ("CrystalSpace.TerrainDemo");
  
  rotX = -20 * 3.1415926f/180;
  rotY = 45 * 3.1415926f/180;
}

TerrainDemo::~TerrainDemo ()
{
}

bool TerrainDemo::Setup ()
{
  // Now get the pointer to various modules we need. We fetch them
  // from the object registry. The RequestPlugins() call we did earlier
  // registered all loaded plugins with the object registry.
  // The virtual clock.
  g3d = CS_QUERY_REGISTRY (GetObjectRegistry(), iGraphics3D);
  if (!g3d) return ReportError ("Failed to locate 3D renderer!");

  engine = CS_QUERY_REGISTRY (GetObjectRegistry(), iEngine);
  if (!engine) return ReportError ("Failed to locate 3D engine!");

  vc = CS_QUERY_REGISTRY (GetObjectRegistry(), iVirtualClock);
  if (!vc) return ReportError ("Failed to locate Virtual Clock!");

  kbd = CS_QUERY_REGISTRY (GetObjectRegistry(), iKeyboardDriver);
  if (!kbd) return ReportError ("Failed to locate Keyboard Driver!");

  loader = CS_QUERY_REGISTRY (GetObjectRegistry(), iLoader);
  if (!loader) return ReportError ("Failed to locate Loader!");

  cdsys = CS_QUERY_REGISTRY (GetObjectRegistry(), iCollideSystem);
  if (!cdsys) return ReportError ("Failed to locate CD system!");

  // We need a View to the virtual world.
  view.AttachNew(new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  // We use the full window to draw the world.
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  // Here we load our world from a map file.
  if (!LoadMap ()) return false;

  // Initialize collision objects for all loaded objects.
  csColliderHelper::InitializeCollisionWrappers (cdsys, engine);

  // Let the engine prepare all lightmaps for use and also free all images 
  // that were loaded for the texture manager.
  engine->Prepare ();

  // Find the starting position in this level.
  csVector3 pos (0);
  /*if (engine->GetCameraPositions ()->GetCount () > 0)
  {
    // There is a valid starting position defined in the level file.
    iCameraPosition* campos = engine->GetCameraPositions ()->Get (0);
    room = engine->GetSectors ()->FindByName (campos->GetSector ());
    pos = campos->GetPosition ();
  }
  else
  {
    // We didn't find a valid starting position. So we default
    // to going to room called 'room' at position (0,0,0).
    room = engine->GetSectors ()->FindByName ("room");
    pos = csVector3 (0, 0, 0);
  }
  if (!room)
    ReportError("Can't find a valid starting position!");*/
    
  room = engine->CreateSector("terrain");
    
  // Attach terrain
  csRef<iMeshObject> terrain_mesh_object = scfQueryInterface<iMeshObject>(terrain);
  csRef<iMeshWrapper> terrain_mesh_wrapper = engine->CreateMeshWrapper("terrain");
  terrain_mesh_wrapper->SetMeshObject(terrain_mesh_object);
  
  terrain_mesh_wrapper->GetMovable()->SetSector(room);
  
  csRef<iLight> light = engine->CreateLight ("DynLight", csVector3 (23, 1, 5),
  10, csColor (1, 0, 0), CS_LIGHT_DYNAMICTYPE_DYNAMIC);
  room->GetLights ()->Add(light);
  
  pos = csVector3(0, 74, 0);

  // Now we need to position the camera in our world.
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (pos);
  
  csPlane3 fp(0, 0, -1, 500);
  view->GetCamera ()->SetFarPlane(&fp);
  
  engine->SetClearScreen(true);
  engine->SetClearZBuf(true);
  
  engine->SetCurrentDefaultRenderloop(rloop);
  
  return true;
}
void TerrainDemo::ProcessFrame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();

  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.06 * 40);

  iCamera* c = view->GetCamera();

  if (kbd->GetKeyState (CSKEY_SHIFT))
  {
    // If the user is holding down shift, the arrow keys will cause
    // the camera to strafe up, down, left or right from it's
    // current position.
    if (kbd->GetKeyState (CSKEY_RIGHT))
      c->Move (CS_VEC_RIGHT * 4000 * speed);
    if (kbd->GetKeyState (CSKEY_LEFT))
      c->Move (CS_VEC_LEFT * 40 * speed);
    if (kbd->GetKeyState (CSKEY_UP))
      c->Move (CS_VEC_UP * 40 * speed);
    if (kbd->GetKeyState (CSKEY_DOWN))
      c->Move (CS_VEC_DOWN * 40 * speed);
  }
  else
  {
    // left and right cause the camera to rotate on the global Y
    // axis; page up and page down cause the camera to rotate on the
    // _camera's_ X axis (more on this in a second) and up and down
    // arrows cause the camera to go forwards and backwards.
    if (kbd->GetKeyState (CSKEY_RIGHT))
      rotY += speed;
    if (kbd->GetKeyState (CSKEY_LEFT))
      rotY -= speed;
    if (kbd->GetKeyState (CSKEY_PGUP))
      rotX += speed;
    if (kbd->GetKeyState (CSKEY_PGDN))
      rotX -= speed;
    if (kbd->GetKeyState (CSKEY_UP))
      c->Move (CS_VEC_FORWARD * 400 * speed);
    if (kbd->GetKeyState (CSKEY_DOWN))
      c->Move (CS_VEC_BACKWARD * 400 * speed);
  }

  // We now assign a new rotation transformation to the camera.  You
  // can think of the rotation this way: starting from the zero
  // position, you first rotate "rotY" radians on your Y axis to get
  // the first rotation.  From there you rotate "rotX" radians on the
  // your X axis to get the final rotation.  We multiply the
  // individual rotations on each axis together to get a single
  // rotation matrix.  The rotations are applied in right to left
  // order .
  csMatrix3 rot = csXRotMatrix3 (rotX) * csYRotMatrix3 (rotY);
  csOrthoTransform ot (rot, c->GetTransform().GetOrigin ());
  c->SetTransform (ot);

  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
    return;
    
  if (kbd->GetKeyState(CSKEY_SPACE))
    g3d->SetRenderState (G3DRENDERSTATE_EDGES, true);
  else
    g3d->SetRenderState (G3DRENDERSTATE_EDGES, false);

    
  // Tell the camera to render into the frame buffer.
  view->Draw ();
}

void TerrainDemo::FinishFrame ()
{
  // Just tell the 3D renderer that everything has been rendered.
  g3d->FinishDraw ();
  g3d->Print (0);
}

bool TerrainDemo::OnKeyboard(iEvent& ev)
{
  // We got a keyboard event.
  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    // The user pressed a key (as opposed to releasing it).
    utf32_char code = csKeyEventHelper::GetCookedCode(&ev);
    if (code == CSKEY_ESC)
    {
      // The user pressed escape to exit the application.
      // The proper way to quit a Crystal Space application
      // is by broadcasting a csevQuit event. That will cause the
      // main runloop to stop. To do that we get the event queue from
      // the object registry and then post the event.
      csRef<iEventQueue> q = 
        CS_QUERY_REGISTRY(GetObjectRegistry(), iEventQueue);
      if (q.IsValid()) q->GetEventOutlet()->Broadcast(csevQuit(GetObjectRegistry()));
    }
  }
  return false;
}

bool TerrainDemo::OnInitialize(int /*argc*/, char* /*argv*/ [])
{
  // RequestPlugins() will load all plugins we specify. In addition
  // it will also check if there are plugins that need to be loaded
  // from the config system (both the application config and CS or
  // global configs). In addition it also supports specifying plugins
  // on the commandline.
  iObjectRegistry *object_reg;
  if (!csInitializer::RequestPlugins(object_reg = GetObjectRegistry(),
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
    CS_REQUEST_PLUGIN("crystalspace.graphics3d.shadermanager",
                iShaderManager),
    CS_REQUEST_END))
    return ReportError("Failed to initialize plugins!");

  csBaseEventHandler::Initialize(GetObjectRegistry());

  // Now we need to setup an event handler for our application.
  // Crystal Space is fully event-driven. Everything (except for this
  // initialization) happens in an event.
  if (!RegisterQueue(GetObjectRegistry(), csevAllEvents(GetObjectRegistry())))
    return ReportError("Failed to set up event handler!");
    
  // Let's create a terrain.
  csRef<iTerrainRenderer> t_renderer = csLoadPlugin<iTerrainRenderer>(
  GetObjectRegistry(),
  "crystalspace.mesh.object.terrainimproved.simplerenderer");
  
  csRef<iTerrainCollider> t_collider = csLoadPlugin<iTerrainCollider>(
  GetObjectRegistry(),
  "crystalspace.mesh.object.terrainimproved.simplecollider");
  
  csRef<iTerrainDataFeeder> t_feeder = csLoadPlugin<iTerrainDataFeeder>(
  GetObjectRegistry(),
  "crystalspace.mesh.object.terrainimproved.simpledatafeeder");

  t_feeder->SetParam("heightmap source", "/lev/terraini/heightmap.png");
  t_feeder->SetParam("materialmap source", "/lev/terraini/material.png");

  csRef<iTerrainDataFeeder> t_feeder_00 = csLoadPlugin<iTerrainDataFeeder>(
  GetObjectRegistry(),
  "crystalspace.mesh.object.terrainimproved.simpledatafeeder");

  t_feeder_00->SetParam("heightmap source", "/lev/terraini/heightmap_00.png");
  t_feeder_00->SetParam("materialmap source", "/lev/terraini/material_00.png");

  csRef<iTerrainDataFeeder> t_feeder_01 = csLoadPlugin<iTerrainDataFeeder>(
  GetObjectRegistry(),
  "crystalspace.mesh.object.terrainimproved.simpledatafeeder");

  t_feeder_01->SetParam("heightmap source", "/lev/terraini/heightmap_01.png");
  t_feeder_01->SetParam("materialmap source", "/lev/terraini/material_01.png");

  csRef<iTerrainDataFeeder> t_feeder_10 = csLoadPlugin<iTerrainDataFeeder>(
  GetObjectRegistry(),
  "crystalspace.mesh.object.terrainimproved.simpledatafeeder");

  t_feeder_10->SetParam("heightmap source", "/lev/terraini/heightmap_10.png");
  t_feeder_10->SetParam("materialmap source", "/lev/terraini/material_10.png");

  csRef<iTerrainDataFeeder> t_feeder_11 = csLoadPlugin<iTerrainDataFeeder>(
  GetObjectRegistry(),
  "crystalspace.mesh.object.terrainimproved.simpledatafeeder");

  t_feeder_11->SetParam("heightmap source", "/lev/terraini/heightmap_11.png");
  t_feeder_11->SetParam("materialmap source", "/lev/terraini/material_11.png");

  csRef<iMeshObjectType> t_mesh_type = csLoadPlugin<iMeshObjectType>(
  GetObjectRegistry(),
  "crystalspace.mesh.object.terrainimproved");
  
  csRef<iMeshObjectFactory> t_mesh_factory = t_mesh_type->NewFactory();
  
  csRef<iTerrainFactory> t_factory = scfQueryInterface<iTerrainFactory>(
  t_mesh_factory);

  t_factory->SetRenderer(t_renderer);
  t_factory->SetCollider(t_collider);
  
  t_factory->AddCell("cell", 65, 65, 64, 64, csVector2(-60, -60),
  csVector3(100, 100, 30), t_feeder);
  
  t_factory->AddCell("cell", 33, 33, 32, 32, csVector2(48, 48),
  csVector3(50, 50, 30), t_feeder_00);
  t_factory->AddCell("cell", 33, 33, 32, 32, csVector2(98, 48),
  csVector3(50, 50, 30), t_feeder_10);
  t_factory->AddCell("cell", 33, 33, 32, 32, csVector2(48, 98),
  csVector3(50, 50, 30), t_feeder_01);
  t_factory->AddCell("cell", 33, 33, 32, 32, csVector2(98, 98),
  csVector3(50, 50, 30), t_feeder_11);

  csRef<iMeshObject> t_mesh = t_mesh_factory->NewInstance();
  
  terrain = scfQueryInterface<iTerrainSystem>(t_mesh);
  
  return true;
}

void TerrainDemo::OnExit()
{
}

bool TerrainDemo::Application()
{
  // Open the main system. This will open all the previously loaded plug-ins.
  // i.e. all windows will be opened.
  if (!OpenApplication (GetObjectRegistry()))
    return ReportError ("Error opening system!");

  if (!Setup())
    return false;

  // This calls the default runloop. This will basically just keep
  // broadcasting process events to keep the game going.
  Run ();

  return true;
}

bool TerrainDemo::LoadMap ()
{
  // Set VFS current directory to the level we want to load.
  csRef<iVFS> VFS (CS_QUERY_REGISTRY (GetObjectRegistry (), iVFS));
  VFS->ChDir ("/lev/terraini");
  // Load the level file which is called 'world'.
  //if (!loader->LoadMapFile ("world"))
  //  ReportError("Error couldn't load level!");
    
  iTextureWrapper* grass_tex = loader->LoadTexture("grass_tex",
  "/lev/terrain/grass.png");
  iTextureWrapper* stone_tex = loader->LoadTexture("stone_tex",
  "/lib/std/stone4.gif");
  iTextureWrapper* lava_tex = loader->LoadTexture("lava_tex",
  "/lev/terraini/lava.png");
  
  rloop = engine->GetRenderLoopManager()->Load(
  "/shader/std_rloop_terrainimproved.xml");

  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
  GetObjectRegistry(), "crystalspace.shared.stringset", iStringSet);
  
  csRef<iShaderManager> shmgr = CS_QUERY_REGISTRY (GetObjectRegistry(),
  iShaderManager);
  
  loader->LoadShader("/lev/terraini/shader_base.xml");
  loader->LoadShader("/lev/terraini/shader_splat.xml");
  
  iShader* shader_base = shmgr->GetShader("terrain_improved_base");
  iShader* shader_splat = shmgr->GetShader("terrain_improved_splatting");
  
  csRefArray<iMaterialWrapper> materials;
  
  iMaterialWrapper* material;
  
  (material = engine->CreateMaterial("LavaMaterial", lava_tex))->
  GetMaterial()->SetShader(strings->Request("ambient"), shader_base);
  materials.Push(material);
  
  (material = engine->CreateMaterial("GrassMaterial", grass_tex))->
  GetMaterial()->SetShader(strings->Request("terrain splat"), shader_splat);
  materials.Push(material);

  (material = engine->CreateMaterial("StoneMaterial", stone_tex))->
  GetMaterial()->SetShader(strings->Request("terrain splat"), shader_splat);
  materials.Push(material);
  
  terrain->SetMaterialPalette(materials);

  return true;
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
  return csApplicationRunner<TerrainDemo>::Run (argc, argv);
}
