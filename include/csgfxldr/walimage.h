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

#ifndef WALIMAGE_H
#define WALIMAGE_H

#include "csgfxldr/csimage.h"

class WALImageLoader;

/**
 * An ImageFile subclass for reading WAL files.
 */
class ImageWALFile : public ImageFile
{
  ///
  friend class ImageFile;	// For constructor
  friend class WALImageLoader;

private:
  /// Read the WAL file from the buffer.
  ImageWALFile (UByte* buf, long size);

public:
  ///
  virtual ~ImageWALFile ();
};

/**
 * The TGA Image Loader.
 */
class WALImageLoader : public ImageLoader
{
protected:
  ///
  virtual ImageFile* LoadImage (UByte* buf, ULong size);

public:
  ///
  virtual const char* GetName() const
  { return "WAL"; }
  ///
  virtual const char* GetDescription() const 
  { return "WAL (Quake2) image format"; }
};

/* Header definition. */
struct WALHeader {
	unsigned char Name[32];		//Internal Name
	unsigned int width;		//Width of largest mipmap
	unsigned int height;		//Height of largest mipmap
	unsigned int offsets[4];	//Offset to 4 mipmaps in file
	char nextframe[32];		//Name of next file in animation if any
	unsigned int flags;		//??
	unsigned int contents;		//??
	unsigned int value;		//??
    }_WALHeader;
#endif

