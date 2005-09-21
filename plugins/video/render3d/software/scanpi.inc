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
    PI_TILE	 Perform tiling on the source textre
*/

/*
    The routine can look clumsy and uncomprehensible
    but that's the price we have to pay to the
    God Of Optimization :-(
*/

#include "pixtype.inc"
#include "blend.inc"

/*
    For Gouraud-shaded polys additional parameters r,g,b and deltas
    dr,dg,db are passed to the routine. They are in #.8 fixed-point
    format for all lighting models, where # is equal to the number of
    bits for the respective color component (i.e. if "r" component
    has 5 bits then 1.0 == 0x2000, 1/32 == 0x100 and so on)
*/
void PI_SCANFUNC (void *dest, int len, uint32 *zbuff,
  int32 u, int32 du, int32 v, int32 dv, float z, float dz,
  unsigned char *bitmap, int bitmap_log2w
#ifdef PI_GOURAUD
  ,uint32 r, uint32 g, uint32 b, int32 dr, int32 dg, int32 db, bool clamp
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
#undef PI_R5G5B5
#undef PI_R5G6B5
#undef PI_R8G8B8
#undef PI_COLORKEY
#undef PI_FLAT
#undef PI_BLEND
#undef PI_CLAMP_RGB
#undef PI_TILE


