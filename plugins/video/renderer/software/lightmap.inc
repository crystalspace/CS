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
	PI_INDEX8,
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

	- iPolygonTexture is the object that is in charge for containing
	  all parameters related to a single mipmap level on a Polygon3D.
	  The variables that contain values related to iPolygonTexture
	  will have the "pt" prefix, i.e. pt_h for example is PolygonTexture's
	  height.
	- iLightMap is the object that is in charge for containing all
	  lighting information for the iPolygonTexture. Basically it contains
	  three lightmaps: red, green an blue, each cell of the lightmap
	  covers a square of texels. The square size can be queried from
	  polygontexture object using GetMipMapSize () method. For example,
	  if it returns 16, this means that every cell of the lightmap
	  covers a 16x16 square of texels on the texture. The prefix for
	  intermediate variables related to the lightmap is "lm".
	- csTexture is the actual unlighted (original) texture in
          driver-dependent format. The csTextureHandleSoftware object
	  contains four of these objects - one for each mipmap level.
	  The intermediate variables related to csTexture are prefixed
	  with "tx".
*/

#include "pixtype.inc"

void LM_NAME (iPolygonTexture *pt, SoftwareCachedTexture *ct, 
	      csTextureHandleSoftware *texmm, csTextureManagerSoftware *texman,
	      float u_min, float v_min, float u_max, float v_max)
{
  int mipmap = ct->mipmap;
  int mipmap_delta = (1 << mipmap);

  csTextureSoftware *tx = (csTextureSoftware *)texmm->get_texture (mipmap);
  csRGBpixel *tx_palette = texmm->GetColorMap ();
  UByte *tx_bitmap = tx->get_bitmap ();

  int pt_min_u = pt->GetIMinU () >> mipmap;
  int pt_min_v = pt->GetIMinV () >> mipmap;
  int pt_cell_size = pt->GetLightCellSize () >> mipmap;
  int pt_cell_shift = pt->GetLightCellShift () - mipmap;
  int pt_shfw = pt->GetShiftU () - mipmap;
  int pt_w = 1 << pt_shfw;
  int pt_h = (pt->GetHeight () + mipmap_delta) >> mipmap;

  iLightMap *lm = pt->GetLightMap ();
  int lm_w = lm->GetWidth ();
  UByte *lm_data = lm->GetMapData ();
  int lm_size = lm->GetSize ();

  void *dst = ct->get_bitmap ();
  ULong *old_lm = (ULong *)ct->get_lightmap ();

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
  UByte *lt_R = texman->lightmap_tables [0];
  UByte *lt_G = texman->lightmap_tables [1];
  UByte *lt_B = texman->lightmap_tables [2];

  int min_lu = QRound (floor (u_min)) >> pt_cell_shift;
  int max_lu = (QRound (ceil (u_max)) + pt_cell_size - 1) >> pt_cell_shift;
  int min_lv = QRound (floor (v_min)) >> pt_cell_shift;
  int max_lv = (QRound (ceil (v_max)) + pt_cell_size - 1) >> pt_cell_shift;

  if (min_lu < 0) min_lu = 0;
  if (min_lv < 0) min_lv = 0;

  if (min_lu >= max_lu) return;
  if (min_lv >= max_lv) return;

  int max_lu2 = ((((pt->GetOriginalWidth () + mipmap_delta) >> mipmap) +
    pt_cell_size - 1) >> pt_cell_shift);
  if (max_lu > max_lu2) max_lu = max_lu2;

  int max_lv2 = lm->GetHeight ();
  if (max_lv > max_lv2) max_lv = max_lv2;

  int src = min_lu + min_lv * lm_w;
  int src_dlu = lm_w - (max_lu - min_lu);

  // @@@ Jorrit: the code below is a bit clumsy. When an overflow
  // is detected we calculate the number of lightmap lines that
  // cannot be calculated due to overflowing the lightmap bounds (why
  // this can happen in the first place is a mistery).
  // For those lines we will reuse the last line of the lightmap
  // that we can safely use.
  int src_end = src + (src_dlu + max_lu-min_lu) * (max_lv-min_lv)
  	- src_dlu + lm_w + 1;
  int overflow_lv;
  if (src_end > lm_size)
    overflow_lv = max_lv - ((src_end-lm_size+lm_w-1) / lm_w);
  else
    overflow_lv = max_lv;

  int lmlm_w = lm_w << 2;
  int src_old = src;
  for (int lv = min_lv; lv < max_lv; lv++)
  {
#if 0
    if (lv >= overflow_lv)
    {
      // Go back one line in the lightmap data.
      src_old = src_old - lm_w;
    }
#endif

    for (int lu = min_lu; lu < max_lu; lu++)
    {
      int lmsrc = src_old << 2;
#if 0
      CS_ASSERT (src_old >= 0);
      CS_ASSERT ((src_old + lm_w + 1) < lm_size);
#endif
#define GET(map, col)				\
      UByte col##00, col##10, col##01, col##11;	\
      col##00 = map [lmsrc];			\
      col##10 = map [lmsrc + 4];		\
      col##01 = map [lmsrc + lmlm_w];		\
      col##11 = map [lmsrc + lmlm_w + 4];

      GET (lm_data, r);
      GET ((lm_data+1), g);
      GET ((lm_data+2), b);
#undef GET

      // this is somewhat tricky but in practice it works well
#if 0
      ULong cellid = ((r00 + g00 + b00)      ) ^
                     ((r01 + g01 + b01) << 8 ) ^
                     ((r10 + g10 + b10) << 16) ^
                     ((r11 + g11 + b11) << 24);
#else
      ULong cellid =
        ((r00 + hash_table [r01] + hash_table [r10 + 64] + hash_table [r11 + 128])      ) +
        ((g00 + hash_table [g01] + hash_table [g10 + 64] + hash_table [g11 + 128]) << 11) +
        ((b00 + hash_table [b01] + hash_table [b10 + 64] + hash_table [b11 + 128]) << 22);
#endif

      // If this lightmap cell did not change, do not recalculate
      if (old_lm [src] == cellid)
      { src_old++; src++; continue; }

      old_lm [src] = cellid;
      src_old++; src++;

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

        int r = r0, rd = (r1 - r0) >> pt_cell_shift;
        int g = g0, gd = (g1 - g0) >> pt_cell_shift;
        int b = b0, bd = (b1 - b0) >> pt_cell_shift;

        int uu = u + pt_min_u;
        int end_uu = uu + lc_w;

        for (; uu < end_uu; uu++)
        {
          csRGBpixel pix = tx_palette [tx_bitmap [tx_idx + (uu & tx_andw)]];
	  *out++ =
#if defined (PI_INDEX8)
            Scan.inv_cmap [
#endif
            (lt_R [pix.red   | (r & 0x3f00)] << PI_RS) |
            (lt_G [pix.green | (g & 0x3f00)] << PI_GS) |
            (lt_B [pix.blue  | (b & 0x3f00)] << PI_BS)
#if defined (PI_INDEX8)
            ]
#endif
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
    src_old += src_dlu;
  }
}

#undef LM_NAME
#undef PI_PIXTYPE
#undef PI_INDEX8
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
