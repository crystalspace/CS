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

#ifndef __SIMPCD_H__
#define __SIMPCD_H__

#include <stdarg.h>

struct iEngine;
struct iLoader;
struct iGraphics3D;
struct iKeyboardDriver;
struct iVirtualClock;
struct iObjectRegistry;
struct iEvent;
struct iSector;
struct iView;
struct iCollider;
struct iCollideSystem;
struct iMeshWrapper;

class Simple
{
private:
  iObjectRegistry* object_reg;
  iEngine* engine;
  iLoader* loader;
  iGraphics3D* g3d;
  iKeyboardDriver* kbd;
  iVirtualClock* vc;
  iSector* room;
  iView* view;
  iCollideSystem* cdsys;

  iMeshWrapper* parent_sprite;
  float rot1_direction;
  iMeshWrapper* sprite1;
  iCollider* sprite1_col;
  float rot2_direction;
  iMeshWrapper* sprite2;
  iCollider* sprite2_col;

  static bool SimpleEventHandler (iEvent& ev);
  bool HandleEvent (iEvent& ev);
  void SetupFrame ();
  void FinishFrame ();
  iCollider* InitCollider (iMeshWrapper* mesh);

public:
  Simple ();
  ~Simple ();

  bool Initialize (int argc, const char* const argv[]);
  void Start ();
};

#endif // __SIMPCD_H__

