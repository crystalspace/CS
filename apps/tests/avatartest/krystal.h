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
#ifndef __KRYSTAL_H__
#define __KRYSTAL_H__

#include "avatartest.h"

class KrystalScene : public AvatarScene
{
 public:
  KrystalScene (AvatarTest* avatarTest);
  ~KrystalScene ();

  // Camera related
  csVector3 GetCameraTarget ();

  // Dynamic simulation related
  float GetSimulationSpeed ();
  bool HasPhysicalObjects ();

  // From csBaseEventHandler
  void Frame ();
  bool OnKeyboard (iEvent &event);
  bool OnMouseDown (iEvent &event);
  bool OnMouseUp (iEvent &event);

  // Creation of objects
  bool CreateAvatar ();

  // User interaction with the scene
  void ResetScene ();
  void ResetSoftBodies ();

  // Display of information on the state of the scene
  void PostFrame ();

 private:
  AvatarTest* avatarTest;

  // Body chains
  CS::Animation::iBodyChain* bodyChain;
  CS::Animation::iBodyChain* armChain;

  // Debug node related
  bool debugBones, debugBBoxes;
  csRef<CS::Animation::iSkeletonDebugNode> debugNode;
  csRef<CS::Animation::iSkeletonDebugNodeFactory> debugNodeFactory;

  // Ragdoll management related
  bool krystalDead;
  csRef<CS::Animation::iSkeletonRagdollNode> ragdollNode;

  // Inverse Kinematics management related
  bool IKenabled;
  csRef<CS::Animation::iSkeletonIKNode> IKNode;
  CS::Animation::EffectorID handEffector;
  csRef<iMeshWrapper> IKMesh;
  bool IKdragging;
  float IKDragDistance;
  csVector3 IKDragOffset;

  // Krystal's hairs & skirt (and their soft bodies)
  csRef<iMeshFactoryWrapper> hairsMeshFact;
  csRef<iMeshFactoryWrapper> skirtMeshFact;
  csRef<iMeshWrapper> hairsMesh;
  csRef<iMeshWrapper> skirtMesh;
  csRef<CS::Physics::Bullet::iSoftBody> hairsBody;
  csRef<CS::Physics::Bullet::iSoftBody> skirtBody;
};

#endif // __KRYSTAL_H__
