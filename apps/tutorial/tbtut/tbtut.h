/*
    Copyright (C) 2001 by Jorrit Tyberghein, Daniel Duhprey

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

#ifndef __TBTUT_H__
#define __TBTUT_H__

#include <csutil/ref.h>

struct iObjectRegistry;
struct iPluginManager;
struct iVFS;
struct iVirtualClock;
struct iEngine;
struct iLoader;
struct iGraphics3D;
struct iEvent;
struct iConsoleOutput;
struct iFontServer;
struct iDynamics;

struct iView;
struct iFont;
struct iDynamicSystem;

class TerrBigTut {
public:
  TerrBigTut (iObjectRegistry *object_reg);
  ~TerrBigTut ();

  void Report (int s, const char *m);

  bool Initialize ();
  void MainLoop ();

private:
  static bool SimpleEventHandler (iEvent &e);
  bool HandleEvent (iEvent &e);
  void SetupFrame ();
  void FinishFrame ();

  iObjectRegistry* object_reg;
  csRef<iVFS> vfs;
  csRef<iEngine> engine;
  csRef<iLoader> loader;
  csRef<iGraphics3D> g3d;
  csRef<iKeyboardDriver> kbd;
  csRef<iVirtualClock> vc;
  csRef<iView> view;
  iSector* room;
};

#endif // __TBTUT_H__
