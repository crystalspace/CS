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

#include "phystut.h"
#include "imesh/ragdoll.h"

CS_IMPLEMENT_APPLICATION

#define ODE_ID 1
#define BULLET_ID 2

#define CAMERA_BODY 1
#define CAMERA_FREE 2

//-----------------------------------------------------------------------------

// The global pointer to simple
Simple *simple;

Simple::Simple (iObjectRegistry* object_reg)
  : dynSysDebugger (object_reg)
{
  Simple::object_reg = object_reg;
  solver = 0;
  disable = false;
  do_bullet_debug = false;
  cameraMode = CAMERA_BODY;
  debugMode = false;
  allStatic = false;
  pauseDynamic = false;
  dynamicSpeed = 1.0f;
  rotX = rotY = 0.0f;
}

Simple::~Simple ()
{
}

void Simple::SetupFrame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();

  // Now rotate the camera according to keyboard state
  const float speed = elapsed_time / 1000.0;

  // Camera is controlled by a rigid body
  if (cameraMode == CAMERA_BODY)
  {
    if (kbd->GetKeyState (CSKEY_RIGHT))
      view->GetCamera()->GetTransform().RotateThis (CS_VEC_ROT_RIGHT, speed);
    if (kbd->GetKeyState (CSKEY_LEFT))
      view->GetCamera()->GetTransform().RotateThis (CS_VEC_ROT_LEFT, speed);
    if (kbd->GetKeyState (CSKEY_PGUP))
      view->GetCamera()->GetTransform().RotateThis (CS_VEC_TILT_UP, speed);
    if (kbd->GetKeyState (CSKEY_PGDN))
      view->GetCamera()->GetTransform().RotateThis (CS_VEC_TILT_DOWN, speed);
    if (kbd->GetKeyState (CSKEY_UP))
    {
      avatarbody->SetLinearVelocity (view->GetCamera()->GetTransform()
				     .GetT2O () * csVector3 (0, 0, 5));
    }
    if (kbd->GetKeyState (CSKEY_DOWN))
    {
      avatarbody->SetLinearVelocity (view->GetCamera()->GetTransform()
				     .GetT2O () * csVector3 (0, 0, -5));
    }
  }

  // Camera is free
  else
  {
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
  }

  if (!pauseDynamic)
    dyn->Step (speed / dynamicSpeed);

  if (cameraMode == CAMERA_BODY)
  {
    view->GetCamera()->GetTransform().SetOrigin(
				    avatarbody->GetTransform().GetOrigin());
    //avatar->GetMovable()->SetTransform(view->GetCamera()->GetTransform());
  }

  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
    return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();

  // Write FPS and other info..
  if(!g3d->BeginDraw (CSDRAW_2DGRAPHICS)) return;

  if (do_bullet_debug)
  {
    bullet_dynSys->DebugDraw (view);
  }

  WriteShadow( 10, 390, g2d->FindRGB (255, 150, 100),"Physics engine: %s", 
    phys_engine_name.GetData ());
  if( speed != 0.0f)
    WriteShadow( 10, 400, g2d->FindRGB (255, 150, 100),"FPS: %.2f",
	1.0f/speed);
  WriteShadow( 10, 410, g2d->FindRGB (255, 150, 100),"%d Objects",
	       dynSys->GetBodysCount ());

  // Write available keys
  DisplayKeys ();

  if (phys_engine_id == ODE_ID)
  {
    if(solver==0)
      WriteShadow( 10, 420, g2d->FindRGB (255, 150, 100),"Solver: WorldStep");
    else if(solver==1)
      WriteShadow( 10, 420, g2d->FindRGB (255, 150, 100),"Solver: StepFast");
    else if(solver==2)
      WriteShadow( 10, 420, g2d->FindRGB (255, 150, 100),"Solver: QuickStep");
  }

  if (disable)
    WriteShadow( 10, 430, g2d->FindRGB (255, 150, 100),"AutoDisable ON");
}

bool Simple::HandleEvent (iEvent& ev)
{
  if (ev.Name == Frame)
  {
    simple->SetupFrame ();
    return true;
  }
  else if (CS_IS_KEYBOARD_EVENT(object_reg, ev)) 
  {
    if (ev.Name == KeyboardDown)
    {
      if (csKeyEventHelper::GetCookedCode (&ev) == CSKEY_SPACE)
      {
	int primitiveCount = phys_engine_id == BULLET_ID ? 7 : 4;
	switch (rand() % primitiveCount)
	{
	case 0: CreateBox (); break;
	case 1: CreateSphere (); break;
	case 2: CreateMesh (); break;
	case 3: CreateJointed (); break;
	case 4: CreateCylinder (); break;
	case 5: CreateCapsule (); break;
	case 6: CreateRagdoll (); break;
	default: break;
	}
	return true;
      }
      else if (csKeyEventHelper::GetCookedCode (&ev) == 'b')
      {
	CreateBox ();
	return true;
      }
      else if (csKeyEventHelper::GetCookedCode (&ev) == 's')
      {
	CreateSphere ();
	return true;
      }
      else if (csKeyEventHelper::GetCookedCode (&ev) == 'c'
	       && phys_engine_id == BULLET_ID)
      {
	CreateCylinder ();
	return true;
      }
      else if (csKeyEventHelper::GetCookedCode (&ev) == 'a'
	       && phys_engine_id == BULLET_ID)
      {
	CreateCapsule ();
	return true;
      }
      else if (csKeyEventHelper::GetCookedCode (&ev) == 'm')
      {
	CreateMesh ();
	return true;
      }
      else if (csKeyEventHelper::GetCookedCode (&ev) == '*')
      {
	CreateStarCollider ();
	return true;
      }
      else if (csKeyEventHelper::GetCookedCode (&ev) == 'j')
      {
	CreateJointed ();
	return true;
      }
      else if (csKeyEventHelper::GetCookedCode (&ev) == 'h'
	       && phys_engine_id == BULLET_ID)
      {
	CreateChain ();
	return true;
      }
      else if (csKeyEventHelper::GetCookedCode (&ev) == 'r'
	       && phys_engine_id == BULLET_ID)
      {
	CreateRagdoll ();
	return true;
      }

      else if (csKeyEventHelper::GetCookedCode (&ev) == 'f')
      {
	// Toggle camera mode
	if (cameraMode == CAMERA_BODY)
	{
	  cameraMode = CAMERA_FREE;
	  printf ("Toggling camera to free mode\n");
	}
	else
	{
	  cameraMode = CAMERA_BODY;
	  printf ("Toggling camera to rigid body mode\n");
	}
	UpdateCameraMode ();
	return true;
      }

      else if (csKeyEventHelper::GetCookedCode (&ev) == 't')
      {
	// Toggle all bodies between dynamic and static
	allStatic = !allStatic;

	if (allStatic)
	  printf ("Toggling all bodies to static mode\n");
	else
	  printf ("Toggling all bodies to dynamic mode\n");

	for (int i = 0; i < dynSys->GetBodysCount (); i++)
	{
	  iRigidBody* body = dynSys->GetBody (i);
	  if (allStatic)
	    body->MakeStatic ();
	  else {
	    body->MakeDynamic ();
	    body->Enable ();
	  }
	}
	return true;
      }

      else if (csKeyEventHelper::GetCookedCode (&ev) == 'p')
      {
	// Toggle pause mode for dynamic simulation
	pauseDynamic = !pauseDynamic;
	if (pauseDynamic)
	  printf ("Dynamic simulation paused\n");
	else
	  printf ("Dynamic simulation resumed\n");
	return true;
      }

      else if (csKeyEventHelper::GetCookedCode (&ev) == 'o')
      {
	// Toggle speed of dynamic simulation
	if (dynamicSpeed - 1.0 < 0.00001)
	{
	  dynamicSpeed = 45.0;
	  printf ("Dynamic simulation slowed\n");
	}
	else
	{
	  dynamicSpeed = 1.0;
	  printf ("Dynamic simulation at normal speed\n");
	}
      }

      else if (csKeyEventHelper::GetCookedCode (&ev) == 'd')
      {
	// Toggle dynamic system visual debug mode
	debugMode = !debugMode;
	dynSysDebugger.SetDebugDisplayMode (debugMode);

	// Hide the last ragdoll mesh if any
	if (ragdollMesh)
	{
	  if (debugMode)
	    ragdollMesh->GetFlags ().Set (CS_ENTITY_INVISIBLEMESH);
	  else
	    ragdollMesh->GetFlags ().Reset (CS_ENTITY_INVISIBLEMESH);
	}

	return true;
      }

      else if (csKeyEventHelper::GetCookedCode (&ev) == '?'
	       && phys_engine_id != BULLET_ID)
      {
	// Toggle collision debug mode
	  do_bullet_debug = !do_bullet_debug;
      }

      else if (csKeyEventHelper::GetCookedCode (&ev) == 'g')
      { // Toggle gravity.
	dynSys->SetGravity (dynSys->GetGravity () == 0 ?
	  csVector3 (0,-7,0) : csVector3 (0));
	return true;
      }
      else if (csKeyEventHelper::GetCookedCode (&ev) == 'i')
      { // Toggle autodisable.
	dynSys->EnableAutoDisable (!dynSys->AutoDisableEnabled ());
	//dynSys->SetAutoDisableParams(1.5f,2.5f,6,0.0f);
	disable=dynSys->AutoDisableEnabled ();
	return true;
      }
      else if (csKeyEventHelper::GetCookedCode (&ev) == '1'
	       && phys_engine_id == ODE_ID)
      { // Toggle stepfast.
        csRef<iODEDynamicSystemState> osys = 
          scfQueryInterface<iODEDynamicSystemState> (dynSys);
	osys->EnableStepFast (0);
	solver=0;
	return true;
      }
      else if (csKeyEventHelper::GetCookedCode (&ev) == '2'
	       && phys_engine_id == ODE_ID)
      { // Toggle stepfast.
        csRef<iODEDynamicSystemState> osys = 
          scfQueryInterface<iODEDynamicSystemState> (dynSys);
	osys->EnableStepFast (1);
	solver=1;
	return true;
      }
      else if (csKeyEventHelper::GetCookedCode (&ev) == '3'
	       && phys_engine_id == ODE_ID)
      { // Toggle quickstep.
        csRef<iODEDynamicSystemState> osys = 
          scfQueryInterface<iODEDynamicSystemState> (dynSys);
	osys->EnableQuickStep (1);
	solver=2;
	return true;
      }
      else if (csKeyEventHelper::GetCookedCode (&ev) == CSKEY_ESC)
      {
	csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (object_reg));
	if (q) q->GetEventOutlet()->Broadcast (csevQuit (object_reg));
	return true;
      }
    }
    else if ((ev.Name == KeyboardUp)
	     && ((csKeyEventHelper::GetCookedCode (&ev) == CSKEY_DOWN) 
	      || (csKeyEventHelper::GetCookedCode (&ev) == CSKEY_UP)))
    {
      avatarbody->SetLinearVelocity(csVector3 (0, 0, 0));
      avatarbody->SetAngularVelocity (csVector3 (0, 0, 0));
    }
  }

  return false;
}

bool Simple::SimpleEventHandler (iEvent& ev)
{
  return simple ? simple->HandleEvent (ev) : false;
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
    CS_REQUEST_PLUGIN ("crystalspace.mesh.object.animesh.body", iBodyManager),
    CS_REQUEST_PLUGIN ("crystalspace.mesh.object.animesh.controllers.ragdoll", iRagdollManager),
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
  CS_INITIALIZE_EVENT_SHORTCUTS (object_reg);

  KeyboardDown = csevKeyboardDown (object_reg);
  KeyboardUp = csevKeyboardUp (object_reg);

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    csPrintf ("Usage: phystut [OPTIONS]\n");
    csPrintf ("Physics tutorial for crystalspace\n\n");
    csPrintf ("Options for phystut:\n");
    csPrintf ("  -phys_engine:      specify which physics plugin to use\n");
    csCommandLineHelper::Help (object_reg);
    return false;
  }

  // Checking for choosen engine
  csRef<iCommandLineParser> clp = csQueryRegistry<iCommandLineParser> (object_reg);
  phys_engine_name = clp->GetOption ("phys_engine");
  if (phys_engine_name == "ode")
  {
    phys_engine_id = ODE_ID;
    csRef<iPluginManager> plugmgr = 
      csQueryRegistry<iPluginManager> (object_reg);
    dyn = csLoadPlugin<iDynamics> (plugmgr, "crystalspace.dynamics.ode");
  }
  else 
  {
    phys_engine_name = "bullet";
    phys_engine_id = BULLET_ID;
    csRef<iPluginManager> plugmgr = 
      csQueryRegistry<iPluginManager> (object_reg);
    dyn = csLoadPlugin<iDynamics> (plugmgr, "crystalspace.dynamics.bullet");
  }
  if (!dyn)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.application.phystut",
      "No iDynamics plugin!");
    return false;
  }

  // The virtual clock.
  vc = csQueryRegistry<iVirtualClock> (object_reg);
  if (vc == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.application.phystut",
      "Can't find the virtual clock!");
    return false;
  }

  // Find the pointer to engine plugin
  engine = csQueryRegistry<iEngine> (object_reg);
  if (engine == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.application.phystut",
      "No iEngine plugin!");
    return false;
  }

  loader = csQueryRegistry<iLoader> (object_reg);
  if (loader == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.application.phystut",
      "No iLoader plugin!");
    return false;
  }

  g3d = csQueryRegistry<iGraphics3D> (object_reg);
  if (g3d == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.application.phystut",
      "No iGraphics3D plugin!");
    return false;
  }

  g2d = csQueryRegistry<iGraphics2D> (object_reg);
  if (g2d == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.application.phystut",
      "No iGraphics2D plugin!");
    return false;
  }

  kbd = csQueryRegistry<iKeyboardDriver> (object_reg);
  if (kbd == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.application.phystut",
      "No iKeyboardDriver plugin!");
    return false;
  }

  bodyManager = csQueryRegistry<iBodyManager> (object_reg);
  if (bodyManager == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.application.avatartest",
      "No iBodyManager plugin!");
    return false;
  }

  ragdollManager = csQueryRegistry<iRagdollManager> (object_reg);
  if (ragdollManager == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.application.avatartest",
      "No iBodyRagdollManager plugin!");
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

  printer.AttachNew (new FramePrinter (object_reg));

  csRef<iFontServer> fs = g3d->GetDriver2D()->GetFontServer ();
  if (fs)
    courierFont = fs->LoadFont (CSFONT_COURIER);
  else
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.application.phystut",
      "Error getting FontServer!");
    return false;
  };

  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.application.phystut",
      "Error loading 'stone4' texture!");
    return false;
  }
  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");

  room = engine->CreateSector ("room");
  dynSysDebugger.SetDebugSector (room);

  // First we make a primitive for our geometry.
  using namespace CS::Geometry;
  DensityTextureMapper mapper (0.3f);
  TesselatedBox box (csVector3 (-5, -5, -5), csVector3 (5, 5, 5));
  box.SetLevel (3);
  box.SetMapper (&mapper);
  box.SetFlags (Primitives::CS_PRIMBOX_INSIDE);

  // Now we make a factory and a mesh at once.
  csRef<iMeshWrapper> walls = GeneralMeshBuilder::CreateFactoryAndMesh (
      engine, room, "walls", "walls_factory", &box);
  walls->GetMeshObject ()->SetMaterialWrapper (tm);

  // Creating the background (usefull when the camera is free and is moved 
  // outside of the room)

  // First we make a primitive for our geometry.
  CS::Geometry::DensityTextureMapper bgMapper (0.3f);
  CS::Geometry::TesselatedBox bgBox (csVector3 (-50000), csVector3 (50000));
  bgBox.SetMapper(&bgMapper);
  bgBox.SetFlags(CS::Geometry::Primitives::CS_PRIMBOX_INSIDE);
  
  // Now we make a factory and a mesh at once.
  csRef<iMeshWrapper> background =
    CS::Geometry::GeneralMeshBuilder::CreateFactoryAndMesh(engine, room,
				   "background", "background_factory", &bgBox);

  csRef<iMaterialWrapper> bgMaterial = ColoredTexture::CreateColoredMaterial
    ("background", csColor (0.898f), object_reg);
  background->GetMeshObject()->SetMaterialWrapper(bgMaterial);

  // Creating lights
  csRef<iLight> light;
  iLightList* ll = room->GetLights ();

  // This light is for the background
  light = engine->CreateLight(0, csVector3(10), 100000, csColor (1));
  ll->Add (light);

  // Other lights
  light = engine->CreateLight (0, csVector3 (3, 0, 0), 8,
    csColor (1, 0, 0));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (-3, 0,  0), 8,
    csColor (0, 0, 1));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (0, 0, 3), 8,
    csColor (0, 1, 0));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (0, -3, 0), 8,
    csColor (1, 1, 0));
  ll->Add (light);

  engine->Prepare ();

  using namespace CS::Lighting;
  SimpleStaticLighter::ShineLights (room, engine, 4);

  view = csPtr<iView> (new csView (engine, g3d));
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, -3));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  iTextureWrapper* txt = loader->LoadTexture ("spark",
    "/lib/std/spark.png");
  if (txt == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.application.phystut",
      "Error loading texture!");
    return false;
  }

  // Load the box mesh factory.
  boxFact = loader->LoadMeshObjectFactory ("/lib/std/sprite1");
  if (boxFact == 0)
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

  // Load the mesh factory.
  meshFact = loader->LoadMeshObjectFactory ("/varia/physmesh");
  if (meshFact == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.application.phystut",
      "Error loading mesh object factory!");
    return false;
  }

  // Create the dynamic system.
  dynSys = dyn->CreateSystem ();
  if (dynSys == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.application.phystut",
      "Error creating dynamic system!");
    return false;
  }

  dynSys->SetGravity (csVector3 (0,-7,0));

  dynSys->SetRollingDampener(.995f);

  dynSysDebugger.SetDynamicSystem (dynSys);

  if (phys_engine_id == ODE_ID)
  {
    csRef<iODEDynamicSystemState> osys= 
      scfQueryInterface<iODEDynamicSystemState> (dynSys);
    osys->SetContactMaxCorrectingVel (.1f);
    osys->SetContactSurfaceLayer (.0001f);
  }
  else
  {
    bullet_dynSys = scfQueryInterface<iBulletDynamicSystem> (dynSys);
  }
  CreateWalls (csVector3 (5));

  // Init the camera
  UpdateCameraMode ();

  // Load the animesh & ragdoll at startup
  if (phys_engine_id == BULLET_ID)
    LoadRagdoll ();

  return true;
}

void Simple::Shutdown ()
{
  printer.Invalidate ();
}

void Simple::UpdateCameraMode ()
{
  if (cameraMode == CAMERA_BODY)
  {
    // Use the camera transform.
    const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

    // Create the avatar.
    avatar = engine->CreateMeshWrapper (boxFact, "box", room);

    // Create a body and attach the mesh.
    avatarbody = dynSys->CreateBody ();
    avatarbody->SetProperties (1, csVector3 (0), csMatrix3 ());
    avatarbody->SetPosition (tc.GetOrigin ());
    avatarbody->AttachMesh (avatar);

    // Create and attach a box collider.
    // const csMatrix3 tmm;
    // const csVector3 tvv (0);
    // csOrthoTransform tt (tmm, tvv);
    // csVector3 size (0.4f, 0.8f, 0.4f); // This should be same size as mesh.
    // avatarbody->AttachColliderBox (size, tt, 10, 1, 0.8f);
    avatarbody->AttachColliderSphere (0.8f, csVector3 (0), 10, 1, 0.8f);
  }

  else
  {
    dynSys->RemoveBody (avatarbody);
    engine->WantToDie (avatar);
    avatar = 0;

    // TODO: update RotX, rotY
  }
}

iRigidBody* Simple::CreateBox ()
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the mesh.
  csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (boxFact, "box", room));

  // Create a body and attach the mesh.
  csRef<iRigidBody> rb = dynSys->CreateBody ();
  rb->SetProperties (1, csVector3 (0), csMatrix3 ());
  rb->SetPosition (tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 1));
  rb->AttachMesh (mesh);

  // Create and attach a box collider.
  const csMatrix3 tm;
  const csVector3 tv (0);
  csOrthoTransform t (tm, tv);
  csVector3 size (0.4f, 0.8f, 0.4f); // This should be the same size as the mesh
  rb->AttachColliderBox (size, t, 10, 1, 0.8f);

  // Fling the body.
  rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 5));
  rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 0, 0));

  return rb;
}

bool Simple::CreateStarCollider ()
{
  csRef<iMeshFactoryWrapper> starFact;
  starFact = engine->FindMeshFactory ("genstar");
  if (!starFact)
  {
    loader->Load ("/lib/std/star.xml");
    starFact = engine->FindMeshFactory ("genstar");
    if (!starFact)
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.avatartest",
        "Error loading 'star.xml'!");
      return false;
    }
  }

  // Use the camera transform.
  csOrthoTransform tc = view->GetCamera ()->GetTransform ();
  tc.SetOrigin (tc.This2Other (csVector3 (0, 0, 3)));

  // Create the mesh.
  csRef<iMeshWrapper> star = engine->CreateMeshWrapper (starFact, "star",
      room);
  star->GetMovable ()->SetTransform (tc);
  star->GetMovable ()->UpdateMove ();

  bool staticCollider = true;
  if (staticCollider)
  {
    csRef<iDynamicsSystemCollider> collider = dynSys->CreateCollider ();
    // TODO: star is not convex
    collider->CreateConvexMeshGeometry (star);
    //collider->CreateMeshGeometry (star);
    collider->SetTransform (tc);
  }

  else
  {
    csRef<iRigidBody> rb = dynSys->CreateBody ();
    rb->SetProperties (1, csVector3 (0), csMatrix3 ());
    rb->SetPosition (tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 2));

    const csMatrix3 tm;
    const csVector3 tv (0);
    csOrthoTransform t (tm, tv);
    //rb->AttachColliderMesh (star, t, 10, 1, 0.8f);
    rb->AttachColliderConvexMesh (star, t, 10, 1, 0.8f);

    rb->AttachMesh (star);

    // Fling the body.
    rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 5));
    rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 0, 0));
  }

  return true;
}

iRigidBody* Simple::CreateMesh ()
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the mesh.
  csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (meshFact, "mesh", room));

  // Create a body and attach the mesh.
  csRef<iRigidBody> rb = dynSys->CreateBody ();
  rb->SetProperties (1, csVector3 (0), csMatrix3 ());
  rb->SetPosition (tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 2));
  rb->AttachMesh (mesh);

  // Create and attach a trimesh collider.
  const csMatrix3 tm;
  const csVector3 tv (0);
  csOrthoTransform t (tm, tv);

  if (!rb->AttachColliderMesh (mesh, t, 10, 1, 0.8f))
  {
    // If dynamic collider meshes are not supported
    // we use a cylinder instead.
    t.RotateThis (csVector3 (1, 0, 0), PI / 2.0f);
    rb->AttachColliderCylinder (0.2f, 1, t, 10, 1, 0.8f);
  }

  // Fling the body.
  rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 5));
  rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 0, 0));

  return rb;
}

iRigidBody* Simple::CreateSphere ()
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the ball mesh factory.
  csRef<iMeshFactoryWrapper> ballFact = engine->CreateMeshFactory(
  	"crystalspace.mesh.object.genmesh", "ballFact");
  if (ballFact == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.application.phystut",
      "Error creating mesh object factory!");
    return 0;
  }

  csRef<iGeneralFactoryState> gmstate = scfQueryInterface<
    iGeneralFactoryState> (ballFact->GetMeshObjectFactory ());
  const float r (rand()%5/10. + .2);
  csVector3 radius (r, r, r);
  csEllipsoid ellips (csVector3 (0), radius);
  gmstate->GenerateSphere (ellips, 16);

  // We do a hardtransform here to make sure our sphere has an artificial
  // offset. That way we can test if the physics engine supports that.
  csMatrix3 m;
  csVector3 artificialOffset (0, .5, 0);
  if (phys_engine_id == ODE_ID)
    artificialOffset.Set (0, 0, 0);
  csReversibleTransform t = csReversibleTransform (m, artificialOffset);
  ballFact->HardTransform (t);

  // Create the mesh.
  csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (ballFact, "ball", room));

  iMaterialWrapper* mat = engine->GetMaterialList ()->FindByName ("spark");
  mesh->GetMeshObject ()->SetMaterialWrapper (mat);

  // Create a body and attach the mesh.
  csRef<iRigidBody> rb = dynSys->CreateBody ();
  rb->SetProperties (r, csVector3 (0), csMatrix3 ());
  rb->SetPosition (tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 1)
		   - artificialOffset);
  rb->AttachMesh (mesh);

  // Create and attach a sphere collider.
  rb->AttachColliderSphere (r, artificialOffset, 10, 1, 0.8f);

  // Fling the body.
  rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 6));
  rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 0, 0));

  return rb;
}

iRigidBody* Simple::CreateCylinder ()
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the cylinder mesh factory.
  csRef<iMeshFactoryWrapper> cylinderFact = engine->CreateMeshFactory(
  	"crystalspace.mesh.object.genmesh", "cylinderFact");
  if (cylinderFact == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.application.phystut",
      "Error creating mesh object factory!");
    return 0;
  }

  csRef<iGeneralFactoryState> gmstate = scfQueryInterface<
    iGeneralFactoryState> (cylinderFact->GetMeshObjectFactory ());
  const float radius (rand() % 10 / 50. + .2);
  const float length (rand() % 3 / 50. + .7);
  gmstate->GenerateCylinder (length, radius, 10);

  // We do a hardtransform here to make sure our cylinder has an artificial
  // offset. That way we can test if the physics engine supports that.
  csVector3 artificialOffset (3, 3, 3);
  csReversibleTransform hardTransform (csYRotMatrix3 (PI/2.0), artificialOffset);
  cylinderFact->HardTransform (hardTransform);

  // Create the mesh.
  csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (
					   cylinderFact, "cylinder", room));

  iMaterialWrapper* mat = engine->GetMaterialList ()->FindByName ("spark");
  mesh->GetMeshObject ()->SetMaterialWrapper (mat);

  // Create a body and attach the mesh.
  csRef<iRigidBody> rb = dynSys->CreateBody ();
  rb->SetProperties (radius, csVector3 (0), csMatrix3 ());
  rb->AttachMesh (mesh);

  // Create and attach a cylinder collider.
  csMatrix3 m;
  csReversibleTransform t = csReversibleTransform (m, artificialOffset);
  rb->AttachColliderCylinder (length, radius, t, 10, 1, 0.8f);
  rb->SetPosition (tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 1)
		   - artificialOffset);
  rb->SetOrientation (csXRotMatrix3 (PI / 5.0));
  //csOrthoTransform bodyTransform (csMatrix3 (), tc.GetOrigin ()
  //         + tc.GetT2O () * csVector3 (0, 0, 1) - artificialOffset);
  //rb->SetTransform (bodyTransform);

  // Fling the body.
  rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 6));
  rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 0, 0));

  return rb;
}

iRigidBody* Simple::CreateCapsule ()
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the ball mesh factory.
  csRef<iMeshFactoryWrapper> capsuleFact = engine->CreateMeshFactory(
  	"crystalspace.mesh.object.genmesh", "capsuleFact");
  if (capsuleFact == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.application.phystut",
      "Error creating mesh object factory!");
    return 0;
  }

  csRef<iGeneralFactoryState> gmstate = scfQueryInterface<
    iGeneralFactoryState> (capsuleFact->GetMeshObjectFactory ());
  const float radius (rand() % 10 / 50. + .2);
  const float length (rand() % 3 / 50. + .7);
  gmstate->GenerateCapsule (length, radius, 10);
  capsuleFact->HardTransform (
        csReversibleTransform (csYRotMatrix3 (PI/2), csVector3 (0)));

  // Create the mesh.
  csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (
				            capsuleFact, "capsule", room));
  iMaterialWrapper* mat = engine->GetMaterialList ()->FindByName ("spark");
  mesh->GetMeshObject ()->SetMaterialWrapper (mat);

  // Create a body and attach the mesh.
  csRef<iRigidBody> rb = dynSys->CreateBody ();
  rb->SetProperties (radius, csVector3 (0), csMatrix3 ());
  rb->AttachMesh (mesh);

  // Create and attach a sphere collider.
  csOrthoTransform t;
  rb->AttachColliderCapsule (length, radius, t, 10, 1, 0.8f);
  rb->SetPosition (tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 1));

  // Fling the body.
  rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 6));
  rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 0, 0));

  return rb;
}

iJoint* Simple::CreateJointed ()
{
  // Create and position objects.
  iRigidBody* rb1 = CreateBox();
  rb1->SetPosition (rb1->GetPosition () +
    rb1->GetOrientation () * csVector3 (-.5, 0, 0));
  iRigidBody* rb2 = CreateSphere();
  rb2->SetPosition (rb2->GetPosition () +
    rb2->GetOrientation () * csVector3 (.5, 0, 0));

  // Create a joint and attach bodies.
  csRef<iJoint> joint = dynSys->CreateJoint ();
  joint->Attach (rb1, rb2);

  // Constrain translation.
  joint->SetMinimumDistance (csVector3 (-1, -1, -1), false);
  joint->SetMaximumDistance (csVector3 (1, 1, 1), false);
  joint->SetTransConstraints (true, true, true, false);

  // Constrain rotation.
  joint->SetMinimumAngle (csVector3 (-PI/4.0, -PI/6.0, -PI/6.0), false);
  joint->SetMaximumAngle (csVector3 (PI/4.0, PI/6.0, PI/6.0), false);
  joint->SetRotConstraints (false, false, false, false);

  joint->RebuildJoint ();

  return joint;
}

void ConstraintJoint (iJoint* joint)
{
  // Constrain translation.
  joint->SetMinimumDistance (csVector3 (-1, -1, -1), false);
  joint->SetMaximumDistance (csVector3 (1, 1, 1), false);
  joint->SetTransConstraints (true, true, true, false);

  // Constrain rotation.
  joint->SetMinimumAngle (csVector3 (-PI/4.0, -PI/6.0, -PI/6.0), false);
  joint->SetMaximumAngle (csVector3 (PI/4.0, PI/6.0, PI/6.0), false);
  joint->SetRotConstraints (false, false, false, false);
}

void Simple::CreateChain ()
{
  iRigidBody* rb1 = CreateBox();
  csVector3 initPos = rb1->GetPosition () + csVector3 (0.0f, 5.0f, 0.0f);
  rb1->MakeStatic ();
  rb1->SetPosition (initPos);

  csVector3 offset (0.0f, 1.3f, 0.0f);

  iRigidBody* rb2 = CreateCapsule();
  rb2->SetLinearVelocity (csVector3 (0.0f));
  rb2->SetAngularVelocity (csVector3 (0.0f));
  rb2->SetPosition (initPos - offset);
  rb2->SetOrientation (csXRotMatrix3 (PI / 2.0f));

  iRigidBody* rb3 = CreateBox();
  rb3->SetLinearVelocity (csVector3 (0.0f));
  rb3->SetAngularVelocity (csVector3 (0.0f));
  rb3->SetPosition (initPos - 2.0f * offset);

  iRigidBody* rb4 = CreateCapsule();
  rb4->SetLinearVelocity (csVector3 (0.0f));
  rb4->SetAngularVelocity (csVector3 (0.0f));
  rb4->SetPosition (initPos - 3.0f * offset);
  rb4->SetOrientation (csXRotMatrix3 (PI / 2.0f));

  iRigidBody* rb5 = CreateBox();
  rb5->SetLinearVelocity (csVector3 (0.0f));
  rb5->SetAngularVelocity (csVector3 (0.0f));
  rb5->SetPosition (initPos - 4.0f * offset);

  // Create joints and attach bodies.
  csRef<iJoint> joint = dynSys->CreateJoint ();
  joint->Attach (rb1, rb2, false);
  ConstraintJoint (joint);
  joint->RebuildJoint ();

  joint = dynSys->CreateJoint ();
  joint->Attach (rb2, rb3, false);
  ConstraintJoint (joint);
  joint->RebuildJoint ();

  joint = dynSys->CreateJoint ();
  joint->Attach (rb3, rb4, false);
  ConstraintJoint (joint);
  joint->RebuildJoint ();

  joint = dynSys->CreateJoint ();
  joint->Attach (rb4, rb5, false);
  ConstraintJoint (joint);
  joint->RebuildJoint ();
}

void Simple::LoadRagdoll ()
{

  // Load animesh factory
  csLoadResult rc = loader->Load ("/lib/frankie/frankie.xml");
  if (!rc.success)
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	      "crystalspace.application.phystut",
	      "Can't load frankie!");
    csRef<iMeshFactoryWrapper> meshfact = engine->FindMeshFactory ("franky_frankie");

  if (!meshfact)
    return;

  csRef<iAnimatedMeshFactory> animeshFactory = scfQueryInterface<iAnimatedMeshFactory>
    (meshfact->GetMeshObjectFactory ());

  if (!animeshFactory)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	      "crystalspace.application.phystut",
	      "Can't find frankie's animesh factory!");
    return;
  }

  // Define animesh's physical properties

  // Create bones (this should be made through a loader)
  iBodySkeleton* skeletonFactory = bodyManager->CreateBodySkeleton ("franky_body",
								    animeshFactory);
  iBodyBone* bone_Main = skeletonFactory->CreateBodyBone
    (animeshFactory->GetSkeletonFactory ()->FindBone ("Frankie_Main"));
  iBodyBone* bone_Spin = skeletonFactory->CreateBodyBone
    (animeshFactory->GetSkeletonFactory ()->FindBone ("CTRL_Spin"));
  iBodyBone* bone_SpinM = skeletonFactory->CreateBodyBone
    (animeshFactory->GetSkeletonFactory ()->FindBone ("CTRL_Spin.M"));
  iBodyBone* bone_RibCage = skeletonFactory->CreateBodyBone
    (animeshFactory->GetSkeletonFactory ()->FindBone ("CTRL_RibCage"));
  iBodyBone* bone_Head = skeletonFactory->CreateBodyBone
    (animeshFactory->GetSkeletonFactory ()->FindBone ("CTRL_Head"));
  iBodyBone* bone_Pelvis = skeletonFactory->CreateBodyBone
    (animeshFactory->GetSkeletonFactory ()->FindBone ("CTRL_Pelvis"));
  iBodyBone* bone1 = skeletonFactory->CreateBodyBone
    (animeshFactory->GetSkeletonFactory ()->FindBone ("Tail_1"));
  iBodyBone* bone2 = skeletonFactory->CreateBodyBone
    (animeshFactory->GetSkeletonFactory ()->FindBone ("Tail_2"));
  iBodyBone* bone3 = skeletonFactory->CreateBodyBone
    (animeshFactory->GetSkeletonFactory ()->FindBone ("Tail_3"));
  iBodyBone* bone4 = skeletonFactory->CreateBodyBone
    (animeshFactory->GetSkeletonFactory ()->FindBone ("Tail_4"));
  iBodyBone* bone5 = skeletonFactory->CreateBodyBone
    (animeshFactory->GetSkeletonFactory ()->FindBone ("Tail_5"));
  iBodyBone* bone6 = skeletonFactory->CreateBodyBone
    (animeshFactory->GetSkeletonFactory ()->FindBone ("Tail_6"));
  iBodyBone* bone7 = skeletonFactory->CreateBodyBone
    (animeshFactory->GetSkeletonFactory ()->FindBone ("Tail_7"));
  iBodyBone* bone8 = skeletonFactory->CreateBodyBone
    (animeshFactory->GetSkeletonFactory ()->FindBone ("Tail_8"));

  // Define bone's properties
  iBodyBoneCollider* collider;
  iBodyBoneJoint* joint;
  csVector3 boxSize (0.03f);
  float density = 1.0f;

  collider = bone_Main->CreateBoneCollider ();
  collider->CreateSphereGeometry (csSphere (csVector3 (0.0f, 0.05f, 0.0f), 0.05f));
  collider->SetFriction (10.0f);
  collider->SetDensity (density);
  collider->SetElasticity (0.8f);
  collider->SetSoftness (0.01f);
  // No joint for root bone

  collider = bone_Spin->CreateBoneCollider ();
  collider->CreateCylinderGeometry (0.02f, 0.1f);
  csOrthoTransform spinTransform (csXRotMatrix3 (PI / 7.0f),
				  csVector3 (0, 0.065f, 0));
  collider->SetTransform (spinTransform);
  collider->SetFriction (10.0f);
  collider->SetDensity (density);
  collider->SetElasticity (0.8f);
  collider->SetSoftness (0.01f);
  joint = bone_Spin->CreateBoneJoint ();
  bone_Spin->GetBoneJoint ()->SetBounce (csVector3 (0.2f, 0.2f, 0.2f));
  bone_Spin->GetBoneJoint ()->SetTransConstraints (true, true, true);
  bone_Spin->GetBoneJoint ()->SetRotConstraints (false, false, false);
  bone_Spin->GetBoneJoint ()->SetMinimumAngle (csVector3 (-0.175f));
  bone_Spin->GetBoneJoint ()->SetMaximumAngle (csVector3 (0.175f));

  collider = bone_SpinM->CreateBoneCollider ();
  collider->CreateCylinderGeometry (0.07f, 0.1f);
  csOrthoTransform spinMTransform (csMatrix3 (), csVector3 (0.0f, 0.095f, 0.0f));
  collider->SetTransform (spinMTransform);
  collider->SetFriction (10.0f);
  collider->SetDensity (density);
  collider->SetElasticity (0.8f);
  collider->SetSoftness (0.01f);
  joint = bone_SpinM->CreateBoneJoint ();
  bone_SpinM->GetBoneJoint ()->SetBounce (csVector3 (0.2f, 0.2f, 0.2f));
  bone_SpinM->GetBoneJoint ()->SetTransConstraints (true, true, true);
  bone_SpinM->GetBoneJoint ()->SetRotConstraints (false, false, false);
  bone_SpinM->GetBoneJoint ()->SetMinimumAngle (csVector3 (-0.175f));
  bone_SpinM->GetBoneJoint ()->SetMaximumAngle (csVector3 (0.175f));

  collider = bone_RibCage->CreateBoneCollider ();
  collider->CreateCylinderGeometry (0.07f, 0.1f);
  csOrthoTransform ribCageTransform (csMatrix3 (), csVector3 (0.0f, 0.105f, 0.0f));
  collider->SetTransform (ribCageTransform);
  collider->SetFriction (10.0f);
  collider->SetDensity (density);
  collider->SetElasticity (0.8f);
  collider->SetSoftness (0.01f);
  joint = bone_RibCage->CreateBoneJoint ();
  bone_RibCage->GetBoneJoint ()->SetBounce (csVector3 (0.2f, 0.2f, 0.2f));
  bone_RibCage->GetBoneJoint ()->SetTransConstraints (true, true, true);
  bone_RibCage->GetBoneJoint ()->SetRotConstraints (false, false, false);
  bone_RibCage->GetBoneJoint ()->SetMinimumAngle (csVector3 (-0.175f));
  bone_RibCage->GetBoneJoint ()->SetMaximumAngle (csVector3 (0.175f));

  collider = bone_Head->CreateBoneCollider ();
  collider->CreateSphereGeometry (csSphere (csVector3 (0.0f, 0.0f, 0.08f), 0.12f));
  collider->SetFriction (10.0f);
  collider->SetDensity (density);
  collider->SetElasticity (0.8f);
  collider->SetSoftness (0.01f);
  joint = bone_Head->CreateBoneJoint ();
  bone_Head->GetBoneJoint ()->SetBounce (csVector3 (0.2f, 0.2f, 0.2f));
  bone_Head->GetBoneJoint ()->SetTransConstraints (true, true, true);
  bone_Head->GetBoneJoint ()->SetRotConstraints (false, false, false);
  bone_Head->GetBoneJoint ()->SetMinimumAngle (csVector3 (-0.175f));
  bone_Head->GetBoneJoint ()->SetMaximumAngle (csVector3 (0.175f));

  collider = bone_Pelvis->CreateBoneCollider ();
  collider->CreateSphereGeometry (csSphere (csVector3 (0.0f, 0.07f, 0.05f), 0.05f));
  collider->SetFriction (10.0f);
  collider->SetDensity (density);
  collider->SetElasticity (0.8f);
  collider->SetSoftness (0.01f);
  joint = bone_Pelvis->CreateBoneJoint ();
  bone_Pelvis->GetBoneJoint ()->SetBounce (csVector3 (0.2f, 0.2f, 0.2f));
  bone_Pelvis->GetBoneJoint ()->SetTransConstraints (true, true, true);
  bone_Pelvis->GetBoneJoint ()->SetRotConstraints (true, true, true);

  float capsuleLength = 0.01f;
  float capsuleRadius = 0.02f;
  csOrthoTransform boneOffset;
  boneOffset.Translate (csVector3 (capsuleLength / 2.0f, 0.0f, 0.0f));
  csVector3 minimumAngle (-0.275f);
  csVector3 maximumAngle (0.275f);

  collider = bone1->CreateBoneCollider ();
  collider->CreateCapsuleGeometry (capsuleLength, capsuleRadius);
  collider->SetFriction (10.0f);
  collider->SetDensity (density);
  collider->SetElasticity (0.8f);
  collider->SetSoftness (0.01f);
  joint = bone1->CreateBoneJoint ();
  bone1->GetBoneJoint ()->SetBounce (csVector3 (0.2f, 0.2f, 0.2f));
  bone1->GetBoneJoint ()->SetTransConstraints (true, true, true);
  bone1->GetBoneJoint ()->SetRotConstraints (false, false, false);
  bone1->GetBoneJoint ()->SetMinimumAngle (minimumAngle);
  bone1->GetBoneJoint ()->SetMaximumAngle (maximumAngle);

  collider = bone2->CreateBoneCollider ();
  collider->CreateCapsuleGeometry (capsuleLength, capsuleRadius);
  collider->SetFriction (10.0f);
  collider->SetDensity (density);
  collider->SetElasticity (0.8f);
  collider->SetSoftness (0.01f);
  joint = bone2->CreateBoneJoint ();
  bone2->GetBoneJoint ()->SetBounce (csVector3 (0.2f, 0.2f, 0.2f));
  bone2->GetBoneJoint ()->SetTransConstraints (true, true, true);
  bone2->GetBoneJoint ()->SetRotConstraints (false, false, false);
  bone2->GetBoneJoint ()->SetMinimumAngle (minimumAngle);
  bone2->GetBoneJoint ()->SetMaximumAngle (maximumAngle);

  collider = bone3->CreateBoneCollider ();
  collider->CreateCapsuleGeometry (capsuleLength, capsuleRadius);
  collider->SetFriction (10.0f);
  collider->SetDensity (density);
  collider->SetElasticity (0.8f);
  collider->SetSoftness (0.01f);
  joint = bone3->CreateBoneJoint ();
  bone3->GetBoneJoint ()->SetBounce (csVector3 (0.2f, 0.2f, 0.2f));
  bone3->GetBoneJoint ()->SetTransConstraints (true, true, true);
  bone3->GetBoneJoint ()->SetRotConstraints (false, false, false);
  bone3->GetBoneJoint ()->SetMinimumAngle (minimumAngle);
  bone3->GetBoneJoint ()->SetMaximumAngle (maximumAngle);

  collider = bone4->CreateBoneCollider ();
  collider->CreateCapsuleGeometry (capsuleLength, capsuleRadius);
  collider->SetFriction (10.0f);
  collider->SetDensity (density);
  collider->SetElasticity (0.8f);
  collider->SetSoftness (0.01f);
  joint = bone4->CreateBoneJoint ();
  bone4->GetBoneJoint ()->SetBounce (csVector3 (0.2f, 0.2f, 0.2f));
  bone4->GetBoneJoint ()->SetTransConstraints (true, true, true);
  bone4->GetBoneJoint ()->SetRotConstraints (false, false, false);
  bone4->GetBoneJoint ()->SetMinimumAngle (minimumAngle);
  bone4->GetBoneJoint ()->SetMaximumAngle (maximumAngle);

  collider = bone5->CreateBoneCollider ();
  collider->CreateCapsuleGeometry (capsuleLength, capsuleRadius);
  collider->SetFriction (10.0f);
  collider->SetDensity (density);
  collider->SetElasticity (0.8f);
  collider->SetSoftness (0.01f);
  joint = bone5->CreateBoneJoint ();
  bone5->GetBoneJoint ()->SetBounce (csVector3 (0.2f, 0.2f, 0.2f));
  bone5->GetBoneJoint ()->SetTransConstraints (true, true, true);
  bone5->GetBoneJoint ()->SetRotConstraints (false, false, false);
  bone5->GetBoneJoint ()->SetMinimumAngle (minimumAngle);
  bone5->GetBoneJoint ()->SetMaximumAngle (maximumAngle);

  collider = bone6->CreateBoneCollider ();
  collider->CreateCapsuleGeometry (capsuleLength, capsuleRadius);
  collider->SetFriction (10.0f);
  collider->SetDensity (density);
  collider->SetElasticity (0.8f);
  collider->SetSoftness (0.01f);
  joint = bone6->CreateBoneJoint ();
  bone6->GetBoneJoint ()->SetBounce (csVector3 (0.2f, 0.2f, 0.2f));
  bone6->GetBoneJoint ()->SetTransConstraints (true, true, true);
  bone6->GetBoneJoint ()->SetRotConstraints (false, false, false);
  bone6->GetBoneJoint ()->SetMinimumAngle (minimumAngle);
  bone6->GetBoneJoint ()->SetMaximumAngle (maximumAngle);

  collider = bone7->CreateBoneCollider ();
  collider->CreateCapsuleGeometry (capsuleLength, capsuleRadius);
  collider->SetFriction (10.0f);
  collider->SetDensity (density);
  collider->SetElasticity (0.8f);
  collider->SetSoftness (0.01f);
  joint = bone7->CreateBoneJoint ();
  bone7->GetBoneJoint ()->SetBounce (csVector3 (0.2f, 0.2f, 0.2f));
  bone7->GetBoneJoint ()->SetTransConstraints (true, true, true);
  bone7->GetBoneJoint ()->SetRotConstraints (false, false, false);
  bone7->GetBoneJoint ()->SetMinimumAngle (minimumAngle);
  bone7->GetBoneJoint ()->SetMaximumAngle (maximumAngle);

  collider = bone8->CreateBoneCollider ();
  collider->CreateCapsuleGeometry (capsuleLength, capsuleRadius);
  collider->SetTransform (boneOffset);
  collider->SetFriction (10.0f);
  collider->SetDensity (density);
  collider->SetElasticity (0.8f);
  collider->SetSoftness (0.01f);
  joint = bone8->CreateBoneJoint ();
  bone8->GetBoneJoint ()->SetBounce (csVector3 (0.2f, 0.2f, 0.2f));
  bone8->GetBoneJoint ()->SetTransConstraints (true, true, true);
  bone8->GetBoneJoint ()->SetRotConstraints (false, false, false);
  bone8->GetBoneJoint ()->SetMinimumAngle (minimumAngle);
  bone8->GetBoneJoint ()->SetMaximumAngle (maximumAngle);

  // Create bone chain
  iBodyChain* chain6 = skeletonFactory->CreateBodyChain
    ("chain6", bone_Main->GetAnimeshBone (), bone_Head->GetAnimeshBone (),
     bone8->GetAnimeshBone (), 0);

  // Create ragdoll factory
  csRef<iRagdollAnimNodeFactory> ragdollFactory =
    ragdollManager->CreateAnimNodeFactory ("franky_ragdoll",
					   skeletonFactory, dynSys);
  ragdollFactory->AddBodyChain (chain6, RAGDOLL_STATE_DYNAMIC);

  // Add ragdoll factory to the frankie's fsm animation node
  csRef<iSkeletonAnimNodeFactory2> animFactory =
    animeshFactory->GetSkeletonFactory ()->GetAnimationPacket ()->
    GetAnimationRoot ()->FindNode("standard");
  if (!animFactory)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	      "crystalspace.application.phystut",
	      "Can't find 'standard' animation node factory!");
    return;
  }

  csRef<iSkeletonFSMNodeFactory2> fsmfact =
    scfQueryInterface<iSkeletonFSMNodeFactory2>(animFactory);
  ragdollState = fsmfact->AddState ();
  fsmfact->SetStateName (ragdollState, "franky_ragdoll");
  fsmfact->SetStateNode (ragdollState, ragdollFactory);
}

void Simple::CreateRagdoll ()
{
  // Load frankie's factory if not yet done
  csRef<iMeshFactoryWrapper> meshfact =
    engine->FindMeshFactory ("franky_frankie");
  if (!meshfact)
  {
    LoadRagdoll ();
    meshfact = engine->FindMeshFactory ("franky_frankie");
  }

  if (!meshfact)
    return;

  // Create animesh
  ragdollMesh = engine->CreateMeshWrapper (meshfact, "Frankie",
					   room, csVector3 (0, -4, 0));

  csRef<iAnimatedMesh> animesh =
    scfQueryInterface<iAnimatedMesh> (ragdollMesh->GetMeshObject ());

  // Position the body
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();
  ragdollMesh->QuerySceneNode ()->GetMovable ()->SetPosition (
                  tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 1));

  // Start the ragdoll controller
  iSkeletonAnimNode2* root = animesh->GetSkeleton ()->GetAnimationPacket ()->
    GetAnimationRoot ();
  csRef<iSkeletonAnimNode2> anim;
  if (root)
  {
    anim = root->FindNode ("standard");

    csRef<iSkeletonFSMNode2> fsm =
      scfQueryInterfaceSafe<iSkeletonFSMNode2> (anim);
    if (fsm)
    {
      // Find the ragdoll anim node and set it up
      csRef<iSkeletonAnimNode2> animNode = fsm->FindNode ("franky_ragdoll");
      csRef<iRagdollAnimNode> ragdoll =
	scfQueryInterfaceSafe<iRagdollAnimNode> (animNode);
      CS_ASSERT (ragdoll);
      ragdoll->SetAnimatedMesh (animesh);

      // Play the animation node
      fsm->SwitchToState(ragdollState);
      root->Play ();
      anim->Play ();
      ragdoll->Play ();

      // Fling the body.
      for (uint i = 0; i < ragdoll->GetBoneCount (RAGDOLL_STATE_DYNAMIC); i++)
      {
	BoneID boneID = ragdoll->GetBone (RAGDOLL_STATE_DYNAMIC, i);
	iRigidBody* rb = ragdoll->GetBoneRigidBody (boneID);
	rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 5));
	rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 5, 0));
      }
    }
  }
}

void Simple::CreateWalls (const csVector3& /*radius*/)
{
  csOrthoTransform t;

  //csRef<iThingFactoryState> walls_state = 
    //scfQueryInterface<iThingFactoryState> (walls->GetMeshObject ()->GetFactory());

#if 0
  // Enabling this will work, however, mesh<->mesh collision
  // requires a lot of hand tuning. When this is enabled,
  // mesh objects created with 'm' will either sink through
  // the floor, or stick in it.

  // Some hints to make mesh<->mesh work better:
  //  * Decrease the time step. 1/300th of a second minimum
  //  * Slow down objects
  //  * Play with softness, cfm, etc.
  dynSys->AttachColliderMesh (walls, t, 10, 1);
#endif
#if 0
  // mesh <-> plane doesn't work yet, so we will use boxes for each
  // wall for now
  for(int i = 0; i < walls_state->GetPolygonCount(); i++)
  {
    rb->AttachColliderPlane(walls_state->GetPolygonObjectPlane(i), 10, 0, 0);
  }
#endif

  csVector3 size (10.0f, 10.0f, 10.0f); // This should be the same size as the mesh.
  t.SetOrigin(csVector3(10.0f,0.0f,0.0f));

  // Just to make sure everything works we create half of the colliders
  // using dynsys->CreateCollider() and the other half using
  // dynsys->AttachColliderBox().
  csRef<iDynamicsSystemCollider> collider = dynSys->CreateCollider ();
  collider->CreateBoxGeometry (size);
  collider->SetTransform (t);

  t.SetOrigin(csVector3(-10.0f,0.0f,0.0f));
  collider = dynSys->CreateCollider ();
  collider->CreateBoxGeometry (size);
  collider->SetTransform (t);

  t.SetOrigin(csVector3(0.0f,10.0f,0.0f));
  collider = dynSys->CreateCollider ();
  collider->CreateBoxGeometry (size);
  collider->SetTransform (t);

  t.SetOrigin(csVector3(0.0f,-10.0f,0.0f));
  dynSys->AttachColliderBox (size, t, 10, 0);
  t.SetOrigin(csVector3(0.0f,0.0f,10.0f));
  dynSys->AttachColliderBox (size, t, 10, 0);
  t.SetOrigin(csVector3(0.0f,0.0f,-10.0f));
  dynSys->AttachColliderBox (size, t, 10, 0);
}

void Simple::Start ()
{
  csDefaultRunLoop (object_reg);
}

void Simple::WriteShadow (int x,int y,int fg,const char *str,...)
{
  csString buf;

  va_list arg;
  va_start (arg, str);
  buf.FormatV (str, arg);
  va_end (arg);

  Write (x+1, y-1, 0, -1, "%s", buf.GetData());
  Write (x, y, fg, -1, "%s", buf.GetData());
}

void Simple::Write(int x,int y,int fg,int bg,const char *str,...)
{
  va_list arg;
  csString buf;

  va_start (arg,str);
  buf.FormatV (str, arg);
  va_end (arg);

  g2d->Write (courierFont, x, y, fg, bg, buf);
}

void Simple::DisplayKeys ()
{
  int x = 20;
  int y = 20;
  int fg = g2d->FindRGB (255, 150, 100);
  int lineSize = 15;

  WriteShadow (x - 5, y, fg, "Keys available:");
  y += lineSize;

  WriteShadow (x, y, fg, "b: spawn a box");
  y += lineSize;

  WriteShadow (x, y, fg, "s: spawn a sphere");
  y += lineSize;

  if (phys_engine_id == BULLET_ID)
  {
    WriteShadow (x, y, fg, "c: spawn a cylinder");
    y += lineSize;

    WriteShadow (x, y, fg, "a: spawn a capsule");
    y += lineSize;
  }

  WriteShadow (x, y, fg, "m: spawn a concave mesh");
  y += lineSize;

  WriteShadow (x, y, fg, "*: spawn a static concave mesh");
  y += lineSize;

  WriteShadow (x, y, fg, "j: spawn two jointed bodies");
  y += lineSize;

  if (phys_engine_id == BULLET_ID)
  {
    WriteShadow (x, y, fg, "h: spawn a chain");
    y += lineSize;

    WriteShadow (x, y, fg, "r: spawn a frankie's ragdoll");
    y += lineSize;
  }

  WriteShadow (x, y, fg, "SPACE: spawn random object");
  y += lineSize;

  WriteShadow (x, y, fg, "f: toggle physical/free camera");
  y += lineSize;

  WriteShadow (x, y, fg, "t: toggle all bodies to dynamic/static");
  y += lineSize;

  WriteShadow (x, y, fg, "p: pause the simulation");
  y += lineSize;

  WriteShadow (x, y, fg, "o: toggle speed of simulation");
  y += lineSize;

  WriteShadow (x, y, fg, "d: toggle colliders displayed/hidden");
  y += lineSize;

  if (phys_engine_id == BULLET_ID)
  {
    WriteShadow (x, y, fg, "?: toggle display of collisions");
    y += lineSize;
  }

  WriteShadow (x, y, fg, "g: toggle gravity");
  y += lineSize;

  WriteShadow (x, y, fg, "i: toggle autodisable enabled or not");
  y += lineSize;

  if (phys_engine_id == ODE_ID)
  {
    WriteShadow (x, y, fg, "1: enable StepFast solver");
    y += lineSize;

    WriteShadow (x, y, fg, "2: disable StepFast solver");
    y += lineSize;

    WriteShadow (x, y, fg, "3: enable QuickStep solver");
    y += lineSize;
  }
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
  simple->Shutdown ();
  delete simple; simple = 0;

  csInitializer::DestroyApplication (object_reg);
  return 0;
}

