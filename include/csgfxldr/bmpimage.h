/*
    BMPImage class
    Copyright (C) 1998 by Olivier Langlois <olanglois@sympatico.ca>
  
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

#ifndef BMPIMAGE_H
#define BMPIMAGE_H

#include "csgfxldr/csimage.h"

class BMPImageLoader;

/** An csImageFile subclass for reading BMP files.
 *
 *  Current Limitations:
 *  Only able to read 24 bits RGB encoded with no palette files and
 *                     8 bits RGB encoded files
 */
class ImageBMPFile : public csImageFile
{
  ///
  friend class csImageFile;	// For constructor
  friend class BMPImageLoader;

private:
  /// Read the BMP file from the buffer.
  ImageBMPFile (UByte* buf, long size);

public:
  ///
  virtual ~ImageBMPFile ();
  /// Return BMP-specific error messages
  virtual const char* get_status_mesg() const;
};

/**
 * The BMP Image Loader.
 */
class BMPImageLoader : public ImageLoader
{
protected:
  ///
  virtual csImageFile* LoadImage (UByte* buf, ULong size);
  virtual AlphaMapFile* LoadAlphaMap(UByte* buf,ULong size);

public:
  ///
  virtual const char* GetName() const
  { return "BMP"; }
  ///
  virtual const char* GetDescription() const 
  { return "BitMap Image Format"; }
};

#endif
