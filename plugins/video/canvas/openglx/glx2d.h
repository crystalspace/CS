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
#include "video/canvas/openglcommon/glcommon2d.h"
#include "ivideo/xwindow.h"

#include <GL/glx.h>

#include "iogldisp.h"

#define XK_MISCELLANY 1
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

/// XLIB version.
class csGraphics2DGLX : public csGraphics2DGLCommon
{
  iXWindow  *xwin;
  // The display context
  Display* dpy;
  int screen_num;
  Window window;
  XVisualInfo *xvis;
  Colormap cmap;
  GLXContext active_GLContext;

  // we are using a specific displaydriver
  iOpenGLDisp *dispdriver;

  /**
   * Helper function, attempts to create a visual/context with the 
   * desired attributes
   */
  bool CreateVisuals ();

public:
  csGraphics2DGLX (iBase *iParent);
  virtual ~csGraphics2DGLX ();

  virtual bool Initialize (iObjectRegistry *object_reg);
  virtual bool Open ();
  virtual void Close ();

  void Report (int severity, const char* msg, ...);

  virtual void Print (csRect *area = NULL);

  virtual bool PerformExtensionV (char const* command, va_list);

  virtual void AllowResize (bool iAllow);

  virtual bool Resize (int width, int height);

  virtual void SetTitle (const char* title)
  { xwin->SetTitle (title); }

  virtual void SetFullScreen (bool yesno);

  virtual bool GetFullScreen ()
  { return xwin->GetFullScreen (); }
  /// Set mouse position.
  // should be the window manager
  virtual bool SetMousePosition (int x, int y)
  { return xwin->SetMousePosition (x, y); }

  /// Set mouse cursor shape
  // should be the window manager
  virtual bool SetMouseCursor (csMouseCursorID iShape)
  { return xwin->SetMouseCursor (iShape);}

};

#endif // __GLX2D_H__
