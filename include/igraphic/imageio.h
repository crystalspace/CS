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

#ifndef __IGRAPHIC_IMAGEIO_H__
#define __IGRAPHIC_IMAGEIO_H__

#include "csutil/scf.h"
struct iImage;
struct iDataBuffer;
class csVector;

SCF_VERSION (iImageIO, 1, 0, 0);

/**
 * The iImageIO interface is used to save and load graphic files.
 */

#define CS_IMAGEIO_LOAD 1
#define CS_IMAGEIO_SAVE 2

struct iImageIO : public iBase
{
  struct FileFormatDescription
  {
    /// mime type of image, e.g. "image/png"
    const char *mime;
    /// descriptive format specifier, e.g. "8 bit palettized"
    const char *subtype;
    /// a combination of CS_IMAGEIO_* flags
    int cap;
  };

  /**
   * Propagate the image fileformats handled by this plugin.
   */
  virtual const csVector& GetDescription () = 0;

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
  virtual iImage *Load (uint8* iBuffer, uint32 iSize, int iFormat) = 0;

  /**
   * Set global image dithering option.<p>
   * By default this option is disabled. If you enable it, all images will
   * be dithered both after loading and after mipmapping/scaling. This will
   * affect all truecolor->paletted image conversions.
   */
  virtual void SetDithering (bool iEnable) = 0;

  /**
   * Save an image using a prefered format.
   */
  virtual iDataBuffer *Save (iImage *image, iImageIO::FileFormatDescription *format,
    const char* extraoptions = NULL) = 0;

  /**
   * Save an image using format <mime>.
   * If omitted format selection is left to the plugin.
   */
  virtual iDataBuffer *Save (iImage *image, const char *mime = NULL,
    const char* extraoptions = NULL) = 0;
};

#endif // __IGRAPHIC_IMAGEIO_H__
