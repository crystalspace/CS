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
#include "cs3d/common/txtmgr.h"
#include "itexture.h"
#include "iimage.h"

class csGraphics3DOpenGL;

/**
 * This is a class derived from csTextureMM that performs additional
 * functions required for texture management with OpenGL renderer.
 */
class csTextureMMOpenGL : public csTextureMM
{
public:
  /// A pointer to the 3D driver object
  csGraphics3DOpenGL *G3D;

  /// Initialize the object
  csTextureMMOpenGL (iImage *image, int flags, csGraphics3DOpenGL *iG3D);
  /// Delete the texture object
  virtual ~csTextureMMOpenGL ();
  /// Create a new texture object
  virtual csTexture *NewTexture (iImage *Image);
  /// Compute the mean color for the just-created texture
  virtual void ComputeMeanColor ();
};

/**
 * csTextureOpenGL is a class derived from csTexture that implements
 * all the additional functionality required by the OpenGL renderer.
 * Every csTextureOpenGL is a RGBA paletted image.
 */
class csTextureOpenGL : public csTexture
{
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
 * OpenGL version of the texture manager.
 */
class csTextureManagerOpenGL : public csTextureManager
{
public:
  /// A pointer to the 3D driver object
  csGraphics3DOpenGL *G3D;

  ///
  csTextureManagerOpenGL (iSystem* iSys, iGraphics2D* iG2D, csIniFile *config,
    csGraphics3DOpenGL *iG3D);
  ///
  virtual ~csTextureManagerOpenGL ();

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
