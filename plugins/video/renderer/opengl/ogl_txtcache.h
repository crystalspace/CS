/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
    Copyright (C) 1998 by Dan Ogles.

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
#include "csgeom/csrect.h"
#include <GL/gl.h>

struct iLightMap;
struct iTextureHandle;
struct iPolygonTexture;
struct iSystem;
class csSubRectangles;

/**
 * Cache element for a texture. This element will be stored
 * in the OpenGL texture cache and is also kept with the polygon
 * itself.
 */
struct csTxtCacheData
{
  /// The size of this texture.
  long Size;
  /// iTextureHandle.
  iTextureHandle* Source;
  /// GL texture handle.
  GLuint Handle;
  /// Linked list.
  csTxtCacheData *next, *prev;
};

/**
 * This is the OpenGL texture cache.
 */
class OpenGLTextureCache
{
protected:
  bool rstate_bilinearmap;

  /// the head and tail of the cache data
  csTxtCacheData *head, *tail;

  /// the maximum size of the cache
  long cache_size;
  /// number of items
  int num;
  /// Total size of all loaded textures
  long total_size;

public:
  /// Takes the maximum size of the cache.
  OpenGLTextureCache (int max_size, int bpp);
  ///
  ~OpenGLTextureCache ();

  /// Make sure this texture is active in OpenGL.
  void Cache (iTextureHandle *texture);
  /// Remove an individual texture from cache.
  void Uncache (iTextureHandle *texh);

  /// Clear the cache.
  void Clear ();

  ///
  void SetBilinearMapping (bool m) { rstate_bilinearmap = m; }
  ///
  bool GetBilinearMapping () { return rstate_bilinearmap; }

protected:
  int bpp;

  /// Really load the texture in OpenGL memory.
  void Load (csTxtCacheData *d, bool reload = false);
  ///
  void Unload (csTxtCacheData *d);
};

#define SUPER_LM_SIZE 256
#define SUPER_LM_NUM 10

/**
 * Cache data for lightmap. This is stored in one of the super
 * lightmaps and also kept with the lightmap itself.
 */
struct csLMCacheData
{
  /// iLightMap.
  iLightMap* Source;
  /// GL texture handle.
  GLuint Handle;
  /// Linked list.
  csLMCacheData *next, *prev;
  /**
   * This contains the precalculated scale and offset
   * for the lightmap relative to the texture.
   */
  float lm_offset_u, lm_scale_u;
  float lm_offset_v, lm_scale_v;

  /**
   * The following is a rectangle in the larger
   * super lightmap. super_lm_idx is the index of the super lightmap.
   */
  csRect super_lm_rect;
  int super_lm_idx;
};

/**
 * This class represents a super-lightmap.
 * A super-lightmap is a collection of smaller lightmaps that
 * fit together in one big texture.
 */
class csSuperLightMap
{
public:
  /// A class holding all the free regions in this texture.
  csSubRectangles* region;
  /// An OpenGL texture handle.
  GLuint Handle;
  /// the head and tail of the cache data
  csLMCacheData *head, *tail;

  csSuperLightMap ();
  ~csSuperLightMap ();

  /// Try to allocate a lightmap here. Return NULL on failure.
  csLMCacheData* Alloc (int w, int h);
  /// Clear all lightmaps in this super lightmap.
  void Clear ();
};

/**
 * Cache for OpenGL lightmaps. This cache keeps a number of
 * super lightmaps. Every super lightmaps holds a number of lightmaps.
 */
class OpenGLLightmapCache
{
private:
  /// A number of super-lightmaps to contain all other lightmaps.
  csSuperLightMap suplm[SUPER_LM_NUM];	// @@@ Make configurable.
  /// Current super lightmap.
  int cur_lm;
  /// If true then setup is ok.
  bool initialized;

  void Load (csLMCacheData *d);
  void Setup ();

public:
  ///
  OpenGLLightmapCache ();
  ///
  ~OpenGLLightmapCache ();

  /// Cache a lightmap.
  void Cache (iPolygonTexture *polytex);
  /// Clear the entire lightmap cache.
  void Clear ();
};

#endif // __GL_TEXTURECACHE_H__

