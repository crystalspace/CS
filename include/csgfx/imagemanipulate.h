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
#include "csutil/cscolor.h"

/**
 * Helper class to manipulate iImage objects.
 * The methods in this class generally return new images.
 */
class CS_CRYSTALSPACE_EXPORT csImageManipulate
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
   * Do color manipulation on the image data and return a new image.
   * This function works for RGBA and paletted images. Since the mult
   * and add parameters are 4-colors you can also manipulate the alpha
   * channel using this function.
   * \param mult is a 4-color indicating a multiplier to use for the colors.
   * \param add is a 4-color indicating an adder to use for the colors.
   */
  static csRef<iImage> TransformColor (iImage* source,
      const csColor4& mult, const csColor4& add);

  /**
   * Create a new grayscale version of the given image.
   * This function works for RGBA and paletted images.
   */
  static csRef<iImage> Gray (iImage* source);
  
  /**
   * Renormalizes a normal map (ie RGB triplets represent XYZ directions,
   * stored biased and scaled so that the value 0 is mapped to -1, 
   * the value 255 is mapped to 1). 
   */
  static csRef<iImage> RenormalizeNormals (iImage* source);
};

/** @} */

#endif // __CS_CSGFX_IMAGEMANIPULATE_H__
