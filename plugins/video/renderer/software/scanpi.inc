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

#ifndef __SCANPI_INC__
#define __SCANPI_INC__

static inline UInt ShiftToPos (UInt val, int lsb, int pos)
{
  if (lsb >= pos)
    return (pos >= 0) ? val >> (lsb - pos) : val >> lsb;
  else
    return val << (pos - lsb);
}

#endif // __SCANPI_INC__

// Depending on the Colorformat, we define different masks and shiftvalues
// to get the accrding values for rgb; R,G,B stands for Red, Green and Blue,
// and M, S, B stands for Mask, Shift and Bits correspondingly.
#undef PI_PIXTYPE
#undef PI_RM
#undef PI_RS
#undef PI_RB
#undef PI_GM
#undef PI_GS
#undef PI_GB
#undef PI_BM
#undef PI_BS
#undef PI_BB

#if defined (PI_R5G5B5)
#  define PI_PIXTYPE	UShort
#  define PI_RM		0x00007C00
#  define PI_RS		10
#  define PI_RB		5
#  define PI_GM		0x000003E0
#  define PI_GS		5
#  define PI_GB		5
#  define PI_BM		0x0000001F
#  define PI_BS		0
#  define PI_BB		5
#elif defined (PI_R5G6B5)
#  define PI_PIXTYPE	UShort
#  define PI_RM		0x0000F800
#  define PI_RS		11
#  define PI_RB		5
#  define PI_GM		0x000007E0
#  define PI_GS		5
#  define PI_GB		6
#  define PI_BM		0x0000001F
#  define PI_BS		0
#  define PI_BB		5
#elif defined (PI_R8G8B8)
#  define PI_PIXTYPE	ULong
#  define PI_RM		0x00FF0000
#  define PI_RS		16
#  define PI_RB		8
#  define PI_GM		0x0000FF00
#  define PI_GS		8
#  define PI_GB		8
#  define PI_BM		0x000000FF
#  define PI_BS		0
#  define PI_BB		8
#else
#  error "ERROR: Pixel format not defined before including scanpi.inc!"
#endif

// Masks for accessing blending table. Since blending table expects a different
// number of bits per color component (for example, in 32bpp mode we have
// 6 bits per color component blending tables) we should strip off
// unused bits from color component.
#undef PI_RM_BLEND
#undef PI_RM2
#undef PI_RS_BLEND
#undef PI_GM_BLEND
#undef PI_GM2
#undef PI_GS_BLEND
#undef PI_BM_BLEND
#undef PI_BM2
#undef PI_BS_BLEND
#undef PI_RBB
#undef PI_GBB
#undef PI_BBB

#ifdef PI_R8G8B8
#  define PI_RS_BLEND	18
#  define PI_RM_BLEND	0x00FC0000
#  define PI_RBB	6
#  define PI_GS_BLEND	10
#  define PI_GM_BLEND	0x0000FC00
#  define PI_GBB	6
#  define PI_BS_BLEND	2
#  define PI_BM_BLEND	0x000000FC
#  define PI_BBB	6
#else
#  define PI_RS_BLEND	PI_RS
#  define PI_RM_BLEND	PI_RM
#  define PI_RBB	PI_RB
#  define PI_GS_BLEND	PI_GS
#  define PI_GM_BLEND	PI_GM
#  define PI_GBB	PI_GB
#  define PI_BS_BLEND	PI_BS
#  define PI_BM_BLEND	PI_BM
#  define PI_BBB	PI_BB
#endif

#define PI_RM2		(PI_RM | (PI_RM << 1))
#define PI_GM2		(PI_GM | (PI_GM << 1))
#define PI_BM2		(PI_BM | (PI_BM << 1))

#undef PI_RBT_OFS
#undef PI_GBT_OFS
#undef PI_BBT_OFS

#ifdef PI_R5G6B5
#  define PI_RBT_OFS	0
#  define PI_GBT_OFS	32*64
#  define PI_BBT_OFS	0
#else
#  define PI_RBT_OFS	0
#  define PI_GBT_OFS	0
#  define PI_BBT_OFS	0
#endif

/*
    For Gouraud-shaded polys additional parameters r,g,b and deltas
    dr,dg,db are passed to the routine. They are in 7.15 fixed-point
    format (i.e. 1.0 == 0x8000, 2.0 == 0x10000) and in #.16 fixed-point
    format for non-textured models, where # is equal to the number of
    bits for the respective color component (i.e. if "r" component
    has 5 bits then 1.0 == 0x200000, 1/32 == 0x10000 and so on)
*/
void PI_SCANFUNC (void *dest, int len, unsigned long *zbuff,
  long u, long du, long v, long dv, unsigned long z, long dz,
  unsigned char *bitmap, int bitmap_log2w
#ifdef PI_GOURAUD
  ,ULong r, ULong g, ULong b, long dr, long dg, long db, bool clamp
#endif
  )
{
  PI_PIXTYPE *_dest = (PI_PIXTYPE *)dest;
  PI_PIXTYPE *_destend = _dest + len;

#ifdef PI_FLAT
  // if flat shading is desired, we will just use white as our texel color
  (void)u; (void)v; (void)du; (void)dv; (void)bitmap; (void)bitmap_log2w;
#endif

#if defined (PI_GOURAUD) && !defined (PI_BLEND)
  // Check for overflow (in the case the light source is brighter than 128)
  if (!clamp)
#  include "scanpi2.inc"
  // If either r,g,b is going to overflow past 1.0 we should clamp it to 1.0
  else
#  define PI_CLAMP_RGB
#  include "scanpi2.inc"
#else
#  include "scanpi2.inc"
#endif
}

#undef PI_SCANFUNC
#undef PI_ZUSE
#undef PI_ZFILL
#undef PI_GOURAUD
#undef PI_R5G5B5
#undef PI_R5G6B5
#undef PI_R8G8B8
#undef PI_COLORKEY
#undef PI_FLAT
#undef PI_BLEND
#undef PI_CLAMP_RGB
