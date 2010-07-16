/*
    Copyright (C) 1998-2006 by Jorrit Tyberghein
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
#include "igraphic/image.h"

class Vector2;
class csMatrix3;
class csVector3;
class csRect;

struct csRGBcolor;
struct csRGBpixel;
struct iImage;
struct iTextureHandle;
struct iMaterial;
struct iString;

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
/**
 * Texture resizing control flag.
 * If a texture needs to be resized to power-of-two dimensions (either because
 * non-power-of-two textures are not supported or just not requested), specify
 * that powers of two larger than or equal to the original dimensions should
 * be selected.
 * \sa #CS_TEXTURE_SCALE_DOWN
 * \remarks If none or both texture scale flags are specified, the renderer
 *   will decide whether a dimension is scaled up or down.
 */
#define CS_TEXTURE_SCALE_UP		0x00000080
/**
 * Texture resizing control flag.
 * If a texture needs to be resized to power-of-two dimensions (either because
 * non-power-of-two textures are not supported or just not requested), specify
 * that powers of two smaller than or equal to the original dimensions should
 * be selected.
 * \sa #CS_TEXTURE_SCALE_UP
 * \remarks If none or both texture scale flags are specified, the renderer
 *   will decide whether a dimension is scaled up or down.
 */
#define CS_TEXTURE_SCALE_DOWN		0x00000100
/**
 * Create cleared texture.
 * When creating a texture with iTextureManager::CreateTexture() its contents
 * are by default undefined. Setting this flag will clear the texture.
 * It only has an effect when used with iTextureManager::CreateTexture().
 */
#define CS_TEXTURE_CREATE_CLEAR		0x00000200
/**
 * Disable filtering across edges for a cubemap.
 * By default, cubemaps are seamless (when filtering near an edge of a face,
 * pixels from neighboring faces are sampled as well). However, this may be
 * undesireable in certain cases (e.g. lookup textures). In that case,
 * seamless filtering can be selectively disabled.
 * \remarks Seamless filtering is subject to hardware/driver support.
 *   On some hardware/drivers, seamless filtering is a global flag. "Disable
 *   seamless" takes precedence, so if one texture with seamless disabled
 *   is used, seamless filtering is disabled globally.
 */
#define CS_TEXTURE_CUBEMAP_DISABLE_SEAMLESS	0x00000400
/** @} */

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
  SCF_INTERFACE(iTextureManager, 4,0,0);
  /**
   * Register a texture. The given input image is IncRef'd and DecRef'ed
   * later when no longer needed. If you want to keep the input image
   * make sure you have called IncRef yourselves.
   *
   * The texture is not converted immediately. Instead, you can make
   * intermediate calls to iTextureHandle::SetKeyColor ().
   *
   * This function returns a handle which should be given
   * to the 3D rasterizer or 2D driver when drawing or otherwise using
   * the texture.
   *
   * The texture manager will reject the texture if it is an inappropiate
   * format (see GetTextureFormat () method).
   *
   * The texture is unregistered at destruction, i.e. as soon as the last
   * reference to the texture handle is released.
   *
   * Note! This function will NOT scale the texture to fit hardware
   * restrictions. This is done later before texture is first used.
   *
   * \param image is the source image.
   * \param flags contains one or several of CS_TEXTURE_XXX flags OR'ed
   *  together. They define the mode texture is going to be used in.
   * \param fail_reason is an optional string which will be filled with
   *  the reason for failure if there was a failure.
   * \return a new texture handle or 0 if the texture couldn't be
   *  created for some reason. The reason will be put in the optional
   *  'fail_reason' parameter.
   */
  virtual csPtr<iTextureHandle> RegisterTexture (iImage *image, int flags,
      iString* fail_reason = 0) = 0;

  /**
   * Create a new texture with the given texture format.
   * 
   * \param w Horizontal size of the texture.
   * \param h Vertical size of the texture.
   * \param imagetype Type of the image.
   * \param format A texture format string.
   * \param flags Contains one or several of CS_TEXTURE_XXX flags OR'ed
   *  together. They define the mode texture is going to be used in.
   * \param fail_reason An optional string which will be filled with
   *  the reason for failure if there was a failure.
   * \return A new texture handle or 0 if the texture couldn't be
   *  created for some reason. The reason will be put in the optional
   *  \a fail_reason parameter.
   * \sa \ref TextureFormatStrings 
   */
  virtual csPtr<iTextureHandle> CreateTexture (int w, int h,
      csImageType imagetype, const char* format, int flags,
      iString* fail_reason = 0) = 0;

  /**
   * Query the basic format of textures that can be registered with this
   * texture manager. It is very likely that the texture manager will
   * reject the texture if it is in an improper format. The alpha channel
   * is optional; the texture can have it and can not have it. Only the
   * bits that fit the CS_IMGFMT_MASK mask matters.
   */
  virtual int GetTextureFormat () = 0;
  
  /**
   * Request maximum texture dimensions.
   */
  virtual void GetMaxTextureSize (int& w, int& h, int& aspect) = 0;

  /**
   * Create a new texture with the given texture format.
   * 
   * \param w Horizontal size of the texture.
   * \param h Vertical size of the texture.
   * \param d Depth size of the texture (for 3D textures).
   * \param imagetype Type of the image.
   * \param format A texture format string.
   * \param flags Contains one or several of CS_TEXTURE_XXX flags OR'ed
   *  together. They define the mode texture is going to be used in.
   * \param fail_reason An optional string which will be filled with
   *  the reason for failure if there was a failure.
   * \return A new texture handle or 0 if the texture couldn't be
   *  created for some reason. The reason will be put in the optional
   *  \a fail_reason parameter.
   * \sa \ref TextureFormatStrings 
   */
  virtual csPtr<iTextureHandle> CreateTexture (int w, int h, int d,
      csImageType imagetype, const char* format, int flags,
      iString* fail_reason = 0) = 0;
};

/** @} */

#endif // __CS_IVIDEO_TXTMGR_H__
