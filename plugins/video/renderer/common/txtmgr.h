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

#ifndef __TXTMGR_H__
#define __TXTMGR_H__

#include "csutil/csvector.h"
#include "ivideo/txtmgr.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "csgfx/rgbpixel.h"

class csTexture;
class csTextureManager;
struct iImage;
struct iConfigFile;

/**
 * This class is the top-level representation of a texture.
 * It contains a number of csTexture objects that represents
 * each a single image. A csTexture object is created for
 * each mipmap and for the 2D texture. This class is responsible
 * for creating these textures and filling them with the correct
 * info. The csTextureHandle class is private to the 3D driver, the
 * driver clients see just the iTextureHandle interface.
 * <p>
 * The handle is initialized by giving the 3D driver a iImage object.
 * At the time client calls TextureManager::PrepareTextures() the
 * mipmaps and the 2D textures are created, if needed. After this
 * you can call TextureManager::FreeImages() which in turn will call
 * csTextureHandle::FreeImage () for each registered texture and the
 * original texture will be released. This means you will free the
 * memory occupied by the original textures, but it also means you
 * cannot call TextureManager::Prepare() again.
 */
class csTextureHandle : public iTextureHandle
{
protected:
  /// The original image object.
  iImage *image;

  /// Texture usage flags: 2d/3d/etc
  int flags;

  /// Texture for mipmap levels 0..3
  csTexture *tex [4];

  /// Texture cache for-internal-use pointer
  void *cachedata;

  /// Does color 0 mean "transparent" for this texture?
  bool transp;
  /// The transparent color
  csRGBpixel transp_color;
  /// Mean color used when texture mapping is disabled.
  csRGBpixel mean_color;

public:
  ///
  csTextureHandle (iImage *Image, int Flags);
  ///
  virtual ~csTextureHandle ();

  /// Get texture usage flags
  int GetFlags () { return flags; }

  /// Release the original image (iImage) as given by the engine.
  void FreeImage ();

  /// Create all mipmapped bitmaps from the first level.
  virtual void CreateMipmaps ();

  /// Get the texture at the corresponding mipmap level (0..3)
  csTexture *get_texture (int mipmap)
  { return (mipmap >= 0) && (mipmap < 4) ? tex [mipmap] : 0; }

  /**
   * Adjusts the textures size, to ensure some restrictions like
   * power of two dimension are met.
   */
  void AdjustSizePo2 ();

  /// Get the transparent color as a RGB pixel
  csRGBpixel *get_transparent ()
  { return &transp_color; }

  /// Create a new texture object (should be implemented by heirs)
  virtual csTexture* NewTexture (iImage *Image) = 0;

  /// Compute the mean color for the just-created texture
  virtual void ComputeMeanColor () = 0;

  ///---------------------- iTextureHandle implementation ----------------------
  DECLARE_IBASE;

  /// Enable transparent color
  virtual void SetKeyColor (bool Enable);

  /// Set the transparent color.
  virtual void SetKeyColor (UByte red, UByte green, UByte blue);

  /// Get the transparent status (false if no transparency, true if transparency).
  virtual bool GetKeyColor ();

  /// Get the transparent color
  virtual void GetKeyColor (UByte &r, UByte &g, UByte &b);

  /**
   * Get the dimensions for a given mipmap level (0 to 3).
   * This function is only valid if the texture has been registered
   * for 3D usage.
   */
  virtual bool GetMipMapDimensions (int mm, int& w, int& h);

  /// Get the mean color.
  virtual void GetMeanColor (UByte &r, UByte &g, UByte &b);

  /// Get data associated internally with this texture by texture cache
  virtual void *GetCacheData ()
  { return cachedata; }
  /// Set data associated internally with this texture by texture cache
  virtual void SetCacheData (void *d)
  { cachedata = d; }

  /// Get the csTextureHandle object associated with the texture handle
  virtual void *GetPrivateObject ()
  { return (csTextureHandle *)this; }

  virtual iGraphics3D *GetProcTextureInterface ()
  { return NULL; }

  virtual void ProcTextureSync () {};

  /**
   * Query if the texture has an alpha channel.<p>
   * This depends both on whenever the original image had an alpha channel
   * and of the fact whenever the renderer supports alpha maps at all.
   */
  virtual bool GetAlphaMap ()
  { return false; }
};

/**
 * A simple texture.
 * Every csTextureHandle contains several csTexture objects.
 * Every csTexture is just a single image and all associated parameters -
 * width, height, shifts and so on. For performance reasons textures
 * are allowed to be only power-of-two sizes (both horizontal and vertical).
 * This allows us to use simple binary shift/and instead of mul/div.
 * It is the responsability of csTextureHandle to resize textures if they
 * do not fulfil this requirement.
 *<p>
 * The actual csTexture class does not implement any storage for the
 * actual texture data. Every 3D driver should derive a own class from
 * csTexture and implement appropiate backing store (for example, most
 * hardware drivers will store the texture as a texture handle).
 */
class csTexture
{
protected:
  /// The parent csTextureHandle object
  csTextureHandle *parent;
  /// Width and height
  int w, h;
  /// log2(width) and log2(height)
  int shf_w, shf_h;
  /// (1 << log2(width)) - 1 and (1 << log2(height)) - 1
  int and_w, and_h;

  /// Compute shf_x and and_x values
  void compute_masks ();

public:
  /// Create a csTexture object
  csTexture (csTextureHandle *Parent) { parent = Parent; }
  /// Destroy the texture object
  virtual ~csTexture () {}

  ///
  int get_width () { return w; }
  ///
  int get_height () { return h; }
  ///
  int get_w_shift () { return shf_w; }
  ///
  int get_h_shift () { return shf_h; }
  ///
  int get_w_mask () { return and_w; }
  ///
  int get_h_mask () { return and_h; }
  /// Query image size (alas we can't do (h << shf_w))
  int get_size () { return w * h; }
  ///
  csTextureHandle *get_parent () { return parent; }
};

/**
 * This class is the top-level representation of a material.
 */
class csMaterialHandle : public iMaterialHandle
{
protected:
  /// A texture.
  iTextureHandle *texture;
  /// Numer of texture layers.
  int num_texture_layers;
  /// Texture layers.
  csTextureLayer texture_layers[4];
  /// The flat color of the material
  csRGBpixel flat_color;
  /// Material reflection parameters
  float diffuse, ambient, reflection;
  /// Original material.
  iMaterial *material;
  /// Parent texture manager
  csTextureManager *texman;

public:
  ///
  csMaterialHandle (iMaterial* material, csTextureManager *parent);
  ///
  csMaterialHandle (iTextureHandle* texture, csTextureManager *parent);
  ///
  virtual ~csMaterialHandle ();

  /// Release the original material (iMaterial).
  void FreeMaterial ();

  ///---------------------- iMaterialHandle implementation ----------------------
  DECLARE_IBASE;

  /**
   * Get a texture from the material.
   */
  virtual iTextureHandle *GetTexture () { return texture; }

  /**
   * Get the flat color. If the material has a texture assigned, this
   * will return the mean texture color.
   */
  virtual void GetFlatColor (csRGBpixel &oColor) { oColor = flat_color; }

  /**
   * Get light reflection parameters for this material.
   */
  virtual void GetReflection (float &oDiffuse, float &oAmbient, float &oReflection)
  { oDiffuse = diffuse; oAmbient = ambient; oReflection = reflection; }

  /**
   * Prepare this material. The material wrapper (remembered during
   * RegisterMaterial()) is queried again for material parameters
   * and a new material descriptor (internal to the texture manager)
   * is associated with given material handle.
   */
  virtual void Prepare ();
};

/**
 * General version of the texture manager.
 * Each 3D driver should derive a texture manager class from this one
 * and implement the missing functionality.
 */
class csTextureManager : public iTextureManager
{
protected:
  // Private class used to keep a list of objects derived from csTextureHandle
  class csTexVector : public csVector
  {
  public:
    // Initialize the array
    csTexVector (int iLimit, int iDelta) : csVector (iLimit, iDelta) 
    { }
    // Shortcut to avoid typecasts
    csTextureHandle *Get (int index)
    { return (csTextureHandle *)csVector::Get (index); }
  };

  /// List of textures.
  csTexVector textures;

  // Private class used to keep a list of objects derived from csMaterialHandle
  class csMatVector : public csVector
  {
  public:
    // Initialize the array
    csMatVector (int iLimit, int iDelta) : csVector (iLimit, iDelta) 
    { }
    // Shortcut to avoid typecasts
    csMaterialHandle *Get (int index)
    { return (csMaterialHandle *)csVector::Get (index); }
  };

  /// List of materials.
  csMatVector materials;

  ///
  iSystem *System;

  /// Verbose mode.
  bool verbose;

  /// Read configuration values from config file.
  virtual void read_config (iConfigFile *config);

public:
  /// Pixel format.
  csPixelFormat pfmt;

  DECLARE_IBASE;

  /// Initialize the texture manager
  csTextureManager (iSystem* iSys, iGraphics2D *iG2D);
  /// Destroy the texture manager
  virtual ~csTextureManager ();

  /**
   * Called from csMaterialHandle destructor to notify parent texture
   * manager that a material is going to be destroyed.
   */
  void UnregisterMaterial (csMaterialHandle* handle);

  /// Clear (free) all textures
  virtual void Clear ()
  { textures.DeleteAll (); }

  /// Toggle verbose mode
  virtual void SetVerbose (bool enable)
  { verbose = enable; }
  /// Free all images associated with textures
  virtual void FreeImages ();

  /**
   * Query the basic format of textures that can be registered with this
   * texture manager. It is very likely that the texture manager will
   * reject the texture if it is in an improper format. The alpha channel
   * is optional; the texture can have it and can not have it. Only the
   * bits that fit the CS_IMGFMT_MASK mask matters.
   */
  virtual int GetTextureFormat ();
  /**
   * Return the index for some color. This works in 8-bit
   * (returns an index in the 256-color table) and in 15/16-bit
   * (returns a 15/16-bit encoded RGB value).
   */
  virtual int FindRGB (int r, int g, int b);
  /// Clear the palette (including all reserved colors)
  virtual void ResetPalette ();
  /// Reserve a color in palette (if any)
  virtual void ReserveColor (int r, int g, int b);
  /// Really allocate the palette on the system.
  virtual void SetPalette ();

  /**
   * Register a material. The input material wrapper is IncRef'd and DecRef'ed
   * later when FreeMaterials () is called or the material handle is destroyed
   * by calling DecRef on it enough times. If you want to keep the input
   * material make sure you have called IncRef yourselves.
   */
  virtual iMaterialHandle* RegisterMaterial (iMaterial* material);

  /**
   * Register a material based on a texture handle. This is a short-cut
   * to quickly make materials based on a single texture.
   */
  virtual iMaterialHandle* RegisterMaterial (iTextureHandle* txthandle);

  /**
   * Default stub implementation until the
   * material system is actually working.
   */
  virtual void PrepareMaterials ();

  /**
   * Default stub implementation until the
   * material system is actually working.
   */
  virtual void FreeMaterials ();
};

#endif // __TXTMGR_H__
