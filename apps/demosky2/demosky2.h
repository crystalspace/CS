/*
    Copyright (C) 2001 by W.C.A. Wijngaards

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

#ifndef DEMOSKY2_H
#define DEMOSKY2_H

#include <stdarg.h>
#include "cssys/sysdriv.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"

struct iMeshWrapper;
struct iSector;
struct iSector;
struct iView;
struct iEngine;
struct iDynLight;
struct iMaterialWrapper;
struct iPolygon3D;
struct iFont;
struct iMeshWrapper;
struct iMaterialWrapper;
struct iLoader;
struct iKeyboardDriver;

class DemoSky : public SysSystemDriver
{
  typedef SysSystemDriver superclass;
private:
  iSector* room;
  iView* view;
  iEngine* engine;
  iMaterialWrapper* matPlasma;
  iFont *font;
  iLoader *LevelLoader;
  iGraphics2D *myG2D;
  iGraphics3D *myG3D;
  iKeyboardDriver* kbd;

  /// the sky 
  iMeshWrapper *skydome;

public:
  DemoSky ();
  virtual ~DemoSky ();

  virtual bool Initialize (int argc, const char* const argv[],
    const char *iConfigName);
  virtual void NextFrame ();
  virtual bool HandleEvent (iEvent &Event);

  void Report (int severity, const char* msg, ...);
};

#endif // DEMOSKY2_H

