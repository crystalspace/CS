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
#include "video/renderer/common/txtmgr.h"
#include "ivideo/itexture.h"
#include "iengine/itexture.h"

class csGraphics3DDirect3DDx6;
class csTextureDirect3D;
struct iImage;

/**
 * A mipmapped texture for Direct3D.
 */
class csTextureHandleDirect3D : public csTextureHandle
{
  csTextureDirect3D* m_pTexture2d;

public:
  /// A pointer to the 3D driver object
  csGraphics3DDirect3DDx6 *G3D;

  /// Initialize the object
  csTextureHandleDirect3D (iImage* image, int flags, csGraphics3DDirect3DDx6 *iG3D);

  /// Delete all members
  ~csTextureHandleDirect3D();
  /// Create a new texture object
  virtual csTexture *NewTexture (iImage *Image);
  /// Compute the mean color for the just-created texture
  virtual void ComputeMeanColor ();

  /// Create all mipmapped bitmaps from the first level.
  virtual void CreateMipmaps ();
  /// Prepare texture for further usage
  virtual void Prepare ();
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
  csTextureDirect3D (csTextureHandle *Parent, iImage *Image, csGraphics3DDirect3DDx6 *iG3D, bool For2d);
  /// Destroy the texture
  virtual ~csTextureDirect3D ();
  /// Get image data
  UByte *get_image_data ()
  { return image; }
};

/**
 * Texture manager for Direct3D driver.
 */
class csTextureManagerDirect3D : public csTextureManager
{
protected:
  /// Pointer to the 2D driver
  iGraphics2D*             m_pG2D;
  csGraphics3DDirect3DDx6* m_pG3D;

public:
  ///
  csTextureManagerDirect3D (iSystem*                 iSys, 
                            iGraphics2D*             iG2D, 
                            iConfigFile*             config,
                            csGraphics3DDirect3DDx6* iG3D);
  ///
  virtual ~csTextureManagerDirect3D ();

  /**
   * Return the index for some color. This works in 8-bit
   * (returns an index in the 256-color table) and in 15/16-bit
   * (returns a 15/16-bit encoded RGB value).
   */
  virtual int FindRGB (int r, int g, int b);

  ///
  virtual void PrepareTextures ();
  ///
  virtual iTextureHandle *RegisterTexture (iImage* image, int flags);
  ///
  virtual void UnregisterTexture (csTextureHandleDirect3D* handle);
};

#endif // TXTMGR_DIRECT3D_H
