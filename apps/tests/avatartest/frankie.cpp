/*
  Copyright (C) 2010 Christian Van Brussel, Communications and Remote
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
#include "frankie.h"

#define LOOKAT_CAMERA 1
#define LOOKAT_POSITION 2
#define LOOKAT_NOTHING 3

#define ROTATION_SLOW 1
#define ROTATION_NORMAL 2
#define ROTATION_IMMEDIATE 3

FrankieScene::FrankieScene (AvatarTest* avatarTest)
  : avatarTest (avatarTest), targetReached (false), lookAtListener (this)
{
  // Define the available keys
  avatarTest->keyDescriptions.DeleteAll ();
  avatarTest->keyDescriptions.Push ("arrow keys: move camera");
  avatarTest->keyDescriptions.Push ("SHIFT-up/down keys: camera closer/farther");
  avatarTest->keyDescriptions.Push ("+/-: walk faster/slower");
  avatarTest->keyDescriptions.Push ("t: toggle 'LookAt' target mode");
  avatarTest->keyDescriptions.Push ("a: toggle 'LookAt: always rotate' mode");
  avatarTest->keyDescriptions.Push ("s: toggle 'LookAt: rotation speed'");
  if (avatarTest->physicsEnabled)
  {
    avatarTest->keyDescriptions.Push ("f: toggle physical tail");
    avatarTest->keyDescriptions.Push ("left mouse: kill Frankie");
    avatarTest->keyDescriptions.Push ("d: display active colliders");
  }
  avatarTest->keyDescriptions.Push ("r: reset scene");
  avatarTest->keyDescriptions.Push ("n: switch to next scene");
}

FrankieScene::~FrankieScene ()
{
  if (!animesh)
    return;

  // Remove the 'lookat' listener
  lookAtNode->RemoveListener (&lookAtListener);

  // Remove the mesh from the scene
  csRef<iMeshObject> animeshObject = scfQueryInterface<iMeshObject> (animesh);
  avatarTest->engine->RemoveObject (animeshObject->GetMeshWrapper ());
}

csVector3 FrankieScene::GetCameraStart ()
{
  return csVector3 (0.0f, 0.0f, -1.25f);
}

float FrankieScene::GetCameraMinimumDistance ()
{
  return 0.75f;
}

csVector3 FrankieScene::GetCameraTarget ()
{
  csRef<iMeshObject> animeshObject = scfQueryInterface<iMeshObject> (animesh);
  csVector3 avatarPosition = animeshObject->GetMeshWrapper ()->QuerySceneNode ()
    ->GetMovable ()->GetTransform ().GetOrigin ();
  avatarPosition.y = 0.35f;
  return avatarPosition;
}

float FrankieScene::GetSimulationSpeed ()
{
  return 0.25f;
}

bool FrankieScene::HasPhysicalObjects ()
{
  return true;
}

void FrankieScene::Frame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = avatarTest->vc->GetElapsedTicks ();

  // Update the morph state (frankie smiles sadistically if no target in view)
  if (targetReached)
    smileWeight -= (float) elapsed_time / 250.0f;
  else 
    smileWeight += (float) elapsed_time / 1500.0f;

  if (smileWeight > 1.0f)
    smileWeight = 1.0f;
  else if (smileWeight < 0.0f)
    smileWeight = 0.0f;

  animesh->SetMorphTargetWeight (animeshFactory->FindMorphTarget ("smile.B"),
				 smileWeight);
  animesh->SetMorphTargetWeight (animeshFactory->FindMorphTarget ("eyebrows_down.B"),
				 smileWeight);
}

bool FrankieScene::OnKeyboard (iEvent &ev)
{
  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    // Toggle the target mode of the 'LookAt' controller
    if (csKeyEventHelper::GetCookedCode (&ev) == 't')
    {
      if (targetMode == LOOKAT_CAMERA)
      {
	lookAtNode->SetTarget (avatarTest->view->GetCamera ()->GetTransform ().GetOrigin ());
	targetMode = LOOKAT_POSITION;
      }

      else if (targetMode == LOOKAT_POSITION)
      {
	lookAtNode->RemoveTarget ();
	targetMode = LOOKAT_NOTHING;
      }

      else
      {
	lookAtNode->SetTarget (avatarTest->view->GetCamera (), csVector3 (0.0f));
	targetMode = LOOKAT_CAMERA;
      }

      return true;
    }

    // Toggle the 'always rotate' option of the 'LookAt' controller
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'a')
    {
      alwaysRotate = !alwaysRotate;
      lookAtNode->SetAlwaysRotate (alwaysRotate);
      return true;
    }

    // Toggle the rotation speed of the 'LookAt' controller
    else if (csKeyEventHelper::GetCookedCode (&ev) == 's')
    {
      if (rotationSpeed == ROTATION_SLOW)
      {
	rotationSpeed = ROTATION_NORMAL;
	lookAtNode->SetMaximumSpeed (5.0f);
      }

      else if (rotationSpeed == ROTATION_NORMAL)
      {
	rotationSpeed = ROTATION_IMMEDIATE;
	lookAtNode->SetMaximumSpeed (0.0f);
      }

      else if (rotationSpeed == ROTATION_IMMEDIATE)
      {
	rotationSpeed = ROTATION_SLOW;
	lookAtNode->SetMaximumSpeed (0.5f);
      }

      return true;
    }

    // Update the walking speed of the 'speed' controller
    else if (csKeyEventHelper::GetCookedCode (&ev) == '+')
    {
      if (currentSpeed < 58)
      {
	currentSpeed += 1;
	speedNode->SetSpeed (((float) currentSpeed) / 10.0f);
      }
      return true;
    }

    else if (csKeyEventHelper::GetCookedCode (&ev) == '-')
    {
      if (currentSpeed > 0)
      {
	currentSpeed -= 1;
	speedNode->SetSpeed (((float) currentSpeed) / 10.0f);
      }
      return true;
    }

    // Toggle the physical animation of the tail
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'f'
	     && avatarTest->physicsEnabled
	     && !frankieDead)
    {
      // If the tail is animated by the classical animation then put the tail chain
      // in kinematic state
      if (ragdollNode->GetBodyChainState (tailChain) == CS_RAGDOLL_STATE_DYNAMIC)
	ragdollNode->SetBodyChainState (tailChain, CS_RAGDOLL_STATE_KINEMATIC);

      // If the tail is animated by the physical simulation then put the tail chain
      // in dynamic state
      else
	ragdollNode->SetBodyChainState (tailChain, CS_RAGDOLL_STATE_DYNAMIC);

      // Update the display of the dynamics debugger
      if (avatarTest->dynamicsDebugMode == DYNDEBUG_COLLIDER
	  || avatarTest->dynamicsDebugMode == DYNDEBUG_MIXED)
	avatarTest->dynamicsDebugger->UpdateDisplay ();
    }

    // Reset of the scene
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'r')
    {
      ResetScene ();
      return true;
    }
  }

  return false;
}

bool FrankieScene::OnMouseDown (iEvent &ev)
{
  if (csMouseEventHelper::GetButton (&ev) == 0
      && avatarTest->physicsEnabled)
  {
    // Trying to kill Frankie

    // We will trace a beam to the point clicked by the mouse to check if
    // something is hit. Let's start by computing the beam end points.
    int mouseX = csMouseEventHelper::GetX (&ev);
    int mouseY = csMouseEventHelper::GetY (&ev);

    csRef<iCamera> camera = avatarTest->view->GetCamera ();
    csVector2 v2d (mouseX, avatarTest->g2d->GetHeight () - mouseY);
    csVector3 v3d = camera->InvPerspective (v2d, 10000);
    csVector3 startBeam = camera->GetTransform ().GetOrigin ();
    csVector3 endBeam = camera->GetTransform ().This2Other (v3d);

    // If Frankie is already dead, simply check for adding a force on him
    if (frankieDead)
    {
      // Trace a physical beam to find if a rigid body was hit
      csBulletHitBeamResult hitResult;
      if (avatarTest->bulletDynamicSystem->HitBeam (startBeam, endBeam, hitResult)
	  && hitResult.bodyType == CS_BULLET_RIGID_BODY)
      {
	csBulletRigidBodyHitBeamResult* physicsResult =
	  (csBulletRigidBodyHitBeamResult*) hitResult.resultData;

	// Apply a big force at the point clicked by the mouse
	csVector3 force = endBeam - startBeam;
	force.Normalize ();
	physicsResult->body->AddForceAtPos (physicsResult->isect, force * 10.0f);
      }

      return true;
    }

    // At first, test with a sector HitBeam if we clicked on an animated mesh
    csSectorHitBeamResult sectorResult = camera->GetSector ()->HitBeam
      (startBeam, endBeam, true);
    if (!sectorResult.mesh)
      return false;

    csRef<iAnimatedMesh> animesh =
      scfQueryInterface<iAnimatedMesh> (sectorResult.mesh->GetMeshObject ());
    if (!animesh)
      return false;

    // OK, it's an animesh, it must be Frankie, let's kill him
    frankieDead = true;

    // Close the eyes of Frankie as he is dead
    animesh->SetMorphTargetWeight
      (animeshFactory->FindMorphTarget ("eyelids_closed"), 0.7f);

    // Set the ragdoll state of the iBodyChain of the body and the tail as dynamic
    ragdollNode->SetBodyChainState (bodyChain, CS_RAGDOLL_STATE_DYNAMIC);
    ragdollNode->SetBodyChainState (tailChain, CS_RAGDOLL_STATE_DYNAMIC);

    // Update the display of the dynamics debugger
    if (avatarTest->dynamicsDebugMode == DYNDEBUG_COLLIDER
	|| avatarTest->dynamicsDebugMode == DYNDEBUG_MIXED)
      avatarTest->dynamicsDebugger->UpdateDisplay ();

    // Fling the body a bit
    const csOrthoTransform& tc = avatarTest->view->GetCamera ()->GetTransform ();
    uint boneCount = ragdollNode->GetBoneCount (CS_RAGDOLL_STATE_DYNAMIC);
    for (uint i = 0; i < boneCount; i++)
    {
      BoneID boneID = ragdollNode->GetBone (CS_RAGDOLL_STATE_DYNAMIC, i);
      iRigidBody* rb = ragdollNode->GetBoneRigidBody (boneID);
      rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0.0f, 0.0f, 0.1f));
    }

    // Trace a physical beam to find which rigid body was hit
    csBulletHitBeamResult hitResult;
    if (avatarTest->bulletDynamicSystem->HitBeam (startBeam, endBeam, hitResult)
	&& hitResult.bodyType == CS_BULLET_RIGID_BODY)
    {
      csBulletRigidBodyHitBeamResult* physicsResult =
	(csBulletRigidBodyHitBeamResult*) hitResult.resultData;

      // Apply a big force at the point clicked by the mouse
      csVector3 force = endBeam - startBeam;
      force.Normalize ();
      physicsResult->body->AddForceAtPos (physicsResult->isect, force * 1.0f);
      physicsResult->body->SetLinearVelocity (tc.GetT2O ()
					      * csVector3 (0.0f, 0.0f, 1.0f));
    }

    return true;
  }

  return false;
}

bool FrankieScene::CreateAvatar ()
{
  printf ("Loading Frankie...\n");

  // Load animesh factory
  csLoadResult rc = avatarTest->loader->Load ("/lib/frankie/frankie.xml");
  if (!rc.success)
    return avatarTest->ReportError ("Can't load Frankie library file!");

  csRef<iMeshFactoryWrapper> meshfact =
    avatarTest->engine->FindMeshFactory ("franky_frankie");
  if (!meshfact)
    return avatarTest->ReportError ("Can't find Frankie's mesh factory!");

  animeshFactory = scfQueryInterface<iAnimatedMeshFactory>
    (meshfact->GetMeshObjectFactory ());
  if (!animeshFactory)
    return avatarTest->ReportError ("Can't find Frankie's animesh factory!");

  // Load bodymesh (animesh's physical properties)
  rc = avatarTest->loader->Load ("/lib/frankie/skelfrankie_body");
  if (!rc.success)
    return avatarTest->ReportError ("Can't load Frankie's body mesh file!");

  csRef<iBodyManager> bodyManager =
    csQueryRegistry<iBodyManager> (avatarTest->GetObjectRegistry ());
  csRef<iBodySkeleton> bodySkeleton = bodyManager->FindBodySkeleton ("frankie_body");
  if (!bodySkeleton)
    return avatarTest->ReportError ("Can't find Frankie's body mesh description!");

  // Create a new animation tree. The structure of the tree is:
  //   + ragdoll controller node (root node - only if physics are enabled)
  //     + 'LookAt' controller node
  //       + 'speed' controller node
  //         + animation nodes for all speeds
  csRef<iSkeletonAnimPacketFactory2> animPacketFactory =
    animeshFactory->GetSkeletonFactory ()->GetAnimationPacket ();

  // Create the 'LookAt' controller
  csRef<iSkeletonLookAtNodeFactory2> lookAtNodeFactory =
    avatarTest->lookAtManager->CreateAnimNodeFactory ("lookat", bodySkeleton);

  // Create the 'idle' animation node
  csRef<iSkeletonAnimationNodeFactory2> idleNodeFactory =
    animPacketFactory->CreateAnimationNode ("idle");
  idleNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_Idle1"));

  // Create the 'walk_slow' animation node
  csRef<iSkeletonAnimationNodeFactory2> walkSlowNodeFactory =
    animPacketFactory->CreateAnimationNode ("walk_slow");
  walkSlowNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_WalkSlow"));

  // Create the 'walk' animation node
  csRef<iSkeletonAnimationNodeFactory2> walkNodeFactory =
    animPacketFactory->CreateAnimationNode ("walk");
  walkNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_Walk"));

  // Create the 'walk_fast' animation node
  csRef<iSkeletonAnimationNodeFactory2> walkFastNodeFactory =
    animPacketFactory->CreateAnimationNode ("walk_fast");
  walkFastNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_WalkFast"));

  // Create the 'footing' animation node
  csRef<iSkeletonAnimationNodeFactory2> footingNodeFactory =
    animPacketFactory->CreateAnimationNode ("footing");
  footingNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_Runs"));

  // Create the 'run_slow' animation node
  csRef<iSkeletonAnimationNodeFactory2> runSlowNodeFactory =
    animPacketFactory->CreateAnimationNode ("run_slow");
  runSlowNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_RunSlow"));

  // Create the 'run' animation node
  csRef<iSkeletonAnimationNodeFactory2> runNodeFactory =
    animPacketFactory->CreateAnimationNode ("run");
  runNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_Run"));

  // Create the 'run_fast' animation node
  csRef<iSkeletonAnimationNodeFactory2> runFastNodeFactory =
    animPacketFactory->CreateAnimationNode ("run_fast");
  runFastNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_RunFaster"));

  // Create the 'run_jump' animation node
  csRef<iSkeletonAnimationNodeFactory2> runJumpNodeFactory =
    animPacketFactory->CreateAnimationNode ("run_jump");
  runJumpNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_RunFast2Jump"));

  // Create the 'speed' controller (and add all animations of Frankie moving at different speeds)
  // Unfortunately, the Frankie animations from 'walk fast' to 'footing'
  // do not blend well together, but this is just an example...
  csRef<iSkeletonSpeedNodeFactory2> speedNodeFactory =
    avatarTest->basicNodesManager->CreateSpeedNodeFactory ("speed");
  speedNodeFactory->AddNode (idleNodeFactory, 0.0f);
  speedNodeFactory->AddNode (walkSlowNodeFactory, 0.4f);
  speedNodeFactory->AddNode (walkNodeFactory, 0.6f);
  speedNodeFactory->AddNode (walkFastNodeFactory, 1.2f);
  speedNodeFactory->AddNode (footingNodeFactory, 1.6f);
  speedNodeFactory->AddNode (runSlowNodeFactory, 2.6f);
  speedNodeFactory->AddNode (runNodeFactory, 3.4f);
  speedNodeFactory->AddNode (runFastNodeFactory, 5.0f);
  speedNodeFactory->AddNode (runJumpNodeFactory, 5.8f);

  lookAtNodeFactory->SetChildNode (speedNodeFactory);

  if (avatarTest->physicsEnabled)
  {
    // Create the ragdoll controller
    csRef<iSkeletonRagdollNodeFactory2> ragdollNodeFactory =
      avatarTest->ragdollManager->CreateAnimNodeFactory ("ragdoll",
					     bodySkeleton, avatarTest->dynamicSystem);
    animPacketFactory->SetAnimationRoot (ragdollNodeFactory);
    ragdollNodeFactory->SetChildNode (lookAtNodeFactory);

    // Create a bone chain for the whole body and add it to the ragdoll controller.
    // The chain will be in kinematic mode when Frankie is alive, and in dynamic state
    // when Frankie has been killed.
    bodyChain = bodySkeleton->CreateBodyChain
      ("body_chain", animeshFactory->GetSkeletonFactory ()->FindBone ("Frankie_Main"),
       animeshFactory->GetSkeletonFactory ()->FindBone ("CTRL_Pelvis"),
       animeshFactory->GetSkeletonFactory ()->FindBone ("CTRL_Head"), 0);
    ragdollNodeFactory->AddBodyChain (bodyChain, CS_RAGDOLL_STATE_KINEMATIC);

    // Create a bone chain for the tail of Frankie and add it to the ragdoll controller.
    // The chain will be in kinematic mode most of the time, and in dynamic mode when the
    // user ask for it with the 'f' key or when Frankie has been killed.
    tailChain = bodySkeleton->CreateBodyChain
      ("tail_chain", animeshFactory->GetSkeletonFactory ()->FindBone ("Tail_1"),
       animeshFactory->GetSkeletonFactory ()->FindBone ("Tail_8"), 0);
    ragdollNodeFactory->AddBodyChain (tailChain, CS_RAGDOLL_STATE_KINEMATIC);
  }

  else
    animPacketFactory->SetAnimationRoot (lookAtNodeFactory);

  // Create the animated mesh
  csRef<iMeshWrapper> avatarMesh =
    avatarTest->engine->CreateMeshWrapper (meshfact, "Frankie",
					   avatarTest->room, csVector3 (0.0f));
  animesh = scfQueryInterface<iAnimatedMesh> (avatarMesh->GetMeshObject ());

  // When the animated mesh is created, the animation nodes are created too.
  // We can therefore set them up now.
  iSkeletonAnimNode2* rootNode =
    animesh->GetSkeleton ()->GetAnimationPacket ()->GetAnimationRoot ();

  // Setup of the LookAt controller
  lookAtNode = scfQueryInterface<iSkeletonLookAtNode2> (rootNode->FindNode ("lookat"));
  lookAtNode->AddListener (&lookAtListener);
  lookAtNode->SetAnimatedMesh (animesh);
  lookAtNode->SetBone (animeshFactory->GetSkeletonFactory ()->FindBone ("CTRL_Head"));
  lookAtNode->SetListenerDelay (0.6f);

  // Setup of the speed controller
  speedNode = scfQueryInterface<iSkeletonSpeedNode2> (rootNode->FindNode ("speed"));

  // Setup of the ragdoll controller
  if (avatarTest->physicsEnabled)
  {
    ragdollNode =
      scfQueryInterface<iSkeletonRagdollNode2> (rootNode->FindNode ("ragdoll"));
    ragdollNode->SetAnimatedMesh (animesh);
  }

  // Reset the scene so as to put the parameters of the animation nodes in a default state
  ResetScene ();

  // Start animation
  rootNode->Play ();

  return true;
}

void FrankieScene::LookAtListener::TargetReached ()
{
  printf ("'LookAt' target reached\n");
  frankieScene->targetReached = true;
}

void FrankieScene::LookAtListener::TargetLost ()
{
  printf ("'LookAt' target lost\n");
  frankieScene->targetReached = false;
}

void FrankieScene::ResetScene ()
{
  // Reset the position of the animesh
  csRef<iMeshObject> animeshObject = scfQueryInterface<iMeshObject> (animesh);
  animeshObject->GetMeshWrapper ()->QuerySceneNode ()->GetMovable ()->SetTransform
    (csOrthoTransform (csMatrix3 (), csVector3 (0.0f)));
  animeshObject->GetMeshWrapper ()->QuerySceneNode ()->GetMovable ()->UpdateMove ();

  frankieDead = false;

  if (avatarTest->physicsEnabled)
  {
    // Set the ragdoll state of the 'body' and 'tail' chains as kinematic
    ragdollNode->SetBodyChainState (bodyChain, CS_RAGDOLL_STATE_KINEMATIC);
    ragdollNode->SetBodyChainState (tailChain, CS_RAGDOLL_STATE_KINEMATIC);

    // Update the display of the dynamics debugger
    if (avatarTest->dynamicsDebugMode == DYNDEBUG_COLLIDER
	|| avatarTest->dynamicsDebugMode == DYNDEBUG_MIXED)
      avatarTest->dynamicsDebugger->UpdateDisplay ();
  }

  // Reset 'LookAt' controller
  alwaysRotate = false;
  lookAtNode->SetAlwaysRotate (alwaysRotate);
  targetMode = LOOKAT_CAMERA;
  lookAtNode->SetTarget (avatarTest->view->GetCamera(), csVector3 (0.0f));
  rotationSpeed = ROTATION_NORMAL;
  lookAtNode->SetMaximumSpeed (5.0f);

  // Reset 'speed' controller
  currentSpeed = 0;
  speedNode->SetSpeed (((float) currentSpeed) / 10.0f);

  // Reset morphing
  smileWeight = 1.0f;
  animesh->SetMorphTargetWeight (animeshFactory->FindMorphTarget ("smile.B"), 1.0f);
  animesh->SetMorphTargetWeight (animeshFactory->FindMorphTarget ("eyebrows_down.B"), 1.0f);
  animesh->SetMorphTargetWeight (animeshFactory->FindMorphTarget ("wings_in"), 1.0f);
  animesh->SetMorphTargetWeight (animeshFactory->FindMorphTarget ("eyelids_closed"), 0.0f);
}

void FrankieScene::UpdateStateDescription ()
{
  avatarTest->stateDescriptions.DeleteAll ();

  if (targetMode == LOOKAT_CAMERA)
    avatarTest->stateDescriptions.Push ("Watch out, Frankie is looking at you!");
  else if (targetMode == LOOKAT_POSITION)
    avatarTest->stateDescriptions.Push ("Frankie is looking at something");
  else if (targetMode == LOOKAT_NOTHING)
    avatarTest->stateDescriptions.Push ("Frankie doesn't care about anything");

  if (alwaysRotate)
    avatarTest->stateDescriptions.Push ("Always rotate: ON");
  else
    avatarTest->stateDescriptions.Push ("Always rotate: OFF");

  if (rotationSpeed == ROTATION_SLOW)
    avatarTest->stateDescriptions.Push ("Rotation speed: really slow");
  else if (rotationSpeed == ROTATION_NORMAL)
    avatarTest->stateDescriptions.Push ("Rotation speed: normal");
  else if (rotationSpeed == ROTATION_IMMEDIATE)
    avatarTest->stateDescriptions.Push ("Rotation speed: infinite");

  csString txt;
  txt.Format ("Walk speed: %.1f", ((float) currentSpeed) / 10.0f);
  avatarTest->stateDescriptions.Push (txt);
}

