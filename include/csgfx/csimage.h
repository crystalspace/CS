/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein
    Contributions made by Ivan Avramovic <ivan@avramovic.com>

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

#ifndef __CS_IMAGE_H__
#define __CS_IMAGE_H__

#include "csextern.h"

#include <stdio.h>
#include "csgfx/rgbpixel.h"
#include "csutil/leakguard.h"
#include "cstypes.h"
#include "igraphic/image.h"

/**
 * An abstract class representing an abstract image. For every image
 * type supported, a subclass should be created for loading that image
 * type. The image file class contains a number of member functions
 * for accessing and manipulating the image contents.
 */
class CS_CSGFX_EXPORT csImageFile : public iImage
{
protected:
  /// Width of image.
  int Width;
  /// Height of image.
  int Height;
  /// The image data.
  void* Image;
  /// The image palette or 0
  csRGBpixel *Palette;
  /// The alpha map
  uint8 *Alpha;
  /// Image file name
  char *fName;
  /// Image format (see CS_IMGFMT_XXX above)
  int Format;
  /// if it has a keycolour, and the keycolour values.
  uint8 has_keycolour, keycolour_r, keycolour_g, keycolour_b;

  /**
   * csImageFile constructor.
   * This object can only be created by an appropriate loader, which is why
   * the constructor is protected.
   */
  csImageFile (int iFormat);

  /**
   * Set the width and height.
   * This will also free the 'image' buffer to hold the bitmap,
   * but it will NOT allocate a new buffer (thus `image' is 0
   * after calling this function). You should pass an appropiate
   * pointer to one of convert_xxx functions below to define the
   * image itself (or assign something to `image' manually).
   */
  void set_dimensions (int w, int h);

  /**
   * Used to convert an truecolor RGB image into requested format.
   * If the image loader cannot handle conversion itself, and the image
   * file is in a format that is different from the requested one,
   * load the image in csRGBpixel format and pass the pointer to this
   * function which will handle the RGB -> target format conversion.
   * NOTE: the pointer should be allocated with new csRGBpixel [] and you should
   * not free it after calling this function: the function will free
   * the buffer itself if it is appropiate (or wont if the buffer
   * size/contents are appropiate for target format).
   */
  void convert_rgba (csRGBpixel* iImage);

  /**
   * Used to convert an 8-bit indexed image into requested format.
   * Pass a pointer to color indices and a pointer to palette, and you're done.
   * NOTE: the pointer should be allocated with new uint8 [] and you should
   * not free it after calling this function: the function will free
   * the buffer itself if it is appropiate (or wont if the buffer
   * size/contents are appropiate for target format). Same about palette.
   */
  void convert_pal8 (uint8* iImage, csRGBpixel* iPalette, int nPalColors = 256);

  /**
   * Same as above but accepts an array of csRGBcolor's as palette.
   * The csRGBcolor array is never freed, so its your responsability
   * if you did it.
   */
  void convert_pal8 (uint8* iImage, const csRGBcolor* iPalette,
  	int nPalColors = 256);

  /**
   * Free all image data: pixels and palette. Takes care of image data format.
   */
  virtual void FreeImage ();

  /// Return the closest color index to given. Fails if image has no palette.
  int closest_index (csRGBpixel *iColor);

public:
  SCF_DECLARE_IBASE;

  CS_LEAKGUARD_DECLARE (csImageFile);

  /// Destroy the image file object and free all associated storage
  virtual ~csImageFile ();

  /**************************** iImage interface *****************************/
  /**
   * Get image data: returns either (csRGBpixel *) or (unsigned char *)
   * depending on format. Note that for RGBA images the csRGBpixel structure
   * contains the alpha channel as well, so GetAlpha (see below) method
   * will return 0 (because alpha is not stored separately, as for
   * paletted images).
   */
  virtual void *GetImageData ();
  /// Query image width
  virtual int GetWidth ();
  /// Query image height
  virtual int GetHeight ();
  /// Query image size in bytes
  virtual int GetSize ();

  /// Rescale the image to the given size
  virtual void Rescale (int newwidth, int newheight);

  /**
   * Create a new csImageFile which is a mipmapped version of this one.
   * 'step' indicates how much the mipmap should be scaled down. Step 0 
   * returns a blurred version of the image without image being scaled down.
   * Step 1 scales the image down to 1/2. Steps &gt; 1 repeat this 
   * <i>'step'</i> times.
   * The new image will have same format as the original one. If you pass
   * a pointer to a transparent color, the texels of that color are handled
   * differently.
   */
  virtual csPtr<iImage> MipMap (int step, csRGBpixel *transp);

  /// Set image file name
  virtual void SetName (const char *iName);
  /// Get image file name
  virtual const char *GetName ();
  /// Get image format
  virtual int GetFormat ();
  /// Get image palette (or 0 if no palette)
  virtual csRGBpixel *GetPalette ();
  /// Get alpha map for image
  virtual uint8 *GetAlpha ();
  /// Convert the image to another format
  virtual void SetFormat (int iFormat);
  /// Create yet another image and copy this one into the new image.
  virtual csPtr<iImage> Clone ();
  /// Create another image and copy a subpart of this image into the new image.
  virtual csPtr<iImage> Crop (int x, int y, int width, int height);
  /// Check if all alpha values are "non-transparent" and if so, discard alpha
  virtual void CheckAlpha ();
  /// check if image has a keycolour stored with it
  virtual bool HasKeycolor ();
  /// get the keycolour stored with the image.
  virtual void GetKeycolor (int &r, int &g, int &b);
  /// Create a sharpened copy of the image
  virtual csPtr<iImage> Sharpen (csRGBpixel *transp, int strength);
  virtual int HasMipmaps ();
};

#endif // __CS_IMAGE_H__
