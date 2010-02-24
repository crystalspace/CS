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

class AvatarScene
{
 public:
  virtual ~AvatarScene () {}

  // Camera related
  virtual csVector3 GetCameraStart () = 0;
  virtual csVector3 GetCameraTarget () = 0;

  // From csBaseEventHandler
  virtual void Frame () = 0;
  virtual bool OnKeyboard (iEvent &event) = 0;
  virtual bool OnMouseDown (iEvent &event) = 0;

  // Creation of objects
  virtual bool CreateAvatar () = 0;

  // User interaction with the scene
  virtual void ResetScene () = 0;

  // Display of comments 
  virtual void DisplayKeys () = 0;
};

class AvatarTest : public csApplicationFramework, public csBaseEventHandler
{
  friend class FrankieScene;
  friend class KrystalScene;

private:
  AvatarScene* avatarScene;

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

  // Animation node plugin managers
  csRef<iSkeletonLookAtManager2> lookAtManager;
  csRef<iSkeletonBasicNodesManager2> basicNodesManager;
  csRef<iSkeletonRagdollManager2> ragdollManager;

  //-- csBaseEventHandler
  void Frame ();
  bool OnKeyboard (iEvent &event);
  bool OnMouseDown (iEvent &event);

  // Creation of objects
  void CreateRoom ();
  int avatarModel;

  // Display of comments 
  void WriteShadow (int x, int y, int fg, const char *str,...);
  void Write(int x, int y, int fg, int bg, const char *str,...);

 public:
  AvatarTest ();
  ~AvatarTest ();

  //-- csApplicationFramework
  void OnExit ();
  bool OnInitialize (int argc, char* argv[]);
  bool Application ();
};

#endif // __AVATARTEST_H__

