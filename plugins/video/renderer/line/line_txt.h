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

#ifndef __LINE_TXT_H__
#define __LINE_TXT_H__

#include "video/renderer/common/txtmgr.h"
#include "igraphic/image.h"

class csTextureManagerLine;

/**
 * csTextureHandleLine represents a texture and all its mipmapped
 * variants.
 */
class csTextureHandleLine : public csTextureHandle
{
protected:
  /**
   * Private colormap -> global colormap table
   * For 16- and 32-bit modes this array contains a 256-element array
   * of either shorts or longs to convert any image pixel from 8-bit
   * paletted format to the native pixel format.
   */
  void *pal2glob;

  /// The private palette
  csRGBpixel palette [256];

  /// Number of used colors in palette
  int palette_size;

  /// The texture manager
  csTextureManagerLine *texman;

  /// Create a new texture object
  virtual csTexture *NewTexture (iImage *Image, bool ismipmap);

  /// Compute the mean color for the just-created texture
  virtual void ComputeMeanColor ();

public:
  /// Create the mipmapped texture object
  csTextureHandleLine (csTextureManagerLine *txtmgr, iImage *image, int flags);
  /// Destroy the object and free all associated storage
  virtual ~csTextureHandleLine ();

  /**
   * Create the [Private colormap] -> global colormap table.
   * In 256-color modes we find the correspondense between private texture
   * colormap and the global colormap; in truecolor modes we just build
   * a [color index] -> [truecolor value] conversion table.
   */
  void remap_texture (csTextureManager *texman);

  /// Query the private texture colormap
  csRGBpixel *GetColorMap () { return palette; }
  /// Query the number of colors in the colormap
  int GetColorMapSize () { return palette_size; }

  /// Query palette -> native format table
  void *GetPaletteToGlobal () { return pal2glob; }

  /// Prepare the texture for usage
  virtual void Prepare ();
};

/**
 * csTextureLine is a class derived from csTexture that implements
 * all the additional functionality required by the software renderer.
 * Every csTextureSoftware is a 8-bit paletted image with a private
 * colormap. The private colormap is common for all mipmapped variants.
 * The colormap is stored inside the parent csTextureHandle object.
 */
class csTextureLine : public csTexture
{
public:
  /// The bitmap
  uint8 *bitmap;
  /// The image (temporary storage)
  iImage *image;

  /// Create a csTexture object
  csTextureLine (csTextureHandle *Parent, iImage *Image) : csTexture (Parent)
  {
    bitmap = NULL;
    image = Image;
    w = Image->GetWidth ();
    h = Image->GetHeight ();
    compute_masks ();
  }
  /// Destroy the texture
  virtual ~csTextureLine ()
  { delete [] bitmap; if (image) image->DecRef (); }
};

/**
 * Software version of the texture manager. This instance of the
 * texture manager is probably the most involved of all 3D rasterizer
 * specific texture manager implementations because it needs to do
 * a lot of work regarding palette management and the creation
 * of lots of lookup tables.
 */
class csTextureManagerLine : public csTextureManager
{
private:
  /// We need a pointer to the 2D driver
  iGraphics2D *G2D;

public:
  /// The multiplication tables used for lightmapping
  uint8 *lightmap_tables [3];

  ///
  csTextureManagerLine (iObjectRegistry *object_reg,
  	iGraphics2D *iG2D, iConfigFile *config);
  ///
  virtual ~csTextureManagerLine ();

  /// Called from G3D::Open ()
  void SetPixelFormat (csPixelFormat &PixelFormat);

  /// Encode RGB values to a 16-bit word (for 16-bit mode).
  uint32 encode_rgb (int r, int g, int b);

  /// Read configuration values from config file.
  virtual void read_config (iConfigFile *config);

  ///
  virtual void Clear ();

  ///
  virtual void PrepareTextures ();
  ///
  virtual csPtr<iTextureHandle> RegisterTexture (iImage* image, int flags);
  ///
  virtual void UnregisterTexture (csTextureHandleLine* handle);
};

#endif // __LINE_TXT_H__
