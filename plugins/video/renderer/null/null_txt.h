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

#ifndef __NULL_TXT_H__
#define __NULL_TXT_H__

#include "video/renderer/common/txtmgr.h"
#include "igraphic/image.h"

class csTextureManagerNull;

/**
 * In 8-bit modes we build a 32K inverse colormap for converting
 * RGB values into palette indices. The following macros defines
 * the number of bits to use for encoding R, G and B values.
 */
#define RGB2PAL_BITS_R	5
#define RGB2PAL_BITS_G	5
#define RGB2PAL_BITS_B	5

/// The prefered distances to use for the color matching.
#define PREFERED_DIST	2000000

/**
 * A class containing a colormap. A object of this class is used
 * for the global colormap in 8-bit modes.
 */
class csColorMapLine
{
public:
  /**
   * Array with RGB values for every color of the palette.
   * An RGB value has four bytes: R, G, B, unused.
   * So there are 256*4 bytes in this array.
   */
  csRGBpixel palette [256];

  /// Colors which are allocated.
  bool alloc [256];

  /// Constructor
  csColorMapLine ()
  { memset (alloc, sizeof (alloc), 0); }

  /// Find a value in the colormap and return the color index.
  int find_rgb (int r, int g, int b, int *d = NULL);

  /// Allocate a new RGB color in the colormap.
  int alloc_rgb (int r, int g, int b, int dist);

  /// Get a palette entry
  inline csRGBpixel &operator [] (int idx)
  { return palette [idx]; }

  /// Compute number of free palette entries
  int FreeEntries ();
};

/**
 * csTextureNull is a class derived from csTexture that implements
 * all the additional functionality required by the software renderer.
 * Every csTextureSoftware is a 8-bit paletted image with a private
 * colormap. The private colormap is common for all mipmapped variants.
 * The colormap is stored inside the parent csTextureHandle object.
 */
class csTextureNull : public csTexture
{
public:
  /// The bitmap
  UByte *bitmap;
  /// The alpha map (NULL if no alphamap)
  uint8 *alphamap;
  /// The image (temporary storage)
  iImage *image;

  /// Create a csTexture object
  csTextureNull (csTextureHandle *Parent, iImage *Image) : csTexture (Parent)
  {
    bitmap = NULL;
    alphamap = NULL;
    image = Image;
    w = Image->GetWidth ();
    h = Image->GetHeight ();
    compute_masks ();
  }
  /// Destroy the texture
  virtual ~csTextureNull ()
  { delete [] bitmap; delete [] alphamap; if (image) image->DecRef (); }
  /// Return a pointer to texture data
  UByte *get_bitmap ()
  { return bitmap; }
  /// Return a pointer to alpha map data
  uint8 *get_alphamap ()
  { return alphamap; }
};

/**
 * csTextureHandleNull represents a texture and all its mipmapped
 * variants.
 */
class csTextureHandleNull : public csTextureHandle
{
protected:
  /**
   * Private colormap -> global colormap table
   * For 16- and 32-bit modes this array contains a 256-element array
   * of either shorts or longs to convert any image pixel from 8-bit
   * paletted format to the native pixel format.
   */
  void *pal2glob;

  /// Same but for 8-bit modes (8bit to 16bit values)
  uint16 *pal2glob8;

  /// The private palette
  csRGBpixel palette [256];

  /// Number of used colors in palette
  int palette_size;

  /// The texture manager
  csTextureManagerNull *texman;

  /// Create a new texture object
  virtual csTexture *NewTexture (iImage *Image);

  /// Compute the mean color for the just-created texture
  virtual void ComputeMeanColor ();

public:
  /// Create the mipmapped texture object
  csTextureHandleNull (csTextureManagerNull *txtmgr, iImage *image, int flags);
  /// Destroy the object and free all associated storage
  virtual ~csTextureHandleNull ();

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
  /// Query palette -> 16-bit values table for 8-bit modes
  uint16 *GetPaletteToGlobal8 () { return pal2glob8; }

  /**
   * Query if the texture has an alpha channel.<p>
   * This depends both on whenever the original image had an alpha channel
   * and of the fact whenever the renderer supports alpha maps at all.
   */
  virtual bool GetAlphaMap ()
  { return !!((csTextureNull *)get_texture (0))->get_alphamap (); }

  /// Prepare the texture for usage
  virtual void Prepare ();
};

/**
 * Software version of the texture manager. This instance of the
 * texture manager is probably the most involved of all 3D rasterizer
 * specific texture manager implementations because it needs to do
 * a lot of work regarding palette management and the creation
 * of lots of lookup tables.
 */
class csTextureManagerNull : public csTextureManager
{
private:
  /// How strong texture manager should push 128 colors towards a uniform palette
  int uniform_bias;

  /// Which colors are locked in the global colormap
  bool locked [256];

  /// true if palette has already been computed
  bool palette_ok;

  /// True if truecolor mode/false if paletted mode
  bool truecolor;

  /// Configuration values for color matching.
  int prefered_dist;

  /// We need a pointer to the 2D driver
  iGraphics2D *G2D;

public:
  /// The global colormap (used in 256-color modes)
  csColorMapLine cmap;

  /// The inverse colormap (for 8-bit modes)
  uint8 *inv_cmap;

  /// The global colormap (for 8-bit modes)
  uint16 *GlobalCMap;

  ///
  csTextureManagerNull (iSystem *iSys, iGraphics2D *iG2D, iConfigFile *config);
  ///
  virtual ~csTextureManagerNull ();

  /// Called from G3D::Open ()
  void SetPixelFormat (csPixelFormat &PixelFormat);

  /// Encode RGB values to a 16-bit word (for 16-bit mode).
  ULong encode_rgb (int r, int g, int b);

  /**
   * Create the inverse colormap. The forward colormap
   * must be created before this can be used.
   */
  void create_inv_cmap ();

  /// Find an rgb value using the faster lookup tables.
  int find_rgb (int r, int g, int b);

  /**
   * Compute the 'best' palette for all loaded textures.
   * This function will exactly behave the same in 16-bit mode
   * since we still need the common 256-color palette (although
   * it will not be installed on the display).
   */
  void compute_palette ();

  /// Read configuration values from config file.
  virtual void read_config (iConfigFile *config);

  ///
  virtual void Clear ();

  /**
   * Return the index for some color. This works in 8-bit
   * (returns an index in the 256-color table), in 15/16-bit
   * (returns a 15/16-bit encoded RGB value) and in 32-bit
   * modes as well.
   */
  virtual int FindRGB (int r, int g, int b);

  ///
  virtual void PrepareTextures ();
  ///
  virtual iTextureHandle *RegisterTexture (iImage* image, int flags);
  ///
  virtual void UnregisterTexture (csTextureHandleNull* handle);
  /// Clear the palette (including all reserved colors)
  virtual void ResetPalette ();
  /// Reserve a color in palette (if any)
  virtual void ReserveColor (int r, int g, int b);
  /// Really allocate the palette on the system.
  virtual void SetPalette ();
};

#endif // __NULL_TXT_H__
