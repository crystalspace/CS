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

#ifndef __CS_CANVAS_COMMON_CURSORCONVERT_H__
#define __CS_CANVAS_COMMON_CURSORCONVERT_H__

#include "csextern.h"
#include "csgfx/rgbpixel.h"
#include "csgfx/quantize.h"

struct iImage;
class csImageMemory;

class CS_CSPLUGINCOMMON_EXPORT csCursorConverter
{
  static bool InternalConvertTo1bpp (iImage* image, 
    csColorQuantizer& quantizer, uint8*& bitmap, uint8*& mask,
    const csRGBcolor forecolor, const csRGBcolor backcolor, 
    csRGBpixel keycolor, bool XbitOrder);
public:
  //static bool ConvertTo1bpp (iImage* image, uint8*& bitmap, uint8*& mask,
  //  const csRGBcolor* keycolor = 0);
  /**
   * Convert an image to 1bpp, computing an appropriate bitmap (by dithering
   * to the given foreground and background colors) and mask (from the optionally 
   * given keycolor).
   */
  static bool ConvertTo1bpp (iImage* image, uint8*& bitmap, uint8*& mask,
    const csRGBcolor forecolor, const csRGBcolor backcolor, 
    const csRGBcolor* keycolor = 0, bool XbitOrder = false);
  static bool ConvertTo8bpp (iImage* image, uint8*& pixels, 
    csRGBpixel*& palette, const csRGBcolor* keycolor = 0);
  /**
   * Remove the alpha from an image by replacing the transparent parts
   * with \p replaceColor.
   */
  static void StripAlphaFromRGBA (iImage* image, csRGBpixel replaceColor);
  static void StripAlphaFromPal8 (csImageMemory* image);
};

#endif // __CS_CANVAS_COMMON_CURSORCONVERT_H__
