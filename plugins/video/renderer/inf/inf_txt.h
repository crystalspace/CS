/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Texture manager for software renderer

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

#ifndef __INF_TXT_H__
#define __INF_TXT_H__

#include "cs3d/common/txtmgr.h"
#include "iimage.h"

class csTextureManagerInfinite;

/**
 * In 8-bit modes we build a 32K inverse colormap for converting
 * RGB values into palette indices. The following macros defines
 * the number of bits to use for encoding R, G and B values.
 */
#define RGB2PAL_BITS_R	5
#define RGB2PAL_BITS_G	5
#define RGB2PAL_BITS_B	5

/**
 * Lookup table for alpha mapping. Converts two palette entries
 * to a new palette entry (alpha blended).
 * (used for display output of 8-bit).
 */
struct csAlphaTables
{
  /// Alpha table for 50%
  UByte alpha_map50 [256*256];
  /// Alpha table for 25% and 75%
  UByte alpha_map25 [256*256];
};

/// The prefered distances to use for the color matching.
#define PREFERED_DIST	2000000

/**
 * csTextureMMInfinite represents a texture and all its mipmapped
 * variants.
 */
class csTextureMMInfinite : public csTextureMM
{
protected:
  /// The private palette
  RGBPixel palette [256];

  /// Number of used colors in palette
  int palette_size;

  /// Create a new texture object
  virtual csTexture *NewTexture (iImage *Image);

  /// Compute the mean color for the just-created texture
  virtual void ComputeMeanColor ();

public:
  /// Create the mipmapped texture object
  csTextureMMInfinite (iImage *image, int flags);
  /// Destroy the object and free all associated storage
  virtual ~csTextureMMInfinite ();

  /// Query the private texture colormap
  RGBPixel *GetColorMap () { return palette; }
  /// Query the number of colors in the colormap
  int GetColorMapSize () { return palette_size; }
};

/**
 * csTextureInfinite is a class derived from csTexture that implements
 * all the additional functionality required by the software renderer.
 * Every csTextureSoftware is a 8-bit paletted image with a private
 * colormap. The private colormap is common for all mipmapped variants.
 * The colormap is stored inside the parent csTextureMM object.
 */
class csTextureInfinite : public csTexture
{
public:
  /// The bitmap
  UByte *bitmap;
  /// The image (temporary storage)
  iImage *image;

  /// Create a csTexture object
  csTextureInfinite (csTextureMM *Parent, iImage *Image) : csTexture (Parent)
  {
    bitmap = NULL;
    image = Image;
    w = Image->GetWidth ();
    h = Image->GetHeight ();
    compute_masks ();
  }
  /// Destroy the texture
  virtual ~csTextureInfinite ()
  { delete [] bitmap; if (image) image->DecRef (); }
};

/**
 * Software version of the texture manager. This instance of the
 * texture manager is probably the most involved of all 3D rasterizer
 * specific texture manager implementations because it needs to do
 * a lot of work regarding palette management and the creation
 * of lots of lookup tables.
 */
class csTextureManagerInfinite : public csTextureManager
{
private:
  /// We need a pointer to the 2D driver
  iGraphics2D *G2D;

public:
  ///
  csTextureManagerInfinite (iSystem *iSys, iGraphics2D *iG2D, csIniFile *config);
  ///
  virtual ~csTextureManagerInfinite () { Clear (); }

  /// Called from G3D::Open ()
  void SetPixelFormat (csPixelFormat &PixelFormat) { pfmt = PixelFormat; }

  /// Encode RGB values to a 16-bit word (for 16-bit mode).
  ULong encode_rgb (int r, int g, int b);

  /**
   * Return the index for some color. This works in 8-bit
   * (returns an index in the 256-color table), in 15/16-bit
   * (returns a 15/16-bit encoded RGB value) and in 32-bit
   * modes as well.
   */
  virtual int FindRGB (int r, int g, int b) { return encode_rgb (r, g, b); }

  virtual void PrepareTextures ();
  virtual iTextureHandle *RegisterTexture (iImage* image, int flags);
  virtual void PrepareTexture (iTextureHandle *handle);
  virtual void UnregisterTexture (iTextureHandle* handle);
};

#endif // __INF_TXT_H__
