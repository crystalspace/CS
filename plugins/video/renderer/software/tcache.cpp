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

#include "sysdef.h"
#include "qint.h"
#include "csutil/garray.h"
#include "csutil/util.h"
#include "tcache.h"
#include "soft_g3d.h"
#include "soft_txt.h"
#include "ilghtmap.h"
#include "isystem.h"

//------------------------------------------------------------------------------

#define LM_NAME		create_lighted_texture_8
#define PI_INDEX8
#include "lightmap.inc"

#define LM_NAME		create_lighted_texture_555
#define PI_R5G5B5
#include "lightmap.inc"

#define LM_NAME		create_lighted_texture_565
#define PI_R5G6B5
#include "lightmap.inc"

#define LM_NAME		create_lighted_texture_888
#define PI_R8G8B8
#include "lightmap.inc"

//------------------------------------------------- csTextureCacheSoftware ---//

static void (*create_lighted_texture) (iPolygonTexture *pt,
  SoftwareCachedTexture *ct, csTextureManagerSoftware *texman,
  float u_min, float v_min, float u_max, float v_max);

csTextureCacheSoftware::csTextureCacheSoftware (csTextureManagerSoftware *TexMan)
{
  head = tail = NULL;
  texman = TexMan;
  Clear ();
  bytes_per_texel = texman->pfmt.PixelBytes;
  if (texman->pfmt.PixelBytes == 1)
    create_lighted_texture = create_lighted_texture_8;
  else if (texman->pfmt.PixelBytes == 2)
    if (texman->pfmt.GreenBits == 5)
      create_lighted_texture = create_lighted_texture_555;
    else
      create_lighted_texture = create_lighted_texture_565;
  else if (texman->pfmt.PixelBytes == 4)
    create_lighted_texture = create_lighted_texture_888;
  else
    abort (); // huh???
}

csTextureCacheSoftware::~csTextureCacheSoftware ()
{
  Clear ();
}

void csTextureCacheSoftware::set_cache_size (long size)
{
  Clear ();
  cache_size = size;
}

void csTextureCacheSoftware::Clear ()
{
  while (head)
  {
    SoftwareCachedTexture *n = head->next;
    delete head;
    head = n;
  }

  head = tail = NULL;
  total_size = 0;
  total_textures = 0;
}

SoftwareCachedTexture *csTextureCacheSoftware::cache_texture (bool verynice,
  int MipMap, iPolygonTexture* pt)
{
  SoftwareCachedTexture *cached_texture =
    (SoftwareCachedTexture *)pt->GetCacheData (MipMap);

  if (cached_texture)
  {
    // Texture is already in the cache.
    // Unlink texture and put it in front (MRU).
    if (cached_texture != head)
    {
      if (cached_texture->prev)
        cached_texture->prev->next = cached_texture->next;
      else
        head = cached_texture->next;
      if (cached_texture->next)
        cached_texture->next->prev = cached_texture->prev;
      else
        tail = cached_texture->prev;

      cached_texture->prev = NULL;
      cached_texture->next = head;
      if (head)
        head->prev = cached_texture;
      else
        tail = cached_texture;
      head = cached_texture;
    }
  }
  else
  {
    int mmshift = verynice && MipMap ? MipMap - 1 : MipMap;
    // Texture is not in the cache.
    int lightmap_size = pt->GetLightMap ()->GetSize () * sizeof (ULong);
    int bitmap_w = (pt->GetWidth () >> mmshift);
    int bitmap_h = ((pt->GetHeight () + (1 << mmshift) - 1) >> mmshift);
    int bitmap_size = lightmap_size + bytes_per_texel * bitmap_w * (H_MARGIN * 2 + bitmap_h);

    total_textures++;
    total_size += bitmap_size;

    // Free lightmaps until we have less than cache_size bytes for the cache
    while (tail && (total_size > cache_size))
    {
      // Total size of textures in cache is too high. Remove the last one.
      cached_texture = tail;
      tail = tail->prev;
      if (tail)
        tail->next = NULL;
      else
        head = NULL;

      total_textures--;
      total_size -= cached_texture->size;

      delete cached_texture;
    }

    CHK (cached_texture = new SoftwareCachedTexture (MipMap, pt));

    int margin_size = H_MARGIN * bitmap_w * bytes_per_texel;
    UByte *data = new UByte [bitmap_size];
    memset (data, 0, lightmap_size);
    cached_texture->data = data;
    cached_texture->bitmap = data + lightmap_size + margin_size; // Skip margin.
    cached_texture->size = bitmap_size;

    // Add new texture to cache.
    cached_texture->next = head;
    cached_texture->prev = NULL;
    if (head)
      head->prev = cached_texture;
    else
      tail = cached_texture;
    head = cached_texture;
  }
  return cached_texture;
}

void csTextureCacheSoftware::fill_texture (bool verynice, int MipMap,
  iPolygonTexture* pt, float u_min, float v_min, float u_max, float v_max)
{
  // Recalculate the lightmaps
  pt->RecalculateDynamicLights ();

  // Now cache the texture
  SoftwareCachedTexture *cached_texture = cache_texture (verynice, MipMap, pt);

  // Compute the rectangle on the lighted texture, if it is dirty
  create_lighted_texture (pt, cached_texture, texman, u_min, v_min, u_max, v_max);
}

#define SysPrintf iG3D->System->Printf

void csTextureCacheSoftware::dump (csGraphics3DSoftware *iG3D)
{
  SysPrintf (MSG_CONSOLE, "Textures in the cache: %d\n", total_textures);
  SysPrintf (MSG_CONSOLE, "Total size: %ld bytes\n", total_size);
  int mean = (total_textures == 0) ? 0 : total_size / total_textures;
  SysPrintf (MSG_CONSOLE, "Bytes per texture: %d\n", mean);
}
