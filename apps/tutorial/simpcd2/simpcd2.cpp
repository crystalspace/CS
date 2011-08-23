/*
Copyright (C) 2011 by Liu Lu

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

#include "simpcd2.h"
#include "imesh/terrain2.h"

CS_IMPLEMENT_APPLICATION

#define ENVIRONMENT_WALLS 1
#define ENVIRONMENT_TERRAIN 2

//---------------------------------------------------------------------------

Simple::Simple ()
{
  SetApplicationName ("CrystalSpace.SimpleCD2");

  environment = ENVIRONMENT_WALLS;

  rot1_direction = 1;
  rot2_direction = -1;
  sprite_col = 0;
  
  localTrans.Identity ();
}

Simple::~Simple ()
{
}

void Simple::Frame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();
  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.06 * 20);

  //---------
  // First rotate the entire mesh. Rotation of the children
  // is independent from this.
  //---------
   csZRotMatrix3 rotmat (speed / 5);
   parent_sprite->GetMovable ()->Transform (rotmat);
   parent_sprite->GetMovable ()->UpdateMove ();

  //---------
  // Rotate the two sprites depending on elapsed time.
  // Remember the old transforms so that we can restore it later if there
  // was a collision.
  //---------
  csZRotMatrix3 rotmat1 (rot1_direction * speed / 2.5);
  csOrthoTransform old_trans1 = sprite1->GetMovable ()->GetFullTransform ();
  csOrthoTransform newTrans1 = old_trans1;
  newTrans1.RotateOther (rotmat1);
  sprite1_obj->SetTransform (newTrans1);
  
  csZRotMatrix3 rotmat2 (rot2_direction * speed / 1.0);
  csOrthoTransform old_trans2 = sprite2->GetMovable ()->GetFullTransform ();
  csOrthoTransform newTrans2 = old_trans2;
  newTrans2.RotateOther (rotmat2);
  sprite2_obj->SetTransform (newTrans2);

  //---------
  // Check for collision between the two children.
  // If there is a collision we undo our transforms of the children and
  // reverse rotation direction for both.
  // Important note! We use GetFullTransform() here because we want
  // to check collision based on the real position of the objects and
  // not the relative positions (which is what GetTransform() would return).
  // But the transform that we remembered for restoration later is the
  // one returned from GetTransform() since there is no equivalent
  // SetFullTransform().
  //---------
  if (environment == ENVIRONMENT_WALLS)
  {
    bool cd = sprite1_obj->Collide (sprite2_obj);
    if (cd)
    {
      // Restore old transforms and reverse turning directions.
      sprite1_obj->SetTransform (old_trans1);
      sprite2_obj->SetTransform (old_trans2);
      rot1_direction = -rot1_direction;
      rot2_direction = -rot2_direction;
    }
  }
  else
  {
    //---------
    // Check for collision between the sprite and the terrain.
    // If there is a collision we undo our transforms of the sprite and
    // reverse rotation direction for it.
    //---------
     csArray<CS::Collision2::CollisionData> data;
     collisionSector->CollisionTest (terrainObject, data);
     for (size_t i = 0; i < data.GetSize (); i++)
     {
       if (data[i].objectB == sprite1_obj || data[i].objectA == sprite1_obj)
       {
         sprite1_obj->SetTransform (old_trans1);
         rot1_direction = -rot1_direction;
       }
       else if (data[i].objectB == sprite2_obj || data[i].objectA == sprite2_obj)
       {
         sprite2_obj->SetTransform (old_trans2);
         rot2_direction = -rot2_direction;
       }
     }
     // This also works.
//     bool cd = sprite1_obj->Collide (terrainObject);
//     if (cd)
//     {
//       // Restore old transforms and reverse turning directions.
//       sprite1_obj->SetTransform (old_trans1);
//       rot1_direction = -rot1_direction;
//     }
// 
//     cd = sprite2_obj->Collide (terrainObject);
//     if (cd)
//     {
//       // Restore old transforms and reverse turning directions.
//       sprite2_obj->SetTransform (old_trans2);
//       rot2_direction = -rot2_direction;
//     }
  }

  iCamera* c = view->GetCamera();

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
  csOrthoTransform ot (rot, c->GetTransform().GetOrigin ());
  c->SetTransform (ot);

  rm->RenderView (view);
}

bool Simple::OnKeyboard(iEvent& ev)
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

bool Simple::OnInitialize(int /*argc*/, char* /*argv*/ [])
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
    CS_REQUEST_PLUGIN("crystalspace.collision.opcode2",
    CS::Collision2::iCollisionSystem),
    CS_REQUEST_END))
    return ReportError("Failed to initialize plugins!");

  // "Warm up" the event handler so it can interact with the world
  csBaseEventHandler::Initialize(GetObjectRegistry());

  // Now we need to register the event handler for our application.
  // Crystal Space is fully event-driven. Everything (except for this
  // initialization) happens in an event.
  // Rather than simply handling all events, we subscribe to the
  // particular events we're interested in.
  csEventID events[] = {
    csevFrame (GetObjectRegistry()),
    csevKeyboardEvent (GetObjectRegistry()),
    csevMouseEvent (GetObjectRegistry ()),
    CS_EVENTLIST_END
  };
  if (!RegisterQueue(GetObjectRegistry(), events))
    return ReportError("Failed to set up event handler!");

  // Report success
  return true;
}

void Simple::OnExit()
{
  // Shut down the event handlers we spawned earlier.
  drawer.Invalidate();
  printer.Invalidate();
}


bool Simple::Application()
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

bool Simple::SetupModules ()
{
  // Now get the pointer to various modules we need. We fetch them
  // from the object registry. The RequestPlugins() call we did earlier
  // registered all loaded plugins with the object registry.
  g3d = csQueryRegistry<iGraphics3D> (GetObjectRegistry());
  if (!g3d) return ReportError("Failed to locate 3D renderer!");

  engine = csQueryRegistry<iEngine> (GetObjectRegistry());
  if (!engine) return ReportError("Failed to locate 3D engine!");

  vc = csQueryRegistry<iVirtualClock> (GetObjectRegistry());
  if (!vc) return ReportError("Failed to locate Virtual Clock!");

  kbd = csQueryRegistry<iKeyboardDriver> (GetObjectRegistry());
  if (!kbd) return ReportError("Failed to locate Keyboard Driver!");

  mouse = csQueryRegistry<iMouseDriver> (GetObjectRegistry ());
  if (!mouse) return ReportError ("Failed to locate Mouse Driver!");

  loader = csQueryRegistry<iLoader> (GetObjectRegistry());
  if (!loader) return ReportError("Failed to locate Loader!");

  collisionSystem = csQueryRegistry<CS::Collision2::iCollisionSystem> (GetObjectRegistry());
  if (!collisionSystem) return ReportError("Failed to locate collision system!");

  // We need a View to the virtual world.
  view.AttachNew(new csView (engine, g3d));
  g2d = g3d->GetDriver2D ();
  // We use the full window to draw the world.
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  // Here we create our world.
  if (environment == ENVIRONMENT_WALLS)
    CreateRoom();
  else
    CreateTerrain ();

  // Let the engine prepare all lightmaps for use and also free all images 
  // that were loaded for the texture manager.
  engine->Prepare ();

  using namespace CS::Lighting;
  SimpleStaticLighter::ShineLights (room, engine, 4);

  rm = engine->GetRenderManager();

  // these are used store the current orientation of the camera
  rotY = rotX = 0;

  // Now we need to position the camera in our world.
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 5, -3));

  // We use some other "helper" event handlers to handle 
  // pushing our work into the 3D engine and rendering it
  // to the screen.
  //drawer.AttachNew(new FrameBegin3DDraw (GetObjectRegistry (), view));
  printer.AttachNew(new FramePrinter (GetObjectRegistry ()));

  return true;
}

void Simple::CreateRoom ()
{
  // Load the texture from the standard library.  This is located in
  // CS/data/standard.zip and mounted as /lib/std using the Virtual
  // File System (VFS) plugin.
  if (!loader->LoadTexture ("brick", "/lib/std/castle/brick1_d.jpg"))
    ReportError("Error loading %s texture!",
		CS::Quote::Single ("brick1_d"));

  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("brick");

  // Add a normal map to the material.
  {
    /* Shader variables are identified by numeric IDs for performance reasons.
    * The shader var string set translates string IDs to numeric IDs. */
    csRef<iShaderVarStringSet> svStrings =
      csQueryRegistryTagInterface<iShaderVarStringSet> (GetObjectRegistry(),
      "crystalspace.shader.variablenameset");
    // Load the normal map texture itself
    csRef<iTextureHandle> normalMap = loader->LoadTexture (
      "/lib/std/castle/brick1_n.jpg");
    // Set this to avoid compression - makes for better quality here
    normalMap->SetTextureClass ("normalmap");
    // The normal map is attached to the material through a shader variable.
    csShaderVariable* svNormalMap =
      tm->GetMaterial()->GetVariableAdd (svStrings->Request ("tex normal"));
    svNormalMap->SetValue (normalMap);
  }

  // We create a new sector called "room".
  room = engine->CreateSector ("room");

  // Create the dynamic system
  collisionSector = collisionSystem->CreateCollisionSector ();
  if (!collisionSector) 
  {
    ReportError ("Error creating collision sector!");
    return;
  }

  // Bind the sector to the collision sector.
  collisionSector->SetSector (room);

  // Creating the walls for our room.

  // First we make a primitive for our geometry.
  using namespace CS::Geometry;
  DensityTextureMapper mapper (0.3f);
  TesselatedBox box (csVector3 (-5, 0, -5), csVector3 (5, 20, 5));
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

  light = engine->CreateLight(0, csVector3(-3, 5, 0), 10, csColor(2, 0, 0));
  ll->Add (light);

  light = engine->CreateLight(0, csVector3(3, 5,  0), 10, csColor(0, 0, 2));
  ll->Add (light);

  light = engine->CreateLight(0, csVector3(0, 5, -3), 10, csColor(0, 2, 0));
  ll->Add (light);

  // Load a texture for our sprite.
  iTextureWrapper* txt = loader->LoadTexture ("spark",
    "/lib/std/spark.png");
  if (txt == 0)
  {
    ReportError ("Error loading texture!");
    return;
  }

  //---------
  // Load a sprite template from disk.
  //---------
  csRef<iMeshFactoryWrapper> imeshfact (loader->LoadMeshObjectFactory (
    "/lib/std/sprite2"));
  if (imeshfact == 0)
  {
    ReportError ("Error loading mesh object factory!");
    return;
  }

  //--------
  // Create two collision groups and make them don't do collision detection with each other.
  // Or you can just create one group. Objects in this group will not collide with others.
  //--------

  /*collisionSector->CreateCollisionGroup ("Sprite");
  collisionSector->CreateCollisionGroup ("SpriteFiltered");

  bool coll = collisionSector->GetGroupCollision ("Sprite", "SpriteFiltered");
  if (coll)
    collisionSector->SetGroupCollision ("Sprite", "SpriteFiltered", false);*/


  //---------
  // Here we create a hierarchical mesh object made from three sprites.
  // There is one 'anchor' (parent_sprite) which has two children
  // ('sprite1' and 'sprite2'). Later on we will rotate the two children
  // and also rotate the entire hierarchical mesh.
  //---------
  csRef<iSprite3DState> spstate;

  // First create the parent sprite.
  parent_sprite = engine->CreateMeshWrapper (
    imeshfact, "Parent", room,
    csVector3 (0, 5, 3.5));
  spstate = 
    scfQueryInterface<iSprite3DState> (parent_sprite->GetMeshObject ());
  spstate->SetAction ("default");
  parent_sprite->GetMovable ()->Transform (csZRotMatrix3 (PI/2.));
  parent_sprite->GetMovable ()->UpdateMove ();
  csOrthoTransform parTrans = parent_sprite->GetMovable ()->GetFullTransform ();

  // Now create the first child.
  sprite1 = engine->CreateMeshWrapper (imeshfact, "Rotater1");
  csOrthoTransform tc (csZRotMatrix3 (PI/2.), csVector3 (0, -.5, -.5));

  spstate = scfQueryInterface<iSprite3DState> (sprite1->GetMeshObject ());
  spstate->SetAction ("default");
  sprite1->QuerySceneNode ()->SetParent (parent_sprite->QuerySceneNode ());

  // Create a collider.
  sprite_col = collisionSystem->CreateColliderConcaveMesh (sprite1);

  // Create a collision object. Set the collider and the movable.
  sprite1_obj = collisionSystem->CreateCollisionObject ();
  sprite1_obj->AddCollider (sprite_col, localTrans);
  sprite1_obj->SetAttachedMovable (sprite1->GetMovable ());
  // You have to set a world transform to collision object.
  sprite1_obj->SetTransform (tc * parTrans);
  // The object must rebuild before it's added to a sector.
  sprite1_obj->RebuildObject ();

  // Add the object to the sector.
  collisionSector->AddCollisionObject (sprite1_obj);
  // Give it a collision group.
  //sprite1_obj->SetCollisionGroup ("Sprite");

  // Now create the second child.
  sprite2 = engine->CreateMeshWrapper (imeshfact, "Rotater2");
  tc = csOrthoTransform (csZRotMatrix3 (PI/2.), csVector3 (0, .5, -.5));

  spstate = scfQueryInterface<iSprite3DState> (sprite2->GetMeshObject ());
  spstate->SetAction ("default");
  sprite2->QuerySceneNode ()->SetParent (parent_sprite->QuerySceneNode ());

  sprite2_obj = collisionSystem->CreateCollisionObject ();
  sprite2_obj->AddCollider (sprite_col, localTrans);
  sprite2_obj->SetAttachedMovable (sprite2->GetMovable ());
  sprite2_obj->SetTransform (tc * parTrans);
  sprite2_obj->RebuildObject ();

  collisionSector->AddCollisionObject (sprite2_obj);
  //sprite2_obj->SetCollisionGroup ("SpriteFiltered");
}

void Simple::CreateTerrain ()
{
  printf ("Loading terrain...\n");

  // Load the level file
  csRef<iVFS> VFS (csQueryRegistry<iVFS> (GetObjectRegistry ()));
  VFS->ChDir ("/lev/terraini");

  if (!loader->LoadMapFile ("world"))
  {
    ReportError("Error couldn't load terrain level!");
    return;
  }

  // We create a new sector called "room".
  room = engine->FindSector ("room");

  engine->Prepare ();

  // Find the terrain mesh
  csRef<iMeshWrapper> terrainWrapper = engine->FindMeshObject ("Terrain");
  if (!terrainWrapper)
  {
    ReportError("Error cannot find the terrain mesh!");
    return;
  }

  // Get the terrain system.
  csRef<iTerrainSystem> terrain =
    scfQueryInterface<iTerrainSystem> (terrainWrapper->GetMeshObject ());
  if (!terrain)
  {
    ReportError("Error cannot find the terrain interface!");
    return;
  }

  // Create the collision sector.
  collisionSector = collisionSystem->CreateCollisionSector ();
  if (!collisionSector) 
  {
    ReportError ("Error creating collision sector!");
    return;
  }

  // Bind the sector to the collision sector.
  collisionSector->SetSector (room);

  // Create a terrain collider.
  csRef<CS::Collision2::iColliderTerrain> terrainCollider = collisionSystem->CreateColliderTerrain (terrain);

  // Create a collision object. Set the collider.
  terrainObject = collisionSystem->CreateCollisionObject ();
  terrainObject->AddCollider (terrainCollider, localTrans);
  terrainObject->RebuildObject ();

  // Add the object to the sector.
  collisionSector->AddCollisionObject (terrainObject);

  // Now we need light to see something.
  csRef<iLight> light;
  iLightList* ll = room->GetLights ();

  light = engine->CreateLight(0, csVector3(-3, 5, 0), 10, csColor(2, 0, 0));
  ll->Add (light);

  light = engine->CreateLight(0, csVector3(3, 5,  0), 10, csColor(0, 0, 2));
  ll->Add (light);

  light = engine->CreateLight(0, csVector3(0, 5, -3), 10, csColor(0, 2, 0));
  ll->Add (light);

  // Load a texture for our sprite.
  iTextureWrapper* txt = loader->LoadTexture ("spark",
    "/lib/std/spark.png");
  if (txt == 0)
  {
    ReportError ("Error loading texture!");
    return;
  }

  //---------
  // Load a sprite template from disk.
  //---------
  csRef<iMeshFactoryWrapper> imeshfact (loader->LoadMeshObjectFactory (
    "/lib/std/sprite2"));
  if (imeshfact == 0)
  {
    ReportError ("Error loading mesh object factory!");
    return;
  }

  //---------
  // Here we create a hierarchical mesh object made from three sprites.
  // There is one 'anchor' (parent_sprite) which has two children
  // ('sprite1' and 'sprite2'). Later on we will rotate the two children
  // and also rotate the entire hierarchical mesh.
  //---------
  csRef<iSprite3DState> spstate;

  // First create the parent sprite.
  parent_sprite = engine->CreateMeshWrapper (
    imeshfact, "Parent", room,
    csVector3 (0, 2, 3.5));
  spstate = 
    scfQueryInterface<iSprite3DState> (parent_sprite->GetMeshObject ());
  spstate->SetAction ("default");
  parent_sprite->GetMovable ()->Transform (csZRotMatrix3 (PI/2.));
  parent_sprite->GetMovable ()->UpdateMove ();
  csOrthoTransform parTrans = parent_sprite->GetMovable ()->GetFullTransform ();

  // Now create the first child.
  sprite1 = engine->CreateMeshWrapper (imeshfact, "Rotater1");
  csOrthoTransform tc (csZRotMatrix3 (0), csVector3 (0, -.5, -.5));

  spstate = scfQueryInterface<iSprite3DState> (sprite1->GetMeshObject ());
  spstate->SetAction ("default");
  sprite1->QuerySceneNode ()->SetParent (parent_sprite->QuerySceneNode ());

  // Create a collider.
  sprite_col = collisionSystem->CreateColliderConcaveMesh (sprite1);

  // Create a collision object. Set the collider and the movable.
  sprite1_obj = collisionSystem->CreateCollisionObject ();
  sprite1_obj->AddCollider (sprite_col, localTrans);
  sprite1_obj->SetAttachedMovable (sprite1->GetMovable ());

  // You have to set a world transform to collision object.
  sprite1_obj->SetTransform (tc * parTrans);
  sprite1_obj->RebuildObject ();

  // Add the object to the sector.
  collisionSector->AddCollisionObject (sprite1_obj);

  // Now create the second child.
  sprite2 = engine->CreateMeshWrapper (imeshfact, "Rotater2");
  tc = csOrthoTransform (csZRotMatrix3 (0), csVector3 (0, .5, -.5));

  spstate = scfQueryInterface<iSprite3DState> (sprite2->GetMeshObject ());
  spstate->SetAction ("default");
  sprite2->QuerySceneNode ()->SetParent (parent_sprite->QuerySceneNode ());

  sprite2_obj = collisionSystem->CreateCollisionObject ();
  sprite2_obj->AddCollider (sprite_col, localTrans);
  sprite2_obj->SetAttachedMovable (sprite2->GetMovable ());
  sprite2_obj->SetTransform (tc * parTrans);
  sprite2_obj->RebuildObject ();

  collisionSector->AddCollisionObject (sprite2_obj);
}

bool Simple::OnMouseDown (iEvent& event)
{
  if (csMouseEventHelper::GetButton (&event) == 0)
  {
    // Find the rigid body that was clicked on
    // Compute the end beam points
    csRef<iCamera> camera = view->GetCamera ();
    csVector2 v2d (mouse->GetLastX (), g2d->GetHeight () - mouse->GetLastY ());
    csVector3 v3d = camera->InvPerspective (v2d, 10000);
    csVector3 startBeam = camera->GetTransform ().GetOrigin ();
    csVector3 endBeam = camera->GetTransform ().This2Other (v3d);

    CS::Collision2::HitBeamResult hitResult = 
      collisionSector->HitBeam (startBeam, endBeam);

    if (!hitResult.hasHit)
      return false;

    // Stop the object.
    if (hitResult.object == sprite1_obj)
    {
      rot1_direction = (rot1_direction == 0.0f? 1.0f : 0.0f);
    }
    else if (hitResult.object == sprite2_obj)
    {
      rot2_direction = (rot2_direction == 0.0f? -1.0f : 0.0f);
    }
  }
  return true;
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
  * again). Simple1 does not use that functionality itself, however, it
  * allows you to later use "Simple.Restart();" and it'll just work.
  */
  return csApplicationRunner<Simple>::Run (argc, argv);
}
