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

#include <stdio.h>
#include "csgfxldr/rgbpixel.h"
#include "cstypes.h"
#include "igraphic/iimage.h"

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
  csRGBpixel *Palette;
  /// The alpha map
  UByte *Alpha;
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
   * load the image in csRGBpixel format and pass the pointer to this
   * function which will handle the RGB -> target format conversion.
   * NOTE: the pointer should be allocated with new csRGBpixel [] and you should
   * not free it after calling this function: the function will free
   * the buffer itself if it is appropiate (or wont if the buffer
   * size/contents are appropiate for target format).
   */
  void convert_rgba (csRGBpixel *iImage);

  /**
   * Used to convert an 8-bit indexed image into requested format.
   * Pass a pointer to color indices and a pointer to palette, and you're done.
   * NOTE: the pointer should be allocated with new UByte [] and you should
   * not free it after calling this function: the function will free
   * the buffer itself if it is appropiate (or wont if the buffer
   * size/contents are appropiate for target format). Same about palette.
   */
  void convert_pal8 (UByte *iImage, csRGBpixel *iPalette, int nPalColors = 256);

  /**
   * Same as above but accepts an array of csRGBcolor's as palette.
   * The csRGBcolor array is never freed, so its your responsability
   * if you did it.
   */
  void convert_pal8 (UByte *iImage, csRGBcolor *iPalette, int nPalColors = 256);

  /**
   * Free all image data: pixels and palette. Takes care of image data format.
   */
  void FreeImage ();

  /// Return the closest color index to given. Fails if image has no palette.
  int closest_index (csRGBpixel *iColor);

public:
  DECLARE_IBASE;

  /// Destroy the image file object and free all associated storage
  virtual ~csImageFile ();

  /**************************** iImage interface *****************************/
  /**
   * Get image data: returns either (csRGBpixel *) or (unsigned char *)
   * depending on format. Note that for RGBA images the csRGBpixel structure
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

  /// Rescale the image to the given size
  virtual void Rescale (int newwidth, int newheight);

  /**
   * Create a new csImageFile which is a mipmapped version of this one.
   * 'step' indicates how much the mipmap should be scaled down. Only
   * steps 0, 1, 2, and 3 are supported. Step 0 returns the blended version
   * of the image without image being scaled down.
   * The new image will have same format as the original one. If you pass
   * a pointer to a transparent color, the texels of that color are handled
   * differently.
   */
  virtual iImage *MipMap (int step, csRGBpixel *transp);

  /// Set image file name
  virtual void SetName (const char *iName);
  /// Get image file name
  virtual const char *GetName ();
  /// Get image format
  virtual int GetFormat ();
  /// Get image palette (or NULL if no palette)
  virtual csRGBpixel *GetPalette ();
  /// Get alpha map for image
  virtual UByte *GetAlpha ();
  /// Convert the image to another format
  virtual void SetFormat (int iFormat);
  /// Create yet another image and copy this one into the new image.
  virtual iImage *Clone ();
  /// Create another image and copy a subpart of this image into the new image.
  virtual iImage *Crop (int x, int y, int width, int height);
  /// Check if all alpha values are "non-transparent" and if so, discard alpha
  virtual void CheckAlpha ();
  /// check if image has a keycolour stored with it
  virtual bool HasKeycolor ();
  /// get the keycolour stored with the image.
  virtual void GetKeycolor (int &r, int &g, int &b);
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

  /*
   * WARNING: This class should have NO destructor otherwise csgfxldr library
   * will be unusable for linking into plugins because on most platforms
   * static objects (see bool RegisterXXX()) will register an atexit()
   * callback for the destructor, and during program exit sequence the
   * plugins won't be present in memory (they are unloaded in ~SystemDriver).
   */

public:
  /**
   * Register a loader for a given image type.<p>
   * Adds 'loader' to the list of image formats to be checked during an
   * csImageLoader::load(...) call.
   */
  static bool Register (csImageLoader *loader);

  /**
   * Load an image from a buffer.<p>
   * This routine will read from the buffer buf of length size, try to
   * recognize the type of image contained within, and return an csImageFile
   * of the appropriate type.  Returns a pointer to the iImage on
   * success, or NULL on failure. The bits that fit the CS_IMGFMT_MASK
   * mask are mandatory: the image always will be loaded in the
   * appropiate format; the bits outside that mask (i.e. CS_IMGFMT_ALPHA)
   * are optional: if the image does not contain alpha mask, the GetFormat()
   * method of the image will return a value without that bit set.
   */
  static iImage *Load (UByte* iBuffer, ULong iSize, int iFormat);

  /**
   * Set global image dithering option.<p>
   * By default this option is disabled. If you enable it, all images will
   * be dithered both after loading and after mipmapping/scaling. This will
   * affect all truecolor->paletted image conversions.
   */
  static void SetDithering (bool iEnable);
};

#endif // __CS_IMAGE_H__
