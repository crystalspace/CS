/*
    Copyright (C) 1998 by Jorrit Tyberghein
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

#ifndef IMAGE_H
#define IMAGE_H

#include <stdio.h>
#include "csgfxldr/rgbpixel.h"
#include "types.h"
#include "iimage.h"

/**
 * An abstract class representing an abstract image. For every image
 * type supported, a subclass should be created for loading that image
 * type. The image file class contains a number of member functions
 * for accessing and manipulating the image contents.
 */
class csImageFile : public iImage
{
protected:
  /// Width of image.
  int Width;
  /// Height of image.
  int Height;
  /// The image data.
  void *Image;
  /// The image palette or NULL
  RGBPixel *Palette;
  /// The alpha map
  UByte *Alpha;
  /// Image file name
  char *fName;
  /// Image format (see CS_IMGFMT_XXX above)
  int Format;

  /**
   * csImageFile constructor.
   * This object can only be created by an appropriate loader, which is why
   * the constructor is protected.
   */
  csImageFile (int iFormat);

  /**
   * Set the width and height.
   * This will also free the 'image' buffer to hold the bitmap,
   * but it will NOT allocate a new buffer (thus `image' is NULL
   * after calling this function). You should pass an appropiate
   * pointer to one of convert_xxx functions below to define the
   * image itself (or assign something to `image' manually).
   */
  void set_dimensions (int w, int h);

  /**
   * Used to convert an truecolor RGB image into requested format.
   * If the image loader cannot handle conversion itself, and the image
   * file is in a format that is different from the requested one,
   * load the image in RGBPixel format and pass the pointer to this
   * function which will handle the RGB -> target format conversion.
   * NOTE: the pointer should be allocated with new RGBPixel [] and you should
   * not free it after calling this function: the function will free
   * the buffer itself if it is appropiate (or wont if the buffer
   * size/contents are appropiate for target format).
   */
  void convert_rgb (RGBPixel *iImage);

  /**
   * Used to convert an 8-bit indexed image into requested format.
   * Pass a pointer to color indices and a pointer to palette, and you're done.
   * NOTE: the pointer should be allocated with new UByte [] and you should
   * not free it after calling this function: the function will free
   * the buffer itself if it is appropiate (or wont if the buffer
   * size/contents are appropiate for target format). Same about palette.
   */
  void convert_8bit (UByte *iImage, RGBPixel *iPalette);

  /**
   * Same as above but accepts an array of RGBcolor's as palette.
   * The RGBcolor array is never freed, so its your responsability
   * if you did it.
   */
  void convert_8bit (UByte *iImage, RGBcolor *iPalette);

  /**
   * Free all image data: pixels and palette. Takes care of image data format.
   */
  void free_image ();

  /// Return the closest color index to given. Fails if image has no palette.
  int closest_index (RGBPixel *iColor);

public:
  DECLARE_IBASE;

  /// Destroy the image file object and free all associated storage
  virtual ~csImageFile ();

  /***************************** iImage interface *****************************/
  /**
   * Get image data: returns either (RGBPixel *) or (unsigned char *)
   * depending on format. Note that for RGBA images the RGBPixel structure
   * contains the alpha channel as well, so GetAlpha (see below) method
   * will return NULL (because alpha is not stored separately, as for
   * paletted images).
   */
  virtual void *GetImageData ();
  /// Query image width
  virtual int GetWidth ();
  /// Query image height
  virtual int GetHeight ();
  /// Query image size in bytes
  virtual int GetSize ();

  /// Resize the image to the given size
  virtual void Resize (int newwidth, int newheight);

  /**
   * Create a new csImageFile which is a mipmapped version of this one.
   * 'step' indicates how much the mipmap should be scaled down. Only
   * steps 0, 1, 2, and 3 are supported. Step 0 returns the blended version
   * of the image without image being scaled down.
   * The new image will have same format as the original one. If you pass
   * a pointer to a transparent color, the texels of that color are handled
   * differently.
   */
  virtual iImage *MipMap (int step, RGBPixel *transp);

  /// Set image file name
  virtual void SetName (const char *iName);
  /// Get image file name
  virtual const char *GetName ();
  /// Get image format
  virtual int GetFormat ();
  /// Get image palette (or NULL if no palette)
  virtual RGBPixel *GetPalette ();
  /// Get alpha map for image
  virtual UByte *GetAlpha ();
};

/**
 * This class handles everything related to image loading.
 * All image loaders are chained in a list and you can register new
 * image handlers even at runtime. You can request the image to be loaded
 * in a specific format (see CS_IMG_XXX constants).
 * Extend this class to support a particular type of image loading.
 */
class csImageLoader
{
  /// A pointer to next image loader (loaders are chained in a list)
  csImageLoader *Next;

protected:
  /**
   * Load an image from the given buffer.
   * Attempts to read an image from the buffer 'buf' of length 'size'.
   * If successful, returns a pointer to the resulting csImageFile.  Otherwise
   * returns NULL.
   */
  virtual csImageFile* LoadImage (UByte* iBuffer, ULong iSize, int iFormat) = 0;

  /// Not really needed
  virtual ~csImageLoader() { }

public:
  /**
   * Register a loader for a given image type.
   * Adds 'loader' to the list of image formats to be checked during an
   * csImageLoader::load(...) call.
   */
  static bool Register (csImageLoader *loader);

  /**
   * Load an image from a buffer.
   * This routine will read from the buffer buf of length size, try to
   * recognize the type of image contained within, and return an csImageFile
   * of the appropriate type.  Returns a pointer to the iImage on
   * success, or NULL on failure.
   */
  static iImage *Load (UByte* iBuffer, ULong iSize, int iFormat);
};

#endif
