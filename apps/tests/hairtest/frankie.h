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

#ifndef __FRANKIE_H__
#define __FRANKIE_H__

#include "hairtest.h"

class FrankieScene : public AvatarScene
{
public:
  FrankieScene (HairTest* hairTest);
  ~FrankieScene ();

  // Camera related
  csVector3 GetCameraTarget ();

  // Dynamic simulation related
  float GetSimulationSpeed ();
  bool HasPhysicalObjects ();

  // Creation of objects
  bool CreateAvatar ();

  // Kill the avatar
  void KillAvatar ();

  // User interaction with the scene
  void ResetScene ();

  // Switch Fur Physics
  void SwitchFurPhysics();

  // Save fur
  void SaveFur();

private:
  HairTest* hairTest;

  // Ragdoll node related
  csRef<CS::Animation::iSkeletonRagdollNode> ragdollNode;
  CS::Animation::iBodyChain* bodyChain;
  CS::Animation::iBodyChain* tailChain;
  
  // Morphing related
  float smileWeight;

  // Fur physics
  bool furPhysicsEnabled;
  csRef<CS::Animation::iFurPhysicsControl> furPhysicsControl;
  csRef<CS::Animation::iFurAnimatedMeshControl> animationPhysicsControl;
};

#endif // __FRANKIE_H__
