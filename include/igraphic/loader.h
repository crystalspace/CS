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

#ifndef __IGRAPHIC_LOADER_H__
#define __IGRAPHIC_LOADER_H__

#include "isys/plugin.h"
struct iImage;


SCF_VERSION (iImageLoader, 0, 0, 1);

/**
 * The image loader is used to load graphic files. Several file formats are
 * supported. 
 */
struct iImageLoader : public iPlugIn
{
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
  virtual iImage *Load (UByte* iBuffer, ULong iSize, int iFormat) = 0;

  /**
   * Set global image dithering option.<p>
   * By default this option is disabled. If you enable it, all images will
   * be dithered both after loading and after mipmapping/scaling. This will
   * affect all truecolor->paletted image conversions.
   */
  virtual void SetDithering (bool iEnable) = 0;
};

#endif // __IGRAPHIC_LOADER_H__

