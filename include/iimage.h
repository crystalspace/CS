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

#ifndef __IIMAGE_H__
#define __IIMAGE_H__

#include "csutil/scf.h"

/**
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
/// Do we need alpha channel or not
#define CS_IMGFMT_ALPHA		0x00010000

struct RGBPixel;

SCF_VERSION (iImage, 1, 0, 0);

/**
 * The iImage interface is used to work with image files
 * (what you expected?). Crystal Space supports loading of images in
 * GIF, JPEG, PNG, SGI, TIFF etc formats, you can work with any image
 * through this interface.
 */
struct iImage : public iBase
{
  /**
   * Get image data: returns either (RGBPixel *) or (unsigned char *)
   * depending on format. Note that for RGBA images the RGBPixel structure
   * contains the alpha channel as well, so GetAlpha (see below) method
   * will return NULL (because alpha is not stored separately, as for
   * paletted images).
   */
  virtual void *GetImageData () = 0;
  /// Query image width
  virtual int GetWidth () = 0;
  /// Query image height
  virtual int GetHeight () = 0;
  /// Query image size in bytes
  virtual int GetSize () = 0;

  /// Resize the image to the given size
  virtual void Resize (int NewWidth, int NewHeight) = 0;

  /**
   * Create a new iImage which is a mipmapped version of this one.
   * 'step' indicates how much the mipmap should be scaled down. Only
   * steps 0, 1, 2, and 3 are supported. Step 0 returns the blended version
   * of the image without image being scaled down.
   * The new image will have same format as the original one. If you pass
   * a pointer to a transparent color, the texels of that color are handled
   * differently.
   */
  virtual iImage *MipMap (int step, RGBPixel *transp) = 0;

  /// Set image file name
  virtual void SetName (const char *iName) = 0;
  /// Get image file name
  virtual const char *GetName () = 0;

  /// Qyery image format (see CS_IMGFMT_XXX above)
  virtual int GetFormat () = 0;
  /// Get image palette (or NULL if no palette)
  virtual RGBPixel *GetPalette () = 0;
  /**
   * Get alpha map for 8-bit paletted image.
   * RGBA images contains alpha within themself.
   * If image has no alpha map, or the image is in RGBA format,
   * this function will return NULL.
   */
  virtual UByte *GetAlpha () = 0;
};

#endif // __IIMAGE_H__
