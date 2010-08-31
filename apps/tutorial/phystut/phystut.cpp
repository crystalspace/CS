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
#include "imesh/terrain2.h"
#include "cstool/genmeshbuilder.h"
#include "cstool/materialbuilder.h"
#include "phystut.h"

#define ODE_ID 1
#define BULLET_ID 2

#define CAMERA_DYNAMIC 1
#define CAMERA_KINEMATIC 2
#define CAMERA_FREE 3

#define ENVIRONMENT_WALLS 1
#define ENVIRONMENT_TERRAIN 2

Simple::Simple ()
  : DemoApplication ("CrystalSpace.PhysTut", "phystut",
		     "phystut <OPTIONS>",
		     "Physics tutorial for Crystal Space."),
    isSoftBodyWorld (false), environment (ENVIRONMENT_WALLS), solver (0),
    autodisable (false), do_bullet_debug (false), remainingStepDuration (0.0f),
    debugMode (false), allStatic (false), pauseDynamic (false), dynamicSpeed (1.0f),
    physicalCameraMode (CAMERA_DYNAMIC), dragging (false), softDragging (false)
{
  // Configure the options for DemoApplication

  // We manage the camera by ourselves
  cameraHelper.SetCameraMode (CS::Demo::CSDEMO_CAMERA_NONE);

  // Command line options
  commandLineHelper.AddCommandLineOption
    ("phys_engine=<name>", "Specify which physics plugin to use (ode, bullet)");
  commandLineHelper.AddCommandLineOption
    ("disable_soft", "Disable the soft bodies");
  commandLineHelper.AddCommandLineOption
    ("terrain", "Start with the terrain environment");
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

    float cameraSpeed = environment == ENVIRONMENT_WALLS ? 4 : 30;
    if (kbd->GetKeyState (CSKEY_SHIFT))
    {
      // If the user is holding down shift, the arrow keys will cause
      // the camera to strafe up, down, left or right from it's
      // current position.
      if (kbd->GetKeyState (CSKEY_RIGHT))
	c->Move (CS_VEC_RIGHT * cameraSpeed * speed);
      if (kbd->GetKeyState (CSKEY_LEFT))
	c->Move (CS_VEC_LEFT * cameraSpeed * speed);
      if (kbd->GetKeyState (CSKEY_UP))
	c->Move (CS_VEC_UP * cameraSpeed * speed);
      if (kbd->GetKeyState (CSKEY_DOWN))
	c->Move (CS_VEC_DOWN * cameraSpeed * speed);
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
	c->Move (CS_VEC_FORWARD * cameraSpeed * speed);
      if (kbd->GetKeyState (CSKEY_DOWN))
	c->Move (CS_VEC_BACKWARD * cameraSpeed * speed);
    }

    // We now assign a new rotation transformation to the camera.
    csQuaternion quaternion;
    quaternion.SetEulerAngles (csVector3 (rotX, rotY, rotZ));
    csOrthoTransform ot (quaternion.GetConjugate ().GetMatrix (), c->GetTransform().GetOrigin ());
    c->SetTransform (ot);
  }

  if (dragging)
  {
    // Keep the drag joint at the same distance to the camera
    csRef<iCamera> camera = view->GetCamera ();
    csVector2 v2d (mouseX, g2d->GetHeight () - mouseY);
    csVector3 v3d = camera->InvPerspective (v2d, 10000);
    csVector3 startBeam = camera->GetTransform ().GetOrigin ();
    csVector3 endBeam = camera->GetTransform ().This2Other (v3d);

    csVector3 newPosition = endBeam - startBeam;
    newPosition.Normalize ();
    newPosition = camera->GetTransform ().GetOrigin () + newPosition * dragDistance;
    dragJoint->SetPosition (newPosition);
  }

  // Step the dynamic simulation
  if (!pauseDynamic)
  {
    /*
    for (int i = 0; i < dynamicSystem->GetBodysCount (); i++)
    {
      iRigidBody* body = dynamicSystem->GetBody (i);
      body->SetLinearVelocity (csVector3 (0.0f));
      body->SetAngularVelocity (csVector3 (0.0f));
    }
    */
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
  hudHelper.stateDescriptions.DeleteAll ();
  csString txt;

  hudHelper.stateDescriptions.Push (csString ("Physics engine: ") + phys_engine_name);

  txt.Format ("Rigid bodies count: %d", dynamicSystem->GetBodysCount ());
  hudHelper.stateDescriptions.Push (txt);

  if (isSoftBodyWorld)
  {
    txt.Format ("Soft bodies count: %d", (int) bulletDynamicSystem->GetSoftBodyCount ());
    hudHelper.stateDescriptions.Push (txt);
  }

  if (phys_engine_id == ODE_ID)
  {
    if (solver==0)
      hudHelper.stateDescriptions.Push (csString ("Solver: WorldStep"));
    else if (solver==1)
      hudHelper.stateDescriptions.Push (csString ("Solver: StepFast"));
    else if (solver==2)
      hudHelper.stateDescriptions.Push (csString ("Solver: QuickStep"));
  }

  if (autodisable)
    hudHelper.stateDescriptions.Push (csString ("AutoDisable: ON"));
  else
    hudHelper.stateDescriptions.Push (csString ("AutoDisable: OFF"));

  switch (physicalCameraMode)
    {
    case CAMERA_DYNAMIC:
      hudHelper.stateDescriptions.Push (csString ("Camera mode: dynamic"));
      break;

    case CAMERA_FREE:
      hudHelper.stateDescriptions.Push (csString ("Camera mode: free"));
      break;

    case CAMERA_KINEMATIC:
      hudHelper.stateDescriptions.Push (csString ("Camera mode: kinematic"));
      break;

    default:
      break;
    }

  // Default behavior from DemoApplication
  DemoApplication::Frame ();

  // Display debug informations
  if (do_bullet_debug)
    bulletDynamicSystem->DebugDraw (view);

  // Display the rope soft bodies
  else if (isSoftBodyWorld)
    for (size_t i = 0; i < bulletDynamicSystem->GetSoftBodyCount (); i++)
    {
      CS::Physics::Bullet::iSoftBody* softBody = bulletDynamicSystem->GetSoftBody (i);
      if (!softBody->GetTriangleCount ())
	softBody->DebugDraw (view);
    }
}

bool Simple::OnKeyboard (iEvent &ev)
{
  // Default behavior from DemoApplication
  DemoApplication::OnKeyboard (ev);

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
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'q'
	     && phys_engine_id == BULLET_ID)
    {
      SpawnCompound ();
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
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'y'
	     && phys_engine_id == BULLET_ID && isSoftBodyWorld)
    {
      SpawnRope ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'u'
	     && phys_engine_id == BULLET_ID && isSoftBodyWorld)
    {
      SpawnCloth ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'i'
	     && phys_engine_id == BULLET_ID && isSoftBodyWorld)
    {
      SpawnSoftBody ();
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

      for (int i = 0; i < dynamicSystem->GetBodysCount (); i++)
      {
	iRigidBody* body = dynamicSystem->GetBody (i);
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
      dynamicSystem->SetGravity (dynamicSystem->GetGravity () == 0 ?
				 csVector3 (0.0f, -9.81f, 0.0f) : csVector3 (0));
      return true;
    }

    else if (csKeyEventHelper::GetCookedCode (&ev) == 'I')
    {
      // Toggle autodisable.
      dynamicSystem->EnableAutoDisable (!dynamicSystem->AutoDisableEnabled ());
      //dynamicSystem->SetAutoDisableParams(1.5f,2.5f,6,0.0f);
      autodisable = dynamicSystem->AutoDisableEnabled ();
      return true;
    }

    else if (csKeyEventHelper::GetCookedCode (&ev) == '1'
	     && phys_engine_id == ODE_ID)
    {
      // Toggle stepfast.
      csRef<iODEDynamicSystemState> osys = 
	scfQueryInterface<iODEDynamicSystemState> (dynamicSystem);
      osys->EnableStepFast (0);
      solver=0;
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == '2'
	     && phys_engine_id == ODE_ID)
    {
      // Toggle stepfast.
      csRef<iODEDynamicSystemState> osys = 
	scfQueryInterface<iODEDynamicSystemState> (dynamicSystem);
      osys->EnableStepFast (1);
      solver=1;
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == '3'
	     && phys_engine_id == ODE_ID)
    {
      // Toggle quickstep.
      csRef<iODEDynamicSystemState> osys = 
	scfQueryInterface<iODEDynamicSystemState> (dynamicSystem);
      osys->EnableQuickStep (1);
      solver=2;
      return true;
    }

    // Cut operation
    else if (csKeyEventHelper::GetRawCode (&ev) == 'x'
	     && kbd->GetKeyState (CSKEY_CTRL)
	     && phys_engine_id == BULLET_ID)
    {
      // Trace a beam to find if a rigid body was under the mouse cursor
      csRef<iCamera> camera = view->GetCamera ();
      csVector2 v2d (mouseX, g2d->GetHeight () - mouseY);
      csVector3 v3d = camera->InvPerspective (v2d, 10000);
      csVector3 startBeam = camera->GetTransform ().GetOrigin ();
      csVector3 endBeam = camera->GetTransform ().This2Other (v3d);

      CS::Physics::Bullet::HitBeamResult hitResult =
	bulletDynamicSystem->HitBeam (startBeam, endBeam);
      if (hitResult.hasHit
	  && hitResult.body->GetType () == CS::Physics::Bullet::RIGID_BODY)
      {
	// Remove the body and the mesh from the simulation, and put them in the clipboard
	clipboardBody = hitResult.body->QueryRigidBody ();
	dynamicSystem->RemoveBody (clipboardBody);
	clipboardMesh = hitResult.body->QueryRigidBody ()->GetAttachedMesh ();
	if (clipboardMesh)
	  room->GetMeshes ()->Remove (clipboardMesh);

	// Update the display of the dynamics debugger
	dynamicsDebugger->UpdateDisplay ();
      }
    }

    // Paste operation
    else if (csKeyEventHelper::GetRawCode (&ev) == 'v'
	     && kbd->GetKeyState (CSKEY_CTRL)
	     && phys_engine_id == BULLET_ID
	     && clipboardBody.IsValid ())
    {
      // Compute the new position of the body
      csRef<iCamera> camera = view->GetCamera ();
      csVector2 v2d (mouseX, g2d->GetHeight () - mouseY);
      csVector3 v3d = camera->InvPerspective (v2d, 10000);
      csVector3 startBeam = camera->GetTransform ().GetOrigin ();
      csVector3 endBeam = camera->GetTransform ().This2Other (v3d);

      csVector3 newPosition = endBeam - startBeam;
      newPosition.Normalize ();
      clipboardBody->SetPosition (camera->GetTransform ().GetOrigin () + newPosition * 1.5f);

      // Put back the body from the clipboard to the simulation
      dynamicSystem->AddBody (clipboardBody);
      room->GetMeshes ()->Add (clipboardMesh);
      clipboardBody = 0;
      clipboardMesh = 0;

      // Update the display of the dynamics debugger
      dynamicsDebugger->UpdateDisplay ();
    }

#ifdef CS_HAVE_BULLET_SERIALIZER
    // Save a .bullet file
    else if (csKeyEventHelper::GetRawCode (&ev) == 's'
	     && kbd->GetKeyState (CSKEY_CTRL)
	     && phys_engine_id == BULLET_ID)
    {
      const char* filename = "phystut_world.bullet";
      if (bulletDynamicSystem->SaveBulletWorld (filename))
	printf ("Dynamic world successfully saved as file %s\n", filename);
      else
	printf ("Problem saving dynamic world to file %s\n", filename);

      return true;
    }
#endif
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

// This method updates the position of the dragging for soft bodies
csVector3 MouseAnchorAnimationControl::GetAnchorPosition () const
{
  // Keep the drag joint at the same distance to the camera
  csRef<iCamera> camera = simple->view->GetCamera ();
  csVector2 v2d (simple->mouseX, simple->g2d->GetHeight () - simple->mouseY);
  csVector3 v3d = camera->InvPerspective (v2d, 10000);
  csVector3 startBeam = camera->GetTransform ().GetOrigin ();
  csVector3 endBeam = camera->GetTransform ().This2Other (v3d);

  csVector3 newPosition = endBeam - startBeam;
  newPosition.Normalize ();
  newPosition = camera->GetTransform ().GetOrigin () + newPosition * simple->dragDistance;
  return newPosition;  
}

bool Simple::OnMouseDown (iEvent& ev)
{
  // Left mouse button: Shoot!
  if (csMouseEventHelper::GetButton (&ev) == 0
      && phys_engine_id == BULLET_ID)
  {
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
    CS::Physics::Bullet::HitBeamResult hitResult =
      bulletDynamicSystem->HitBeam (startBeam, endBeam);
    if (!hitResult.hasHit)
      return false;

    // Add a force at the point clicked
    if (hitResult.body->GetType () == CS::Physics::Bullet::RIGID_BODY)
    {
      csVector3 force = endBeam - startBeam;
      force.Normalize ();
      force *= 2.0f;

      // Check if the body hit is not static or kinematic
      csRef<CS::Physics::Bullet::iRigidBody> bulletBody =
	scfQueryInterface<CS::Physics::Bullet::iRigidBody> (hitResult.body->QueryRigidBody ());
      if (bulletBody->GetDynamicState () != CS::Physics::Bullet::STATE_DYNAMIC)
	return false;

      hitResult.body->QueryRigidBody ()->AddForceAtPos (force, hitResult.isect);

      // This would work too
      //csOrthoTransform transform (hitResult.body->QueryRigidBody ()->GetTransform ());
      //csVector3 relativePosition = transform.Other2This (hitResult.isect);
      //hitResult.body->QueryRigidBody ()->AddForceAtRelPos (force, relativePosition);
    }

    else if (hitResult.body->GetType () == CS::Physics::Bullet::SOFT_BODY)
    {
      csVector3 force = endBeam - startBeam;
      force.Normalize ();
      force *= 2.0f;
      hitResult.body->QuerySoftBody ()->AddForce (force, hitResult.vertexIndex);
    }

    return true;
  }

  // Right mouse button: dragging
  else if (csMouseEventHelper::GetButton (&ev) == 1
	   && phys_engine_id == BULLET_ID)
  {
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
    CS::Physics::Bullet::HitBeamResult hitResult =
      bulletDynamicSystem->HitBeam (startBeam, endBeam);
    if (!hitResult.hasHit)
      return false;

    // Check if we hit a rigid body
    if (hitResult.body->GetType () == CS::Physics::Bullet::RIGID_BODY)
    {
      // Create a pivot joint at the point clicked
      dragJoint = bulletDynamicSystem->CreatePivotJoint ();
      dragJoint->Attach (hitResult.body->QueryRigidBody (), hitResult.isect);

      dragging = true;
      dragDistance = (hitResult.isect - startBeam).Norm ();

      // Set some dampening on the rigid body to have a more stable dragging
      csRef<CS::Physics::Bullet::iRigidBody> bulletBody =
	scfQueryInterface<CS::Physics::Bullet::iRigidBody> (hitResult.body->QueryRigidBody ());
      linearDampening = bulletBody->GetLinearDampener ();
      angularDampening = bulletBody->GetRollingDampener ();
      bulletBody->SetLinearDampener (0.9f);
      bulletBody->SetRollingDampener (0.9f);
    }

    else if (hitResult.body->GetType () == CS::Physics::Bullet::SOFT_BODY)
    {
      softDragging = true;
      draggedBody = hitResult.body->QuerySoftBody ();
      draggedVertex = hitResult.vertexIndex;
      dragDistance = (hitResult.isect - startBeam).Norm ();
      grabAnimationControl.AttachNew (new MouseAnchorAnimationControl (this));
      hitResult.body->QuerySoftBody ()->AnchorVertex (hitResult.vertexIndex, grabAnimationControl);
    }

    else return false;

    return true;
  }

  return false;
}

bool Simple::OnMouseUp (iEvent& ev)
{
  if (dragging)
  {
    dragging = false;

    // Put back the original dampening on the rigid body
    csRef<CS::Physics::Bullet::iRigidBody> bulletBody =
      scfQueryInterface<CS::Physics::Bullet::iRigidBody> (dragJoint->GetAttachedBody ());
    bulletBody->SetLinearDampener (linearDampening);
    bulletBody->SetRollingDampener (angularDampening);

    // Remove the drag joint
    bulletDynamicSystem->RemovePivotJoint (dragJoint);
    dragJoint = 0;

    return true;
  }

  if (softDragging)
  {
    softDragging = false;
    draggedBody->RemoveAnchor (draggedVertex);
    draggedBody = 0;
  }

  return false;
}

bool Simple::OnMouseMove (iEvent& ev)
{
  // Save the mouse position
  mouseX = csMouseEventHelper::GetX (&ev);
  mouseY = csMouseEventHelper::GetY (&ev);

  return false;
}

bool Simple::OnInitialize (int argc, char* argv[])
{
  // Default behavior from DemoApplication
  if (!DemoApplication::OnInitialize (argc, argv))
    return false;

  // Request plugins
  if (!csInitializer::RequestPlugins (GetObjectRegistry (),
    CS_REQUEST_PLUGIN ("crystalspace.dynamics.debug",
		       CS::Debug::iDynamicsDebuggerManager),
    CS_REQUEST_PLUGIN ("crystalspace.mesh.animesh.controllers.ragdoll",
		       CS::Animation::iSkeletonRagdollManager),
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

    // Check whether the soft bodies are enabled or not
    isSoftBodyWorld = !clp->GetBoolOption ("disable_soft", false);

    // Load the soft body animation control plugin & factory
    if (isSoftBodyWorld)
    {
      csRef<CS::Animation::iSoftBodyAnimationControlType> softBodyAnimationType =
	csLoadPlugin<CS::Animation::iSoftBodyAnimationControlType>
	(plugmgr, "crystalspace.dynamics.softanim");

      csRef<iGenMeshAnimationControlFactory> animationFactory =
	softBodyAnimationType->CreateAnimationControlFactory ();
      softBodyAnimationFactory =
	scfQueryInterface<CS::Animation::iSoftBodyAnimationControlFactory> (animationFactory);
    }

    // Check which environment has to be loaded
    if (clp->GetBoolOption ("terrain", false))
      environment = ENVIRONMENT_TERRAIN;
  }

  if (!dyn)
    return ReportError ("No iDynamics plugin!");

  // Now that we know the physical plugin in use, we can define the available keys
  hudHelper.keyDescriptions.DeleteAll ();
  hudHelper.keyDescriptions.Push ("b: spawn a box");
  hudHelper.keyDescriptions.Push ("s: spawn a sphere");
  if (phys_engine_id == BULLET_ID)
  {
    hudHelper.keyDescriptions.Push ("c: spawn a cylinder");
    hudHelper.keyDescriptions.Push ("a: spawn a capsule");
  }
  hudHelper.keyDescriptions.Push ("v: spawn a convex mesh");
  hudHelper.keyDescriptions.Push ("m: spawn a concave mesh");
  hudHelper.keyDescriptions.Push ("*: spawn a static concave mesh");
  if (phys_engine_id == BULLET_ID)
    hudHelper.keyDescriptions.Push ("q: spawn a compound body");
  hudHelper.keyDescriptions.Push ("j: spawn two jointed bodies");
  if (phys_engine_id == BULLET_ID)
  {
    hudHelper.keyDescriptions.Push ("h: spawn a chain");
    hudHelper.keyDescriptions.Push ("r: spawn a Frankie's ragdoll");
  }
  if (isSoftBodyWorld)
  {
    hudHelper.keyDescriptions.Push ("y: spawn a rope");
    hudHelper.keyDescriptions.Push ("u: spawn a cloth");
    hudHelper.keyDescriptions.Push ("i: spawn a soft body");
  }
  hudHelper.keyDescriptions.Push ("SPACE: spawn random object");
  if (phys_engine_id == BULLET_ID)
  {
    hudHelper.keyDescriptions.Push ("left mouse: fire!");
    hudHelper.keyDescriptions.Push ("right mouse: drag object");
    hudHelper.keyDescriptions.Push ("CTRL-x: cut selected object");
    hudHelper.keyDescriptions.Push ("CTRL-v: paste object");
  }
  hudHelper.keyDescriptions.Push ("f: toggle camera modes");
  hudHelper.keyDescriptions.Push ("t: toggle all bodies dynamic/static");
  hudHelper.keyDescriptions.Push ("p: pause the simulation");
  hudHelper.keyDescriptions.Push ("o: toggle speed of simulation");
  hudHelper.keyDescriptions.Push ("d: toggle display of colliders");
  if (phys_engine_id == BULLET_ID)
    hudHelper.keyDescriptions.Push ("?: toggle display of collisions");
  hudHelper.keyDescriptions.Push ("g: toggle gravity");
  hudHelper.keyDescriptions.Push ("I: toggle autodisable");
  if (phys_engine_id == ODE_ID)
  {
    hudHelper.keyDescriptions.Push ("1: enable StepFast solver");
    hudHelper.keyDescriptions.Push ("2: disable StepFast solver");
    hudHelper.keyDescriptions.Push ("3: enable QuickStep solver");
  }
#ifdef CS_HAVE_BULLET_SERIALIZER
  if (phys_engine_id == BULLET_ID)
    hudHelper.keyDescriptions.Push ("CTRL-s: save the dynamic world");
#endif
  /*
  if (phys_engine_id == BULLET_ID)
    hudHelper.keyDescriptions.Push ("CTRL-n: next environment");
  */
  return true;
}

bool Simple::Application ()
{
  // Default behavior from DemoApplication
  if (!DemoApplication::Application ())
    return false;

  // Find references to main objects
  debuggerManager =
    csQueryRegistry<CS::Debug::iDynamicsDebuggerManager> (GetObjectRegistry ());
  if (!debuggerManager)
    return ReportError ("Failed to locate dynamic's debug manager!");

  ragdollManager =
    csQueryRegistry<CS::Animation::iSkeletonRagdollManager> (GetObjectRegistry ());
  if (!ragdollManager)
    return ReportError ("Failed to locate ragdoll manager!");

  // Create the dynamic system
  dynamicSystem = dyn->CreateSystem ();
  if (!dynamicSystem) return ReportError ("Error creating dynamic system!");

  // Set some linear and angular dampening in order to have a reduction of
  // the movements of the objects
  dynamicSystem->SetLinearDampener(0.1f);
  dynamicSystem->SetRollingDampener(0.1f);

  // Configure the physical plugins
  if (phys_engine_id == ODE_ID)
  {
    csRef<iODEDynamicSystemState> osys = 
      scfQueryInterface<iODEDynamicSystemState> (dynamicSystem);
    osys->SetContactMaxCorrectingVel (.1f);
    osys->SetContactSurfaceLayer (.0001f);
  }
  else
  {
    bulletDynamicSystem = scfQueryInterface<CS::Physics::Bullet::iDynamicSystem> (dynamicSystem);

    // We have some objects of size smaller than 0.035 units, so we scale up the
    // whole world for a better behavior of the dynamic simulation.
    bulletDynamicSystem->SetInternalScale (10.0f);

    // Enable soft bodies
    if (isSoftBodyWorld)
      bulletDynamicSystem->SetSoftBodyWorld (true);
  }

  // Create the dynamic's debugger
  dynamicsDebugger = debuggerManager->CreateDebugger ();
  dynamicsDebugger->SetDynamicSystem (dynamicSystem);

  // Don't display static colliders as the z-fighting with the original mesh
  // is very ugly
  dynamicsDebugger->SetStaticBodyMaterial (0);

  // Create the environment
  if (environment == ENVIRONMENT_WALLS)
  {
    CreateWalls (csVector3 (5));
    view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, -3));
  }
  else
  {
    CreateTerrain ();
    view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 30, -3));
  }

  // Initialize the dynamics debugger
  dynamicsDebugger->SetDebugSector (room);

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
	  cameraBody = dynamicSystem->CreateBody ();
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
	dynamicSystem->RemoveBody (cameraBody);
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
	cameraBody = dynamicSystem->CreateBody ();
	cameraBody->SetProperties (1.0f, csVector3 (0.0f), csMatrix3 ());
	cameraBody->SetTransform (view->GetCamera ()->GetTransform ());
	cameraBody->AttachColliderSphere
	  (0.8f, csVector3 (0.0f), 10.0f, 1.0f, 0.8f);

	// Make it kinematic
	csRef<CS::Physics::Bullet::iRigidBody> bulletBody =
	  scfQueryInterface<CS::Physics::Bullet::iRigidBody> (cameraBody);
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
  csRef<iRigidBody> rb = dynamicSystem->CreateBody ();
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
  // Find the 'star' mesh factory
  csRef<iMeshFactoryWrapper> starFact;
  starFact = engine->FindMeshFactory ("genstar");
  if (!starFact)
  {
    loader->Load ("/lib/std/star.xml");
    starFact = engine->FindMeshFactory ("genstar");
    if (!starFact)
      return ReportError ("Error loading 'star.xml'!");
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
    csRef<iDynamicsSystemCollider> collider = dynamicSystem->CreateCollider ();
    collider->CreateMeshGeometry (star);
    collider->SetTransform (tc);
  }

  else
  {
    csRef<iRigidBody> rb = dynamicSystem->CreateBody ();
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
  csRef<iRigidBody> rb = dynamicSystem->CreateBody ();
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
  csRef<iRigidBody> rb = dynamicSystem->CreateBody ();
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
  csRef<iRigidBody> rb = dynamicSystem->CreateBody ();
  rb->SetProperties (1.0f, csVector3 (0.0f), csMatrix3 ());
  rb->AttachMesh (mesh);

  // Create and attach a cylinder collider.
  csMatrix3 m;
  csReversibleTransform t = csReversibleTransform (m, artificialOffset);
  rb->AttachColliderCylinder (length, radius, t, 10, 1, 0.8f);
  rb->SetPosition (tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 1)
		   - artificialOffset);
  rb->SetOrientation (csXRotMatrix3 (PI / 5.0));

  // Fling the body.
  rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 6));
  rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 0, 0));

  // Update the display of the dynamics debugger
  dynamicsDebugger->UpdateDisplay ();

  return rb;
}

iRigidBody* Simple::SpawnCapsule (float length, float radius)
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the capsule mesh factory.
  csRef<iMeshFactoryWrapper> capsuleFact = engine->CreateMeshFactory (
  	"crystalspace.mesh.object.genmesh", "capsuleFact");
  if (!capsuleFact)
  {
    ReportError ("Error creating mesh object factory!");
    return 0;
  }

  csRef<iGeneralFactoryState> gmstate =
    scfQueryInterface<iGeneralFactoryState> (capsuleFact->GetMeshObjectFactory ());
  gmstate->GenerateCapsule (length, radius, 10);
  capsuleFact->HardTransform (
        csReversibleTransform (csYRotMatrix3 (PI/2), csVector3 (0)));

  // Create the mesh.
  csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (
				            capsuleFact, "capsule", room));
  iMaterialWrapper* mat = engine->GetMaterialList ()->FindByName ("spark");
  mesh->GetMeshObject ()->SetMaterialWrapper (mat);

  // Create a body and attach the mesh.
  csRef<iRigidBody> rb = dynamicSystem->CreateBody ();
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
  csRef<iRigidBody> rb = dynamicSystem->CreateBody ();
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

iRigidBody* Simple::SpawnCompound ()
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the capsule mesh factory.
  csRef<iMeshFactoryWrapper> capsuleFact = engine->CreateMeshFactory (
  	"crystalspace.mesh.object.genmesh", "capsuleFact");
  if (!capsuleFact)
  {
    ReportError ("Error creating mesh object factory!");
    return 0;
  }

  csRef<iGeneralFactoryState> gmstate =
    scfQueryInterface<iGeneralFactoryState> (capsuleFact->GetMeshObjectFactory ());
  gmstate->GenerateCapsule (0.7f, 0.3f, 10);
  capsuleFact->HardTransform
    (csReversibleTransform (csYRotMatrix3 (PI/2), csVector3 (-0.2f)));

  // Create the mesh.
  csRef<iMeshWrapper> capsuleMesh (engine->CreateMeshWrapper
			    (capsuleFact, "capsule", room));
  iMaterialWrapper* mat = engine->GetMaterialList ()->FindByName ("spark");
  capsuleMesh->GetMeshObject ()->SetMaterialWrapper (mat);

  // Create the sphere mesh factory.
  csRef<iMeshFactoryWrapper> sphereFact = engine->CreateMeshFactory (
  	"crystalspace.mesh.object.genmesh", "sphereFact");
  if (!sphereFact)
  {
    ReportError ("Error creating mesh object factory!");
    return 0;
  }

  gmstate =
    scfQueryInterface<iGeneralFactoryState> (sphereFact->GetMeshObjectFactory ());
  gmstate->GenerateSphere (csEllipsoid (csVector3 (-0.2f, 0.3f, -0.1f), csVector3 (0.3f)), 16);

  // Create the mesh.
  csRef<iMeshWrapper> sphereMesh (engine->CreateMeshWrapper
			    (sphereFact, "sphere", room));
  sphereMesh->GetMeshObject ()->SetMaterialWrapper (mat);

  // Create the box mesh factory.
  csRef<iMeshFactoryWrapper> boxFact = engine->CreateMeshFactory (
  	"crystalspace.mesh.object.genmesh", "boxFact");
  if (!boxFact)
  {
    ReportError ("Error creating mesh object factory!");
    return 0;
  }

  gmstate =
    scfQueryInterface<iGeneralFactoryState> (boxFact->GetMeshObjectFactory ());
  gmstate->GenerateBox (csBox3 (csVector3 (0.1f, -0.1f, 0.2f), csVector3 (0.3f)));

  // Create the mesh.
  csRef<iMeshWrapper> boxMesh (engine->CreateMeshWrapper
			    (boxFact, "box", room));
  boxMesh->GetMeshObject ()->SetMaterialWrapper (mat);

  // Set the sphere and box meshes as child nodes of the capsule mesh
  sphereMesh->QuerySceneNode ()->SetParent (capsuleMesh->QuerySceneNode ());
  boxMesh->QuerySceneNode ()->SetParent (capsuleMesh->QuerySceneNode ());

  // Create a body and attach the capsule mesh.
  csRef<iRigidBody> rb = dynamicSystem->CreateBody ();
  rb->SetProperties (4.0f, csVector3 (0.0f), csMatrix3 ());
  rb->SetPosition (tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 1));
  rb->AttachMesh (capsuleMesh);

  // Create and attach the colliders.
  csOrthoTransform t;
  t.SetOrigin (csVector3 (-0.2f));
  rb->AttachColliderCapsule (0.7f, 0.3f, t, 10, 1, 0.8f);
  rb->AttachColliderSphere (0.3f, csVector3 (-0.2f, 0.3f, -0.1f), 10, 1, 0.8f);
  t.SetOrigin (csVector3 (0.2f, 0.1f, 0.25f));
  rb->AttachColliderBox (csVector3 (0.2f, 0.4f, 0.1f), t, 10, 1, 0.8f);

  // Fling the body.
  rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 6));
  rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 0, 0));

  // Update the display of the dynamics debugger.
  dynamicsDebugger->UpdateDisplay ();

  return rb;
}

iJoint* Simple::SpawnJointed ()
{
  // Create and position two rigid bodies
  iRigidBody* rb1 = SpawnBox ();
  rb1->SetPosition (rb1->GetPosition () +
    rb1->GetOrientation () * csVector3 (-.5f, 0.0f, 0.0f));
  iRigidBody* rb2 = SpawnSphere ();
  rb2->SetPosition (rb2->GetPosition () +
    rb2->GetOrientation () * csVector3 (.5f, 0.0f, 0.0f));

  // Create a joint and attach the two bodies to it.
  csRef<iJoint> joint = dynamicSystem->CreateJoint ();
  joint->Attach (rb1, rb2, false);

  // Set the transform of the joint at the midpoint between the two attached bodies
  csOrthoTransform jointTransform (csMatrix3 (),
				   rb2->GetOrientation () * csVector3 (-.6f, 0.5f, 0.0f));
  joint->SetTransform (jointTransform);

  // Constrain the translations of the joint (locked on all axis).
  joint->SetTransConstraints (true, true, true, false);

  // Constrain the rotations of the joint (free along X axis, locked for Y and Z).
  joint->SetMinimumAngle (csVector3 (1.0f, 0.0f, 0.0f), false);
  joint->SetMaximumAngle (csVector3 (-1.0f, 0.0f, 0.0f), false);
  joint->SetRotConstraints (false, false, false, false);

  // Add a motor to the joint
  joint->SetDesiredVelocity (csVector3 (10.0f, 0.0f, 0.0f), false);
  joint->SetMaxForce (csVector3 (0.1f, 0.0f, 0.0f), false);

  joint->RebuildJoint ();

  // Update the display of the dynamics debugger
  dynamicsDebugger->UpdateDisplay ();

  return joint;
}

void ConstraintJoint (iJoint* joint)
{
  // The translations are fully constrained.
  joint->SetTransConstraints (true, true, true, false);

  // The rotations are bounded
  joint->SetMinimumAngle (csVector3 (-PI/4.0, -PI/6.0, -PI/6.0), false);
  joint->SetMaximumAngle (csVector3 (PI/4.0, PI/6.0, PI/6.0), false);
  joint->SetRotConstraints (false, false, false, false);
}

void Simple::SpawnChain ()
{
  iRigidBody* rb1 = SpawnBox ();
  csVector3 initPos = rb1->GetPosition () + csVector3 (0.0f, 5.0f, 0.0f);
  rb1->MakeStatic ();
  rb1->SetPosition (initPos);

  csVector3 offset (0.0f, 1.3f, 0.0f);

  iRigidBody* rb2 = SpawnCapsule (0.4f, 0.3f);
  rb2->SetLinearVelocity (csVector3 (0.0f));
  rb2->SetAngularVelocity (csVector3 (0.0f));
  rb2->SetPosition (initPos - offset);
  rb2->SetOrientation (csXRotMatrix3 (PI / 2.0f));

  iRigidBody* rb3 = SpawnBox ();
  rb3->SetLinearVelocity (csVector3 (0.0f));
  rb3->SetAngularVelocity (csVector3 (0.0f));
  rb3->SetPosition (initPos - 2.0f * offset);

  iRigidBody* rb4 = SpawnCapsule (0.4f, 0.3f);
  rb4->SetLinearVelocity (csVector3 (0.0f));
  rb4->SetAngularVelocity (csVector3 (0.0f));
  rb4->SetPosition (initPos - 3.0f * offset);
  rb4->SetOrientation (csXRotMatrix3 (PI / 2.0f));

  iRigidBody* rb5 = SpawnBox ();
  rb5->SetLinearVelocity (csVector3 (0.0f));
  rb5->SetAngularVelocity (csVector3 (0.0f));
  rb5->SetPosition (initPos - 4.0f * offset);

  // Create joints and attach bodies.
  csOrthoTransform jointTransform;
  csRef<iJoint> joint;

  joint = dynamicSystem->CreateJoint ();
  jointTransform.Identity ();
  jointTransform.SetOrigin (initPos - csVector3 (0.0f, 0.6f, 0.0f));
  jointTransform = jointTransform * rb2->GetTransform ().GetInverse ();
  joint->SetTransform (jointTransform);
  joint->Attach (rb1, rb2, false);
  ConstraintJoint (joint);
  joint->RebuildJoint ();

  joint = dynamicSystem->CreateJoint ();
  jointTransform.Identity ();
  jointTransform.SetOrigin (initPos - csVector3 (0.0f, 0.6f, 0.0f) - offset);
  jointTransform = jointTransform * rb3->GetTransform ().GetInverse ();
  joint->SetTransform (jointTransform);
  joint->Attach (rb2, rb3, false);
  ConstraintJoint (joint);
  joint->RebuildJoint ();

  joint = dynamicSystem->CreateJoint ();
  jointTransform.Identity ();
  jointTransform.SetOrigin (initPos - csVector3 (0.0f, 0.6f, 0.0f) - 2.0f * offset);
  jointTransform = jointTransform * rb4->GetTransform ().GetInverse ();
  joint->SetTransform (jointTransform);
  joint->Attach (rb3, rb4, false);
  ConstraintJoint (joint);
  joint->RebuildJoint ();

  joint = dynamicSystem->CreateJoint ();
  jointTransform.Identity ();
  jointTransform.SetOrigin (initPos - csVector3 (0.0f, 0.6f, 0.0f) - 3.0f * offset);
  jointTransform = jointTransform * rb5->GetTransform ().GetInverse ();
  joint->SetTransform (jointTransform);
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

  csRef<CS::Mesh::iAnimatedMeshFactory> animeshFactory =
    scfQueryInterface<CS::Mesh::iAnimatedMeshFactory>
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

  csRef<CS::Animation::iBodyManager> bodyManager =
    csQueryRegistry<CS::Animation::iBodyManager> (GetObjectRegistry ());
  CS::Animation::iBodySkeleton* bodySkeleton = bodyManager->FindBodySkeleton ("frankie_body");
  if (!bodySkeleton)
  {
    ReportError ("Can't find Frankie's body description!");
    return;
  }

  // Create bone chain
  CS::Animation::iBodyChain* chain = bodySkeleton->CreateBodyChain
    ("chain", animeshFactory->GetSkeletonFactory ()->FindBone ("Frankie_Main"),
     animeshFactory->GetSkeletonFactory ()->FindBone ("CTRL_Head"),
     animeshFactory->GetSkeletonFactory ()->FindBone ("Tail_8"), 0);

  // Create ragdoll animation node factory
  csRef<CS::Animation::iSkeletonRagdollNodeFactory> ragdollFactory =
    ragdollManager->CreateAnimNodeFactory ("frankie_ragdoll",
					   bodySkeleton, dynamicSystem);
  ragdollFactory->AddBodyChain (chain, CS::Animation::STATE_DYNAMIC);

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
  csRef<CS::Mesh::iAnimatedMesh> animesh =
    scfQueryInterface<CS::Mesh::iAnimatedMesh> (ragdollMesh->GetMeshObject ());

  // Close the eyes of Frankie as he is dead
  csRef<CS::Mesh::iAnimatedMeshFactory> animeshFactory =
    scfQueryInterface<CS::Mesh::iAnimatedMeshFactory>
    (meshfact->GetMeshObjectFactory ());

  animesh->SetMorphTargetWeight (animeshFactory->FindMorphTarget ("eyelids_closed"), 0.7f);

  // Set initial position of the body
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();
  ragdollMesh->QuerySceneNode ()->GetMovable ()->SetPosition (
                  tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 1));

  // Start the ragdoll anim node
  CS::Animation::iSkeletonAnimNode* root = animesh->GetSkeleton ()->GetAnimationPacket ()->
    GetAnimationRoot ();

  csRef<CS::Animation::iSkeletonRagdollNode> ragdoll =
    scfQueryInterfaceSafe<CS::Animation::iSkeletonRagdollNode> (root);

  // Fling the body.
  // (start the ragdoll node before so that the rigid bodies are created)
  ragdoll->Play ();
  for (uint i = 0; i < ragdoll->GetBoneCount (CS::Animation::STATE_DYNAMIC); i++)
  {
    CS::Animation::BoneID boneID = ragdoll->GetBone (CS::Animation::STATE_DYNAMIC, i);
    iRigidBody* rb = ragdoll->GetBoneRigidBody (boneID);
    rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 5));
    rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 5, 0));
  }

  // Update the display of the dynamics debugger
  dynamicsDebugger->UpdateDisplay ();
}

void Simple::SpawnRope ()
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Spawn a box
  iRigidBody* box = SpawnBox ();

  // First example using ropes defined by their extremities
#if 1
  // Spawn a first rope and attach it to the box
  CS::Physics::Bullet::iSoftBody* body = bulletDynamicSystem->CreateRope
    (tc.GetOrigin () + tc.GetT2O () * csVector3 (-2, 2, 0),
     tc.GetOrigin () + tc.GetT2O () * csVector3 (-0.2f, 0, 1), 20);
  body->SetMass (2.0f);
  body->SetRigidity (0.95f);
  body->AnchorVertex (0);
  body->AnchorVertex (body->GetVertexCount () - 1, box);

  // Spawn a second rope and attach it to the box
  body = bulletDynamicSystem->CreateRope
    (tc.GetOrigin () + tc.GetT2O () * csVector3 (2, 2, 0),
     tc.GetOrigin () + tc.GetT2O () * csVector3 (0.2f, 0, 1), 20);
  body->SetMass (1.0f);
  body->SetRigidity (0.95f);
  body->AnchorVertex (0);
  body->AnchorVertex (body->GetVertexCount () - 1, box);

  // Second example using ropes defined by the position of each of their vertices
#else
  // Spawn a first rope and attach it to the box
  {
    // Define the positions of the vertices
    size_t vertexCount = 10;
    CS_ALLOC_STACK_ARRAY(csVector3, nodes, vertexCount);
    nodes[0] = tc.GetOrigin () + tc.GetT2O () * csVector3 (-2, 2, 0);
    csVector3 step = (tc.GetT2O () * csVector3 (-0.2f, 0, 1) -
		      tc.GetT2O () * csVector3 (-2, 2, 0)) / (((float) vertexCount) - 1);
    for (size_t i = 1; i < vertexCount; i++)
      nodes[i] = nodes[0] + ((int) (i % 2)) * csVector3 (-0.2f, 0, 0) + ((int) i) * step;

    // Create the soft body
    CS::Physics::Bullet::iSoftBody* body = bulletDynamicSystem->CreateRope
      (nodes, vertexCount);
    body->SetMass (2.0f);
    body->SetRigidity (0.95f);
    body->AnchorVertex (0);
    body->AnchorVertex (body->GetVertexCount () - 1, box);
  }

  // Spawn a second rope and attach it to the box
  {
    // Define the positions of the vertices
    size_t vertexCount = 10;
    CS_ALLOC_STACK_ARRAY(csVector3, nodes, vertexCount);
    nodes[0] = tc.GetOrigin () + tc.GetT2O () * csVector3 (2, 2, 0);
    csVector3 step = (tc.GetT2O () * csVector3 (0.2f, 0, 1) -
		      tc.GetT2O () * csVector3 (2, 2, 0)) / (((float) vertexCount) - 1);
    for (size_t i = 1; i < vertexCount; i++)
      nodes[i] = nodes[0] + ((int) (i % 2)) * csVector3 (0.2f, 0, 0) + ((int) i) * step;

    // Create the soft body
    CS::Physics::Bullet::iSoftBody* body = bulletDynamicSystem->CreateRope
      (nodes, vertexCount);
    body->SetMass (2.0f);
    body->SetRigidity (0.95f);
    body->AnchorVertex (0);
    body->AnchorVertex (body->GetVertexCount () - 1, box);
  }
#endif
}

void Simple::SpawnCloth ()
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the cloth
  CS::Physics::Bullet::iSoftBody* body = bulletDynamicSystem->CreateCloth
    (tc.GetOrigin () + tc.GetT2O () * csVector3 (-2, 2, 1),
     tc.GetOrigin () + tc.GetT2O () * csVector3 (2, 2, 1),
     tc.GetOrigin () + tc.GetT2O () * csVector3 (-2, 0, 1),
     tc.GetOrigin () + tc.GetT2O () * csVector3 (2, 0, 1),
     10, 10, true);
  body->SetMass (5.0f);

  // Attach the two top corners
  body->AnchorVertex (0);
  body->AnchorVertex (9);

  // Create the cloth mesh factory
  csRef<iMeshFactoryWrapper> clothFact =
    CS::Physics::Bullet::SoftBodyHelper::CreateClothGenMeshFactory
    (GetObjectRegistry (), "clothFact", body);
  csRef<iGeneralFactoryState> gmstate = scfQueryInterface<iGeneralFactoryState>
    (clothFact->GetMeshObjectFactory ());

  // Create the mesh
  gmstate->SetAnimationControlFactory (softBodyAnimationFactory);
  csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (
  			            clothFact, "cloth_body", room));
  iMaterialWrapper* mat = CS::Material::MaterialBuilder::CreateColorMaterial
    (GetObjectRegistry (), "cloth", csColor4 (1.0f, 0.0f, 0.0f, 1.0f));
  mesh->GetMeshObject ()->SetMaterialWrapper (mat);

  // Init the animation control for the animation of the genmesh
  csRef<iGeneralMeshState> meshState =
    scfQueryInterface<iGeneralMeshState> (mesh->GetMeshObject ());
  csRef<CS::Animation::iSoftBodyAnimationControl> animationControl =
    scfQueryInterface<CS::Animation::iSoftBodyAnimationControl> (meshState->GetAnimationControl ());
  animationControl->SetSoftBody (body, true);
}

void Simple::SpawnSoftBody ()
{
  // Create the ball mesh factory.
  csRef<iMeshFactoryWrapper> ballFact = engine->CreateMeshFactory(
  	"crystalspace.mesh.object.genmesh", "ballFact");
  if (!ballFact)
  {
    ReportError ("Error creating mesh object factory!");
    return;
  }

  csRef<iGeneralFactoryState> gmstate = scfQueryInterface<
    iGeneralFactoryState> (ballFact->GetMeshObjectFactory ());
  const float r (rand()%5/10. + .4);
  csVector3 radius (r, r, r);
  csEllipsoid ellips (csVector3 (0), radius);
  gmstate->GenerateSphere (ellips, 16);

  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the soft body
  CS::Physics::Bullet::iSoftBody* body = bulletDynamicSystem->CreateSoftBody
    (gmstate, csOrthoTransform (csMatrix3 (), csVector3 (0.0f, 0.0f, 1.0f)) * tc);
  // This would have worked too
  //iBulletSoftBody* body = bulletDynamicSystem->CreateSoftBody
  //  (gmstate->GetVertices (), gmstate->GetVertexCount (),
  //   gmstate->GetTriangles (), gmstate->GetTriangleCount ());
  body->SetMass (2.0f);
  body->SetRigidity (0.8f);

  // Create the mesh
  gmstate->SetAnimationControlFactory (softBodyAnimationFactory);
  csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (
  			            ballFact, "soft_body", room));
  iMaterialWrapper* mat = engine->GetMaterialList ()->FindByName ("spark");
  mesh->GetMeshObject ()->SetMaterialWrapper (mat);

  // Init the animation control for the animation of the genmesh
  csRef<iGeneralMeshState> meshState =
    scfQueryInterface<iGeneralMeshState> (mesh->GetMeshObject ());
  csRef<CS::Animation::iSoftBodyAnimationControl> animationControl =
    scfQueryInterface<CS::Animation::iSoftBodyAnimationControl> (meshState->GetAnimationControl ());
  animationControl->SetSoftBody (body);

  // Fling the body.
  body->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 5));
  // This would have worked too
  //for (size_t i = 0; i < body->GetVertexCount (); i++)
  //  body->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 5), i);
}

void Simple::CreateWalls (const csVector3& /*radius*/)
{
  // Default behavior from DemoApplication for the creation of the scene
  if (!DemoApplication::CreateRoom ())
    return;

  // First we make a primitive for our geometry.
  using namespace CS::Geometry;
  DensityTextureMapper mapper (0.3f);
  TesselatedBox box (csVector3 (-5, -5, -5), csVector3 (5, 5, 5));
  box.SetLevel (3);
  box.SetMapper (&mapper);
  box.SetFlags (Primitives::CS_PRIMBOX_INSIDE);

  // Now we make a factory and a mesh at once.
  walls = GeneralMeshBuilder::CreateFactoryAndMesh (
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
  dynamicSystem->AttachColliderMesh (walls, t, 10, 1);
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
  csRef<iDynamicsSystemCollider> collider = dynamicSystem->CreateCollider ();
  collider->CreateBoxGeometry (size);
  collider->SetTransform (t);

  t.SetOrigin(csVector3(-10.0f, 0.0f, 0.0f));
  collider = dynamicSystem->CreateCollider ();
  collider->CreateBoxGeometry (size);
  collider->SetTransform (t);

  t.SetOrigin(csVector3(0.0f, 10.0f, 0.0f));
  collider = dynamicSystem->CreateCollider ();
  collider->CreateBoxGeometry (size);
  collider->SetTransform (t);

  // If we use the Bullet plugin, then use a plane collider for the floor
  // Also, soft bodies don't work well with planes, so use a box in this case
  if (phys_engine_id == ODE_ID || isSoftBodyWorld)
  {
    t.SetOrigin(csVector3(0.0f, -10.0f, 0.0f));
    dynamicSystem->AttachColliderBox (size, t, 10.0f, 0.0f);
  }
  else
    dynamicSystem->AttachColliderPlane (csPlane3 (csVector3 (0.0f, 1.0f, 0.0f), -5.0f),
					10.0f, 0.0f);

  t.SetOrigin(csVector3(0.0f, 0.0f, 10.0f));
  dynamicSystem->AttachColliderBox (size, t, 10.0f, 0.0f);

  t.SetOrigin(csVector3(0.0f, 0.0f, -10.0f));
  dynamicSystem->AttachColliderBox (size, t, 10.0f, 0.0f);

  // Set our own lights
  room->SetDynamicAmbientLight (csColor (0.3, 0.3, 0.3));

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

  // Setup the sector
  room = engine->FindSector ("room");
  view->GetCamera ()->SetSector (room);
  engine->Prepare ();

  // Find the terrain mesh
  csRef<iMeshWrapper> terrainWrapper = engine->FindMeshObject ("Terrain");
  if (!terrainWrapper)
  {
    ReportError("Error cannot find the terrain mesh!");
    return;
  }

  csRef<iTerrainSystem> terrain =
    scfQueryInterface<iTerrainSystem> (terrainWrapper->GetMeshObject ());
  if (!terrain)
  {
    ReportError("Error cannot find the terrain interface!");
    return;
  }

  // Create a terrain collider for each cell of the terrain
  bulletDynamicSystem->AttachColliderTerrain (terrain);
}

//---------------------------------------------------------------------------

CS_IMPLEMENT_APPLICATION

int main (int argc, char* argv[])
{
  return Simple ().Main(argc, argv);
}
