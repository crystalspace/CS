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

#ifndef __CSWSENG_H__
#define __CSWSENG_H__

#include <stdarg.h>
#include "csws/csws.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "ivideo/graph3d.h"
#include "imap/parser.h"

struct iSector;
struct iView;
struct iEngine;
struct iVFS;
struct iGraphics3D;
struct iGraphics2D;
struct iVirtualClock;
struct iObjectRegistry;

enum
{
  cecmdLoad = 9000,
  cecmdNewView,
  cecmdQuit
};

class ceEngineView;

/**
 * This subclass of csApp is our main entry point
 * for the CSWS application. It controls everything including
 * the engine view.
 */
class ceCswsEngineApp : public csApp
{
public:
  iSector* start_sector;
  csVector3 start_pos;
  csRef<iEngine> engine;
  csRef<iGraphics3D> pG3D;
  csRef<iConsoleOutput> Console;
  csRef<iVFS> VFS;
  csRef<iLoader> LevelLoader;
  csArray<ceEngineView*> engine_views;

  void SetupDefaultWorld ();
  void LoadNewMap (const char* name);

public:
  ceCswsEngineApp (iObjectRegistry *object_reg, csSkin &skin);
  ~ceCswsEngineApp ();

  virtual bool HandleEvent (iEvent &Event);
  virtual bool Initialize ();
};

/**
 * This is a view on the 3D engine. It is a subclass of csComponent
 * so that it behaves nicely in the CSWS framework.
 */
class ceEngineView : public csComponent
{
  csRef<iView> view;
  // A bit mask saying which kind of motion should be done once per frame
  int motion;

public:
  ceEngineView (csComponent *iParent, iEngine *Engine, iSector *Start,
    const csVector3& start_pos, iGraphics3D *G3D);
  virtual ~ceEngineView ();

  // Track movement of the window and update engine.
  virtual bool SetRect (int xmin, int ymin, int xmax, int ymax);
  // Redraw the engine view.
  virtual void Draw ();
  // Do motion etc.
  virtual bool HandleEvent (iEvent &Event);
  // Disable motion when we lose focus.
  virtual void SetState (int mask, bool enable);
  // Get the view.
  iView* GetView () { return view; }
};

/**
 * This is a small window with three buttons.
 */
class ceControlWindow : public csWindow
{
public:
  ceControlWindow (csComponent *iParent, char *iTitle,
  	int iWindowStyle = CSWS_DEFAULTVALUE,
	csWindowFrameStyle iFrameStyle = cswfs3D) :
	csWindow (iParent, iTitle, iWindowStyle, iFrameStyle) { }
  virtual bool HandleEvent (iEvent& Event);
};

#endif // __CSWSENG_H__

