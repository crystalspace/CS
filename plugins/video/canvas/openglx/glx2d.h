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

#ifndef __GLX2D_H__
#define __GLX2D_H__

#include "csutil/scf.h"
#include "cssys/unix/iunix.h"
#include "cs2d/openglcommon/glcommon2d.h"

#include <GL/glx.h>

#define XK_MISCELLANY 1
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/cursorfont.h>

#ifdef DO_SHM
#  include <X11/extensions/XShm.h>
#  include <sys/ipc.h>
#  include <sys/shm.h>
#endif /* DO_SHM */

/// XLIB version.
class csGraphics2DGLX : public csGraphics2DGLCommon
{
  // The display context
  Display* dpy;
  int screen_num;
  int display_width, display_height;
  Window window;
  XImage* xim;
  GC gc;
  XVisualInfo *active_GLVisual;
  GLXContext active_GLContext;

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

  /// Pointer to DOS-specific interface
  IUnixSystemDriver* UnixSystem;

public:
  csGraphics2DGLX (iBase *iParent);
  virtual ~csGraphics2DGLX ();

  virtual bool Initialize (iSystem *pSystem);
  virtual bool Open (const char *Title);
  virtual void Close ();

  virtual bool BeginDraw () { return (Memory != NULL); }

  virtual void Print (csRect *area = NULL);

  // All graphics functions are inherited from csGraphics2DGLCommon

  /// Set mouse position.
  virtual bool SetMousePosition (int x, int y);

  /// Set mouse cursor shape
  virtual bool SetMouseCursor (csMouseCursorID iShape, iTextureHandle *iBitmap);

protected:
  /// This routine is called once per event loop
  static void ProcessEvents (void *Param);
};

#endif // __XLIB2D_H__
