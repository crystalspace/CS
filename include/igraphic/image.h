/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#ifndef __CS_IGRAPHIC_IMAGE_H__
#define __CS_IGRAPHIC_IMAGE_H__

/**\file
 * Image interface
 */

/**
 * \addtogroup gfx2d
 * @{
 */

#include "csutil/scf.h"

/*
 * We can request to load image in one of several formats.
 * We can request from csImageLoader to load a image in several formats.
 * The format we should use depends on what we want to do with the image;
 * usually if we're going to use the image as a texture we the format we
 * should use is determined by querying the 3D rasterizer for the preferred
 * image format.
 */

/// The mask to separate the image format apart
#define CS_IMGFMT_MASK		0x0000ffff
/// We don't want the pixels at all, just (possibly) the alphamap
#define CS_IMGFMT_NONE		0x00000000
/// Truecolor format (r/g/b/unused per each pixel)
#define CS_IMGFMT_TRUECOLOR	0x00000001
/// 8-bit indexed paletted image
#define CS_IMGFMT_PALETTED8	0x00000002
/// Autodetect: use whatever format the file is in. Use ONLY for loading.
#define CS_IMGFMT_ANY		CS_IMGFMT_MASK
/// Do we need alpha channel or not
#define CS_IMGFMT_ALPHA		0x00010000
/**
 * This flag indicates an invalid image format. No image may have this set,
 * and you may not load images with this flag set.
 */
#define CS_IMGFMT_INVALID	0x80000000


struct csRGBpixel;

SCF_VERSION (iImage, 1, 0, 3);

/**
 * The iImage interface is used to work with image files
 * (what did you expect?). Crystal Space supports loading of images in
 * GIF, JPEG, PNG, SGI etc formats, you can work with any image
 * through this interface.
 */
struct iImage : public iBase
{
  /**
   * Get image data: returns either (csRGBpixel *) or (unsigned char *)
   * depending on format. Note that for RGBA images the csRGBpixel structure
   * contains the alpha channel as well, so GetAlpha (see below) method
   * will return 0 (because alpha is not stored separately, as for
   * paletted images).
   */
  virtual void *GetImageData () = 0;
  /// Query image width
  virtual int GetWidth () = 0;
  /// Query image height
  virtual int GetHeight () = 0;
  /// Query image size in bytes
  virtual int GetSize () = 0;

  /// Rescale the image to the given size
  virtual void Rescale (int NewWidth, int NewHeight) = 0;

  /**
   * Create a new iImage which is a mipmapped version of this one.
   * 'step' indicates how much the mipmap should be scaled down. Step 0 
   * returns a blurred version of the image without image being scaled down.
   * Step 1 scales the image down to 1/2. Steps &gt; 1 repeat this 
   * <i>'step'</i> times.
   * The new image will have same format as the original one. If you pass
   * a pointer to a transparent color, the texels of that color are handled
   * differently.
   */
  virtual csPtr<iImage> MipMap (int step, csRGBpixel *transp) = 0;

  /// Set image file name
  virtual void SetName (const char *iName) = 0;
  /// Get image file name
  virtual const char *GetName () = 0;

  /// Qyery image format (see CS_IMGFMT_XXX above)
  virtual int GetFormat () = 0;
  /// Get image palette (or 0 if no palette)
  virtual csRGBpixel *GetPalette () = 0;
  /**
   * Get alpha map for 8-bit paletted image.
   * RGBA images contains alpha within themself.
   * If image has no alpha map, or the image is in RGBA format,
   * this function will return 0.
   */
  virtual uint8 *GetAlpha () = 0;
  /**
   * Convert the image to another format.
   * This method will allocate a respective color component if
   * it was not allocated before. For example, you can use this
   * method to add alpha channel to paletted images, to allocate
   * a image for CS_IMGFMT_NONE alphamaps or vice versa, to remove
   * the image and leave alphamap alone. This routine may be used
   * as well for removing alpha channel.
   */
  virtual void SetFormat (int iFormat) = 0;

  /// Create yet another image and copy this one into the new image.
  virtual csPtr<iImage> Clone () = 0;

  /// Create a new image and copy a subpart of the actual image into the new image.
  virtual csPtr<iImage> Crop (int x, int y, int width, int height) = 0;

  /// Check if all alpha values are "non-transparent" and if so, discard alpha
  virtual void CheckAlpha () = 0;

  /// check if image has a keycolour stored with it
  virtual bool HasKeycolor () = 0;

  /// get the keycolour stored with the image.
  virtual void GetKeycolor (int &r, int &g, int &b) = 0;

  /**
   * Create a sharpened copy of the image.
   * The effect of 'strength' differs from image to image. Values around 128-512 
   * give good results. On really blurry images values up to 1024 or 2048 can be 
   * used.
   */
  virtual csPtr<iImage> Sharpen (csRGBpixel *transp, int strength) = 0;

  /** Returns the number of mipmaps contained in the image (in case there
   * exist any precalculated mipmaps.
   */
  virtual int HasMipmaps () = 0;
};

/** @} */

#endif // __CS_IGRAPHIC_IMAGE_H__
