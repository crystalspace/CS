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

#ifndef TXTMGR_DIRECT3D_H
#define TXTMGR_DIRECT3D_H

#include "csutil/scf.h"
#include "cs3d/common/txtmgr.h"
#include "itexture.h"

class csGraphics3DDirect3DDx6;
struct iImage;

/**
 * A mipmapped texture for Direct3D.
 */
class csTextureMMDirect3D : public csTextureMM
{
public:
  /// A pointer to the 3D driver object
  csGraphics3DDirect3DDx6 *G3D;

  /// Initialize the object
  csTextureMMDirect3D (iImage* image, int flags, csGraphics3DDirect3DDx6 *iG3D);
  /// Create a new texture object
  virtual csTexture *new_texture (iImage *Image);
  /// Compute the mean color for the just-created texture
  virtual void compute_mean_color ();
};

/**
 * csTextureDirect3D is a class derived from csTexture that implements
 * all the additional functionality required by the Direct3D renderer.
 * Every csTextureDirect3D is converted into the internal device format
 * during initialization.
 */
class csTextureDirect3D : public csTexture
{
  // The actual image (in device-dependent format)
  UByte *image;

public:
  /// Create a csTexture object
  csTextureDirect3D (csTextureMM *Parent, iImage *Image, csGraphics3DDirect3DDx6 *iG3D);
  /// Destroy the texture
  virtual ~csTextureDirect3D ();
  /// Return a pointer to texture data
  virtual void *get_bitmap ();
  /// Get image data
  UByte *get_image_data ()
  { return image; }
};

/**
 * Texture manager for Direct3D driver.
 */
class csTextureManagerDirect3D : public csTextureManager
{
public:
  /// Shift counters for converting R8G8B8 to internal texture format
  rsr, rsl, gsr, gsl, bsr, bsl;

  ///
  csTextureManagerDirect3D (iSystem* iSys, iGraphics2D* iG2D, csIniFile *config);
  ///
  virtual ~csTextureManagerDirect3D ();

  ///
  virtual void Clear ();

  ///
  virtual void PrepareTextures ();
  ///
  virtual iTextureHandle *RegisterTexture (iImage* image, int flags);
  ///
  virtual void PrepareTexture (iTextureHandle *handle);
  ///
  virtual void UnregisterTexture (iTextureHandle* handle);
};

#endif // TXTMGR_DIRECT3D_H
