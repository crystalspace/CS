/*
    Copyright (C) 1998 by Tor Andersson

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

#ifndef JPGIMAGE_H
#define JPGIMAGE_H

#include "csgfxldr/csimage.h"

class JPGImageLoader;

/**
 * An ImageFile subclass for reading JPG files.<p>
 * This implementation needs libjpeg to read JFIF files.
 */
class ImageJpgFile : public ImageFile
{
  ///
  friend class ImageFile;	// For constructor
  friend class JPGImageLoader;

private:
  /// Read the JPG file from the buffer.
  ImageJpgFile (UByte* buf, long size);

public:
  ///
  virtual ~ImageJpgFile ();
};

/**
 * the JPEG Image Loader.
 */
class JPGImageLoader : public ImageLoader
{
protected:
  ///
  virtual ImageFile* LoadImage (UByte* buf, ULong size);

public:
  ///
  virtual const char* GetName() const
  { return "JPEG"; }
  ///
  virtual const char* GetDescription() const 
  { return "JPEG image format"; }
};

#endif //JPGIMAGE_H
