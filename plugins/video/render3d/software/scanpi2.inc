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
    This is a subinclude file for scanpi.inc
    It does what scan_pi_xxx is supposed to do with or
    without clamping texture R,G,B after multiplication to 1.0
    (so that 2*SRC lighting will work correctly)
*/

  while (_dest < _destend)
  {
#if defined (PI_ZUSE) || defined (PI_ZTEST) || defined (PI_ZFILL)
    const uint32 zFixed = csQfixed24 (z);
#endif
#if defined (PI_ZUSE) || defined (PI_ZTEST)
    if (zFixed > *zbuff)
#endif
    {
#ifndef PI_FLAT
  #ifdef PI_TILE
      uint32 texel = bitmap [(((v >> 16) & Scan.and_h) << bitmap_log2w)
      	+ ((u >> 16) & Scan.and_w)];
  #else
      uint32 texel = bitmap [((v >> 16) << bitmap_log2w) + (u >> 16)];
  #endif
  #ifdef PI_COLORKEY
      // if colorkeying is enabled, we will ignore texels with the value 0.
      if (texel)
  #endif
#endif
      {
#if defined (PI_ZUSE) || defined (PI_ZFILL)
        *zbuff = zFixed;
#endif
#if defined (PI_GOURAUD) || defined (PI_BLEND)
  #if defined (PI_FLAT)
    #if defined (PI_BLEND)
        csRGBpixel rgbtexel = Scan.FlatRGB;
    #endif
  #else
        csRGBpixel rgbtexel = *(csRGBpixel)&texel;
  #endif
#endif
#if defined (PI_BLEND)
        // The color of the source texel (SRC)
  #if defined (PI_GOURAUD)
        // This works as follows (for example let's take the case when red
        // component is 5 bits wide). The `r' variable is in 5.16 format in
        // this case, so after multiplication we have something in 5.24 format
        // (the rgbtexel variable has 8 bits for every of red, green and blue).
        // Now we should shrink those .24 bits to .5 (i.e. >> 5.5 format
        // (actually 6.5 format since we allow 2*SRC lighting)).
        // For this we use the formula: shift_amount = (24 - X) where
        // X is the number of bits. In the above example we'll get 24+5=19.
        // Thus, 24 - 19 = 5.
        unsigned rs = ((r*rgbtexel.red  ) >> (24 - PI_RB_BLEND)) & PI_RM_BLEND;
        unsigned gs = ((g*rgbtexel.green) >> (24 - PI_GB_BLEND)) & PI_GM_BLEND;
        unsigned bs = ((b*rgbtexel.blue ) >> (24 - PI_BB_BLEND)) & PI_BM_BLEND;
  #else
        // Flat lighting. Here we use the following formula: since we need
        // to convert the 8-bit r, g and b values from rgbtexel to X.X format,
        // thus we shift the 8-bit value (X + X - 8) times. For the example
        // above (5 bits) this turns into 5 + 5 - 8 = 2 times. After this we're
        // AND'ing the result with some mask to strip off unused lower bits.
        unsigned rs = (rgbtexel.red   << (PI_RB_BLEND * 2 - 8)) & PI_RM_BLEND;
        unsigned gs = (rgbtexel.green << (PI_GB_BLEND * 2 - 8)) & PI_GM_BLEND;
        unsigned bs = (rgbtexel.blue  << (PI_BB_BLEND * 2 - 8)) & PI_BM_BLEND;
  #endif

        // The color of the screen pixel (DST)
        // The DST is in screen format, thus we AND and shift the corresponding
        // subvalues to get just the lower X bits from them (where X is the
        // number of bits per color component except in 32bpp modes where we
        // use 6 bit values).
        unsigned pixel = *_dest;
        unsigned rd = (pixel & PI_RM) >> PI_RS_DST_BLEND;
        unsigned gd = (pixel & PI_GM) >> PI_GS_DST_BLEND;
        unsigned bd = (pixel & PI_BM) >> PI_BS_DST_BLEND;

        rs = unsigned (Scan.BlendTable [PI_RBT_OFS + rs + rd]);
        gs = unsigned (Scan.BlendTable [PI_GBT_OFS + gs + gd]);
        bs = unsigned (Scan.BlendTable [PI_BBT_OFS + bs + bd]);

        pixel = (rs << PI_RS) | (gs << PI_GS) | (bs << PI_BS);
#elif defined (PI_GOURAUD)
        // The color of the source texel (SRC)
        unsigned rs, gs, bs;
  #ifdef PI_FLAT
    #ifdef PI_CLAMP_RGB
        if (r >= ((PI_RM >> PI_RS) << 16)) rs = PI_RM; else
    #endif
        // Separate case if high-byte is occupied by color component.
    #if (PI_RS > 16)
        rs = (r << (PI_RS - 16)) & PI_RM;
    #else
        rs = (r >> (16 - PI_RS)) & PI_RM;
    #endif
    #ifdef PI_CLAMP_RGB
        if (g >= ((PI_GM >> PI_GS) << 16)) gs = PI_GM; else
    #endif
        gs = (g >> (16 - PI_GS)) & PI_GM;
    #ifdef PI_CLAMP_RGB
        if (b >= ((PI_BM >> PI_BS) << 16)) bs = PI_BM; else
    #endif
        bs = (b >> (16 - PI_BS)) & PI_BM;
        unsigned pixel = rs | gs | bs;
  #else
        // If in textured Gouraud but not in blending mode, we should normalize
        // R,G,B components as they were multiplied with #.16 light components
        rs = (rgbtexel.red * r);
        // Separate case if high-byte is occupied by color component.
    #ifdef PI_CLAMP_RGB
        if (rs > PI_RM_GOUTEX) rs = PI_RM_GOUTEX;
    #endif
    #if (PI_RS_GOUTEX >= PI_RS)
        rs = (rs >> (PI_RS_GOUTEX - PI_RS)) & PI_RM;
    #else
        rs = (rs << (PI_RS - PI_RS_GOUTEX)) & PI_RM;
    #endif
        gs = (rgbtexel.green * g);
    #ifdef PI_CLAMP_RGB
        if (gs > PI_GM_GOUTEX) gs = PI_GM_GOUTEX;
    #endif
    #if (PI_GS_GOUTEX >= PI_GS)
        gs = (gs >> (PI_GS_GOUTEX - PI_GS)) & PI_GM;
    #else
        gs = (gs << (PI_GS - PI_GS_GOUTEX)) & PI_GM;
    #endif
        bs = (rgbtexel.blue * b);
    #ifdef PI_CLAMP_RGB
        if (bs > PI_BM_GOUTEX) bs = PI_BM_GOUTEX;
    #endif
    #if (PI_BS_GOUTEX >= PI_BS)
        bs = (bs >> (PI_BS_GOUTEX - PI_BS)) & PI_BM;
    #else
        bs = (bs << (PI_BS - PI_BS_GOUTEX)) & PI_BM;
    #endif
        unsigned pixel = rs | gs | bs;
  #endif
#elif !defined (PI_FLAT)
        unsigned pixel = texel;
#endif

        *_dest = pixel;
      }
    }
    _dest++;
    u += du; v += dv;
#if defined (PI_ZUSE) || defined (PI_ZFILL) || defined (PI_ZTEST)
    zbuff++;
    z += dz;
#endif
#ifdef PI_GOURAUD
    r += dr; g += dg; b += db;
#endif
  } /* endwhile */
