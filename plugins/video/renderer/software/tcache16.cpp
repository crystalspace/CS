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
#include "cs3d/software/tcache16.h"
#include "cs3d/software/soft_txt.h"
#include "igraph2d.h"
#include "igraph3d.h"
#include "ipolygon.h"

//---------------------------------------------------------------------------

TextureCache16::TextureCache16 (csPixelFormat* pfmt) : TextureCache (pfmt)
{
  gi_pixelbytes = 2;
}

TextureCache16::~TextureCache16 ()
{
}

void TextureCache16::create_lighted_texture (TCacheData& tcd,
  TCacheLightedTexture* tclt, csTextureManagerSoftware* txtmgr)
{
  if (tcd.txtmode == TXT_PRIVATE)
    create_lighted_true_rgb_priv (tcd, tclt, txtmgr);
  else if (tcd.txtmode == TXT_24BIT)
    create_lighted_24bit (tcd, tclt, txtmgr);
  else
    create_lighted_true_rgb (tcd, tclt, txtmgr);

  if (tcd.lm_grid)
    show_lightmap_grid (tcd, tclt, txtmgr);
}

void TextureCache16::create_lighted_true_rgb (TCacheData& tcd,
  TCacheLightedTexture* tclt, csTextureManagerSoftware* txtmgr)
{
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

  UShort* tm, * tm2;
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

  TextureTablesTrueRgb* lt_truergb = txtmgr->lt_truergb;
  PalIdxLookup* lut = lt_truergb->lut;
  PalIdxLookup* pil;

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
	    unsigned char* ot = otmap+ov_idx;
	    for (uu = u+Imin_u ; uu < end_u ; uu++)
	    {
	      //light = wt[whi >> 16];
	      //*tm++ = light[ot[uu & and_w]];
	      pil = lut+ot[uu & and_w];
	      *tm++ = pil->red[whi>>16] | pil->green[whi>>16] | pil->blue[whi>>16];

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
	  unsigned char* ot = otmap+ov_idx;
	  for (uu = u+Imin_u ; uu < end_u ; uu++)
	  {
	    //*tm++ = i_tc->mix_lights_16 (red, gre, blu, ot[uu & and_w]);
	    pil = lut+ot[uu & and_w];
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
}

void TextureCache16::create_lighted_24bit (TCacheData& tcd, TCacheLightedTexture* tclt,
  csTextureManagerSoftware* txtmgr)
{
  (void)txtmgr;
  int w = tcd.width;
  int h = tcd.height;
  int Imin_u = tcd.Imin_u;
  int Imin_v = tcd.Imin_v;

  int rs = txtmgr->pixel_format ().RedShift;

  unsigned char* mapR = tcd.mapR;
  unsigned char* mapG = tcd.mapG;
  unsigned char* mapB = tcd.mapB;

  ULong* otmap = (ULong*)tcd.tdata;
  int shf_w = tcd.shf_w;
  int and_w = tcd.and_w;
  int and_h = tcd.and_h;
  and_h <<= shf_w;

  UShort* tm, * tm2;
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
      // NOTE: level 127 means "fully lit", levels above 127 should be
      // clipped to 127. We do clipping *inside* lighting loop, this is
      // not as costly to do it here (and introduce yet another inaccuracy)
      // If we miss levels above 127, we'll never get the white color, just
      // the original color.

      red_00 = mapR[luv];        red_10 = mapR[luv+1];
      red_01 = mapR[luv+tcd.lw]; red_11 = mapR[luv+tcd.lw+1];
      gre_00 = mapG[luv];        gre_10 = mapG[luv+1];
      gre_01 = mapG[luv+tcd.lw]; gre_11 = mapG[luv+tcd.lw+1];
      blu_00 = mapB[luv];        blu_10 = mapB[luv+1];
      blu_01 = mapB[luv+tcd.lw]; blu_11 = mapB[luv+tcd.lw+1];

      u = lu << tcd.mipmap_shift;
      v = lv << tcd.mipmap_shift;
      tm = &tclt->get_tmap16 ()[w*v+u];

      if (blu_00 == gre_00 && blu_10 == gre_10 && blu_01 == gre_01 && blu_11 == gre_11 &&
          blu_00 == red_00 && blu_10 == red_10 && blu_01 == red_01 && blu_11 == red_11)
      {
        //*****
	// Pure white light.
        //*****
	whi_0 = gre_00 << 16; whi_0d = ((gre_01 - gre_00) << 16) >> tcd.mipmap_shift;
	whi_1 = gre_10 << 16; whi_1d = ((gre_11 - gre_10) << 16) >> tcd.mipmap_shift;

        if (rs == 11)
	  #define PI_R5G6B5
	  #define TL_WHITE
	  #include "texl24.inc"
	else
	  #define PI_R5G5B5
	  #define TL_WHITE
	  #include "texl24.inc"

	luv++;
      }
      else
      {
        //*****
        // Most general case: varying levels of red, green, and blue light.
        //*****

        red_0 = red_00 << 16; red_0d = ((red_01-red_00)<<16) >> tcd.mipmap_shift;
        red_1 = red_10 << 16; red_1d = ((red_11-red_10)<<16) >> tcd.mipmap_shift;
        gre_0 = gre_00 << 16; gre_0d = ((gre_01-gre_00)<<16) >> tcd.mipmap_shift;
        gre_1 = gre_10 << 16; gre_1d = ((gre_11-gre_10)<<16) >> tcd.mipmap_shift;
        blu_0 = blu_00 << 16; blu_0d = ((blu_01-blu_00)<<16) >> tcd.mipmap_shift;
        blu_1 = blu_10 << 16; blu_1d = ((blu_11-blu_10)<<16) >> tcd.mipmap_shift;

        if (rs == 11)
	  #define PI_R5G6B5
	  #define TL_RGB
	  #include "texl24.inc"
        else
	  #define PI_R5G5B5
	  #define TL_RGB
	  #include "texl24.inc"

        luv++;
      }
    }
    luv += tcd.d_lw;
  }
}

void TextureCache16::create_lighted_true_rgb_priv (TCacheData& tcd, TCacheLightedTexture* tclt,
	csTextureManagerSoftware* txtmgr)
{
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

  UShort* tm, * tm2;
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

  unsigned char* rgb_values = tcd.txt_mm->GetPrivateColorMap ();
  unsigned char* rgb;

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
      tm = &tclt->get_tmap16 ()[w*v+u];

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
	    //*tm++ = i_tc->add_light_red_private (*rgb, red) |
	    //	    i_tc->add_light_green_private (*(rgb+1), gre) |
	    //	    i_tc->add_light_blue_private (*(rgb+2), blu);
	    *tm++ = lt_light [*rgb].red [red>>16] |
	    	    lt_light [*(rgb+1)].green [gre>>16] |
	    	    lt_light [*(rgb+2)].blue [blu>>16];
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

void TextureCache16::show_lightmap_grid (TCacheData& tcd, TCacheLightedTexture* tclt, csTextureManagerSoftware* txtmgr)
{
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
}
