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

#ifndef DEMO_H
#define DEMO_H

#include <stdarg.h>
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"

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
  iEngine* engine;
  iSector* room;
  iView* view;
  iGraphics3D *myG3D;
  iGraphics2D *myG2D;
  iVFS *myVFS;
  iKeyboardDriver* kbd;
  iConsoleOutput* myConsole;
  iObjectRegistry* object_reg;
  iVirtualClock* vc;
  iLoader* loader;

  DemoSequenceManager* seqmgr;
  iFont* font;
  int col_red, col_blue, col_white, col_black;
  int col_yellow, col_cyan, col_green, col_gray;
  char message[255];
  csTicks message_timer;
  bool message_error;
  bool do_demo;

private:
  iMeshWrapper* LoadObject (const char* objname, const char* filename,
	const char* classId, const char* loaderClassId,
	iSector* sector, const csVector3& pos);
  void GfxWrite (int x, int y, int fg, int bg, char *str, ...);
  void FileWrite (iFile* file, char *str, ...);

  void DrawEditInfo ();

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

#endif // DEMO_H
