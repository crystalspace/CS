/*  -*- Mode: C++; -*- */
/*
    Crystal Space 3D engine
    Copyright (C) 2000 by Jorrit Tyberghein

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

/*
    This file contains the routine that creates lightmapped textures.
    Basically it takes a texture, a lightmap and combines them together.
    This routine is used for all three bit depths - 8, 16 and 32bpp.
    The following macros can be defined in order to control the way
    output lighted texture is created:

	LM_NAME		The name of the routine.
	PI_R5G5B5,
	PI_R5G6B5,
	PI_R8G8B8	The pixel format.
*/

/*
    Lots of actors comes into play inside the following routine. All those
    polygon textures, lightmaps, textureMM, bitmaps etc causes big troubles
    if you're trying to understand how the algorithm works. Let's try
    to classify them and introduce a notation for intermediate variable
    that will (I hope) ease the understanding:

	- csPolyTextureMapping is the object that is in charge for containing
	  all parameters related to a single mipmap level on a Polygon3D. The
	  variables that contain values related to this structure will have
	  the "pt" prefix.
	- The lightmap is the object that is in charge for containing all
	  lighting information for the polygon. Basically it contains
	  three lightmaps: red, green an blue, each cell of the lightmap
	  covers a square of texels. The square size can be queried from
	  polygontexture object using GetMipMapSize () method. For example,
	  if it returns 16, this means that every cell of the lightmap
	  covers a 16x16 square of texels on the texture. The prefix for
	  intermediate variables related to the lightmap is "lm".
	- csTexture is the actual unlighted (original) texture in
          driver-dependent format. The csSoftwareTextureHandle object
	  contains four of these objects - one for each mipmap level.
	  The intermediate variables related to csTexture are prefixed
	  with "tx".
*/

#include "pixtype.inc"

//#define DEBUG_LIMITS

void LM_NAME (csPolyTextureMapping* tmapping,
	      csSoftRendererLightmap* rlm, 
	      SoftwareCachedTexture *ct,
	      csSoftwareTextureHandle *texmm, csSoftwareTextureManager *texman,
	      float u_min, float v_min, float u_max, float v_max)
{
  int mipmap = ct->mipmap;
  int mipmap_delta = (1 << mipmap);

  csSoftwareTexture *tx = (csSoftwareTexture *)texmm->get_texture (mipmap);
  csRGBpixel *tx_palette = texmm->GetColorMap ();
  uint8 *tx_bitmap = tx->get_bitmap ();

  //int pt_cell_size = pt->GetLightCellSize () >> mipmap; // from iRendererLightMap
  int pt_cell_size = rlm->lightCellSize >> mipmap; 
  
  if (!pt_cell_size)
  {
  // this happens when the lightcell size is <8
  // so a texel on the current mipmap spans multiple
  // lightmap texels
  // so we need a different algorithm:
  // multiple lightmap texels are averaged
  
	int pt_min_u = tmapping->GetIMinU () >> mipmap;
	int pt_min_v = tmapping->GetIMinV () >> mipmap;
	int pt_cell_shift = -rlm->lightCellShift /*pt->GetLightCellShift ()*/ + mipmap;
	pt_cell_size = 1 << pt_cell_shift;
	int pt_shfw = tmapping->GetShiftU () - mipmap;
	int pt_w = 1 << pt_shfw;
	
	int lm_color_shift = 6 - pt_cell_shift*2;

	int lm_w = rlm->rect.Width (); //lm->GetWidth ();
	csRGBpixel *lm_data = rlm->data; //lm->GetMapData ();
	int lm_size = rlm->rect.Width () * rlm->rect.Height (); //lm->GetSize ();
    
	void *dst = ct->get_bitmap ();
	uint32 *old_lm = (uint32 *)ct->get_lightmap ();

	int tx_shfw = tx->get_w_shift ();
	int tx_andw = tx->get_w_mask ();
	int tx_andh = tx->get_h_mask ();

	// Get the multiplication tables for red, green and blue
	uint8 *lt_R = texman->lightmap_tables [0];
	uint8 *lt_G = texman->lightmap_tables [1];
	uint8 *lt_B = texman->lightmap_tables [2];

	int min_lu = csQround (floor (u_min));
	int max_lu = csQround (ceil (u_max));
	int min_lv = csQround (floor (v_min));
	int max_lv = csQround (ceil (v_max));

	if (min_lu < 0) min_lu = 0;
	if (min_lv < 0) min_lv = 0;

	if (min_lu >= max_lu) return;
	if (min_lv >= max_lv) return;

	int max_lu2 = ((tmapping->GetLitOriginalWidth () + mipmap_delta) >> mipmap);
	if (max_lu > max_lu2) max_lu = max_lu2;

	int max_lv2 = (rlm->rect.Height ()) >> pt_cell_shift;
	if (max_lv > max_lv2) max_lv = max_lv2;

	int src = (min_lu << pt_cell_shift) + (min_lv << pt_cell_shift) * lm_w;
	CS_ASSERT (src >= 0);
	int src_dlu = (lm_w - (max_lu - min_lu)) << pt_cell_shift;

	int src_end = lm_size - lm_w - 1;
	
	PI_PIXTYPE *out = (PI_PIXTYPE *)dst + (min_lv << pt_shfw) + min_lu;
	
	for (int vv = min_lv; vv < max_lv; vv++)
	{
		int tx_idx = ((pt_min_v + vv) & tx_andh) << tx_shfw;
		PI_PIXTYPE *__out = out;

		if (src >= src_end) return;
		
		for (int uu = min_lu; uu < max_lu; uu++)
		{
			int r = 0;
			int g = 0;
			int b = 0;
		
			uint32 cellid = ((uint32*) lm_data) [src];

			// If this lightmap cell did not change, do not recalculate
			if (old_lm [src] == cellid)
			{ src += pt_cell_size; out++; continue; }

			old_lm [src] = cellid;
			
			for (int lv = 0; lv < pt_cell_size; lv++)
			{
				for (int lu = 0; lu < pt_cell_size; lu++)
				{
					r += lm_data [src + lu + lv * lm_w].red;
					g += lm_data [src + lu + lv * lm_w].green;
					b += lm_data [src + lu + lv * lm_w].blue;
				}
			}
			src += pt_cell_size;
			
			r <<= lm_color_shift;
			g <<= lm_color_shift;
			b <<= lm_color_shift;
			
			csRGBpixel pix = tx_palette [tx_bitmap [tx_idx + ((pt_min_u + uu) & tx_andw)]];
		      #ifdef DEBUG_LIMITS
			CS_ASSERT ((uint8*)out < (uint8*)dst + ct->size);
		      #endif
			*out++ =
				(lt_R [pix.red   | (r & 0x3f00)] << PI_RS) |
				(lt_G [pix.green | (g & 0x3f00)] << PI_GS) |
				(lt_B [pix.blue  | (b & 0x3f00)] << PI_BS)
			;
		}

		src += src_dlu;
				
		out = __out + pt_w;
	}
  }
  else
  {
	int pt_min_u = tmapping->GetIMinU () >> mipmap;
	int pt_min_v = tmapping->GetIMinV () >> mipmap;
	int pt_cell_shift = rlm->lightCellShift - mipmap;
	int pt_shfw = tmapping->GetShiftU () - mipmap;
	int pt_w = 1 << pt_shfw;
	int pt_h = (tmapping->GetLitHeight () + mipmap_delta) >> mipmap;

	int lm_w = rlm->rect.Width (); //lm->GetWidth ();
	csRGBpixel *lm_data = rlm->data; //lm->GetMapData ();
	int lm_size = rlm->rect.Width () * rlm->rect.Height (); //lm->GetSize ();

	void *dst = ct->get_bitmap ();
	uint32 *old_lm = (uint32 *)ct->get_lightmap ();

	int tx_shfw = tx->get_w_shift ();
	int tx_andw = tx->get_w_mask ();
	int tx_andh = tx->get_h_mask ();

	/*
		The textures are lighted in squares. A single lightmap value corresponds
		to all pixels in a square on the texture. To get more pleasant results,
		we interpolate the lightmap along square edges. The square edges are
		labeled 00, 10, 01 and 11 as shown below.
					We hold the R/G/B values for all four corners of the square
		00      10  in variables called r00, g00, b00, r10, g10 and so on.
		*--------*  The square is scanned from top to bottom; during this scan
		|        |  the corresponding R/G/B values are kept in r0, g0, b0 and
	       0*--------*1 r1, g1, b1 and interpolated while moving from top to bottom
		|        |  using r0d/g0d/b0d and r1d/g1d/b1d deltas. When scanning
		|        |  from left to right the deltas are kept in rd, gd and bd.
		*--------*
		01      11
	*/

	// Get the multiplication tables for red, green and blue
	uint8 *lt_R = texman->lightmap_tables [0];
	uint8 *lt_G = texman->lightmap_tables [1];
	uint8 *lt_B = texman->lightmap_tables [2];

	int min_lu = csQround (floor (u_min)) >> pt_cell_shift;
	int max_lu = (csQround (ceil (u_max)) + pt_cell_size - 1) >> pt_cell_shift;
	int min_lv = csQround (floor (v_min)) >> pt_cell_shift;
	int max_lv = (csQround (ceil (v_max)) + pt_cell_size - 1) >> pt_cell_shift;

	if (min_lu < 0) min_lu = 0;
	if (min_lv < 0) min_lv = 0;

	if (min_lu >= max_lu) return;
	if (min_lv >= max_lv) return;

	int max_lu2 = ((((tmapping->GetLitOriginalWidth () + mipmap_delta)
		>> mipmap) + pt_cell_size - 1) >> pt_cell_shift);
	if (max_lu > max_lu2) max_lu = max_lu2;

	int max_lv2 = rlm->rect.Height ();
	if (max_lv > max_lv2) max_lv = max_lv2;

	int src = min_lu + min_lv * lm_w;
	CS_ASSERT (src >= 0);
	int src_dlu = lm_w - (max_lu - min_lu);

	// @@@ Jorrit: the code below is a bit clumsy. For some reason
	// the loop below can easily go beyond lightmap boundary. To avoid
	// this problem there is now a test to see if we stay within bounds.
	int src_end = lm_size - lm_w - 1;

	for (int lv = min_lv; lv < max_lv; lv++)
	{
		for (int lu = min_lu; lu < max_lu; lu++)
		{
		if (src >= src_end) return;
	#define GET(map, col, col_long)                 \
		uint8 col##00, col##10, col##01, col##11; \
		col##00 = map [src].col_long;             \
		col##10 = map [src + 1].col_long;         \
		col##01 = map [src + lm_w].col_long;      \
		col##11 = map [src + lm_w + 1].col_long;

		GET (lm_data, r, red);
		GET (lm_data, g, green);
		GET (lm_data, b, blue);
	#undef GET

		// this is somewhat tricky but in practice it works well
	#if 0
		uint32 cellid = ((r00 + g00 + b00)      ) ^
						((r01 + g01 + b01) << 8 ) ^
						((r10 + g10 + b10) << 16) ^
						((r11 + g11 + b11) << 24);
	#else
		uint32 cellid =
			((r00 + hash_table [r01] + hash_table [r10 + 64] + hash_table [r11 + 128])      ) +
			((g00 + hash_table [g01] + hash_table [g10 + 64] + hash_table [g11 + 128]) << 11) +
			((b00 + hash_table [b01] + hash_table [b10 + 64] + hash_table [b11 + 128]) << 22);
	#endif

		// If this lightmap cell did not change, do not recalculate
		if (old_lm [src] == cellid)
		{ src++; continue; }

		old_lm [src] = cellid;
		src++;

		int u = lu << pt_cell_shift;
		int v = lv << pt_cell_shift;
		PI_PIXTYPE *out = ((PI_PIXTYPE *)dst) + (v << pt_shfw) + u;

		int r0 = r00 << 6, r0d = ((r01 - r00) << (6 - pt_cell_shift));
		int r1 = r10 << 6, r1d = ((r11 - r10) << (6 - pt_cell_shift));
		int g0 = g00 << 6, g0d = ((g01 - g00) << (6 - pt_cell_shift));
		int g1 = g10 << 6, g1d = ((g11 - g10) << (6 - pt_cell_shift));
		int b0 = b00 << 6, b0d = ((b01 - b00) << (6 - pt_cell_shift));
		int b1 = b10 << 6, b1d = ((b11 - b10) << (6 - pt_cell_shift));

		// If the texture is less than one lightcell width,
		// we shouldn't fill past the borders of the texture
		int lc_w = pt_w - u;
		if (lc_w > pt_cell_size) lc_w = pt_cell_size;

		int lc_h = pt_h - v;
		if (lc_h > pt_cell_size) lc_h = pt_cell_size;

		for (int vv = 0; vv < lc_h; vv++)
		{
			int tx_idx = ((v + vv + pt_min_v) & tx_andh) << tx_shfw;
			PI_PIXTYPE *__out = out;

			int r = r0;
			
			int g = g0;
			int b = b0;
			int rd, gd, bd;
			if (r0 > r1)
			{
			  // if r1-r0 is very small and negative
			  // rd is slightly wrong, but enough to
			  // cause visible artifacts due to an 
			  // underflow
			  // we work around this by distinguishing
			  // between r0>r1 and r1>r0.
			  rd = -((r0 - r1) >> pt_cell_shift);
			}
			else
			{
			  rd = (r1 - r0) >> pt_cell_shift;
			}
			if (g0 > g1)
			{
			  gd = -((g0 - g1) >> pt_cell_shift);
			}
			else
			{
			  gd = (g1 - g0) >> pt_cell_shift;
			}
			if (b0 > b1)
			{
			  bd = -((b0 - b1) >> pt_cell_shift);
			}
			else
			{
			  bd = (b1 - b0) >> pt_cell_shift;
			}

			int uu = u + pt_min_u;
			int end_uu = uu + lc_w;

			for (; uu < end_uu; uu++)
			{
			  csRGBpixel pix = tx_palette [tx_bitmap [tx_idx + (uu & tx_andw)]];
			#ifdef DEBUG_LIMITS
			  CS_ASSERT ((uint8*)out < (uint8*)dst + ct->size);
			#endif
			  *out++ =
				(lt_R [pix.red   | (r & 0x3f00)] << PI_RS) |
				(lt_G [pix.green | (g & 0x3f00)] << PI_GS) |
				(lt_B [pix.blue  | (b & 0x3f00)] << PI_BS)
			;
			  r += rd; g += gd; b += bd;
			}

			out = __out + pt_w;

			r0 += r0d; r1 += r1d;
			g0 += g0d; g1 += g1d;
			b0 += b0d; b1 += b1d;
		}
		}
		src += src_dlu;
	}
  }
}

#undef LM_NAME
#undef PI_PIXTYPE	  
#undef PI_R5G5B5
#undef PI_R5G6B5
#undef PI_R8G8B8
#undef PI_RM
#undef PI_RS
#undef PI_RB
#undef PI_GM
#undef PI_GS
#undef PI_GB
#undef PI_BM
#undef PI_BS
#undef PI_BB
