/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein
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

#ifndef ISOTEST_H
#define ISOTEST_H

#include <stdarg.h>
#include "cssys/sysdriv.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"

struct iIsoEngine;
struct iIsoView;
struct iIsoWorld;
struct iIsoSprite;
struct iIsoLight;
struct iLoader;
struct iFont;
struct iMaterialWrapper;

class IsoTest : public SysSystemDriver
{
  typedef SysSystemDriver superclass;
private:
  /// the iso engine
  iIsoEngine *engine;
  /// world to display, the 'level'
  iIsoWorld *world;
  /// view on the world, the 'camera'
  iIsoView *view;
  /// G2D and G3D plugin
  iGraphics2D *myG2D;
  iGraphics3D *myG3D;

  /// the font for text display
  iFont *font;
  /// the player sprite
  iIsoSprite *player;
  /// the light
  iIsoLight *light;

  /// to keep track of mouseclicks and actions. last click pos in world space.
  csVector3 lastclick;
  /// we are moving to lastclick position
  bool walking;

public:
  IsoTest ();
  virtual ~IsoTest ();

  virtual bool Initialize (int argc, const char* const argv[],
    const char *iConfigName);
  virtual void NextFrame ();
  virtual bool HandleEvent (iEvent &Event);
  iIsoLight *GetLight () const {return light;}
  void AddMazeGrid(iIsoWorld *world, float posx, float posy, 
    iMaterialWrapper *floor, iMaterialWrapper *wall);

  void Report (int severity, const char* msg, ...);
};

#endif // ISOTEST_H

