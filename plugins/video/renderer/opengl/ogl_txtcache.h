/*
    Copyright (C) 1998 by Jorrit Tyberghein and Dan Ogles.

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

#ifndef __GL_TEXTURECACHE_H__
#define __GL_TEXTURECACHE_H__

#include "csutil/scf.h"
#include <GL/gl.h>

struct iLightMap;
struct iTextureHandle;
struct iPolygonTexture;
struct iSystem;

///
struct csGLCacheData
{
  /// the size of this texture/lightmap
  long Size;
  /// iTextureHandle if texture, iLightMap if lightmap.
  void *Source;
  /// GL texture handle
  GLuint Handle;
  /// linked list
  csGLCacheData *next, *prev;
  /**
   * If lightmap then this contains the precalculated scale and offset
   * for the lightmap relative to the texture.
   */
  float lm_offset_u, lm_scale_u;
  float lm_offset_v, lm_scale_v;
};

///
enum csCacheType
{
  CS_TEXTURE, CS_LIGHTMAP
};

///
class OpenGLCache
{
protected:
  ///
  csCacheType type;
  /// the head and tail of the cache data
  csGLCacheData *head, *tail;

  /// the maximum size of the cache
  long cache_size;
  /// number of items
  int num;
  /// Total size of all loaded textures
  long total_size;

public:
  /// takes the maximum size of the cache
  OpenGLCache (int max_size, csCacheType type, int bpp);
  ///
  virtual ~OpenGLCache ();

  ///
  void cache_texture (iTextureHandle *texture);

  ///
  void cache_lightmap (iPolygonTexture *polytex);
  ///
  void Clear ();

  ///
  void ReportStats ();

protected:
  ///
  int bpp;

  ///
  virtual void Load (csGLCacheData *d, bool reload = false) = 0;
  ///
  void Unload (csGLCacheData *d);
};

///
class OpenGLTextureCache : public OpenGLCache
{
  bool rstate_bilinearmap;

public:
  ///
  OpenGLTextureCache (int size, int bitdepth);
  ///
  virtual ~OpenGLTextureCache ();

  ///
  void SetBilinearMapping (bool m) { rstate_bilinearmap = m; }
  ///
  bool GetBilinearMapping () { return rstate_bilinearmap; }
  /// Remove an individual texture from cache
  virtual void Uncache (iTextureHandle *texh);

protected:
  ///
  virtual void Load (csGLCacheData *d, bool reload = false);
};

///
class OpenGLLightmapCache : public OpenGLCache
{
public:
  ///
  OpenGLLightmapCache (int size, int bitdepth);
  ///
  virtual ~OpenGLLightmapCache ();

protected:
  ///
  virtual void Load (csGLCacheData *d, bool reload = false);
};

#endif // __GL_TEXTURECACHE_H__
