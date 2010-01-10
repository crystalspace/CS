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
#include <ivaria/ode.h>
#include "dynsysdebug.h"

// TODO: remove that
#include "imesh/ragdoll.h"

class Simple
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
  csRef<iCollideSystem> cdsys;
  csRef<FramePrinter> printer;
  iSector* room;
  int solver;
  bool disable;

  csString phys_engine_name;
  int  phys_engine_id;

  csRef<iDynamics> dyn;
  csRef<iDynamicSystem> dynSys;
  csRef<iBulletDynamicSystem> bullet_dynSys;
  csRef<iMeshFactoryWrapper> boxFact;
  csRef<iMeshFactoryWrapper> meshFact;
  csRef<iFont> courierFont;
  bool do_bullet_debug;

  static bool SimpleEventHandler (iEvent& ev);
  bool HandleEvent (iEvent& ev);
  void SetupFrame ();
  void WriteShadow (int x,int y,int fg,const char *str,...);
  void Write(int x,int y,int fg,int bg,const char *str,...);
  
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

  csRef<iMeshWrapper> walls;

  // Camera related
  int cameraMode;
  csRef<iRigidBody> cameraBody;
  float rotX, rotY, rotZ;

  csDynamicSystemDebugger dynSysDebugger;
  bool debugMode;
  bool allStatic;
  bool pauseDynamic;
  float dynamicSpeed;

  // ragdoll related
  csRef<iBodyManager> bodyManager;
  csRef<iSkeletonRagdollManager2> ragdollManager;
  CS::Animation::StateID ragdollState;
  csRef<iMeshWrapper> ragdollMesh;

  CS_DECLARE_EVENT_SHORTCUTS;

  csEventID KeyboardDown;
  csEventID KeyboardUp;

public:
  Simple (iObjectRegistry *obj);
  ~Simple ();

  bool Initialize ();
  void Start ();
  void Shutdown ();
};

#endif // __PHYSTUT_H__

