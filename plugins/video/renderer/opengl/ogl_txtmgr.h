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

#ifndef TXTMGR_OPENGL_H
#define TXTMGR_OPENGL_H

#include "csutil/scf.h"
#include "video/renderer/common/txtmgr.h"
#include "ogl_proctexsoft.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "igraphic/image.h"

class csGraphics3DOGLCommon;
class csTextureManagerOpenGL;

enum csGLProcTexType
{
  SOFTWARE_TEXTURE = 1,
  BACK_BUFFER_TEXTURE = 2,
  AUXILIARY_BUFFER_TEXTURE = 3
};

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

  /// Create a csTexture object
  csTextureOpenGL (csTextureHandle *Parent, iImage *Image);
  /// Destroy the texture
  virtual ~csTextureOpenGL ();
  /// Get image data
  uint8 *&get_image_data ()
  { return image_data; }
  virtual bool Compressable (){ return true;}
};

/**
 * The procedural texture object.
 */
class csTextureProcOpenGL : public csTextureOpenGL
{
public:
  iGraphics3D *texG3D;
  csTextureProcOpenGL (csTextureHandle *Parent, iImage *image)
    : csTextureOpenGL (Parent, image), texG3D (NULL)
  {};
  /// Destroy the texture
  virtual ~csTextureProcOpenGL ();
  virtual bool Compressable (){ return false;}
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
  GLint sourceFormat, targetFormat;
  GLenum sourceType; // what size does each fragment have ? e.g. GL_UNSIGNED_BYTE
  int bpp; 
  bool FindFormatType ();
  bool transform (iImage *Image, csTextureOpenGL *tex);
  void ShowFormat ();

  class texVector : public csVector
  {
  public:
    csTextureOpenGL* operator [] (int idx) const { return (csTextureOpenGL*)csVector::Get (idx);}
    int Push (csTexture *t){ return csVector::Push ((csSome)t);}
  };
  
public:
  /// Retains original size of image before any readjustment
  int orig_width, orig_height;
  /// A pointer to the 3D driver object
  csGraphics3DOGLCommon *G3D;
  /// True if the texture has alphamap
  bool has_alpha;
  texVector vTex;
  long size;

  /// Initialize the object
  csTextureHandleOpenGL (iImage *image, int flags, GLint sourceFormat, int bpp, 
			 csGraphics3DOGLCommon *iG3D);
  /// Delete the texture object
  virtual ~csTextureHandleOpenGL ();
  /// Adjust size, mipmap, create procedural texture etc
  void InitTexture (csTextureManagerOpenGL *texman, csPixelFormat *pfmt);

  virtual bool csTextureHandleOpenGL::GetMipMapDimensions (int mipmap, int &w, int &h);

  /// Override from csTextureHandle.
  virtual void GetOriginalDimensions (int& w, int& h)
  {
    w = orig_width;
    h = orig_height;
  }

  /// Create a new texture object
  virtual csTexture *NewTexture (iImage *Image);
  /// Compute the mean color for the just-created texture
  virtual void ComputeMeanColor (){}
  void ComputeMeanColor (int w, int h, csRGBpixel *src);
  virtual void CreateMipmaps ();

  /// Returns dynamic texture interface if any.
  virtual iGraphics3D *GetProcTextureInterface ();
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
  GLint SourceFormat (){return sourceFormat;}
  GLint TargetFormat (){return targetFormat;}

  iImage *get_image (){return image;}
};

/**
 * OpenGL version of the texture manager.
 */
class csTextureManagerOpenGL : public csTextureManager
{

 protected:
  struct formatDescription 
  {
    GLint targetFormat;
    char *name;
    GLint sourceFormat;
    int components; // number of components in texture
    GLint compressedFormat;
    GLint forcedFormat;
    int texelbytes;
  };

  void AlterTargetFormat (const char *oldTarget, const char *newTarget);
  void DetermineStorageSizes ();

 public:
  /// A pointer to the 3D driver object
  csGraphics3DOGLCommon *G3D;
  int max_tex_size;
  csGLProcTexType proc_tex_type;
  // The first instance of a sharing software texture
  csOpenGLProcSoftware *head_soft_proc_tex;
  static formatDescription glformats [];
  ///
  csTextureManagerOpenGL (iObjectRegistry* object_reg,
			  iGraphics2D* iG2D, iConfigFile *config,
			  csGraphics3DOGLCommon *iG3D);
  ///
  virtual ~csTextureManagerOpenGL ();

  /// Called from G3D::Open ()
  void SetPixelFormat (csPixelFormat &PixelFormat);
  /// Called by texture's destructor
  void UnregisterTexture (csTextureHandleOpenGL *handle);
  /// Called by applications
  virtual void UnregisterTexture(iTextureHandle* handle)
  { UnregisterTexture((csTextureHandleOpenGL *)handle); }

  /// remove all textures and materials
  virtual void Clear ();

  /// Read configuration values from config file.
  virtual void read_config (iConfigFile *config);
  ///
  virtual void PrepareTextures ();
  ///
  virtual iTextureHandle *RegisterTexture (iImage* image, int flags);
};

#define CS_GL_FORMAT_TABLE(var) \
csTextureManagerOpenGL::formatDescription var[] = {

#define CS_GL_FORMAT(dsttype, srctype, size, texelsize) \
{dsttype, #dsttype, srctype, size, 0, 0, texelsize},

#define CS_GL_FORMAT_TABLE_END \
{0, NULL, (GLenum)0, 0, 0, 0, 0}};


#endif // TXTMGR_OPENGL_H
