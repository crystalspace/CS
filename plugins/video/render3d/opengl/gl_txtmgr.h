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

#ifndef __CS_GL_TXTMGR_H__
#define __CS_GL_TXTMGR_H__

#include "csutil/scf.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "igraphic/image.h"

#include "video/render3d/common/txtmgr.h"

#include "gl_render3d.h"
#include "glextmanager.h"

class csGLTextureManager;

/**
 * csTextureOpenGL is a class derived from csTexture that implements
 * all the additional functionality required by the OpenGL renderer.
 * Every csTextureOpenGL is a RGBA paletted image.
 */
class csGLTexture : public csTexture
{
public:
  /// The actual image
  uint8 *image_data;
  GLint compressed;
  GLint internalFormat;
  GLint size;

  /// Create a csTexture object
  csGLTexture (csTextureHandle *Parent, iImage *Image);
  /// Destroy the texture
  virtual ~csGLTexture ();
  /// Get image data
  uint8 *&get_image_data ()
  { return image_data; }
  virtual bool Compressable (){ return true;}
};

/**
 * This is a class derived from csTextureHandle that performs additional
 * functions required for texture management with OpenGL renderer.
 */
class csGLTextureHandle : public csTextureHandle
{
protected:
  csGLTextureManager *txtmgr;
  int formatidx;
  GLenum sourceFormat, targetFormat;
  GLenum sourceType; // what size does each fragment have? e.g. GL_UNSIGNED_BYTE
  int bpp;
  /// Reference to internal canvas.
  csRef<iGraphics2D> canvas;

  bool FindFormatType ();
  bool transform (iImage *Image, csGLTexture *tex);
  void ShowFormat ();


  class texVector : public csVector
  {
  public:
    csGLTexture* operator [] (int idx) const
    { return (csGLTexture*)csVector::Get (idx);}
    int Push (csTexture *t){ return csVector::Push ((csSome)t);}
  };

public:
  /// Retains original size of image before any readjustment
  int orig_width, orig_height;
  /// A pointer to the 3D driver object
  csGLRender3D *R3D;
  /// True if the texture has alphamap
  bool has_alpha;
  texVector vTex;
  long size;
  /// true if texture has already been used as a render target
  bool was_render_target;

  /// Initialize the object
  csGLTextureHandle (iImage *image, int flags, GLenum sourceFormat,
    int bpp, csGLRender3D *iR3D);
  /// Delete the texture object
  virtual ~csGLTextureHandle ();
  /// Adjust size, mipmap, create procedural texture etc
  void InitTexture (csGLTextureManager *texman, csPixelFormat *pfmt);

  virtual bool csGLTextureHandle::GetMipMapDimensions (int mipmap,
    int &w, int &h);

  /// Override from csTextureHandle.
  virtual void GetOriginalDimensions (int& w, int& h)
  {
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

  virtual void Prepare ();
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

/**
 * OpenGL version of the texture manager.
 */
class csGLTextureManager : public csTextureManager
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
  csGLRender3D *R3D;
  int max_tex_size;
  /// Sharpen mipmaps?
  int sharpen_mipmaps;
  /// downsample textures?
  int texture_downsample;
  static formatDescription glformats [];
  ///
  csGLTextureManager (iObjectRegistry* object_reg,
        iGraphics2D* iG2D, iConfigFile *config,
        csGLRender3D *R3D);
  ///
  virtual ~csGLTextureManager ();

  /// Called from G3D::Open ()
  void SetPixelFormat (csPixelFormat &PixelFormat);
  /// Called by texture's destructor
  void UnregisterTexture (csGLTextureHandle *handle);

  /// remove all textures and materials
  virtual void Clear ();

  /// Read configuration values from config file.
  virtual void read_config (iConfigFile *config);
  ///
  virtual void PrepareTextures ();
  ///
  virtual csPtr<iTextureHandle> RegisterTexture (iImage* image, int flags);

  /** 
   * Free all images associated with textures
   * (ensures that all software proctexes have the needed textures) 
   */
  virtual void FreeImages ();
};

#define CS_GL_FORMAT_TABLE(var) \
csGLTextureManager::formatDescription var[] = {

#define CS_GL_FORMAT(dsttype, srctype, size, texelsize) \
{dsttype, #dsttype, srctype, size, 0, (GLenum)0, texelsize},

#define CS_GL_FORMAT_TABLE_END \
{(GLenum)0, NULL, (GLenum)0, 0, 0, (GLenum)0, 0}};


#endif // __CS_GL_TXTMGR_H__
