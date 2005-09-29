/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Copyright (C) 2001 by Samuel Humphreys

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

#ifndef __CS_IVIDEO_XWINDOW_H__
#define __CS_IVIDEO_XWINDOW_H__

/**\file
 */
#define XK_MISCELLANY 1
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include "ivideo/cursor.h"
#include "csgfx/rgbpixel.h"

struct iGraphics2D;
struct iImage;

SCF_VERSION (iXWindow, 1, 0, 0);

/// Document me! @@@
struct iXWindow : public virtual iBase
{
  // These should be inherited from csNativeWindow
  virtual bool Open () = 0;
  virtual void Close () = 0;

  virtual void AllowResize (bool iAllow) = 0;

  virtual bool GetFullScreen () = 0;
  virtual void SetFullScreen (bool yesno) = 0;

  virtual void SetTitle (const char* title) = 0;
  virtual void SetCanvas (iGraphics2D *canvas) = 0;

  // These are X specific
  virtual Display *GetDisplay () = 0;
  virtual int GetScreen () = 0;
  virtual Window GetWindow () = 0;
  virtual GC GetGC () = 0;

  virtual void SetVisualInfo (XVisualInfo *xvis) = 0;
  virtual void SetColormap (Colormap cmap) = 0;

  // Should be in the window manager...
  virtual bool SetMousePosition (int x, int y) = 0;
  virtual bool SetMouseCursor (csMouseCursorID iShape) = 0;
  virtual bool SetMouseCursor (iImage *image, const csRGBcolor* keycolor,
                               int hotspot_x, int hotspot_y,
                               csRGBcolor fg, csRGBcolor bg) = 0;
  /**
   * Show a message box.
   * \return Whether the message was actually shown. If 'false', the
   * caller should display the message in an alternate way.
   */
  virtual bool AlertV (int type, const char* title, const char* okMsg,
        const char* msg, va_list arg) CS_GNUC_PRINTF (5, 0) = 0;
};

#endif // __CS_IVIDEO_XWINDOW_H__
