/*
  Copyright (C) 2010 Christian Van Brussel, Institute of Information
      and Communication Technologies, Electronics and Applied Mathematics
      at Universite catholique de Louvain, Belgium
      http://www.uclouvain.be/en-icteam.html

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
#include "ivaria/decal.h"

#define LOOKAT_CAMERA 1
#define LOOKAT_POSITION 2
#define LOOKAT_NOTHING 3

#define ROTATION_SLOW 1
#define ROTATION_NORMAL 2
#define ROTATION_IMMEDIATE 3

FrankieScene::FrankieScene (AvatarTest* avatarTest)
  : avatarTest (avatarTest), debugBones (false), debugBBoxes (false), targetReached (false),
    lookAtListener (this), decalsEnabled (false), decal (nullptr), decalPosition (0.0f)
{
  // Setup the parameters of the camera manager
  avatarTest->cameraManager->SetStartPosition (csVector3 (0.0f, 0.25f, -1.25f));
  avatarTest->cameraManager->SetCameraMinimumDistance (0.75f);

  // Define the available keys
  avatarTest->hudManager->GetKeyDescriptions ()->Empty ();
  avatarTest->hudManager->GetKeyDescriptions ()->Push ("arrow keys: move camera");
  avatarTest->hudManager->GetKeyDescriptions ()->Push ("SHIFT-up/down keys: camera closer/farther");
  avatarTest->hudManager->GetKeyDescriptions ()->Push ("+/-: walk faster/slower");
  avatarTest->hudManager->GetKeyDescriptions ()->Push (csString().Format ("t: toggle %s target mode",
					      CS::Quote::Single ("LookAt")));
  avatarTest->hudManager->GetKeyDescriptions ()->Push (csString().Format ("y: toggle %s mode",
					      CS::Quote::Single ("LookAt: always rotate")));
  avatarTest->hudManager->GetKeyDescriptions ()->Push (csString().Format ("u: toggle %s",
					      CS::Quote::Single ("LookAt: rotation speed")));
  if (avatarTest->physicsEnabled)
  {
    avatarTest->hudManager->GetKeyDescriptions ()->Push ("f: toggle physical tail");
    avatarTest->hudManager->GetKeyDescriptions ()->Push ("left mouse: kill Frankie");
    avatarTest->hudManager->GetKeyDescriptions ()->Push ("d: display active colliders");
  }
  avatarTest->hudManager->GetKeyDescriptions ()->Push ("a: display bone positions");
  avatarTest->hudManager->GetKeyDescriptions ()->Push ("b: display bounding boxes");
  if (avatarTest->decalManager)
    avatarTest->hudManager->GetKeyDescriptions ()->Push ("c: toggle decals under the mouse");
  avatarTest->hudManager->GetKeyDescriptions ()->Push ("r: reset scene");
  avatarTest->hudManager->GetKeyDescriptions ()->Push ("n: switch to next scene");
}

FrankieScene::~FrankieScene ()
{
  if (!animesh)
    return;

  // Remove the 'lookat' listener
  lookAtNode->RemoveListener (&lookAtListener);

  // Remove any active decal
  if (decal)
    avatarTest->decalManager->DeleteDecal (decal);

  // Remove the mesh from the scene
  csRef<iMeshObject> animeshObject = scfQueryInterface<iMeshObject> (animesh);
  avatarTest->engine->RemoveObject (animeshObject->GetMeshWrapper ());
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

  // Update the morph state (frankie smiles sadistically if there is no target in view)
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

  // Update the decal
  if (decalsEnabled)
  {
    // Check if the mouse is over the animesh
    csVector3 isect;
    csVector3 direction;
    int triangle;
    bool hit = avatarTest->HitBeamAnimatedMesh (isect, direction, triangle);

    if (hit)
    {
      // Check if the position of the decal has changed
      if (!decal || (decalPosition - isect).Norm () > 0.01f)
      {
	// Remove the previous decal
	if (decal)
	  avatarTest->decalManager->DeleteDecal (decal);

	// Create a new decal
	csVector3 up =
	  avatarTest->view->GetCamera()->GetTransform ().This2OtherRelative (csVector3 (0,1,0));
	csRef<iMeshObject> meshObject = scfQueryInterface<iMeshObject> (animesh);
	decal = avatarTest->decalManager->CreateDecal
	  (decalTemplate, meshObject->GetMeshWrapper (),
	   meshObject->GetMeshWrapper ()->GetMovable ()->GetTransform ().This2Other (isect),
	   up, -direction, 0.1f, 0.1f, decal);
	decalPosition = isect;
      }
    }
  }
}

void FrankieScene::PostFrame ()
{
  debugNode->Draw (avatarTest->view->GetCamera ());
}


bool FrankieScene::OnKeyboard (iEvent &ev)
{
  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    // Toggle the target mode of the 'LookAt' animation node
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

    // Toggle the 'always rotate' option of the 'LookAt' animation node
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'y')
    {
      alwaysRotate = !alwaysRotate;
      lookAtNodeFactory->SetAlwaysRotate (alwaysRotate);
      return true;
    }

    // Toggle the rotation speed of the 'LookAt' animation node
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'u')
    {
      if (rotationSpeed == ROTATION_SLOW)
      {
	rotationSpeed = ROTATION_NORMAL;
	lookAtNodeFactory->SetMaximumSpeed (5.0f);
      }

      else if (rotationSpeed == ROTATION_NORMAL)
      {
	rotationSpeed = ROTATION_IMMEDIATE;
	lookAtNodeFactory->SetMaximumSpeed (0.0f);
      }

      else if (rotationSpeed == ROTATION_IMMEDIATE)
      {
	rotationSpeed = ROTATION_SLOW;
	lookAtNodeFactory->SetMaximumSpeed (0.5f);
      }

      return true;
    }

    // Update the walking speed of the 'speed' animation node
    else if (csKeyEventHelper::GetCookedCode (&ev) == '+')
    {
      if (currentSpeed < 50)
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
      if (ragdollNode->GetBodyChainState (tailChain) == CS::Animation::STATE_DYNAMIC)
	ragdollNode->SetBodyChainState (tailChain, CS::Animation::STATE_KINEMATIC);

      // If the tail is animated by the physical simulation then put the tail chain
      // in dynamic state
      else
	ragdollNode->SetBodyChainState (tailChain, CS::Animation::STATE_DYNAMIC);

      // Update the display of the dynamics debugger
      if (avatarTest->dynamicsDebugMode == DYNDEBUG_COLLIDER
	  || avatarTest->dynamicsDebugMode == DYNDEBUG_MIXED)
	avatarTest->dynamicsDebugger->UpdateDisplay ();

      return true;
    }

    // Toggle decals
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'c'
	     && avatarTest->decalManager)
    {
      decalsEnabled = !decalsEnabled;

      if (!decalsEnabled && decal)
      {
	avatarTest->decalManager->DeleteDecal (decal);
	decal = nullptr;
      }

      return true;
    }

    // Switching debug modes
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'a')
    {
      debugBones = !debugBones;
      int debugFlags = debugBones ? 
	(debugNodeFactory->GetDebugModes () | 
	 (CS::Animation::DEBUG_2DLINES | CS::Animation::DEBUG_SQUARES))
	: (debugNodeFactory->GetDebugModes () &
	   ~(CS::Animation::DEBUG_2DLINES | CS::Animation::DEBUG_SQUARES));
      debugNodeFactory->SetDebugModes ((CS::Animation::SkeletonDebugMode) debugFlags);

      return true;
    }

    else if (csKeyEventHelper::GetCookedCode (&ev) == 'b')
    {
      debugBBoxes = !debugBBoxes;
      int debugFlags = debugBBoxes ? 
        (debugNodeFactory->GetDebugModes () | CS::Animation::DEBUG_BBOXES)
	: (debugNodeFactory->GetDebugModes () & ~(CS::Animation::DEBUG_BBOXES));
      debugNodeFactory->SetDebugModes ((CS::Animation::SkeletonDebugMode) debugFlags);

      return true;
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
      CS::Physics::Bullet::HitBeamResult hitResult =
	avatarTest->bulletDynamicSystem->HitBeam (startBeam, endBeam);
      if (hitResult.hasHit
	  && hitResult.body->GetType () == CS::Physics::Bullet::RIGID_BODY)
      {
	// Apply a big force at the point clicked by the mouse
	csVector3 force = endBeam - startBeam;
	force.Normalize ();
	hitResult.body->QueryRigidBody ()->AddForceAtPos (hitResult.isect, force * 10.0f);
      }

      return true;
    }

    // At first, test with a sector HitBeam if we clicked on an animated mesh
    csSectorHitBeamResult sectorResult = camera->GetSector ()->HitBeam
      (startBeam, endBeam, true);
    if (!sectorResult.mesh)
      return false;

    csRef<CS::Mesh::iAnimatedMesh> animesh =
      scfQueryInterface<CS::Mesh::iAnimatedMesh> (sectorResult.mesh->GetMeshObject ());
    if (!animesh)
      return false;

    // OK, it's an animesh, it must be Frankie, let's kill him
    frankieDead = true;

    // Close the eyes of Frankie as he is dead
    animesh->SetMorphTargetWeight
      (animeshFactory->FindMorphTarget ("eyelids_closed"), 0.7f);

    // Stop the child animations, there is only the ragdoll animation node which is active
    lookAtNode->Stop ();

    // Set the ragdoll state of the CS::Animation::iBodyChain of the body and the tail as dynamic
    ragdollNode->SetBodyChainState (bodyChain, CS::Animation::STATE_DYNAMIC);
    ragdollNode->SetBodyChainState (tailChain, CS::Animation::STATE_DYNAMIC);

    // Update the display of the dynamics debugger
    if (avatarTest->dynamicsDebugMode == DYNDEBUG_COLLIDER
	|| avatarTest->dynamicsDebugMode == DYNDEBUG_MIXED)
      avatarTest->dynamicsDebugger->UpdateDisplay ();

    // Fling the body a bit
    const csOrthoTransform& tc = avatarTest->view->GetCamera ()->GetTransform ();
    uint boneCount = ragdollNode->GetBoneCount (CS::Animation::STATE_DYNAMIC);
    for (uint i = 0; i < boneCount; i++)
    {
      CS::Animation::BoneID boneID = ragdollNode->GetBone (CS::Animation::STATE_DYNAMIC, i);
      iRigidBody* rb = ragdollNode->GetBoneRigidBody (boneID);
      rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0.0f, 0.0f, 0.1f));
    }

    // Trace a physical beam to find which rigid body was hit
    CS::Physics::Bullet::HitBeamResult hitResult =
      avatarTest->bulletDynamicSystem->HitBeam (startBeam, endBeam);
    if (hitResult.hasHit
	&& hitResult.body->GetType () == CS::Physics::Bullet::RIGID_BODY)
    {
      // Apply a big force at the point clicked by the mouse
      csVector3 force = endBeam - startBeam;
      force.Normalize ();
      hitResult.body->QueryRigidBody ()->AddForceAtPos (hitResult.isect, force * 1.0f);
      hitResult.body->QueryRigidBody ()->SetLinearVelocity
	(tc.GetT2O () * csVector3 (0.0f, 0.0f, 1.0f));
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

  animeshFactory = scfQueryInterface<CS::Mesh::iAnimatedMeshFactory>
    (meshfact->GetMeshObjectFactory ());
  if (!animeshFactory)
    return avatarTest->ReportError ("Can't find Frankie's animesh factory!");

  // Load bodymesh (animesh's physical properties)
  rc = avatarTest->loader->Load ("/lib/frankie/skelfrankie_body");
  if (!rc.success)
    return avatarTest->ReportError ("Can't load Frankie's body mesh file!");

  csRef<CS::Animation::iBodyManager> bodyManager =
    csQueryRegistry<CS::Animation::iBodyManager> (avatarTest->GetObjectRegistry ());
  csRef<CS::Animation::iBodySkeleton> bodySkeleton = bodyManager->FindBodySkeleton ("frankie_body");
  if (!bodySkeleton)
    return avatarTest->ReportError ("Can't find Frankie's body mesh description!");

  // Create a new animation tree. The structure of the tree is:
  //   + Debug animation node (root node)
  //     + Ragdoll animation node (only if physics are enabled)
  //       + 'LookAt' animation node
  //         + 'speed' animation node
  //           + animation nodes for all speeds
  csRef<CS::Animation::iSkeletonAnimPacketFactory> animPacketFactory =
    animeshFactory->GetSkeletonFactory ()->GetAnimationPacket ();

  // Create the 'debug' node
  debugNodeFactory = avatarTest->debugManager->CreateAnimNodeFactory ("debug");
  debugNodeFactory->SetDebugModes (CS::Animation::DEBUG_NONE);
  debugNodeFactory->SetRandomColor (true);
  animPacketFactory->SetAnimationRoot (debugNodeFactory);

  // Create the 'LookAt' animation node
  lookAtNodeFactory = avatarTest->lookAtManager->CreateAnimNodeFactory ("lookat");
  lookAtNodeFactory->SetBodySkeleton (bodySkeleton);
  lookAtNodeFactory->SetBone (animeshFactory->GetSkeletonFactory ()->FindBone ("CTRL_Head"));
  lookAtNodeFactory->SetListenerDelay (0.6f);

  // Create the 'idle' animation node
  csRef<CS::Animation::iSkeletonAnimationNodeFactory> idleNodeFactory =
    animPacketFactory->CreateAnimationNode ("idle");
  idleNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_Idle1"));

  // Create the 'walk_slow' animation node
  csRef<CS::Animation::iSkeletonAnimationNodeFactory> walkSlowNodeFactory =
    animPacketFactory->CreateAnimationNode ("walk_slow");
  walkSlowNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_WalkSlow"));

  // Create the 'walk' animation node
  csRef<CS::Animation::iSkeletonAnimationNodeFactory> walkNodeFactory =
    animPacketFactory->CreateAnimationNode ("walk");
  walkNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_Walk"));

  // Create the 'walk_fast' animation node
  csRef<CS::Animation::iSkeletonAnimationNodeFactory> walkFastNodeFactory =
    animPacketFactory->CreateAnimationNode ("walk_fast");
  walkFastNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_WalkFast"));

  // Create the 'footing' animation node
  csRef<CS::Animation::iSkeletonAnimationNodeFactory> footingNodeFactory =
    animPacketFactory->CreateAnimationNode ("footing");
  footingNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_Runs"));

  // Create the 'run_slow' animation node
  csRef<CS::Animation::iSkeletonAnimationNodeFactory> runSlowNodeFactory =
    animPacketFactory->CreateAnimationNode ("run_slow");
  runSlowNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_RunSlow"));

  // Create the 'run' animation node
  csRef<CS::Animation::iSkeletonAnimationNodeFactory> runNodeFactory =
    animPacketFactory->CreateAnimationNode ("run");
  runNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_Run"));

  // Create the 'run_fast' animation node
  csRef<CS::Animation::iSkeletonAnimationNodeFactory> runFastNodeFactory =
    animPacketFactory->CreateAnimationNode ("run_fast");
  runFastNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_RunFaster"));

  // Create the 'speed' animation node (and add all animations of Frankie moving at different speeds)
  // Unfortunately, the Frankie animations from 'walk fast' to 'footing'
  // do not blend well together, but this is just an example...
  csRef<CS::Animation::iSkeletonSpeedNodeFactory> speedNodeFactory =
    avatarTest->speedManager->CreateAnimNodeFactory ("speed");
  speedNodeFactory->AddNode (idleNodeFactory, 0.0f);
  speedNodeFactory->AddNode (walkSlowNodeFactory, 0.4f);
  speedNodeFactory->AddNode (walkNodeFactory, 0.6f);
  speedNodeFactory->AddNode (walkFastNodeFactory, 1.2f);
  speedNodeFactory->AddNode (footingNodeFactory, 1.6f);
  speedNodeFactory->AddNode (runSlowNodeFactory, 2.6f);
  speedNodeFactory->AddNode (runNodeFactory, 3.4f);
  speedNodeFactory->AddNode (runFastNodeFactory, 5.0f);

  lookAtNodeFactory->SetChildNode (speedNodeFactory);

  if (avatarTest->physicsEnabled)
  {
    // Create the ragdoll animation node
    csRef<CS::Animation::iSkeletonRagdollNodeFactory> ragdollNodeFactory =
      avatarTest->ragdollManager->CreateAnimNodeFactory ("ragdoll");
    debugNodeFactory->SetChildNode (ragdollNodeFactory);
    ragdollNodeFactory->SetBodySkeleton (bodySkeleton);
    ragdollNodeFactory->SetChildNode (lookAtNodeFactory);

    // Create a bone chain for the whole body and add it to the ragdoll animation node.
    // The chain will be in kinematic mode when Frankie is alive, and in dynamic state
    // when Frankie has been killed.
    bodyChain = bodySkeleton->CreateBodyChain
      ("body_chain", animeshFactory->GetSkeletonFactory ()->FindBone ("Frankie_Main"));
    bodyChain->AddSubChain (animeshFactory->GetSkeletonFactory ()->FindBone ("CTRL_Pelvis"));
    bodyChain->AddSubChain (animeshFactory->GetSkeletonFactory ()->FindBone ("CTRL_Head"));
    ragdollNodeFactory->AddBodyChain (bodyChain, CS::Animation::STATE_KINEMATIC);

    // Create a bone chain for the tail of Frankie and add it to the ragdoll animation node.
    // The chain will be in kinematic mode most of the time, and in dynamic mode when the
    // user ask for it with the 'f' key or when Frankie has been killed.
    tailChain = bodySkeleton->CreateBodyChain
      ("tail_chain", animeshFactory->GetSkeletonFactory ()->FindBone ("Tail_1"));
    tailChain->AddSubChain (animeshFactory->GetSkeletonFactory ()->FindBone ("Tail_8"));
    ragdollNodeFactory->AddBodyChain (tailChain, CS::Animation::STATE_KINEMATIC);
  }

  else
    debugNodeFactory->SetChildNode (lookAtNodeFactory);

  // Create the animated mesh
  csRef<iMeshWrapper> avatarMesh =
    avatarTest->engine->CreateMeshWrapper (meshfact, "Frankie",
					   avatarTest->room, csVector3 (0.0f));
  animesh = scfQueryInterface<CS::Mesh::iAnimatedMesh> (avatarMesh->GetMeshObject ());

  // When the animated mesh is created, the animation nodes are created too.
  // We can therefore set them up now.
  CS::Animation::iSkeletonAnimNode* rootNode =
    animesh->GetSkeleton ()->GetAnimationPacket ()->GetAnimationRoot ();

  // Find a reference to the debug node
  debugNode = scfQueryInterface<CS::Animation::iSkeletonDebugNode> (rootNode->FindNode ("debug"));

  // Setup of the LookAt animation node
  lookAtNode = scfQueryInterface<CS::Animation::iSkeletonLookAtNode> (rootNode->FindNode ("lookat"));
  lookAtNode->AddListener (&lookAtListener);

  // Setup of the speed animation node
  speedNode = scfQueryInterface<CS::Animation::iSkeletonSpeedNode> (rootNode->FindNode ("speed"));

  // Setup of the ragdoll animation node
  if (avatarTest->physicsEnabled)
  {
    ragdollNode =
      scfQueryInterface<CS::Animation::iSkeletonRagdollNode> (rootNode->FindNode ("ragdoll"));
    ragdollNode->SetDynamicSystem (avatarTest->dynamicSystem);
  }

  // Reset the scene so as to put the parameters of the animation nodes in a default state
  ResetScene ();

  // Setup the decal
  if (avatarTest->decalManager)
  {
    // Load the decal material
    iMaterialWrapper * material = 
      avatarTest->engine->GetMaterialList()->FindByName("decal");
    if (!material)
    {
      if (!avatarTest->loader->LoadTexture ("decal", "/lib/std/cslogo2.png"))
	avatarTest->ReportWarning ("Could not load the decal texture!");

      material = avatarTest->engine->GetMaterialList()->FindByName("decal");
    }
    
    if (!material)
      avatarTest->ReportWarning ("Error finding decal material");

    // Setup the decal template
    else
    {
      decalTemplate = avatarTest->decalManager->CreateDecalTemplate (material);
      decalTemplate->SetDecalOffset (0.001f);
      decalTemplate->SetClipping (false);
    }
  }

  return true;
}

void FrankieScene::LookAtListener::TargetReached ()
{
  csPrintf ("%s target reached\n", CS::Quote::Single ("LookAt"));
  frankieScene->targetReached = true;
}

void FrankieScene::LookAtListener::TargetLost ()
{
  csPrintf ("%s target lost\n", CS::Quote::Single ("LookAt"));
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
    ragdollNode->SetBodyChainState (bodyChain, CS::Animation::STATE_KINEMATIC);
    ragdollNode->SetBodyChainState (tailChain, CS::Animation::STATE_KINEMATIC);

    // Update the display of the dynamics debugger
    if (avatarTest->dynamicsDebugMode == DYNDEBUG_COLLIDER
	|| avatarTest->dynamicsDebugMode == DYNDEBUG_MIXED)
      avatarTest->dynamicsDebugger->UpdateDisplay ();
  }

  // Reset the 'LookAt' animation node
  alwaysRotate = false;
  lookAtNodeFactory->SetAlwaysRotate (alwaysRotate);
  targetMode = LOOKAT_CAMERA;
  lookAtNode->SetTarget (avatarTest->view->GetCamera(), csVector3 (0.0f));
  rotationSpeed = ROTATION_NORMAL;
  lookAtNodeFactory->SetMaximumSpeed (5.0f);
  lookAtNode->Play ();

  // Reset the 'speed' animation node
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
  avatarTest->hudManager->GetStateDescriptions ()->Empty ();

  if (targetMode == LOOKAT_CAMERA)
    avatarTest->hudManager->GetStateDescriptions ()->Push ("Watch out, Frankie is looking at you!");
  else if (targetMode == LOOKAT_POSITION)
    avatarTest->hudManager->GetStateDescriptions ()->Push ("Frankie is looking at something");
  else if (targetMode == LOOKAT_NOTHING)
    avatarTest->hudManager->GetStateDescriptions ()->Push ("Frankie doesn't care about anything");

  if (alwaysRotate)
    avatarTest->hudManager->GetStateDescriptions ()->Push ("Always rotate: ON");
  else
    avatarTest->hudManager->GetStateDescriptions ()->Push ("Always rotate: OFF");

  if (rotationSpeed == ROTATION_SLOW)
    avatarTest->hudManager->GetStateDescriptions ()->Push ("Rotation speed: really slow");
  else if (rotationSpeed == ROTATION_NORMAL)
    avatarTest->hudManager->GetStateDescriptions ()->Push ("Rotation speed: normal");
  else if (rotationSpeed == ROTATION_IMMEDIATE)
    avatarTest->hudManager->GetStateDescriptions ()->Push ("Rotation speed: infinite");

  csString txt;
  txt.Format ("Walk speed: %.1f", ((float) currentSpeed) / 10.0f);
  avatarTest->hudManager->GetStateDescriptions ()->Push (txt);

  if (decalsEnabled)
    avatarTest->hudManager->GetStateDescriptions ()->Push ("Decal textures: ON");
  else
    avatarTest->hudManager->GetStateDescriptions ()->Push ("Decal textures: OFF");
}

