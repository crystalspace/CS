/*
    Copyright (C) 1998, 2000 by Jorrit Tyberghein

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

#ifndef __IVIDEO_TEXTURE_H__
#define __IVIDEO_TEXTURE_H__

#include "csutil/scf.h"
#include "cstypes.h"

struct iGraphics3D;

SCF_VERSION (iTextureHandle, 2, 1, 1);

/**
 * A texture handle as returned by iTextureManager.
 */
struct iTextureHandle : public iBase
{
  /// Retrieve the flags set for this texture
  virtual int GetFlags () = 0;

  /// Enable key color
  virtual void SetKeyColor (bool Enable) = 0;

  /// Set the key color.
  virtual void SetKeyColor (uint8 red, uint8 green, uint8 blue) = 0;

  /// Get the key color status (false if disabled, true if enabled).
  virtual bool GetKeyColor () = 0;

  /// Get the key color
  virtual void GetKeyColor (uint8 &red, uint8 &green, uint8 &blue) = 0;

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
  virtual bool GetMipMapDimensions (int mipmap, int &mw, int &mh) = 0;

  /**
   * Return the original dimensions of the image used to create this texture.
   * This is most often equal to GetMipMapDimensions (0, mw, mh) but in
   * some cases the texture will have been resized in order to accomodate
   * hardware restrictions (like power of two and maximum texture size).
   * This function returns the uncorrected coordinates.
   */
  virtual void GetOriginalDimensions (int& mw, int& mh) = 0;

  /// Get the mean color.
  virtual void GetMeanColor (uint8 &red, uint8 &green, uint8 &blue) = 0;

  /// Get data associated internally with this texture by texture cache
  virtual void *GetCacheData () = 0;

  /// Set data associated internally with this texture by texture cache
  virtual void SetCacheData (void *d) = 0;

  /**
   * Query the private object associated with this handle.
   * For internal usage by the 3D driver.
   */
  virtual void *GetPrivateObject () = 0;

  /**
   * If the texture handle was created with as a procedural texture, this
   * function returns an iGraphics3D interface to a texture buffer which
   * can be used in the  same way as a frame buffer based iGraphics3D.
   * This interface only becomes available once the texture has been
   * prepared by the texture manager.
   */
  virtual iGraphics3D *GetProcTextureInterface () = 0;

  /**
   * If this is a procedural texture with mip-mapping on sync enabled, call
   * this function to update its mip maps when required.  (Currently
   * unimplemented)
   */
  virtual void ProcTextureSync () = 0;

  /**
   * Query if the texture has an alpha channel.<p>
   * This depends both on whenever the original image had an alpha channel
   * and of the fact whenever the renderer supports alpha maps at all.
   */
  virtual bool GetAlphaMap () = 0;

  /**
   * Merge this texture into current palette, compute mipmaps and so on.
   * You should call either Prepare() or iTextureManager::PrepareTextures()
   * before using any texture.
   */
  virtual void Prepare () = 0;
};

#endif // __IVIDEO_TEXTURE_H__
