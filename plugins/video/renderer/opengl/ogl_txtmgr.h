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
#include "itexture.h"
#include "iimage.h"

class csGraphics3DOGLCommon;
class csTextureManagerOpenGL;

enum csGLProcTexType
{
  SOFTWARE_TEXTURE = 1,
  BACK_BUFFER_TEXTURE = 2,
  AUXILIARY_BUFFER_TEXTURE = 3,
};

/**
 * csTextureOpenGL is a class derived from csTexture that implements
 * all the additional functionality required by the OpenGL renderer.
 * Every csTextureOpenGL is a RGBA paletted image.
 */
class csTextureOpenGL : public csTexture
{
protected:
  /// The actual image
  iImage *image;

public:
  /// Create a csTexture object
  csTextureOpenGL (csTextureHandle *Parent, iImage *Image);
  /// Destroy the texture
  virtual ~csTextureOpenGL ();
  /// Get image data
  csRGBpixel *get_image_data ()
  { return (csRGBpixel *)image->GetImageData (); }
  /// Get the image object
  iImage *get_image ()
  { return image; }
};

/**
 * This is a class derived from csTextureHandle that performs additional
 * functions required for texture management with OpenGL renderer.
 */
class csTextureHandleOpenGL : public csTextureHandle
{
public:
  /// Retains original size of image before any readjustment
  int orig_width, orig_height;
  /// A pointer to the 3D driver object
  csGraphics3DOGLCommon *G3D;
  /// True if the texture has alphamap
  bool has_alpha;

  /// Initialize the object
  csTextureHandleOpenGL (iImage *image, int flags, csGraphics3DOGLCommon *iG3D);
  /// Delete the texture object
  virtual ~csTextureHandleOpenGL ();
  /// Adjust size, mipmap, create procedural texture etc
  void InitTexture (csTextureManagerOpenGL *texman, csPixelFormat *pfmt);

  /// Create a new texture object
  virtual csTexture *NewTexture (iImage *Image);
  /// Compute the mean color for the just-created texture
  virtual void ComputeMeanColor ();
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
};

/**
 * The procedural texture object.
 */
class csTextureProcOpenGL : public csTextureOpenGL
{
public:
  iGraphics3D *texG3D;
  csTextureProcOpenGL (csTextureHandle *Parent, iImage *Image)
    : csTextureOpenGL (Parent, Image), texG3D (NULL)
  {};
  /// Destroy the texture
  virtual ~csTextureProcOpenGL ();
};

/**
 * OpenGL version of the texture manager.
 */
class csTextureManagerOpenGL : public csTextureManager
{
public:
  /// A pointer to the 3D driver object
  csGraphics3DOGLCommon *G3D;
  int max_tex_size;
  csGLProcTexType proc_tex_type;
  // The first instance of a sharing software texture
  csOpenGLProcSoftware *head_soft_proc_tex;

  ///
  csTextureManagerOpenGL (iSystem* iSys, iGraphics2D* iG2D, iConfigFileNew *config,
    csGraphics3DOGLCommon *iG3D);
  ///
  virtual ~csTextureManagerOpenGL ();

  /// Called from G3D::Open ()
  void SetPixelFormat (csPixelFormat &PixelFormat);
  /// Called by texture's destructor
  void UnregisterTexture (csTextureHandleOpenGL *handle);

  /// Read configuration values from config file.
  virtual void read_config (iConfigFileNew *config);
  ///
  virtual void PrepareTextures ();
  ///
  virtual iTextureHandle *RegisterTexture (iImage* image, int flags);
};

#endif // TXTMGR_OPENGL_H
