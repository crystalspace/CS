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

#ifndef __CS_NULL_TXT_H__
#define __CS_NULL_TXT_H__

#include "csplugincommon/render3d/txtmgr.h"
#include "igraphic/image.h"

class csTextureManagerNull;

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
  uint8 *bitmap;
  /// The alpha map (0 if no alphamap)
  uint8 *alphamap;
  /// The image (temporary storage)
  iImage *image;

  /// Create a csTexture object
  csTextureNull (csTextureHandle *Parent, iImage *Image) : csTexture (Parent)
  {
    bitmap = 0;
    alphamap = 0;
    image = Image;
    image->IncRef();
    w = Image->GetWidth ();
    h = Image->GetHeight ();
    compute_masks ();
  }
  /// Destroy the texture
  virtual ~csTextureNull ()
  { delete [] bitmap;
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

  /// The private palette
  csRGBpixel palette [256];

  /// Number of used colors in palette
  int palette_size;

  /// Is already prepared.
  bool prepared;

  /// The texture manager
  csTextureManagerNull *texman;

  /// Create a new texture object
  csTexture *NewTexture (iImage *Image, bool ismipmap);

  /// Compute the mean color for the just-created texture
  void ComputeMeanColor ();

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

  /**
   * Query if the texture has an alpha channel.<p>
   * This depends both on whenever the original image had an alpha channel
   * and of the fact whenever the renderer supports alpha maps at all.
   */
  bool GetAlphaMap ()
  { return !!((csTextureNull *)get_texture (0))->get_alphamap (); }

  /// Prepare the texture for usage
  void PrepareInt ();

  csAlphaMode::AlphaType GetAlphaType () { return csAlphaMode::alphaNone; }

  bool GetRendererDimensions (int &mw, int &mh, int &md)
  { md = 0; return csTextureHandle::GetRendererDimensions (mw, mh); }
  void GetOriginalDimensions (int& mw, int& mh, int& md)
  { GetRendererDimensions (mw, mh, md); }
  void SetTextureTarget (int target) { }
  int GetTextureTarget () const { return iTextureHandle::CS_TEX_IMG_2D; }
  const char* GetImageName () const { return image->GetName (); }
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
  /// We need a pointer to the 2D driver
  iGraphics2D *G2D;

public:
  ///
  csTextureManagerNull (iObjectRegistry *object_reg,
  	iGraphics2D *iG2D, iConfigFile *config);
  ///
  virtual ~csTextureManagerNull ();

  /// Called from G3D::Open ()
  void SetPixelFormat (csPixelFormat &PixelFormat);

  /// Encode RGB values to a 16-bit word (for 16-bit mode).
  uint32 encode_rgb (int r, int g, int b);

  /// Read configuration values from config file.
  virtual void read_config (iConfigFile *config);

  ///
  virtual void Clear ();

  ///
  virtual csPtr<iTextureHandle> RegisterTexture (iImage* image, int flags);
  ///
  virtual void UnregisterTexture (csTextureHandleNull* handle);

  virtual csPtr<iSuperLightmap> CreateSuperLightmap(int w, int h);
  
  virtual void GetMaxTextureSize (int& w, int& h, int& aspect);

  virtual void GetLightmapRendererCoords (int slmWidth, int slmHeight,
    int lm_x1, int lm_y1, int lm_x2, int lm_y2,
    float& lm_u1, float& lm_v1, float &lm_u2, float& lm_v2);
};

#endif // __CS_NULL_TXT_H__
