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
#include "itexture.h"
#include "iimage.h"

class csGraphics3DOGLCommon;
class csOpenGLDynamic;

enum csGLDynTexType
{
  SOFTWARE_TEXTURE = 1,
  BACK_BUFFER_TEXTURE = 2,
  AUXILIARY_BUFFER_TEXTURE = 3,
};

/**
 * This is a class derived from csTextureMM that performs additional
 * functions required for texture management with OpenGL renderer.
 */
class csTextureMMOpenGL : public csTextureMM
{
public:
  /// Retains original size of image before any readjustment
  int orig_width, orig_height;
  /// A pointer to the 3D driver object
  csGraphics3DOGLCommon *G3D;

  /// Initialize the object
  csTextureMMOpenGL (iImage *image, int flags, csGraphics3DOGLCommon *iG3D);
  /// Delete the texture object
  virtual ~csTextureMMOpenGL ();
  /// Create a new texture object
  virtual csTexture *NewTexture (iImage *Image);
  /// Adjust size, mipmap etc
  void InitTexture (int max_tex_size, csPixelFormat *pfmt, 
		    csGLDynTexType dyn_tex_type);
  /// Compute the mean color for the just-created texture
  virtual void ComputeMeanColor ();


  void CreateDynamicTexture(csGraphics3DOGLCommon *parentG3D, 
			    csGLDynTexType type, csPixelFormat *PixelFormat);

  virtual iGraphics3D *GetDynamicTextureInterface ();
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
  csTextureOpenGL (csTextureMM *Parent, iImage *Image);
  /// Destroy the texture
  virtual ~csTextureOpenGL ();
  /// Get image data
  RGBPixel *get_image_data ()
  { return (RGBPixel *)image->GetImageData (); }
  /// Get the image object
  iImage *get_image ()
  { return image; }
};

/**
 * csTextureOpenGLDynamic is a class derived from csTextureOpenGL that
 * implements the additional functionality to allow acess to the texture
 * memory via iGraphics2D/3D interfaces.
 */
class csTextureOpenGLDynamic : public csTextureOpenGL
{
public:
  iGraphics3D *texG3D;

  csTextureOpenGLDynamic (csTextureMM *Parent, iImage *Image)
    : csTextureOpenGL (Parent, Image), texG3D (NULL)
  {};
  /// Destroy the texture
  virtual ~csTextureOpenGLDynamic ();

  void CreateInterfaces (csTextureMMOpenGL *mm_tex, csGLDynTexType type,
			 csGraphics3DOGLCommon *parentG3D, csPixelFormat *pfmt);
};

/**
 * OpenGL version of the texture manager.
 */
class csTextureManagerOpenGL : public csTextureManager
{
public:
  /// A pointer to the 3D driver object
  csGraphics3DOGLCommon *G3D;
  csGLDynTexType dyn_tex_type;
  int max_tex_size;

  ///
  csTextureManagerOpenGL (iSystem* iSys, iGraphics2D* iG2D, csIniFile *config,
    csGraphics3DOGLCommon *iG3D);
  ///
  virtual ~csTextureManagerOpenGL ();
  /// Read configuration values from config file.
  virtual void read_config (csIniFile *config);
  ///
  virtual void PrepareTextures ();
  ///
  virtual iTextureHandle *RegisterTexture (iImage* image, int flags);
  ///
  virtual void PrepareTexture (iTextureHandle *handle);
  ///
  virtual void UnregisterTexture (iTextureHandle* handle);
};

#endif // TXTMGR_OPENGL_H
