/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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

/**\file
 * Functions to bake keycolor into the alpha channel of images.
 */
 
#ifndef __CS_CSGFX_BAKEKEYCOLOR_H__
#define __CS_CSGFX_BAKEKEYCOLOR_H__

#include "csextern.h"
#include "igraphic/image.h"

/**\addtogroup gfx
 * @{ 
 */

/**
 * Functions to bake keycolor into the alpha channel of images.
 */
class CS_CRYSTALSPACE_EXPORT csBakeKeyColor
{
public:
  // @{
  /**
   * Set alpha of all pixels to 0 that match \a transpColor.
   * The color of pixels made transparent is changed such that it has the
   * average color of some near non-transparent pixels to prevent halos.
   * \remarks Intermediately uses a truecolor images, i.e. paletted images
   *  are not handled with maximum efficiency.
   */
  static csRef<iImage> Image (iImage* source, const csRGBpixel& transpColor);
  static void RGBA2D (uint8* dest, const uint8* source, int w, int h, 
    const csRGBpixel& transpColor);
  // @}
};

/** @} */

#endif // __CS_CSGFX_BAKEKEYCOLOR_H__
