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

#include "csutil/scf.h"

/**
 * iGraphics2DGlide interface -- for Glide-specific properties.
 * This interface should be implemented by each 2D Glide driver
 * in order for 3D driver to be able to query for specific
 * properties of the 2D driver.
 */
SCF_INTERFACE (iGraphics2DGlide, 0, 0, 1) : public iBase
{
#if defined(OS_WIN32)
  /// Query the handle of window
  virtual HWND GethWnd () = 0;
#endif
#if defined (OS_LINUX)
  /// Query the display handle
  virtual Display *GetDisplay () = 0;
#endif
};

#endif // __IGLIDE2D_H__
