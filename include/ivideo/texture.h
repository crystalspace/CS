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

#ifndef __CS_IVIDEO_TEXTURE_H__
#define __CS_IVIDEO_TEXTURE_H__

/**\file
 * Texture handle interface
 */
 
/**
 * \addtogroup gfx3d
 * @{ */
 
#include "csutil/scf.h"
#include "cstypes.h"
#include "ivideo/graph3d.h"

struct iGraphics2D;
struct iGraphics3D;

SCF_VERSION (iTextureHandle, 2, 2, 3);

/**
 * A texture handle as returned by iTextureManager.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iTextureManager::RegisterTexture()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>iTextureWrapper::GetTextureHandle()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>3D renderer implementations (iGraphics3D).
 *   </ul>
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

  // CHANGED TO ADD SUPPORT FOR CUBEMAPS AND 3D TEXTURES
  // done by Phil Aumayr (phil@rarebyte.com)
  enum { CS_TEX_IMG_1D = 0, CS_TEX_IMG_2D, CS_TEX_IMG_3D, CS_TEX_IMG_CUBEMAP };
  /**
   * Texture Depth Indices are used for Cubemap interface
   */
  enum { CS_TEXTURE_CUBE_POS_X = 0, CS_TEXTURE_CUBE_NEG_X, 
         CS_TEXTURE_CUBE_POS_Y, CS_TEXTURE_CUBE_NEG_Y,
         CS_TEXTURE_CUBE_POS_Z, CS_TEXTURE_CUBE_NEG_Z };

#ifdef CS_USE_NEW_RENDERER
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
  virtual bool GetMipMapDimensions (int mipmap, int &mw, int &mh, int &md) = 0;

  /**
   * Return the original dimensions of the image used to create this texture.
   * This is most often equal to GetMipMapDimensions (0, mw, mh, md) but in
   * some cases the texture will have been resized in order to accomodate
   * hardware restrictions (like power of two and maximum texture size).
   * This function returns the uncorrected coordinates.
   */
  virtual void GetOriginalDimensions (int& mw, int& mh, int &md) = 0;

  /**
   * Sets Texture Target. 
   * This can either be CS_TEXTURE_1D, CS_TEXTURE_2D, CS_TEXTURE_3D, 
   * CS_TEXTURE_CUBEMAP etc.
   * With CS_TEXTURE_CUBEMAP, the depth index specifies the side 
   * of the cubemap (CS_TEXTURE_CUBE_POS_X, CS_TEXTURE_CUBE_NEG_X,
   * CS_TEXTURE_CUBE_POS_Y, etc.
   */
  virtual void SetTextureTarget (int target) = 0;
#endif // CS_USE_NEW_RENDERER

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
   * Query if the texture has an alpha channel.<p>
   * This depends both on whenever the original image had an alpha channel
   * and of the fact whenever the renderer supports alpha maps at all.
   */
  virtual bool GetAlphaMap () = 0;

  /**
   * Get a canvas instance which is suitable for rendering on this
   * texture. Note that it is not allowed to change the palette of
   * the returned canvas.
   */
  virtual iGraphics2D* GetCanvas () = 0;

  virtual csAlphaMode::AlphaType GetAlphaType () = 0;

  /**
   * Precache this texture. This might free up temporary memory and
   * makes later usage of the texture faster.
   */
  virtual void Precache () = 0;
};

/** @} */

#endif // __CS_IVIDEO_TEXTURE_H__
