/*
    Copyright (C) 2004 by Jorrit Tyberghein

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

#ifndef __ISOTEST_H__
#define __ISOTEST_H__

#include <stdarg.h>
#include "csutil/ref.h"
#include "csgeom/vector3.h"

struct iEngine;
struct iLoader;
struct iGraphics3D;
struct iKeyboardDriver;
struct iVirtualClock;
struct iObjectRegistry;
struct iEvent;
struct iSector;
struct iView;
struct iMeshWrapper;
struct iLight;

struct IsoView
{
  csVector3 camera_offset;
};

class IsoTest
{
private:
  iObjectRegistry* object_reg;
  csRef<iEngine> engine;
  csRef<iLoader> loader;
  csRef<iGraphics3D> g3d;
  csRef<iKeyboardDriver> kbd;
  csRef<iVirtualClock> vc;
  iSector* room;
  csRef<iView> view;
  csRef<iMeshWrapper> actor;
  iMeshWrapper* plane;
  csRef<iLight> actor_light;

  int current_view;
  IsoView views[4];

  static bool IsoTestEventHandler (iEvent& ev);
  bool HandleEvent (iEvent& ev);
  void SetupFrame ();
  void FinishFrame ();

  bool CreateActor ();
  bool LoadMap ();

public:
  IsoTest (iObjectRegistry* object_reg);
  ~IsoTest ();

  bool Initialize ();
  void Start ();
};

#endif // __ISOTEST_H__

