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

#ifndef __SOFT_TXT_H__
#define __SOFT_TXT_H__

#include "csutil/debug.h"
#include "video/renderer/common/txtmgr.h"
#include "igraphic/image.h"
#include "ivideo/graph2d.h"

class csGraphics3DSoftwareCommon;
class csTextureManagerSoftware;
class csTextureHandleSoftware;
class csSoftProcTexture3D;
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
  uint8 alpha_map50 [256*256];
  /// Alpha table for 25% and 75%
  uint8 alpha_map25 [256*256];
};

/// The prefered distances to use for the color matching.
#define PREFERED_DIST	2000000

/**
 * A class containing a colormap. A object of this class is used
 * for the global colormap in 8-bit modes.
 */
class csColorMap
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
  csColorMap ()
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
 * csTextureSoftware is a class derived from csTexture that implements
 * all the additional functionality required by the software renderer.
 * Every csTextureSoftware is a 8-bit paletted image with a private
 * colormap. The private colormap is common for all mipmapped variants.
 * The colormap is stored inside the parent csTextureHandle object.
 */
class csTextureSoftware : public csTexture
{
public:
  /// The bitmap
  uint8 *bitmap;
  /// The alpha map (NULL if no alphamap)
  uint8 *alphamap;
  /// The image (temporary storage)
  iImage *image;

  /// Create a csTexture object
  csTextureSoftware (csTextureHandle *Parent, iImage *Image) : csTexture (Parent)
  {
    bitmap = NULL;
    alphamap = NULL;
    image = Image;
    DG_LINK (this, image);
    image->IncRef ();
    w = Image->GetWidth ();
    h = Image->GetHeight ();
    compute_masks ();
  }
  /// Destroy the texture
  virtual ~csTextureSoftware ()
  {
    delete [] bitmap;
    delete [] alphamap;
    if (image) image->DecRef ();
  }

  /// Return a pointer to texture data
  uint8 *get_bitmap ()
  { return bitmap; }
  /// Return a pointer to alpha map data
  uint8 *get_alphamap ()
  { return alphamap; }
};

/**
 * csTextureSoftwareProc is a class derived from csTextureSoftware that
 * implements the additional functionality to allow acess to the texture
 * memory via iGraphics2D/3D interfaces. Internally iGraphics2D/3D are
 * initialised in 8bit mode and use the palette as calculated by
 * csTextureHandleSoftware, so you can render to them as usual without
 * requiring recalculation of palettes each frame.
 */
class csTextureSoftwareProc : public csTextureSoftware
{
public:
  csSoftProcTexture3D *texG3D;
  bool proc_ok;

  csTextureSoftwareProc (csTextureHandle *Parent, iImage *Image)
    : csTextureSoftware (Parent, Image), texG3D(NULL), proc_ok(false)
  {};
  /// Destroy the texture
  virtual ~csTextureSoftwareProc ();
};


/**
 * csTextureHandleSoftware represents a texture and all its mipmapped
 * variants.
 */
class csTextureHandleSoftware : public csTextureHandle
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

  /// The private palette for this texture (compressed a little)
  uint8 *orig_palette;

  /// The private palette (with gamma applied)
  csRGBpixel palette [256];

  /// Number of used colors in palette
  int palette_size;

  /// Create a new texture object
  virtual csTexture *NewTexture (iImage *Image, bool ismipmap);

  /// Compute the mean color for the just-created texture
  virtual void ComputeMeanColor ();

  /// Helper function: Initialises according to given palette
  void SetupFromPalette ();

  /// the texture manager
  csTextureManagerSoftware *texman;

public:
  /// Create the mipmapped texture object
  csTextureHandleSoftware (csTextureManagerSoftware *texman, iImage *image,
		       int flags);
  /// Destroy the object and free all associated storage
  virtual ~csTextureHandleSoftware ();

  /**
   * Create the [Private colormap] -> global colormap table.
   * In 256-color modes we find the correspondense between private texture
   * colormap and the global colormap; in truecolor modes we just build
   * a [color index] -> [truecolor value] conversion table.
   */
  void remap_texture ();

  /// Query the private texture colormap
  csRGBpixel *GetColorMap () { return palette; }
  /// Query the number of colors in the colormap
  int GetColorMapSize () { return palette_size; }
  /// Get original colormap (packed)
  void GetOriginalColormap (csRGBpixel *oPalette, int &oCount);

  /// Query palette -> native format table
  void *GetPaletteToGlobal () { return pal2glob; }

  /// Query palette -> 16-bit values table for 8-bit modes
  uint16 *GetPaletteToGlobal8 () { return pal2glob8; }

  /// Apply gamma correction to private palette
  void ApplyGamma (uint8 *GammaTable);

  /**
   * Query if the texture has an alpha channel.<p>
   * This depends both on whenever the original image had an alpha channel
   * and of the fact whenever the renderer supports alpha maps at all.
   */
  virtual bool GetAlphaMap ()
  { return !!((csTextureSoftware *)get_texture (0))->get_alphamap (); }

  /**
   * Merge this texture into current palette, compute mipmaps and so on.
   * You should call either Prepare() or iTextureManager::PrepareTextures()
   * before using any texture.
   */
  virtual void Prepare ();

  //-------------------------------Procedural Texture support--------------------
  /// Return the interfaces to the procedural texture buffer
  virtual iGraphics3D *GetProcTextureInterface ();

  virtual void ProcTextureSync ();

  /**
   * Called when the procedural texture shares texture manager with parent
   * context and in either 16 or 32bit. The 8bit version doesn't require
   * repreparing. Recalculates palette and remaps texture.
   */
  void ReprepareProcTexture ();

  /**
   * Remaps 8bit procedural texture to the global palette as calculated either
   * by a dedicated 8bit texture manager or the main texture manager in 8bit
   * display mode
   */
  void RemapProcToGlobalPalette (csTextureManagerSoftware *txtmgr);
};

/**
 * Software version of the texture manager. This instance of the
 * texture manager is probably the most involved of all 3D rasterizer
 * specific texture manager implementations because it needs to do
 * a lot of work regarding palette management and the creation
 * of lots of lookup tables.
 */
class csTextureManagerSoftware : public csTextureManager
{
  friend class csTextureHandleSoftware;
private:
  /// How strong texture manager should push 128 colors towards
  /// a uniform palette
  int uniform_bias;

  /// Which colors are locked in the global colormap
  bool locked [256];

  /// true if palette has already been computed
  bool palette_ok;

  /// True if truecolor mode/false if paletted mode
  bool truecolor;

  /// Configuration values for color matching.
  int prefered_dist;

public:

  /// We need a pointer to the 3D driver
  csGraphics3DSoftwareCommon *G3D;

  /// Apply dithering to textures while reducing from 24-bit to 8-bit paletted?
  bool dither_textures;

  /// Sharpen mipmaps?
  int sharpen_mipmaps;

  /// The global colormap (used in 256-color modes)
  csColorMap cmap;

  /// Lookup table (8-bit modes)
  csAlphaTables *alpha_tables;

  /// The multiplication tables used for lightmapping
  uint8 *lightmap_tables [3];

  /// The translation table for applying gamma
  uint8 GammaTable [256];

  /// Texture gamma
  float Gamma;

  ///
  csTextureManagerSoftware (iObjectRegistry *object_reg,
  			    csGraphics3DSoftwareCommon *iG3D,
			    iConfigFile *config);
  ///
  virtual ~csTextureManagerSoftware ();

  /// Called from G3D::Open ()
  void SetPixelFormat (csPixelFormat &PixelFormat);

  /// Encode RGB values to a 16-bit word (for 16-bit mode).
  uint32 encode_rgb (int r, int g, int b);

  /**
   * Create the inverse colormap. The forward colormap
   * must be created before this can be used.
   */
  void create_inv_cmap ();

  /// Create the alpha tables.
  void create_alpha_tables ();

  /// Find an rgb value using the faster lookup tables.
  int find_rgb (int r, int g, int b);

  /**
   * Compute the 'best' palette for all loaded textures.
   * This function will exactly behave the same in 16-bit mode
   * since we still need the common 256-color palette (although
   * it will not be installed on the display).
   */
  void compute_palette ();

  /// Called from csTextureHandleSoftware destructor to deregister before death
  void UnregisterTexture (csTextureHandleSoftware* handle);

  /// Set gamma correction value.
  void SetGamma (float iGamma);

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
  /// Clear the palette (including all reserved colors)
  virtual void ResetPalette ();
  /// Reserve a color in palette (if any)
  virtual void ReserveColor (int r, int g, int b);
  /// Really allocate the palette on the system.
  virtual void SetPalette ();

  //-----------------------Procedural Texture Support--------------------------
  /**
   * When in 16/32 bit mode and a dedicated procedural texture manager is present
   * the main texture manager needs to keep track of the first 8bit procedural
   * texture
   */
  csGraphics3DSoftwareCommon *first_8bit_proc_tex;

  /// Retrieve first 8bit Procedural texture registered while in true colour mode
  csGraphics3DSoftwareCommon *GetFirst8bitProcTexture ()
  { return (csGraphics3DSoftwareCommon *)first_8bit_proc_tex; }

  ///
  void SetMainTextureManager (csTextureManagerSoftware *main_txt_mgr)
  { main_txtmgr = main_txt_mgr; }
  ///
  void SetProcTextureManager (csTextureManagerSoftware *proc_txt_mgr)
  { proc_txtmgr =  proc_txt_mgr; }
  ///
  csTextureManagerSoftware *GetMainTextureManager () const
  { return main_txtmgr; }
  ///
  csTextureManagerSoftware *GetProcTextureManager () const
  { return proc_txtmgr; }

protected:
  /**
   * When in 16/32bit mode and a dedicated 8bit procedural texture manager is
   * present, its' textures are remapped to the global palette.
   */
  void Reprepare8BitProcs ();

  ///The dedicated procedural txtmgr (if there is one)
  csTextureManagerSoftware *proc_txtmgr;

  /// The main txtmgr (if this is the dedicated procedural texture manager)
  csTextureManagerSoftware *main_txtmgr;
};

#endif // __SOFT_TXT_H__
