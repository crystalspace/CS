/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#ifndef PHYZTEST_H
#define PHYZTEST_H

#include <stdarg.h>
#include "cssys/sysdriv.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "cstool/collider.h"

struct iSector;
struct iView;
struct iEngine;
struct iDynLight;
struct iCollideSystem;
struct iFont;
struct iLoader;
struct iKeyboardDriver;
class csRigidSpaceTimeObj;

enum TextAlignmentModes {ALIGN_LEFT,ALIGN_RIGHT,ALIGN_CENTER};

class Phyztest : public SysSystemDriver
{
  typedef SysSystemDriver superclass;
public:
  iFont* courierFont;
  int write_colour;
  iSector* room;
  iView* view;
  iEngine* engine;
  iDynLight* dynlight;
  float angle;
  int motion_flags;
  iCollideSystem* cdsys;
  iLoader* LevelLoader;
  iGraphics2D* myG2D;
  iGraphics3D* myG3D;
  iKeyboardDriver* kbd;
  csColliderWrapper *room_collwrap;
  csRigidSpaceTimeObj *bot_sto;

  void WriteShadow (int align, int x, int y, int fg, char *str,...);
  void Write (int align, int x, int y, int fg, int bg, char *str,...);
public:
  Phyztest ();
  virtual ~Phyztest ();

  virtual bool Initialize (int argc, const char* const argv[],
    const char *iConfigName);
  virtual void NextFrame ();
  virtual bool HandleEvent (iEvent &Event);

  void Report (int severity, const char* msg, ...);
};

#endif // PHYZTEST_H
