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

#ifndef GIFIMAGE_H
#define GIFIMAGE_H

#include "csgfxldr/csimage.h"

class GIFImageLoader;

/// An csImageFile subclass for reading GIF files.
class ImageGifFile : public csImageFile
{
  ///
  friend class csImageFile;	// For constructor
  friend class GIFImageLoader;

private:
  /// Read the GIF file from the buffer.
  ImageGifFile (UByte* buf, long size);
  void decode_gif(UByte*, long, int*, int*, int*);

public:
  ///
  virtual ~ImageGifFile ();
  /// Return GIF-specific error messages
  virtual const char* get_status_mesg() const;
};

/**
 * The GIF Image Loader.
 */
class GIFImageLoader : public ImageLoader
{
protected:
  ///
  virtual csImageFile* LoadImage (UByte* buf, ULong size);
  virtual AlphaMapFile* LoadAlphaMap(UByte *buf,ULong size);
public:
  ///
  virtual const char* GetName() const
  { return "GIF"; }
  ///
  virtual const char* GetDescription() const 
  { return "CompuServe Graphics InterChange Format (GIF87a,GIF89a)"; }
};

#endif
