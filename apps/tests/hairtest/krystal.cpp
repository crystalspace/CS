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
  : hairTest (hairTest)
{
  // Define the available keys
  hairTest->keyDescriptions.DeleteAll ();
  hairTest->keyDescriptions.Push ("arrow keys: move camera");
  hairTest->keyDescriptions.Push ("SHIFT-up/down keys: camera closer/farther");
  if (hairTest->physicsEnabled)
  {
    hairTest->keyDescriptions.Push ("d: display active colliders");
  }
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
    ragdollNodeFactory->AddBodyChain (bodyChain, CS_RAGDOLL_STATE_KINEMATIC);

	// Create the geometry for the hairs
	csRef<iGeneralFactoryState> hairsFactoryState =
		scfQueryInterface<iGeneralFactoryState> (hairsMeshFact->GetMeshObjectFactory ());
/*  
      // Create the mesh of the hairs
	hairsMesh = hairTest->engine->CreateMeshWrapper
		(hairsMeshFact, "krystal_hairs", hairTest->room, csVector3 (0.0f));
*/	
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

	iRigidBody* headBody = ragdollNode->GetBoneRigidBody
		(animeshFactory->GetSkeletonFactory ()->FindBone ("Head"));

	iAnimatedMeshSubMesh* skullmesh = animesh -> GetSubMesh(1);

	iRenderBuffer* indices = skullmesh->GetFactorySubMesh()->GetIndices(0);
	iRenderBuffer* vertices = animeshFactory->GetVertices();

	csRenderBufferLock<csVector3> positions (vertices, CS_BUF_LOCK_READ);
	csArray<int> uniqueIndices;
	CS::TriangleIndicesStream<size_t> tris (indices, CS_MESHTYPE_TRIANGLES);

	// chose unique indices
    while (tris.HasNext())
    {
      CS::TriangleT<size_t> tri (tris.Next ());

	  if(uniqueIndices.Contains(tri.a) == csArrayItemNotFound)
	    uniqueIndices.Push(tri.a);
	  if(uniqueIndices.Contains(tri.b) == csArrayItemNotFound)
	    uniqueIndices.Push(tri.b);
	  if(uniqueIndices.Contains(tri.c) == csArrayItemNotFound)
	    uniqueIndices.Push(tri.c);
    }

	// attach rope for them
	for (size_t i = 0; i < uniqueIndices.GetSize(); i ++)
	{
  	  csVector3 pos = positions.Get(uniqueIndices.Get(i));
	  csRef<iBulletSoftBody> bulletBody = hairTest->bulletDynamicSystem->
		CreateRope(pos,pos + csVector3(0,0.5f,0),5);
	  bulletBody->SetMass (0.1f);
	  bulletBody->SetRigidity (0.99f);
	  bulletBody->AnchorVertex (0, headBody);
	  //hairsBody.Push(bulletBody);
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
  if (hairTest->physicsEnabled)
  {
    // Reset the position of the animesh
    csRef<iMeshObject> animeshObject = scfQueryInterface<iMeshObject> (animesh);
    animeshObject->GetMeshWrapper ()->QuerySceneNode ()->GetMovable ()->SetTransform
      (csOrthoTransform (csMatrix3 (), csVector3 (0.0f)));
    animeshObject->GetMeshWrapper ()->QuerySceneNode ()->GetMovable ()->UpdateMove ();

    krystalDead = false;

    // Set the ragdoll state of the 'body' chain as kinematic
    ragdollNode->SetBodyChainState (bodyChain, CS_RAGDOLL_STATE_KINEMATIC);

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
  hairTest->stateDescriptions.DeleteAll ();
}
