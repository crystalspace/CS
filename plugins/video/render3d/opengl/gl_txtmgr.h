/*
    Copyright (C) 1998-2004 by Jorrit Tyberghein
	      (C) 2003 by Philip Aumayr
	      (C) 2004 by Frank Richter

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

#ifndef __CS_GL_NEWTXTMGR_H__
#define __CS_GL_NEWTXTMGR_H__

#define CS_MATERIAL_MAX_TEXTURE_LAYERS 4

#include "csgfx/rgbpixel.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "ivideo/txtmgr.h"
#include "iengine/texture.h"
#include "igraphic/imgvec.h"
#include "csutil/blockallocator.h"
#include "csutil/flags.h"
#include "csutil/scf.h"
#include "csutil/refarr.h"
#include "csutil/weakref.h"
#include "csutil/weakrefarr.h"
#include "csutil/blockallocator.h"
#include "csutil/leakguard.h"
#include "csutil/stringarray.h"
#include "igraphic/image.h"
#include "igraphic/imageio.h"
#include "iutil/vfs.h"


#include "gl_render3d.h"
#include "csplugincommon/opengl/glextmanager.h"

class csGLTextureHandle;
class csGLTextureManager;
class csGLTextureCache;

struct csGLUploadData
{
  const void* image_data;
  int w, h, d;
  csRef<iBase> dataRef;
  GLenum targetFormat;
  bool compressed;
  union
  {
    struct 
    {
      GLenum sourceFormat;
      GLenum sourceType;
    };
    struct
    {
      size_t compressedSize;
    };
  };
  int mip;
  int imageNum;

  csGLUploadData() : image_data(0), compressed(false) {}
};

struct csGLTextureClassSettings;

class csGLTextureHandle : public iTextureHandle
{
private:
  CS_LEAKGUARD_DECLARE(csGLTextureHandle);

  /// texturemanager handle
  csRef<csGLTextureManager> txtmgr;

  //GLenum targetFormat;
  csStringID textureClass;

  /// The transparent color
  csRGBpixel transp_color;
  
  /// Used until Prepare() is called
  csRef<iImage> image;

  /// Stores the names of the images
  char* origName;

  /// Texture flags (combined public and private)
  csFlags texFlags;
  /// Private texture flags
  enum
  {
    flagTexupdateNeeded = 1 << 31, 
    flagPrepared = 1 << 30, 
    /// Does it have a keycolor?
    flagTransp = 1 << 29,
    flagForeignHandle = 1 << 28,
    flagWasRenderTarget = 1 << 27,
    flagSizeAdjusted = 1 << 26,
    /// Is the color valid?
    flagTranspSet = 1 << 25,
    flagNeedMips = 1 << 24,

    flagLast,
    /// Mask to get only the "public" flags
    flagsPublicMask = flagLast - 2
  };

  //bool has_alpha;
  csAlphaMode::AlphaType alphaType;
  bool IsTexupdateNeeded() const { return texFlags.Check (flagTexupdateNeeded); }
  void SetTexupdateNeeded (bool b) { texFlags.SetBool (flagTexupdateNeeded, b); }
  bool IsPrepared() const { return texFlags.Check (flagPrepared); }
  void SetPrepared (bool b) { texFlags.SetBool (flagPrepared, b); }
  bool IsTransp() const { return texFlags.Check (flagTransp); }
  void SetTransp (bool b) { texFlags.SetBool (flagTransp, b); }
  bool IsForeignHandle() const { return texFlags.Check (flagForeignHandle); }
  void SetForeignHandle (bool b) { texFlags.SetBool (flagForeignHandle, b); }
  bool IsSizeAdjusted() const { return texFlags.Check (flagSizeAdjusted); }
  void SetSizeAdjusted (bool b) { texFlags.SetBool (flagSizeAdjusted, b); }
  bool IsTranspSet() const { return texFlags.Check (flagTranspSet); }
  void SetTranspSet (bool b) { texFlags.SetBool (flagTranspSet, b); }

  void *cachedata;

  GLenum DetermineTargetFormat (GLenum defFormat, bool allowCompress,
    const char* rawFormat, bool& compressedTarget);
  bool transform (bool allowCompressed, 
    GLenum targetFormat, iImage* Image, int mipNum, int imageNum);

  GLuint Handle;
  /// Upload the texture to GL.
  void Load ();
  void Unload ();
public:
  int orig_width, orig_height, orig_d;
  int actual_width, actual_height, actual_d;
  csArray<csGLUploadData>* uploadData;
  csWeakRef<csGLGraphics3D> G3D;
  int target;
  bool IsWasRenderTarget() const { return texFlags.Check (flagWasRenderTarget); }
  void SetWasRenderTarget (bool b) { texFlags.SetBool (flagWasRenderTarget, b); }
  bool IsNeedMips() const { return texFlags.Check (flagNeedMips); }
  void SetNeedMips (bool b) { texFlags.SetBool (flagNeedMips, b); }

  csGLTextureHandle (iImage* image, int flags, csGLGraphics3D *iG3D);

  csGLTextureHandle (int target, GLuint Handle, csGLGraphics3D *iG3D);

  virtual ~csGLTextureHandle ();

  void Clear();

  void AdjustSizePo2 ();
  void CreateMipMaps();
  void PrepareKeycolor (csRef<iImage>& image, const csRGBpixel& transp_color,
    csAlphaMode::AlphaType& alphaType);
  void CheckAlpha (int w, int h, int d, csRGBpixel *src, 
    const csRGBpixel* transp_color, csAlphaMode::AlphaType& alphaType);
  csRef<iImage>& GetImage () { return image; }
  void Unprepare () { SetPrepared (false); }
  /// Merge this texture into current palette, compute mipmaps and so on.
  void PrepareInt ();

  SCF_DECLARE_IBASE;

  /// Retrieve the flags set for this texture
  virtual int GetFlags () const;

  /// Enable key color
  virtual void SetKeyColor (bool Enable);

  /// Set the key color.
  virtual void SetKeyColor (uint8 red, uint8 green, uint8 blue);

  /// Get the key color status (false if disabled, true if enabled).
  virtual bool GetKeyColor () const;

  /// Get the key color
  virtual void GetKeyColor (uint8 &red, uint8 &green, uint8 &blue) const;

  /// Release the original image (iImage) as given by the engine.
  void FreeImage ();
  /**
   * Get the dimensions for a given mipmap level (0 to 3).
   * If the texture was registered just for 2D usage, mipmap levels above
   * 0 will return false. Note that the result of this function will
   * be the size that the renderer uses for this texture. In most cases
   * this corresponds to the size that was used to create this texture
   * but some renderers have texture size limitations (like power of two)
   * and in that case the size returned here will be the corrected size.
   * You can get the original image size with GetOriginalDimensions().
   */
  virtual bool GetRendererDimensions (int &mw, int &mh);

  /**
   * Return the original dimensions of the image used to create this texture.
   * This is most often equal to GetMipMapDimensions (0, mw, mh) but in
   * some cases the texture will have been resized in order to accomodate
   * hardware restrictions (like power of two and maximum texture size).
   * This function returns the uncorrected coordinates.
   */
  virtual void GetOriginalDimensions (int& mw, int& mh);


  //enum { CS_TEX_IMG_1D = 0, CS_TEX_IMG_2D, CS_TEX_IMG_3D, CS_TEX_IMG_CUBEMAP };
  /**
   * Texture Depth Indices are used for Cubemap interface
   */
  //enum { CS_TEXTURE_CUBE_POS_X = 0, CS_TEXTURE_CUBE_NEG_X, 
  //       CS_TEXTURE_CUBE_POS_Y, CS_TEXTURE_CUBE_NEG_Y,
  //       CS_TEXTURE_CUBE_POS_Z, CS_TEXTURE_CUBE_NEG_Z };

  /**
   * Get the dimensions for a given mipmap level (0 to 3).
   * If the texture was registered just for 2D usage, mipmap levels above
   * 0 will return false. Note that the result of this function will
   * be the size that the renderer uses for this texture. In most cases
   * this corresponds to the size that was used to create this texture
   * but some renderers have texture size limitations (like power of two)
   * and in that case the size returned here will be the corrected size.
   * You can get the original image size with GetOriginalDimensions().
   */
  virtual bool GetRendererDimensions (int &mw, int &mh, int &md);

  /**
   * Return the original dimensions of the image used to create this texture.
   * This is most often equal to GetMipMapDimensions (0, mw, mh, md) but in
   * some cases the texture will have been resized in order to accomodate
   * hardware restrictions (like power of two and maximum texture size).
   * This function returns the uncorrected coordinates.
   */
  virtual void GetOriginalDimensions (int& mw, int& mh, int &md);

  virtual void Blit (int x, int y, int width, int height,
    unsigned char const* data);
  void SetupAutoMipping();

  /// Get the texture target
  virtual int GetTextureTarget () const { return target; }

  /// Get the original image name
  virtual const char* GetImageName () const;

  /// Get data associated internally with this texture by texture cache
  virtual void *GetCacheData ();

  /// Set data associated internally with this texture by texture cache
  virtual void SetCacheData (void *d);

  /**
   * Query the private object associated with this handle.
   * For internal usage by the 3D driver.
   */
  virtual void *GetPrivateObject ();

  /**
   * Query if the texture has an alpha channel.<p>
   * This depends both on whenever the original image had an alpha channel
   * and of the fact whenever the renderer supports alpha maps at all.
   */
  virtual bool GetAlphaMap ();

  virtual csAlphaMode::AlphaType GetAlphaType () const
  { return alphaType; }
  virtual void SetAlphaType (csAlphaMode::AlphaType alphaType)
  { this->alphaType = alphaType; }

  virtual void Precache ();

  virtual void SetTextureClass (const char* className);
  virtual const char* GetTextureClass ();

  void UpdateTexture ();

  GLuint GetHandle ();
};

class csGLSuperLightmap;

/**
 * A single lightmap on a super lightmap.
 */
class csGLRendererLightmap : public iRendererLightmap
{
private:

  friend class csGLSuperLightmap;
  friend class csGLGraphics3D;

  /// Texture coordinates (in pixels)
  csRect rect;
  /// The SLM this lightmap is a part of.
  csRef<csGLSuperLightmap> slm;
public:
  CS_LEAKGUARD_DECLARE (csGLRendererLightmap);

  SCF_DECLARE_IBASE;

  csGLRendererLightmap ();
  virtual ~csGLRendererLightmap ();

  /// Return the LM texture coords, in pixels.
  virtual void GetSLMCoords (int& left, int& top, 
    int& width, int& height);

  /// Set the light data.
  virtual void SetData (csRGBpixel* data);

  /// Set the size of a light cell.
  virtual void SetLightCellSize (int size);
};

/**
 * An OpenGL super lightmap.
 */
class csGLSuperLightmap : public iSuperLightmap
{
private:
  friend class csGLRendererLightmap;

  /// Number of lightmaps on this SLM.
  int numRLMs;

  csRef<csGLTextureHandle> th;

  /// Actually create the GL texture.
  void CreateTexture ();
  /**
   * Free a lightmap. Will also delete the GL texture if no LMs are on 
   * this SLM.
   */
  void FreeRLM (csGLRendererLightmap* rlm);
public:
  CS_LEAKGUARD_DECLARE (csGLSuperLightmap);

  /// GL texture handle
  GLuint texHandle;
  /// Dimensions of this SLM
  int w, h;
  /// Remove the GL texture.
  void DeleteTexture ();

  /// The texture manager that created this SLM.
  csGLTextureManager* txtmgr;

  SCF_DECLARE_IBASE;

  csGLSuperLightmap (csGLTextureManager* txtmgr, int width, int height);
  virtual ~csGLSuperLightmap ();

  /// Add a lightmap.
  virtual csPtr<iRendererLightmap> RegisterLightmap (int left, int top, 
    int width, int height);

  /// Dump the contents onto an image.
  virtual csPtr<iImage> Dump ();

  virtual iTextureHandle* GetTexture ();
};

struct csGLTextureClassSettings
{
  GLenum formatRGB;
  GLenum formatRGBA;
  bool sharpenPrecomputedMipmaps;
  bool forceDecompress;
  bool allowDownsample;
  bool allowMipSharpen;
};

/*
*
* New Texture Manager... done by Phil Aumayr (phil@rarebyte.com)
*
*/
class csGLTextureManager : public iTextureManager
{
private:
  typedef csWeakRefArray<csGLTextureHandle> csTexVector;
  /// List of textures.
  csTexVector textures;

  csPixelFormat pfmt;

  csStringSet textureClassIDs;
  csHash<csGLTextureClassSettings, csStringID> textureClasses;
  struct TextureFormat
  {
    GLenum format;
    bool supported;

    TextureFormat (GLenum fmt, bool supp) : format (fmt), supported (supp) {}
  };
  csHash<TextureFormat, csStrKey> textureFormats;

  GLenum ParseTextureFormat (const char* formatName, GLenum defaultFormat);
  void ReadTextureClasses (iConfigFile* config);

  iObjectRegistry *object_reg;
public:
  CS_LEAKGUARD_DECLARE (csGLTextureManager);

  csWeakRef<csGLGraphics3D> G3D;

  /// All SLMs currently in use.
  csArray<csGLSuperLightmap*> superLMs;

  int max_tex_size;
  /// Sharpen mipmaps?
  int sharpen_mipmaps;
  /// downsample textures?
  int texture_downsample;
  /// texture filtering anisotropy
  float texture_filter_anisotropy;
  int rstate_bilinearmap;

  csStringID nameDiffuseTexture;

  csGLTextureManager (iObjectRegistry* object_reg,
        iGraphics2D* iG2D, iConfigFile *config,
        csGLGraphics3D *G3D);

  virtual ~csGLTextureManager ();

  /// Read configuration values from config file.
  void read_config (iConfigFile *config);
  void Clear ();

  const csGLTextureClassSettings* GetTextureClassSettings (csStringID texclass);
  csStringID GetTextureClassID (const char* className)
  {
    return textureClassIDs.Request (className);
  }
  const char* GetTextureClassName (csStringID classID)
  {
    return textureClassIDs.Request (classID);
  }

  /**
   * Helper function to make sure a texture isn't selected on any TU.
   * Useful when deleting a texture.
   */
  static void UnsetTexture (GLenum target, GLuint texture);

  SCF_DECLARE_IBASE;
  /**
   * Register a texture. The given input image is IncRef'd and DecRef'ed
   * later when not needed any more. If you want to keep the input image
   * make sure you have called IncRef yourselves.
   */
  virtual csPtr<iTextureHandle> RegisterTexture (iImage *image, int flags);

  /**
   * Called from csGLTextureHandle destructor to notify parent texture
   * manager that a material is going to be destroyed.
   */
  void UnregisterTexture (csGLTextureHandle* handle);

  /**
   * Query the basic format of textures that can be registered with this
   * texture manager. It is very likely that the texture manager will
   * reject the texture if it is in an improper format. The alpha channel
   * is optional; the texture can have it and can not have it. Only the
   * bits that fit the CS_IMGFMT_MASK mask matters.
   */
  virtual int GetTextureFormat ();

  virtual csPtr<iSuperLightmap> CreateSuperLightmap(int width, int height);

  virtual void GetMaxTextureSize (int& w, int& h, int& aspect);

  /// Dump all SLMs to image files.
  void DumpSuperLightmaps (iVFS* VFS, iImageIO* iio, const char* dir);

  virtual void GetLightmapRendererCoords (int slmWidth, int slmHeight,
    int lm_x1, int lm_y1, int lm_x2, int lm_y2,
    float& lm_u1, float& lm_v1, float &lm_u2, float& lm_v2);
};

#endif // __CS_GL_TXTMGR_H__

