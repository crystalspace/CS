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
#ifndef __FRANKIE_H__
#define __FRANKIE_H__

#include "avatartest.h"

class FrankieScene : public AvatarScene
{
 public:
  FrankieScene (AvatarTest* avatarTest);
  ~FrankieScene ();

  // Camera related
  csVector3 GetCameraStart ();
  float GetCameraMinimumDistance ();
  csVector3 GetCameraTarget ();

  // Dynamic simuation related
  float GetSimulationSpeed ();

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

 private:
  AvatarTest* avatarTest;

  // FSM node related
  csRef<iSkeletonFSMNode2> FSMNode;
  CS::Animation::StateID mainFSMState;
  CS::Animation::StateID ragdollFSMState;

  // LookAt node related
  csRef<iSkeletonLookAtNode2> lookAtNode;
  char targetMode;
  bool alwaysRotate;
  char rotationSpeed;
  bool targetReached;

  // LookAt listener
  class LookAtListener : public scfImplementation1<LookAtListener,
    iSkeletonLookAtListener2>
  {
    FrankieScene* frankieScene;

  public:
    LookAtListener (FrankieScene* frankieScene)
      : scfImplementationType (this), frankieScene (frankieScene) {}

    //-- iSkeletonLookAtListener2
    void TargetReached ();
    void TargetLost ();
  } lookAtListener;

  // Speed node related
  csRef<iSkeletonSpeedNode2> speedNode;
  int currentSpeed; // We use a 'int' instead of a 'float' to avoid
                    // accumulated rounding errors

  // Ragdoll node related
  bool frankieDead;
  csRef<iSkeletonRagdollNode2> ragdollNode;

  // Morphing related
  float smileWeight;
};

#endif // __FRANKIE_H__
