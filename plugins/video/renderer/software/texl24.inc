/*
    This include file is used by texture cache code (16- and 32- bit).
    This is internal loop from ::create_lighted_24bit routine optimized
    for different target pixel formats.

    On entry:

	PI_R5G6B5,
	PI_R5G5B5 or
	PI_R8G8B8 defined (target pixel format)

	TL_WHITE or
	TL_RGB defined (whenever we have white or RGB light)
*/

#include "pixtype.inc"

#ifdef TL_WHITE
#  define __red	whi
#  define __gre	whi
#  define __blu	whi
#else
#  define __red	red
#  define __gre	gre
#  define __blu	blu
#endif

{
  for (dv = 0 ; dv < tcd.mipmap_size ; dv++, tm += w-tcd.mipmap_size)
  {
    if (v+dv >= h)
      break;

    ov_idx = ((v + dv + Imin_v) << shf_w) & and_h;
    end_u = u+tcd.mipmap_size;
    if (end_u > w) end_u = w;
    end_u += Imin_u;
    tm2 = tm + tcd.mipmap_size;
    ULong* ot = otmap + ov_idx;

#ifdef TL_WHITE
    whi = whi_0; whi_d = (whi_1 - whi_0) >> tcd.mipmap_shift;
#else
    red = red_0; red_d = (red_1 - red_0) >> tcd.mipmap_shift;
    gre = gre_0; gre_d = (gre_1 - gre_0) >> tcd.mipmap_shift;
    blu = blu_0; blu_d = (blu_1 - blu_0) >> tcd.mipmap_shift;
#endif

    for (uu = u + Imin_u ; uu < end_u ; uu++)
    {
      UInt pixel = ot [uu & and_w];
      UInt r = (pixel >> 16) * __red >> (31 - PI_RS - PI_RB);
      if (r > PI_RM) r = PI_RM;
#if PI_RS > 0
      else r &= PI_RM;
#endif
      UInt g = ((pixel >> 8) & 0xff) * __gre >> (31 - PI_GS - PI_GB);
      if (g > PI_GM) g = PI_GM;
#if PI_GS > 0
      else g &= PI_GM;
#endif
      UInt b = (pixel & 0xff) * __blu >> (31 - PI_BS - PI_BB);
      if (b > PI_BM) b = PI_BM;
#if PI_BS > 0
      else b &= PI_BM;
#endif
#if defined (PI_R8G8B8) && defined (TOP8BITS_R8G8B8_USED)
      *tm++ = (r | g | b) << PostShift;
#else
      *tm++ = r | g | b;
#endif
#ifdef TL_WHITE
      whi += whi_d;
#else
      red += red_d;
      gre += gre_d;
      blu += blu_d;
#endif
    }
    tm = tm2;
#ifdef TL_WHITE
    whi_0 += whi_0d;
    whi_1 += whi_1d;
#else
    red_0 += red_0d;
    red_1 += red_1d;
    gre_0 += gre_0d;
    gre_1 += gre_1d;
    blu_0 += blu_0d;
    blu_1 += blu_1d;
#endif
  }
}

#undef __red
#undef __gre
#undef __blu
#undef TL_WHITE
#undef TL_RGB
#undef PI_R5G5B5
#undef PI_R5G6B5
#undef PI_R8G8B8
