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

#ifndef __CS_SCRSHOT_H__
#define __CS_SCRSHOT_H__

#include "ivideo/graph2d.h"
#include "igraphic/image.h"

class csScreenShot : public iImage
{
  int Format;
  void *Data;
  csRGBpixel *Palette;
  int Width, Height;
public:
  SCF_DECLARE_IBASE;
  /// Initialize the screenshot object
  csScreenShot (iGraphics2D *G2D);
  /// Destroy the screenshot object
  virtual ~csScreenShot ();
  /// Get a pointer to image data
  virtual void *GetImageData ()
  { return Data; }
  /// Query image width
  virtual int GetWidth () const
  { return Width; }
  /// Query image height
  virtual int GetHeight () const
  { return Height; }
  /// Query image size in bytes
  virtual int GetSize () const { return 0; }
  /// Rescale the image to the given size
  virtual void Rescale (int NewWidth, int NewHeight)
  { (void) NewWidth; (void) NewHeight; }
  /// Create a new iImage which is a mipmapped version of this one.
  virtual csPtr<iImage> MipMap (int step, csRGBpixel *transp)
  { (void)step; (void)transp; return 0; }
  /// Set image file name
  virtual void SetName (const char *iName)
  { (void) iName; }
  /// Get image file name
  virtual const char *GetName () const
  { return 0; }
  /// Qyery image format (see CS_IMGFMT_XXX above)
  virtual int GetFormat () const
  { return Format; }
  /// Get image palette (or 0 if no palette)
  virtual csRGBpixel *GetPalette ()
  { return Palette; }
  /// Get alpha map for 8-bit paletted image.
  virtual uint8 *GetAlpha ()
  { return 0; }
  /// Change image format
  virtual void SetFormat (int /*iFormat*/)
  { }
  /// Create yet another image and copy this one into the new image.
  virtual csPtr<iImage> Clone () const
  { return 0; }
  /// Create another image holding a subimage of current image
  virtual csPtr<iImage> Crop (int, int, int, int) const
  { return 0; }
  /// Check if the alpha channel is valid
  virtual void CheckAlpha ()
  { }
  /// Copy an image as subpart in the actual Image.
  virtual bool Copy (iImage* sImage, int x, int y, int width, int height) const
  { return false; }
  /**
   * Copy an image as subpart in the actual Image and scale it to
   * the given size.
   */
  virtual bool CopyScale (iImage* sImage, int x, int y,
  	int width, int height) const
  { return false; }
  /**
   * Copy an image as subpart in the actual Image and tile and scale it
   * to the given size.
   */
  virtual bool CopyTile (iImage* sImage, int x, int y,
  	int width, int height) const 
  { return false; }


  /// Check if image has a keycolour stored with it.
  virtual bool HasKeyColor () const
  { return 0; }
  virtual bool HasKeycolor () const
  { return HasKeyColor(); }
  /// Get the keycolour stored with the image.
  virtual void GetKeyColor (int &r, int &g, int &b) const
  { r=0;g=0;b=0; }
  virtual void GetKeycolor (int &r, int &g, int &b) const
  { GetKeyColor(r,g,b); }
  /// Sharpen the image.
  virtual csPtr<iImage> Sharpen (csRGBpixel *transp, int strength) const
  { transp = 0; strength = 0; return 0; }
  /// Has mipmaps.
  virtual int HasMipmaps () const
  { return 0; }
};

#endif // __CS_SCRSHOT_H__

