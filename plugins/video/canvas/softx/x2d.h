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

#ifndef __CS_X2D_H__
#define __CS_X2D_H__

#include "csplugincommon/canvas/graph2d.h"
#include "iutil/event.h"
#include "plugins/video/canvas/xwindowcommon/xwindow.h"
#include "plugins/video/canvas/xwindowcommon/xextshm.h"

#define XK_MISCELLANY 1
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

/// XLIB version.
class csGraphics2DXLib : public csGraphics2D, public iEventPlug
{
  csRef<iXWindow> xwin;
  /// Shared memory extension (manages the shared memory backbuffer)
  csRef<iXExtSHM> xshm;
  /// Used for back buffer when not using shared memory
  XImage* xim;
  /// The event outlet
  csRef<iEventOutlet> EventOutlet;
  // The display context
  Display* dpy;
  int screen_num;
  Window window;
  GC gc;
  XVisualInfo xvis;
  // Window colormap
  Colormap cmap;

  // Everything for simulated depth
  int sim_depth;
  csPixelFormat real_pfmt;	// Contains the real pfmt is simulating stuff
  unsigned char* real_Memory;	// Real memory to the display
  unsigned char* sim_lt8;	// 8-bit lookup table (with 16-bit index) for simulated depth
  uint16* sim_lt16;		// 16-bit lookup table (with 8-bit index) for simulated depth

  bool CreateVisuals ();
  bool AllocateMemory ();
  bool TryAllocateMemory ();

public:
  SCF_DECLARE_IBASE_EXT(csGraphics2D);
  csGraphics2DXLib (iBase*);
  virtual ~csGraphics2DXLib ();

  virtual bool Initialize (iObjectRegistry*);
  virtual bool Open ();
  virtual void Close ();
  virtual bool HandleEvent (iEvent &Event);

  void Report (int severity, const char* msg, ...);

  virtual void Print (csRect const* area = 0);
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
  virtual bool PerformExtensionV (char const* command, va_list);

  virtual void AllowResize (bool iAllow);

  virtual bool Resize (int width, int height);

  virtual void AlertV (int type, const char* title, const char* okMsg,
  	const char* msg, va_list arg) 
  {
    if (!xwin->AlertV (type, title, okMsg, msg, arg))
      csGraphics2D::AlertV (type, title, okMsg, msg, arg);
  }

  virtual void SetTitle (const char* title)
  { xwin->SetTitle (title); }

  virtual bool GetFullScreen ()
  { return xwin->GetFullScreen (); }

  virtual void SetFullScreen (bool yesno);

  /// Set mouse position.
  // should be the window manager
  virtual bool SetMousePosition (int x, int y)
  { return xwin->SetMousePosition (x, y); }

  /// Set mouse cursor shape
  // should be the window manager
  virtual bool SetMouseCursor (csMouseCursorID iShape)
  { return xwin->SetMouseCursor (iShape); }

  /// Set mouse cursor shape
  // should be the window manager
  virtual bool SetMouseCursor (iImage *image, 
    const csRGBcolor* keycolor, int x, int y, csRGBcolor fg, csRGBcolor bg)
  { return xwin->SetMouseCursor (image, keycolor, x, y, fg, bg); }

  //------------------------ iEventPlug interface ---------------------------//

  virtual unsigned GetPotentiallyConflictingEvents ()
  { return 0; }
  virtual unsigned QueryEventPriority (unsigned /*iType*/)
  { return 150; }

};

#endif // __CS_X2D_H__
