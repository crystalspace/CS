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

#ifndef __CS_SOFT_TXT_H__
#define __CS_SOFT_TXT_H__

#include "csutil/blockallocator.h"
#include "csutil/debug.h"
#include "csutil/hashr.h"
#include "csgeom/csrect.h"
#include "csplugincommon/render3d/txtmgr.h"
#include "igraphic/image.h"
#include "ivideo/graph2d.h"

class csSoftwareGraphics3DCommon;
class csSoftwareTextureManager;
class csSoftwareTextureHandle;

/**
 * csSoftwareTexture is a class derived from csTexture that implements
 * all the additional functionality required by the software renderer.
 * Every csSoftwareTexture is a 8-bit paletted image with a private
 * colormap. The private colormap is common for all mipmapped variants.
 * The colormap is stored inside the parent csTextureHandle object.
 */
class csSoftwareTexture : public csTexture
{
public:
  /// The bitmap
  uint8 *bitmap;
  /// The alpha map (0 if no alphamap)
  uint8 *alphamap;
  /// The image (temporary storage)
  csRef<iImage> image;

  /// Create a csTexture object
  csSoftwareTexture (csTextureHandle *Parent, iImage *Image)
	  : csTexture (Parent)
  {
    bitmap = 0;
    alphamap = 0;
    image = Image;
    DG_LINK (this, image);
    w = Image->GetWidth ();
    h = Image->GetHeight ();
    compute_masks ();
  }
  /// Destroy the texture
  virtual ~csSoftwareTexture ()
  {
    delete [] bitmap;
    delete [] alphamap;
    image = 0;
  }

  /// Return a pointer to texture data
  uint8 *get_bitmap ()
  { return bitmap; }
  /// Return a pointer to alpha map data
  uint8 *get_alphamap () const
  { return alphamap; }
};

/**
 * csSoftwareTextureHandle represents a texture and all its mipmapped
 * variants.
 */
class csSoftwareTextureHandle : public csTextureHandle
{
protected:
  /**
   * Private colormap -> global colormap table
   * For 16- and 32-bit modes this array contains a 256-element array
   * of either shorts or longs to convert any image pixel from 8-bit
   * paletted format to the native pixel format.
   */
  void *pal2glob;

  /// The private palette.
  csRGBpixel palette [256];

  /// A number that is incremented if the texture is modified (proc texture).
  uint32 update_number;

  /// If true then PrepareInt() has done its job.
  bool prepared;

  /// If true then the palette of the canvas is being initialized.
  bool is_palette_init;

  /**
   * If the following flag is true then this texture uses a uniform
   * 3:3:2 palette. This is used when the texture has been rendered
   * on (using SetRenderTarget()).
   */
  bool use_332_palette;

  /// Number of used colors in palette
  int palette_size;

  /// Create a new texture object
  virtual csTexture *NewTexture (iImage *Image, bool ismipmap);

  /// Compute the mean color for the just-created texture
  virtual void ComputeMeanColor ();

  /// Helper function: Initialises according to given palette
  void SetupFromPalette ();

  /// the texture manager
  csRef<csSoftwareTextureManager> texman;

public:
  /// Create the mipmapped texture object
  csSoftwareTextureHandle (csSoftwareTextureManager *texman, iImage *image,
		       int flags);
  /// Destroy the object and free all associated storage
  virtual ~csSoftwareTextureHandle ();

  /**
   * Create the [Private colormap] -> global colormap table.
   * In 256-color modes we find the correspondense between private texture
   * colormap and the global colormap; in truecolor modes we just build
   * a [color index] -> [truecolor value] conversion table.
   */
  void remap_texture ();

  /**
   * Setup a 332 palette for this texture. This is useful when the
   * texture is being used as a procedural texture. If the texture
   * is already a 332 texture then nothing will happen.
   */
  void Setup332Palette ();

  /// Query the private texture colormap
  csRGBpixel *GetColorMap () { return palette; }
  /// Query the number of colors in the colormap
  int GetColorMapSize () { return palette_size; }

  /// Query palette -> native format table
  void *GetPaletteToGlobal () { return pal2glob; }

  /**
   * Query if the texture has an alpha channel.<p>
   * This depends both on whenever the original image had an alpha channel
   * and of the fact whenever the renderer supports alpha maps at all.
   */
  virtual bool GetAlphaMap () 
  { return !!((const csSoftwareTexture *)get_texture (0))->get_alphamap (); }

  /**
   * Merge this texture into current palette, compute mipmaps and so on.
   */
  virtual void PrepareInt ();

  /**
   * Indicate the texture is modified (update update_number).
   */
  void UpdateTexture () { update_number++; }

  /**
   * Change a palette entry. This is called from within the callback
   * from the canvas associated with this texture.
   */
  void ChangePaletteEntry (int idx, int r, int g, int b);

  /**
   * Get the texture update number.
   */
  uint32 GetUpdateNumber () const { return update_number; }

  virtual bool GetRendererDimensions (int &mw, int &mh, int& md)
  {
    return false;
  }

  virtual void GetOriginalDimensions (int& mw, int& mh, int &md)
  {
  }

  virtual void SetTextureTarget(int target)
  {
  }
  
  virtual int GetTextureTarget () const { return iTextureHandle::CS_TEX_IMG_2D; }

  virtual const char* GetImageName () const { return image->GetName (); }

  virtual void Blit (int x, int y, int width, int height,
    unsigned char const* data);
};

class csSoftSuperLightmap;

class csSoftRendererLightmap : public iRendererLightmap
{
  friend class csSoftSuperLightmap;
  friend class csSoftwareTextureCache;

  csRect rect;
  float u1, v1, u2, v2;
  csRef<csSoftSuperLightmap> slm;

  csRGBpixel* data;
  size_t lmSize;
  bool dirty;

  int lightCellSize;
  int lightCellShift;
public:
  void* cacheData[4];

  SCF_DECLARE_IBASE;

  csSoftRendererLightmap ();
  virtual ~csSoftRendererLightmap ();

  void SetSize (size_t lmPixels);
  
  virtual void GetSLMCoords (int& left, int& top, 
    int& width, int& height);

  virtual void SetData (csRGBpixel* data);

  virtual void SetLightCellSize (int size);
  virtual int GetLightCellSize ();
};

class csSoftSuperLightmap : public iSuperLightmap
{
  friend class csSoftRendererLightmap;

  int w, h;

  csBlockAllocator<csSoftRendererLightmap> RLMs;

  csRef<iTextureHandle> tex;
  void FreeRLM (csSoftRendererLightmap* rlm);

  csHashReversible<csSoftRendererLightmap*, int> idmap;
public:
  csSoftRendererLightmap* GetRlmForID (int id);

  SCF_DECLARE_IBASE;

  csSoftSuperLightmap (csSoftwareTextureManager* texman, int width, int height);
  virtual ~csSoftSuperLightmap ();

  virtual csPtr<iRendererLightmap> RegisterLightmap (int left, int top, 
    int width, int height);

  virtual csPtr<iImage> Dump ();

  virtual iTextureHandle* GetTexture ();

  /**
   * Compute the ID for a renderer LM. 
   */
  static int ComputeRlmID (int u, int v)
  { 
    /*
      Cantor's pairing function: maps every pair (n,n) uniquely to a number n
      (http://en.wikipedia.org/wiki/Pairing_function)
     */
    return ((u + v) * (u + v + 1)) / 2 + v; 
  }
};

/**
 * Software version of the texture manager. This instance of the
 * texture manager is probably the most involved of all 3D rasterizer
 * specific texture manager implementations because it needs to do
 * a lot of work regarding palette management and the creation
 * of lots of lookup tables.
 */
class csSoftwareTextureManager : public csTextureManager
{
  friend class csSoftwareTextureHandle;

public:

  /// We need a pointer to the 3D driver
  csSoftwareGraphics3DCommon *G3D;

  /// Apply dithering to textures while reducing from 24-bit to 8-bit paletted?
  bool dither_textures;

  /// Sharpen mipmaps?
  int sharpen_mipmaps;

  /// The multiplication tables used for lightmapping
  uint8 *lightmap_tables [3];

  ///
  csSoftwareTextureManager (iObjectRegistry *object_reg,
  			    csSoftwareGraphics3DCommon *iG3D,
			    iConfigFile *config);
  ///
  virtual ~csSoftwareTextureManager ();

  /// Called from G3D::Open ()
  void SetPixelFormat (csPixelFormat const& PixelFormat);

  /// Encode RGB values to a 16-bit word (for 16-bit mode).
  uint32 encode_rgb (int r, int g, int b);

  /// Called from csSoftwareTextureHandle destructor to deregister before death
  void UnregisterTexture (csSoftwareTextureHandle* handle);

  /// Read configuration values from config file.
  virtual void read_config (iConfigFile *config);

  ///
  virtual void Clear ();

  ///
  virtual csPtr<iTextureHandle> RegisterTexture (iImage* image, int flags);

  virtual csPtr<iSuperLightmap> CreateSuperLightmap (int width, 
    int height);

  virtual void GetMaxTextureSize (int& w, int& h, int& aspect);

  /**
   * Retrieve the coordinates of a lightmap in the its superlightmap, in a
   * system the renderer uses internally. Calculate lightmap U/Vs within this
   * bounds when they are intended to be passed to the renderer.
   */
  virtual void GetLightmapRendererCoords (int slmWidth, int slmHeight,
    int lm_x1, int lm_y1, int lm_x2, int lm_y2,
    float& lm_u1, float& lm_v1, float &lm_u2, float& lm_v2);
};

#endif // __CS_SOFT_TXT_H__
