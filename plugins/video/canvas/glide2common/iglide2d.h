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

#ifndef __IGLIDE2D_H__
#define __IGLIDE2D_H__

#include <glide.h>
#include "csutil/scf.h"

#ifdef GLIDE3
typedef struct {
  float sow;
  float tow;
  float oow;
} MyGrTmuVertex;

typedef struct {
  float x,y;
  float ooz;
  float oow;
  float r,g,b;
  MyGrTmuVertex tmuvtx[3];
} MyGrVertex;
#else
typedef GrVertex MyGrVertex;
#endif

#if defined (OS_LINUX)
#include <X11/Xlib.h>
#endif

SCF_VERSION (iGraphics2DGlide, 0, 0, 1);

/**
 * iGraphics2DGlide interface -- for Glide-specific properties.
 * This interface should be implemented by each 2D Glide driver
 * in order for 3D driver to be able to query for specific
 * properties of the 2D driver.
 */
struct iGraphics2DGlide : public iBase
{
#if defined(OS_WIN32)
  /// Query the handle of window
  virtual HWND GethWnd () = 0;
#endif
#if defined (OS_LINUX)
  /// Query the display handle
  virtual Display *GetDisplay () = 0;
#endif
  /// Do we want to wait for a vertical retrace before we swap front and backbuffer ?
  virtual void SetVRetrace ( bool wait4vretrace )=0;
  virtual void ForceResolution ( int width, int height )=0;
  virtual float GetZBuffValue (int x, int y)=0;
};

#endif // __IGLIDE2D_H__
