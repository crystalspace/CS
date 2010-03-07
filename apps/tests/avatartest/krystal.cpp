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
#include "krystal.h"

KrystalScene::KrystalScene (AvatarTest* avatarTest)
  : avatarTest (avatarTest)
{
}

KrystalScene::~KrystalScene ()
{
  // Remove the mesh from the scene
  csRef<iMeshObject> animeshObject = scfQueryInterface<iMeshObject> (animesh);
  avatarTest->engine->RemoveObject (animeshObject->GetMeshWrapper ());
}

csVector3 KrystalScene::GetCameraStart ()
{
  return csVector3 (0.0f, 0.0f, -2.5f);
}

csVector3 KrystalScene::GetCameraTarget ()
{
  // The target of the camera is the hips of Krystal
  csQuaternion boneRotation;
  csVector3 boneOffset;
  animesh->GetSkeleton ()->GetTransformBoneSpace
    (animeshFactory->GetSkeletonFactory ()->FindBone ("Hips"), boneRotation, boneOffset);
  csOrthoTransform boneTransform (csMatrix3 (boneRotation.GetConjugate ()),
				  boneOffset);
  csRef<iMeshObject> animeshObject = scfQueryInterface<iMeshObject> (animesh);
  csVector3 avatarPosition = (boneTransform
			      * animeshObject->GetMeshWrapper ()->QuerySceneNode ()
			      ->GetMovable ()->GetTransform ()).GetOrigin ();
  avatarPosition.y = 1.1f;
  return avatarPosition;
}

float KrystalScene::GetSimulationSpeed ()
{
  if (krystalDead)
    return 0.2f;

  return 1.0f;
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

    // Compute the beam points to check what was hit
    int mouseX = csMouseEventHelper::GetX (&ev);
    int mouseY = csMouseEventHelper::GetY (&ev);

    csRef<iCamera> camera = avatarTest->view->GetCamera ();
    csVector2 v2d (mouseX, camera->GetShiftY () * 2 - mouseY);
    csVector3 v3d = camera->InvPerspective (v2d, 10000);
    csVector3 startBeam = camera->GetTransform ().GetOrigin ();
    csVector3 endBeam = camera->GetTransform ().This2Other (v3d);

    // If Krystal is already dead, simply check for adding a force on him
    if (krystalDead)
    {
      // Trace a physical beam to find if a rigid body was hit
      csRef<iBulletDynamicSystem> bulletSystem =
	scfQueryInterface<iBulletDynamicSystem> (avatarTest->dynamicSystem);
      csBulletHitBeamResult physicsResult = bulletSystem->HitBeam (startBeam, endBeam);

      // Apply a big force at the point clicked by the mouse
      if (physicsResult.body)
      {
	csVector3 force = endBeam - startBeam;
	force.Normalize ();
	physicsResult.body->AddForceAtPos (physicsResult.isect, force * 5.0f);
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

    // OK, it's an animesh, it must be Krystal, start the ragdoll
    krystalDead = true;

    // Set the ragdoll state of the iBodyChain of the whole body as dynamic
    // (hairs are already in the good state)
    ragdollNode->SetBodyChainState (bodyChain, RAGDOLL_STATE_DYNAMIC);

    // Update the display of the dynamics debugger
    if (avatarTest->dynamicsDebugMode == DYNDEBUG_COLLIDER
	|| avatarTest->dynamicsDebugMode == DYNDEBUG_MIXED)
      avatarTest->dynamicsDebugger->UpdateDisplay ();

    // Fling the body a bit
    const csOrthoTransform& tc = avatarTest->view->GetCamera ()->GetTransform ();
    uint boneCount = ragdollNode->GetBoneCount (RAGDOLL_STATE_DYNAMIC);
    for (uint i = 0; i < boneCount; i++)
    {
      BoneID boneID = ragdollNode->GetBone (RAGDOLL_STATE_DYNAMIC, i);
      iRigidBody* rb = ragdollNode->GetBoneRigidBody (boneID);
      rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0.0f, 0.0f, 0.1f));
    }

    // Trace a physical beam to find which rigid body was hit
    csRef<iBulletDynamicSystem> bulletSystem =
      scfQueryInterface<iBulletDynamicSystem> (avatarTest->dynamicSystem);
    csBulletHitBeamResult physicsResult = bulletSystem->HitBeam (startBeam, endBeam);

    // Apply a big force at the point clicked by the mouse
    if (physicsResult.body)
    {
      csVector3 force = endBeam - startBeam;
      force.Normalize ();
      physicsResult.body->AddForceAtPos (physicsResult.isect, force * 5.0f);
      physicsResult.body->SetLinearVelocity (tc.GetT2O () * csVector3 (0.0f, 0.0f, 5.0f));
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

  animeshFactory = scfQueryInterface<iAnimatedMeshFactory>
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
      avatarTest->ragdollManager->CreateAnimNodeFactory ("ragdoll",
					     bodySkeleton, avatarTest->dynamicSystem);
    animPacketFactory->SetAnimationRoot (ragdollNodeFactory);
    ragdollNodeFactory->SetChildNode (randomNodeFactory);

    // Create bone chain for whole body and add it to the ragdoll controller. The chain
    // will be in kinematic mode when Krystal is alive, and in dynamic state when
    // Krystal has been killed.
    bodyChain = bodySkeleton->CreateBodyChain
      ("body_chain", animeshFactory->GetSkeletonFactory ()->FindBone ("Hips"),
       animeshFactory->GetSkeletonFactory ()->FindBone ("Head"),
       animeshFactory->GetSkeletonFactory ()->FindBone ("RightFoot"),
       animeshFactory->GetSkeletonFactory ()->FindBone ("RightHand"),
       animeshFactory->GetSkeletonFactory ()->FindBone ("LeftFoot"),
       animeshFactory->GetSkeletonFactory ()->FindBone ("LeftHand"), 0);
    ragdollNodeFactory->AddBodyChain (bodyChain, RAGDOLL_STATE_KINEMATIC);

    // Create bone chain for hairs and add it to the ragdoll controller. The chain will
    // always be in dynamic mode.
    hairChain = bodySkeleton->CreateBodyChain
      ("hair_chain", animeshFactory->GetSkeletonFactory ()->FindBone ("Hairs01"),
       animeshFactory->GetSkeletonFactory ()->FindBone ("Hairs06"), 0);
    ragdollNodeFactory->AddBodyChain (hairChain, RAGDOLL_STATE_DYNAMIC);
  }

  else
    animPacketFactory->SetAnimationRoot (randomNodeFactory);

  // Create the animated mesh
  csRef<iMeshWrapper> avatarMesh =
    avatarTest->engine->CreateMeshWrapper (meshfact, "krystal",
					   avatarTest->room, csVector3 (0.0f));
  animesh = scfQueryInterface<iAnimatedMesh> (avatarMesh->GetMeshObject ());

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
    ragdollNode->SetBodyChainState (bodyChain, RAGDOLL_STATE_KINEMATIC);

    // Reset the transform of the 'hairs' chain, since the mesh is moved abruptly,
    // otherwise it can lead to unstability.
    ragdollNode->ResetChainTransform (hairChain);

    // Update the display of the dynamics debugger
    if (avatarTest->dynamicsDebugMode == DYNDEBUG_COLLIDER
	|| avatarTest->dynamicsDebugMode == DYNDEBUG_MIXED)
      avatarTest->dynamicsDebugger->UpdateDisplay ();
  }
}

void KrystalScene::DisplayKeys ()
{
  int x = 20;
  int y = 20;
  int fg = avatarTest->g2d->FindRGB (255, 150, 100);
  int lineSize = 18;

  // Write available keys
  avatarTest->WriteShadow (x - 5, y, fg, "Keys available:");
  y += lineSize;

  if (avatarTest->physicsEnabled)
  {
    avatarTest->WriteShadow (x, y, fg, "left mouse: kill Krystal");
    y += lineSize;

    avatarTest->WriteShadow (x, y, fg, "d: display active colliders");
    y += lineSize;
  }

  avatarTest->WriteShadow (x, y, fg, "r: reset scene");
  y += lineSize;

  avatarTest->WriteShadow (x, y, fg, "m: switch to Frankie");
  y += lineSize;

  // Write FPS and other info
  y = 480;
  csTicks elapsed_time = avatarTest->vc->GetElapsedTicks ();
  const float speed = elapsed_time / 1000.0f;
  if (speed != 0.0f)
  {
    avatarTest->WriteShadow (x, y, fg, "FPS: %.2f", 1.0f / speed);
    y += lineSize;
  }
}
