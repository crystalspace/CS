/*
    Copyright (C) 2001 by Jorrit Tyberghein

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __PHYSTUT_H__
#define __PHYSTUT_H__

#include <stdarg.h>
#include <crystalspace.h>
#include "dynsysdebug.h"

class Simple : public csApplicationFramework, public csBaseEventHandler
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
  csRef<iCollideSystem> cdsys;
  csRef<FramePrinter> printer;
  csRef<iFont> courierFont;

  // Pointers to main data
  csRef<iDynamics> dyn;
  csRef<iDynamicSystem> dynSys;
  csRef<iBulletDynamicSystem> bullet_dynSys;
  csRef<iMeshFactoryWrapper> boxFact;
  csRef<iMeshFactoryWrapper> meshFact;
  csRef<iMeshWrapper> walls;
  iSector* room;

  // Configuration related
  int solver;
  bool autodisable;
  csString phys_engine_name;
  int phys_engine_id;
  bool do_bullet_debug;

  // Dynamic simulation related
  csDynamicSystemDebugger dynSysDebugger;
  bool debugMode;
  bool allStatic;
  bool pauseDynamic;
  float dynamicSpeed;

  // Camera related
  int cameraMode;
  csRef<iRigidBody> cameraBody;
  float rotX, rotY, rotZ;

  // Ragdoll related
  csRef<iBodyManager> bodyManager;
  csRef<iSkeletonRagdollManager2> ragdollManager;
  CS::Animation::StateID ragdollState;
  csRef<iMeshWrapper> ragdollMesh;

  //-- csBaseEventHandler
  void Frame ();
  bool OnKeyboard (iEvent &event);
  bool OnMouseDown (iEvent &event);

  void UpdateCameraMode ();

  bool CreateStarCollider ();
  iRigidBody* CreateBox ();
  iRigidBody* CreateSphere ();
  iRigidBody* CreateCylinder ();
  iRigidBody* CreateCapsule ();
  iRigidBody* CreateMesh ();
  iRigidBody* CreateConvexMesh ();
  iJoint* CreateJointed ();
  void CreateChain ();
  void LoadRagdoll ();
  void CreateRagdoll ();
  void CreateWalls (const csVector3& radius);

  void DisplayKeys ();
  void WriteShadow (int x,int y,int fg,const char *str,...);
  void Write(int x,int y,int fg,int bg,const char *str,...);

public:
  Simple ();
  ~Simple ();

  //-- csApplicationFramework
  void OnExit ();
  bool OnInitialize (int argc, char* argv[]);
  bool Application ();
};

#endif // __PHYSTUT_H__

