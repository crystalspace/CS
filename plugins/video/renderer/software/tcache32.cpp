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
  RedByte = pfmt->RedShift >> 3;
  GreenByte = pfmt->GreenShift >> 3;
  BlueByte = pfmt->BlueShift >> 3;
  if (RedByte && GreenByte && BlueByte)
    PostShift = 8, RedByte--, GreenByte--, BlueByte--;
  else
    PostShift = 0;
}

TextureCache32::~TextureCache32 ()
{
}

void TextureCache32::create_lighted_texture (TCacheData& tcd, TCacheLightedTexture* tclt,
	csTextureManagerSoftware* txtmgr)
{
  if (tcd.lm_only) create_lighted_texture_lightmaps (tcd, tclt, txtmgr);
  else create_lighted_24bit (tcd, tclt, txtmgr);
  if (tcd.lm_grid) show_lightmap_grid (tcd, tclt, txtmgr);
}

/*
    Implementation note. We don't pay attention to whenever we have RGB or BGR
    or whatever pixel format except for the moment when we get the light map
    for R, G and B. Because texture is kept in "native" form, bits 16-23 of
    texture will end in bits 16-23 of lighted texture, bits 8-15 of source
    texture will influence bits 8-15 of the result and so on, no matter if
    it is R, G or B component. The only thing that really matters is where
    the lightmap pointer (mapR, mapG or mapB) points. We can decide this
    once at the beginning of the function.
*/
void TextureCache32::create_lighted_24bit (TCacheData& tcd, TCacheLightedTexture* tclt,
  csTextureManagerSoftware* txtmgr)
{
  (void)txtmgr;
  int w = tcd.width;
  int h = tcd.height;
  int Imin_u = tcd.Imin_u;
  int Imin_v = tcd.Imin_v;

  unsigned char *LightMap [3];
  LightMap [RedByte] = tcd.mapR;
  LightMap [GreenByte] = tcd.mapG;
  LightMap [BlueByte] = tcd.mapB;

  // The line below should be optimized well by most decent compilers
  unsigned char *mapR = LightMap [2], *mapG = LightMap [1], *mapB = LightMap [0];

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
      tm = &tclt->get_tmap32 ()[w*v+u];

      if (blu_00 == gre_00 && blu_10 == gre_10 && blu_01 == gre_01 && blu_11 == gre_11 &&
          blu_00 == red_00 && blu_10 == red_10 && blu_01 == red_01 && blu_11 == red_11)
      {
        //*****
	// Pure white light.
        //*****
	whi_0 = gre_00 << 16; whi_0d = ((gre_01-gre_00)<<16) >> tcd.mipmap_shift;
	whi_1 = gre_10 << 16; whi_1d = ((gre_11-gre_10)<<16) >> tcd.mipmap_shift;

        #define PI_R8G8B8
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

	#define PI_R8G8B8
	#define TL_RGB
	#include "texl24.inc"

        luv++;
      }
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
