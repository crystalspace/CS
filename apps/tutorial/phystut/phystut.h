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
  iSector* room;
  int objcnt;
  int solver;
  bool disable;
  float remaining_delta;

  csRef<iDynamics> dyn;
  csRef<iDynamicSystem> dynSys;
  csRef<iMeshFactoryWrapper> boxFact;
  csRef<iMeshFactoryWrapper> meshFact;
  csRef<iMeshFactoryWrapper> ballFact;
  csRef<iFont> courierFont;

  static bool SimpleEventHandler (iEvent& ev);
  bool HandleEvent (iEvent& ev);
  void SetupFrame ();
  void FinishFrame ();
  void WriteShadow (int x,int y,int fg,const char *str,...);
  void Write(int x,int y,int fg,int bg,const char *str,...);
  
  iRigidBody* CreateBox (void);
  iRigidBody* CreateSphere (void);
  iRigidBody* CreateMesh (void);
  iJoint* CreateJointed (void);
  iRigidBody* CreateWalls (const csVector3& radius);
  csRef<iMeshWrapper> walls;
  csRef<iMeshWrapper> avatar;
  csRef<iRigidBody> avatarbody;

public:
  Simple (iObjectRegistry *obj);
  ~Simple ();

  bool Initialize ();
  void Start ();
};

#endif // __PHYSTUT_H__

