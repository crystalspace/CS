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

#ifndef __CS_IVIDEO_XEXTF86VM_H__
#define __CS_IVIDEO_XEXTF86VM_H__

/**\file
 */
#define XK_MISCELLANY 1
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>


SCF_VERSION (iXExtF86VM, 1, 0, 0);

/// Document me!@@@
struct iXExtF86VM : public iBase
{
  /// Open Video Mode plugin
  virtual bool Open (Display *dpy, int screen_num,
		     XVisualInfo *xvis, Colormap cmap) = 0;
  /// Finish
  virtual void Close () = 0;
  /// Set the context window and its parent window while in windowed mode
  virtual void SetWindows (Window ctx_win, Window wm_win) = 0;
  /// Set whether in full screen mode, returns whether status changes
  virtual bool SetFullScreen (bool yesno) = 0;
  /// Query full screen status
  virtual bool IsFullScreen () = 0;
  /// While in full screen jump to next higher resolution mode
  virtual void IncVideoMode () = 0;
  /// While in full screen jump to next lower resolution mode
  virtual void DecVideoMode () = 0;
  /// Get Current Dimensions
  virtual void GetDimensions (int &w, int &h) = 0;
};

#endif // __CS_IVIDEO_XEXTF86VM_H__
