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

#ifndef TCACHE32_H
#define TCACHE32_H

#include "types.h"
#include "cs3d/software/tcache.h"

struct csPixelFormat;

/**
 * This class implements the software texture cache for 32-bit mode.
 */
class TextureCache32 : public TextureCache
{
private:
  /**
   * Create a texture in the texture cache.
   */
  virtual void create_lighted_texture (TCacheData& tcd, TCacheLightedTexture* tclt, csTextureManagerSoftware* txtmgr);

  /// Create a texture in the texture cache (24bit version).
  void create_lighted_24bit (TCacheData& tcd, TCacheLightedTexture* tclt, csTextureManagerSoftware* txtmgr);
#if 0
  /// Create a texture in the texture cache (true_rgb version).
  void create_lighted_true_rgb (TCacheData& tcd, TCacheLightedTexture* tclt, csTextureManagerSoftware* txtmgr);
  /// Create a texture in the texture cache (true_rgb and private colormaps).
  void create_lighted_true_rgb_priv (TCacheData& tcd, TCacheLightedTexture* tclt, csTextureManagerSoftware* txtmgr);
#endif
  enum { PIX_RGB, PIX_BGR } pixmode;

protected:
  /**
   * For debugging: overlay the lightmap grid on the lighted texture. This function should
   * be called after calling create_lighted_texture ().
   */
  virtual void show_lightmap_grid (TCacheData& tcd, TCacheLightedTexture* tclt, csTextureManagerSoftware* txtmgr);

  /**
   * For debugging: don't add the texture but only the lightmap data.
   */
  virtual void create_lighted_texture_lightmaps (TCacheData& tcd, TCacheLightedTexture* tclt, csTextureManagerSoftware* txtmgr);

public:
  ///
  TextureCache32 (csPixelFormat* pfmt);
  ///
  virtual ~TextureCache32 ();
};

#endif /*TCACHE32_H*/
