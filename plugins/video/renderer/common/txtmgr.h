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

#ifndef TXTMGR_H
#define TXTMGR_H

#include "cs3d/common/imgtools.h"
#include "csengine/cscolor.h"	// @@@BAD?
#include "csutil/csvector.h"
#include "itxtmgr.h"
#include "itexture.h"
#include "igraph2d.h"

class ImageColorInfo;
class csTextureMM;
class csTextureManager;
class csTexture;
struct iImage;

#define MIPMAP_NICE	0
#define MIPMAP_VERYNICE	1

/**
 * csTextureMM represents a texture and all its mipmapped
 * variants. It implements the iTextureHandle interface.
 */
class csTextureMM : public iTextureHandle
{
protected:
  /// The corresponding ImageFile.
  iImage* ifile;
  /// A sorted table with all used colors in the image.
  ImageColorInfo* usage;
  /// Transparent color
  RGBPixel transp_color;
  /// Does color 0 mean "transparent" for this texture?
  bool istransp;
  /// Mean color used when texture mapping is disabled.
  csColor mean_color;
  /// Mean color index
  int mean_idx;

  /// Is the texture to be used for the 3d rasterizer?
  bool for3d;
  /// Is the texture to be used for the 2d driver?
  bool for2d;

  /// Texture for mipmap level 0
  csTexture* t1;
  /// Texture for mipmap level 1
  csTexture* t2;
  /// Texture for mipmap level 2
  csTexture* t3;
  /// Texture for mipmap level 3
  csTexture* t4;
  /// Texture for 2D driver use
  csTexture* t2d;

  /**
   * In 32bpp we have to use the native pixel format
   * These are the shift values for R,G and B
   */
  int rs24, gs24, bs24;

  /**
   * Create a mipmapped bitmap from a previous level.
   * 'steps' is the number of steps to mipmap (only 1, 2, or 3 supported).
   */
  void create_mipmap_bitmap (csTextureManager* tex, int steps, unsigned char* bm);

  /// Create a blended mipmap with the same size as the previous level.
  void create_blended_mipmap (csTextureManager* tex, unsigned char* bm);

  /// Convert ImageFile to internal format.
  virtual void convert_to_internal (csTextureManager* tex, iImage* imfile, unsigned char* bm) = 0;

  /// Adjusts the textures size, to ensure some restrictions like power of two dimension are met.
  void AdjustSize();

public:
  ///
  csTextureMM (iImage* image);
  ///
  virtual ~csTextureMM ();

  /// Set 3d/2d usage.
  void set_3d2d (bool f3d, bool f2d) { for3d = f3d; for2d = f2d; }

  /// For 3d?
  bool for_3d () { return for3d; }

  /// For 2d?
  bool for_2d () { return for2d; }

  /// Allocate all mipmap levels for this texture.
  void alloc_mipmaps (csTextureManager* tex);

  /// Allocate the 2d texture for this texture.
  void alloc_2dtexture (csTextureManager* tex);

  /// Blend mipmap level 0.
  void blend_mipmap0 (csTextureManager* tex);

  /// Create all mipmapped bitmaps from the first level.
  void create_mipmaps (csTextureManager* tex);

  /// Free the color usage table linked to a texture.
  void free_usage_table ();

  /// Release the original image (iImage) as given by the engine.
  void free_image ();

  /// Return true if the texture has been loaded correctly.
  bool loaded_correctly () { return (ifile != NULL); }

  /// Get the mipmapped texture at some level (0..3).
  csTexture* get_texture (int lev);

  /// Set the transparent color.
  void set_transparent (int red, int green, int blue);

  ///
  int get_num_colors () { return usage->get_num_colors (); }

  /// Compute the 'usage' table.
  void compute_color_usage ();

  /**
   * Remap the texture in the best possible way. The default implementation
   * will switch only between 16Bit and 32Bit modes. That is simple, but enough 
   * for OpenGL and DirectX. Other renderers will have to override this 
   * method for appropriate results.
   */
  virtual void remap_texture (csTextureManager* new_palette);

  /**
   * This function does not really remap but it converts
   * the format to an ULong format suitable for 24-bit
   * internal texture format.
   */
  virtual void remap_palette_24bit (csTextureManager* new_palette);

  /**
   * Remap the 2d texture to 16-bit display format.
   */
  virtual void remap_texture_16 (csTextureManager* new_palette);

  /**
   * Remap the 2d texture to 32-bit display format.
   */
  virtual void remap_texture_32 (csTextureManager* new_palette);

  ///
  const RGBPalEntry& get_usage (int idx) 
  { return usage->get_color_table()[idx]; }

  ///---------------------- iTextureHandle implementation ----------------------
  DECLARE_IBASE;

  /// Set the transparent color.
  virtual void SetTransparent (int red, int green, int blue);

  /// Get the transparent status (false if no transparency, true if transparency).
  virtual bool GetTransparent ();

  /// Get the transparent color
  virtual void GetTransparent (int &red, int &green, int &blue);

  /**
   * Get the dimensions for a given mipmap level (0 to 3).
   * This function is only valid if the texture has been registered
   * for 3D usage.
   */
  virtual void GetMipMapDimensions (int mm, int& w, int& h);

  /**
   * Get the bitmap data for the given mipmap.
   * This function is not always available: it depends on implementation.
   */
  virtual void *GetMipMapData (int mm);

  /**
   * Get the dimensions for the 2D texture.
   * This function is only valid if the texture has been registered
   * for 2D usage.
   */
  virtual void GetBitmapDimensions (int& bw, int& bh);

  /**
   * Get the bitmap data for the 2D texture.
   * This function is only valid if the texture has been registered
   * for 2D usage.
   */
  virtual void *GetBitmapData ();

  /// Get the mean color.
  virtual void GetMeanColor (float& r, float& g, float& b);
  /// Get the mean color index.
  virtual int GetMeanColor () { return mean_idx; }

  /// Returns the number of colors in this texture.
  virtual int GetNumberOfColors ();

  ///
  virtual csHighColorCacheData *GetHighColorCacheData ()
  { return NULL; }
  ///
  virtual void SetHighColorCacheData (csHighColorCacheData *d)
  { (void)d; }

  /// Query whenever the texture is in texture cache
  virtual bool IsCached () { return false; }

  /// Set "in-cache" state
  virtual void SetInCache (bool InCache) { (void)InCache; }

  /// Get the csTextureMM object associated with the texture handle
  virtual void *GetPrivateObject () { return (csTextureMM *)this; }
};

/**
 * adds some methods and members needed for hardware accelerated renderers
 */
class csHardwareAcceleratedTextureMM : public csTextureMM
{
protected:
  ///
  csHighColorCacheData *hicolorcache;
  ///
  bool in_memory;

  /// Convert ImageFile to internal format. Will just convert to 24 bit in most HW renderers
  virtual void convert_to_internal (csTextureManager* tex, iImage* imfile, unsigned char* bm);

public:
  ///
  csHardwareAcceleratedTextureMM (iImage* image) : csTextureMM (image)
  { CONSTRUCT_IBASE (NULL); in_memory = false; hicolorcache = NULL;}

  ///
  virtual csHighColorCacheData *GetHighColorCacheData () { return hicolorcache; }
  ///
  virtual void SetHighColorCacheData (csHighColorCacheData *d) { hicolorcache = d; }

  /// Query whenever the texture is in texture cache
  virtual bool IsCached () { return in_memory; }

  /// Set "in-cache" state
  virtual void SetInCache (bool InCache)
  { in_memory = InCache; }
};

/**
 * A simple Texture.
 */
class csTexture
{
protected:
  ///
  csTextureMM* parent;
  ///
  int width;
  ///
  int height;
  int shf_w, shf_h;
  int and_w, and_h;
  /// Raw data.
  unsigned char* bitmap;

private:
  void init ();

protected:
  /**
   * Create a texture with a width and height.
   * This constructor is protected because Texture objects
   * should not be declared directly.
   */
  csTexture (csTextureMM* p, int w, int h);

public:
  ///
  virtual ~csTexture ();

  ///
  int get_width () { return width; }
  ///
  int get_height () { return height; }
  ///
  int get_w_shift () { return shf_w; }
  ///
  int get_h_shift () { return shf_h; }
  ///
  int get_w_mask () { return and_w; }
  ///
  int get_h_mask () { return and_h; }
  ///
  csTextureMM* get_parent () { return parent; }
  ///
  virtual unsigned char *get_bitmap () { return bitmap; }
  ///
  virtual void copy (csTexture* src) = 0;
};

/**
 * 8-bit version of this texture.
 */
class csTexture8 : public csTexture
{
public:
  /// Create a texture with a width and height.
  csTexture8 (csTextureMM* p, int w, int h);
  ///
  virtual ~csTexture8 ();
  ///
  virtual void copy (csTexture* src)
  { memcpy (bitmap, src->get_bitmap (), width*height); }
};

/**
 * 16-bit version of this texture.
 */
class csTexture16 : public csTexture
{
public:
  /// Create a texture with a width and height.
  csTexture16 (csTextureMM* p, int w, int h);
  ///
  virtual ~csTexture16 ();
  ///
  virtual void copy (csTexture* src)
  { memcpy (bitmap, src->get_bitmap (), width*height*sizeof (UShort)); }
};

/**
 * 32-bit version of this texture.
 */
class csTexture32 : public csTexture
{
public:
  /// Create a texture with a width and height.
  csTexture32 (csTextureMM* p, int w, int h);
  ///
  virtual ~csTexture32 ();
  ///
  virtual void copy (csTexture* src)
  { memcpy (bitmap, src->get_bitmap (), width*height*sizeof (ULong)); }
};

/**
 * Create textures.
 */
class csTextureFactory
{
public:
  /// Create a texture.
  virtual csTexture* new_texture (csTextureMM* parent, int w, int h) = 0;
};

/**
 * 8-bit texture creator.
 */
class csTextureFactory8 : public csTextureFactory
{
public:
  /// Create a texture.
  virtual csTexture* new_texture (csTextureMM* parent, int w, int h)
  { CHK (csTexture8* txt = new csTexture8 (parent, w, h)); return txt; }
};

/**
 * 16-bit texture creator.
 */
class csTextureFactory16 : public csTextureFactory
{
public:
  /// Create a texture.
  virtual csTexture* new_texture (csTextureMM* parent, int w, int h)
  { CHK (csTexture16* txt = new csTexture16 (parent, w, h)); return txt; }
};

/**
 * 32-bit texture creator.
 */
class csTextureFactory32 : public csTextureFactory
{
public:
  /// Create a texture.
  virtual csTexture* new_texture (csTextureMM* parent, int w, int h)
  { CHK (csTexture32* txt = new csTexture32 (parent, w, h)); return txt; }
};

/**
 * General version of the texture manager.
 */
class csTextureManager : public iTextureManager
{
  // Private class used to keep a list of csTextureMM heirs
  class csTexVector : public csVector
  {
  public:
    // Initialize the array
    csTexVector (int iLimit, int iDelta) : csVector (iLimit, iDelta) {}
    // Free a single texture
    virtual bool FreeItem (csSome Item)
    {
      if (Item)
        ((iTextureHandle *)Item)->DecRef ();
      return true;
    }
  };

protected:
  /// List of textures.
  csTexVector textures;

  /// Pixel format.
  csPixelFormat pfmt;

  ///
  int red_color;
  int blue_color;
  int yellow_color;
  int green_color;
  int white_color;
  int black_color;

public:
  DECLARE_IBASE;

  ///
  iSystem* System;

  ///
  iGraphics2D* G2D;

  /// Verbose mode.
  bool verbose;

  /// Factory to create the 3D textures.
  csTextureFactory* factory_3d;

  /// Factory to create the 2D textures.
  csTextureFactory* factory_2d;

  /// Texture gamma
  float Gamma;

  /// Configuration value: how is mipmapping done (one of MIPMAP_...)
  int mipmap_mode;

  /// Configuration value: blend mipmap level 0.
  bool do_blend_mipmap0;

  /// For debugging: show lightmapgrid.
  bool do_lightmapgrid;

  /// For debugging: don't show textures but only map lightmaps.
  bool do_lightmaponly;

  ///
  csTextureManager (iSystem* iSys, iGraphics2D* iG2D);
  ///
  virtual ~csTextureManager ();
  ///
  virtual void Initialize ();

  ///
  virtual int find_color (int r, int g, int b) = 0;

  ///
  virtual void clear ();

  ///
  virtual int FindRGB (int r, int g, int b);
  ///
  virtual bool GetVeryNice ()
  { return (mipmap_mode == MIPMAP_VERYNICE); }
  ///
  virtual void SetVerbose (bool vb)
  { verbose = vb; }

  ///
  void SysPrintf (int mode, char* str, ...);

  /**
   * Get depth of the display for which this texture manager is used.
   */
  int get_display_depth () { return pfmt.PixelBytes * 8; }

  /// Get pixel format structure
  const csPixelFormat &pixel_format () const { return pfmt; }

  ///
  int red () { return red_color; }
  ///
  int blue () { return blue_color; }
  ///
  int yellow () { return yellow_color; }
  ///
  int green () { return green_color; }
  ///
  int white () { return white_color; }
  ///
  int black () { return black_color; }

  /// Query the "almost-black" color used for opaque black in transparent textures
  int get_almost_black ();

  /**
   * Query the basic format of textures that can be registered with this
   * texture manager. It is very likely that the texture manager will
   * reject the texture if it is in an improper format. The alpha channel
   * is optional; the texture can have it and can not have it. Only the
   * bits that fit the CS_IMGFMT_MASK mask matters.
   */
  virtual int GetTextureFormat ();
};

#endif // TXTMGR_H
