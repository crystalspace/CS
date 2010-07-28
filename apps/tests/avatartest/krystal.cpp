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
#include "krystal.h"

#define CAMERA_MINIMUM_DISTANCE 0.75f
#define CAMERA_HIPS_DISTANCE 3.0f

KrystalScene::KrystalScene (AvatarTest* avatarTest)
  : avatarTest (avatarTest)
{
  // Define the available keys
  avatarTest->hudHelper.keyDescriptions.DeleteAll ();
  avatarTest->hudHelper.keyDescriptions.Push ("arrow keys: move camera");
  avatarTest->hudHelper.keyDescriptions.Push ("SHIFT-up/down keys: camera closer/farther");
  if (avatarTest->physicsEnabled)
  {
    avatarTest->hudHelper.keyDescriptions.Push ("d: display active colliders");
    avatarTest->hudHelper.keyDescriptions.Push ("left mouse: kill Krystal");
  }
  avatarTest->hudHelper.keyDescriptions.Push ("r: reset scene");
  avatarTest->hudHelper.keyDescriptions.Push ("n: switch to next scene");
}

KrystalScene::~KrystalScene ()
{
  // Remove the meshes and soft bodies from the scene
  if (animesh)
  {
    csRef<iMeshObject> animeshObject = scfQueryInterface<iMeshObject> (animesh);
    avatarTest->engine->RemoveObject (animeshObject->GetMeshWrapper ());
  }

  if (hairsMesh)
    avatarTest->engine->RemoveObject (hairsMesh);

  if (hairsBody)
    avatarTest->bulletDynamicSystem->RemoveSoftBody (hairsBody);

  if (skirtMesh)
    avatarTest->engine->RemoveObject (skirtMesh);

  if (skirtBody)
    avatarTest->bulletDynamicSystem->RemoveSoftBody (skirtBody);
}

csVector3 KrystalScene::GetCameraStart ()
{
  return csVector3 (0.0f, 1.0f, -2.5f);
}

float KrystalScene::GetCameraMinimumDistance ()
{
  return CAMERA_MINIMUM_DISTANCE;
}

csVector3 KrystalScene::GetCameraTarget ()
{
  // The target of the camera is the hips of Krystal when we are far away,
  // and the head when we are close
  csQuaternion boneRotation;
  csVector3 boneOffset;
  csRef<iMeshObject> animeshObject = scfQueryInterface<iMeshObject> (animesh);

  // Compute the position of the hips
  animesh->GetSkeleton ()->GetTransformAbsSpace
    (animeshFactory->GetSkeletonFactory ()->FindBone ("Hips"), boneRotation, boneOffset);
  csOrthoTransform hipsTransform (csMatrix3 (boneRotation.GetConjugate ()),
				  boneOffset);
  csVector3 hipsPosition = (hipsTransform
			    * animeshObject->GetMeshWrapper ()->QuerySceneNode ()
			    ->GetMovable ()->GetTransform ()).GetOrigin ();

  // Compute the position of the head
  animesh->GetSkeleton ()->GetTransformAbsSpace
    (animeshFactory->GetSkeletonFactory ()->FindBone ("Head"), boneRotation, boneOffset);
  csOrthoTransform headTransform (csMatrix3 (boneRotation.GetConjugate ()),
				  boneOffset);
  csVector3 headPosition = (headTransform
			    * animeshObject->GetMeshWrapper ()->QuerySceneNode ()
			    ->GetMovable ()->GetTransform ()).GetOrigin ();

  // Compute the distance between the camera and the head
  float distance = (avatarTest->view->GetCamera ()->GetTransform ().GetOrigin ()
		    - headPosition).Norm ();

  // Compute the camera target
  csVector3 cameraTarget;
  if (distance >= CAMERA_HIPS_DISTANCE)
    cameraTarget = hipsPosition;

  else if (distance <= CAMERA_MINIMUM_DISTANCE)
    cameraTarget = headPosition;

  else
    cameraTarget = hipsPosition + (headPosition - hipsPosition)
      * (CAMERA_HIPS_DISTANCE - distance)
      / (CAMERA_HIPS_DISTANCE - CAMERA_MINIMUM_DISTANCE);

  return cameraTarget;
}

float KrystalScene::GetSimulationSpeed ()
{
  if (krystalDead)
    return 0.3f;

  return 1.0f;
}

bool KrystalScene::HasPhysicalObjects ()
{
  return true;
}

void KrystalScene::Frame ()
{
}

bool KrystalScene::OnKeyboard (iEvent &ev)
{
  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    // Reset of the scene
    if (csKeyEventHelper::GetCookedCode (&ev) == 'r')
    {
      ResetScene ();
      return true;
    }
  }

  return false;
}

bool KrystalScene::OnMouseDown (iEvent &ev)
{
  if (csMouseEventHelper::GetButton (&ev) == 0
      && avatarTest->physicsEnabled)
  {
    // Trying to kill Krystal

    // We will trace a beam to the point clicked by the mouse to check if
    // something is hit. Let's start by computing the beam end points.
    int mouseX = csMouseEventHelper::GetX (&ev);
    int mouseY = csMouseEventHelper::GetY (&ev);

    csRef<iCamera> camera = avatarTest->view->GetCamera ();
    csVector2 v2d (mouseX, avatarTest->g2d->GetHeight () - mouseY);
    csVector3 v3d = camera->InvPerspective (v2d, 10000);
    csVector3 startBeam = camera->GetTransform ().GetOrigin ();
    csVector3 endBeam = camera->GetTransform ().This2Other (v3d);

    // If Krystal is already dead, simply check for adding a force on him
    if (krystalDead)
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
	hitResult.body->QueryRigidBody ()->AddForceAtPos (hitResult.isect, force * 5.0f);
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

    // OK, it's an animesh, it must be Krystal, let's kill her
    krystalDead = true;

    // The ragdoll model of Krystal is rather complex. We therefore use high
    // accuracy/low performance parameters for a better behavior of the dynamic
    // simulation.
    avatarTest->bulletDynamicSystem->SetStepParameters (0.008f, 150, 10);

    // Set the ragdoll state of the iBodyChain of the whole body as dynamic
    // (hairs are already in the good state)
    ragdollNode->SetBodyChainState (bodyChain, CS::Animation::STATE_DYNAMIC);

    // Update the display of the dynamics debugger
    if (avatarTest->dynamicsDebugMode == DYNDEBUG_COLLIDER
	|| avatarTest->dynamicsDebugMode == DYNDEBUG_MIXED)
      avatarTest->dynamicsDebugger->UpdateDisplay ();

    // Fling the body a bit
    const csOrthoTransform& tc = avatarTest->view->GetCamera ()->GetTransform ();
    uint boneCount = ragdollNode->GetBoneCount (CS::Animation::STATE_DYNAMIC);
    for (uint i = 0; i < boneCount; i++)
    {
      BoneID boneID = ragdollNode->GetBone (CS::Animation::STATE_DYNAMIC, i);
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
      hitResult.body->QueryRigidBody ()->AddForceAtPos (hitResult.isect, force * 5.0f);
      hitResult.body->QueryRigidBody ()->SetLinearVelocity (tc.GetT2O ()
					      * csVector3 (0.0f, 0.0f, 5.0f));
    }

    return true;
  }

  return false;
}

bool KrystalScene::CreateAvatar ()
{
  printf ("Loading Krystal...\n");

  // Load animesh factory
  csLoadResult rc = avatarTest->loader->Load ("/lib/krystal/krystal.xml");
  if (!rc.success)
    return avatarTest->ReportError ("Can't load Krystal library file!");

  csRef<iMeshFactoryWrapper> meshfact =
    avatarTest->engine->FindMeshFactory ("krystal");
  if (!meshfact)
    return avatarTest->ReportError ("Can't find Krystal's mesh factory!");

  animeshFactory = scfQueryInterface<CS::Mesh::iAnimatedMeshFactory>
    (meshfact->GetMeshObjectFactory ());
  if (!animeshFactory)
    return avatarTest->ReportError ("Can't find Krystal's animesh factory!");

  // Load bodymesh (animesh's physical properties)
  rc = avatarTest->loader->Load ("/lib/krystal/skelkrystal_body");
  if (!rc.success)
    return avatarTest->ReportError ("Can't load Krystal's body mesh file!");

  csRef<iBodyManager> bodyManager =
    csQueryRegistry<iBodyManager> (avatarTest->GetObjectRegistry ());
  csRef<iBodySkeleton> bodySkeleton = bodyManager->FindBodySkeleton ("krystal_body");
  if (!bodySkeleton)
    return avatarTest->ReportError ("Can't find Krystal's body mesh description!");

  // Load Krystal's hairs
  rc = avatarTest->loader->Load ("/lib/krystal/krystal_hairs.xml");
  if (!rc.success)
    return avatarTest->ReportError ("Can't load Krystal's hairs library file!");

  csRef<iMeshFactoryWrapper> hairsMeshFact =
    avatarTest->engine->FindMeshFactory ("krystal_hairs");
  if (!hairsMeshFact)
    return avatarTest->ReportError ("Can't find Krystal's hairs mesh factory!");

  // Load Krystal's skirt
  rc = avatarTest->loader->Load ("/lib/krystal/krystal_skirt.xml");
  if (!rc.success)
    return avatarTest->ReportError ("Can't load Krystal's skirt library file!");

  csRef<iMeshFactoryWrapper> skirtMeshFact =
    avatarTest->engine->FindMeshFactory ("krystal_skirt");
  if (!skirtMeshFact)
    return avatarTest->ReportError ("Can't find Krystal's skirt mesh factory!");

  // Create a new animation tree. The structure of the tree is:
  //   + ragdoll controller node (root node - only if physics are enabled)
  //     + Random node
  //       + idle animation nodes
  csRef<iSkeletonAnimPacketFactory2> animPacketFactory =
    animeshFactory->GetSkeletonFactory ()->GetAnimationPacket ();

  // Create the 'random' node
  csRef<iSkeletonRandomNodeFactory2> randomNodeFactory =
    animPacketFactory->CreateRandomNode ("random");
  randomNodeFactory->SetAutomaticSwitch (true);

  // Create the 'idle01' animation node
  csRef<iSkeletonAnimationNodeFactory2> idle01NodeFactory =
    animPacketFactory->CreateAnimationNode ("idle01");
  idle01NodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("idle01"));

  // Create the 'idle02' animation node
  csRef<iSkeletonAnimationNodeFactory2> idle02NodeFactory =
    animPacketFactory->CreateAnimationNode ("idle02");
  idle02NodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("idle02"));

  // Create the 'idle03' animation node
  csRef<iSkeletonAnimationNodeFactory2> idle03NodeFactory =
    animPacketFactory->CreateAnimationNode ("idle03");
  idle03NodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("idle03"));

  // Create the 'idle04' animation node
  csRef<iSkeletonAnimationNodeFactory2> idle04NodeFactory =
    animPacketFactory->CreateAnimationNode ("idle04");
  idle04NodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("idle04"));

  // Create the 'idle05' animation node
  csRef<iSkeletonAnimationNodeFactory2> idle05NodeFactory =
    animPacketFactory->CreateAnimationNode ("idle05");
  idle05NodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("idle05"));

  // Create the 'idle06' animation node
  csRef<iSkeletonAnimationNodeFactory2> idle06NodeFactory =
    animPacketFactory->CreateAnimationNode ("idle06");
  idle06NodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("idle06"));

  // Create the 'stand' animation node
  csRef<iSkeletonAnimationNodeFactory2> standNodeFactory =
    animPacketFactory->CreateAnimationNode ("stand");
  standNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("stand"));

  idle01NodeFactory->SetAutomaticReset (true);
  idle02NodeFactory->SetAutomaticReset (true);
  idle03NodeFactory->SetAutomaticReset (true);
  idle04NodeFactory->SetAutomaticReset (true);
  idle05NodeFactory->SetAutomaticReset (true);
  idle06NodeFactory->SetAutomaticReset (true);
  standNodeFactory->SetAutomaticReset (true);

  idle01NodeFactory->SetAutomaticStop (false);
  idle02NodeFactory->SetAutomaticStop (false);
  idle03NodeFactory->SetAutomaticStop (false);
  idle04NodeFactory->SetAutomaticStop (false);
  idle05NodeFactory->SetAutomaticStop (false);
  idle06NodeFactory->SetAutomaticStop (false);
  standNodeFactory->SetAutomaticStop (false);

  randomNodeFactory->AddNode (idle01NodeFactory, 1.0f);
  randomNodeFactory->AddNode (idle02NodeFactory, 1.0f);
  randomNodeFactory->AddNode (idle03NodeFactory, 1.0f);
  randomNodeFactory->AddNode (idle04NodeFactory, 1.0f);
  randomNodeFactory->AddNode (idle05NodeFactory, 1.0f);
  randomNodeFactory->AddNode (idle06NodeFactory, 1.0f);
  randomNodeFactory->AddNode (standNodeFactory, 10.0f);

  if (avatarTest->physicsEnabled)
  {
    // Create the ragdoll controller
    csRef<iSkeletonRagdollNodeFactory2> ragdollNodeFactory =
      avatarTest->ragdollManager->CreateAnimNodeFactory
      ("ragdoll", bodySkeleton, avatarTest->dynamicSystem);
    animPacketFactory->SetAnimationRoot (ragdollNodeFactory);
    ragdollNodeFactory->SetChildNode (randomNodeFactory);

    // Create a bone chain for the whole body and add it to the ragdoll controller.
    // The chain will be in kinematic mode when Krystal is alive, and in dynamic state
    // when Krystal has been killed.
    bodyChain = bodySkeleton->CreateBodyChain
      ("body_chain", animeshFactory->GetSkeletonFactory ()->FindBone ("Hips"),
       animeshFactory->GetSkeletonFactory ()->FindBone ("Head"),
       animeshFactory->GetSkeletonFactory ()->FindBone ("RightFoot"),
       animeshFactory->GetSkeletonFactory ()->FindBone ("RightHand"),
       animeshFactory->GetSkeletonFactory ()->FindBone ("LeftFoot"),
       animeshFactory->GetSkeletonFactory ()->FindBone ("LeftHand"), 0);
    ragdollNodeFactory->AddBodyChain (bodyChain, CS::Animation::STATE_KINEMATIC);

    if (avatarTest->softBodiesEnabled)
    {
      // Create the soft body for the hairs
      csRef<iGeneralFactoryState> hairsFactoryState =
	scfQueryInterface<iGeneralFactoryState> (hairsMeshFact->GetMeshObjectFactory ());
      hairsBody = avatarTest->bulletDynamicSystem->CreateSoftBody
	(hairsFactoryState, csOrthoTransform (csMatrix3 (), csVector3 (0.0f)));
      hairsBody->SetMass (2.0f);
      hairsBody->SetRigidity (0.02f);

      // Create the mesh of the hairs
      hairsFactoryState->SetAnimationControlFactory (avatarTest->softBodyAnimationFactory);
      hairsMesh = avatarTest->engine->CreateMeshWrapper
	(hairsMeshFact, "krystal_hairs", avatarTest->room, csVector3 (0.0f));

      // Init the animation control for the animation of the genmesh
      csRef<iGeneralMeshState> hairsMeshState =
	scfQueryInterface<iGeneralMeshState> (hairsMesh->GetMeshObject ());
      csRef<iSoftBodyAnimationControl> animationControl =
	scfQueryInterface<iSoftBodyAnimationControl> (hairsMeshState->GetAnimationControl ());
      animationControl->SetSoftBody (hairsBody);

      // Create the soft body for the skirt
      csRef<iGeneralFactoryState> skirtFactoryState =
	scfQueryInterface<iGeneralFactoryState> (skirtMeshFact->GetMeshObjectFactory ());
      skirtBody = avatarTest->bulletDynamicSystem->CreateSoftBody
	(skirtFactoryState, csOrthoTransform (csMatrix3 (), csVector3 (0.0f, 0.0f, -0.055f)));
      skirtBody->SetMass (2.0f);
      skirtBody->SetRigidity (0.02f);

      // Create the mesh of the skirt
      skirtFactoryState->SetAnimationControlFactory (avatarTest->softBodyAnimationFactory);
      skirtMesh = avatarTest->engine->CreateMeshWrapper
	(skirtMeshFact, "krystal_skirt", avatarTest->room, csVector3 (0.0f));

      // Init the animation control for the animation of the genmesh
      csRef<iGeneralMeshState> skirtMeshState =
	scfQueryInterface<iGeneralMeshState> (skirtMesh->GetMeshObject ());
      animationControl =
	scfQueryInterface<iSoftBodyAnimationControl> (skirtMeshState->GetAnimationControl ());
      animationControl->SetSoftBody (skirtBody);
    }
  }

  else
    animPacketFactory->SetAnimationRoot (randomNodeFactory);

  // Create the animated mesh
  csRef<iMeshWrapper> avatarMesh =
    avatarTest->engine->CreateMeshWrapper (meshfact, "krystal",
					   avatarTest->room, csVector3 (0.0f));
  animesh = scfQueryInterface<CS::Mesh::iAnimatedMesh> (avatarMesh->GetMeshObject ());

  // When the animated mesh is created, the animation nodes are created too.
  // We can therefore set them up now.
  iSkeletonAnimNode2* rootNode =
    animesh->GetSkeleton ()->GetAnimationPacket ()->GetAnimationRoot ();

  // Setup of the ragdoll controller
  if (avatarTest->physicsEnabled)
  {
    ragdollNode =
      scfQueryInterface<iSkeletonRagdollNode2> (rootNode->FindNode ("ragdoll"));
    ragdollNode->SetAnimatedMesh (animesh);

    // Start the ragdoll animation node in order to have the rigid bodies created
    ragdollNode->Play ();

    if (avatarTest->softBodiesEnabled)
    {
      // Find the rigid body of the head of Krystal
      iRigidBody* headBody = ragdollNode->GetBoneRigidBody
	(animeshFactory->GetSkeletonFactory ()->FindBone ("Head"));

      // Attach the hairs to the rigid body of the head of Krystal.
      // We are lazy and simply attach all the vertices higher than a threshold.
      for (size_t i = 0; i < hairsBody->GetVertexCount (); i++)
	if (hairsBody->GetVertexPosition (i)[1] > 1.7f)
	  hairsBody->AnchorVertex (i, headBody);

      // Find the rigid body of the spine of Krystal
      iRigidBody* spineBody = ragdollNode->GetBoneRigidBody
	(animeshFactory->GetSkeletonFactory ()->FindBone ("ToSpine"));

      // Attach the skirt to the rigid body of the spine of Krystal.
      // The indices of the vertices have been found by a manual investigation
      // of the vertex list
      skirtBody->AnchorVertex (7, spineBody);
      skirtBody->AnchorVertex (8, spineBody);
      skirtBody->AnchorVertex (10, spineBody);
      skirtBody->AnchorVertex (67, spineBody);
      skirtBody->AnchorVertex (68, spineBody);
      skirtBody->AnchorVertex (69, spineBody);
      skirtBody->AnchorVertex (70, spineBody);
      skirtBody->AnchorVertex (71, spineBody);
      skirtBody->AnchorVertex (76, spineBody);
      skirtBody->AnchorVertex (77, spineBody);
      skirtBody->AnchorVertex (78, spineBody);
      skirtBody->AnchorVertex (79, spineBody);
      skirtBody->AnchorVertex (80, spineBody);
    }
  }

  // Start animation
  rootNode->Play ();

  // Reset the scene so as to put the parameters of the animation nodes in a default state
  ResetScene ();

  return true;
}

void KrystalScene::ResetScene ()
{
  if (avatarTest->physicsEnabled)
  {
    // Reset the position of the animesh
    csRef<iMeshObject> animeshObject = scfQueryInterface<iMeshObject> (animesh);
    animeshObject->GetMeshWrapper ()->QuerySceneNode ()->GetMovable ()->SetTransform
      (csOrthoTransform (csMatrix3 (), csVector3 (0.0f)));
    animeshObject->GetMeshWrapper ()->QuerySceneNode ()->GetMovable ()->UpdateMove ();

    krystalDead = false;

    // Set the ragdoll state of the 'body' chain as kinematic
    ragdollNode->SetBodyChainState (bodyChain, CS::Animation::STATE_KINEMATIC);

    // Update the display of the dynamics debugger
    if (avatarTest->dynamicsDebugMode == DYNDEBUG_COLLIDER
	|| avatarTest->dynamicsDebugMode == DYNDEBUG_MIXED)
      avatarTest->dynamicsDebugger->UpdateDisplay ();

    // There are still big unlinerarities in the transition of Krystal's animations.
    // These gaps create a bad behavior of the simulation of the hairs, this is
    // better with lower step parameters.
    avatarTest->bulletDynamicSystem->SetStepParameters (0.016667f, 1, 10);
  }
}

void KrystalScene::UpdateStateDescription ()
{
  avatarTest->hudHelper.stateDescriptions.DeleteAll ();
}
