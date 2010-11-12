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

KrystalScene::KrystalScene (HairTest* hairTest)
: hairTest (hairTest), hairPhysicsEnabled(true), isDead(false)
{
  // Define the available keys
  hairTest->hudHelper.keyDescriptions.DeleteAll ();
  hairTest->hudHelper.keyDescriptions.Push ("arrow keys: move camera");
  hairTest->hudHelper.keyDescriptions.Push ("SHIFT-up/down keys: camera closer/farther");
  
  if (hairTest->physicsEnabled)
    hairTest->hudHelper.keyDescriptions.Push ("d: display active colliders");

  hairTest->hudHelper.keyDescriptions.Push ("e: stop/start fur physics");
}

KrystalScene::~KrystalScene ()
{
  if (animesh)
  {
    // Remove the mesh from the scene
    csRef<iMeshObject> animeshObject = scfQueryInterface<iMeshObject> (animesh);
    hairTest->engine->RemoveObject (animeshObject->GetMeshWrapper ());
  }

  if (furMesh)
  {
    // Remove the fur mesh from the scene
    csRef<iMeshObject> furMeshObject = scfQueryInterface<iMeshObject> (furMesh);
    hairTest->engine->RemoveObject (furMeshObject->GetMeshWrapper ());
  }
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
  float distance = (hairTest->view->GetCamera ()->GetTransform ().GetOrigin ()
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
  if (isDead)
    return 0.3f;
  return 1.0f;
}

bool KrystalScene::HasPhysicalObjects ()
{
  return true;
}

void KrystalScene::SwitchFurPhysics()
{
  if (!furMesh)
    return;

  // Disable ropes
  if (hairPhysicsEnabled)
  {
    furMesh->StopAnimationControl();
    furMesh->SetAnimationControl(animationPhysicsControl);
    furMesh->StartAnimationControl();
    hairPhysicsEnabled = false;
  }
  else 
  {
    furMesh->StopAnimationControl();
    furMesh->SetAnimationControl(hairPhysicsControl);
    furMesh->StartAnimationControl();
    hairPhysicsEnabled = true;
  }
}

void KrystalScene::SaveFur()
{
  iMeshFactoryWrapper* meshfactwrap = 
    hairTest->engine->FindMeshFactory("krystal_furmesh_factory");
  if (!meshfactwrap)
  {
    hairTest->ReportError("Could not find krystal mesh factory!");
    return;
  }

  hairTest->SaveFactory(meshfactwrap, "/lib/krystal/krystal_furmesh_factory.save");

  iMeshWrapper* meshwrap = 
    hairTest->engine->FindMeshObject("krystal_furmesh_object");
  if (!meshwrap)
  {
    hairTest->ReportError("Could not find krystal mesh!");
    return;
  }

  hairTest->SaveObject(meshwrap, "/lib/krystal/krystal_furmesh_object.save");
}

bool KrystalScene::CreateAvatar ()
{
  printf ("Loading Krystal...\n");

  // Load animesh factory
  csLoadResult rc = hairTest->loader->Load ("/lib/krystal/krystal.xml");
  if (!rc.success)
    return hairTest->ReportError ("Can't load Krystal library file!");

  // Load some fur
  rc = hairTest->loader->Load ("/lib/krystal/krystal_furmesh.xml");
  if (!rc.success)
    return hairTest->ReportError ("Can't load krystal furmesh library!");

  csRef<iMeshWrapper> krystalFurmeshObject = 
    hairTest->engine->FindMeshObject("krystal_furmesh_object");
  if (!krystalFurmeshObject)
    return hairTest->ReportError ("Can't find fur mesh object!");

  csRef<iMeshFactoryWrapper> meshfact =
    hairTest->engine->FindMeshFactory ("krystal");
  if (!meshfact)
    return hairTest->ReportError ("Can't find Krystal's mesh factory!");

  animeshFactory = scfQueryInterface<CS::Mesh::iAnimatedMeshFactory>
    (meshfact->GetMeshObjectFactory ());
  if (!animeshFactory)
    return hairTest->ReportError ("Can't find Krystal's animesh factory!");

  // Load bodymesh (animesh's physical properties)
  rc = hairTest->loader->Load ("/lib/krystal/skelkrystal_body");
  if (!rc.success)
    return hairTest->ReportError ("Can't load Krystal's body mesh file!");

  csRef<CS::Animation::iBodyManager> bodyManager =
    csQueryRegistry<CS::Animation::iBodyManager> (hairTest->GetObjectRegistry ());
  csRef<CS::Animation::iBodySkeleton> bodySkeleton = 
    bodyManager->FindBodySkeleton ("krystal_body");
  if (!bodySkeleton)
    return hairTest->ReportError ("Can't find Krystal's body mesh description!");

  // Get plugin manager
  csRef<iPluginManager> plugmgr = 
    csQueryRegistry<iPluginManager> (hairTest->object_reg);
  if (!plugmgr)
    return hairTest->ReportError("Failed to locate Plugin Manager!");

  // Load furMesh
  csRef<CS::Mesh::iFurMeshType> furMeshType = 
    csQueryRegistry<CS::Mesh::iFurMeshType> (hairTest->object_reg);
  if (!furMeshType)
    return hairTest->ReportError("Failed to locate CS::Mesh::iFurMeshType plugin!");

  // Create a new animation tree. The structure of the tree is:
  //   + ragdoll controller node (root node - only if physics are enabled)
  //     + Random node
  //       + idle animation nodes
  csRef<CS::Animation::iSkeletonAnimPacketFactory> animPacketFactory =
    animeshFactory->GetSkeletonFactory ()->GetAnimationPacket ();

  // Create the 'random' node
  csRef<CS::Animation::iSkeletonRandomNodeFactory> randomNodeFactory =
    animPacketFactory->CreateRandomNode ("random");
  randomNodeFactory->SetAutomaticSwitch (true);

  // Create the 'idle01' animation node
  csRef<CS::Animation::iSkeletonAnimationNodeFactory> idle01NodeFactory =
    animPacketFactory->CreateAnimationNode ("idle01");
  idle01NodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("idle01"));

  // Create the 'idle02' animation node
  csRef<CS::Animation::iSkeletonAnimationNodeFactory> idle02NodeFactory =
    animPacketFactory->CreateAnimationNode ("idle02");
  idle02NodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("idle02"));

  // Create the 'idle03' animation node
  csRef<CS::Animation::iSkeletonAnimationNodeFactory> idle03NodeFactory =
    animPacketFactory->CreateAnimationNode ("idle03");
  idle03NodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("idle03"));

  // Create the 'idle04' animation node
  csRef<CS::Animation::iSkeletonAnimationNodeFactory> idle04NodeFactory =
    animPacketFactory->CreateAnimationNode ("idle04");
  idle04NodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("idle04"));

  // Create the 'idle05' animation node
  csRef<CS::Animation::iSkeletonAnimationNodeFactory> idle05NodeFactory =
    animPacketFactory->CreateAnimationNode ("idle05");
  idle05NodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("idle05"));

  // Create the 'idle06' animation node
  csRef<CS::Animation::iSkeletonAnimationNodeFactory> idle06NodeFactory =
    animPacketFactory->CreateAnimationNode ("idle06");
  idle06NodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("idle06"));

  // Create the 'stand' animation node
  csRef<CS::Animation::iSkeletonAnimationNodeFactory> standNodeFactory =
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
//   randomNodeFactory->AddNode (idle03NodeFactory, 1.0f);
  randomNodeFactory->AddNode (idle04NodeFactory, 1.0f);
//   randomNodeFactory->AddNode (idle05NodeFactory, 1.0f);
  randomNodeFactory->AddNode (idle06NodeFactory, 1.0f);
  randomNodeFactory->AddNode (standNodeFactory, 1.0f);
  
  if (hairTest->physicsEnabled)
  {
    // Create the ragdoll controller
    csRef<CS::Animation::iSkeletonRagdollNodeFactory> ragdollNodeFactory =
      hairTest->ragdollManager->CreateAnimNodeFactory
      ("ragdoll", bodySkeleton, hairTest->dynamicSystem);
    animPacketFactory->SetAnimationRoot (ragdollNodeFactory);
    ragdollNodeFactory->SetChildNode (randomNodeFactory);

    // Create bone chain for whole body and add it to the ragdoll controller. The chain
    // will be in kinematic mode when Krystal is alive, and in dynamic state when
    // Krystal has been killed.
    bodyChain = bodySkeleton->CreateBodyChain
      ("body_chain", animeshFactory->GetSkeletonFactory ()->FindBone ("Hips"));
    bodyChain->AddSubChain (animeshFactory->GetSkeletonFactory ()->FindBone ("Head"));
    bodyChain->AddSubChain (animeshFactory->GetSkeletonFactory ()->FindBone ("RightFoot"));
    bodyChain->AddSubChain (animeshFactory->GetSkeletonFactory ()->FindBone ("RightHand"));
    bodyChain->AddSubChain (animeshFactory->GetSkeletonFactory ()->FindBone ("LeftFoot"));
    bodyChain->AddSubChain (animeshFactory->GetSkeletonFactory ()->FindBone ("LeftHand"));
    ragdollNodeFactory->AddBodyChain (bodyChain, CS::Animation::STATE_KINEMATIC);

  }

  else
    animPacketFactory->SetAnimationRoot (randomNodeFactory);

  // Create the animated mesh
  csRef<iMeshWrapper> avatarMesh =
    hairTest->engine->CreateMeshWrapper (meshfact, "krystal",
    hairTest->room, csVector3 (0.0f));
  animesh = scfQueryInterface<CS::Mesh::iAnimatedMesh> (avatarMesh->GetMeshObject ());

  // When the animated mesh is created, the animation nodes are created too.
  // We can therefore set them up now.
  CS::Animation::iSkeletonAnimNode* rootNode =
    animesh->GetSkeleton ()->GetAnimationPacket ()->GetAnimationRoot ();

  // Setup of the ragdoll controller
  if (hairTest->physicsEnabled)
  {
    ragdollNode =
      scfQueryInterface<CS::Animation::iSkeletonRagdollNode> (rootNode->FindNode ("ragdoll"));

    // Start the ragdoll animation node in order to have the rigid bodies created
    ragdollNode->Play ();
  }

  csRef<iRigidBody> headBody = ragdollNode->GetBoneRigidBody
    (animeshFactory->GetSkeletonFactory ()->FindBone ("Head"));

  // Load fur material
  rc = hairTest-> loader ->Load ("/lib/hairtest/fur_material_krystal.xml");
  if (!rc.success)
    hairTest->ReportError("Can't load Fur library file!");

  // Load Marschner shader
  csRef<iMaterialWrapper> materialWrapper = 
    hairTest->engine->FindMaterial("marschner_material");
  if (!materialWrapper)
    hairTest->ReportError("Can't find marschner material!");

  // Create hairMeshProperties
  csRef<CS::Mesh::iFurMeshMaterialProperties> hairMeshProperties = 
    furMeshType->CreateHairMeshMarschnerProperties("krsytal_marschner");

  hairMeshProperties->SetMaterial(materialWrapper->GetMaterial());

  hairPhysicsControl = scfQueryInterface<CS::Animation::iFurPhysicsControl>
    (furMeshType->CreateFurPhysicsControl("krystal_hairs_physics"));
  animationPhysicsControl = scfQueryInterface<CS::Animation::iFurAnimatedMeshControl>
    (furMeshType->CreateFurAnimatedMeshControl("krystal_hairs_animation"));

  hairPhysicsControl->SetBulletDynamicSystem(hairTest->bulletDynamicSystem);
  hairPhysicsControl->SetRigidBody(headBody);
//   hairPhysicsControl->SetAnimatedMesh(animesh);

  animationPhysicsControl->SetAnimatedMesh(animesh);

  iSector* sector = hairTest->engine->FindSector("room");

  if (!sector)
    return hairTest->ReportError("Could not find default room!");

  krystalFurmeshObject->GetMovable()->SetSector(sector);
  krystalFurmeshObject->GetMovable()->UpdateMove();

  csRef<iMeshObject> imo = krystalFurmeshObject->GetMeshObject();

  // Get reference to the iFurMesh interface
  furMesh = scfQueryInterface<CS::Mesh::iFurMesh>(imo);

  csRef<CS::Mesh::iFurMeshState> ifms = 
    scfQueryInterface<CS::Mesh::iFurMeshState>(furMesh);

  animationPhysicsControl->SetDisplacement(ifms->GetDisplacement());

  furMesh->SetFurMeshProperties(hairMeshProperties);

  furMesh->SetAnimatedMesh(animesh);
  furMesh->SetMeshFactory(animeshFactory);
  furMesh->SetMeshFactorySubMesh(animesh -> GetSubMesh(1)->GetFactorySubMesh());
  furMesh->GenerateGeometry(hairTest->view, hairTest->room);

  furMesh->SetAnimationControl(hairPhysicsControl);
  furMesh->StartAnimationControl();

  furMesh->SetGuideLOD(0);
  furMesh->SetStrandLOD(1);
  furMesh->SetControlPointsLOD(0.0f);

  // Reset the scene so as to put the parameters of the animation nodes in a default state
  ResetScene ();

  return true;
}

void KrystalScene::KillAvatar()
{
  ragdollNode->SetBodyChainState (bodyChain, CS::Animation::STATE_DYNAMIC);
  hairTest->bulletDynamicSystem->SetStepParameters (0.008f, 150, 10);
  
  // Update the display of the dynamics debugger
  if (hairTest->dynamicsDebugMode == DYNDEBUG_COLLIDER
    || hairTest->dynamicsDebugMode == DYNDEBUG_MIXED)
    hairTest->dynamicsDebugger->UpdateDisplay ();

  // Fling the body a bit
  const csOrthoTransform& tc = hairTest->view->GetCamera ()->GetTransform ();
  uint boneCount = ragdollNode->GetBoneCount (CS::Animation::STATE_DYNAMIC);
  for (uint i = 0; i < boneCount; i++)
  {
    CS::Animation::BoneID boneID = ragdollNode->GetBone (CS::Animation::STATE_DYNAMIC, i);
    iRigidBody* rb = ragdollNode->GetBoneRigidBody (boneID);
    rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0.0f, 0.0f, 0.1f));
  }  

  isDead = true;
}

void KrystalScene::ResetScene ()
{
  if (hairTest->physicsEnabled)
  {
    // Reset the position of the animesh
    csRef<iMeshObject> animeshObject = scfQueryInterface<iMeshObject> (animesh);

    animeshObject->GetMeshWrapper ()->QuerySceneNode ()->GetMovable ()->SetTransform
      (csOrthoTransform (csMatrix3 (), csVector3 (0.0f)));
    animeshObject->GetMeshWrapper ()->QuerySceneNode ()->GetMovable ()->UpdateMove ();

    // Set the ragdoll state of the 'body' chain as kinematic
    ragdollNode->SetBodyChainState (bodyChain, CS::Animation::STATE_KINEMATIC);
    ragdollNode->SetPlaybackPosition(0);
    // Update the display of the dynamics debugger
    if (hairTest->dynamicsDebugMode == DYNDEBUG_COLLIDER
      || hairTest->dynamicsDebugMode == DYNDEBUG_MIXED)
      hairTest->dynamicsDebugger->UpdateDisplay ();

    // There are still big unlinerarities in the transition of Krystal's animations.
    // These gaps create a bad behavior of the simulation of the hairs, this is
    // better with lower step parameters.
    hairTest->bulletDynamicSystem->SetStepParameters (0.016667f, 1, 10);

    if (furMesh)
      furMesh->ResetMesh();
  }

  isDead = false;
}

void KrystalScene::UpdateStateDescription ()
{
  hairTest->hudHelper.stateDescriptions.DeleteAll ();
}
