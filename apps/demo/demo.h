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

#ifndef __DEMO_H__
#define __DEMO_H__

#include <stdarg.h>
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csutil/stringarray.h"

struct iEngine;
struct iSector;
struct iView;
struct iFont;
struct iFile;
struct iKeyboardDriver;
struct iImageLoader;
struct iLoaderPlugin;
struct iMeshWrapper;
struct iConsoleOutput;
struct iVirtualClock;
struct iObjectRegistry;
struct iGraphics3D;
struct iGraphics2D;
struct iLoader;
struct iVFS;
struct iEvent;
class DemoSequenceManager;
class csTransform;

class Demo
{
public:
  csRef<iEngine> engine;
  iSector* room;
  csRef<iView> view;
  csRef<iGraphics3D> myG3D;
  csRef<iGraphics2D> myG2D;
  csRef<iVFS> myVFS;
  csRef<iKeyboardDriver> kbd;
  csRef<iConsoleOutput> myConsole;
  iObjectRegistry* object_reg;
  csRef<iVirtualClock> vc;
  csRef<iLoader> loader;

  DemoSequenceManager* seqmgr;
  csRef<iFont> font;
  int col_red, col_blue, col_white, col_black;
  int col_yellow, col_cyan, col_green, col_gray;
  csString message;
  csTicks message_timer;
  bool message_error;

  int do_demo;
  size_t selected_demo;
  csStringArray demos;
  int first_y;	// First y location where list of demo files start.

private:
  void GfxWrite (int x, int y, int fg, int bg, const char *str, ...);
  void FileWrite (iFile* file, char *str, ...);

  void DrawEditInfo ();

  bool LoadDemoFile (const char* demofile);

public:
  Demo ();
  virtual ~Demo ();

  void Report (int severity, const char* msg, ...);

  bool Initialize (int argc, const char* const argv[],
    const char *iConfigName);
  void SetupFrame ();
  void FinishFrame ();
  bool DemoHandleEvent (iEvent &Event);

  void ShowMessage (const char* msg, ...);
  void ShowError (const char* msg, ...);
};

#endif // __DEMO_H__
