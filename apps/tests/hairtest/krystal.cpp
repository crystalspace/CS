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
: hairTest (hairTest), hairPhysicsEnabled(true)
{
  // Define the available keys
  hairTest->hudHelper.keyDescriptions.DeleteAll ();
  hairTest->hudHelper.keyDescriptions.Push ("arrow keys: move camera");
  hairTest->hudHelper.keyDescriptions.Push ("SHIFT-up/down keys: camera closer/farther");
  
  if (hairTest->physicsEnabled)
  {
    hairTest->hudHelper.keyDescriptions.Push ("d: display active colliders");
  }

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
  /*
  for (size_t i = 0; i < hairsBody.GetSize(); i ++)
  hairTest->bulletDynamicSystem->RemoveSoftBody (hairsBody.Get(i));

  hairsBody.DeleteAll();
  */

  if (hairsMesh)
    hairTest->engine->RemoveObject (hairsMesh);

  if (hairsBody)
    hairTest->bulletDynamicSystem->RemoveSoftBody (hairsBody);

  if (skirtMesh)
    hairTest->engine->RemoveObject (skirtMesh);

  if (skirtBody)
    hairTest->bulletDynamicSystem->RemoveSoftBody (skirtBody);
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
  if (krystalDead)
    return 0.3f;

  return 1.0f;
}

bool KrystalScene::HasPhysicalObjects ()
{
  return true;
}

void KrystalScene::SwitchFurPhysics()
{
  if (!furMaterial)
    return;

  if (hairPhysicsEnabled)
  {
    furMaterial->SetGuideLOD(0.0f);
    furMaterial->StopPhysicsControl();
    animationPhysicsControl->SetRigidBody(headBody);
    furMaterial->SetPhysicsControl(animationPhysicsControl);
    furMaterial->StartPhysicsControl();
    hairPhysicsEnabled = false;
  }
  else 
  {
    furMaterial->SetGuideLOD(0.0f);
    furMaterial->StopPhysicsControl();
    furMaterial->SetPhysicsControl(hairPhysicsControl);
    furMaterial->StartPhysicsControl();
    hairPhysicsEnabled = true;
  }
}

bool KrystalScene::CreateAvatar ()
{
  printf ("Loading Krystal...\n");

  // Load animesh factory
  csLoadResult rc = hairTest->loader->Load ("/lib/krystal/krystal.xml");
  if (!rc.success)
    return hairTest->ReportError ("Can't load Krystal library file!");

  csRef<iMeshFactoryWrapper> meshfact =
    hairTest->engine->FindMeshFactory ("krystal");
  if (!meshfact)
    return hairTest->ReportError ("Can't find Krystal's mesh factory!");

  animeshFactory = scfQueryInterface<iAnimatedMeshFactory>
    (meshfact->GetMeshObjectFactory ());
  if (!animeshFactory)
    return hairTest->ReportError ("Can't find Krystal's animesh factory!");

  csRef<iMaterialWrapper> skullMaterial = 
    hairTest->engine->FindMaterial("skull_material");
  if (!skullMaterial)	
    return hairTest->ReportError ("Can't find Krystal's skull material!");

  // Load bodymesh (animesh's physical properties)
  rc = hairTest->loader->Load ("/lib/krystal/skelkrystal_body");
  if (!rc.success)
    return hairTest->ReportError ("Can't load Krystal's body mesh file!");

  csRef<iBodyManager> bodyManager =
    csQueryRegistry<iBodyManager> (hairTest->GetObjectRegistry ());
  csRef<iBodySkeleton> bodySkeleton = bodyManager->FindBodySkeleton ("krystal_body");
  if (!bodySkeleton)
    return hairTest->ReportError ("Can't find Krystal's body mesh description!");

  // Load Krystal's hairs
  rc = hairTest->loader->Load ("/lib/krystal/krystal_hairs.xml");
  if (!rc.success)
    return hairTest->ReportError ("Can't load Krystal's hairs library file!");

  csRef<iMeshFactoryWrapper> hairsMeshFact =
    hairTest->engine->FindMeshFactory ("krystal_hairs");
  if (!hairsMeshFact)
    return hairTest->ReportError ("Can't find Krystal's hairs mesh factory!");  

  // Load Krystal's skirt
  rc = hairTest->loader->Load ("/lib/krystal/krystal_skirt.xml");
  if (!rc.success)
    return hairTest->ReportError ("Can't load Krystal's skirt library file!");

  csRef<iMeshFactoryWrapper> skirtMeshFact =
    hairTest->engine->FindMeshFactory ("krystal_skirt");
  if (!skirtMeshFact)
    return hairTest->ReportError ("Can't find Krystal's skirt mesh factory!");

  // Get plugin manager
  csRef<iPluginManager> plugmgr = 
    csQueryRegistry<iPluginManager> (hairTest->object_reg);
  if (!plugmgr)
    return hairTest->ReportError("Failed to locate Plugin Manager!");

  // Load hairPhysicsControl
  hairPhysicsControl = csLoadPlugin<iFurPhysicsControl>
    (plugmgr, "crystalspace.physics.hairphysics");
  if (!hairPhysicsControl)
    return hairTest->ReportError("Failed to locate iFurPhysicsControl plugin!");

  // Load animationPhysicsControl
  animationPhysicsControl = csLoadPlugin<iFurPhysicsControl>
    (plugmgr, "crystalspace.physics.animationphysics");
  if (!animationPhysicsControl)
    return hairTest->ReportError("Failed to locate iFurPhysicsControl plugin!");

  // Load hairStrandGenerator
  csRef<iFurStrandGenerator> hairStrandGenerator = csQueryRegistry<iFurStrandGenerator> 
    (hairTest->object_reg);
  if (!hairStrandGenerator)
    return hairTest->ReportError("Failed to locate iFurStrandGenerator plugin!");

  // Load furMaterial
  csRef<iFurMaterialType> furMaterialType = csQueryRegistry<iFurMaterialType> 
    (hairTest->object_reg);
  if (!furMaterialType)
    return hairTest->ReportError("Failed to locate iFurMaterialType plugin!");

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
  //randomNodeFactory->AddNode (idle03NodeFactory, 1.0f);
  randomNodeFactory->AddNode (idle04NodeFactory, 1.0f);
  //randomNodeFactory->AddNode (idle05NodeFactory, 1.0f);
  randomNodeFactory->AddNode (idle06NodeFactory, 1.0f);
  randomNodeFactory->AddNode (standNodeFactory, 1.0f);
  
  if (hairTest->physicsEnabled)
  {
    // Create the ragdoll controller
    csRef<iSkeletonRagdollNodeFactory2> ragdollNodeFactory =
      hairTest->ragdollManager->CreateAnimNodeFactory
      ("ragdoll", bodySkeleton, hairTest->dynamicSystem);
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
    ragdollNodeFactory->AddBodyChain (bodyChain, CS::Animation::STATE_KINEMATIC);

    // Create the mesh of the skirt
    skirtMesh = hairTest->engine->CreateMeshWrapper
      (skirtMeshFact, "krystal_skirt", hairTest->room, csVector3 (0.0f));

    // Create the geometry for the hairs
    csRef<iGeneralFactoryState> hairsFactoryState =
      scfQueryInterface<iGeneralFactoryState> (hairsMeshFact->GetMeshObjectFactory ());
    
    // Create the mesh of the hairs
//     hairsMesh = hairTest->engine->CreateMeshWrapper
//     (hairsMeshFact, "krystal_hairs", hairTest->room, csVector3 (0.0f));
  }

  else
    animPacketFactory->SetAnimationRoot (randomNodeFactory);

  // Create the animated mesh
  csRef<iMeshWrapper> avatarMesh =
    hairTest->engine->CreateMeshWrapper (meshfact, "krystal",
    hairTest->room, csVector3 (0.0f));
  animesh = scfQueryInterface<iAnimatedMesh> (avatarMesh->GetMeshObject ());

  // When the animated mesh is created, the animation nodes are created too.
  // We can therefore set them up now.
  iSkeletonAnimNode2* rootNode =
    animesh->GetSkeleton ()->GetAnimationPacket ()->GetAnimationRoot ();

  // Setup of the ragdoll controller
  if (hairTest->physicsEnabled)
  {
    ragdollNode =
      scfQueryInterface<iSkeletonRagdollNode2> (rootNode->FindNode ("ragdoll"));
    ragdollNode->SetAnimatedMesh (animesh);

    // Start the ragdoll animation node in order to have the rigid bodies created
    ragdollNode->Play ();
  }

  headBody = ragdollNode->GetBoneRigidBody
    (animeshFactory->GetSkeletonFactory ()->FindBone ("Head"));

  rc = hairTest-> loader ->Load ("/hairtest/fur_material.xml");
  if (!rc.success)
    hairTest->ReportError("Can't load Fur library file!");

  csRef<iMaterialWrapper> materialWrapper = 
    hairTest->engine->FindMaterial("marschner_material");
  if (!materialWrapper)
    hairTest->ReportError("Can't find marschner material!");

  hairStrandGenerator->SetMaterial(materialWrapper->GetMaterial());

  hairPhysicsControl->SetBulletDynamicSystem(hairTest->bulletDynamicSystem);
  hairPhysicsControl->SetRigidBody(headBody);

  animationPhysicsControl->SetRigidBody(headBody);

  // Initialize fur material
  furMaterial = furMaterialType->CreateFurMaterial("hair");
  furMaterial->SetPhysicsControl(animationPhysicsControl);
  furMaterial->SetFurStrandGenerator(hairStrandGenerator);

  furMaterial->SetMeshFactory(animeshFactory);
  furMaterial->SetMeshFactorySubMesh(animesh -> GetSubMesh(1)->GetFactorySubMesh());
  furMaterial->SetMaterial(skullMaterial->GetMaterial());
  furMaterial->GenerateGeometry(hairTest->view, hairTest->room);
  furMaterial->SetGuideLOD(0);
  furMaterial->SetStrandLOD(1);

  // add light info for marschner
  csRef<iShaderVarStringSet> svStrings = 
    csQueryRegistryTagInterface<iShaderVarStringSet> (
      hairTest->object_reg, "crystalspace.shader.variablenameset");

  if (!svStrings) 
    csPrintfErr ("No SV names string set!");

  hairTest->room->GetLights()->RemoveAll();

  // This light is for the background
  csRef<iLight> light = 
    hairTest->engine->CreateLight(0, csVector3(10, 10, 0), 9000, csColor (1));
  light->SetAttenuationMode (CS_ATTN_NONE);
  light->SetType(CS_LIGHT_DIRECTIONAL);
  csMatrix3 matrixY (cos(PI/2), 0, -sin(PI/2), 0, 1, 0, sin(PI/2), 0, cos(PI/2)); // PI/4
  csMatrix3 matrixX (1, 0, 0, 0, cos(PI/2), -sin(PI/2), 0, sin(PI/2), cos(PI/2));
  light->GetMovable()->Transform(matrixY);
  hairTest->room->GetLights()->Add (light);

  // Add plane
//   csRef<iMeshFactoryWrapper> planeFactory = hairTest->engine->CreateMeshFactory (
//     "crystalspace.mesh.object.genmesh", "planeFactory");
// 
//   csRef<iGeneralFactoryState> planeFactoryState = 
//     scfQueryInterface<iGeneralFactoryState> ( planeFactory->GetMeshObjectFactory ());
// 
//   planeFactoryState -> SetVertexCount ( 4 );
//   planeFactoryState -> SetTriangleCount ( 2 );
// 
//   csVector3 *vbuf = planeFactoryState->GetVertices (); 
//   csTriangle *ibuf = planeFactoryState->GetTriangles ();
// 
//   vbuf[0] = csVector3(-10, 0, -10);
//   vbuf[1] = csVector3(10, 0, -10);
//   vbuf[2] = csVector3(10, 0, 10);
//   vbuf[3] = csVector3(-10, 0, 10);
// 
//   ibuf[0] = csTriangle(0, 2, 1);
//   ibuf[1] = csTriangle(3, 2, 0);
// 
//   planeFactoryState -> CalculateNormals();
//   planeFactoryState -> Invalidate();
// 
//   // Make a material
//   csRef<iMeshWrapper> planeMeshWrapper = hairTest->engine->CreateMeshWrapper 
//     (planeFactory, "plane", hairTest->room, csVector3 (0, 0, 0));
// 
//   csRef<iMaterialWrapper> planeMaterialWrapper = 
//     CS::Material::MaterialBuilder::CreateColorMaterial
//     (hairTest->object_reg,"planeMaterial",csColor(0,0,1));
// 
//   planeMeshWrapper->GetMeshObject()->SetMaterialWrapper(planeMaterialWrapper);

  // Start animation
  rootNode->Play ();

  // Reset the scene so as to put the parameters of the animation nodes in a default state
  ResetScene ();

  return true;
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

    krystalDead = false;

    // Set the ragdoll state of the 'body' chain as kinematic
    ragdollNode->SetBodyChainState (bodyChain, CS::Animation::STATE_KINEMATIC);

    // Update the display of the dynamics debugger
    if (hairTest->dynamicsDebugMode == DYNDEBUG_COLLIDER
      || hairTest->dynamicsDebugMode == DYNDEBUG_MIXED)
      hairTest->dynamicsDebugger->UpdateDisplay ();

    // There are still big unlinerarities in the transition of Krystal's animations.
    // These gaps create a bad behavior of the simulation of the hairs, this is
    // better with lower step parameters.
    hairTest->bulletDynamicSystem->SetStepParameters (0.016667f, 1, 10);
  }
}

void KrystalScene::UpdateStateDescription ()
{
  hairTest->hudHelper.stateDescriptions.DeleteAll ();
}
