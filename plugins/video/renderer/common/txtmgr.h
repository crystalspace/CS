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

#include "cscom/com.h"
#include "csgfxldr/boxfilt.h"
#include "cs3d/common/imgtools.h"
#include "csutil/csvector.h"
#include "itxtmgr.h"
#include "itexture.h"
#include "igraph2d.h"

class ImageColorInfo;
class csTextureMM;
class csTextureManager;
class csTexture;
interface IImageFile;
interface ITextureHandle;

#define MIPMAP_UGLY 0
#define MIPMAP_DEFAULT 1
#define MIPMAP_NICE 2
#define MIPMAP_VERYNICE 3

#define MIX_TRUE_RGB 0
#define MIX_NOCOLOR 1

/**
 * csTextureMM represents a texture and all its mipmapped
 * variants.
 */
class csTextureMM
{
protected:
  /// The corresponding ImageFile.
  IImageFile* ifile;
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
   * Create a mipmapped bitmap from a previous level.
   * 'steps' is the number of steps to mipmap (only 1, 2, or 3 supported).
   */
  void create_mipmap_bitmap (csTextureManager* tex, int steps, unsigned char* bm);

  /// Create a blended mipmap with the same size as the previous level.
  void create_blended_mipmap (csTextureManager* tex, unsigned char* bm);

  /// Convert ImageFile to internal format.
  virtual void convert_to_internal (csTextureManager* tex, IImageFile* imfile, unsigned char* bm) = 0;

public:
  /// Set this to TRUE to ignore image loading errors
  static bool fIgnoreLoadingErrors;

  ///
  csTextureMM (IImageFile* image);
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

  /// Release the original image (IImageFile) as given by the engine.
  void free_image ();

  /// Return true if the texture has been loaded correctly.
  bool loaded_correctly () { return fIgnoreLoadingErrors || (ifile != NULL); }

  /// Get the mipmapped texture at some level (0..3).
  csTexture* get_texture (int lev);

  /// Set the transparent color.
  void set_transparent (int red, int green, int blue);

  /// Query color 0 status (true - transparent; false - opaque)
  bool get_transparent () { return istransp; }

  /// Get the transparent color
  void get_transparent (int &red, int &green, int &blue)
  { red = transp_color.red; green = transp_color.green; blue = transp_color.blue; }

  /// Get the mean color index.
  int get_mean_color_idx () { return mean_idx; }

  /// Get the mean color.
  const csColor& get_mean_color () { return mean_color; }

  ///
  int get_num_colors () { return usage->get_num_colors (); }

  /// Compute the 'usage' table.
  void compute_color_usage ();

  ///
  const RGBPalEntry& get_usage (int idx) 
  { return usage->get_color_table()[idx]; }

  /**
   * Remap this texture in the most optimal way
   * for the 3D/2D driver.
   */
  virtual void remap_texture (csTextureManager* new_palette) = 0;

  DECLARE_INTERFACE_TABLE (csTextureMM)
  DECLARE_IUNKNOWN()
  DECLARE_COMPOSITE_INTERFACE (TextureHandle)
};

#define GetITextureHandleFromcsTextureMM(a)  &a->m_xTextureHandle
#define GetcsTextureMMFromITextureHandle(a)  ((csTextureMM*)((size_t)a - offsetof(csTextureMM, m_xTextureHandle)))

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
  virtual unsigned char* get_bitmap8 () = 0;
  ///
  virtual UShort* get_bitmap16 () = 0;
  ///
  virtual ULong* get_bitmap32 () = 0;
  ///
  virtual void copy (csTexture* src) = 0;
};

/**
 * 8-bit version of this texture.
 */
class csTexture8 : public csTexture
{
private:
  /// Raw data.
  unsigned char* bitmap;

public:
  /// Create a texture with a width and height.
  csTexture8 (csTextureMM* p, int w, int h);
  ///
  virtual ~csTexture8 ();
  ///
  virtual unsigned char* get_bitmap8 () { return bitmap; }
  ///
  virtual UShort* get_bitmap16 () { return (UShort*)bitmap; }
  ///
  virtual ULong* get_bitmap32 () { return (ULong*)bitmap; }
  ///
  virtual void copy (csTexture* src)
  {
    memcpy (bitmap, src->get_bitmap8 (), width*height);
  }
};

/**
 * 16-bit version of this texture.
 */
class csTexture16 : public csTexture
{
private:
  /// Raw data.
  UShort* bitmap;

public:
  /// Create a texture with a width and height.
  csTexture16 (csTextureMM* p, int w, int h);
  ///
  virtual ~csTexture16 ();
  ///
  virtual unsigned char* get_bitmap8 () { return (unsigned char*)bitmap; }
  ///
  virtual UShort* get_bitmap16 () { return bitmap; }
  ///
  virtual ULong* get_bitmap32 () { return (ULong*)bitmap; }
  ///
  virtual void copy (csTexture* src)
  {
    memcpy (bitmap, src->get_bitmap16 (), width*height*sizeof (UShort));
  }
};

/**
 * 32-bit version of this texture.
 */
class csTexture32 : public csTexture
{
private:
  /// Raw data.
  ULong* bitmap;

public:
  /// Create a texture with a width and height.
  csTexture32 (csTextureMM* p, int w, int h);
  ///
  virtual ~csTexture32 ();
  ///
  virtual unsigned char* get_bitmap8 () { return (unsigned char*)bitmap; }
  ///
  virtual UShort* get_bitmap16 () { return (UShort*)bitmap; }
  ///
  virtual ULong* get_bitmap32 () { return bitmap; }
  ///
  virtual void copy (csTexture* src)
  {
    memcpy (bitmap, src->get_bitmap32 (), width*height*sizeof (ULong));
  }
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
  {
    CHK (csTexture8* txt = new csTexture8 (parent, w, h));
    return txt;
  }
};

/**
 * 16-bit texture creator.
 */
class csTextureFactory16 : public csTextureFactory
{
public:
  /// Create a texture.
  virtual csTexture* new_texture (csTextureMM* parent, int w, int h)
  {
    CHK (csTexture16* txt = new csTexture16 (parent, w, h));
    return txt;
  }
};

/**
 * 32-bit texture creator.
 */
class csTextureFactory32 : public csTextureFactory
{
public:
  /// Create a texture.
  virtual csTexture* new_texture (csTextureMM* parent, int w, int h)
  {
    CHK (csTexture32* txt = new csTexture32 (parent, w, h));
    return txt;
  }
};

/**
 * General version of the texture manager.
 */
class csTextureManager : public ITextureManager
{
protected:
  /// List of textures.
  csVector textures;

  /// Pixel format.
  csPixelFormat pfmt;

  int red_color;
  int yellow_color;
  int green_color;
  int blue_color;
  int white_color;
  int black_color;

public:
  ///
  ISystem* m_piSystem;

  ///
  IGraphics2D* m_piG2D;

  /// Verbose mode.
  bool verbose;

  /// Factory to create the 3D textures.
  csTextureFactory* factory_3d;

  /// Factory to create the 2D textures.
  csTextureFactory* factory_2d;

  /// Texture gamma
  float Gamma;

  /// Configuration value: how is mipmapping done (one of MIPMAP_...)
  int mipmap_nice;

  /// Configuration value: blend mipmap level 0.
  bool do_blend_mipmap0;

  /// For debugging: show lightmapgrid.
  bool do_lightmapgrid;

  /// For debugging: don't show textures but only map lightmaps.
  bool do_lightmaponly;

  /// Filter to use when mipmapping 1 level.
  static Filter3x3 mipmap_filter_1;

  /// Filter to use when mipmapping 2 levels.
  static Filter5x5 mipmap_filter_2;

  /// Filter to use when performing blending.
  static Filter3x3 blend_filter;

  /// How do we mix colors (one of MIX_...)
  int mixing;
  /// Force value (set by commandline) (-1 = no force)
  int force_mix;

  /// If true we have R,G,B lights.
  bool use_rgb;

  ///
  csTextureManager (ISystem* piSystem, IGraphics2D* piG2D);
  ///
  virtual ~csTextureManager ();
  ///
  virtual void InitSystem ();

  ///
  virtual int find_color (int r, int g, int b) = 0;

  ///
  virtual void clear ();

  ///
  STDMETHODIMP FindRGB (int r, int g, int b, int& color);
  ///
  STDMETHODIMP GetVeryNice (bool& result) { result = (mipmap_nice == MIPMAP_VERYNICE); return S_OK; }
  ///
  STDMETHODIMP SetVerbose (bool vb) { verbose = vb; return S_OK; }

  DECLARE_IUNKNOWN ()
  DECLARE_INTERFACE_TABLE (csTextureManager)

  ///
  void SysPrintf (int mode, char* str, ...);

  /**
   * Get depth of the display for which this texture manager is used.
   */
  int get_display_depth () { return pfmt.PixelBytes * 8; }

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
};


#endif // TXTMGR_H

