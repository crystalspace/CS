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
#include "crystalin.h"

CrystalinScene::CrystalinScene (AvatarTest* avatarTest)
  : avatarTest (avatarTest)
{
}

CrystalinScene::~CrystalinScene ()
{
  // Remove the mesh from the scene
  csRef<iMeshObject> animeshObject = scfQueryInterface<iMeshObject> (animesh);
  avatarTest->engine->RemoveObject (animeshObject->GetMeshWrapper ());
}

csVector3 CrystalinScene::GetCameraStart ()
{
  return csVector3 (0.0f, 0.0f, -2.5f);
}

csVector3 CrystalinScene::GetCameraTarget ()
{
  csRef<iMeshObject> animeshObject = scfQueryInterface<iMeshObject> (animesh);
  csVector3 avatarPosition = animeshObject->GetMeshWrapper ()->QuerySceneNode ()
    ->GetMovable ()->GetTransform ().GetOrigin ();
  avatarPosition.y = 1.1f;
  return avatarPosition;
}

void CrystalinScene::Frame ()
{
}

bool CrystalinScene::OnKeyboard (iEvent &ev)
{
  return false;
}

bool CrystalinScene::OnMouseDown (iEvent &ev)
{
  return false;
}

bool CrystalinScene::CreateAvatar ()
{
  printf ("Loading Crystalin...\n");

  // Load animesh factory
  csLoadResult rc = avatarTest->loader->Load ("/lib/crystalin/crystalin.xml");
  if (!rc.success)
    return avatarTest->ReportError ("Can't load Crystalin library file!");

  csRef<iMeshFactoryWrapper> meshfact =
    avatarTest->engine->FindMeshFactory ("crystalin");
  if (!meshfact)
    return avatarTest->ReportError ("Can't find Crystalin's mesh factory!");

  animeshFactory = scfQueryInterface<iAnimatedMeshFactory>
    (meshfact->GetMeshObjectFactory ());
  if (!animeshFactory)
    return avatarTest->ReportError ("Can't find Crystalin's animesh factory!");

  // Create a new animation tree. The structure of the tree is:
  //   + Random node (root node)
  //     + idle animation nodes
  csRef<iSkeletonAnimPacketFactory2> animPacketFactory =
    animeshFactory->GetSkeletonFactory ()->GetAnimationPacket ();

  // Create the 'random' node
  csRef<iSkeletonRandomNodeFactory2> randomNodeFactory =
    animPacketFactory->CreateRandomNode ("random");
  randomNodeFactory->SetAutomaticSwitch (true);
  animPacketFactory->SetAnimationRoot (randomNodeFactory);

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

  // Create the animated mesh
  csRef<iMeshWrapper> avatarMesh =
    avatarTest->engine->CreateMeshWrapper (meshfact, "crystalin",
					   avatarTest->room, csVector3 (0.0f));
  animesh = scfQueryInterface<iAnimatedMesh> (avatarMesh->GetMeshObject ());

  // Reset the scene so as to put the parameters of the animation nodes in a default state
  ResetScene ();

  // Start animation
  iSkeletonAnimNode2* rootNode =
    animesh->GetSkeleton ()->GetAnimationPacket ()->GetAnimationRoot ();
  rootNode->Play ();

  return true;
}

void CrystalinScene::ResetScene ()
{
}

void CrystalinScene::DisplayKeys ()
{
  int x = 20;
  int y = 20;
  int fg = avatarTest->g2d->FindRGB (255, 150, 100);
  int lineSize = 18;

  // Write available keys
  avatarTest->WriteShadow (x - 5, y, fg, "Keys available:");
  y += lineSize;

  avatarTest->WriteShadow (x, y, fg, "m: switch to frankie");
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
