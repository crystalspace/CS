/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Jorrit Tyberghein.

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

#ifndef __CS_IVIDEO_TXTMGR_H__
#define __CS_IVIDEO_TXTMGR_H__

/**\file
 * Texture manager interface
 */
 
/**
 * \addtogroup gfx3d
 * @{ */

#include "csutil/scf.h"

class Vector2;
class csMatrix3;
class csVector3;
class csRect;

struct csRGBpixel;
struct csLightMapMapping;
struct iImage;
struct iTextureHandle;
struct iMaterial;

/**\name Texture registration flags.
 * During texture registration you should tell
 * the manager which way you're going to use the texture: whenever you're
 * going to use it for 2D (DrawPixmap ()), for 3D (DrawPolygon ()), whenever
 * the texture will be dynamically modified.
 * @{ */
/// You're going to use the texture for 2D drawing
#define CS_TEXTURE_2D			0x00000001
/// You're going to use the texture for 3D drawing
#define CS_TEXTURE_3D			0x00000002
/**
 * Dither texture or not.
 * Some renderers may use dithering while converting textures to internal
 * format (say from truecolor to 8-bit paletted). For most textures dithering
 * won't give any visual effect, but very seldom there are textures that looks
 * relatively bad after being converted. In this case you can enable this
 * per-texture flag.
 */
#define CS_TEXTURE_DITHER		0x00000004
/**
 * Create mipmaps for this texture?
 * Sometimes we know in advance that some texture will need just one
 * mipmap (or we just don't care about the mipmapping artifacts because of,
 * say, how texture is looking (smoothed etc)). This flag is a <b>hint</b>
 * for texture manager so that it will know this.<p>
 * Note that if texture is not registered for 3D usage (i.e. if CS_TEXTURE_3D
 * is not set) this flag does not matter - 2D textures do not use mipmaps.
 */
#define CS_TEXTURE_NOMIPMAPS		0x00000008
/**
 * This texture will not be tiled, and color lookups outside the 0..1 range 
 * will be clamped to the edge of the texture.
 */
#define CS_TEXTURE_CLAMP		0x00000010
/**
 * This texture will not be filtered, even if texture filtering is available.
 */
#define CS_TEXTURE_NOFILTER		0x00000020
/**
 * Store texture as non-power-of-two sized (NPOTS) if possible.
 * \remarks On a lot of hardware, support for NPOTS-texture is rather limited;
 *  for example, commonly only clamped and textures without mipmaps are 
 *  supported, hence a texture may only be considered for NPOTS use if the 
 *  CS_TEXTURE_CLAMP and CS_TEXTURE_NOMIPMAPS flags are also set.
 *  Some hardware may not support NPOTS textures at all. It is at the 
 *  discretion of the renderer to decide whether a texture really becomes
 *  NPOTS or not. This flag serves merely as an expression of the wish that
 *  a texture should be NPOTS, if possible. If that cannot be granted, the 
 *  flag will be removed (thus you can query whether a texture was stored
 *  NPOTS or not by checking if this flag is set on it).
 */
#define CS_TEXTURE_NPOTS		0x00000040
/** @} */

SCF_VERSION (iRendererLightmap, 1, 1, 0);

/**
 * A lightmap registered with a renderer.
 */
struct iRendererLightmap : public iBase
{
  /**
   * Retrieve the coordinates of this lightmap in the superlightmap, in the
   * 'absolute' system used by iSuperLightmap::RegisterLightmap().
   */
  virtual void GetSLMCoords (int& left, int& top, 
    int& width, int& height) = 0;
    
  /// Set the image data of this lightmap.
  virtual void SetData (csRGBpixel* data) = 0;
  
  virtual void SetLightCellSize (int size) = 0;
};

SCF_VERSION (iSuperLightmap, 1, 0, 1);

/**
 * A super light map.
 */
struct iSuperLightmap : public iBase
{
  /// Add a lightmap to this SLM.
  virtual csPtr<iRendererLightmap> RegisterLightmap (int left, int top, 
    int width, int height) = 0;
    
  /// Retrieve an image of the whole SLM (for debugging purposes)
  virtual csPtr<iImage> Dump () = 0;

  virtual iTextureHandle* GetTexture () = 0;
};

/**
 * This is the standard texture manager interface.
 * A 3D rasterizer will have to implement a subclass of this one and
 * return a pointer to it in Graphics3D.
 * This class is responsible for receiving all textures
 * from the 3D engine, converting them to an internal format if
 * needed, calculating a palette if needed, and calculating all
 * lookup tables related to the textures. Mipmap creation is
 * also done in this class.
 *
 * Main creators of instances implementing this interface:
 * - The 3D renderers create an instance of this.
 *
 * Main ways to get pointers to this interface:
 * - iGraphics3D::GetTextureManager()
 */
struct iTextureManager : public virtual iBase
{
  SCF_INTERFACE(iTextureManager, 3,0,0);
  /**
   * Register a texture. The given input image is IncRef'd and DecRef'ed
   * later when no longer needed. If you want to keep the input image
   * make sure you have called IncRef yourselves.
   *<p>
   * The texture is not converted immediately. Instead, you can make
   * intermediate calls to iTextureHandle::SetKeyColor ().
   *<p>
   * This function returns a handle which should be given
   * to the 3D rasterizer or 2D driver when drawing or otherwise using
   * the texture.
   *<p>
   * The `flags' contains one or several of CS_TEXTURE_XXX flags OR'ed
   * together. They define the mode texture is going to be used in.
   *<p>
   * The texture manager will reject the texture if it is an inappropiate
   * format (see GetTextureFormat () method).
   *<p>
   * The texture is unregistered at destruction, i.e. as soon as the last
   * reference to the texture handle is released.
   *<p>
   * Param target specifies the texture target. Defines for that can be
   * found in ivideo/texture.h
   *<p>
   * Note! This function will NOT scale the texture to fit hardware
   * restrictions. This is done later before texture is first used.
   */
  virtual csPtr<iTextureHandle> RegisterTexture (iImage *image, int flags) = 0;

  /**
   * Query the basic format of textures that can be registered with this
   * texture manager. It is very likely that the texture manager will
   * reject the texture if it is in an improper format. The alpha channel
   * is optional; the texture can have it and can not have it. Only the
   * bits that fit the CS_IMGFMT_MASK mask matters.
   */
  virtual int GetTextureFormat () = 0;
  
  /**
   * Create a new super lightmap with the specified dimensions.
   */
  virtual csPtr<iSuperLightmap> CreateSuperLightmap (int width, 
    int height) = 0;

  /**
   * Request maximum texture dimensions.
   */
  virtual void GetMaxTextureSize (int& w, int& h, int& aspect) = 0;

  /**
   * Retrieve the coordinates of a lightmap in the its superlightmap, in a 
   * system the renderer uses internally. Calculate lightmap U/Vs within this
   * bounds when they are intended to be passed to the renderer.
   */
  virtual void GetLightmapRendererCoords (int slmWidth, int slmHeight,
    int lm_x1, int lm_y1, int lm_x2, int lm_y2,
    float& lm_u1, float& lm_v1, float &lm_u2, float& lm_v2) = 0;
};

/** @} */

#endif // __CS_IVIDEO_TXTMGR_H__
