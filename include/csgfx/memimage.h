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

#include "csextern.h"

#include "csimage.h"

/**
 * Memory image.
 * \remark Does not support cropping or cloning.
 */
class CS_CSGFX_EXPORT csImageMemory : public csImageFile
{
private:
  /// If we are a blank image, we can take a short cut with rescaling
  bool short_cut;
  /// If true when these interfaces are destroyed the image is also.
  bool destroy_image;
 
protected:
  virtual void FreeImage ();

public:
  /**
   * Create a blank image of these dimensions and the specified
   * format.
   * \param width Width of the image
   * \param height Height of the image
   * \param format Image format. Default: #CS_IMGFMT_TRUECOLOR
   */
  csImageMemory (int width, int height, int format = CS_IMGFMT_TRUECOLOR);
  /**
   * Create an iImage interface for this true colour buffer with
   * these dimensions. If destroy is set to true then the supplied buffer
   * will be destroyed when the interfaces are.
   * \param width Width of the image
   * \param height Height of the image
   * \param buffer Data containing initial data
   * \param destroy Destroy the buffer when the Image is destroyed
   * \param format Image format. Data in \arg buffer must be in this format.
   * Default: #CS_IMGFMT_TRUECOLOR
   * \param palette Palette for indexed images.
   */
  csImageMemory (int width, int height, void *buffer, bool destroy,
    int format = CS_IMGFMT_TRUECOLOR, csRGBpixel *palette = 0);

  virtual ~csImageMemory ();

  /// Clears image to colour. Only works for truecolor images.
  void Clear (const csRGBpixel &colour);

  /// Rescale the image to the given size
  virtual void Rescale (int NewWidth, int NewHeight);

  /// Set the keycolor
  virtual void SetKeycolor (int r, int g, int b);
  /// Remove the keycolor
  virtual void ClearKeycolor ();

  /**
   * Apply the keycolor, that is, set all pixels which match the
   * keycolor to 0.
   */
  virtual void ApplyKeycolor ();
};


#endif // __CS_MEMIMAGE_H__
