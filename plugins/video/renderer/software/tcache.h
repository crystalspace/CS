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

#include "cstypes.h"
#include "csutil/scf.h"
#include "csutil/bitset.h"
#include "iengine/ipolygon.h"

class csGraphics3DSoftwareCommon;
class csTextureManagerSoftware;
class csTextureHandleSoftware;
struct csPixelFormat;
struct iTextureHandle;

/// The default texture cache size.
#define DEFAULT_CACHE_SIZE	8*1024*1024

/**
 * Define a small (3 pixels) margin at the bottom and top of
 * the texture. This is the easiest way I could find to 'fix' the
 * overflow problems with the texture mapper.
 */
#define H_MARGIN	3

/**
 * This structure represents a lighted texture as used
 * by the software texture cache.
 */
class SoftwareCachedTexture
{
public:

  /// Linked in the texture cache.
  SoftwareCachedTexture *next, *prev;

  /// Size (in bytes).
  long size;

  /**
   * The lighted texture data. The beginning of this array
   * holds the lightmap used to generate this lighted texture.
   */
  UByte *data;

  /**
   * bitmap points to the real data inside 'data' (thus skipping the
   * H_MARGIN pixel border at the top of the texture map).
   */
  UByte *bitmap;

  /// The original polygon texture
  iPolygonTexture *source;

  /// Mipmap level for this texture
  int mipmap;

  /// The frame number this unit was allocated at
  int frameno;

  /// Initialize the lighted texture object
  SoftwareCachedTexture (int MipMap, iPolygonTexture *Source)
  {
    data = bitmap = NULL; next = prev = NULL;
    (source = Source)->SetCacheData (mipmap = MipMap, this);
  }
  /// Destroy the lighted texture
  ~SoftwareCachedTexture ()
  {
	// BUG: Following code line causes an error upon exit
	//      in "appptlab", when run in Windows. If line is
	//      removed it causes lock-ups in "appwalktest"
    source->SetCacheData (mipmap, NULL);

    delete [] data;
  }

  /// Get the pointer to the bitmap
  UByte *get_bitmap ()
  { return bitmap; }

  /// Get the pointer to lightmap
  UByte *get_lightmap ()
  { return data; }
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

public:
  void (csTextureCacheSoftware::*create_lighted_texture) 
                        (iPolygonTexture *pt, SoftwareCachedTexture *ct, 
		   csTextureHandleSoftware *texmm, csTextureManagerSoftware *texman,
		         float u_min, float v_min, float u_max, float v_max);

  void create_lighted_texture_8 (iPolygonTexture *pt, SoftwareCachedTexture *ct,
                   csTextureHandleSoftware *texmm, csTextureManagerSoftware *texman,
		        float u_min, float v_min, float u_max, float v_max);

  void create_lighted_texture_555(iPolygonTexture *pt,SoftwareCachedTexture *ct,
                   csTextureHandleSoftware *texmm, csTextureManagerSoftware *texman,
		        float u_min, float v_min, float u_max, float v_max);

  void create_lighted_texture_565(iPolygonTexture *pt,SoftwareCachedTexture *ct,
                   csTextureHandleSoftware *texmm, csTextureManagerSoftware *texman,
		        float u_min, float v_min, float u_max, float v_max);

  void create_lighted_texture_888(iPolygonTexture *pt,SoftwareCachedTexture *ct,
                   csTextureHandleSoftware *texmm, csTextureManagerSoftware *texman,
		        float u_min, float v_min, float u_max, float v_max);

  /// Current frame number
  int frameno;

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
  SoftwareCachedTexture *cache_texture (int MipMap, iPolygonTexture* pt);

  /// Remove a texture from the cache.
  void uncache_texture (int MipMap, iPolygonTexture* pt);

  /// Remove all cached textures dependent on iTextureHandle
  void uncache_texture (int MipMap, iTextureHandle *itexh);

  /**
   * Check if the given texture is in the cache and possibly
   * add it if not. The parts of lighted texture that were changed
   * will be recomputed.
   */
  void fill_texture (int MipMap, iPolygonTexture* pt, 
		     csTextureHandleSoftware *tex_mm, 
		     float u_min, float v_min, float u_max, float v_max);

  /**
   * Do a debugging dump.
   */
  void dump (csGraphics3DSoftwareCommon *iG3D);
};

#endif // __TCACHE_H__
