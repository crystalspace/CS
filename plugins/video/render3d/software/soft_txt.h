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

#include "csgeom/csrect.h"
#include "csplugincommon/render3d/txtmgr.h"
#include "csplugincommon/softshader/texture.h"
#include "csutil/blockallocator.h"
#include "csutil/debug.h"
#include "csutil/hashr.h"
#include "igraphic/image.h"
#include "ivideo/graph2d.h"

namespace cspluginSoft3d
{
  using namespace CrystalSpace::SoftShader;


class csSoftwareGraphics3DCommon;
class csSoftwareTextureManager;
class csSoftwareTextureHandle;

/**
 * A software texture.
 * Every csSoftwareTextureHandle contains several csSoftwareTexture 
 * objects.
 * Every csSoftwareTexture is just a single image and all associated 
 * parameters - width, height, shifts and so on. For performance reasons 
 * textures are allowed to be only power-of-two sizes (both horizontal and 
 * vertical).
 * This allows us to use simple binary shift/and instead of mul/div.
 * It is the responsability of csSoftwareTextureHandle  to resize textures 
 * if they do not fulfil this requirement.
 */
class csSoftwareTexture : public SoftwareTexture
{
protected:

  /// Compute shf_x and and_x values
  void compute_masks ();
  void ImageToBitmap (iImage *Image);
public:
  /// The parent csSoftwareTextureHandle object
  csSoftwareTextureHandle* parent;

  /// Create a csTexture object
  csSoftwareTexture (csSoftwareTextureHandle* Parent, iImage *Image) : 
    parent (Parent)
  {
    bitmap = 0;
    w = Image->GetWidth ();
    h = Image->GetHeight ();
    compute_masks ();
    ImageToBitmap (Image);
  }
  /// Destroy the texture
  virtual ~csSoftwareTexture ()
  {
    delete [] bitmap;
  }
};

/**
 * csSoftwareTextureHandle represents a texture and all its mipmapped
 * variants.
 */
class csSoftwareTextureHandle : public csTextureHandle
{
protected:
  friend class csSoftwareGraphics3DCommon;

  /// If true then PrepareInt() has done its job.
  bool prepared;

  /// Create a new texture object
  virtual csSoftwareTexture* NewTexture (iImage *Image, bool ismipmap);

  /// the texture manager
  csRef<csSoftwareTextureManager> texman;

  /// Texture for mipmap levels 0..3
  csSoftwareTexture *tex [4];

  csRef<iImage> image;

  /// Create all mipmapped bitmaps from the first level.
  void CreateMipmaps ();
public:
  /// Create the mipmapped texture object
  csSoftwareTextureHandle (csSoftwareTextureManager *texman, iImage *image,
		       int flags);
  /// Destroy the object and free all associated storage
  virtual ~csSoftwareTextureHandle ();

  /**
   * Query if the texture has an alpha channel.<p>
   * This depends both on whenever the original image had an alpha channel
   * and of the fact whenever the renderer supports alpha maps at all.
   */
  virtual bool GetAlphaMap () 
  { return alphaType != csAlphaMode::alphaNone; }

  /**
   * Merge this texture into current palette, compute mipmaps and so on.
   */
  void PrepareInt ();

  /**\name iTextureHandle implementation
   * @{ */
  virtual void Precache () {}

  virtual bool GetRendererDimensions (int &mw, int &mh, int& md)
  { return false; }

  virtual bool GetRendererDimensions (int &mw, int &mh);

  virtual void GetOriginalDimensions (int& mw, int& mh, int &md)
  { }

  virtual void GetOriginalDimensions (int& w, int& h)
  { GetRendererDimensions (w, h); }

  virtual void SetTextureTarget(int target)
  { }
  
  virtual int GetTextureTarget () const
  {
    return iTextureHandle::CS_TEX_IMG_2D;
  }

  virtual const char* GetImageName () const { return image->GetName (); }

  virtual void Blit (int x, int y, int width, int height,
    unsigned char const* data, TextureBlitDataFormat format = RGBA8888);
  /** @} */

  /// Get the texture at the corresponding mipmap level (0..3)
  csSoftwareTexture* GetTexture (int mipmap)
  {
    PrepareInt ();
    return (mipmap >= 0) && (mipmap < 4) ? tex [mipmap] : 0;
  }
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

  csHashReversible<csPtrKey<csSoftRendererLightmap>, int> idmap;
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

  /**
   * Query the basic format of textures that can be registered with this
   * texture manager. It is very likely that the texture manager will
   * reject the texture if it is in an improper format. The alpha channel
   * is optional; the texture can have it and can not have it. Only the
   * bits that fit the CS_IMGFMT_MASK mask matters.
   */
  virtual int GetTextureFormat ();

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

} // namespace cspluginSoft3d

#endif // __CS_SOFT_TXT_H__
