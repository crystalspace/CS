/*
    This is a subinclude file for scanpi.inc
    It does what draw_pi_scanline_xxx is supposed to do
    with or without clamping r,g,b multipliers to 1.0
    (so that 2*SRC lighting will work correctly)
*/

  while (_dest < _destend)
  {
#ifdef ZUSE
    if (z >= *zbuff)
#endif
    {

#ifdef FLAT
      // if flat shading is desired, we will just use white as our texel color
      (void)u; (void)v; (void)du; (void)dv; (void)bitmap; (void)bitmap_log2w;
  #ifndef GOURAUD
      // if Gouraud is selected, we don't need to intialize texel here.
    #ifdef BPP16
      texel = 0xffff;
    #else
      texel = 0xffffff;
    #endif
  #endif
#else
      // read the texel from the texture
  #ifdef BPP16
      texel = Scan.PaletteTable [*(bitmap + ((v >> 16) << bitmap_log2w) + (u >> 16))];
  #else
      texel = *(((ULong*)bitmap) + ((v >> 16) << bitmap_log2w) + (u >> 16));
  #endif
#endif

#ifdef COLORKEY
      // if colorkeying is enabled, we will ignore texels with the value 0.
      if (texel)
#endif
      {
#ifdef GOURAUD
  #ifdef FLAT
        r1 = r >> 16;
        g1 = g >> 16;
        b1 = b >> 16;
  #else
        r1 =  texel >> REDSHIFT;
        g1 = (texel >> GREENSHIFT) & GREENMASK;
        b1 =  texel  & BLUEMASK;
        r1 = (r1*r) >> (16+REDBITS);
        g1 = (g1*g) >> (16+GREENBITS);
        b1 = (b1*b) >> (16+BLUEBITS);
  #endif
        // Now r1, g1 and b1 contain the color of the new pixel on screen
  #ifdef USEEFFECTS
        ULong pixel = *_dest;
        r2 = (pixel >> REDSHIFT) & REDMASK;
        g2 = (pixel >> GREENSHIFT) & GREENMASK;
        b2 = (pixel) & BLUEMASK;
        // Now r2, g2 and b2 contain the color of the old pixel on pixel
    #ifdef BPP16
        r1 = BlendingTable [r1 * 2 + r2 * 128] / 2;
      #ifdef COLOR555
        g1 = BlendingTable [g1 * 2 + g2 * 128] / 2;
      #else
        g1 = BlendingTable [g1 + g2 * 64];
      #endif
        b1 = BlendingTable [b1 * 2 + b2 * 128] / 2;
    #else
        // 32Bit Blending Mode
        r1 = BlendingTable [(r1 >> 2) + (r2 >> 2) * 64] << 2;
        g1 = BlendingTable [(g1 >> 2) + (g2 >> 2) * 64] << 2;
        b1 = BlendingTable [(b1 >> 2) + (b2 >> 2) * 64] << 2;
    #endif
        // Now r1, g1 and b1 now contain a blend between old and new pixel.
  #endif
  #ifdef CLAMP_RGB
        if (r1 > REDMASK)   r1 = REDMASK;
        if (g1 > GREENMASK) g1 = GREENMASK;
        if (b1 > BLUEMASK)  b1 = BLUEMASK;
  #endif
        texel = (r1 << REDSHIFT) | (g1 << GREENSHIFT) | (b1);
#endif

        *_dest = texel;
#if defined (ZUSE) || defined (ZFILL)
        *zbuff = z;
#endif
      }
    }
    _dest++;
    zbuff++;
    u += du; v += dv; z += dz;
#ifdef GOURAUD
    r += dr; g += dg; b += db;
#endif
  } /* endwhile */
