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
#include "iutil/visualdebug.h"

#include "avatartest.h"
#include "frankie.h"
#include "krystal.h"
#include "sintel.h"

#define MODEL_FRANKIE 1
#define MODEL_KRYSTAL 2
#define MODEL_SINTEL 3

AvatarTest::AvatarTest ()
  : DemoApplication ("CrystalSpace.AvatarTest", "avatartest",
		     "avatartest <OPTIONS>",
		     "Tests on the animation of objects iAnimatedMesh."),
    avatarScene (0), dynamicsDebugMode (DYNDEBUG_NONE)
{
  // Configure the options for DemoApplication

  // Set the camera mode
  cameraHelper.SetCameraMode (CS::Demo::CSDEMO_CAMERA_ROTATE);

  // Command line options
  commandLineHelper.AddCommandLineOption
    ("scene=<name>", "Set the starting scene (frankie, krystal, sintel)");
  commandLineHelper.AddCommandLineOption
    ("disable-physics", "Disable the physical animations");
  commandLineHelper.AddCommandLineOption
    ("disable-soft", "Disable the soft bodies");
}

AvatarTest::~AvatarTest ()
{
  delete avatarScene;
}

csVector3 AvatarTest::GetCameraStart ()
{
  if (avatarScene)
    return avatarScene->GetCameraStart ();

  return csVector3 (0.0f);
}

csVector3 AvatarTest::GetCameraTarget ()
{
  if (avatarScene)
    return avatarScene->GetCameraTarget ();

  return csVector3 (0.0f);
}

float AvatarTest::GetCameraMinimumDistance ()
{
  if (avatarScene)
    return avatarScene->GetCameraMinimumDistance ();

  return 0.1f;
}

void AvatarTest::Frame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsedTime = vc->GetElapsedTicks ();
  const float speed = elapsedTime / 1000.0f;

  // Step the dynamic simulation
  if (physicsEnabled)
    dynamics->Step (speed * avatarScene->GetSimulationSpeed ());

  // Update the avatar
  avatarScene->Frame ();

  // Update the information on the current state of the application
  avatarScene->UpdateStateDescription ();

  // Default behavior from DemoApplication
  DemoApplication::Frame ();

  // Post-frame process
  avatarScene->PostFrame ();

  // Display the Bullet debug information
  if (avatarScene->HasPhysicalObjects ()
      && dynamicsDebugMode == DYNDEBUG_BULLET)
    bulletDynamicSystem->DebugDraw (view);
}

bool AvatarTest::OnKeyboard (iEvent &ev)
{
  // Default behavior from DemoApplication
  DemoApplication::OnKeyboard (ev);

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
      cameraHelper.ResetCamera ();

      // Toggle the debug mode of the dynamic system
      if (physicsEnabled)
      {
	dynamicsDebugMode = DYNDEBUG_NONE;
	dynamicsDebugger->SetDebugDisplayMode (false);
      }

      return true;
    }

    // Toggle the debug mode of the dynamic system
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'd'
	     && physicsEnabled && avatarScene->HasPhysicalObjects ())
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
	dynamicsDebugMode = DYNDEBUG_BULLET;
	dynamicsDebugger->SetDebugDisplayMode (false);
	animeshObject->GetMeshWrapper ()->GetFlags ().Reset (CS_ENTITY_INVISIBLEMESH);
      }

      else if (dynamicsDebugMode == DYNDEBUG_BULLET)
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
  if (avatarScene->OnMouseDown (ev))
    return true;

  // Default behavior from DemoApplication
  return DemoApplication::OnMouseDown (ev);
}

bool AvatarTest::OnMouseUp (iEvent& ev)
{
  if (avatarScene->OnMouseUp (ev))
    return true;

  // Default behavior from DemoApplication
  return DemoApplication::OnMouseUp (ev);
}

bool AvatarTest::HitBeamAnimatedMesh (csVector3& isect, csVector3& direction, int& triangle)
{
  if (!avatarScene)
    return false;

  csVector2 v2d (mouseX, g2d->GetHeight () - mouseY);
  iCamera* camera = view->GetCamera ();
  csVector3 v3d = camera->InvPerspective (v2d, 10000);
  csVector3 startBeam = camera->GetTransform ().GetOrigin ();
  csVector3 endBeam = camera->GetTransform ().This2Other (v3d);

  direction = endBeam - startBeam;
  direction.Normalize ();

  csRef<iMeshObject> mesh = scfQueryInterface<iMeshObject> (avatarScene->animesh);
  iMovable* movable = mesh->GetMeshWrapper ()->GetMovable ();
  startBeam = movable->GetTransform ().Other2This (startBeam);
  endBeam = movable->GetTransform ().Other2This (endBeam);

  return mesh->HitBeamObject (startBeam, endBeam, isect, nullptr, &triangle);
}

bool AvatarTest::OnInitialize (int argc, char* argv[])
{
  // Default behavior from DemoApplication
  if (!DemoApplication::OnInitialize (argc, argv))
    return false;

  if (!csInitializer::RequestPlugins (GetObjectRegistry (),
    CS_REQUEST_PLUGIN ("crystalspace.mesh.animesh.animnode.debug",
		       CS::Animation::iSkeletonDebugNodeManager),
    //CS_REQUEST_PLUGIN ("crystalspace.mesh.animesh.animnode.ik.physical",
    CS_REQUEST_PLUGIN ("crystalspace.mesh.animesh.animnode.ik.ccd",
		       CS::Animation::iSkeletonIKNodeManager),
    CS_REQUEST_PLUGIN ("crystalspace.mesh.animesh.animnode.lookat",
		       CS::Animation::iSkeletonLookAtNodeManager),
    CS_REQUEST_PLUGIN ("crystalspace.mesh.animesh.animnode.speed",
		       CS::Animation::iSkeletonSpeedNodeManager),
    CS_REQUEST_PLUGIN ("crystalspace.decal.manager",
		       iDecalManager),
    CS_REQUEST_END))
    return ReportError ("Failed to initialize plugins!");

  csBaseEventHandler::Initialize (GetObjectRegistry ());
  if (!RegisterQueue (GetObjectRegistry (), csevAllEvents (GetObjectRegistry ())))
    return ReportError ("Failed to set up event handler!");

  // Check if physical effects are enabled
  csRef<iCommandLineParser> clp =
    csQueryRegistry<iCommandLineParser> (GetObjectRegistry ());
  physicsEnabled = !clp->GetBoolOption ("disable-physics", false);

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
    debuggerManager = csLoadPlugin<CS::Debug::iDynamicsDebuggerManager>
      (plugmgr, "crystalspace.dynamics.debug");

    if (!debuggerManager)
    {
      ReportWarning
	("Can't load Dynamics Debugger plugin, continuing with reduced functionalities");
      physicsEnabled = false;
      break;
    }

    // Load the ragdoll plugin
    ragdollManager = csLoadPlugin<CS::Animation::iSkeletonRagdollNodeManager>
      (plugmgr, "crystalspace.mesh.animesh.animnode.ragdoll");

    if (!ragdollManager)
    {
      ReportWarning
	("Can't load ragdoll plugin, continuing with reduced functionalities");
      physicsEnabled = false;
      break;
    }

    // Check whether the soft bodies are enabled or not
    softBodiesEnabled = !clp->GetBoolOption ("disable-soft", false);

    // Load the soft body animation control plugin & factory
    if (softBodiesEnabled)
    {
      csRef<iPluginManager> plugmgr = 
	csQueryRegistry<iPluginManager> (GetObjectRegistry ());
      softBodyAnimationType = csLoadPlugin<CS::Animation::iSoftBodyAnimationControlType>
	(plugmgr, "crystalspace.dynamics.softanim");

      if (!softBodyAnimationType)
      {
	ReportWarning
	  ("Can't load soft body animation plugin, continuing with reduced functionalities");
	softBodiesEnabled = false;
	break;
      }
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
      csPrintf ("Given scene (%s) is not one of {%s, %s, %s}. Falling back to Frankie\n",
		CS::Quote::Single (sceneName.GetData ()),
		CS::Quote::Single ("frankie"),
		CS::Quote::Single ("krystal"),
		CS::Quote::Single ("sintel"));
      avatarSceneType = MODEL_FRANKIE;
    }
  }

  return true;
}

bool AvatarTest::Application ()
{
  // Default behavior from DemoApplication
  if (!DemoApplication::Application ())
    return false;

  // Find references to the plugins of the animation nodes
  debugManager = csQueryRegistry<CS::Animation::iSkeletonDebugNodeManager> (GetObjectRegistry ());
  if (!debugManager) return ReportError("Failed to locate iSkeletonDebugNodeManager plugin!");

  IKNodeManager = csQueryRegistry<CS::Animation::iSkeletonIKNodeManager> (GetObjectRegistry ());
  if (!IKNodeManager) return ReportError("Failed to locate iSkeletonIKNodeManager plugin!");

  lookAtManager = csQueryRegistry<CS::Animation::iSkeletonLookAtNodeManager> (GetObjectRegistry ());
  if (!lookAtManager) return ReportError("Failed to locate iSkeletonLookAtNodeManager plugin!");

  speedManager =
    csQueryRegistry<CS::Animation::iSkeletonSpeedNodeManager> (GetObjectRegistry ());
  if (!speedManager)
    return ReportError("Failed to locate iSkeletonSpeedNodeManager plugin!");

  // Find a reference to the decal plugin
  decalManager = csQueryRegistry<iDecalManager> (GetObjectRegistry ());

  // Default behavior from DemoApplication for the creation of the scene
  if (!DemoApplication::CreateRoom ())
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
      // Set up some deactivation and dampening parameters
      dynamicSystem->SetAutoDisableParams (1.6f, 2.5f, 0, 0.8f);
      dynamicSystem->SetLinearDampener (0.05f);
      dynamicSystem->SetRollingDampener (0.85f);

      // Find the Bullet interface of the dynamic system
      bulletDynamicSystem =
	scfQueryInterface<CS::Physics::Bullet::iDynamicSystem> (dynamicSystem);

      // We have some objects of size smaller than 0.035 units, so we scale up the
      // whole world for a better behavior of the dynamic simulation.
      bulletDynamicSystem->SetInternalScale (10.0f);

      // The physical scene are rather complex in this demo. We therefore use high
      // accuracy/low performance parameters for a better behavior of the dynamic
      // simulation.
      bulletDynamicSystem->SetStepParameters (0.008f, 150, 10);

      // Create the dynamic's debugger
      dynamicsDebugger = debuggerManager->CreateDebugger ();
      dynamicsDebugger->SetDynamicSystem (dynamicSystem);
      dynamicsDebugger->SetDebugSector (room);

      if (softBodiesEnabled)
      {
	// Set the dynamic system as a soft body world in order to animate the skirt
	// of Krystal
	bulletDynamicSystem->SetSoftBodyWorld (true);

	// Create the soft body animation factory
	csRef<iGenMeshAnimationControlFactory> animationFactory =
	  softBodyAnimationType->CreateAnimationControlFactory ();
	softBodyAnimationFactory =
	  scfQueryInterface<CS::Animation::iSoftBodyAnimationControlFactory> (animationFactory);

	// Set up the physical collider for the roof (soft bodies don't like plane
	// colliders, so use a box instead)
	csOrthoTransform t;
	t.SetOrigin(csVector3(0.0f, -50.0f, 0.0f));
	dynamicSystem->AttachColliderBox (csVector3 (100.0f), t, 10.0f, 0.0f);
      }

      // Set up the physical collider for the roof
      else
	dynamicSystem->AttachColliderPlane (csPlane3 (csVector3 (0.0f, 1.0f, 0.0f), 0.0f),
					    10.0f, 0.0f);
    }
  }

  // Create the avatar scene
  if (avatarSceneType == MODEL_KRYSTAL)
    avatarScene = new KrystalScene (this);
  else if (avatarSceneType == MODEL_SINTEL)
    avatarScene = new SintelScene (this);
  else
    avatarScene = new FrankieScene (this);

  if (!avatarScene->CreateAvatar ())
    return false;

  // Initialize the camera position
  cameraHelper.ResetCamera ();

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
