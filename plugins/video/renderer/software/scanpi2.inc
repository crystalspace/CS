/*
    This is a subinclude file for scanpi.inc
    It does what draw_pi_scanline_xxx is supposed to do with or
    without clamping texture R,G,B after multiplication to 1.0
    (so that 2*SRC lighting will work correctly)
*/

  while (_dest < _destend)
  {
#ifdef PI_ZUSE
    if (z >= *zbuff)
#endif
    {
#ifndef PI_FLAT
  #ifdef PI_R8G8B8
      register UInt texel = *(((ULong*)bitmap) + ((v >> 16) << bitmap_log2w) + (u >> 16));
  #else
      register UInt texel = Scan.PaletteTable [*(bitmap + ((v >> 16) << bitmap_log2w) + (u >> 16))];
  #endif
  #ifdef PI_COLORKEY
      // if colorkeying is enabled, we will ignore texels with the value 0.
      if (texel)
  #endif
#endif
      {
#if defined (PI_ZUSE) || defined (PI_ZFILL)
        *zbuff = z;
#endif
#if defined (PI_BLEND)
        // The color of the source texel (SRC)
  #if defined (PI_GOURAUD)
    #if PI_RS >= 15
        register UInt rs = ShiftToPos ((texel >> PI_RS_BLEND) * r, 15, PI_RBB) & ((PI_RM2 >> PI_RS) << PI_RBB);
    #else
        register UInt rs = ShiftToPos ((texel & PI_RM_BLEND) * r, PI_RS_BLEND + 15, PI_RBB) & ((PI_RM2 >> PI_RS) << PI_RBB);
    #endif
        register UInt gs = ShiftToPos ((texel & PI_GM_BLEND) * g, PI_GS_BLEND + 15, PI_GBB) & ((PI_GM2 >> PI_GS) << PI_GBB);
        register UInt bs = ShiftToPos ((texel & PI_BM_BLEND) * b, PI_BS_BLEND + 15, PI_BBB) & ((PI_BM2 >> PI_BS) << PI_BBB);
  #else
        register UInt rs = ShiftToPos (texel & PI_RM_BLEND, PI_RS_BLEND, PI_RBB);
        register UInt gs = ShiftToPos (texel & PI_GM_BLEND, PI_GS_BLEND, PI_GBB);
        register UInt bs = ShiftToPos (texel & PI_BM_BLEND, PI_BS_BLEND, PI_BBB);
  #endif

        // The color of the screen pixel (DST)
        UInt pixel = *_dest;
        register UInt rd = (pixel              ) >> PI_RS_BLEND;
        register UInt gd = (pixel & PI_GM_BLEND) >> PI_GS_BLEND;
        register UInt bd = (pixel & PI_BM_BLEND) >> PI_BS_BLEND;

        rs = UInt (Scan.BlendTable [PI_RBT_OFS + rs + rd]);
        gs = UInt (Scan.BlendTable [PI_GBT_OFS + gs + gd]);
        bs = UInt (Scan.BlendTable [PI_BBT_OFS + bs + bd]);

        texel = (rs << PI_RS) | (gs << PI_GS) | (bs << PI_BS);
#elif defined (PI_GOURAUD)
        // The color of the source texel (SRC)
        register UInt rs, gs, bs;
  #ifdef PI_FLAT
    #ifdef PI_CLAMP_RGB
        if (r >= ((PI_RM >> PI_RS) << 16)) rs = PI_RM; else
    #endif
        rs = (r >> (16 - PI_RS)) & PI_RM;
    #ifdef PI_CLAMP_RGB
        if (g >= ((PI_GM >> PI_GS) << 16)) gs = PI_GM; else
    #endif
        gs = (g >> (16 - PI_GS)) & PI_GM;
    #ifdef PI_CLAMP_RGB
        if (b >= ((PI_BM >> PI_BS) << 16)) bs = PI_BM; else
    #endif
        bs = (b >> (16 - PI_BS))/* & PI_BM*/;
        register UInt texel = (rs | gs | bs);
  #else
    #if PI_RS >= 15
        rs = (((texel & PI_RM) >> 15) * r) & (PI_RM | (PI_RM << 1));
    #else
        rs = ((texel & PI_RM) * r) & ((PI_RM << 15) | (PI_RM << 16));
    #endif
        gs = ((texel & PI_GM) * g) & ((PI_GM << 15) | (PI_GM << 16));
        bs = ((texel & PI_BM) * b) & ((PI_BM << 15) | (PI_BM << 16));

        // If in textured Gouraud but not in blending mode, we should normalize
        // R,G,B components as they were multiplied with 7.15 light components
    #ifdef PI_CLAMP_RGB
      #if PI_RS >= 15
        if (rs > PI_RM) rs = PI_RM;
      #else
        if (rs > PI_RM << 15) rs = PI_RM << 15;
      #endif
        if (gs > PI_GM << 15) gs = PI_GM << 15;
        if (bs > PI_BM << 15) bs = PI_BM << 15;
    #endif
    #if PI_RS >= 15
        texel = rs | ((gs | bs) >> 15);
    #else
        texel = (rs | gs | bs) >> 15;
    #endif
  #endif
#elif defined (PI_FLAT)
        UInt texel = Scan.FlatColor;
#endif

        *_dest = texel;
      }
    }
    _dest++;
    u += du; v += dv;
#if defined (PI_ZUSE) || defined (PI_ZFILL)
    zbuff++;
    z += dz;
#endif;
#ifdef PI_GOURAUD
    r += dr; g += dg; b += db;
#endif
  } /* endwhile */
