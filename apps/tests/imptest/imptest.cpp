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

#include "imptest.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

ImposterTest::ImposterTest ()
{
  SetApplicationName ("CrystalSpace.ImposterTest");
}

ImposterTest::~ImposterTest ()
{
}

void ImposterTest::Frame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();
  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.06 * 20);

  iCamera* c = view->GetCamera();

  if (kbd->GetKeyState (CSKEY_SPACE))
  {
    // Reset.
    csVector3 orig = c->GetTransform().GetOrigin();
    c->GetTransform ().SetOrigin (look_point
	+ (orig - look_point).Unit () * distance);
    c->GetTransform ().LookAt (look_point-orig, csVector3(0,1,0) );
  }
  else if (kbd->GetKeyState (CSKEY_SHIFT))
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
    // Pan around the object.
    csVector3 orig = c->GetTransform().GetOrigin();
    csVector3 new_look_point = c->GetTransform ().This2Other (
	csVector3 (0, 0, distance));
    
    if (kbd->GetKeyState (CSKEY_LEFT))
      orig = csYRotMatrix3(-speed) * (orig-new_look_point) + new_look_point;
    if (kbd->GetKeyState (CSKEY_RIGHT))
      orig = csYRotMatrix3(speed) * (orig-new_look_point) + new_look_point;
    if (kbd->GetKeyState (CSKEY_UP))
      orig = csXRotMatrix3(speed) * (orig-new_look_point) + new_look_point;
    if (kbd->GetKeyState (CSKEY_DOWN))
      orig = csXRotMatrix3(-speed) * (orig-new_look_point) + new_look_point;

    c->GetTransform().SetOrigin(orig);

    if (kbd->GetKeyState (CSKEY_PGUP))
    {
      distance -= speed * 5.0f;
      if (distance < 0.5f) distance = 0.5f;
      c->GetTransform ().SetOrigin (look_point
	+ (orig - look_point).Unit () * distance);
    }
    if (kbd->GetKeyState (CSKEY_PGDN))
    {
      distance += speed * 5.0f;
      c->GetTransform ().SetOrigin (look_point
	+ (orig - look_point).Unit () * distance);
    }

    c->GetTransform().LookAt (look_point-orig, csVector3(0,1,0) );
  }

  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (CSDRAW_3DGRAPHICS))
    return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();
}

bool ImposterTest::OnKeyboard(iEvent& ev)
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
        csQueryRegistry<iEventQueue> (GetObjectRegistry());
      if (q.IsValid()) q->GetEventOutlet()->Broadcast(
	  csevQuit(GetObjectRegistry()));
    }
  }
  return false;
}

bool ImposterTest::OnInitialize(int /*argc*/, char* /*argv*/ [])
{
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
    CS_REQUEST_END))
    return ReportError("Failed to initialize plugins!");

  csBaseEventHandler::Initialize(GetObjectRegistry());

  // Now we need to setup an event handler for our application.
  // Crystal Space is fully event-driven. Everything (except for this
  // initialization) happens in an event.
  if (!RegisterQueue(GetObjectRegistry(), csevAllEvents(GetObjectRegistry())))
    return ReportError("Failed to set up event handler!");

  return true;
}

void ImposterTest::OnExit()
{
  printer.Invalidate ();
}

bool ImposterTest::Application()
{
  // Open the main system. This will open all the previously loaded plug-ins.
  // i.e. all windows will be opened.
  if (!OpenApplication(GetObjectRegistry()))
    return ReportError("Error opening system!");

  if (SetupModules())
  {
    // This calls the default runloop. This will basically just keep
    // broadcasting process events to keep the game going.
    Run();
  }
  return true;
}

bool ImposterTest::SetupModules()
{
  // Now get the pointer to various modules we need. We fetch them
  // from the object registry. The RequestPlugins() call we did earlier
  // registered all loaded plugins with the object registry.
  // The virtual clock.
  g3d = csQueryRegistry<iGraphics3D> (GetObjectRegistry());
  if (!g3d) return ReportError("Failed to locate 3D renderer!");

  csRef<iConfigManager> cfman = csQueryRegistry<iConfigManager>(GetObjectRegistry());
  cfman->SetInt("Engine.Imposters.UpdatePerFrame", 25);

  engine = csQueryRegistry<iEngine> (GetObjectRegistry());
  if (!engine) return ReportError("Failed to locate 3D engine!");

  vc = csQueryRegistry<iVirtualClock> (GetObjectRegistry());
  if (!vc) return ReportError("Failed to locate Virtual Clock!");

  kbd = csQueryRegistry<iKeyboardDriver> (GetObjectRegistry());
  if (!kbd) return ReportError("Failed to locate Keyboard Driver!");

  loader = csQueryRegistry<iLoader> (GetObjectRegistry());
  if (!loader) return ReportError("Failed to locate Loader!");

  // We need a View to the virtual world.
  view.AttachNew(new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  // We use the full window to draw the world.
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  // Here we create our world.
  CreateRoom();

  // Here we create our world.
  CreateSprites();

  // Let the engine prepare all lightmaps for use and also free all images 
  // that were loaded for the texture manager.
  engine->Prepare ();

  using namespace CS::Lighting;
  SimpleStaticLighter::ShineLights (room, engine, 4);

  distance = 5.0f;

  // Now we need to position the camera in our world.
  iCamera* c = view->GetCamera ();
  c->SetSector (room);
  csVector3 orig = csVector3 (0, 0, 0);
  c->GetTransform ().SetOrigin (look_point
	+ (orig - look_point).Unit () * distance);
  c->GetTransform ().LookAt (look_point-orig, csVector3(0,1,0) );

  printer.AttachNew (new FramePrinter (object_reg));

  return true;
}


void ImposterTest::CreateRoom ()
{
  // Load the texture from the standard library.  This is located in
  // CS/data/standard.zip and mounted as /lib/std using the Virtual
  // File System (VFS) plugin.
  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
    ReportError("Error loading %s texture!",
		CS::Quote::Single ("stone4"));

  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");

  // We create a new sector called "room".
  room = engine->CreateSector ("room");

  // First we make a primitive for our geometry.
  using namespace CS::Geometry;
  DensityTextureMapper mapper (0.3f);
  TesselatedBox box (csVector3 (-6, -6, -6), csVector3 (6, 6, 6));
  box.SetLevel (3);
  box.SetMapper (&mapper);
  box.SetFlags (Primitives::CS_PRIMBOX_INSIDE);

  // Now we make a factory and a mesh at once.
  csRef<iMeshWrapper> walls = GeneralMeshBuilder::CreateFactoryAndMesh (
      engine, room, "walls", "walls_factory", &box);
  walls->GetMeshObject ()->SetMaterialWrapper (tm);

  // Now we need light to see something.
  csRef<iLight> light;
  iLightList* ll = room->GetLights ();

  light = engine->CreateLight(0, csVector3(-3, 3, 0), 10, csColor(1, 0, 0));
  ll->Add (light);

  light = engine->CreateLight(0, csVector3(3, 3,  0), 10, csColor(0, 0, 1));
  ll->Add (light);

  light = engine->CreateLight(0, csVector3(0, 3, -3), 10, csColor(0, 1, 0));
  ll->Add (light);

  // Load shader required for this test app.
  loader->LoadShader("/shader/lighting/lighting_imposter.xml");
}

void ImposterTest::CreateSprite (iMeshFactoryWrapper* imeshfact,
    const csVector3& pos)
{
  static int cnt = 0;
  cnt++;
  csString spname = "MySprite";
  spname += cnt;
  // Create the sprite and add it to the engine.
  csRef<iMeshWrapper> sprite = engine->CreateMeshWrapper (
    imeshfact, spname, room, pos);
  csMatrix3 m; m.Identity ();
  sprite->GetMovable ()->SetTransform (m);
  sprite->GetMovable ()->UpdateMove ();
  csRef<iSprite3DState> spstate (
    scfQueryInterface<iSprite3DState> (sprite->GetMeshObject ()));
  spstate->SetAction ("default");
  //spstate->SetMixMode (CS_FX_SETALPHA (.5));

  // The following two calls are not needed since CS_ZBUF_USE and
  // Object render priority are the default but they show how you
  // can do this.
  sprite->SetZBufMode (CS_ZBUF_USE);
  sprite->SetRenderPriority (engine->GetObjectRenderPriority ());
}

void ImposterTest::CreateSprites ()
{
  // Load a texture for our sprite.
  iTextureWrapper* txt = loader->LoadTexture ("spark",
    "/lib/std/spark.png");
  if (txt == 0)
    ReportError("Error loading texture!");

  // Load a sprite template from disk.
  csRef<iMeshFactoryWrapper> imeshfact (
    loader->LoadMeshObjectFactory ("/lib/std/sprite1"));
  if (imeshfact == 0)
    ReportError("Error loading mesh object factory!");

  // Set imposter settings.
  csRef<iImposterFactory> i = scfQueryInterface<iImposterFactory> (imeshfact);
  i->SetMinDistance(3.0f);
  i->SetRotationTolerance(0.40f);
  i->SetCameraRotationTolerance(3.0f);
  i->SetShader("base", "lighting_imposter");
  i->SetShader("diffuse", "lighting_imposter");
  i->SetRenderReal(true);

  look_point = csVector3 (1, 1, 1);

  float x, y, z;
  for (x = 0 ; x <= 2.1 ; x += .5)
    for (y = 0 ; y <= 2.1 ; y += .5)
      for (z = 0 ; z <= 2.1 ; z += .5)
        CreateSprite (imeshfact, csVector3 (x, y, z));

  printf("done...\n");
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
   * again). ImposterTest1 does not use that functionality itself, however, it
   * allows you to later use "ImposterTest.Restart();" and it'll just work.
   */
  return csApplicationRunner<ImposterTest>::Run (argc, argv);
}
