/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Written by Samuel Humphreys

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

#ifndef __MEMIMAGE_H__
#define __MEMIMAGE_H__


#include "iimage.h"

class csImageMemory : public iImage
{
public:
  DECLARE_IBASE;
  void *Data;
  int Width, Height;

  csImageMemory (int width, int height);
  virtual ~csImageMemory ();

  /// Get a pointer to image data
  virtual void *GetImageData ()
  { return Data; }
  /// Query image width
  virtual int GetWidth ()
  { return Width; }
  /// Query image height
  virtual int GetHeight ()
  { return Height; }
  /// Query image size in bytes
  virtual int GetSize () { return Width * Height; }
  /// Rescale the image to the given size
  virtual void Rescale (int NewWidth, int NewHeight);
  /// Create a new iImage which is a mipmapped version of this one.
  virtual iImage *MipMap (int step, RGBPixel *transp)
  { (void)step; (void)transp; return NULL; }
  /// Set image file name
  virtual void SetName (const char *iName)
  { (void) iName; }
  /// Get image file name
  virtual const char *GetName ()
  { return NULL; }
  /// Qyery image format (see CS_IMGFMT_XXX above)
  virtual int GetFormat ()
  { return CS_IMGFMT_TRUECOLOR; }
  /// Get image palette (or NULL if no palette)
  virtual RGBPixel *GetPalette ()
  { return NULL; }
  /// Get alpha map for 8-bit paletted image.
  virtual UByte *GetAlpha ()
  { return NULL; }
  /// Change image format
  virtual void SetFormat (int /*iFormat*/)
  {  }
  /// Create yet another image and copy this one into the new image.
  virtual iImage *Clone ()
  { return NULL; }
  /// Create another image holding a subimage of current image
  virtual iImage *Crop (int /*x*/, int /*y*/, int /*width*/, int /*height*/ )
  { return NULL; }
};


#endif // __MEMIMAGE_H__
