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

#include <math.h>

#include "sysdef.h"
#include "cs3d/software/tcache.h"
#include "cs3d/software/soft_txt.h"
#include "cs3d/common/memheap.h"
#include "ipolygon.h"
#include "ilghtmap.h"
#include "igraph2d.h"
#include "igraph3d.h"

//---------------------------------------------------------------------------

int TextureCache::cache_size = DEFAULT_CACHE_SIZE;

void TextureCache::init_pool ()
{
  destroy_pool ();
  real_cache_size = cache_size*gi_pixelbytes;
  real_cache_size = (real_cache_size+7)&~7;		// Round to higher multiple of 8
  CHK (memory = new MemoryHeap (real_cache_size));
}

void TextureCache::destroy_pool ()
{
  CHK (delete memory); memory = NULL;
}

void TextureCache::set_cache_size (long size)
{
  if (size != -1) cache_size = size;
  init_pool ();
  clear ();
}


void dump_pool (MemoryHeap *h)
{
  (void)h;
//@@@TEMPORARILY DISABLED
#if 0
  ULong totsize = sizeof(heap);
  ULong cur;
 
  CsPrintf (MSG_STDOUT,"\nStart      : %-8d\n", ((heap *)h->memory)->start);
  CsPrintf (MSG_STDOUT,  "First free : %-8d\n", ((heap *)h->memory)->first_free);
  CsPrintf (MSG_STDOUT,  "End        : %-8d\n", ((heap *)h->memory)->end);
  CsPrintf (MSG_STDOUT,  "Size       : %d\n",   h->cache_size);
 
  CsPrintf (MSG_STDOUT, "\nSTATE   SIZE    OFFSET  ADDRESS\n");
  for (cur = ((heap *)h->memory)->start;
       cur <= ((heap *)h->memory)->end;
       cur = (ULong)(cur + ((bufhd *)(h->memory + (ULong)(cur)))->size))
  {
    CsPrintf (MSG_STDOUT, "%-6d  %-6d  %-6d  %-8X\n",
              ((bufhd *)(h->memory + (ULong)(cur)))->full,
              ((bufhd *)(h->memory + (ULong)(cur)))->size, cur,
              ((char *)h->memory + (ULong)(cur) + sizeof(bufhd)));
    totsize += ((bufhd *)(h->memory + (ULong)(cur)))->size;
    if (!((bufhd *)(h->memory + (ULong)(cur)))->size) break;
  }
 
  CsPrintf (MSG_STDOUT, "\nTotal size : %d\n", totsize);
  CsPrintf (MSG_STDOUT,   "Missing    : %d\n", h->cache_size-totsize);
#endif
}

void* TextureCache::alloc_pool (int size)
{
  return memory->alloc (size);
}

void TextureCache::free_pool (void* mem, int /*dummy*/)
{
  memory->free (mem);
}

TextureCache::TextureCache (csPixelFormat* /*pfmt*/)
{
  first = last = NULL;
  memory = NULL;
  gi_pixelbytes = 1;
}

TextureCache::~TextureCache ()
{
  destroy_pool ();
}

void TextureCache::clear ()
{
  while (first)
  {
    TCacheLightedTexture* n = first->next;
    first->unlink_list ();
    first->in_cache = false;
    if (first->tmap)
    {
      free_pool ((void*)(first->tmap), first->size * gi_pixelbytes);
      first->tmap = first->tmap_m = NULL;
    }
    //@@@
    //if (first->dirty_matrix)
    //{
      //CHK (delete [] first->dirty_matrix);
      //first->dirty_matrix = NULL;
    //}
    first = n;
  }

  first = last = NULL;
  total_size = 0;
  total_textures = 0;
}

void TextureCache::dump ()
{
//@@@
  //CsPrintf (MSG_CONSOLE, "Textures in the cache: %d\n", total_textures);
  //CsPrintf (MSG_CONSOLE, "Total size: %ld bytes\n", total_size);
  int mean;
  if (total_textures == 0) mean = 0;
  else mean = total_size/total_textures;
  //CsPrintf (MSG_CONSOLE, "Bytes per texture: %d\n", mean);
  //int cnt_dirty = 0, cnt_clean = 0;
  //@@@
  //PolyTexture* pt = first;
  //while (pt)
  //{
    //cnt_dirty += pt->count_dirty_subtextures ();
    //cnt_clean += pt->count_clean_subtextures ();
    //pt = pt->get_next ();
  //}
  //CsPrintf (MSG_CONSOLE, "Sub-textures: dirty=%d clean=%d\n", cnt_dirty, cnt_clean);
}

void TextureCache::init_texture (IPolygonTexture* pt, csTextureManagerSoftware* /*txtmgr*/)
{
  IPolygon3D* i_pol;
  ILightMap* i_lm;
  pt->GetPolygon (&i_pol);
  i_pol->GetLightMap (&i_lm);
  if (!i_lm) return;	// @@@ See if this test can be avoided.

  pt->CreateDirtyMatrix ();

  void* rc;
  pt->GetTCacheData (&rc);
  TCacheLightedTexture* tclt = (TCacheLightedTexture*)rc;
  if (!tclt)
  {
    CHK (tclt = new TCacheLightedTexture ()); //@@@ Memory leak: there is no cleanup yet!
    pt->SetTCacheData ((void*)tclt);
  }

  bool recalc;
  pt->RecalculateDynamicLights (recalc);

  if (recalc && tclt->in_cache)
  {
    // Texture is in cache and we just recalculated the dynamic lighting
    // information. Just return.
    bool opt;
    pt->GetDynlightOpt (opt);
    if (!opt) pt->MakeAllDirty ();
    return;
  }

  if (tclt->in_cache)
  {
    // Texture is already in the cache.

    // Unlink texture and put it in front (MRU).
    if (tclt != first)
    {
      if (tclt->prev) tclt->prev->next = tclt->next;
      else first = tclt->next;
      if (tclt->next) tclt->next->prev = tclt->prev;
      else last = tclt->prev;

      tclt->prev = NULL;
      tclt->next = first;
      if (first) first->prev = tclt;
      else last = tclt;
      first = tclt;
    }
  }
  else
  {
    // Texture is not in the cache.
    long pt_size, size;
    int w;
    pt->GetSize (pt_size);
    pt->GetWidth (w);

    size = pt_size * gi_pixelbytes;
    UByte* map;
    while ((map = (UByte*)alloc_pool (size)) == NULL)
    {
      // Total size of textures in cache is too high. Remove the last one.
      TCacheLightedTexture* l = last;
      last = last->prev;
      if (last) last->next = NULL;
      else first = NULL;
      l->prev = NULL;
      l->in_cache = false;
      if (l->tmap)
      {        
        free_pool ((void*)(l->tmap), l->size * gi_pixelbytes);
	l->tmap = NULL;
	l->tmap_m = NULL;
      }
      total_textures--;
      total_size -= size;
    }
    tclt->tmap = map;
    tclt->tmap_m = map+H_MARGIN*w*gi_pixelbytes; // Skip margin.
    tclt->size = pt_size;
    total_textures++;
    total_size += size;
    pt->MakeAllDirty ();

    // Add new texture to cache.
    tclt->next = first;
    tclt->prev = NULL;
    if (first) first->prev = tclt;
    else last = tclt;
    first = tclt;
    tclt->in_cache = true;
  }
}

void TextureCache::use_sub_texture (IPolygonTexture* pt, csTextureManagerSoftware* txtmgr, int u, int v)
{
  IPolygon3D* i_pol;
  ILightMap* i_lm;
  pt->GetPolygon (&i_pol);
  i_pol->GetLightMap (&i_lm);
  if (!i_lm) return;	// @@@ See if this test can be avoided.

  int w, h;
  pt->GetWidth (w);
  pt->GetHeight (h);

  if (u < 0) u = 0; else if (u >= w) u = w-1;
  if (v < 0) v = 0; else if (v >= h) v = h-1;
  int subtex_size;
  pt->GetSubtexSize (subtex_size);
  int lu = u / subtex_size;
  int lv = v / subtex_size;
  bool was_dirty;
  pt->CleanIfDirty (lu, lv, was_dirty);
  if (was_dirty)
  {
    TCacheData tcd;
    init_cache_filler (tcd, pt, txtmgr, lu, lv);
    void* rc;
    pt->GetTCacheData (&rc);
    create_lighted_texture (tcd, (TCacheLightedTexture*)rc, txtmgr);
  }
}

void TextureCache::use_texture (IPolygonTexture* pt, csTextureManagerSoftware* txtmgr)
{
  IPolygon3D* i_pol;
  ILightMap* i_lm;
  pt->GetPolygon (&i_pol);
  i_pol->GetLightMap (&i_lm);
  if (!i_lm) return;	// @@@ See if this test can be avoided.

  void* rc;
  pt->GetTCacheData (&rc);
  TCacheLightedTexture* tclt = (TCacheLightedTexture*)rc;
  if (!tclt)
  {
    CHK (tclt = new TCacheLightedTexture ()); //@@@ Memory leak: there is no cleanup yet!
    pt->SetTCacheData ((void*)tclt);
  }

  bool recalc;
  pt->RecalculateDynamicLights (recalc);

  if (recalc && tclt->in_cache)
  {
    // Just recalculate the texture.
    TCacheData tcd;
    init_cache_filler (tcd, pt, txtmgr);
    create_lighted_texture (tcd, tclt, txtmgr);
    return;
  }

  if (tclt->in_cache)
  {
    // Texture is already in the cache.

    // Unlink texture and put it in front (MRU).
    if (tclt != first)
    {
      if (tclt->prev) tclt->prev->next = tclt->next;
      else first = tclt->next;
      if (tclt->next) tclt->next->prev = tclt->prev;
      else last = tclt->prev;

      tclt->prev = NULL;
      tclt->next = first;
      if (first) first->prev = tclt;
      else last = tclt;
      first = tclt;
    }
  }
  else
  {
    // Texture is not in the cache.
    long pt_size;
    int w;
    pt->GetSize (pt_size);
    pt->GetWidth (w);
    long size = pt_size * gi_pixelbytes;
    UByte* map;
    while ((map = (UByte*)alloc_pool (size)) == NULL)
    {
      // Total size of textures in cache is too high. Remove the last one.
      TCacheLightedTexture* l = last;
      last = last->prev;
      if (last) last->next = NULL;
      else first = NULL;
      l->prev = NULL;
      l->in_cache = false;
      if (l->tmap)
      {
        free_pool ((void*)(l->tmap), l->size * gi_pixelbytes);
	l->tmap = NULL;
	l->tmap_m = NULL;
      }
      total_textures--;
      total_size -= size;
    }
    tclt->tmap = map;
    tclt->tmap_m = map+H_MARGIN*w*gi_pixelbytes; // Skip margin.
    tclt->size = pt_size;
    total_textures++;
    total_size += size;

    // Add new texture to cache.
    tclt->next = first;
    tclt->prev = NULL;
    if (first) first->prev = tclt;
    else last = tclt;
    first = tclt;
    tclt->in_cache = true;

    TCacheData tcd;
    init_cache_filler (tcd, pt, txtmgr);
    create_lighted_texture (tcd, tclt, txtmgr);
  }
}


void TextureCache::create_lighted_texture (TCacheData& tcd, TCacheLightedTexture* tclt,
	csTextureManagerSoftware* txtmgr)
{
  if (tcd.lm_only) create_lighted_texture_lightmaps (tcd, tclt, txtmgr);
  else if (tcd.txtmode == TXT_PRIVATE) create_lighted_true_rgb_priv (tcd, tclt, txtmgr);
  else
    switch (tcd.mixing)
    {
      case MIX_NOCOLOR: create_lighted_nocolor (tcd, tclt, txtmgr); break;
      case MIX_TRUE_RGB: create_lighted_true_rgb (tcd, tclt, txtmgr); break;
    }
  if (tcd.lm_grid) show_lightmap_grid (tcd, tclt, txtmgr);
}

void TextureCache::init_cache_filler (TCacheData& tcd, IPolygonTexture* pt, csTextureManagerSoftware* txtmgr, int stu, int stv)
{
  tcd.txtmode = txtmgr->txtMode;
  tcd.mixing = txtmgr->mixing;
  tcd.lm_only = txtmgr->do_lightmaponly;
  tcd.lm_grid = txtmgr->do_lightmapgrid;

  ILightMap* i_lm;
  pt->GetLightMap (&i_lm);
  pt->GetWidth (tcd.width);
  pt->GetHeight (tcd.height);
  pt->GetMipMapSize (tcd.mipmap_size);
  pt->GetMipMapShift (tcd.mipmap_shift);
  pt->GetIMinU (tcd.Imin_u);
  pt->GetIMinV (tcd.Imin_v);

  //@@@ CAN BE MORE OPTIMAL?
  ITextureHandle* handle;
  pt->GetTextureHandle (&handle);
  tcd.txt_mm = (csTextureMMSoftware*)GetcsTextureMMFromITextureHandle (handle);
  int mm;
  pt->GetMipmapLevel (mm);

  csTexture* txt_unl = tcd.txt_mm->get_texture (mm);
  tcd.tdata = txt_unl->get_bitmap8 ();
  int uw, uh;
  uw = txt_unl->get_width ();
  uh = txt_unl->get_height ();
  tcd.shf_w = txt_unl->get_w_shift ();
  tcd.and_w = txt_unl->get_w_mask ();
  tcd.and_h = txt_unl->get_h_mask ();

  int lh;
  i_lm->GetWidth (tcd.lw);
  i_lm->GetHeight (lh);
  i_lm->GetMap (0, &tcd.mapR);
  i_lm->GetMap (1, &tcd.mapG);
  i_lm->GetMap (2, &tcd.mapB);
  int lw_orig;
  pt->GetOriginalWidth (lw_orig);
  lw_orig = (lw_orig>>tcd.mipmap_shift)+2;

  if (stu == -1 || stv == -1)
  {
    // Just calculate everything.
    tcd.lu1 = 0;
    tcd.lv1 = 0;
    tcd.lu2 = lw_orig-1;
    tcd.lv2 = lh-1;
  }
  else
  {
    // Calculate only one sub-texture.
    int subtex_size;
    pt->GetSubtexSize (subtex_size);
    int lu = (stu*subtex_size) >> tcd.mipmap_shift;
    int lv = (stv*subtex_size) >> tcd.mipmap_shift;
    tcd.lu1 = lu;
    tcd.lv1 = lv;
    tcd.lu2 = lu+(subtex_size>>tcd.mipmap_shift);
    tcd.lv2 = lv+(subtex_size>>tcd.mipmap_shift);
    if (tcd.lu2 > lw_orig-1) tcd.lu2 = lw_orig-1;
    if (tcd.lu2 > tcd.lw-1) tcd.lu2 = tcd.lw-1;
    if (tcd.lv2 > lh-1) tcd.lv2 = lh-1;
  }
  tcd.d_lw = tcd.lw-(tcd.lu2-tcd.lu1);
}

void TextureCache::create_lighted_nocolor (TCacheData& tcd,
  TCacheLightedTexture* tclt, csTextureManagerSoftware* txtmgr)
{
  int w = tcd.width;
  int h = tcd.height;
  int Imin_u = tcd.Imin_u;
  int Imin_v = tcd.Imin_v;

  unsigned char* map = tcd.mapR;
  unsigned char* otmap = tcd.tdata;
  int shf_w = tcd.shf_w;
  int and_w = tcd.and_w;
  int and_h = tcd.and_h;
  and_h <<= shf_w;

  unsigned char* tm, * tm2;
  int u, v, end_u, uu;

  unsigned char* light;

  int o_idx, ov_idx;

  int whi_00, whi_10, whi_01, whi_11;
  int whi_0, whi_1, whi_d, whi_0d, whi_1d;
  int whi;

  TextureTablesWhite8* w8 = txtmgr->lt_white8;
  RGB8map* wl = w8->white2_light;

  int lu, lv, luv, dv;
  luv = tcd.lv1 * tcd.lw + tcd.lu1;
  for (lv = tcd.lv1 ; lv < tcd.lv2 ; lv++)
  {
    for (lu = tcd.lu1 ; lu < tcd.lu2 ; lu++)
    {
      whi_00 = map[luv];
      whi_10 = map[luv+1];
      whi_01 = map[luv+tcd.lw];
      whi_11 = map[luv+tcd.lw+1];

      u = lu<<tcd.mipmap_shift;
      v = lv<<tcd.mipmap_shift;
      tm = &tclt->get_tmap8 ()[w*v+u];	//@@@

      if (whi_00 == whi_10 && whi_00 == whi_01 && whi_00 == whi_11)
      {
	//*****
	// Constant level of white light.
	//*****

	light = wl[whi_00];

	for (dv = 0 ; dv < tcd.mipmap_size ; dv++, tm += w-tcd.mipmap_size)
	  if (v+dv < h)
	  {
	    ov_idx = ((v+dv+Imin_v)<<shf_w) & and_h;

	    end_u = u+tcd.mipmap_size;
	    if (end_u > w) end_u = w;
	    end_u += Imin_u;
	    tm2 = tm + tcd.mipmap_size;
	    for (uu = u+Imin_u ; uu < end_u ; uu++)
	    {
	      o_idx = ov_idx + (uu & and_w);
	      *tm++ = light[otmap[o_idx]];
	    }
	    tm = tm2;
	  }
	  else break;
      }
      else
      {
	//*****
	// A varying level of white light.
	//*****

	whi_0 = whi_00 << 16; whi_0d = ((whi_01-whi_00)<<16) >> tcd.mipmap_shift;
	whi_1 = whi_10 << 16; whi_1d = ((whi_11-whi_10)<<16) >> tcd.mipmap_shift;

	for (dv = 0 ; dv < tcd.mipmap_size ; dv++, tm += w-tcd.mipmap_size)
	  if (v+dv < h)
	  {
	    ov_idx = ((v+dv+Imin_v)<<shf_w) & and_h;
	    whi = whi_0; whi_d = (whi_1-whi_0) >> tcd.mipmap_shift;

	    end_u = u+tcd.mipmap_size;
	    if (end_u > w) end_u = w;
	    end_u += Imin_u;
	    tm2 = tm + tcd.mipmap_size;
	    for (uu = u+Imin_u ; uu < end_u ; uu++)
	    {
	      light = wl[whi >> 16];
	      o_idx = ov_idx + (uu & and_w);
	      *tm++ = light[otmap[o_idx]];
	      whi += whi_d;
	    }
	    tm = tm2;

	    whi_0 += whi_0d;
	    whi_1 += whi_1d;
	  }
	  else break;
      }
      luv++;
    }
    luv += tcd.d_lw;
  }
}

void TextureCache::create_lighted_true_rgb (TCacheData& tcd, TCacheLightedTexture* tclt,
	csTextureManagerSoftware* txtmgr)
{
  //LightMap* lm = tcd.lm;
  int w = tcd.width;
  int h = tcd.height;
  int Imin_u = tcd.Imin_u;
  int Imin_v = tcd.Imin_v;

  unsigned char* mapR = tcd.mapR;
  unsigned char* mapG = tcd.mapG;
  unsigned char* mapB = tcd.mapB;

  unsigned char* otmap = tcd.tdata;
  int shf_w = tcd.shf_w;
  int and_w = tcd.and_w;
  int and_h = tcd.and_h;
  and_h <<= shf_w;

  unsigned char* tm, * tm2;
  int u, v, end_u, uu;

  unsigned char* light = NULL;

  int ov_idx;

  int whi_0, whi_1, whi_d, whi_0d, whi_1d;
  int whi;
  int red_00, gre_00, blu_00;
  int red_10, gre_10, blu_10;
  int red_01, gre_01, blu_01;
  int red_11, gre_11, blu_11;
  int red_0, red_1, red_d, red_0d, red_1d;
  int gre_0, gre_1, gre_d, gre_0d, gre_1d;
  int blu_0, blu_1, blu_d, blu_0d, blu_1d;
  int red, gre, blu;

  TextureTablesWhite8* w8 = txtmgr->lt_white8;
  RGB8map* wl = w8->white2_light;

  PalIdxLookup* lt_light, * pil;
  lt_light = txtmgr->lt_light;

  TextureTablesPalette* lt_pal = txtmgr->lt_pal;
  unsigned char* true_to_pal = lt_pal->true_to_pal;

  int lu, lv, luv, dv;
  luv = tcd.lv1 * tcd.lw + tcd.lu1;
  for (lv = tcd.lv1 ; lv < tcd.lv2 ; lv++)
  {
    for (lu = tcd.lu1 ; lu < tcd.lu2 ; lu++)
    {
      red_00 = mapR[luv];
      red_10 = mapR[luv+1];
      red_01 = mapR[luv+tcd.lw];
      red_11 = mapR[luv+tcd.lw+1];
      gre_00 = mapG[luv];
      gre_10 = mapG[luv+1];
      gre_01 = mapG[luv+tcd.lw];
      gre_11 = mapG[luv+tcd.lw+1];
      blu_00 = mapB[luv];
      blu_10 = mapB[luv+1];
      blu_01 = mapB[luv+tcd.lw];
      blu_11 = mapB[luv+tcd.lw+1];

      u = lu << tcd.mipmap_shift;
      v = lv << tcd.mipmap_shift;
      tm = &tclt->get_tmap8 ()[w*v+u];	//@@@

      if (blu_00 == gre_00 && blu_10 == gre_10 && blu_01 == gre_01 && blu_11 == gre_11 &&
          blu_00 == red_00 && blu_10 == red_10 && blu_01 == red_01 && blu_11 == red_11)
      {
        //*****
	// Pure white light.
        //*****
	whi_0 = gre_00 << 16; whi_0d = ((gre_01-gre_00)<<16) >> tcd.mipmap_shift;
	whi_1 = gre_10 << 16; whi_1d = ((gre_11-gre_10)<<16) >> tcd.mipmap_shift;

	for (dv = 0 ; dv < tcd.mipmap_size ; dv++, tm += w-tcd.mipmap_size)
	  if (v+dv < h)
	  {
	    ov_idx = ((v+dv+Imin_v)<<shf_w) & and_h;
	    whi = whi_0; whi_d = (whi_1-whi_0) >> tcd.mipmap_shift;

	    end_u = u+tcd.mipmap_size;
	    if (end_u > w) end_u = w;
	    end_u += Imin_u;
	    tm2 = tm + tcd.mipmap_size;
	    for (uu = u+Imin_u ; uu < end_u ; uu++)
	    {
	      light = wl[whi >> 16];
	      *tm++ = light[otmap[ov_idx + (uu & and_w)]];
	      whi += whi_d;
	    }
	    tm = tm2;

	    whi_0 += whi_0d;
	    whi_1 += whi_1d;
	  }
	  else break;

	luv++;
	continue;
      }

      //*****
      // Most general case: varying levels of red, green, and blue light.
      //*****

      red_0 = red_00 << 16; red_0d = ((red_01-red_00)<<16) >> tcd.mipmap_shift;
      red_1 = red_10 << 16; red_1d = ((red_11-red_10)<<16) >> tcd.mipmap_shift;
      gre_0 = gre_00 << 16; gre_0d = ((gre_01-gre_00)<<16) >> tcd.mipmap_shift;
      gre_1 = gre_10 << 16; gre_1d = ((gre_11-gre_10)<<16) >> tcd.mipmap_shift;
      blu_0 = blu_00 << 16; blu_0d = ((blu_01-blu_00)<<16) >> tcd.mipmap_shift;
      blu_1 = blu_10 << 16; blu_1d = ((blu_11-blu_10)<<16) >> tcd.mipmap_shift;

      for (dv = 0 ; dv < tcd.mipmap_size ; dv++, tm += w-tcd.mipmap_size)
	if (v+dv < h)
        {
	  ov_idx = ((v+dv+Imin_v)<<shf_w) & and_h;

	  red = red_0; red_d = (red_1-red_0) >> tcd.mipmap_shift;
	  gre = gre_0; gre_d = (gre_1-gre_0) >> tcd.mipmap_shift;
	  blu = blu_0; blu_d = (blu_1-blu_0) >> tcd.mipmap_shift;

	  end_u = u+tcd.mipmap_size;
	  if (end_u > w) end_u = w;
	  end_u += Imin_u;
	  tm2 = tm + tcd.mipmap_size;
	  for (uu = u+Imin_u ; uu < end_u ; uu++)
	  {
	    //*tm++ = i_tc->mix_lights(red, gre, blu, otmap[ov_idx + (uu & and_w)]);
	    pil = lt_light+otmap[ov_idx + (uu & and_w)];
	    *tm++ = true_to_pal[pil->red[red>>16] | pil->green[gre>>16] | pil->blue[blu>>16]];
	    red += red_d;
	    gre += gre_d;
	    blu += blu_d;
	  }
	  tm = tm2;

	  red_0 += red_0d;
	  red_1 += red_1d;
	  gre_0 += gre_0d;
	  gre_1 += gre_1d;
	  blu_0 += blu_0d;
	  blu_1 += blu_1d;
	}
	else break;

      luv++;
    }
    luv += tcd.d_lw;
  }
}

void TextureCache::create_lighted_true_rgb_priv (TCacheData& tcd, TCacheLightedTexture* tclt,
	csTextureManagerSoftware* txtmgr)
{
  //LightMap* lm = tcd.lm;
  int w = tcd.width;
  int h = tcd.height;
  int Imin_u = tcd.Imin_u;
  int Imin_v = tcd.Imin_v;

  unsigned char* mapR = tcd.mapR;
  unsigned char* mapG = tcd.mapG;
  unsigned char* mapB = tcd.mapB;

  unsigned char* otmap = tcd.tdata;
  int shf_w = tcd.shf_w;
  int and_w = tcd.and_w;
  int and_h = tcd.and_h;
  and_h <<= shf_w;

  unsigned char* tm, * tm2;
  int u, v, end_u, uu;

  int ov_idx;

  int red_00, gre_00, blu_00;
  int red_10, gre_10, blu_10;
  int red_01, gre_01, blu_01;
  int red_11, gre_11, blu_11;
  int red_0, red_1, red_d, red_0d, red_1d;
  int gre_0, gre_1, gre_d, gre_0d, gre_1d;
  int blu_0, blu_1, blu_d, blu_0d, blu_1d;
  int red, gre, blu;

  unsigned char* rgb_values = tcd.txt_mm->get_colormap_private ();
  unsigned char* rgb;

  TextureTablesPalette* lt_pal = txtmgr->lt_pal;
  unsigned char* palt = lt_pal->true_to_pal;

  PalIdxLookup* lt_light = txtmgr->lt_light;

  int lu, lv, luv, dv;
  luv = tcd.lv1 * tcd.lw + tcd.lu1;
  for (lv = tcd.lv1 ; lv < tcd.lv2 ; lv++)
  {
    for (lu = tcd.lu1 ; lu < tcd.lu2 ; lu++)
    {
      red_00 = mapR[luv];
      red_10 = mapR[luv+1];
      red_01 = mapR[luv+tcd.lw];
      red_11 = mapR[luv+tcd.lw+1];
      gre_00 = mapG[luv];
      gre_10 = mapG[luv+1];
      gre_01 = mapG[luv+tcd.lw];
      gre_11 = mapG[luv+tcd.lw+1];
      blu_00 = mapB[luv];
      blu_10 = mapB[luv+1];
      blu_01 = mapB[luv+tcd.lw];
      blu_11 = mapB[luv+tcd.lw+1];

      u = lu << tcd.mipmap_shift;
      v = lv << tcd.mipmap_shift;
      tm = &tclt->get_tmap8 ()[w*v+u];

      //*****
      // Most general case: varying levels of red, green, and blue light.
      //*****

      red_0 = red_00 << 16; red_0d = ((red_01-red_00)<<16) >> tcd.mipmap_shift;
      red_1 = red_10 << 16; red_1d = ((red_11-red_10)<<16) >> tcd.mipmap_shift;
      gre_0 = gre_00 << 16; gre_0d = ((gre_01-gre_00)<<16) >> tcd.mipmap_shift;
      gre_1 = gre_10 << 16; gre_1d = ((gre_11-gre_10)<<16) >> tcd.mipmap_shift;
      blu_0 = blu_00 << 16; blu_0d = ((blu_01-blu_00)<<16) >> tcd.mipmap_shift;
      blu_1 = blu_10 << 16; blu_1d = ((blu_11-blu_10)<<16) >> tcd.mipmap_shift;

      for (dv = 0 ; dv < tcd.mipmap_size ; dv++, tm += w-tcd.mipmap_size)
	if (v+dv < h)
        {
	  ov_idx = ((v+dv+Imin_v)<<shf_w) & and_h;

	  red = red_0; red_d = (red_1-red_0) >> tcd.mipmap_shift;
	  gre = gre_0; gre_d = (gre_1-gre_0) >> tcd.mipmap_shift;
	  blu = blu_0; blu_d = (blu_1-blu_0) >> tcd.mipmap_shift;

	  end_u = u+tcd.mipmap_size;
	  if (end_u > w) end_u = w;
	  end_u += Imin_u;
	  tm2 = tm + tcd.mipmap_size;
	  for (uu = u+Imin_u ; uu < end_u ; uu++)
	  {
	    rgb = rgb_values + ((otmap[ov_idx + (uu & and_w)]) << 2);
	    //*tm++ = palt[i_tc->add_light_red_private (*rgb, red) |
	    //		i_tc->add_light_green_private (*(rgb+1), gre) |
	    //		i_tc->add_light_blue_private (*(rgb+2), blu)];
	    *tm++ = palt[lt_light[*rgb].red[red>>16] |
			 lt_light[*(rgb+1)].green[gre>>16] |
			 lt_light[*(rgb+2)].blue[blu>>16]];
	    red += red_d;
	    gre += gre_d;
	    blu += blu_d;
	  }
	  tm = tm2;

	  red_0 += red_0d;
	  red_1 += red_1d;
	  gre_0 += gre_0d;
	  gre_1 += gre_1d;
	  blu_0 += blu_0d;
	  blu_1 += blu_1d;
	}
	else break;

      luv++;
    }
    luv += tcd.d_lw;
  }
}

void TextureCache::show_lightmap_grid (TCacheData& tcd, TCacheLightedTexture* tclt, csTextureManagerSoftware* txtmgr)
{
  //LightMap* lm = tcd.lm;
  int w = tcd.width;

  unsigned char* mapR = tcd.mapR;
  unsigned char* mapG = tcd.mapG;
  unsigned char* mapB = tcd.mapB;

  unsigned char* tm;
  int u, v;

  int lu, lv, luv;
  luv = tcd.lv1 * tcd.lw + tcd.lu1;
  for (lv = tcd.lv1 ; lv < tcd.lv2 ; lv++)
  {
    for (lu = tcd.lu1 ; lu < tcd.lu2 ; lu++)
    {
      u = lu << tcd.mipmap_shift;
      v = lv << tcd.mipmap_shift;
      tm = &tclt->get_tmap8 ()[w*v+u];	//@@@
      //@@@ This is NOT optimal but luckily it is a debug function only.
      *tm = txtmgr->find_rgb (mapR[luv], mapG[luv], mapB[luv]);
      luv++;
    }
    luv += tcd.d_lw;
  }
}

void TextureCache::create_lighted_texture_lightmaps (TCacheData& /*tcd*/, TCacheLightedTexture* /*tclt*/,
  csTextureManagerSoftware* /*txtmgr*/)
{
#if 0
  int w = tcd.width;
  int h = tcd.height;
  int Imin_u = tcd.Imin_u;

  unsigned char* mapR = tcd.mapR;
  unsigned char* mapG = tcd.mapG;
  unsigned char* mapB = tcd.mapB;

  unsigned char* light1 = NULL, * light2 = NULL, * light3 = NULL;
  TextureTablesFastXxx* lt_fastxxx = txtmgr->lt_fastxxx;
  RGB8map* lt1 = lt_fastxxx->light1;
  RGB8map* lt2 = lt_fastxxx->light2;
  RGB8map* lt3 = lt_fastxxx->light3;

  int white_color = txtmgr->find_rgb (255, 255, 255);

  unsigned char* tm, * tm2;
  int u, v, end_u, uu;

  int red_00, gre_00, blu_00;
  int red_10, gre_10, blu_10;
  int red_01, gre_01, blu_01;
  int red_11, gre_11, blu_11;
  int red_0, red_1, red_d, red_0d, red_1d;
  int gre_0, gre_1, gre_d, gre_0d, gre_1d;
  int blu_0, blu_1, blu_d, blu_0d, blu_1d;
  int red, gre, blu;

  int lu, lv, luv, dv;
  luv = tcd.lv1 * tcd.lw + tcd.lu1;
  for (lv = tcd.lv1 ; lv < tcd.lv2 ; lv++)
  {
    for (lu = tcd.lu1 ; lu < tcd.lu2 ; lu++)
    {
      red_00 = mapR[luv];
      red_10 = mapR[luv+1];
      red_01 = mapR[luv+tcd.lw];
      red_11 = mapR[luv+tcd.lw+1];
      gre_00 = mapG[luv];
      gre_10 = mapG[luv+1];
      gre_01 = mapG[luv+tcd.lw];
      gre_11 = mapG[luv+tcd.lw+1];
      blu_00 = mapB[luv];
      blu_10 = mapB[luv+1];
      blu_01 = mapB[luv+tcd.lw];
      blu_11 = mapB[luv+tcd.lw+1];

      u = lu << tcd.mipmap_shift;
      v = lv << tcd.mipmap_shift;
      tm = &tclt->get_tmap8 ()[w*v+u];	//@@@

      red_0 = red_00 << 16; red_0d = ((red_01-red_00)<<16) >> tcd.mipmap_shift;
      red_1 = red_10 << 16; red_1d = ((red_11-red_10)<<16) >> tcd.mipmap_shift;
      gre_0 = gre_00 << 16; gre_0d = ((gre_01-gre_00)<<16) >> tcd.mipmap_shift;
      gre_1 = gre_10 << 16; gre_1d = ((gre_11-gre_10)<<16) >> tcd.mipmap_shift;
      blu_0 = blu_00 << 16; blu_0d = ((blu_01-blu_00)<<16) >> tcd.mipmap_shift;
      blu_1 = blu_10 << 16; blu_1d = ((blu_11-blu_10)<<16) >> tcd.mipmap_shift;

      for (dv = 0 ; dv < tcd.mipmap_size ; dv++, tm += w-tcd.mipmap_size)
	if (v+dv < h)
        {
	  red = red_0; red_d = (red_1-red_0) >> tcd.mipmap_shift;
	  gre = gre_0; gre_d = (gre_1-gre_0) >> tcd.mipmap_shift;
	  blu = blu_0; blu_d = (blu_1-blu_0) >> tcd.mipmap_shift;

	  end_u = u+tcd.mipmap_size;
	  if (end_u > w) end_u = w;
	  end_u += Imin_u;
	  tm2 = tm + tcd.mipmap_size;
	  for (uu = u+Imin_u ; uu < end_u ; uu++)
	  {
	    if (tcd.mixing == MIX_TRUE_RGB)
	    {
	      //@@@Not optimal but this is a debug function only.
	      *tm++ = txtmgr->mix_lights (red, gre, blu, white_color);
	    }
	    else
	    {
	      light1 = lt1[red >> 16];
	      light2 = lt2[gre >> 16];
	      light3 = lt3[blu >> 16];
	      *tm++ = light3[light2[light1[white_color]]];
	    }
	    red += red_d;
	    gre += gre_d;
	    blu += blu_d;
	  }
	  tm = tm2;

	  red_0 += red_0d;
	  red_1 += red_1d;
	  gre_0 += gre_0d;
	  gre_1 += gre_1d;
	  blu_0 += blu_0d;
	  blu_1 += blu_1d;
	}
	else break;

      luv++;
    }
    luv += tcd.d_lw;
  }
#endif
}

//---------------------------------------------------------------------------
