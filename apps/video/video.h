/*
    Copyright (C) 2001 by Norman Krämer

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

#ifndef VIDEO_H
#define VIDEO_H

#include <stdarg.h>
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "ivideo/codec.h"
#include "imap/parser.h"
#include "isys/plugin.h"

struct iSector;
struct iView;
struct iEngine;
struct iKeyboardDriver;
struct iObjectRegistry;
struct iPluginManager;
struct iVirtualClock;
struct iGraphics3D;
struct iEvent;

class Video
{
public:
  iObjectRegistry* object_reg;

private:
  iPluginManager* plugin_mgr;
  iSector* room;
  iView* view;
  iEngine* engine;
  iStreamFormat *pVideoFormat;
  iVideoStream *pVStream;
  iLoader *LevelLoader;
  iGraphics3D *myG3D;
  iKeyboardDriver* kbd;
  iVirtualClock* vc;

  bool InitProcDemo ();

public:
  Video ();
  virtual ~Video ();

  bool Initialize (int argc, const char* const argv[],
    const char *iConfigName);
  void SetupFrame ();
  void FinishFrame ();
  bool HandleEvent (iEvent &Event);

  void Report (int severity, const char* msg, ...);
};

#endif // VIDEO_H

