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

#ifndef __IMGLOAD_H__
#define __IMGLOAD_H__

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
   * WARNING: This class should have NO destructor otherwise the image loader
   * won't work as a plugin because on most platforms static objects (see bool
   * RegisterXXX()) will register an atexit() callback for the destructor, and
   * during program exit sequence the plugins won't be present in memory (they
   * are unloaded in ~SystemDriver).
   */

public:
  
  /// Return the next loader in the chain
  csImageLoader *GetNext()
  {
    return Next;
  }

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

#endif // __IMGLOAD_H__
