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

#ifndef TXTMGR_SOFT_H
#define TXTMGR_SOFT_H

#include "cscom/com.h"
#include "cs3d/common/txtmgr.h"
#include "itexture.h"

class csTextureMMSoftware;
class csTextureManagerSoftware;
struct HighColorCache_Data;
interface IImageFile;

/// The internal texture mapping modes.
#define TXT_GLOBAL 0    // Textures are mapped with a single palette
#define TXT_PRIVATE 1   // Every texture has it's own 256-color palette
#define TXT_24BIT 2     // Every texture is represented in 24-bits (RGB)

// Colors are encoded in a 16-bit short using the following
// distribution (only for 8-bit mode):
#define BITS_RED 6
#define BITS_GREEN 6
#define BITS_BLUE 4
#define MASK_RED ((1<<BITS_RED)-1)
#define MASK_GREEN ((1<<BITS_GREEN)-1)
#define MASK_BLUE ((1<<BITS_BLUE)-1)
#define NUM_RED (1<<BITS_RED)
#define NUM_GREEN (1<<BITS_GREEN)
#define NUM_BLUE (1<<BITS_BLUE)

#define TABLE_WHITE 0
#define TABLE_RED 1
#define TABLE_GREEN 2
#define TABLE_BLUE 3
#define TABLE_WHITE_HI 4
#define TABLE_RED_HI 5
#define TABLE_GREEN_HI 6
#define TABLE_BLUE_HI 7


/**
 * Define a small (3 pixels) margin at the bottom and top of
 * the texture. This is the easiest way I could find to 'fix' the
 * overflow problems with the texture mapper.
 */
#define H_MARGIN 3

typedef UShort RGB16map[256];
typedef unsigned char RGB8map[256];

/**
 * Lookup table entry corresponding to one palette entry.
 * 'red', 'green', and 'blue' are tables giving the red, green,
 * and blue components for all light levels of that palette index.
 */
struct PalIdxLookup
{
  RGB16map red;
  RGB16map green;
  RGB16map blue;
};

/**
 * Lookup table class for true_rgb mode
 * (used for display output of 8 and 16-bit and texture width
 * of 8-bit and with true_rgb mode).
 * The tables in this class convert an 8-bit palette index
 * to a red, green, or blue 16-bit value (using the specific
 * mask for each color). The result of these three tables
 * can then be or'ed together for a 16-bit truecolor value
 * which can then in turn be translated (using another lookup
 * table) to 8-bit if needed.
 */
struct TextureTablesTrueRgb
{
  /// Lookup table.
  PalIdxLookup lut[256];
};

/**
 * Lookup table class for true_rgb mode and TXT_PRIVATE textures.
 * (used for display output of 8 and 16-bit and texture width
 * of 8-bit/private colormap and with true_rgb mode).
 * The tables in this class convert an 8-bit color component
 * to a red, green, or blue 16-bit value (using the specific
 * mask for each color). The result of these three tables
 * can then be or'ed together for a 16-bit truecolor value
 * which can then in turn be translated (using another lookup
 * table) to 8-bit if needed.
 */
struct TextureTablesTrueRgbPriv
{
  /// Lookup table.
  PalIdxLookup lut[256];
};

/**
 * Two tables with white light. They add white light to a palette
 * index and return a 16-bit truecolor value.
 * (used for display output of 16-bit and texture width of 8-bit).
 */
struct TextureTablesWhite16
{
  /// White light table with index 0 as default (no light added).
  RGB16map white1_light[256];
  /// White light table with index NORMAL_LIGHT_LEVEL as default (no light added).
  RGB16map white2_light[256];
};

/**
 * Two tables with white light. They add white light to a palette
 * index and return a new 8-bit palette index.
 * (used for display output of 8-bit and texture width of 8-bit).
 */
struct TextureTablesWhite8
{
  /// White light table with index 0 as default (no light added).
  RGB8map white1_light[256];
  /// White light table with index NORMAL_LIGHT_LEVEL as default (no light added).
  RGB8map white2_light[256];
};

/**
 * Lookup table to convert a 16-bit truecolor value to
 * an 8-bit palette index and the other way around.
 * (used for display output of 8-bit/16-bit and texture width of 8-bit).
 */
struct TextureTablesPalette
{
  /// Lookup table for true_rgb mode.
  unsigned char true_to_pal[65536];
  /// From palette to truecolor value.
  UShort pal_to_true[256];
};

/**
 * Lookup table for alpha mapping. Converts two palette entries
 * to a new palette entry (alpha blended).
 * (used for display output of 8-bit).
 */
struct TextureTablesAlpha
{
  /// Alpha table for 50%
  RGB8map alpha_map50[256];
  /// Alpha table for 25% and 75%
  RGB8map alpha_map25[256];
};

/// The prefered distances to use for the color matching.
#define PREFERED_DIST 16333
#define PREFERED_COL_DIST 133333

#define R24(rgb) (((rgb)>>16)&0xff)
#define G24(rgb) (((rgb)>>8)&0xff)
#define B24(rgb) ((rgb)&0xff)

/**
 * A class containing the private colormap and translation
 * to the global colormap for a single texture. This class
 * is only used in TXT_PRIVATE mode (private colormap for
 * every texture).
 */
class TxtCmapPrivate
{
  friend class csTextureMMSoftware;
  friend interface ITextureHandle;

private:
  /**
   * Array with RGB values for every color of the palette.
   * An RGB value has four bytes: R, G, B, unused.
   * So there are 256*4 bytes in this array.
   */
  unsigned char rgb_values[256*4];

  /**
   * Array which maps the private palette index to the global
   * palette index.
   */
  unsigned char priv_to_global[256];

  /// Colors which are allocated.
  unsigned char alloc[256];

private:
  /// Constructor
  TxtCmapPrivate ();

  /**
   * Find an RGB value from the private map and return the
   * index in the private colormap.
   */
  int find_rgb (int r, int g, int b);

  /**
   * Allocate a new RGB color in the private colormap.
   */
  int alloc_rgb (int r, int g, int b, int dist);
};

/**
 * csTextureMMSoftware represents a texture and all its mipmapped
 * variants.
 */
class csTextureMMSoftware : public csTextureMM
{
protected:
  /// Private colormap if needed.
  TxtCmapPrivate* priv_cmap;
 
  /// Convert ImageFile to internal format.
  virtual void convert_to_internal (csTextureManager* tex, IImageFile* imfile, unsigned char* bm);
  void convert_to_internal_global (csTextureManagerSoftware* tex, IImageFile* imfile, unsigned char* bm);
  void convert_to_internal_24bit (csTextureManagerSoftware* tex, IImageFile* imfile, unsigned char* bm);
  void convert_to_internal_private (csTextureManagerSoftware* tex, IImageFile* imfile, unsigned char* bm);

public:
  ///
  csTextureMMSoftware (IImageFile* image);
  ///
  virtual ~csTextureMMSoftware ();

  /// Get the private colormap if any.
  unsigned char* get_colormap_private () { return priv_cmap->rgb_values; }

  /// Get the conversion table from private to global colormap if any.
  unsigned char* get_private_to_global () { return priv_cmap->priv_to_global; }

  /**
   * Remap the palette of this texture according to the internal
   * texture mode.
   */
  virtual void remap_texture (csTextureManager* new_palette);

  /**
   * Remap the palette of this texture using the given global
   * palette (inside csTextureManagerSoftware).
   * If do_2d is true then this remapping is done on the 2d driver texture.
   */
  void remap_palette_global (csTextureManagerSoftware* new_palette, bool do_2d = false);

  /**
   * Remap the palette of this texture using a private
   * palette and make a mapping of this palette to RGB
   * and to the global palette (inside csTextureManagerSoftware).
   */
  void remap_palette_private (csTextureManagerSoftware* new_palette);

  /**
   * This function does not really remap but it converts
   * the format to an ULong format suitable for 24-bit
   * internal texture format.
   */
  void remap_palette_24bit (csTextureManagerSoftware* new_palette);

  /**
   * Remap the 2d texture to 16-bit display format.
   */
  void remap_texture_16 (csTextureManagerSoftware* new_palette);

  /**
   * Remap the 2d texture to 32-bit display format.
   */
  void remap_texture_32 (csTextureManagerSoftware* new_palette);
};

/**
 * Software version of the texture manager. This
 * instance of the texture manager is probably the most involved
 * of all 3D rasterizer specific texture manager implementations because
 * it needs to do a lot of work regarding palette management and the
 * creation of lots of lookup tables.
 */
class csTextureManagerSoftware : public csTextureManager
{
private:
  int num_red, num_green, num_blue;

  /// True if truecolor mode is enabled.
  bool truecolor;

  /**
   * The shared palette. This palette is also used in truecolor (16-bit) mode.
   * It then contains the single palette that is shared by all textures.
   */
  RGBcolor pal[256];
  /// Which colors are allocated and which are not?
  int alloc[256];

  /// Configuration values for color matching.
  int prefered_dist;
  /// Configuration values for color matching.
  int prefered_col_dist;

  /// Read configuration values from config file.
  void read_config ();

  /**
   * Encode RGB values to a 16-bit word (for 16-bit mode).
   */
  ULong encode_rgb (int r, int g, int b);

  /**
   * Encode RGB values to 16-bit word (safe mode).
   */
  ULong encode_rgb_safe (int r, int g, int b);

  /**
   * Create the palette lookup table. The colormap must
   * be created before this can be used.
   */
  void create_lt_palette ();

  /**
   * Create the lookup tables for the white tables in 16-bit display
   * mode.
   */
  void create_lt_white16 ();

  /**
   * Create the lookup tables for the white tables in 8-bit display mode.
   * The tables are cached if needed (with the name 'table_white8').
   */
  void create_lt_white8 ();

  /**
   * Create the truergb 16-bit tables.
   */
  void create_lt_truergb ();

  /**
   * Create the truergb 16-bit tables for TXT_PRIVATE mode.
   */
  void create_lt_truergb_private ();

  /**
   * Create the alpha tables.
   * The tables are cached if needed (with the name 'table_alpha').
   */
  void create_lt_alpha ();

  ///
  csTexture* get_texture (int idx, int lev);

  /**
   * Find rgb for a specific map type and apply an intensity.
   * 'map_type' is one of TABLE_....
   */
  int find_rgb_map (int r, int g, int b, int map_type, int l);

public:
  /// For optimization: points to table in lt_pal.
  unsigned char* lt_palette_table;
  /// For optimization: points to tables in lt_truergb or in lt_truergb_priv.
  PalIdxLookup* lt_light;

  /// Lookup table.
  TextureTablesTrueRgb* lt_truergb;
  /// Lookup table.
  TextureTablesTrueRgbPriv* lt_truergb_private;
  /// Lookup table.
  TextureTablesWhite16* lt_white16;
  /// Lookup table.
  TextureTablesWhite8* lt_white8;
  /// Lookup table.
  TextureTablesPalette* lt_pal;
  /// Lookup table.
  TextureTablesAlpha* lt_alpha;

  /// Alpha mask used for 16-bit mode.
  static UShort alpha_mask;

  /// How are texture represented internally.
  int txtMode;
  /// Force value (set by commandline) (-1 = no force)
  int force_txtMode;

  ///
  csTextureManagerSoftware (ISystem* piSystem, IGraphics2D* piG2D);
  ///
  virtual ~csTextureManagerSoftware ();
  ///
  virtual void InitSystem ();

  ///
  virtual void clear ();

  ///
  STDMETHODIMP Initialize ();
  ///
  STDMETHODIMP Prepare ();
  ///
  STDMETHODIMP RegisterTexture (IImageFile* image, ITextureHandle** handle, bool for3d, bool for2d);
  ///
  STDMETHODIMP UnregisterTexture (ITextureHandle* handle);
  ///
  STDMETHODIMP MergeTexture (ITextureHandle* handle);
  ///
  STDMETHODIMP FreeImages ();
  ///
  STDMETHODIMP ReserveColor (int r, int g, int b, bool privcolor);
  ///
  STDMETHODIMP AllocPalette ();

  /// Create a new texture.
  csTextureMMSoftware* new_texture (IImageFile* image);

  /**
   * Find an rgb value using the palette directly (not use
   * the faster lookup tables).
   */
  int find_rgb_slow (int r, int g, int b);

  /**
   * Allocate a new RGB color.
   */
  int alloc_rgb (int r, int g, int b, int dist);

  ///
  bool force_mixing (char* mix);

  ///
  bool force_txtmode (char* txtmode);

  /**
   * Find an rgb value using the faster lookup tables.
   */
  int find_rgb (int r, int g, int b);

  /**
   * Return the index for some color. This works in 8-bit
   * (returns an index in the 256-color table) and in 15/16-bit
   * (returns a 15/16-bit encoded RGB value).
   */
  virtual int find_color (int r, int g, int b);

  /**
   * This version of find_rgb finds some r,g,b value AFTER gamma
   * correction is applied. This is useful for console messages
   * that always need the same color regardless of the gamma
   * correction.
   */
  int find_rgb_real (int r, int g, int b);

  /**
   * Compute the 'best' palette for all loaded textures.
   * This function will exactly behave the same in 16-bit mode
   * since we still need the common 256-color palette (although
   * it will not be installed on the display).
   */
  void compute_palette ();

  /**
   * Compute the light tables using the previously compute palette.
   */
  void compute_light_tables ();

  /**
   * Really allocate the palette on the system.
   */
  void alloc_palette ();

  /**
   * Get the palette.
   */
  RGBcolor* get_palette () { return pal; }

  /**
   * GAC: Use a slower method of mixing lights that produces a better result
   * Accepts 16.16 rgb values and a palette entry
   * Returns palette entry of color with lights added
   * Assumes lights are RGB format (Not WRB).
   */
  int mix_lights (int r, int g, int b, int p)
  {
    PalIdxLookup* pil = lt_light+p;
    return lt_palette_table[pil->red[r>>16] |
			   pil->green[g>>16] |
			   pil->blue[b>>16]];
  }

  /**
   * Add some light to some light component. This is used for TXT_PRIVATE
   * mode. This returns a 16-bit truecolor component for the specific color.
   */
  int add_light_red_private (int r, int l)
  {
    return lt_light[r].red[l>>16];
  }
  ///
  int add_light_green_private (int g, int l)
  {
    return lt_light[g].green[l>>16];
  }
  ///
  int add_light_blue_private (int b, int l)
  {
    return lt_light[b].blue[l>>16];
  }

  /**
   * Version of mix_lights for 16-bit mode. This is slightly faster since
   * the last lookup is not needed.
   */
  UShort mix_lights_16 (int r, int g, int b, int p)
  {
    PalIdxLookup* pil = lt_light+p;
    return pil->red[r>>16] |
	   pil->green[g>>16] |
	   pil->blue[b>>16];
  }

  /// Query the "almost-black" color used for opaque black in transparent textures
  int get_almost_black ();
};


#endif // TXTMGR_SOFT_H

