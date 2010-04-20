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
#include "csgeom/sphere.h"
#include "imesh/genmesh.h"
#include "cstool/genmeshbuilder.h"
#include "phystut.h"

#define ODE_ID 1
#define BULLET_ID 2

#define CAMERA_DYNAMIC 1
#define CAMERA_KINEMATIC 2
#define CAMERA_FREE 3

Simple::Simple ()
  : csDemoApplication ("CrystalSpace.PhysTut", "phystut",
		       "phystut <OPTIONS>",
		       "Physics tutorial for Crystal Space."),
    solver (0), autodisable (false), do_bullet_debug (false),
    remainingStepDuration (0.0f), debugMode (false), allStatic (false),
    pauseDynamic (false), dynamicSpeed (1.0f),
    physicalCameraMode (CAMERA_DYNAMIC)    
{
  // Configure the options for csDemoApplication

  // We manage the camera by ourselves
  cameraMode = CSDEMO_CAMERA_NONE;

  // Command line options
  commandLineHelper.commandOptions.Push
    (csDemoCommandLineHelper::CommandOption
     ("phys_engine=<name>", "Specify which physics plugin to use (ode, bullet)"));
}

Simple::~Simple ()
{
}

void Simple::Frame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();

  // Now rotate the camera according to keyboard state
  const float speed = elapsed_time / 1000.0;

  // Camera is controlled by a rigid body
  if (physicalCameraMode == CAMERA_DYNAMIC)
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
      cameraBody->SetLinearVelocity (view->GetCamera()->GetTransform()
				     .GetT2O () * csVector3 (0, 0, 5));
    }
    if (kbd->GetKeyState (CSKEY_DOWN))
    {
      cameraBody->SetLinearVelocity (view->GetCamera()->GetTransform()
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
	rotX -=speed;
      if (kbd->GetKeyState (CSKEY_PGDN))
	rotX += speed;
      if (kbd->GetKeyState (CSKEY_UP))
	c->Move (CS_VEC_FORWARD * 4 * speed);
      if (kbd->GetKeyState (CSKEY_DOWN))
	c->Move (CS_VEC_BACKWARD * 4 * speed);
    }

    // We now assign a new rotation transformation to the camera.
    csQuaternion quaternion;
    quaternion.SetEulerAngles (csVector3 (rotX, rotY, rotZ));
    csOrthoTransform ot (quaternion.GetConjugate ().GetMatrix (), c->GetTransform().GetOrigin ());
    c->SetTransform (ot);
  }

  // Step the dynamic simulation
  if (!pauseDynamic)
  {
    // If the physics engine is ODE, then we have to take care of calling
    // the update of the dynamic simulation with a constant step time.
    if (phys_engine_id == ODE_ID)
    {
      float odeStepDuration = 0.01f;
      float totalDuration = remainingStepDuration + speed / dynamicSpeed;
      int iterationCount = (int) (totalDuration / odeStepDuration);
      for (int i = 0; i < iterationCount; i++)
	dyn->Step (odeStepDuration);

      // Store the remaining step duration
      remainingStepDuration = totalDuration -
	((float) iterationCount) * odeStepDuration;
    }

    // The bullet plugin uses a constant step time on its own
    else
      dyn->Step (speed / dynamicSpeed);
  }

  // Update camera position if it is controlled by a rigid body.
  // (in this mode we want to control the orientation of the camera,
  // so we update the camera position by ourselves instead of using
  // 'cameraBody->AttachCamera (camera)')
  if (physicalCameraMode == CAMERA_DYNAMIC)
    view->GetCamera ()->GetTransform ().SetOrigin
      (cameraBody->GetTransform ().GetOrigin ());

  // Update the demo's state information
  stateDescriptions.DeleteAll ();
  csString txt;

  stateDescriptions.Push (csString ("Physics engine: ") + phys_engine_name);
  txt.Format ("Physical objects count: %d", dynSys->GetBodysCount ());
  stateDescriptions.Push (txt);

  if (phys_engine_id == ODE_ID)
  {
    if (solver==0)
      stateDescriptions.Push (csString ("Solver: WorldStep"));
    else if (solver==1)
      stateDescriptions.Push (csString ("Solver: StepFast"));
    else if (solver==2)
      stateDescriptions.Push (csString ("Solver: QuickStep"));
  }

  if (autodisable)
    stateDescriptions.Push (csString ("AutoDisable: ON"));
  else
    stateDescriptions.Push (csString ("AutoDisable: OFF"));

  switch (physicalCameraMode)
    {
    case CAMERA_DYNAMIC:
      stateDescriptions.Push (csString ("Camera mode: dynamic"));
      break;

    case CAMERA_FREE:
      stateDescriptions.Push (csString ("Camera mode: free"));
      break;

    case CAMERA_KINEMATIC:
      stateDescriptions.Push (csString ("Camera mode: kinematic"));
      break;

    default:
      break;
    }

  // Default behavior from csDemoApplication
  csDemoApplication::Frame ();

  // Display debug informations
  if (do_bullet_debug)
    bullet_dynSys->DebugDraw (view);
}

bool Simple::OnKeyboard (iEvent &ev)
{
  // Default behavior from csDemoApplication
  csDemoApplication::OnKeyboard (ev);

  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    if (csKeyEventHelper::GetCookedCode (&ev) == CSKEY_SPACE)
      {
	int primitiveCount = phys_engine_id == BULLET_ID ? 7 : 4;
	switch (rand() % primitiveCount)
	  {
	  case 0: SpawnBox (); break;
	  case 1: SpawnSphere (); break;
	  case 2: SpawnMesh (); break;
	  case 3: SpawnJointed (); break;
	  case 4: SpawnCylinder (); break;
	  case 5: SpawnCapsule (); break;
	  case 6: SpawnRagdoll (); break;
	  default: break;
	  }
	return true;
      }

    else if (csKeyEventHelper::GetCookedCode (&ev) == 'b')
    {
      SpawnBox ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 's')
    {
      SpawnSphere ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'c'
	     && phys_engine_id == BULLET_ID)
    {
      SpawnCylinder ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'a'
	     && phys_engine_id == BULLET_ID)
    {
      SpawnCapsule ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'm')
    {
      SpawnMesh ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'v')
    {
      SpawnConvexMesh ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == '*')
    {
      SpawnStarCollider ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'j')
    {
      SpawnJointed ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'h'
	     && phys_engine_id == BULLET_ID)
    {
      SpawnChain ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'r'
	     && phys_engine_id == BULLET_ID)
    {
      SpawnRagdoll ();
      return true;
    }

    else if (csKeyEventHelper::GetCookedCode (&ev) == 'f')
    {
      // Toggle camera mode
      switch (physicalCameraMode)
	{
	case CAMERA_DYNAMIC:
	  physicalCameraMode = CAMERA_FREE;
	  break;

	case CAMERA_FREE:
	  if (phys_engine_id == BULLET_ID)
	    physicalCameraMode = CAMERA_KINEMATIC;
	  else
	    physicalCameraMode = CAMERA_DYNAMIC;
	  break;

	case CAMERA_KINEMATIC:
	  physicalCameraMode = CAMERA_DYNAMIC;
	  break;
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
	else
	{
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
      dynamicsDebugger->SetDebugDisplayMode (debugMode);

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
	     && phys_engine_id == BULLET_ID)
    {
      // Toggle collision debug mode
      // (this only works with the static 'Star' mesh spawned with key '*')
      do_bullet_debug = !do_bullet_debug;
    }

    else if (csKeyEventHelper::GetCookedCode (&ev) == 'g')
    {
      // Toggle gravity.
      dynSys->SetGravity (dynSys->GetGravity () == 0 ?
			  csVector3 (0.0f, -9.81f, 0.0f) : csVector3 (0));
      return true;
    }

    else if (csKeyEventHelper::GetCookedCode (&ev) == 'i')
    {
      // Toggle autodisable.
      dynSys->EnableAutoDisable (!dynSys->AutoDisableEnabled ());
      //dynSys->SetAutoDisableParams(1.5f,2.5f,6,0.0f);
      autodisable = dynSys->AutoDisableEnabled ();
      return true;
    }

    else if (csKeyEventHelper::GetCookedCode (&ev) == '1'
	     && phys_engine_id == ODE_ID)
    {
      // Toggle stepfast.
      csRef<iODEDynamicSystemState> osys = 
	scfQueryInterface<iODEDynamicSystemState> (dynSys);
      osys->EnableStepFast (0);
      solver=0;
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == '2'
	     && phys_engine_id == ODE_ID)
    {
      // Toggle stepfast.
      csRef<iODEDynamicSystemState> osys = 
	scfQueryInterface<iODEDynamicSystemState> (dynSys);
      osys->EnableStepFast (1);
      solver=1;
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == '3'
	     && phys_engine_id == ODE_ID)
    {
      // Toggle quickstep.
      csRef<iODEDynamicSystemState> osys = 
	scfQueryInterface<iODEDynamicSystemState> (dynSys);
      osys->EnableQuickStep (1);
      solver=2;
      return true;
    }
  }

  // Slow down the camera's body
  else if (physicalCameraMode == CAMERA_DYNAMIC
	   && (eventtype == csKeyEventTypeUp)
	   && ((csKeyEventHelper::GetCookedCode (&ev) == CSKEY_DOWN) 
	       || (csKeyEventHelper::GetCookedCode (&ev) == CSKEY_UP)))
  {
    cameraBody->SetLinearVelocity(csVector3 (0, 0, 0));
    cameraBody->SetAngularVelocity (csVector3 (0, 0, 0));
  }

  return false;
}

bool Simple::OnMouseDown (iEvent& ev)
{
  if (csMouseEventHelper::GetButton (&ev) == 0
      && phys_engine_id == BULLET_ID)
  {
    // Shoot!
    // Find the rigid body that was clicked on
    int mouseX = csMouseEventHelper::GetX (&ev);
    int mouseY = csMouseEventHelper::GetY (&ev);

    // Compute the end beam points
    csRef<iCamera> camera = view->GetCamera ();
    csVector2 v2d (mouseX, g2d->GetHeight () - mouseY);
    csVector3 v3d = camera->InvPerspective (v2d, 10000);
    csVector3 startBeam = camera->GetTransform ().GetOrigin ();
    csVector3 endBeam = camera->GetTransform ().This2Other (v3d);

    // Trace the physical beam
    csRef<iBulletDynamicSystem> bulletSystem =
      scfQueryInterface<iBulletDynamicSystem> (dynSys);
    csBulletHitBeamResult result = bulletSystem->HitBeam (startBeam, endBeam);

    // Add a force at the point clicked
    if (result.body)
    {
      csVector3 force = endBeam - startBeam;
      force.Normalize ();
      force *= 2.0f;
      result.body->AddForceAtPos (force, result.isect);

      // This would work too
      //csOrthoTransform transform (result.body->GetTransform ());
      //csVector3 relativePosition = transform.Other2This (result.isect);
      //result.body->AddForceAtRelPos (force, relativePosition);
    }

    return true;
  }

  return false;
}

bool Simple::OnInitialize (int argc, char* argv[])
{
  // Default behavior from csDemoApplication
  if (!csDemoApplication::OnInitialize (argc, argv))
    return false;

  // Request plugins
  if (!csInitializer::RequestPlugins (GetObjectRegistry (),
    CS_REQUEST_PLUGIN ("crystalspace.dynamics.debug",
		       iDynamicsDebuggerManager),
    CS_REQUEST_PLUGIN ("crystalspace.mesh.animesh.controllers.ragdoll",
		       iSkeletonRagdollManager2),
    CS_REQUEST_END))
    return ReportError ("Failed to initialize plugins!");

  csBaseEventHandler::Initialize (GetObjectRegistry ());
  if (!RegisterQueue (GetObjectRegistry (), csevAllEvents (GetObjectRegistry ())))
    return ReportError ("Failed to set up event handler!");

  // Checking for choosen dynamic system
  csRef<iCommandLineParser> clp = csQueryRegistry<iCommandLineParser> (GetObjectRegistry ());
  phys_engine_name = clp->GetOption ("phys_engine");
  if (phys_engine_name == "ode")
  {
    phys_engine_name = "ODE";
    phys_engine_id = ODE_ID;
    csRef<iPluginManager> plugmgr = 
      csQueryRegistry<iPluginManager> (GetObjectRegistry ());
    dyn = csLoadPlugin<iDynamics> (plugmgr, "crystalspace.dynamics.ode");
  }
  else 
  {
    phys_engine_name = "Bullet";
    phys_engine_id = BULLET_ID;
    csRef<iPluginManager> plugmgr = 
      csQueryRegistry<iPluginManager> (GetObjectRegistry ());
    dyn = csLoadPlugin<iDynamics> (plugmgr, "crystalspace.dynamics.bullet");
  }
  if (!dyn)
    return ReportError ("No iDynamics plugin!");

  // Now that we know the physical plugin in use, we can define the available keys
  keyDescriptions.DeleteAll ();
  keyDescriptions.Push ("b: spawn a box");
  keyDescriptions.Push ("s: spawn a sphere");
  if (phys_engine_id == BULLET_ID)
  {
    keyDescriptions.Push ("c: spawn a cylinder");
    keyDescriptions.Push ("a: spawn a capsule");
  }
  keyDescriptions.Push ("v: spawn a convex mesh");
  keyDescriptions.Push ("m: spawn a concave mesh");
  keyDescriptions.Push ("*: spawn a static concave mesh");
  keyDescriptions.Push ("j: spawn two jointed bodies");
  if (phys_engine_id == BULLET_ID)
  {
    keyDescriptions.Push ("h: spawn a chain");
    keyDescriptions.Push ("r: spawn a Frankie's ragdoll");
  }
  keyDescriptions.Push ("SPACE: spawn random object");
  if (phys_engine_id == BULLET_ID)
    keyDescriptions.Push ("left mouse: fire!");
  keyDescriptions.Push ("f: toggle camera modes");
  keyDescriptions.Push ("t: toggle all bodies dynamic/static");
  keyDescriptions.Push ("p: pause the simulation");
  keyDescriptions.Push ("o: toggle speed of simulation");
  keyDescriptions.Push ("d: toggle display of colliders");
  if (phys_engine_id == BULLET_ID)
    keyDescriptions.Push ("?: toggle display of collisions");
  keyDescriptions.Push ("g: toggle gravity");
  keyDescriptions.Push ("i: toggle autodisable");
  if (phys_engine_id == ODE_ID)
  {
    keyDescriptions.Push ("1: enable StepFast solver");
    keyDescriptions.Push ("2: disable StepFast solver");
    keyDescriptions.Push ("3: enable QuickStep solver");
  }

  return true;
}

bool Simple::Application ()
{
  // Default behavior from csDemoApplication
  if (!csDemoApplication::Application ())
    return false;

  // Find references to main objects
  debuggerManager =
    csQueryRegistry<iDynamicsDebuggerManager> (GetObjectRegistry ());
  if (!debuggerManager)
    return ReportError ("Failed to locate dynamic's debug manager!");

  ragdollManager =
    csQueryRegistry<iSkeletonRagdollManager2> (GetObjectRegistry ());
  if (!ragdollManager)
    return ReportError ("Failed to locate ragdoll manager!");

  // Create the dynamic system
  dynSys = dyn->CreateSystem ();
  if (!dynSys) return ReportError ("Error creating dynamic system!");
  dynSys->SetRollingDampener(.995f);

  // Configure the physical plugins
  if (phys_engine_id == ODE_ID)
  {
    csRef<iODEDynamicSystemState> osys = 
      scfQueryInterface<iODEDynamicSystemState> (dynSys);
    osys->SetContactMaxCorrectingVel (.1f);
    osys->SetContactSurfaceLayer (.0001f);
  }
  else
  {
    bullet_dynSys = scfQueryInterface<iBulletDynamicSystem> (dynSys);

    // We have some objects of size smaller than 0.035 units, so we scale up the
    // whole world for a better behavior of the dynamic simulation.
    bullet_dynSys->SetInternalScale (10.0f);
  }

  // Create the dynamic's debugger
  dynamicsDebugger = debuggerManager->CreateDebugger ();
  dynamicsDebugger->SetDynamicSystem (dynSys);

  // Don't display static colliders as the z-fighting with the original mesh
  // is very ugly
  dynamicsDebugger->SetStaticBodyMaterial (0);

  // Default behavior from csDemoApplication for the creation of the scene
  if (!csDemoApplication::CreateRoom ())
    return false;

  // Creating the scene's room
  dynamicsDebugger->SetDebugSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, -3));
  CreateWalls (csVector3 (5));

  // Set our own lights
  csRef<iLight> light;
  iLightList* lightList = room->GetLights ();
  lightList->RemoveAll ();

  light = engine->CreateLight(0, csVector3(10), 9000, csColor (1));
  lightList->Add (light);

  light = engine->CreateLight (0, csVector3 (3, 0, 0), 8, csColor (1, 0, 0));
  lightList->Add (light);

  light = engine->CreateLight (0, csVector3 (-3, 0,  0), 8, csColor (0, 0, 1));
  lightList->Add (light);

  light = engine->CreateLight (0, csVector3 (0, 0, 3), 8, csColor (0, 1, 0));
  lightList->Add (light);

  light = engine->CreateLight (0, csVector3 (0, -3, 0), 8, csColor (1, 1, 0));
  lightList->Add (light);

  engine->Prepare ();
  CS::Lighting::SimpleStaticLighter::ShineLights (room, engine, 4);

  // Preload some meshes and materials
  iTextureWrapper* txt = loader->LoadTexture ("spark",
    "/lib/std/spark.png");
  if (!txt) return ReportError ("Error loading texture!");

  // Load the box mesh factory.
  boxFact = loader->LoadMeshObjectFactory ("/lib/std/sprite1");
  if (!boxFact) return ReportError ("Error loading mesh object factory!");

  // Double the size.
  csMatrix3 m; m *= .5;
  csReversibleTransform t = csReversibleTransform (m, csVector3 (0));
  boxFact->HardTransform (t);

  // Load the mesh factory.
  meshFact = loader->LoadMeshObjectFactory ("/varia/physmesh");
  if (!meshFact) return ReportError ("Error loading mesh object factory!");

  // Init the camera
  UpdateCameraMode ();

  // Load the animesh & ragdoll at startup
  if (phys_engine_id == BULLET_ID)
    LoadRagdoll ();

  // Run the application
  Run();

  return true;
}

void Simple::UpdateCameraMode ()
{
  switch (physicalCameraMode)
    {
    // The camera is controlled by a rigid body
    case CAMERA_DYNAMIC:
      {
	// Check if there is already a rigid body created for the 'kinematic' mode
	if (cameraBody)
	{
	  cameraBody->MakeDynamic ();

	  // Remove the attached camera (in this mode we want to control
	  // the orientation of the camera, so we update the camera
	  // position by ourselves)
	  cameraBody->AttachCamera (0);
	}

	// Create a new rigid body
	else
	{
	  cameraBody = dynSys->CreateBody ();
	  cameraBody->SetProperties (1.0f, csVector3 (0.0f), csMatrix3 ());
	  cameraBody->SetTransform (view->GetCamera ()->GetTransform ());
	  cameraBody->AttachColliderSphere
	    (0.8f, csVector3 (0.0f), 10.0f, 1.0f, 0.8f);
	}

	break;
      }

    // The camera is free
    case CAMERA_FREE:
      {
	dynSys->RemoveBody (cameraBody);
	cameraBody = 0;

	// Update rotX, rotY, rotZ
	csQuaternion quaternion;
	quaternion.SetMatrix
	  (((csReversibleTransform) view->GetCamera ()->GetTransform ()).GetT2O ());
	csVector3 eulerAngles = quaternion.GetEulerAngles ();
	rotX = eulerAngles.x;
	rotY = eulerAngles.y;
	rotZ = eulerAngles.z;

	// Update the display of the dynamics debugger
	dynamicsDebugger->UpdateDisplay ();

	break;
      }

    // The camera is kinematic
    case CAMERA_KINEMATIC:
      {
	// Create a body
	cameraBody = dynSys->CreateBody ();
	cameraBody->SetProperties (1.0f, csVector3 (0.0f), csMatrix3 ());
	cameraBody->SetTransform (view->GetCamera ()->GetTransform ());
	cameraBody->AttachColliderSphere
	  (0.8f, csVector3 (0.0f), 10.0f, 1.0f, 0.8f);

	// Make it kinematic
	csRef<iBulletRigidBody> bulletBody =
	  scfQueryInterface<iBulletRigidBody> (cameraBody);
	bulletBody->MakeKinematic ();

	// Attach the camera to the body so as to benefit of the default
	// kinematic callback
	cameraBody->AttachCamera (view->GetCamera ());

	break;
      }

    default:
      break;
    }
}

iRigidBody* Simple::SpawnBox ()
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the mesh.
  csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (boxFact, "box", room));

  // Create a body and attach the mesh.
  csRef<iRigidBody> rb = dynSys->CreateBody ();
  rb->SetProperties (1.0f, csVector3 (0.0f), csMatrix3 ());
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

  // Update the display of the dynamics debugger
  dynamicsDebugger->UpdateDisplay ();

  return rb;
}

bool Simple::SpawnStarCollider ()
{
  csRef<iMeshFactoryWrapper> starFact;
  starFact = engine->FindMeshFactory ("genstar");
  if (!starFact)
  {
    loader->Load ("/lib/std/star.xml");
    starFact = engine->FindMeshFactory ("genstar");
    if (!starFact)
    {
      return ReportError ("Error loading 'star.xml'!");
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
    collider->CreateMeshGeometry (star);
    collider->SetTransform (tc);
  }

  else
  {
    csRef<iRigidBody> rb = dynSys->CreateBody ();
    rb->SetProperties (1.0f, csVector3 (0.0f), csMatrix3 ());
    rb->SetPosition (tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 2));

    const csMatrix3 tm;
    const csVector3 tv (0);
    csOrthoTransform t (tm, tv);
    rb->AttachColliderMesh (star, t, 10, 1, 0.8f);

    rb->AttachMesh (star);

    // Fling the body.
    rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 5));
    rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 0, 0));
  }

  // Update the display of the dynamics debugger
  dynamicsDebugger->UpdateDisplay ();

  return true;
}

iRigidBody* Simple::SpawnMesh ()
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the mesh.
  csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (meshFact, "mesh", room));

  // Create a body and attach the mesh.
  csRef<iRigidBody> rb = dynSys->CreateBody ();
  rb->SetProperties (1.0f, csVector3 (0.0f), csMatrix3 ());
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

  // Update the display of the dynamics debugger
  dynamicsDebugger->UpdateDisplay ();

  return rb;
}

iRigidBody* Simple::SpawnSphere ()
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the ball mesh factory.
  csRef<iMeshFactoryWrapper> ballFact = engine->CreateMeshFactory(
  	"crystalspace.mesh.object.genmesh", "ballFact");
  if (!ballFact)
  {
    ReportError ("Error creating mesh object factory!");
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
  rb->SetProperties (1.0f, csVector3 (0.0f), csMatrix3 ());
  rb->SetPosition (tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 1)
		   - artificialOffset);
  rb->AttachMesh (mesh);

  // Create and attach a sphere collider.
  rb->AttachColliderSphere (r, artificialOffset, 10, 1, 0.8f);

  // Fling the body.
  rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 6));
  rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 0, 0));

  // Update the display of the dynamics debugger
  dynamicsDebugger->UpdateDisplay ();

  return rb;
}

iRigidBody* Simple::SpawnCylinder ()
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the cylinder mesh factory.
  csRef<iMeshFactoryWrapper> cylinderFact = engine->CreateMeshFactory(
  	"crystalspace.mesh.object.genmesh", "cylinderFact");
  if (!cylinderFact)
  {
    ReportError ("Error creating mesh object factory!");
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
  rb->SetProperties (1.0f, csVector3 (0.0f), csMatrix3 ());
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

  // Update the display of the dynamics debugger
  dynamicsDebugger->UpdateDisplay ();

  return rb;
}

iRigidBody* Simple::SpawnCapsule ()
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the capsule mesh factory.
  csRef<iMeshFactoryWrapper> capsuleFact = engine->CreateMeshFactory(
  	"crystalspace.mesh.object.genmesh", "capsuleFact");
  if (!capsuleFact)
  {
    ReportError ("Error creating mesh object factory!");
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
  rb->SetProperties (1.0f, csVector3 (0.0f), csMatrix3 ());
  rb->AttachMesh (mesh);

  // Create and attach a capsule collider.
  csOrthoTransform t;
  rb->AttachColliderCapsule (length, radius, t, 10, 1, 0.8f);
  rb->SetPosition (tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 1));

  // Fling the body.
  rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 6));
  rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 0, 0));

  // Update the display of the dynamics debugger
  dynamicsDebugger->UpdateDisplay ();

  return rb;
}

iRigidBody* Simple::SpawnConvexMesh ()
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the mesh factory (a capsule in this example)
  csRef<iMeshFactoryWrapper> capsuleFact = engine->CreateMeshFactory(
  	"crystalspace.mesh.object.genmesh", "capsuleFact");
  if (!capsuleFact)
  {
    ReportError ("Error creating mesh object factory!");
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
  rb->SetProperties (1.0f, csVector3 (0.0f), csMatrix3 ());
  rb->AttachMesh (mesh);

  // Create and attach a mesh collider.
  csOrthoTransform t;
  rb->AttachColliderConvexMesh (mesh, t, 10, 1, 0.8f);
  rb->SetPosition (tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 1));

  // Fling the body.
  rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 6));
  rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 0, 0));

  // Update the display of the dynamics debugger
  dynamicsDebugger->UpdateDisplay ();

  return rb;
}

iJoint* Simple::SpawnJointed ()
{
  // Create and position objects.
  iRigidBody* rb1 = SpawnBox();
  rb1->SetPosition (rb1->GetPosition () +
    rb1->GetOrientation () * csVector3 (-.5, 0, 0));
  iRigidBody* rb2 = SpawnSphere();
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

  // Update the display of the dynamics debugger
  dynamicsDebugger->UpdateDisplay ();

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

void Simple::SpawnChain ()
{
  iRigidBody* rb1 = SpawnBox();
  csVector3 initPos = rb1->GetPosition () + csVector3 (0.0f, 5.0f, 0.0f);
  rb1->MakeStatic ();
  rb1->SetPosition (initPos);

  csVector3 offset (0.0f, 1.3f, 0.0f);

  iRigidBody* rb2 = SpawnCapsule();
  rb2->SetLinearVelocity (csVector3 (0.0f));
  rb2->SetAngularVelocity (csVector3 (0.0f));
  rb2->SetPosition (initPos - offset);
  rb2->SetOrientation (csXRotMatrix3 (PI / 2.0f));

  iRigidBody* rb3 = SpawnBox();
  rb3->SetLinearVelocity (csVector3 (0.0f));
  rb3->SetAngularVelocity (csVector3 (0.0f));
  rb3->SetPosition (initPos - 2.0f * offset);

  iRigidBody* rb4 = SpawnCapsule();
  rb4->SetLinearVelocity (csVector3 (0.0f));
  rb4->SetAngularVelocity (csVector3 (0.0f));
  rb4->SetPosition (initPos - 3.0f * offset);
  rb4->SetOrientation (csXRotMatrix3 (PI / 2.0f));

  iRigidBody* rb5 = SpawnBox();
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

  // Update the display of the dynamics debugger
  dynamicsDebugger->UpdateDisplay ();
}

void Simple::LoadRagdoll ()
{
  // Load animesh factory
  csLoadResult rc = loader->Load ("/lib/frankie/frankie.xml");
  if (!rc.success)
  {
    ReportError ("Can't load Frankie!");
    return;
  }
 
  csRef<iMeshFactoryWrapper> meshfact = engine->FindMeshFactory ("franky_frankie");
  if (!meshfact)
    return;

  csRef<iAnimatedMeshFactory> animeshFactory = scfQueryInterface<iAnimatedMeshFactory>
    (meshfact->GetMeshObjectFactory ());
  if (!animeshFactory)
  {
    ReportError ("Can't find Frankie's animesh factory!");
    return;
  }

  // Load bodymesh (animesh's physical properties)
  rc = loader->Load ("/lib/frankie/skelfrankie_body");
  if (!rc.success)
  {
    ReportError ("Can't load Frankie's body description!");
    return;
  }

  csRef<iBodyManager> bodyManager = csQueryRegistry<iBodyManager> (GetObjectRegistry ());
  iBodySkeleton* bodySkeleton = bodyManager->FindBodySkeleton ("frankie_body");
  if (!bodySkeleton)
  {
    ReportError ("Can't find Frankie's body description!");
    return;
  }

  // Create bone chain
  iBodyChain* chain = bodySkeleton->CreateBodyChain
    ("chain", animeshFactory->GetSkeletonFactory ()->FindBone ("Frankie_Main"),
     animeshFactory->GetSkeletonFactory ()->FindBone ("CTRL_Head"),
     animeshFactory->GetSkeletonFactory ()->FindBone ("Tail_8"), 0);

  // Create ragdoll animation node factory
  csRef<iSkeletonRagdollNodeFactory2> ragdollFactory =
    ragdollManager->CreateAnimNodeFactory ("frankie_ragdoll",
					   bodySkeleton, dynSys);
  ragdollFactory->AddBodyChain (chain, RAGDOLL_STATE_DYNAMIC);

  // Set the ragdoll anim node as the only node of the animation tree
  animeshFactory->GetSkeletonFactory ()->GetAnimationPacket ()
    ->SetAnimationRoot (ragdollFactory);
}

void Simple::SpawnRagdoll ()
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

  // Close the eyes of Frankie as he is dead
  csRef<iAnimatedMeshFactory> animeshFactory = scfQueryInterface<iAnimatedMeshFactory>
    (meshfact->GetMeshObjectFactory ());

  animesh->SetMorphTargetWeight (animeshFactory->FindMorphTarget ("eyelids_closed"), 0.7f);

  // Set initial position of the body
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();
  ragdollMesh->QuerySceneNode ()->GetMovable ()->SetPosition (
                  tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 1));

  // Start the ragdoll anim node
  iSkeletonAnimNode2* root = animesh->GetSkeleton ()->GetAnimationPacket ()->
    GetAnimationRoot ();

  csRef<iSkeletonRagdollNode2> ragdoll =
    scfQueryInterfaceSafe<iSkeletonRagdollNode2> (root);
  ragdoll->SetAnimatedMesh (animesh);

  // Fling the body.
  // (start the ragdoll node before so that the rigid bodies are created)
  ragdoll->Play ();
  for (uint i = 0; i < ragdoll->GetBoneCount (RAGDOLL_STATE_DYNAMIC); i++)
  {
    BoneID boneID = ragdoll->GetBone (RAGDOLL_STATE_DYNAMIC, i);
    iRigidBody* rb = ragdoll->GetBoneRigidBody (boneID);
    rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 5));
    rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 5, 0));
  }

  // Update the display of the dynamics debugger
  dynamicsDebugger->UpdateDisplay ();
}

void Simple::CreateWalls (const csVector3& /*radius*/)
{
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

  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
    ReportWarning ("Could not load texture 'stone'");
  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");
  walls->GetMeshObject ()->SetMaterialWrapper (tm);

  csOrthoTransform t;

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
  // With ODE, mesh <-> plane doesn't work yet, so we will use boxes for each
  // wall for now
  for (int i = 0; i < walls_state->GetPolygonCount (); i++)
  {
    rb->AttachColliderPlane (walls_state->GetPolygonObjectPlane (i), 10, 0, 0);
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

  t.SetOrigin(csVector3(-10.0f, 0.0f, 0.0f));
  collider = dynSys->CreateCollider ();
  collider->CreateBoxGeometry (size);
  collider->SetTransform (t);

  t.SetOrigin(csVector3(0.0f, 10.0f, 0.0f));
  collider = dynSys->CreateCollider ();
  collider->CreateBoxGeometry (size);
  collider->SetTransform (t);

  // If we use the Bullet plugin, then use a plane collider for the floor
  if (phys_engine_id == ODE_ID)
  {
    t.SetOrigin(csVector3(0.0f, -10.0f, 0.0f));
    dynSys->AttachColliderBox (size, t, 10.0f, 0.0f);
  }
  else
    dynSys->AttachColliderPlane (csPlane3 (csVector3 (0.0f, 1.0f, 0.0f), -5.0f),
				 10.0f, 0.0f);

  t.SetOrigin(csVector3(0.0f, 0.0f, 10.0f));
  dynSys->AttachColliderBox (size, t, 10.0f, 0.0f);

  t.SetOrigin(csVector3(0.0f, 0.0f, -10.0f));
  dynSys->AttachColliderBox (size, t, 10.0f, 0.0f);
}

//---------------------------------------------------------------------------

CS_IMPLEMENT_APPLICATION

int main (int argc, char* argv[])
{
  return Simple ().Main(argc, argv);
}
