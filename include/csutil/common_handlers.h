/*
  Crystal Space Common Event Handlers
  Copyright (C) 2006 by Adam D. Bradley <artdodge@cs.bu.edu>

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
#ifndef __CSUTIL_COMMON_HANDLERS_H__
#define __CSUTIL_COMMON_HANDLERS_H__

#include "cssysdef.h"
#include "csutil/scf_implementation.h"
#include "iutil/eventh.h"
#include "csutil/eventnames.h"
#include "ivideo/graph3d.h"
#include "ivaria/view.h"
#include "iengine/engine.h"
#include "csutil/eventhandlers.h"

/**\file
 * A collection of stock event handlers that are widely useful.
 */

/**
 * \addtogroup event_handling
 * @{ */

/**
 * FrameBegin3DDraw handles every csevFrame event in the 3D phase.
 * It calls g3d->BeginDraw(engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS)
 * and view->Draw (), where engine and g3d are loaded from the registry
 * by the constructor and view is loaded from the registry each time the 
 * event is handled.
 * The handler returns false, so it will not prevent any subsequent
 * csevFrame handlers being called.
 */
class FrameBegin3DDraw
: public scfImplementation1<FrameBegin3DDraw, iEventHandler>
{
 protected:
  iObjectRegistry *object_reg;
  csRef<iGraphics3D> g3d;
  csRef<iEngine> engine;
  csRef<iView> view;
 public:
  FrameBegin3DDraw (iObjectRegistry *, csRef<iView> &);
  virtual ~FrameBegin3DDraw ();
  virtual bool HandleEvent (iEvent&);
  CS_EVENTHANDLER_PHASE_3D ("crystalspace.frame.3d_drawer")
};

/**
 * FramePrinter handles every csevFrame event in the FRAME (final) phase.
 * It calls g3d->FinishFraw() and g3d->Print(0).  The handler returns false, 
 * so it will not prevent any subsequent csevFrame handlers being called.
 * The class retrieves a reference to the g3d object from the object registry 
 * when the constructor is called.  The constructor also subscribes the new 
 * instance to the event queue automatically.
 */
class FramePrinter
: public scfImplementation1<FramePrinter, iEventHandler>
{
 protected:
  csRef<iGraphics3D> g3d;
 public:
  FramePrinter (iObjectRegistry *);
  virtual ~FramePrinter ();
  virtual bool HandleEvent (iEvent&);
  CS_EVENTHANDLER_PHASE_FRAME ("crystalspace.frame.printer")
};


#endif
