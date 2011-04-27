/*
    Copyright (C) 2010 by Jelle Hellemans

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

#include "csterrainedtest.h"

#include <imesh/modifiableterrain.h>

CS_IMPLEMENT_APPLICATION

//---------------------------------------------------------------------------

TerrainEd::TerrainEd ()
{
  SetApplicationName ("CrystalSpace.TerrainEd");
  rectSize = 100.0f;
  rectHeight = -20.0f;
}

TerrainEd::~TerrainEd ()
{
}

void TerrainEd::Frame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();
  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.06 * 60);

  iCamera* c = view->GetCamera ();

  if (kbd->GetKeyState (CSKEY_SHIFT))
  {
    // If the user is holding down shift, the arrow keys will cause
    // the camera to strafe up, down, left or right from it's
    // current position.
    if (kbd->GetKeyState (CSKEY_RIGHT))
      c->Move (CS_VEC_RIGHT * 4 * speed);
    if (kbd->GetKeyState (CSKEY_LEFT))
      c->Move (CS_VEC_LEFT * 4 * speed);
    if (kbd->GetKeyState (CSKEY_UP))
      c->Move (CS_VEC_UP * 4 * speed);
    if (kbd->GetKeyState (CSKEY_DOWN))
      c->Move (CS_VEC_DOWN * 4 * speed);
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
      c->Move (CS_VEC_FORWARD * 4 * speed);
    if (kbd->GetKeyState (CSKEY_DOWN))
      c->Move (CS_VEC_BACKWARD * 4 * speed);
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
  csOrthoTransform ot (rot, c->GetTransform ().GetOrigin ());
  c->SetTransform (ot);

  rm->RenderView (view);
}

bool TerrainEd::OnKeyboard (iEvent& ev)
{
  // We got a keyboard event.
  csKeyEventType eventtype = csKeyEventHelper::GetEventType (&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    // The user pressed a key (as opposed to releasing it).
    utf32_char code = csKeyEventHelper::GetCookedCode (&ev);
    utf32_char rawcode = csKeyEventHelper::GetRawCode (&ev);
    if (code == CSKEY_ESC)
    {
      // The user pressed escape to exit the application.
      // The proper way to quit a Crystal Space application
      // is by broadcasting a csevQuit event. That will cause the
      // main runloop to stop. To do that we get the event queue from
      // the object registry and then post the event.
      csRef<iEventQueue> q = 
        csQueryRegistry<iEventQueue> (GetObjectRegistry ());
      if (q.IsValid ()) q->GetEventOutlet ()->Broadcast(
      	csevQuit (GetObjectRegistry ()));
    }
    else if (code == 'b')
    {
      printf("AddCell!\n");
      csRef<iTerrainFactoryCell> cellf (factory->AddCell());
      cellf->SetName("3");
      cellf->SetPosition(csVector2 (384.0f, -128.0f));
      csRef<iTerrainCell> cell (terrain->AddCell(cellf));
    }
    else if (code == 'n')
    {
      csRef<iTerrainCell> cell (terrain->GetCell("3"));
      terrain->RemoveCell(cell);
      csRef<iTerrainFactoryCell> cellf (factory->GetCell("3"));
      factory->RemoveCell(cellf);
      if (cell) printf("RemoveCell1\n");
      if (cellf) printf("RemoveCell2\n");
    }
    else if (rawcode == CSKEY_PADPLUS)
    {
      printf("CSKEY_PADPLUS\n");
      rectHeight += 20.0f;
      UpdateModifier(ev);
    }
    else if (rawcode == CSKEY_PADMINUS)
    {
      printf("CSKEY_PADMINUS\n");
      rectHeight -= 20.0f;
      UpdateModifier(ev);
    }
    else if (code == 'z')
    {
      if (undoStack.GetSize())
      {
        printf("UNDO\n");
        csRef<iModifiableDataFeeder> feeder = scfQueryInterface<iModifiableDataFeeder> (factory->GetFeeder());
        csRef<iTerrainModifier> m = undoStack.Pop();
        feeder->RemoveModifier(m);
      }
    }

  }

  return false;
}

void TerrainEd::UpdateModifier(iEvent& ev)
{
  csScreenTargetResult result = csEngineTools::FindScreenTarget (
    csVector2 (mouse_x, mouse_y), 10000.0f, view->GetCamera ());

  if (result.mesh)
  {
      csRef<iModifiableDataFeeder> feeder = scfQueryInterface<iModifiableDataFeeder> (factory->GetFeeder());
      if (mod) feeder->RemoveModifier(mod);
      //TODO: why this minus on Z???
      mod = feeder->AddModifier(csVector3(result.isect.x, rectHeight, -result.isect.z), rectSize, rectSize);    
  }
}

bool TerrainEd::OnMouseClick (iEvent& ev)
{
  if (csMouseEventHelper::GetButton(&ev) == csmbLeft)
  {
    printf("csmbLeft\n");
    if (mod) undoStack.Push(mod);
    mod.Invalidate();
  }

  return false;
}

bool TerrainEd::OnMouseDown (iEvent& ev)
{
  if (csMouseEventHelper::GetButton(&ev) == csmbWheelUp)
  {
    printf("csmbWheelUp\n");
    rectSize += 10;
    UpdateModifier(ev);
  }
  else if (csMouseEventHelper::GetButton(&ev) == csmbWheelDown)
  {
    printf("csmbWheelDown\n");
    if (rectSize-10.0f >= 0.0f) rectSize -= 10;
    UpdateModifier(ev);
  }

  return false;
}

bool TerrainEd::OnMouseMove (iEvent& ev)
{
  mouse_x = csMouseEventHelper::GetX(&ev);
  mouse_y = csMouseEventHelper::GetY(&ev);

  UpdateModifier(ev);

  return false;
}

bool TerrainEd::OnInitialize (int /*argc*/, char* /*argv*/ [])
{
  // RequestPlugins() will load all plugins we specify. In addition
  // it will also check if there are plugins that need to be loaded
  // from the config system (both the application config and CS or
  // global configs). In addition it also supports specifying plugins
  // on the commandline.
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
    return ReportError ("Failed to initialize plugins!");

  // "Warm up" the event handler so it can interact with the world
  csBaseEventHandler::Initialize (GetObjectRegistry ());

  if (!RegisterQueue(GetObjectRegistry(), csevAllEvents(GetObjectRegistry())))
    return ReportError("Failed to set up event handler!");

  // Report success
  return true;
}

void TerrainEd::OnExit ()
{
  // Shut down the event handlers we spawned earlier.
  printer.Invalidate ();
}

bool TerrainEd::Application ()
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

bool TerrainEd::SetupModules ()
{
  // Now get the pointer to various modules we need. We fetch them
  // from the object registry. The RequestPlugins() call we did earlier
  // registered all loaded plugins with the object registry.
  g3d = csQueryRegistry<iGraphics3D> (GetObjectRegistry ());
  if (!g3d) return ReportError ("Failed to locate 3D renderer!");

  engine = csQueryRegistry<iEngine> (GetObjectRegistry ());
  if (!engine) return ReportError ("Failed to locate 3D engine!");

  vc = csQueryRegistry<iVirtualClock> (GetObjectRegistry ());
  if (!vc) return ReportError ("Failed to locate Virtual Clock!");

  kbd = csQueryRegistry<iKeyboardDriver> (GetObjectRegistry ());
  if (!kbd) return ReportError ("Failed to locate Keyboard Driver!");

  loader = csQueryRegistry<iLoader> (GetObjectRegistry ());
  if (!loader) return ReportError ("Failed to locate Loader!");

  // We need a View to the virtual world.
  view.AttachNew(new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  // We use the full window to draw the world.
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  // Here we create our world.
  CreateRoom ();

  // Let the engine prepare the meshes and textures.
  engine->Prepare ();

  // Now calculate static lighting for our geometry.
  using namespace CS::Lighting;
  SimpleStaticLighter::ShineLights (room, engine, 4);

  rm = engine->GetRenderManager ();

  // These are used store the current orientation of the camera
  rotY = rotX = 0;
 
  // Now we need to position the camera in our world.
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 25, -3));

  // We use some other "helper" event handlers to handle 
  // pushing our work into the 3D engine and rendering it
  // to the screen.
  printer.AttachNew (new FramePrinter (GetObjectRegistry ()));

  return true;
}

void TerrainEd::CreateRoom ()
{  
  // We create a new sector called "room".
  room = engine->CreateSector ("room");

  // Now we need light to see something.
  csRef<iLight> light;
  iLightList* ll = room->GetLights ();

  light = engine->CreateLight (0, csVector3 (-3, 200, 0), 1000, csColor (1, 1, 1));
  ll->Add (light);


  iTextureWrapper* grass = loader->LoadTexture ("grass", "/this/data/terrained/cobgrass.png", 2, 0, true, false);
  if (!grass)
    ReportError ("Error loading %s texture!",
		 CS::Quote::Single ("grass"));

  
  iTextureWrapper* stone = loader->LoadTexture ("stone", "/this/data/terrained/cliff34ug6.jpg", 2, 0, true, false);
  if (!stone)
    ReportError ("Error loading %s texture!",
		 CS::Quote::Single ("stone"));

  csRef<iShader> shader = loader->LoadShader ("/shader/terrain/terrain.xml");
  if (!stone)
    ReportError ("Error loading %s shader!",
		 CS::Quote::Single ("terrain"));

  csRef<iMaterialWrapper> terrainmat = engine->CreateMaterial("terrain", 0);

  csRef<iShaderVarStringSet> stringSet = csQueryRegistryTagInterface<iShaderVarStringSet> (object_reg, "crystalspace.shader.variablenameset");

  csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet> (object_reg, "crystalspace.shared.stringset");
  csStringID ambient = strings->Request("ambient");
  csStringID diffuse = strings->Request("diffuse");

  terrainmat->GetMaterial()->SetShader(ambient, shader);
  terrainmat->GetMaterial()->SetShader(diffuse, shader);

  CS::ShaderVarStringID mat1 = stringSet->Request("tex material1");
  CS::ShaderVarStringID mat1s = stringSet->Request("material1 texscale");
  CS::ShaderVarStringID mat1a = stringSet->Request("material1 attributes");

  CS::ShaderVarStringID mat2 = stringSet->Request("tex material2");
  CS::ShaderVarStringID mat2s = stringSet->Request("material2 texscale");
  CS::ShaderVarStringID mat2a = stringSet->Request("material2 attributes");

  csRef<csShaderVariable> sv;
  sv = terrainmat->GetMaterial()->GetVariableAdd(mat1);
  sv->SetValue(grass);
  sv = terrainmat->GetMaterial()->GetVariableAdd(mat1s);
  sv->SetValue(csVector2(15,15));
  sv = terrainmat->GetMaterial()->GetVariableAdd(mat1a);
  sv->SetValue(csVector4(-1.0,17.0,-0.001,0.6));

  sv = terrainmat->GetMaterial()->GetVariableAdd(mat2);
  sv->SetValue(stone);
  sv = terrainmat->GetMaterial()->GetVariableAdd(mat2s);
  sv->SetValue(csVector2(15,15));
  sv = terrainmat->GetMaterial()->GetVariableAdd(mat2a);
  sv->SetValue(csVector4(0.0,17.0,0.3,1.0));


  csRef<iPluginManager> pluginManager = csQueryRegistry<iPluginManager> (GetObjectRegistry ());

  csRef<iMeshObjectType> meshType = csLoadPlugin<iMeshObjectType> (GetObjectRegistry (), "crystalspace.mesh.object.terrain2");

  csRef<iMeshObjectFactory> meshFactory = meshType->NewFactory ();
  /*csRef<iTerrainFactory>*/ factory = scfQueryInterface<iTerrainFactory> (meshFactory);

  csRef<iTerrainRenderer> renderer = csLoadPluginCheck<iTerrainRenderer> (pluginManager, "crystalspace.mesh.object.terrain2.bruteblockrenderer");
  csRef<iTerrainCollider> collider = csLoadPluginCheck<iTerrainCollider> (pluginManager, "crystalspace.mesh.object.terrain2.collider");
  csRef<iTerrainDataFeeder> feeder = csLoadPluginCheck<iTerrainDataFeeder> (pluginManager, "crystalspace.mesh.object.terrain2.modifiabledatafeeder");

  factory->SetRenderer(renderer);
  factory->SetCollider(collider);
  factory->SetFeeder(feeder);

  factory->SetAutoPreLoad(true);
  factory->SetMaxLoadedCells(20);

  csRef<iTerrainFactoryCell> defaultCell (factory->GetDefaultCell());
  defaultCell->SetSize (csVector3 (256.0f, 16.0f, 256.0f));
  defaultCell->SetGridWidth (257);
  defaultCell->SetGridHeight (257);
  defaultCell->SetMaterialMapWidth (256);
  defaultCell->SetMaterialMapHeight (256);
  defaultCell->SetMaterialPersistent (false);

  defaultCell->SetBaseMaterial (terrainmat);

  defaultCell->GetRenderProperties()->SetParameter("block resolution", "16");
  defaultCell->GetRenderProperties()->SetParameter("lod splitcoeff", "16");

  defaultCell->GetFeederProperties()->SetParameter("heightmap source", "/lev/terrain/heightmap.png");
  //defaultCell->GetFeederProperties()->SetParameter("materialmap source", "/lev/terrain/materialmap.png");
  
  {
    csRef<iTerrainFactoryCell> cell (factory->AddCell());
    cell->SetName("1");
    cell->SetPosition(csVector2 (-128.0f, -128.0f));
    //cell->GetFeederProperties()->SetParameter("heightmap source", "/lev/terraini/heightmap_01.png");
  }
  {
    csRef<iTerrainFactoryCell> cell (factory->AddCell());
    cell->SetName("2");
    cell->SetPosition(csVector2 (128.0f, -128.0f));
    //cell->GetFeederProperties()->SetParameter("heightmap source", "/lev/terraini/heightmap_11.png");
  }

  csRef<iMeshFactoryWrapper> meshf = engine->CreateMeshFactory(meshFactory, "terrainFact");
  csRef<iMeshWrapper> mesh = engine->CreateMeshWrapper(meshf, "terrain", room);

  /*csRef<iTerrainSystem>*/ terrain = scfQueryInterface<iTerrainSystem> (mesh->GetMeshObject());
  if (terrain) printf("blah\n");

  
}

/*-------------------------------------------------------------------------*
 * Main function
 *-------------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  return csApplicationRunner<TerrainEd>::Run (argc, argv);
}
