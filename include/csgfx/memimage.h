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

#ifndef __CS_MEMIMAGE_H__
#define __CS_MEMIMAGE_H__

#include "csimage.h"

/**
 * This class represents a block of memory in the true colour format only.
 * It is used with dynamic textures so that the usual method of registering
 * and preparing textures if fulfilled. 8bit images and cropping/cloning make
 * no  sense in this context.
 */

class csImageMemory : public csImageFile
{
  /// If we are a blank image, we can take a short cut with rescaling
  bool short_cut;
  /// If true when these interfaces are destroyed the image is also.
  bool destroy_image;
public:
  /// Create a blank true colour image of these dimensions.
  csImageMemory (int width, int height);
  /**
   * Create an iImage interface for this true colour buffer with
   * these dimensions. If destroy is set to true then the supplied buffer
   * will be destroyed when the interfaces are.
   */
  csImageMemory (int width, int height, csRGBpixel *buffer, bool destroy);

  virtual ~csImageMemory ();

  /// Clears image to colour
  void Clear (const csRGBpixel &colour);

  /// Rescale the image to the given size
  virtual void Rescale (int NewWidth, int NewHeight);

  /// Qyery image format (see CS_IMGFMT_XXX above)
  virtual int GetFormat ()
  { return CS_IMGFMT_TRUECOLOR; }
  /// Get image palette (or NULL if no palette)
  virtual csRGBpixel *GetPalette ()
  { return NULL; }
  /// Get alpha map for 8-bit paletted image.
  virtual uint8 *GetAlpha ()
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


#endif // __CS_MEMIMAGE_H__
