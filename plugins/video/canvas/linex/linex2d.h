/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef __LINEX2D_H__
#define __LINEX2D_H__

#include "csutil/scf.h"
#include "cs2d/common/graph2d.h"
#include "cssys/unix/iunix.h"

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
class csGraphics2DLineXLib : public csGraphics2D
{
  // The display context
  Display* dpy;
  int screen_num;
  int display_width, display_height;
  Window window;
  Pixmap back;
  GC gc, gc_back;
  Visual *visual;
  XVisualInfo vinfo;
  unsigned int vclass;

  int seg_color;
  XSegment segments[100];
  int nr_segments;

  // "WM_DELETE_WINDOW" atom
  Atom wm_delete_window;

  // Window colormap
  Colormap cmap;

  // Hardware mouse cursor or software emulation?
  bool do_hwmouse;
  /// Mouse cursors (if hardware mouse cursors are used)  
  Cursor MouseCursor [int(csmcWait) + 1];
  /// Empty mouse cursor (consist of EmptyPixmap)
  Cursor EmptyMouseCursor;
  /// A empty pixmap
  Pixmap EmptyPixmap;

  /// Pointer to DOS-specific interface
  iUnixSystemDriver* UnixSystem;

  /// This routine is called once per event loop
  static void ProcessEvents (void *Param);

public:
  DECLARE_IBASE;

  csGraphics2DLineXLib (iBase *iParent);
  virtual ~csGraphics2DLineXLib ();

  virtual bool Initialize (iSystem *pSystem);
  virtual bool Open (const char *Title);
  virtual void Close ();

  virtual bool BeginDraw ();

  virtual void Print (csRect *area = NULL);
  virtual void SetRGB (int i, int r, int g, int b);

  virtual void DrawLine (float x1, float y1, float x2, float y2, int color);
  virtual void Clear (int color);
  virtual void Write (int x, int y, int fg, int bg, const char *text);

  /// Set mouse position.
  virtual bool SetMousePosition (int x, int y);

  /// Set mouse cursor shape
  virtual bool SetMouseCursor (csMouseCursorID iShape);
};

#endif // __LINEX2D_H__
