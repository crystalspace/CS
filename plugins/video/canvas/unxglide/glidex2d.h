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

#ifndef __GLIDEX2D_H__
#define __GLIDEX2D_H__

#include "csutil/scf.h"
#include "video/canvas/common/graph2d.h"
#include "video/canvas/glide2common/iglide2d.h"
#include "video/canvas/glide2common/glide2common2d.h"
#include "isys/event.h"

#include <glide.h>

#define XK_MISCELLANY 1
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>

#ifdef DO_SHM
#  include <X11/extensions/XShm.h>
#  include <sys/ipc.h>
#  include <sys/shm.h>
#endif /* DO_SHM */

class csGraphics2DGlideX : public csGraphics2DGlideCommon, public iEventPlug
{
private:
  // The display context
  Display* dpy;
  int screen_num;
  int display_width, display_height;
  Window window;
  XImage* xim;
  GC gc;
  XVisualInfo *active_GLVisual;
  Atom wm_delete_window;
  
  // Window colormap
  Colormap cmap;

  // Use SHM or not?
  bool do_shm;
#ifdef DO_SHM
  XShmSegmentInfo shmi;
  XImage shm_image;
#endif

  // Hardware mouse cursor or software emulation?
  bool do_hwmouse;
  /// Mouse cursors (if hardware mouse cursors are used)  
  Cursor MouseCursor [int(csmcWait) + 1];
  /// Empty mouse cursor (consist of EmptyPixmap)
  Cursor EmptyMouseCursor;
  /// A empty pixmap
  Pixmap EmptyPixmap;
  // The event outlet
  iEventOutlet *EventOutlet;

public:
  SCF_DECLARE_IBASE_EXT(csGraphics2DGlideCommon);

  csGraphics2DGlideX (iBase *iParent);
  virtual ~csGraphics2DGlideX ();

  virtual bool Initialize (iSystem *pSystem);
  virtual bool Open (const char *Title);
  virtual void Close ();
  
  virtual void Print (csRect *area = NULL);

  /// Set mouse cursor shape
  virtual bool SetMouseCursor (csMouseCursorID iShape);
  virtual bool SetMousePosition (int x, int y);

  Display *GetDisplay ()
  { return dpy; }

  /// Called on every frame by system driver
  virtual bool HandleEvent (iEvent &Event);

  //------------------------ iEventPlug interface ---------------------------//

  virtual unsigned GetPotentiallyConflictingEvents ()
  { return CSEVTYPE_Keyboard | CSEVTYPE_Mouse; }
  virtual unsigned QueryEventPriority (unsigned /*iType*/)
  { return 150; }

protected:
  /// This method is used for GlideInWindow...
  void FXgetImage();
};

#endif // __GLIDEX2D_H__
