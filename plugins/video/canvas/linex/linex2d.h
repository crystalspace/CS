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
#include "video/canvas/common/graph2d.h"

#define XK_MISCELLANY 1
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>

#ifdef XFREE86VM
#  include <X11/extensions/xf86vmode.h>
#endif

/// XLIB version.
class csGraphics2DLineXLib : public csGraphics2D
{
  // The display context
  Display* dpy;
  int screen_num;
  int display_width, display_height;
  Window wm_window;
  int wm_width;
  int wm_height;
  Window window;
  Window leader_window;
  Window root_window;
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

  bool currently_full_screen;

#ifdef XFREE86VM
  XF86VidModeModeInfo orig_mode;
  XF86VidModeModeInfo fs_mode;
  Window fs_window;
  int fs_width;
  int fs_height;
  int orig_x;
  int orig_y;
#endif

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

  virtual bool PerformExtension (const char* args);

  /// Set mouse position.
  virtual bool SetMousePosition (int x, int y);

  /// Set mouse cursor shape
  virtual bool SetMouseCursor (csMouseCursorID iShape);

  /// Called on every frame by system driver
  virtual bool HandleEvent (csEvent &Event);

  /// helper function which allocates buffers
  bool AllocateMemory ();
  /// helper function which deallocates buffers
  void DeAllocateMemory ();

  void EnterFullScreen ();
  void LeaveFullScreen ();

#ifdef XFREE86VM
  void InitVidModes ();
#endif
};

#endif // __LINEX2D_H__
