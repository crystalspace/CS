/*
  Copyright (C) 2009-10 Christian Van Brussel, Communications and Remote
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
#ifndef __AVATARTEST_H__
#define __AVATARTEST_H__

#include <stdarg.h>
#include <crystalspace.h>

class AvatarTest : public csApplicationFramework, public csBaseEventHandler
{
private:
  // Engine related
  csRef<iEngine> engine;
  csRef<iLoader> loader;
  csRef<iGraphics3D> g3d;
  csRef<iGraphics2D> g2d;
  csRef<iKeyboardDriver> kbd;
  csRef<iVirtualClock> vc;
  csRef<iView> view;
  csRef<FramePrinter> printer;

  csRef<iFont> courierFont;
  iSector* room;

  // Physics related
  bool physicsEnabled;
  csRef<iDynamics> dynamics;
  csRef<iDynamicSystem> dynamicSystem;

  // Animesh & animation nodes
  csRef<iAnimatedMeshFactory> animeshFactory;
  csRef<iAnimatedMesh> animesh;
  csRef<iSkeletonLookAtManager2> lookAtManager;
  csRef<iSkeletonBasicNodesManager2> basicNodesManager;
  csRef<iSkeletonFSMNode2> FSMNode;

  // LookAt node related
  csRef<iSkeletonLookAtNode2> lookAtNode;
  char targetMode;
  bool alwaysRotate;
  char rotationSpeed;
  bool targetReached;

  // Speed node related
  csRef<iSkeletonSpeedNode2> speedNode;
  int currentSpeed; // We use a 'int' instead of a 'float' to avoid round errors

  // Ragdoll node related
  bool frankieDead;
  csRef<iSkeletonRagdollManager2> ragdollManager;
  csRef<iSkeletonRagdollNode2> ragdollNode;
  CS::Animation::StateID mainFSMState;
  CS::Animation::StateID ragdollFSMState;

  // Morphing related
  float smileWeight;

  //-- csBaseEventHandler
  void Frame ();
  bool OnKeyboard (iEvent &event);
  bool OnMouseDown (iEvent &event);

  // Creation of objects
  void CreateRoom ();
  bool CreateAvatar ();

  // User interaction with the scene
  void ResetScene ();

  // Display of comments 
  void WriteShadow (int x, int y, int fg, const char *str,...);
  void Write(int x, int y, int fg, int bg, const char *str,...);
  void DisplayKeys ();

  // LookAt listener
  class LookAtListener : public scfImplementation1<LookAtListener,
    iSkeletonLookAtListener2>
  {
    AvatarTest* avatarTest;

  public:
    LookAtListener (AvatarTest* avatarTest)
      : scfImplementationType (this), avatarTest (avatarTest) {}

    //-- iSkeletonLookAtListener2
    void TargetReached ();
    void TargetLost ();
  } lookAtListener;

 public:
  AvatarTest ();
  ~AvatarTest ();

  //-- csApplicationFramework
  void OnExit ();
  bool OnInitialize (int argc, char* argv[]);
  bool Application ();
};

#endif // __AVATARTEST_H__

