/*
    This is the template for all 16 and 32 Bit perspective-incorrect
    scanline functions, used by DrawPolygonQuick and DrawPolygonFX.
    To use this include file, you will have to define the following Macros:

    PI_SCANFUNC: Name of the function
    BPP16:       generate code for 16 Bit display
    BPP32:       generate code for 32 Bit display
    GOURAUD:     define only, if you want gouraud shading
    COLOR555:    define, if you are using RGB555 format. Only needed, if you are
                 also using GOURAUD shading and you are using BPP16
    COLOR565:    define, if you are using RGB565 format. Only needed, if you are
                 also using GOURAUD shading and you are using BPP16
    COLORKEY:    define, if you want transparent sections of bitmaps to be drawn
                 transparent.
    FLAT:        The base for all drawing will be a virtual white texture.
                 this setting makes no sense together with COLORKEY.
    ZFILL:       define, if the function is supposed to fill only the z-buffer,
                 but don't test it.
    ZUSE:        define, if you want full z-buffer support (set and test). This
                 can not be combined with ZFILL, but you can omit both.
    USEEFFECTS   Use the optional blending table to determine, what color the new
                 Pixel is supposed to have. This is depending on the original
                 Color on screen and the color of the new pixel. This must be
                 combined with GOURAUD!
*/

// Depending on the Colorformat, we have different masks and shiftvalues
// to get the accrding values for rgb:
#undef REDSHIFT
#undef GREENSHIFT
#undef GREENMASK
#undef BLUEMASK
#undef PIXTYPE
#undef REDBITS
#undef GREENBITS
#undef BLUEBITS
#undef REDMAXFP
#undef GREENMAXFP
#undef BLUEMAXFP

#ifdef BPP16
  #define PIXTYPE UShort

  #ifdef COLOR555
    #define REDSHIFT		10
    #define REDBITS		5
    #define REDMASK		0x1f
    #define GREENSHIFT		5
    #define GREENBITS		5
    #define GREENMASK		0x1f
    #define BLUEMASK		0x1f
    #define BLUEBITS		5
  #endif

  #ifdef COLOR565
    #define REDSHIFT		11
    #define REDBITS		5
    #define REDMASK		0x1f
    #define GREENSHIFT		5
    #define GREENBITS		6
    #define GREENMASK		0x3f
    #define BLUEMASK		0x1f
    #define BLUEBITS		5
  #endif
#endif

#ifdef BPP32
  #define PIXTYPE ULong

  #define REDSHIFT		16
  #define REDBITS		8
  #define REDMASK		0xff
  #define GREENSHIFT		8
  #define GREENBITS		8
  #define GREENMASK		0xff
  #define BLUEMASK		0xff
  #define BLUEBITS		8
#endif

#define REDMAXFP		((REDMASK << 16) | 0xffff)
#define GREENMAXFP		((GREENMASK << 16) | 0xffff)
#define BLUEMAXFP		((BLUEMASK << 16) | 0xffff)

void PI_SCANFUNC (void *dest, int len,
  unsigned long *zbuff, long u, long du, long v, long dv,
  unsigned long z, long dz, unsigned char *bitmap, int bitmap_log2w
#ifdef GOURAUD
  ,ULong r, ULong g, ULong b, long dr, long dg, long db
#endif
#ifdef USEEFFECTS
  ,UByte* BlendingTable
#endif
  )
{
  PIXTYPE *_dest = (PIXTYPE *)dest;
  PIXTYPE *_destend = _dest + len;
  ULong texel;

#ifdef GOURAUD
  ULong r1, g1, b1;
#endif

#ifdef USEEFFECTS
  ULong r2, g2, b2;
#endif

#ifdef GOURAUD
  // Check for overflow (in the cause the source is brighter than 128)
  if (r <= REDMAXFP   && r + dr * len <= REDMAXFP
   && g <= GREENMAXFP && g + dg * len <= GREENMAXFP
   && b <= BLUEMAXFP  && b + db * len <= BLUEMAXFP)
#endif
#include "scanpi2.inc"
#ifdef GOURAUD
  // If either r,g,b is going to overflow past 1.0 we should clamp it to 1.0
  else
  {
#define CLAMP_RGB
#include "scanpi2.inc"
  }
#endif

}

#undef PI_SCANFUNC
#undef ZUSE
#undef ZFILL
#undef GOURAUD
#undef COLOR555
#undef COLOR565
#undef COLORKEY
#undef FLAT
#undef BPP16
#undef BPP32
#undef USEEFFECTS
#undef CLAMP_RGB
