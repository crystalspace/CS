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
#include "plugins/video/canvas/openglcommon/glextmanager.h"

class csGLTextureHandle;
class csGLTextureManager;
class csGLTextureCache;

class csGLTexture
{
public:
  CS_LEAKGUARD_DECLARE(csGLTexture);

  csGLTexture (csGLTextureHandle *p, iImage* Image);
  virtual ~csGLTexture();

  csGLTextureHandle *Parent;

  struct UploadData
  {
    uint8* image_data;
    /** 
     * Ref to image that owns image_data.
     * Note: only if imgRef == 0 image_data will be delete[]d.
     */
    csRef<iImage> imgRef;
    GLint compressed;
    GLint internalFormat;
    GLint size;

    UploadData ();
    ~UploadData ();
  };
  UploadData* uploadData;
  int w, h, d/*, components*/;

  void CleanupImageData ();
   ///
  int get_width () const { return w; }
  ///
  int get_height () const { return h; }

  int get_depth () const { return d; }

  //int get_components () const { return components; } 

  uint8 *&get_image_data ()
  { 
    CS_ASSERT (uploadData);
    return uploadData->image_data; 
  }

  csGLTextureHandle *get_parent () { return Parent; }
};

class csGLTextureHandle : public iTextureHandle
{
private:
  CS_LEAKGUARD_DECLARE(csGLTextureHandle);

  /// texturemanager handle
  csRef<csGLTextureManager> txtmgr;

  
  GLenum sourceFormat, targetFormat;
  GLenum sourceType; // what size does each fragment have? e.g. GL_UNSIGNED_BYTE

  /// 2D Canvas
  csRef<iGraphics2D> canvas;
  /// The transparent color
  csRGBpixel transp_color;
  
  /// Used until Prepare() is called
  csRef<iImageVector> images;

  /// Stores the names of the images
  csStringArray origNames;

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

  void *cachedata;

  bool Compressable () { return !texFlags.Check (CS_TEXTURE_2D); }
  bool transform (iImageVector *ImageVec, csGLTexture *tex);

  csGLTexture* NewTexture (iImage *Image, bool ismipmap);

  GLuint Handle;
  /// Upload the texture to GL.
  void Load ();
  void Unload ();
public:
  int formatidx;
  int orig_width, orig_height;
  csArray<csGLTexture*> vTex;
  csWeakRef<csGLGraphics3D> G3D;
  long size;
  int target;
  bool IsWasRenderTarget() const { return texFlags.Check (flagWasRenderTarget); }
  void SetWasRenderTarget (bool b) { texFlags.SetBool (flagWasRenderTarget, b); }

  csGLTextureHandle (iImage* image, int flags, int target, 
    csGLGraphics3D *iG3D);

  csGLTextureHandle (iImageVector* image, int flags, int target, 
    csGLGraphics3D *iG3D);

  csGLTextureHandle (int target, GLuint Handle, csGLGraphics3D *iG3D);

  virtual ~csGLTextureHandle ();

  GLenum SourceType () const { return sourceType; }
  GLenum SourceFormat () const { return sourceFormat; }
  GLenum TargetFormat () const { return targetFormat; }

  void Clear();

  void AdjustSizePo2 ();
  void CreateMipMaps();
  bool FindFormatType();
  void PrepareKeycolor (iImage* image, const csRGBpixel& transp_color,
    csAlphaMode::AlphaType& alphaType);
  void ComputeMeanColor (int w, int h, csRGBpixel *src, 
    const csRGBpixel* transp_color, csRGBpixel& mean_color);
  void CheckAlpha (int w, int h, csRGBpixel *src, 
    const csRGBpixel* transp_color, csAlphaMode::AlphaType& alphaType);
  csRef<iImageVector>& GetImages () { return images; }
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
  virtual bool GetMipMapDimensions (int mipmap, int &mw, int &mh);

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
  virtual bool GetMipMapDimensions (int mipmap, int &mw, int &mh, int &md);

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

  /**
   * Sets Texture Target. 
   * This can either be CS_TEX_IMG_1D, CS_TEX_IMG_2D, CS_TEX_IMG_3D, 
   * CS_TEX_IMG_CUBEMAP etc.
   * With CS_TEX_IMG_CUBEMAP, the depth index specifies the side 
   * of the cubemap (CS_TEXTURE_CUBE_POS_X, CS_TEXTURE_CUBE_NEG_X,
   * CS_TEXTURE_CUBE_POS_Y, etc.
   */
  virtual void SetTextureTarget(int target);

  /// Get the texture target
  virtual int GetTextureTarget () const { return target; }

  /// Get the original image name at the given depth
  virtual const char* GetImageName (int depth = 0) const;

  /// Get the mean color.
  virtual void GetMeanColor (uint8 &red, uint8 &green, uint8 &blue) const;

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
  virtual bool GetAlphaMap () const;

  /**
   * Get a canvas instance which is suitable for rendering on this
   * texture. Note that it is not allowed to change the palette of
   * the returned canvas.
   */
  virtual iGraphics2D* GetCanvas ();

  virtual csAlphaMode::AlphaType GetAlphaType () const
  { return alphaType; }

  virtual void Precache ();

  void UpdateTexture ();

  GLuint GetHandle ();
};

/*
*
* new MaterialHandle Implementation
* by Phil Aumayr (phil@rarebyte.com)
*
*/
class csGLMaterialHandle : public iMaterialHandle
{
protected:
  /// Original material.
  csRef<iMaterial> material;
  /// Texture handle, if created from one
  csRef<iTextureHandle> texture;
  /// Parent texture manager
  csGLTextureManager* texman;

public:
  CS_LEAKGUARD_DECLARE(csGLMaterialHandle);

  ///
  csGLMaterialHandle (iMaterial* material, csGLTextureManager *parent);
  ///
  csGLMaterialHandle (iTextureHandle* texture, csGLTextureManager *parent);
  ///
  virtual ~csGLMaterialHandle ();

  /// Release the original material (iMaterial).
  void FreeMaterial ();

  /// Get the material.
  iMaterial* GetMaterial () { return material; }

  //--------------------- iMaterialHandle implementation ----------------------
  SCF_DECLARE_IBASE;

  /**
   * Get associated shader
   */
  virtual iShader* GetShader (csStringID type);

  /**
   * Get a texture from the material.
   */
  virtual iTextureHandle *GetTexture ();

  /**
   * Get the flat color. If the material has a texture assigned, this
   * will return the mean texture color.
   */
  virtual void GetFlatColor (csRGBpixel &oColor);

  /**
   * Get light reflection parameters for this material.
   */
  virtual void GetReflection (float &oDiffuse, float &oAmbient,
    float &oReflection);
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

/*
*
* New Texture Manager... done by Phil Aumayr (phil@rarebyte.com)
*
*/
class csGLTextureManager : public iTextureManager
{
private:
  struct formatDescription
  {
    GLenum targetFormat;
    char *name;
    GLenum sourceFormat;
    int components; // number of components in texture
    GLint compressedFormat;
    GLenum forcedFormat;
    int texelbytes;
  };

  typedef csWeakRefArray<csGLTextureHandle> csTexVector;
  /// List of textures.
  csTexVector textures;
  // Private class used to keep a list of objects derived from csMaterialHandle
  typedef csWeakRefArray<csGLMaterialHandle> csMatVector;
  /// List of materials.
  csMatVector materials;

  csPixelFormat pfmt;

  void AlterTargetFormatForBits (GLenum target, int bits);
  void AlterTargetFormat (const char *oldTarget, const char *newTarget);

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

  static formatDescription glformats [];

  csGLTextureManager (iObjectRegistry* object_reg,
        iGraphics2D* iG2D, iConfigFile *config,
        csGLGraphics3D *G3D);

  virtual ~csGLTextureManager ();

  /// Read configuration values from config file.
  void read_config (iConfigFile *config);
  void Clear ();


  /**
   * Called from csMaterialHandle destructor to notify parent texture
   * manager that a material is going to be destroyed.
   */
  void UnregisterMaterial (csGLMaterialHandle* handle);

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
   * Register a texture. The given input image is IncRef'd and DecRef'ed
   * later when not needed any more. If you want to keep the input image
   * make sure you have called IncRef yourselves.
   */
  virtual csPtr<iTextureHandle> RegisterTexture (iImageVector *image,
  	int flags, int target);

  /**
   * Called from csGLTextureHandle destructor to notify parent texture
   * manager that a material is going to be destroyed.
   */
  void UnregisterTexture (csGLTextureHandle* handle);

  /**
   * Register a material. The input material is IncRef'd and DecRef'ed
   * later when FreeMaterials () is called or the material handle is destroyed
   * by calling DecRef on it enough times. If you want to keep the input
   * material make sure you have called IncRef yourselves. <p>
   *
   * The material is unregistered at destruction, i.e. as soon as the last
   * reference to the material handle is released.
   */
  virtual csPtr<iMaterialHandle> RegisterMaterial (iMaterial* material);

  /**
   * Register a material based on a texture handle. This is a short-cut
   * to quickly make materials based on a single texture.
   *
   * The material is unregistered at destruction, i.e. as soon as the last
   * reference to the material handle is released.
   */
  virtual csPtr<iMaterialHandle> RegisterMaterial (
  	iTextureHandle* txthandle);

  /**
   * Call this function if you want to release all iMaterial's as
   * given to this texture manager.
   */
  virtual void FreeMaterials ();

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

  virtual int GetMaterialIndex (iMaterialHandle* mat)
  {
    return materials.Find ((csGLMaterialHandle*)mat); // @@@ Evil cast?
  }

  /// Dump all SLMs to image files.
  void DumpSuperLightmaps (iVFS* VFS, iImageIO* iio, const char* dir);

  virtual void GetLightmapRendererCoords (int slmWidth, int slmHeight,
    int lm_x1, int lm_y1, int lm_x2, int lm_y2,
    float& lm_u1, float& lm_v1, float &lm_u2, float& lm_v2);
};

#define CS_GL_FORMAT_TABLE(var) \
csGLTextureManager::formatDescription var[] = {

#define CS_GL_FORMAT(dsttype, srctype, size, texelsize) \
{dsttype, #dsttype, srctype, size, 0, (GLenum)0, texelsize},

#define CS_GL_FORMAT_TABLE_END \
{(GLenum)0, 0, (GLenum)0, 0, 0, (GLenum)0, 0}};


#endif // __CS_GL_TXTMGR_H__

