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

#ifndef __TCACHE_H__
#define __TCACHE_H__

#include "types.h"
#include "csutil/scf.h"
#include "csutil/bitset.h"
#include "ipolygon.h"

class csGraphics3DSoftware;
class csTextureManagerSoftware;
struct csPixelFormat;

/// The default texture cache size.
#define DEFAULT_CACHE_SIZE	8*1024*1024

/**
 * This structure represents a lighted texture as used
 * by the software texture cache.
 */
class SoftwareCachedTexture
{
  friend class csTextureCacheSoftware;

  /// Linked in the texture cache.
  SoftwareCachedTexture *next, *prev;

  /// Size (in bytes).
  long size;

  /// The lighted texture data.
  UByte *data;

  /**
   * bitmap points to the real data inside 'data' (thus skipping the
   * H_MARGIN pixel border at the top of the texture map).
   */
  UByte *bitmap;

  /// The original polygon texture
  iPolygonTexture *source;

public:
  /// Initialize the lighted texture object
  SoftwareCachedTexture (iPolygonTexture *Source)
  {
    data = bitmap = NULL; next = prev = NULL;
    (source = Source)->SetCacheData (this);
  }
  /// Destroy the lighted texture
  ~SoftwareCachedTexture ()
  {
    source->SetCacheData (NULL);
    delete [] data;
  }

  /// Get the pointer to the bitmap
  inline UByte *get_bitmap ()
  { return bitmap; }
};

/**
 * This class implements the software texture cache
 * for 8-bit modes (16-bit mode overrides this class).
 */
class csTextureCacheSoftware
{
private:
  /// Total size of the cache (in pixels)
  int cache_size;
  /// Total size of all textures in the cache (in pixels)
  long total_size;
  /// Total number of textures in the cache.
  int total_textures;
  /// Bytes per texel
  int bytes_per_texel;

  /// This is the first texture in the cache.
  SoftwareCachedTexture *head;
  /// This is the last texture in the cache.
  SoftwareCachedTexture *tail;

protected:
  /// The texture manager
  csTextureManagerSoftware *texman;

  /// Debugging: show the lightmap without the texture
  void show_lightmap_grid (csBitSet *dirty, iPolygonTexture *pt,
    void *dst, csTextureManagerSoftware *texman);

public:
  /// Initialize the texture cache
  csTextureCacheSoftware (csTextureManagerSoftware *TexMan);
  /// Destroy all lightmaps
  virtual ~csTextureCacheSoftware ();

  /// Clear the texture cache completely.
  void Clear ();

  /**
   * Set the size of the texture cache and allocate the memory.
   * Note that you should call 'set_cache_size' at least once.
   */
  void set_cache_size (long size);

  /**
   * Add a texture to the texture cache. If the texture is already
   * there, just move it to the head of texture list (MRU). If the
   * overall size of all textures exceeds maximal cache size, the
   * least used texture is discarded.
   */
  SoftwareCachedTexture *cache_texture (iPolygonTexture* pt);

  /**
   * Load a texture in the texture cache but do not do any calculations yet.
   * This is meant to be used in combination with use_sub_texture.
   * This function will also make sure that the dirty matrix is allocated
   * and has the right size. If changes need to be made here it will set
   * the dirty matrix to all dirty.
   */
  void init_texture (iPolygonTexture* pt);

  /**
   * Check if the given texture is in the cache and possibly
   * add it if not.
   */
  void use_texture (iPolygonTexture* pt);

  /**
   * Check if the given sub-texture is in the cache and possibly
   * add it if not. WARNING! This function assumes that the texture is
   * already in the cache (put there with init_texture possibly).
   * The bit set contains a bit matrix with "1" in the positions that
   * should be updated.
   */
  void use_sub_texture (iPolygonTexture* pt, csBitSet *dirty);

  /**
   * Do a debugging dump.
   */
  void dump (csGraphics3DSoftware *iG3D);
};

#endif // __TCACHE_H__
