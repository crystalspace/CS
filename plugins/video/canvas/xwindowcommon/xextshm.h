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

#ifndef __CS_IVIDEO_XEXTSHM_H__
#define __CS_IVIDEO_XEXTSHM_H__

/**\file
 */
#define XK_MISCELLANY 1
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

struct iGraphics2D;

SCF_VERSION (iXExtSHM, 1, 0, 0);

/// Document me!@@@
struct iXExtSHM : public iBase
{
  virtual void SetDisplayScreen (Display *dpy, int screen_num) = 0;
  /// Create Shared Memory
  virtual unsigned char *CreateMemory (int Width, int Height) = 0;
  /// Destroy Shared Memory
  virtual void DestroyMemory () = 0;
  /// Print Image
  virtual void Print (Window window, GC gc, csRect const* area) = 0;
};

#endif // __CS_IVIDEO_XEXTSHM_H__
