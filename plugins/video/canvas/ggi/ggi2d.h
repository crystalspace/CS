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

#ifndef __CS_GGI2D_H__
#define __CS_GGI2D_H__

#include "csutil/scf.h"
#include "video/canvas/common/graph2d.h"
#include "isys/event.h"

#include <ggi/ggi.h>

/// GGI version.
class csGraphics2DGGI : public csGraphics2D, public iEventPlug
{
private:
  ggi_visual_t vis;
  ggi_mode vis_mode;
  int display_width, display_height;

  // The event outlet
  iEventOutlet *EventOutlet;

  virtual int translate_key(ggi_event *ev);

public:
  SCF_DECLARE_IBASE_EXT(csGraphics2D);

  csGraphics2DGGI (iBase *iParent);
  virtual ~csGraphics2DGGI ();

  virtual bool Initialize (iSystem *pSystem);
  virtual bool Open (const char *Title);
  virtual void Close ();

  virtual bool BeginDraw () { return (Memory != NULL); }

  virtual void Print (csRect *area = NULL);
  virtual void SetRGB (int i, int r, int g, int b);

  /// Called on every frame by system driver
  virtual bool HandleEvent (iEvent &Event);

  //------------------------ iEventPlug interface ---------------------------//

  virtual unsigned GetPotentiallyConflictingEvents ()
  { return CSEVTYPE_Keyboard | CSEVTYPE_Mouse; }
  virtual unsigned QueryEventPriority (unsigned /*iType*/)
  { return 150; }
};

#endif // __CS_GGI2D_H__
