/*
    Copyright (C) 1998 by Andrew Zabolotny <bit@eltech.ru>

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

#include "csgfxldr/csimage.h"

class PNGImageLoader;

/**
 * An ImageFile subclass for reading PNG files.<p>
 * This implementation needs both zlib and pnglib to read .PNG files.
 */
class ImagePngFile : public ImageFile
{
  ///
  friend class ImageFile;	// For constructor
  friend class PNGImageLoader;

private:
  /// Read the PNG file from the buffer.
  ImagePngFile (UByte* buf, size_t size);
  ///
  static void PNG_read (png_structp png, png_bytep data, png_size_t size);

  UByte *r_data;
  size_t r_size;

public:
  ///
  virtual ~ImagePngFile ();
};

/**
 * The PNG Image Loader
 */
class PNGImageLoader : public ImageLoader
{
protected:
  ///
  virtual ImageFile* LoadImage (UByte* buf, ULong size);

public:
  ///
  virtual const char* GetName() const
  { return "PNG"; }
  ///
  virtual const char* GetDescription() const 
  { return "Portable Network Graphics"; }
};

#endif //PNGIMAGE_H
