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

#ifndef __CS_TXTMGR_OPENGL_H__
#define __CS_TXTMGR_OPENGL_H__

#include "csutil/scf.h"
#include "csutil/blockallocator.h"
#include "csgeom/csrect.h"
#include "plugins/video/renderer/common/txtmgr.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "igraphic/image.h"

class csGraphics3DOGLCommon;
class csTextureManagerOpenGL;

struct iVFS;
struct iImageIO;

/**
 * csTextureOpenGL is a class derived from csTexture that implements
 * all the additional functionality required by the OpenGL renderer.
 * Every csTextureOpenGL is a RGBA paletted image.
 */
class csTextureOpenGL : public csTexture
{
public:
  /// The actual image
  uint8 *image_data;
  GLint compressed;
  GLint internalFormat;
  GLint size;
  csTextureHandle *parent;

  /// Create a csTexture object
  csTextureOpenGL (csTextureHandle *Parent, iImage *Image);
  /// Destroy the texture
  virtual ~csTextureOpenGL ();
  /// Get image data
  uint8 *&get_image_data ()
  { return image_data; }
  virtual bool Compressable (){ return !(parent->GetFlags() & CS_TEXTURE_2D); }
};

/**
 * This is a class derived from csTextureHandle that performs additional
 * functions required for texture management with OpenGL renderer.
 */
class csTextureHandleOpenGL : public csTextureHandle
{
protected:
  csTextureManagerOpenGL *txtmgr;
  int formatidx;
  GLenum sourceFormat, targetFormat;
  GLenum sourceType; // what size does each fragment have? e.g. GL_UNSIGNED_BYTE
  int bpp;
  /// Reference to internal canvas.
  csRef<iGraphics2D> canvas;
  bool prepared;

  bool FindFormatType ();
  bool transform (iImage *Image, csTextureOpenGL *tex);
  void ShowFormat ();
  void PrepareKeycolor ();

public:
  csRef<iImage>& GetImage () { return image; }
  bool& GetTransp () { return transp; }

  /// Retains original size of image before any readjustment
  int orig_width, orig_height;
  /// A pointer to the 3D driver object
  csGraphics3DOGLCommon *G3D;
  /// True if the texture has alphamap
  bool has_alpha;
  csArray<csTextureOpenGL*> vTex;
  long size;
  /// true if texture has already been used as a render target
  bool was_render_target;

  /// Initialize the object
  csTextureHandleOpenGL (iImage *image, int flags, GLenum sourceFormat,
    int bpp, csGraphics3DOGLCommon *iG3D);
  /// Delete the texture object
  virtual ~csTextureHandleOpenGL ();
  /// Adjust size, mipmap, create procedural texture etc
  void InitTexture (csTextureManagerOpenGL *texman, csPixelFormat const* pfmt);

  virtual bool GetMipMapDimensions (int mipmap, int &w, int &h);

  /// Override from csTextureHandle.
  virtual void GetOriginalDimensions (int& w, int& h)
  {
    PrepareInt ();
    w = orig_width;
    h = orig_height;
  }

  /// Create a new texture object
  virtual csTexture *NewTexture (iImage *Image, bool ismipmap);
  /// Compute the mean color for the just-created texture
  virtual void ComputeMeanColor (){}
  void ComputeMeanColor (int w, int h, csRGBpixel *src);
  virtual void CreateMipmaps ();

  /// Prepare the texture for usage
  void PrepareInt ();

  /**
   * Query if the texture has an alpha channel.<p>
   * This depends both on whenever the original image had an alpha channel
   * and of the fact whenever the renderer supports alpha maps at all.
   */
  virtual bool GetAlphaMap ()
  { return has_alpha; }

  /// maybe we are an proc texture and have additionally things to free
  virtual void Clear ();

  GLenum SourceType (){return sourceType;}
  GLenum SourceFormat (){return sourceFormat;}
  GLenum TargetFormat (){return targetFormat;}

  iImage *get_image (){return image;}

  /**
   * Get canvas for texture.
   */
  virtual iGraphics2D* GetCanvas ();

  void UpdateTexture ();
};

class csGLSuperLightmap;

/**
 * A single lightmap on a super lightmap.
 */
class csGLRendererLightmap : public iRendererLightmap
{
  friend class csGLSuperLightmap;
  friend class csGraphics3DOGLCommon;
  friend class csTriangleArrayPolygonBuffer;

  /// Texture coordinates (in pixels)
  csRect rect;
  /// The SLM this lightmap is a part of.
  csRef<csGLSuperLightmap> slm;
  /// Raw lightmap data.
  csRGBpixel* data;

  /// Has the mean light of this LM been calculated?
  bool mean_calculated;
  /// Mean light of this LM
  float mean_r, mean_g, mean_b;

public:
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

  /// Get the mean light of this LM.
  void GetMeanColor (float& r, float& g, float& b);
};

/**
 * An OpenGL super lightmap.
 */
class csGLSuperLightmap : public iSuperLightmap
{
  friend class csGLRendererLightmap;

  /// Number of lightmaps on this SLM.
  int numRLMs;

  /// Actually create the GL texture.
  void CreateTexture ();
  /**
   * Free a lightmap. Will also delete the GL texture if no LMs are on 
   * this SLM.
   */
  void FreeRLM (csGLRendererLightmap* rlm);
public:
  /// GL texture handle
  GLuint texHandle;
  /// Dimensions of this SLM
  int w, h;
  /// Remove the GL texture.
  void DeleteTexture ();

  /// The texture manager that created this SLM.
  csTextureManagerOpenGL* txtmgr;

  SCF_DECLARE_IBASE;

  csGLSuperLightmap (csTextureManagerOpenGL* txtmgr, int width, int height);
  virtual ~csGLSuperLightmap ();

  /// Add a lightmap.
  virtual csPtr<iRendererLightmap> RegisterLightmap (int left, int top, 
    int width, int height);

  /// Dump the contents onto an image.
  virtual csPtr<iImage> Dump ();
  
  virtual iTextureHandle* GetTexture ();
};

/**
 * OpenGL version of the texture manager.
 */
class csTextureManagerOpenGL : public csTextureManager
{
protected:
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

  void AlterTargetFormat (const char *oldTarget, const char *newTarget);
  void DetermineStorageSizes ();
public:
  /// A pointer to the 3D driver object
  csGraphics3DOGLCommon *G3D;
  int max_tex_size;
  /// Sharpen mipmaps?
  int sharpen_mipmaps;
  /// downsample textures?
  int texture_downsample;
  /// texture filtering anisotropy
  float texture_filter_anisotropy;
  /// what bpp should textures have?
  int texture_bits;

  /// All SLMs currently in use.
  csArray<csGLSuperLightmap*> superLMs;

  static formatDescription glformats [];
  ///
  csTextureManagerOpenGL (iObjectRegistry* object_reg,
        iGraphics2D* iG2D, iConfigFile *config,
        csGraphics3DOGLCommon *iG3D);
  ///
  virtual ~csTextureManagerOpenGL ();

  /// Called from G3D::Open ()
  void SetPixelFormat (csPixelFormat const& PixelFormat);
  /// Called by texture's destructor
  void UnregisterTexture (csTextureHandleOpenGL *handle);

  /// remove all textures and materials
  virtual void Clear ();

  /// Read configuration values from config file.
  virtual void read_config (iConfigFile *config);
  ///
  virtual csPtr<iTextureHandle> RegisterTexture (iImage* image, int flags);
  virtual csPtr<iTextureHandle> RegisterTexture (iImageVector*, int, int)
  {
    return 0;
  }

  /** 
   * Free all images associated with textures
   * (ensures that all software proctexes have the needed textures) 
   */
  virtual void FreeImages ();

  /// Create a new super light map.
  virtual csPtr<iSuperLightmap> CreateSuperLightmap(int w, int h);
  
  /// Retrieve maximum texture size.
  virtual void GetMaxTextureSize (int& w, int& h, int& aspect);

  /// Dump all SLMs to image files.
  void DumpSuperLightmaps (iVFS* VFS, iImageIO* iio, const char* dir);

  virtual void GetLightmapRendererCoords (int slmWidth, int slmHeight,
    int lm_x1, int lm_y1, int lm_x2, int lm_y2,
    float& lm_u1, float& lm_v1, float &lm_u2, float& lm_v2);
};

#define CS_GL_FORMAT_TABLE(var) \
csTextureManagerOpenGL::formatDescription var[] = {

#define CS_GL_FORMAT(dsttype, srctype, size, texelsize) \
{dsttype, #dsttype, srctype, size, 0, (GLenum)0, texelsize},

#define CS_GL_FORMAT_TABLE_END \
{(GLenum)0, 0, (GLenum)0, 0, 0, (GLenum)0, 0}};


#endif // __CS_TXTMGR_OPENGL_H__
