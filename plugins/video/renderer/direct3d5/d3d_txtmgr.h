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

struct iImage;

/**
 * A mipmapped texture for Direct3D.
 */
class csTextureMMDirect3D : public csTextureMM
{
public:
  csTextureMMDirect3D (iImage* image, int flags);
};

/**
 * Texture manager for Direct3D driver.
 */
class csTextureManagerDirect3D : public csTextureManager
{
private:
  /**
   * Encode RGB values to a 16-bit word (for 16-bit mode).
   */
  ULong encode_rgb (int r, int g, int b);

public:
  ///
  csTextureManagerDirect3D (iSystem* iSys, iGraphics2D* iG2D, csIniFile *config);
  ///
  virtual ~csTextureManagerDirect3D ();

  ///
  virtual void PrepareTextures ();
  ///
  virtual iTextureHandle *RegisterTexture (iImage* image, int flags);
  ///
  virtual void PrepareTexture (iTextureHandle *handle);
  ///
  virtual void UnregisterTexture (iTextureHandle* handle);

  /**
   * Remap all textures.
   */
  void remap_textures ();
};


#endif // TXTMGR_DIRECT3D_H

