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
    This is the template for all 16 and 32 Bit perspective-incorrect
    scanline functions, used by DrawPolygonQuick and DrawPolygonFX.
    To use this include file, you will have to define the following Macros:

    PI_SCANFUNC  Name of the function
    PI_R5G5B5    generate code for 15 bit R5/G5/B5 display
    PI_R5G6B5    generate code for 16 bit R5/G6/B5 display
    PI_R8G8B8    generate code for 32 bit R8/G8/B8 display
    PI_GOURAUD   define only, if you want gouraud shading
    PI_COLORKEY  define, if you want bitmap pixels equal to zero
                 to be drawn transparent (to not be drawn, in fact).
    PI_FLAT      The base for all drawing will be a virtual white texture.
                 this setting makes no sense together with COLORKEY.
    PI_ZFILL     define if Z-buffer test should be disabled, just fill.
    PI_ZUSE      define if each pixel should be tested against Z-buffer
                 contents, and not be drawn if it is below the "surface".
    PI_BLEND     Apply the function given with a blending table passed as
                 an argument to screen pixel and texel to determine the color
                 of resulting pixel. This must be combined with PI_GOURAUD!
*/

/*
    The routine can look clumsy and uncomprehensible
    but that's the price we have to pay to the
    God Of Optimization :-(
*/

#include "pixtype.inc"

// Masks for accessing blending table. Since blending table expects a different
// number of bits per color component (for example, in 32bpp mode we have
// 6 bits per color component blending tables) we should strip off
// unused bits from color component. Since we allow 2*SRC lighting, we actually
// use one more bit per each component (to allow multiplication overflows).
#undef PI_RM_BLEND
#undef PI_GM_BLEND
#undef PI_BM_BLEND
#undef PI_RS_GOUTEX
#undef PI_GS_GOUTEX
#undef PI_BS_GOUTEX
#undef PI_RM_GOUTEX
#undef PI_GM_GOUTEX
#undef PI_BM_GOUTEX
#undef PI_RBB
#undef PI_GBB
#undef PI_BBB
#undef PI_RBT_OFS
#undef PI_GBT_OFS
#undef PI_BBT_OFS

#ifdef PI_R8G8B8
#  define PI_RB_BLEND	6
#  define PI_GB_BLEND	6
#  define PI_BB_BLEND	6
#else
#  define PI_RB_BLEND	PI_RB
#  define PI_GB_BLEND	PI_GB
#  define PI_BB_BLEND	PI_BB
#endif

#define PI_RM_BLEND	(((2 << PI_RB_BLEND) - 1) << PI_RB_BLEND)
#define PI_GM_BLEND	(((2 << PI_GB_BLEND) - 1) << PI_GB_BLEND)
#define PI_BM_BLEND	(((2 << PI_BB_BLEND) - 1) << PI_BB_BLEND)

#define PI_RS_DST_BLEND	(PI_RS + (PI_RB - PI_RB_BLEND))
#define PI_GS_DST_BLEND	(PI_GS + (PI_GB - PI_GB_BLEND))
#define PI_BS_DST_BLEND	(PI_BS + (PI_BB - PI_BB_BLEND))

#define PI_RM_GOUTEX	((PI_RM >> PI_RS) << (24 - (PI_RB - PI_RB_BLEND)))
#define PI_GM_GOUTEX	((PI_GM >> PI_GS) << (24 - (PI_GB - PI_GB_BLEND)))
#define PI_BM_GOUTEX	((PI_BM >> PI_BS) << (24 - (PI_BB - PI_BB_BLEND)))

#define PI_RS_GOUTEX	(24 - (PI_RB - PI_RB_BLEND))
#define PI_GS_GOUTEX	(24 - (PI_GB - PI_GB_BLEND))
#define PI_BS_GOUTEX	(24 - (PI_BB - PI_BB_BLEND))

// In 5-6-5 and 8-bit indexed modes we'll need two blending tables:
// one for 5-bit transformations and second for 6-bit transformations
#if (PI_GB == 6)
#  define PI_RBT_OFS	0
#  define PI_GBT_OFS	32*64
#  define PI_BBT_OFS	0
#else
#  define PI_RBT_OFS	0
#  define PI_GBT_OFS	0
#  define PI_BBT_OFS	0
#endif

#if defined (PI_INDEX8)
#  define INV_COLORMAP(x) Scan.inv_cmap [x]
#  define SCR_COLORMAP(x) Scan.GlobalCMap [x]
#else
#  define INV_COLORMAP(x) x
#  define SCR_COLORMAP(x) x
#endif

/*
    For Gouraud-shaded polys additional parameters r,g,b and deltas
    dr,dg,db are passed to the routine. They are in #.8 fixed-point
    format for all lighting models, where # is equal to the number of
    bits for the respective color component (i.e. if "r" component
    has 5 bits then 1.0 == 0x2000, 1/32 == 0x100 and so on)
*/
void PI_SCANFUNC (void *dest, int len, unsigned long *zbuff,
  long u, long du, long v, long dv, unsigned long z, long dz,
  unsigned char *bitmap, int bitmap_log2w
#ifdef PI_GOURAUD
  ,ULong r, ULong g, ULong b, long dr, long dg, long db, bool clamp
#endif
  )
{
  (void)z; (void)dz; (void)zbuff;

#if defined (PI_GOURAUD) && defined (PI_BLEND)
  (void) clamp; //remove the warning about an unused parameter
#endif

  PI_PIXTYPE *_dest = (PI_PIXTYPE *)dest;
  PI_PIXTYPE *_destend = _dest + len;

#ifdef PI_FLAT
  // if flat shading is desired, we will just use one color
  (void)u; (void)v; (void)du; (void)dv; (void)bitmap; (void)bitmap_log2w;
  #if !defined (PI_GOURAUD) && !defined (PI_BLEND)
  unsigned pixel = Scan.FlatColor;
  #endif
#endif

#if defined (PI_GOURAUD) && !defined (PI_BLEND)
  // Check for overflow (in the case the light source is brighter than 128)
  if (!clamp)
  #include "scanpi2.inc"
  // If either r,g,b is going to overflow past 1.0 we should clamp it to 1.0
  else
  #define PI_CLAMP_RGB
#endif
#include "scanpi2.inc"
}

#undef PI_SCANFUNC
#undef PI_ZUSE
#undef PI_ZFILL
#undef PI_ZTEST
#undef PI_GOURAUD
#undef PI_INDEX8
#undef PI_R5G5B5
#undef PI_R5G6B5
#undef PI_R8G8B8
#undef PI_COLORKEY
#undef PI_FLAT
#undef PI_BLEND
#undef PI_CLAMP_RGB
