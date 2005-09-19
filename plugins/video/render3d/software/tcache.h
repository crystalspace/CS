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

#ifndef __CS_TCACHE_H__
#define __CS_TCACHE_H__

#include "cstypes.h"
#include "csutil/scf.h"
#include "soft_txt.h"

struct csPixelFormat;
struct iTextureHandle;
struct iRendererLightmap;
  
namespace cspluginSoft3d
{

class csSoftwareGraphics3DCommon;
class csSoftwareTextureManager;
class csSoftwareTextureHandle;
class csSoftRendererLightmap;

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
   * Last update number of unlit texture. This is used to see if the
   * texture changed and we need to flush this cache entry.
   */
  uint32 last_texture_number;

  /**
   * The lighted texture data. The beginning of this array
   * holds the lightmap used to generate this lighted texture.
   */
  uint8 *data;

  /**
   * bitmap points to the real data inside 'data' (thus skipping the
   * H_MARGIN pixel border at the top of the texture map).
   */
  uint8 *bitmap;

  /// The original polygon texture
  csSoftRendererLightmap* source;
  iTextureHandle* sourceTexH;

  /// Mipmap level for this texture
  int mipmap;

  /// The frame number this unit was allocated at
  int frameno;

  /// Initialize the lighted texture object
  SoftwareCachedTexture (int MipMap, 
    csSoftRendererLightmap* srlm, iTextureHandle* texh)
  {
    data = bitmap = 0; next = prev = 0;
    sourceTexH = texh;
    (source = srlm)->cacheData[mipmap = MipMap] = this;
    //(source = Source)->SetCacheData (mipmap = MipMap, this);
  }
  /// Destroy the lighted texture
  ~SoftwareCachedTexture ()
  {
	// BUG: Following code line causes an error upon exit
	//      in combination with procedural textures, when
	//      run in Windows. If line is removed it causes
	//	lock-ups in "appwalktest".
    //source->SetCacheData (mipmap, 0);
    source->cacheData[mipmap] = 0;

    delete [] data;
  }

  /// Get the pointer to the bitmap
  uint8 *get_bitmap ()
  { return bitmap; }

  /// Get the pointer to lightmap
  uint8 *get_lightmap ()
  { return data; }
};

/**
 * This class implements the software texture cache.
 */
class csSoftwareTextureCache
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
  csSoftwareTextureManager *texman;

public:
  void (csSoftwareTextureCache::*create_lighted_texture) (
			 csPolyTextureMapping* tmapping,
			 csSoftRendererLightmap* rlm, 
			 SoftwareCachedTexture *ct,
			 csSoftwareTextureHandle *texmm, 
			 csSoftwareTextureManager *texman,
		         float u_min, float v_min, float u_max, float v_max);

  void create_lighted_texture_8 (
				 csPolyTextureMapping* tmapping,
				 csSoftRendererLightmap* rlm, 
				 SoftwareCachedTexture *ct,
				 csSoftwareTextureHandle *texmm, 
				 csSoftwareTextureManager *texman,
				 float u_min, float v_min, 
				 float u_max, float v_max);

  void create_lighted_texture_555(
				  csPolyTextureMapping* tmapping,
				  csSoftRendererLightmap* rlm, 
				  SoftwareCachedTexture *ct,
				  csSoftwareTextureHandle *texmm, 
				  csSoftwareTextureManager *texman,
				  float u_min, float v_min, 
				  float u_max, float v_max);

  void create_lighted_texture_565(
				  csPolyTextureMapping* tmapping,
				  csSoftRendererLightmap* rlm, 
				  SoftwareCachedTexture *ct,
				  csSoftwareTextureHandle *texmm, 
				  csSoftwareTextureManager *texman,
				  float u_min, float v_min, 
				  float u_max, float v_max);

  void create_lighted_texture_888(
				  csPolyTextureMapping* tmapping,
				  csSoftRendererLightmap* rlm, 
				  SoftwareCachedTexture *ct,
				  csSoftwareTextureHandle *texmm, 
				  csSoftwareTextureManager *texman,
				  float u_min, float v_min, 
				  float u_max, float v_max);

  /// Current frame number
  int frameno;

  /// Initialize the texture cache
  csSoftwareTextureCache (csSoftwareTextureManager *TexMan);
  /// Destroy all lightmaps
  virtual ~csSoftwareTextureCache();

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
  SoftwareCachedTexture *cache_texture (int MipMap,      
    csPolyTextureMapping* tmapping,
    csSoftRendererLightmap* rlm,
    iTextureHandle* itexh);

  /// Remove a texture from the cache.
  void uncache_texture (int MipMap,
    csSoftRendererLightmap* rlm);

  /// Remove all cached textures dependent on iTextureHandle
  void uncache_texture (int MipMap, iTextureHandle *itexh);

  /**
   * Check if the given texture is in the cache and possibly
   * add it if not. The parts of lighted texture that were changed
   * will be recomputed.
   */
  void fill_texture (int MipMap,
		     csPolyTextureMapping* tmapping,
		     csSoftRendererLightmap* rlm, 
                     csSoftwareTextureHandle *tex_mm,
		     float u_min, float v_min, float u_max, float v_max);

  /**
   * Do a debugging dump.
   */
  void dump (csSoftwareGraphics3DCommon *iG3D);
};

} // namespace cspluginSoft3d

#endif // __CS_TCACHE_H__
