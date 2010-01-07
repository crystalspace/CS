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
#include "csutil/scf_implementation.h"

// TODO: to be removed
class iSkeletonBasicNodesManager2;
class iSkeletonSpeedNode2;

class AvatarTest : public scfImplementation1<AvatarTest, iLookAtListener>
{
private:
  iObjectRegistry* object_reg;
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

  csRef<iAnimatedMeshFactory> animeshFactory;
  csRef<iAnimatedMesh> animesh;
  csRef<iBodyManager> bodyManager;
  csRef<iLookAtManager> lookAtManager;
  csRef<iSkeletonBasicNodesManager2> basicNodesManager;

  csRef<iLookAtAnimNode> lookAtNode;
  char targetMode;
  bool alwaysRotate;
  char rotationSpeed;
  bool targetReached;

  csRef<iSkeletonSpeedNode2> speedNode;
  // We use a 'int' instead of a 'float' to avoid round errors
  int currentSpeed;

  float smileWeight;

  static bool AvatarTestEventHandler (iEvent& ev);
  bool HandleEvent (iEvent& ev);
  void SetupFrame ();

  void CreateRoom ();
  void CreateAvatar ();

  void WriteShadow (int x,int y,int fg,const char *str,...);
  void Write(int x,int y,int fg,int bg,const char *str,...);
  void DisplayKeys ();

  CS_DECLARE_EVENT_SHORTCUTS;

  csEventID KeyboardDown;
  csEventID KeyboardUp;

 public:
  AvatarTest (iObjectRegistry *obj);
  ~AvatarTest ();

  bool Initialize ();
  void Start ();
  void Shutdown ();

  //-- iLookAtListener
  void TargetReached ();
  void TargetLost ();
};

#endif // __AVATARTEST_H__

