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

#include "igraphic/image.h"

#include "customcursor.h"
#include "plugins/video/canvas/common/cursorconvert.h"
#include <windows.h>

csWin32CustomCursors::~csWin32CustomCursors ()
{
  csHash<HCURSOR, csStrKey, csConstCharHashKeyHandler>::GlobalIterator it =
    cachedCursors.GetIterator ();

  while (it.HasNext ())
  {
    HCURSOR cur = it.Next ();
    DestroyCursor (cur);
  }
}

HCURSOR csWin32CustomCursors::GetMouseCursor (iImage* image, 
					      const csRGBcolor* keycolor, 
					      int hotspot_x, int hotspot_y, 
					      csRGBcolor fg, csRGBcolor bg)
{
  HCURSOR cursor;
  const char* cacheName = image->GetName ();
  if (cacheName != 0)
  {
    cursor = cachedCursors.Get (cacheName, 0);
    if (cursor != 0)
      return cursor;
  }

  uint8* ANDmask;
  uint8* XORmask;
  if (!csCursorConverter::ConvertTo1bpp (image, XORmask, ANDmask, csRGBcolor (255, 255, 255),
    csRGBcolor (0, 0, 0), keycolor)) // @@@ Force color to black & white for now
    return false;

  // Need to invert AND mask
  {
    uint8* ANDptr = ANDmask;
    int byteNum = ((image->GetWidth () + 7) / 8) * image->GetHeight ();
    while (byteNum-- > 0)
    {
      *ANDptr++ ^= 0xff;
    }
  }

  cursor = CreateCursor (0, hotspot_x, hotspot_y, image->GetWidth (), 
    image->GetHeight (), ANDmask, XORmask);
  delete[] ANDmask;
  delete[] XORmask;
  if (cacheName != 0)
  {
    cachedCursors.Put (cacheName, cursor);
  }
  return cursor;
}
