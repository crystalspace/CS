/*
  Copyright (C) 2009-10 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "csgeom/plane3.h"

#include "avatartest.h"
#include "frankie.h"
#include "krystal.h"
#include "sintel.h"

#define MODEL_FRANKIE 1
#define MODEL_KRYSTAL 2
#define MODEL_SINTEL 3

AvatarTest::AvatarTest ()
  : csDemoApplication ("CrystalSpace.AvatarTest", "avatartest",
		       "avatartest <OPTIONS>",
		       "Tests on the animation of objects iAnimatedMesh."),
    avatarScene (0), dynamicsDebugMode (DYNDEBUG_NONE)
{
  // Configure the options for csDemoApplication

  // We manage the camera by ourselves
  cameraMode = CSDEMO_CAMERA_NONE;

  // Command line options
  commandOptions.Push (CommandOption
		       ("scene=<name>",
			"Set the starting scene (frankie, krystal, sintel)"));
  commandOptions.Push (CommandOption
		       ("no_physics", "Disable the physical animations"));
}

AvatarTest::~AvatarTest ()
{
  delete avatarScene;
}

void AvatarTest::Frame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsedTime = vc->GetElapsedTicks ();

  // Now rotate the camera according to keyboard state
  const float speed = elapsedTime / 1000.0f;

  // Compute camera and animesh position
  iCamera* camera = view->GetCamera ();
  csVector3 cameraPosition = camera->GetTransform ().GetOrigin ();
  csVector3 cameraTarget = avatarScene->GetCameraTarget ();
  float minimumDistance = avatarScene->GetCameraMinimumDistance ();

  // Move camera
  if (kbd->GetKeyState (CSKEY_SHIFT))
  {
    // If the user is holding down shift, the Up/Down arrow keys will cause
    // the camera to go forwards and backwards (forward only allowed if camera 
    // not too close). Left/Right arrows work also when shift is hold.
    if (kbd->GetKeyState (CSKEY_UP)
	&& (cameraPosition - cameraTarget).Norm () > minimumDistance)
      camera->Move (CS_VEC_FORWARD * 4 * speed);
    if (kbd->GetKeyState (CSKEY_DOWN))
      camera->Move (CS_VEC_BACKWARD * 4 * speed);
    if (kbd->GetKeyState (CSKEY_RIGHT))
      camera->Move (CS_VEC_RIGHT * 4 * speed);
    if (kbd->GetKeyState (CSKEY_LEFT))
      camera->Move (CS_VEC_LEFT * 4 * speed);
  }
  else
  {
    // Left and right arrows cause the camera to strafe on the X axis; up and 
    // down arrows cause the camera to strafe on the Y axis
    if (kbd->GetKeyState (CSKEY_RIGHT))
      camera->Move (CS_VEC_RIGHT * 4 * speed);
    if (kbd->GetKeyState (CSKEY_LEFT))
      camera->Move (CS_VEC_LEFT * 4 * speed);

    // Avoid gimbal lock of camera
    cameraPosition.Normalize ();
    float cameraDot = cameraPosition * csVector3 (0.0f, 1.0f, 0.0f);
    if (kbd->GetKeyState (CSKEY_UP)
	&& cameraDot < 0.98f)
      camera->Move (CS_VEC_UP * 4 * speed);
    if (kbd->GetKeyState (CSKEY_DOWN)
	&& cameraDot > -0.98f)
      camera->Move (CS_VEC_DOWN * 4 * speed);
  }

  // Make the camera look at the animesh
  camera->GetTransform ().LookAt (cameraTarget - camera->GetTransform ().GetOrigin (),
				  csVector3 (0.0f, 1.0f, 0.0f) );

  // Step the dynamic simulation (we slow down artificially the simulation in
  // order to achieve a 'slow motion' effect)
  if (physicsEnabled)
    dynamics->Step (speed * avatarScene->GetSimulationSpeed ());

  // Update the avatar
  avatarScene->Frame ();

  // Update the information on the current state of the application
  avatarScene->UpdateStateDescription ();

  // Default behavior from csDemoApplication
  csDemoApplication::Frame ();
}

bool AvatarTest::OnKeyboard (iEvent &ev)
{
  // Default behavior from csDemoApplication
  csDemoApplication::OnKeyboard (ev);

  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    // Check for switching of scene
    if (csKeyEventHelper::GetCookedCode (&ev) == 'n')
    {
      delete avatarScene;

      if (avatarSceneType == MODEL_FRANKIE)
      {
	avatarSceneType = MODEL_KRYSTAL;
	avatarScene = new KrystalScene (this);
      }

      else if (avatarSceneType == MODEL_KRYSTAL)
      {
	avatarSceneType = MODEL_SINTEL;
	avatarScene = new SintelScene (this);
      }

      else
      {
	avatarSceneType = MODEL_FRANKIE;
	avatarScene = new FrankieScene (this);
      }

      if (!avatarScene->CreateAvatar ())
      {
	printf ("Problem loading scene. Exiting.\n");
	csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (GetObjectRegistry ()));
	if (q) q->GetEventOutlet()->Broadcast (csevQuit (GetObjectRegistry ()));
	return true;
      }

      // Re-initialize camera position
      view->GetCamera ()->GetTransform ().SetOrigin (avatarScene->GetCameraStart ());

      return true;
    }

    // Toggle the debug mode of the dynamic system
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'd'
	     && physicsEnabled)
    {
      csRef<iMeshObject> animeshObject =
	scfQueryInterface<iMeshObject> (avatarScene->animesh);

      if (dynamicsDebugMode == DYNDEBUG_NONE)
      {
	dynamicsDebugMode = DYNDEBUG_MIXED;
	dynamicsDebugger->SetDebugDisplayMode (true);
	animeshObject->GetMeshWrapper ()->GetFlags ().Reset (CS_ENTITY_INVISIBLEMESH);
      }

      else if (dynamicsDebugMode == DYNDEBUG_MIXED)
      {
	dynamicsDebugMode = DYNDEBUG_COLLIDER;
	dynamicsDebugger->SetDebugDisplayMode (true);
	animeshObject->GetMeshWrapper ()->GetFlags ().Set (CS_ENTITY_INVISIBLEMESH);
      }

      else if (dynamicsDebugMode == DYNDEBUG_COLLIDER)
      {
	dynamicsDebugMode = DYNDEBUG_NONE;
	dynamicsDebugger->SetDebugDisplayMode (false);
	animeshObject->GetMeshWrapper ()->GetFlags ().Reset (CS_ENTITY_INVISIBLEMESH);
      }

      return true;
    }
  }

  return avatarScene->OnKeyboard (ev);
}

bool AvatarTest::OnMouseDown (iEvent& ev)
{
  return avatarScene->OnMouseDown (ev);
}

bool AvatarTest::OnInitialize (int argc, char* argv[])
{
  // Default behavior from csDemoApplication
  if (!csDemoApplication::OnInitialize (argc, argv))
    return false;

  if (!csInitializer::RequestPlugins (GetObjectRegistry (),
    CS_REQUEST_PLUGIN ("crystalspace.mesh.animesh.controllers.lookat",
		       iSkeletonLookAtManager2),
    CS_REQUEST_PLUGIN ("crystalspace.mesh.animesh.controllers.basic",
		       iSkeletonBasicNodesManager2),
    CS_REQUEST_END))
    return ReportError ("Failed to initialize plugins!");

  csBaseEventHandler::Initialize (GetObjectRegistry ());
  if (!RegisterQueue (GetObjectRegistry (), csevAllEvents (GetObjectRegistry ())))
    return ReportError ("Failed to set up event handler!");

  // Check if physical effects are enabled
  csRef<iCommandLineParser> clp =
    csQueryRegistry<iCommandLineParser> (GetObjectRegistry ());
  physicsEnabled = !clp->GetBoolOption ("no_physics", false);

  while (physicsEnabled)
  {
    // Load the Bullet plugin
    csRef<iPluginManager> plugmgr = 
      csQueryRegistry<iPluginManager> (GetObjectRegistry ());
    dynamics = csLoadPlugin<iDynamics> (plugmgr, "crystalspace.dynamics.bullet");

    if (!dynamics)
    {
      ReportWarning
	("Can't load Bullet plugin, continuing with reduced functionalities");
      physicsEnabled = false;
      break;
    }

    // Load the dynamics debugger
    debuggerManager = csLoadPlugin<iDynamicsDebuggerManager>
      (plugmgr, "crystalspace.dynamics.debug");

    if (!debuggerManager)
    {
      ReportWarning
	("Can't load Dynamics Debugger plugin, continuing with reduced functionalities");
      physicsEnabled = false;
      break;
    }

    // Load the ragdoll plugin
    ragdollManager = csLoadPlugin<iSkeletonRagdollManager2>
      (plugmgr, "crystalspace.mesh.animesh.controllers.ragdoll");

    if (!ragdollManager)
    {
      ReportWarning
	("Can't load ragdoll plugin, continuing with reduced functionalities");
      physicsEnabled = false;
      break;
    }

    break;
  }

  // Read which scene to display at first
  csString sceneName = clp->GetOption ("scene");
  if (sceneName.IsEmpty ())
    avatarSceneType = MODEL_FRANKIE;

  else
  {
    if (sceneName == "krystal")
      avatarSceneType = MODEL_KRYSTAL;

    else if (sceneName == "sintel")
      avatarSceneType = MODEL_SINTEL;

    else
    {
      printf ("Given scene ('%s') is not one of {'frankie', 'krystal', 'sintel'}. Falling back to Frankie\n",
	      sceneName.GetData ());
      avatarSceneType = MODEL_FRANKIE;
    }
  }

  return true;
}

bool AvatarTest::Application ()
{
  // Default behavior from csDemoApplication
  if (!csDemoApplication::Application ())
    return false;

  // Find references to the plugins of the animation nodes
  lookAtManager = csQueryRegistry<iSkeletonLookAtManager2> (GetObjectRegistry ());
  if (!lookAtManager) return ReportError("Failed to locate iLookAtManager plugin!");

  basicNodesManager =
    csQueryRegistry<iSkeletonBasicNodesManager2> (GetObjectRegistry ());
  if (!basicNodesManager)
    return ReportError("Failed to locate iSkeletonBasicNodesManager2 plugin!");

  // Default behavior from csDemoApplication for the creation of the scene
  if (!csDemoApplication::CreateRoom ())
    return false;

  // Create the dynamic system
  if (physicsEnabled)
  {
    dynamicSystem = dynamics->CreateSystem ();
    if (!dynamicSystem) 
    {
      ReportWarning
	("Can't create dynamic system, continuing with reduced functionalities");
      physicsEnabled = false;
    }

    else
    {
      // Find the Bullet interface of the dynamic system
      bulletDynamicSystem =
	scfQueryInterface<iBulletDynamicSystem> (dynamicSystem);

      // We have some objects of size smaller than 0.035 units, so we scale up the
      // whole world for a better behavior of the dynamic simulation.
      bulletDynamicSystem->SetInternalScale (10.0f);

      // The ragdoll model of Krystal is rather complex, and the model of Frankie
      // is unstable because of the overlap of its colliders. We therefore use high
      // accuracy/low performance parameters for a better behavior of the dynamic
      // simulation.
      bulletDynamicSystem->SetStepParameters (0.008f, 150, 10);

      // Create the dynamic's debugger
      dynamicsDebugger = debuggerManager->CreateDebugger ();
      dynamicsDebugger->SetDynamicSystem (dynamicSystem);
      dynamicsDebugger->SetDebugSector (room);

      // Set up the physical collider for the roof
      dynamicSystem->AttachColliderPlane (csPlane3 (csVector3 (0.0f, 1.0f, 0.0f), 0.0f),
					  10.0f, 0.0f);
    }
  }

  // Create avatar
  if (avatarSceneType == MODEL_KRYSTAL)
    avatarScene = new KrystalScene (this);
  else if (avatarSceneType == MODEL_SINTEL)
    avatarScene = new SintelScene (this);
  else
    avatarScene = new FrankieScene (this);

  if (!avatarScene->CreateAvatar ())
    return false;

  // Initialize camera position
  view->GetCamera ()->GetTransform ().SetOrigin (avatarScene->GetCameraStart ());

  // Run the application
  Run();

  return true;
}

//---------------------------------------------------------------------------

CS_IMPLEMENT_APPLICATION

int main (int argc, char* argv[])
{
  return AvatarTest ().Main (argc, argv);
}
