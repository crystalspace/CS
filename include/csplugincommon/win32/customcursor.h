/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

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

#ifndef __CS_CANVAS_WIN32CANVASCOMMON_CUSTOMCURSOR_H__
#define __CS_CANVAS_WIN32CANVASCOMMON_CUSTOMCURSOR_H__

/**\file
 * Create native Win32 mouse cursors.
 */

#include "csextern_win.h"
#include "csutil/hash.h"
#include "csgfx/rgbpixel.h"

/**\addtogroup plugincommon
 * @{ */

/**
 * Class to create native Win32 mouse cursors from images.
 */
class CS_CSPLUGINCOMMON_WIN_EXPORT csWin32CustomCursors
{
  struct CachedCursor
  {
    HCURSOR cursor;
    bool destroyAsIcon;

    CachedCursor ()
    {
      cursor = 0;
      destroyAsIcon = false;
    }
    CachedCursor (HCURSOR h, bool b) : cursor (h), destroyAsIcon (b) {}
  };
  csHash<CachedCursor, csStrKey> cachedCursors;
  csArray<CachedCursor> blindCursors;

  CachedCursor CreateMonoCursor (iImage* image, const csRGBcolor* keycolor, 
    int hotspot_x, int hotspot_y);
public:
  ~csWin32CustomCursors ();

  HCURSOR GetMouseCursor (iImage* image, const csRGBcolor* keycolor, 
    int hotspot_x, int hotspot_y, csRGBcolor fg, csRGBcolor bg);
private:

  CachedCursor CreateCursor (iImage* image, const csRGBcolor* keycolor,
    int hotspot_x, int hotspot_y);
};

/** @} */

#endif // __CS_CANVAS_WIN32CANVASCOMMON_CUSTOMCURSOR_H__

