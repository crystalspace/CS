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
 * This class is a superclass for all image file format loaders. Every
 * subclass of this class is responsible for loading a specific file
 * format. You can request the image to be loaded in a specific internal
 * format (see CS_IMG_XXX constants).
 */
class csImageLoader
{
public:
  /// this class needs a virtual destructor
  virtual ~csImageLoader() {}

  /**
   * Load an image from the given buffer.
   * Attempts to read an image from the buffer 'buf' of length 'size'.
   * If successful, returns a pointer to the resulting csImageFile.  Otherwise
   * returns NULL.
   */
  virtual csImageFile* LoadImage (UByte* iBuffer, ULong iSize, int iFormat) = 0;
};

#endif // __IMGLOAD_H__
