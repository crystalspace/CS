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
#include "cssys/sysdriv.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"

struct iEngine;
struct iSector;
struct iView;
struct iFont;
struct iFile;
struct iImageLoader;
struct iLoaderPlugIn;
class DemoSequenceManager;

class Demo : public SysSystemDriver
{
  typedef SysSystemDriver superclass;

public:
  iEngine* engine;
  iSector* room;
  iView* view;
  DemoSequenceManager* seqmgr;
  iFont* font;
  int col_red, col_blue, col_white, col_black;
  int col_yellow, col_cyan, col_green, col_gray;
  char message[255];
  cs_time message_timer;
  bool message_error;

private:
  void LoadMaterial (const char* matname, const char* filename);
  void LoadFactory (const char* factname, const char* filename,
  	const char* classId, iLoaderPlugIn* plug);
  void GfxWrite (int x, int y, int fg, int bg, char *str, ...);
  void FileWrite (iFile* file, char *str, ...);

  void DrawEditInfo ();

public:
  Demo ();
  virtual ~Demo ();

  virtual bool Initialize (int argc, const char* const argv[],
    const char *iConfigName);
  virtual void NextFrame ();
  virtual bool HandleEvent (iEvent &Event);

  void ShowMessage (const char* msg, ...);
  void ShowError (const char* msg, ...);

  void SetupFactories ();
  void SetupMaterials ();
  void SetupSector ();
  void SetupObjects ();
};

#endif // DEMO_H
