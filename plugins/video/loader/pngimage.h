/*
    Copyright (C) 1998,2000 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef PNGIMAGE_H
#define PNGIMAGE_H

#include "csgfx/csimage.h"
#include "imgload.h"

/**
 * The PNG Image Loader
 */
class csPNGImageLoader : public csImageLoader
{
protected:
  /// Try to load the image
  virtual csImageFile* LoadImage (UByte* iBuffer, ULong iSize, int iFormat);
};

/**
 * An csImageFile subclass for reading PNG files.<p>
 * This implementation needs both zlib and pnglib to read .PNG files.
 */
class ImagePngFile : public csImageFile
{
  friend class csPNGImageLoader;
private:
  /// Initialize the image object
  ImagePngFile (int iFormat) : csImageFile (iFormat) { };
  /// Try to read the PNG file from the buffer and return success status
  bool Load (UByte* iBuffer, ULong iSize);
};

#endif // PNGIMAGE_H
