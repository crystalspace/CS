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

struct iDataBuffer;
struct csRGBpixel;

/*
 * We can request to load image in one of several formats.
 * We can request from csImageLoader to load a image in several formats.
 * The format we should use depends on what we want to do with the image;
 * usually if we're going to use the image as a texture we the format we
 * should use is determined by querying the 3D rasterizer for the preferred
 * image format.
 */
enum
{
  /// The mask to separate the image format apart
  CS_IMGFMT_MASK = 0x0000ffff,
  /// We don't want the pixels at all, just (possibly) the alphamap
  CS_IMGFMT_NONE = 0x00000000,
  /// Truecolor format (r/g/b/unused per each pixel)
  CS_IMGFMT_TRUECOLOR = 0x00000001,
  /// 8-bit indexed paletted image
  CS_IMGFMT_PALETTED8 = 0x00000002,
  /// Autodetect: use whatever format the file is in. Use ONLY for loading.
  CS_IMGFMT_ANY = CS_IMGFMT_MASK,
  /// Do we need alpha channel or not
  CS_IMGFMT_ALPHA = 0x00010000,
  /**
  * This flag indicates an invalid image format. No image may have this set,
  * and you may not load images with this flag set.
  */
  CS_IMGFMT_INVALID = 0x80000000
};

/** Type of an image. */
enum csImageType
{
  /// 2D image. Nothing special.
  csimg2D = 0,
  /**
   * 3D image. The depth slices are arranged consecutively.
   */
  csimg3D,
  /**
   * Cube map. The cube faces are stored as sub images, the indices
   * are the CS_TEXTURE_CUBE_XXX values.
   * \sa CS_TEXTURE_CUBE_POS_X
   */
  csimgCube
};

SCF_VERSION (iImage, 2, 0, 1);

/**
 * The iImage interface is used to work with image objects.
 * <p>
 * You cannot manipulate the pixel data of iImage objects directly. 
 * To do this, you need to instantiate a your own copy of the image, e.g.
 * by creating a csImageMemory instance (which allows access to the
 * pixel data).
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iImageIO::Load()
 *   </ul>
 * \sa csImageMemory, csImageManipulate, csImageTools, csImageCubeMapMaker,
 *   csImageVolumeMaker
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
  virtual const void *GetImageData () = 0;
  /// Query image width
  virtual int GetWidth() const = 0;
  /// Query image height
  virtual int GetHeight() const = 0;
  /// Query image depth (only sensible when the image type is csimg3D)
  virtual int GetDepth() const = 0;

  /// Set image file name
  virtual void SetName (const char *iName) = 0;
  /// Get image file name
  virtual const char *GetName () const = 0;

  /// Qyery image format (see CS_IMGFMT_XXX above)
  virtual int GetFormat () const = 0;
  /// Get image palette (or 0 if no palette)
  virtual const csRGBpixel* GetPalette () = 0;
  /**
   * Get alpha map for 8-bit paletted image.
   * RGBA images contains alpha within themself.
   * If image has no alpha map, or the image is in RGBA format,
   * this function will return 0.
   */
  virtual const uint8* GetAlpha () = 0;

  /// Check if image has a keycolour stored with it.
  virtual bool HasKeyColor () const = 0;

  /**
   * Check if image has a keycolour stored with it.
   * \deprecated Use HasKeyColor() instead.
   */
  CS_DEPRECATED_METHOD virtual bool HasKeycolor () const = 0;

  /// Get the keycolour stored with the image.
  virtual void GetKeyColor (int &r, int &g, int &b) const = 0;

  /**
   * Get the keycolour stored with the image.
   * \deprecated Use GetKeyColor() instead.
   */
  CS_DEPRECATED_METHOD virtual void GetKeycolor (int &r, int &g, int &b) const = 0;

  /**
   * Returns the number of mipmaps contained in the image (in case there exist
   * any precalculated mipmaps), in addition to the original image. 0 means
   * there are no precomputed mipmaps.
   */
  virtual uint HasMipmaps () const = 0;
  /**
   * Return a precomputed mipmap. \a num specifies which mipmap to return;
   * 0 returns the original image, \a num <= the return value of HasMipmaps()
   * returns that mipmap.
   */
  virtual csRef<iImage> GetMipmap (uint num) = 0;
  
  /**
   * Get a string identifying the format of the raw data of the image
   * (or 0 if raw data is not provided).
   */
  virtual const char* GetRawFormat() const = 0;
  /**
   * Get the raw data of the image (or 0 if raw data is not provided).
   */
  virtual csRef<iDataBuffer> GetRawData() const = 0;
  /**
   * Get the type of the contained image.
   */
  virtual csImageType  GetImageType() const = 0;
  /**
   * Returns the number of sub images, in addition to this image. 
   * Subimages are usually used for cube map faces.
   */
  virtual uint HasSubImages() const = 0;
  /**
   * Query a sub image.
   * A value of 0 for \a num returns the original image, a value larger or equal
   * than the return value of HasSubImages() returns that sub image, any other
   * value returns 0.
   */
  virtual csRef<iImage> GetSubImage (uint num) = 0;
};

/** @} */

#endif // __CS_IGRAPHIC_IMAGE_H__

