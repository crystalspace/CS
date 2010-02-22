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
  csRef<iMeshObject> animeshObject = scfQueryInterface<iMeshObject> (animesh);
  csVector3 avatarPosition = animeshObject->GetMeshWrapper ()->QuerySceneNode ()
    ->GetMovable ()->GetTransform ().GetOrigin ();
  avatarPosition.y = 1.1f;
  return avatarPosition;
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
	physicsResult.body->AddForceAtPos (physicsResult.isect, force * 10.0f);
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

    // Set the ragdoll animation node as the active state of the Finite State Machine
    // (start the ragdoll node so that the rigid bodies are created)
    FSMNode->SwitchToState (ragdollFSMState);
    FSMNode->GetStateNode (ragdollFSMState)->Play ();

    // Fling the body a bit
    const csOrthoTransform& tc = avatarTest->view->GetCamera ()->GetTransform ();
    for (uint i = 0; i < ragdollNode->GetBoneCount (RAGDOLL_STATE_DYNAMIC); i++)
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
      physicsResult.body->AddForceAtPos (physicsResult.isect, force * 1.0f);
      physicsResult.body->SetLinearVelocity (tc.GetT2O () * csVector3 (0.0f, 0.0f, 1.0f));
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
  //   + Finite State Machine node (root node)
  //     + Random node
  //       + idle animation nodes
  //     + ragdoll controller node
  csRef<iSkeletonAnimPacketFactory2> animPacketFactory =
    animeshFactory->GetSkeletonFactory ()->GetAnimationPacket ();

  // Create the Finite State Machine node
  csRef<iSkeletonFSMNodeFactory2> FSMNodeFactory =
    animPacketFactory->CreateFSMNode ("fsm");
  animPacketFactory->SetAnimationRoot (FSMNodeFactory);

  // Create the 'random' node
  csRef<iSkeletonRandomNodeFactory2> randomNodeFactory =
    animPacketFactory->CreateRandomNode ("random");
  randomNodeFactory->SetAutomaticSwitch (true);
  mainFSMState = FSMNodeFactory->AddState
    ("main_state", randomNodeFactory);
  FSMNodeFactory->SetStartState (mainFSMState);

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
    ragdollFSMState = FSMNodeFactory->AddState
      ("ragdoll_state", ragdollNodeFactory);

    // Create bone chain
    iBodyChain* chain = bodySkeleton->CreateBodyChain
      ("body_chain", animeshFactory->GetSkeletonFactory ()->FindBone ("Hips"),
       animeshFactory->GetSkeletonFactory ()->FindBone ("Head"),
       animeshFactory->GetSkeletonFactory ()->FindBone ("RightFoot"),
       animeshFactory->GetSkeletonFactory ()->FindBone ("LeftFoot"),
       animeshFactory->GetSkeletonFactory ()->FindBone ("RightHand"),
       animeshFactory->GetSkeletonFactory ()->FindBone ("LeftHand"), 0);
    ragdollNodeFactory->AddBodyChain (chain, RAGDOLL_STATE_DYNAMIC);
  }

  // Create the animated mesh
  csRef<iMeshWrapper> avatarMesh =
    avatarTest->engine->CreateMeshWrapper (meshfact, "krystal",
					   avatarTest->room, csVector3 (0.0f));
  animesh = scfQueryInterface<iAnimatedMesh> (avatarMesh->GetMeshObject ());

  // When the animated mesh is created, the animation nodes are created too.
  // We can therefore set them up now.
  iSkeletonAnimNode2* rootNode =
    animesh->GetSkeleton ()->GetAnimationPacket ()->GetAnimationRoot ();

  // Setup of the FSM node
  FSMNode = scfQueryInterface<iSkeletonFSMNode2> (rootNode->FindNode ("fsm"));

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

void KrystalScene::ResetScene ()
{
  // Reset the position of the animesh
  csRef<iMeshObject> animeshObject = scfQueryInterface<iMeshObject> (animesh);
  animeshObject->GetMeshWrapper ()->QuerySceneNode ()->GetMovable ()->SetTransform
    (csOrthoTransform (csMatrix3 (), csVector3 (0.0f)));
  animeshObject->GetMeshWrapper ()->QuerySceneNode ()->GetMovable ()->UpdateMove ();

  // Reset initial state of the Finite State Machine
  FSMNode->SwitchToState (mainFSMState);
  //FSMNode->SwitchToState (ragdollFSMState);
  //FSMNodeFactory->SetStartState (ragdollFSMState);

  // The FSM doesn't stop the child nodes
  if (avatarTest->physicsEnabled)
    ragdollNode->Stop ();

  krystalDead = false;
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
