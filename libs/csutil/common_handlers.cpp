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

#include "csutil/common_handlers.h"
#include "iutil/eventq.h"

FrameBegin3DDraw::FrameBegin3DDraw (iObjectRegistry *obj_reg, 
	csRef<iView> &the_view) :
  scfImplementationType (this),
  object_reg (obj_reg),
  g3d (csQueryRegistry<iGraphics3D> (obj_reg)),
  engine (csQueryRegistry<iEngine> (obj_reg)),
  view (the_view)
{
  csRef<iEventQueue> queue = csQueryRegistry<iEventQueue> (object_reg);
  queue->RegisterListener(this, csevFrame (object_reg));
}

FrameBegin3DDraw::~FrameBegin3DDraw ()
{
  engine.Invalidate();
}

bool FrameBegin3DDraw::HandleEvent (iEvent & /*ev*/)
{
  if (!g3d->BeginDraw (CSDRAW_3DGRAPHICS))
    return false;
  view->Draw();
  return false;
}


FramePrinter::FramePrinter(iObjectRegistry *object_reg) : 
  scfImplementationType (this),
  g3d(csQueryRegistry<iGraphics3D> (object_reg))
{
  csRef<iEventQueue> queue = csQueryRegistry<iEventQueue> (object_reg);
  queue->RegisterListener(this, csevFrame (object_reg));
}

FramePrinter::~FramePrinter()
{
  g3d.Invalidate();
}

bool FramePrinter::HandleEvent (iEvent & /*ev*/) {
  g3d->FinishDraw ();
  g3d->Print (0);
  return false;
}




