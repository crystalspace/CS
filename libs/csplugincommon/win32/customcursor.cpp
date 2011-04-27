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

#include "cssysdef.h"

#include "csgfx/imagememory.h"
#include "igraphic/image.h"

#include "csplugincommon/win32/customcursor.h"
#include "csplugincommon/win32/icontools.h"
#include "csplugincommon/canvas/cursorconvert.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

csWin32CustomCursors::CachedCursor::~CachedCursor()
{
  if (!cursor) return;
  if (destroyAsIcon)
    DestroyIcon (cursor);
  else
    DestroyCursor (cursor);
}

csWin32CustomCursors::~csWin32CustomCursors ()
{
}

csPtr<csWin32CustomCursors::CachedCursor> csWin32CustomCursors::CreateMonoCursor (
  iImage* image, const csRGBcolor* keycolor, int hotspot_x, int hotspot_y)
{
  HCURSOR cursor;

  uint8* ANDmask;
  uint8* XORmask;
  if (!csCursorConverter::ConvertTo1bpp (image, XORmask, ANDmask, 
    csRGBcolor (255, 255, 255), csRGBcolor (0, 0, 0), keycolor)) 
    // @@@ Force color to black & white for now
    return csPtr<CachedCursor> (nullptr);

  // Need to invert AND mask
  {
    uint8* ANDptr = ANDmask;
    int byteNum = ((image->GetWidth() + 7) / 8) * image->GetHeight();
    while (byteNum-- > 0)
    {
      *ANDptr++ ^= 0xff;
    }
  }

  cursor = ::CreateCursor (0, hotspot_x, hotspot_y, image->GetWidth(), 
    image->GetHeight(), ANDmask, XORmask);
  delete[] ANDmask;
  delete[] XORmask;

  return csPtr<CachedCursor> (new CachedCursor (cursor, false));
}

HCURSOR csWin32CustomCursors::GetMouseCursor (iImage* image, 
					      const csRGBcolor* keycolor, 
					      int hotspot_x, int hotspot_y, 
					      csRGBcolor /*fg*/, 
					      csRGBcolor /*bg*/)
{
  csRef<CachedCursor> cursor;
  cursor = cachedCursors.Get (image, csRef<CachedCursor> ());
  if (cursor)
    return cursor->cursor;

  cursor = CreateCursor (image, keycolor, hotspot_x, hotspot_y);
  //cursor = CreateMonoCursor (image, keycolor, hotspot_x, hotspot_y);

  if (cursor != 0)
  {
    cachedCursors.Put (image, cursor);
  }
  return cursor->cursor;
}

csPtr<csWin32CustomCursors::CachedCursor> csWin32CustomCursors::CreateCursor (
  iImage* image, const csRGBcolor* keycolor,  int hotspot_x, int hotspot_y)
{
  ICONINFO iconInfo;
  iconInfo.fIcon          = false;
  iconInfo.xHotspot	  = hotspot_x;
  iconInfo.yHotspot	  = hotspot_y;

  HCURSOR hCursor =
    CS::Platform::Win32::IconTools::IconFromImage (image, &iconInfo);

  if (hCursor == 0)
    return csPtr<CachedCursor> (nullptr);

  return csPtr<CachedCursor> (new CachedCursor (hCursor, true));
}
