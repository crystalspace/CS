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

#ifndef TCACHE_H
#define TCACHE_H

#include "types.h"
#include "cscom/com.h"

class csGraphics2D;
class MemoryHeap;
class TextureCache;
class csTextureManagerSoftware;
class csTextureMMSoftware;
class csTexture;

interface IPolygonTexture;
interface ILightMap;
struct csPixelFormat;

/// Structure used by the texture cache routines.
struct TCacheData
{
  int mipmap_shift, mipmap_size;
  // The coordinates (in lightmap space) of the block that we want
  // to recalculate (lu2 and lv2 are NOT included).
  int lu1, lv1, lu2, lv2;
  // Offset to use to go to the next line afer recreating one line.
  int d_lw;
  // Total width of the lightmap.
  int lw;
  // The lightmap data.
  UByte* mapR, * mapG, * mapB;
  // Dimensions of lighted texture.
  int width, height;
  // Offsets in lighted texture.
  int Imin_u, Imin_v;

  // Unlighted texture.
  csTextureMMSoftware* txt_mm;
  // Unlighted texture data.
  UByte* tdata;
  // Unlighted texture shifts.
  int shf_w, and_w, and_h;

  // Some global settings.
  int txtmode, mixing;
  bool lm_only, lm_grid;
};

/**
 * This structure represents a lighted texture as used
 * by the software texture cache.
 */
class TCacheLightedTexture
{
  friend class TextureCache;

private:
  /// Linked in the texture cache.
  TCacheLightedTexture* next;
  /// Linked in the texture cache.
  TCacheLightedTexture* prev;

  /// True if in the cache.
  bool in_cache;

  /// Size (in texels).
  long size;

  /// The lighted texture data.
  UByte* tmap;

  /**
   * tmap_m points to the real data inside 'tmap' (thus skipping the
   * H_MARGIN pixel border at the top of the texture map).
   */
  UByte* tmap_m;

public:
  ///
  TCacheLightedTexture () { tmap = tmap_m = NULL; next = prev = NULL; in_cache = false; }
  ///
  ~TCacheLightedTexture () { }

  ///
  UByte* get_tmap8 () { return tmap_m; }
  ///
  UShort* get_tmap16 () { return (UShort*)tmap_m; }
  ///
  ULong* get_tmap32 () { return (ULong*)tmap_m; }

  /// Unlink from list (set next and prev to NULL)
  void unlink_list () { next = prev = NULL; }
};

/**
 * This class implements the software texture cache
 * for 8-bit modes (16-bit mode overrides this class).
 */
class TextureCache
{
private:
  /// Total size of the cache (expressed in pixels).
  static int cache_size;
  /// The size that we finally used.
  int real_cache_size;

private:
  /// This is the first texture in the cache.
  TCacheLightedTexture* first;
  /// This is the last texture in the cache.
  TCacheLightedTexture* last;
  /// Total size of all textures in the cache.
  long total_size;
  /// Total number of textures in the cache.
  int total_textures;

  /**
   * The cache itself. This is a private pool of memory
   * used only for lighted textures.
   */
  MemoryHeap* memory;

  /**
   * Pointer to first free entry in pool.
   */
  //UByte* first_free;

  /**
   * Create a texture in the texture cache.
   * This routine will automatically select the right create_lighted_texture_???
   * depending on the mode Crystal Space is in.
   */
  virtual void create_lighted_texture (TCacheData& tcd, TCacheLightedTexture* pt, csTextureManagerSoftware* txtmgr);

  /// Create a texture in the texture cache (nocolor version).
  void create_lighted_texture_nocolor (TCacheData& tcd, TCacheLightedTexture* pt, csTextureManagerSoftware* txtmgr);
  /// Create a texture in the texture cache (true_rgb version).
  void create_lighted_texture_true_rgb (TCacheData& tcd, TCacheLightedTexture* pt, csTextureManagerSoftware* txtmgr);
  /// Create a texture in the texture cache (fast_wxx version).
  void create_lighted_texture_fast_wxx (TCacheData& tcd, TCacheLightedTexture* pt, csTextureManagerSoftware* txtmgr);
  /**
   * Create a texture in the texture cache (true_rgb version and for 'private'
   * colormap textures).
   */
  void create_lighted_texture_true_rgb_priv (TCacheData& tcd, TCacheLightedTexture* pt, csTextureManagerSoftware* txtmgr);

  /**
   * Initialize the memory pool.
   */
  void init_pool ();

  /**
   * Destroy the memory pool.
   */
  void destroy_pool ();

protected:
  /// The number of bytes per pixel.
  int gi_pixelbytes;

  /**
   * Allocate memory from the pool.
   * Returns NULL if no memory available.
   */
  void* alloc_pool (int size);

  /**
   * Delete memory from the pool.
   */
  void free_pool (void* mem, int size);

  /**
   * Initialize the TCacheData structure for create_lighted_texture_...
   * If u and v are given (not equal to -1) then the cache filler will only
   * update the texture in the texture cache for the given sub-texture containing
   * that (u,v) coordinate.
   * (u and v are in sub-texture space: blocks of subtex_size*subtex_size)
   */
  void init_cache_filler (TCacheData& tcd, IPolygonTexture* pt, csTextureManagerSoftware* txtmgr, int u = -1, int v = -1);

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
  TextureCache (csPixelFormat* pfmt);
  ///
  virtual ~TextureCache ();

  /// Clear the texture cache completely.
  void clear ();

  /**
   * Set the size of the texture cache.
   * If -1 this will simply initialize the texture cache. Note that
   * you should call 'set_cache_size' at least once.
   */
  void set_cache_size (long size);

  /// Set the default size of the texture cache.
  static void set_default_cache_size (long size) { cache_size = size; }

  /**
   * Check if the given texture is in the cache and possibly
   * add it if not.
   */
  void use_texture (IPolygonTexture* pt, csTextureManagerSoftware* txtmgr);

  /**
   * Check if the given sub-texture is in the cache and possibly
   * add it if not. WARNING! This function assumes that the texture is
   * already in the cache (put there with init_texture possibly).
   */
  void use_sub_texture (IPolygonTexture* pt, csTextureManagerSoftware* txtmgr, int u, int v);

  /**
   * Load a texture in the texture cache but do not do any calculations yet.
   * This is meant to be used in combination with use_sub_texture.
   * This function will also make sure that the dirty matrix is allocated
   * and has the right size. If changes need to be made here it will set
   * the dirty matrix to all dirty.
   */
  void init_texture (IPolygonTexture* pt, csTextureManagerSoftware* txtmgr);

  /**
   * Do a debugging dump.
   */
  void dump ();
};

#endif /*TCACHE_H*/
