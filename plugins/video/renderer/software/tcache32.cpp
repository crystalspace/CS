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
#include "cs3d/software/tcache32.h"
#include "cs3d/software/soft_txt.h"
#include "igraph2d.h"
#include "igraph3d.h"
#include "ipolygon.h"

//---------------------------------------------------------------------------

TextureCache32::TextureCache32 (csPixelFormat* pfmt) : TextureCache (pfmt)
{
  gi_pixelbytes = 4;
  red_shift = pfmt->RedShift;
  green_shift = pfmt->GreenShift;
  blue_shift = pfmt->BlueShift;
}

TextureCache32::~TextureCache32 ()
{
}

void TextureCache32::create_lighted_texture (TCacheData& tcd, TCacheLightedTexture* tclt,
	csTextureManagerSoftware* txtmgr)
{
  if (tcd.lm_only) create_lighted_texture_lightmaps (tcd, tclt, txtmgr);
  else create_lighted_texture_24bit (tcd, tclt, txtmgr);
  if (tcd.lm_grid) show_lightmap_grid (tcd, tclt, txtmgr);
}

void TextureCache32::create_lighted_texture_24bit (TCacheData& tcd, TCacheLightedTexture* tclt,
  csTextureManagerSoftware* txtmgr)
{
  (void)txtmgr;
  int w = tcd.width;
  int h = tcd.height;
  int Imin_u = tcd.Imin_u;
  int Imin_v = tcd.Imin_v;

  unsigned char* mapR = tcd.mapR;
  unsigned char* mapG = tcd.mapG;
  unsigned char* mapB = tcd.mapB;

  ULong* otmap = (ULong*)tcd.tdata;
  int shf_w = tcd.shf_w;
  int and_w = tcd.and_w;
  int and_h = tcd.and_h;
  and_h <<= shf_w;

  ULong* tm, * tm2;
  int u, v, end_u, uu;

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

  int lu, lv, luv, dv;
  luv = tcd.lv1 * tcd.lw + tcd.lu1;
  for (lv = tcd.lv1 ; lv < tcd.lv2 ; lv++)
  {
    for (lu = tcd.lu1 ; lu < tcd.lu2 ; lu++)
    {
      // Note. Here we correct for the fact that level 128 in the lightmaps
      // means fully lit. So we multiply the lightmap light level to have 256
      // as the fully lit level. However we need to cap the final value of the
      // texture to 255. To be fully correct we should do this in this inner loop
      // but we already cap here. This means that in some cases we will actually
      // interpolate wrong. Imagine interpolating from 300 to 200 in 3 steps.
      // If we cap where we should cap then we actually get the values: 255 255 200.
      // But we cap before interpolation so we get: 255 225 200 instead.
      // I don't think this matters much however. The interpolation is wrong in
      // anycase. This is just another kind of wrongness.
      red_00 = mapR[luv] << 1; if (red_00 > 255) red_00 = 255;
      red_10 = mapR[luv+1] << 1; if (red_10 > 255) red_10 = 255;
      red_01 = mapR[luv+tcd.lw] << 1; if (red_01 > 255) red_01 = 255;
      red_11 = mapR[luv+tcd.lw+1] << 1; if (red_11 > 255) red_11 = 255;
      gre_00 = mapG[luv] << 1; if (gre_00 > 255) gre_00 = 255;
      gre_10 = mapG[luv+1] << 1; if (gre_10 > 255) gre_10 = 255;
      gre_01 = mapG[luv+tcd.lw] << 1; if (gre_01 > 255) gre_01 = 255;
      gre_11 = mapG[luv+tcd.lw+1] << 1; if (gre_11 > 255) gre_11 = 255;
      blu_00 = mapB[luv] << 1; if (blu_00 > 255) blu_00 = 255;
      blu_10 = mapB[luv+1] << 1; if (blu_10 > 255) blu_10 = 255;
      blu_01 = mapB[luv+tcd.lw] << 1; if (blu_01 > 255) blu_01 = 255;
      blu_11 = mapB[luv+tcd.lw+1] << 1; if (blu_11 > 255) blu_11 = 255;

      u = lu << tcd.mipmap_shift;
      v = lv << tcd.mipmap_shift;
      tm = &tclt->get_tmap32 ()[w*v+u];

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
	    ULong* ot = otmap+ov_idx;
	    ULong RGB;
	    for (uu = u+Imin_u ; uu < end_u ; uu++)
	    {
	      RGB = ot[uu & and_w];
	      *tm++ = ( (RGB >> 16) * (whi>>16) >> 8) << red_shift |
	               ((RGB >> 8 & 0xff) * (whi>>16) >> 8) << green_shift |
		       ((RGB & 0xff) * (whi>>16) >> 8) << blue_shift;
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
	  ULong* ot = otmap+ov_idx;
	  ULong RGB;
	  for (uu = u+Imin_u ; uu < end_u ; uu++)
	  {
	    RGB = ot[uu & and_w];
	    *tm++ = ((RGB >> 16) * (red>>16) >> 8) << red_shift |
	            ((RGB >> 8 & 0xff) * (gre>>16) >> 8) << green_shift |
		    ((RGB & 0xff) * (blu>>16) >> 8) << blue_shift;

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

void TextureCache32::show_lightmap_grid (TCacheData& /*tcd*/, 
                                         TCacheLightedTexture* /*tclt*/, 
                                         csTextureManagerSoftware* /*txtmgr*/)
{
#if 0
  int w = tcd.width;

  unsigned char* mapR = tcd.mapR;
  unsigned char* mapG = tcd.mapG;
  unsigned char* mapB = tcd.mapB;
  TextureTablesTrueRgb* lt_truergb = txtmgr->lt_truergb;
  PalIdxLookup* lut = lt_truergb->lut;
  PalIdxLookup* pil;

  int white_color = txtmgr->find_rgb (255, 255, 255);

  UShort* tm;
  int u, v;

  int lu, lv, luv;
  luv = tcd.lv1 * tcd.lw + tcd.lu1;
  for (lv = tcd.lv1 ; lv < tcd.lv2 ; lv++)
  {
    for (lu = tcd.lu1 ; lu < tcd.lu2 ; lu++)
    {
      u = lu << tcd.mipmap_shift;
      v = lv << tcd.mipmap_shift;
      tm = &tclt->get_tmap16 ()[w*v+u];	//@@@
      pil = lut+white_color;
      *tm = pil->red[mapR[luv]] | pil->green[mapG[luv]] | pil->blue[mapB[luv]];
      luv++;
    }
    luv += tcd.d_lw;
  }
#endif
}

void TextureCache32::create_lighted_texture_lightmaps (TCacheData& /*tcd*/, TCacheLightedTexture* /*tclt*/,
  csTextureManagerSoftware* /*txtmgr*/)
{
#if 0
  int w = tcd.width;
  int h = tcd.height;
  int Imin_u = tcd.Imin_u;

  unsigned char* mapR = tcd.mapR;
  unsigned char* mapG = tcd.mapG;
  unsigned char* mapB = tcd.mapB;

  UShort* tm, * tm2;
  int u, v, end_u, uu;

  int red_00, gre_00, blu_00;
  int red_10, gre_10, blu_10;
  int red_01, gre_01, blu_01;
  int red_11, gre_11, blu_11;
  int red_0, red_1, red_d, red_0d, red_1d;
  int gre_0, gre_1, gre_d, gre_0d, gre_1d;
  int blu_0, blu_1, blu_d, blu_0d, blu_1d;
  int red, gre, blu;

  TextureTablesTrueRgb* lt_truergb = txtmgr->lt_truergb;
  PalIdxLookup* lut = lt_truergb->lut;
  PalIdxLookup* pil;

  int white_color = txtmgr->find_rgb (255, 255, 255);

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
      tm = &tclt->get_tmap16 ()[w*v+u];

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
	    //*tm++ = i_tc->mix_lights_16 (red, gre, blu, ot[uu & and_w]);
	    pil = lut+white_color;
	    *tm++ = pil->red[red>>16] | pil->green[gre>>16] | pil->blue[blu>>16];

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
