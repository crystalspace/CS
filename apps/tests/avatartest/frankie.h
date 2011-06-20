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

#include "avatartest.h"

struct iDecal;

class FrankieScene : public AvatarScene
{
 public:
  FrankieScene (AvatarTest* avatarTest);
  ~FrankieScene ();

  // Camera related
  csVector3 GetCameraTarget ();

  // Dynamic simulation related
  float GetSimulationSpeed ();
  bool HasPhysicalObjects ();

  // From csBaseEventHandler
  void Frame ();
  bool OnKeyboard (iEvent &event);
  bool OnMouseDown (iEvent &event);

  // Creation of objects
  bool CreateAvatar ();

  // User interaction with the scene
  void ResetScene ();

  // Display of information on the state of the scene
  void UpdateStateDescription ();
  void PostFrame ();

 private:
  AvatarTest* avatarTest;

  // Debug node related
  bool debugBones, debugBBoxes;
  csRef<CS::Animation::iSkeletonDebugNode> debugNode;
  csRef<CS::Animation::iSkeletonDebugNodeFactory> debugNodeFactory;

  // LookAt node related
  csRef<CS::Animation::iSkeletonLookAtNode> lookAtNode;
  csRef<CS::Animation::iSkeletonLookAtNodeFactory> lookAtNodeFactory;
  char targetMode;
  bool alwaysRotate;
  char rotationSpeed;
  bool targetReached;

  // LookAt listener
  class LookAtListener : public scfImplementation1<LookAtListener,
    CS::Animation::iSkeletonLookAtListener>
  {
    FrankieScene* frankieScene;

  public:
    LookAtListener (FrankieScene* frankieScene)
      : scfImplementationType (this), frankieScene (frankieScene) {}

    //-- CS::Animation::iSkeletonLookAtListener
    void TargetReached ();
    void TargetLost ();
  } lookAtListener;

  // Speed node related
  csRef<CS::Animation::iSkeletonSpeedNode> speedNode;
  int currentSpeed; // We use a 'int' instead of a 'float' to avoid
                    // accumulated rounding errors

  // Ragdoll node related
  bool frankieDead;
  csRef<CS::Animation::iSkeletonRagdollNode> ragdollNode;
  CS::Animation::iBodyChain* bodyChain;
  CS::Animation::iBodyChain* tailChain;

  // Morphing related
  float smileWeight;

  // Decals related
  bool decalsEnabled;
  csRef<iDecalTemplate> decalTemplate;
  iDecal* decal;
  csVector3 decalPosition;
};

#endif // __FRANKIE_H__
