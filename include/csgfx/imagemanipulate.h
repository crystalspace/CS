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
 * Functions to manipulate iImage objects.
 */
 
/**\addtogroup gfx
 * @{ 
 */

#ifndef __CS_CSGFX_IMAGEMANIPULATE_H__
#define __CS_CSGFX_IMAGEMANIPULATE_H__

#include "csextern.h"
#include "igraphic/image.h"

/**
 * Helper class to manipulate iImage objects.
 * The methods in this class generally return new images.
 */
class CS_CSGFX_EXPORT csImageManipulate
{
  static csRef<iImage> Mipmap2D (iImage* source, int step, 
    csRGBpixel* transp = 0);
  static csRef<iImage> Mipmap3D (iImage* source, int step, 
    csRGBpixel* transp = 0);
  static csRef<iImage> Rescale2D (iImage* source, int NewWidth, 
    int NewHeight);
public:
  /// Rescale an image to the given size.
  static csRef<iImage> Rescale (iImage* source, int NewWidth, 
    int NewHeight, int NewDepth = 1);
  /**
   * Create a new iImage which is a mipmapped version of this one.
   * 'step' indicates how much the mipmap should be scaled down. Step 0 
   * returns the original image.
   * Step 1 scales the image down to 1/2. Steps &gt; 1 repeat this 
   * <i>'step'</i> times.
   * The new image will have same format as the original one. If you pass
   * a pointer to a transparent color, the texels of that color are handled
   * differently.
   */
  static csRef<iImage> Mipmap (iImage* source, int step, 
    csRGBpixel* transp = 0);
  /**
   * Return a blurred version of the image.
   */
  static csRef<iImage> Blur (iImage* source, csRGBpixel* transp = 0);
  /**
   * Create a new image and copy a subpart of the actual image into the new
   * image.
   */
  static csRef<iImage> Crop (iImage* source, int x, int y, 
    int width, int height);
  /**
   * Create a sharpened copy of the image.
   * The effect of 'strength' differs from image to image. Values around
   * 128-512 give good results. On really blurry images values up to 1024 or
   * 2048 can be used.
   */
  static csRef<iImage> Sharpen (iImage* source, int strength, 
    csRGBpixel* transp = 0);
  /**
   * Set alpha of all pixels to 0 that match \a transpColor.
   * Pixels that have non-transparent neighbours will be set to the mean
   * color of all non-transparent neighbours, otherwise to \a fillColor.
   * \remarks Intermediately uses a truecolor images, i.e. paletted images
   *  are not handled with maximum efficiency.
   */
  static csRef<iImage> RenderKeycolorToAlpha (iImage* source, 
    const csRGBpixel& transpColor, const csRGBpixel& fillColor);
};

#endif // __CS_CSGFX_IMAGEMANIPULATE_H__
