/*
    Copyright (C) 2002 by Anders Stenberg

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

#ifndef __R3DTEST_H__
#define __R3DTEST_H__

#include <stdarg.h>
#include "csutil/ref.h"

struct iEngine;
struct iObjectRegistry;
struct iEvent;
struct iVirtualClock;
struct iMaterialWrapper;
struct iKeyboardDriver;
struct iMouseDriver;
struct iVFS;


class csTestMesh;

class R3DTest
{
private:
  iObjectRegistry* object_reg;
  csRef<iEngine> engine;
#ifdef CS_USE_NEW_RENDERER
  csRef<iRender3D> r3d;
#else
  csRef<iGraphics3D> r3d;
#endif
  csRef<iView> view;
  csRef<iKeyboardDriver> kbd;
  csRef<iMouseDriver> mouse;

  csRef<iVFS> vfs;
  csRef<iVirtualClock> vc;

  bool hasfocus;

  iSector* room;

  csTestMesh* testmesh;

  static bool SimpleEventHandler (iEvent& ev);
  bool HandleEvent (iEvent& ev);
  void SetupFrame ();
  void FinishFrame ();

public:
  R3DTest (iObjectRegistry* object_reg);
  ~R3DTest ();

  bool Initialize ();
  void Start ();
};

#endif // __R3DTEST_H__

