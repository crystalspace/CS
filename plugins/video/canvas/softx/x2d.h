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

#ifndef __X2D_H__
#define __X2D_H__

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
class csGraphics2DXLib : public csGraphics2D
{
  // The display context
  Display* dpy;
  int screen_num;
  int display_width, display_height;
  Window window;
  XImage* xim;
  GC gc;
  Visual *visual;
  XVisualInfo vinfo;
  unsigned int vclass;

  // "WM_DELETE_WINDOW" atom
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

  // Everything for simulated depth
  int sim_depth;
  csPixelFormat real_pfmt;	// Contains the real pfmt is simulating stuff
  unsigned char* real_Memory;	// Real memory to the display
  unsigned char* sim_lt8;	// 8-bit lookup table (with 16-bit index) for simulated depth
  UShort* sim_lt16;		// 16-bit lookup table (with 8-bit index) for simulated depth
  
  /// Pointer to DOS-specific interface
  iUnixSystemDriver* UnixSystem;

  /// This routine is called once per event loop
  static void ProcessEvents (void *Param);

public:
  DECLARE_IBASE;

  csGraphics2DXLib (iBase *iParent);
  virtual ~csGraphics2DXLib ();

  virtual bool Initialize (iSystem *pSystem);
  virtual bool Open (const char *Title);
  virtual void Close ();

  virtual bool BeginDraw () { return (Memory != NULL); }

  virtual void Print (csRect *area = NULL);
  virtual void SetRGB (int i, int r, int g, int b);

  /**
   * Special function that is available only on the X
   * version. This recomputes the colormap and the lookup table
   * for simulation of 15/16-bit on an 8-bit display to get an optimal
   * display. The default colormap simulates a 3:3:2 truecolor
   * display.
   */
  void recompute_simulated_palette ();

  /**
   * This function restores the standard 3:3:2 truecolor palette while
   * simulating 15/16-bit on an 8-bit display.
   */
  void restore_332_palette ();

  /// Use greyscale palette for simulation of 15/16-bit on an 8-bit display.
  void recompute_grey_palette ();

  /// Extensions for X11 port.
  virtual bool PerformExtension (char* args);

  /// Set mouse position.
  virtual bool SetMousePosition (int x, int y);

  /// Set mouse cursor shape
  virtual bool SetMouseCursor (csMouseCursorID iShape);
};

#endif // __X2D_H__
