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

#ifndef __CS_IGRAPHIC_IMAGEIO_H__
#define __CS_IGRAPHIC_IMAGEIO_H__

/**\file
 * Image input/output interface
 */

/**
 * \addtogroup gfx2d
 * @{ */

#include "csutil/scf.h"
#include "csutil/array.h"
#include "igraphic/image.h"
#include "iutil/databuff.h"

SCF_VERSION (iImageIO, 1, 0, 1);

/** \internal
 * Format can be read.
 */
#define CS_IMAGEIO_LOAD 1
/** \internal
 * Format can be written.
 */
#define CS_IMAGEIO_SAVE 2

/// Description for a file format supported by an image loader.
struct csImageIOFileFormatDescription
{
  /// mime type of image, e.g. "image/png"
  const char *mime;
  /// descriptive format specifier, e.g. "8 bit palettized"
  const char *subtype;
  /// a combination of CS_IMAGEIO_* flags
  int cap;
};

/// Description for the array of file formats.
typedef csArray<csImageIOFileFormatDescription const*>
	csImageIOFileFormatDescriptions;

/**
 * The iImageIO interface is used to save and load graphic files.
 */
struct iImageIO : public iBase
{
  /// Description for a file format supported by an image loader.
  typedef csImageIOFileFormatDescription FileFormatDescription;

  /**
   * Propagate the image fileformats handled by this plugin.
   */
  virtual const csImageIOFileFormatDescriptions& GetDescription () = 0;

  /**
   * Load an image from a buffer.<p>
   * This routine will read from the buffer buf , try to
   * recognize the type of image contained within, and return an csImageFile
   * of the appropriate type.  Returns a pointer to the iImage on
   * success, or 0 on failure. The bits that fit the CS_IMGFMT_MASK
   * mask are mandatory: the image always will be loaded in the
   * appropiate format; the bits outside that mask (i.e. CS_IMGFMT_ALPHA)
   * are optional: if the image does not contain alpha mask, the GetFormat()
   * method of the image will return a value without that bit set.
   */
  virtual csPtr<iImage> Load (iDataBuffer* buf, int iFormat) = 0;

  /**
   * Set global image dithering option.<p>
   * By default this option is disabled. If you enable it, all images will
   * be dithered both after loading and after mipmapping/scaling. This will
   * affect all truecolor->paletted image conversions.
   */
  virtual void SetDithering (bool iEnable) = 0;

  /**
   * Save an image using a prefered format.<p>
   * <code>extraoptions</code> allows to specify additional output options.
   * Those options consist of a comma-separated list and can be either 'option' 
   * or 'option=value'. The available options vary from plugin to plugin, some
   * common ones are:<p>
   * <code>compress=#</code> - Set image compression, from 0..100. Higher values 
   * give smaller files, but at the expense of quality(e.g. JPEG) or 
   * speed(e.g. PNG).<br>
   * <code>progressive</code> - Progressive/interlaced encoding.<p>
   * Examples:<br>
   * <code>compress=50</code><br>
   * <code>progressive,compress=30</code>
   */
  virtual csPtr<iDataBuffer> Save (iImage *image, FileFormatDescription *format,
    const char* extraoptions = 0) = 0;

  /**
   * Save an image using format MIME.
   * If omitted format selection is left to the plugin.
   */
  virtual csPtr<iDataBuffer> Save (iImage *image, const char *mime = 0,
    const char* extraoptions = 0) = 0;
};

/** @} */

#endif // __CS_IGRAPHIC_IMAGEIO_H__
