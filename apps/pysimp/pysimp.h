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

#ifndef __CS_PYSIMP_H__
#define __CS_PYSIMP_H__

#include <stdarg.h>
#include "cssys/sysdriv.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"

class csView;
struct iSector;
struct iEngine;
struct iLoader;

class PySimple : public SysSystemDriver
{
  typedef SysSystemDriver superclass;
public:
  csView* view;
  iEngine* engine;
  int motion_flags;
  iLoader *LevelLoader;
  iGraphics3D *myG3D;

public:
  PySimple ();
  virtual ~PySimple ();

  virtual void Help ();
  virtual bool Initialize (int argc, const char* const argv[],
    const char *iConfigName);
  virtual void NextFrame ();
  virtual bool HandleEvent (iEvent &Event);

  void Report (int severity, const char* msg, ...);
};

#endif // __CS_PYSIMP_H__

