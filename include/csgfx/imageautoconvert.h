/*
    Copyright (C) 2006 by Jorrit Tyberghein
	      (C) 2006 by Frank Richter

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

#ifndef __CS_CSGFX_IMAGEAUTOCONVERT_H__
#define __CS_CSGFX_IMAGEAUTOCONVERT_H__

#include "csgfx/imagememory.h"

/**\file
 * Automatic conversion of an image into a different format.
 */
 
/**\addtogroup gfx
 * @{ 
 */

namespace CS
{
  /**
   * Small wrapper class to automatically convert an image into a different
   * storage format, if needed.
   */
  class ImageAutoConvert
  {
    csRef<iImage> theImage;
  public:
    /**
     * Provide access to \a image itself or a copy in with a format of
     * \a desiredFormat.
     * \param image Image to wrap.
     * \param desiredFormat The desired image format. If \a image has the
     *   correct format it's used directly; otherwise, a copy having the given
     *   format will be created.
     */
    ImageAutoConvert (iImage* image, int desiredFormat)
    {
      if (image)
      {
        if (image->GetFormat () == desiredFormat)
          theImage = image;
        else
          theImage.AttachNew (new csImageMemory (image, desiredFormat));
      }
    }
    /// Provide access to the image.
    iImage* operator->() const { return theImage; }
    /// Provide access to the image.
    operator iImage*() const { return theImage; }
  };
}

/** @} */

#endif // __CS_CSGFX_IMAGEAUTOCONVERT_H__
