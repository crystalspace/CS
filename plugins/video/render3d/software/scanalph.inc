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
    Perspective-correct alpha-mapped texture drawing with or without lightmap.
    Before including this file the following macros can/should be defined:

	A_SCANFUNC
	  - The name of scanline function
	A_MAP
	  - If using a lightmap (otherwise using an unlighted texture)
	A_PI
	  - If you want the perspective-incorrect scanline function
	A_R5G5B5,
	A_R5G6B5,
	A_R8G8B8
	  - The corresponding pixel format.

*/

// Intermediate pixel format
#if defined (A_R5G5B5) || defined (A_R5G6B5)
#  define A_IPIXTYPE	uint16
#else
#  define A_IPIXTYPE	uint32
#endif

#if defined (A_R5G5B5)
#  define A_MASK_RB   0x00007c1f
#  define A_MASK_G    0x000003e0
#  if defined (A_MAP)
#    define A_COLORMAP(x)     x
#  else
#    define A_COLORMAP(x)     COLORMAP [x]
#  endif
#  define A_DSTTEX    *_dest
#  define A_NORMSHIFT 5
#  define A_PIXTYPE   uint16
#elif defined (A_R5G6B5)
#  define A_MASK_RB	0x0000f81f
#  define A_MASK_G	0x000007e0
#  if defined (A_MAP)
#    define A_COLORMAP(x)	x
#  else
#    define A_COLORMAP(x)	COLORMAP [x]
#  endif
#  define A_DSTTEX	*_dest
#  define A_NORMSHIFT	5
#  define A_PIXTYPE	uint16
#elif defined (A_R8G8B8)
#  define A_MASK_RB	0x00ff00ff
#  define A_MASK_G	0x0000ff00
#  if defined (A_MAP)
#    define A_COLORMAP(x)	x
#  else
#    define A_COLORMAP(x)	COLORMAP [x]
#  endif
#  define A_DSTTEX	*_dest
#  define A_NORMSHIFT	8
#  define A_PIXTYPE	uint32
#else
#  error "ERROR: Pixel format not defined before including scanalph.inc!"
#endif

#if defined (A_R8G8B8)
#define A_PIXEL_PREPROC(x)  R8G8B8_PIXEL_PREPROC(x)
#define A_PIXEL_POSTPROC(x) R8G8B8_PIXEL_POSTPROC(x)
#else
#define A_PIXEL_PREPROC(x)  (x)
#define A_PIXEL_POSTPROC(x) (x)
#endif

#ifndef A_PI

#if defined (A_ZUSE) || defined (A_ZTEST)
#  define IF_ZBUF_OK if (izz > *z_buf)
#else
#  define IF_ZBUF_OK
#endif

#if defined (A_ZNONE)
#  define Z_FILL
#  define Z_INC
#elif defined (A_ZTEST)
#  define Z_FILL
#  define Z_INC		izz += dzz; z_buf++
#else
#  define Z_FILL	*z_buf = izz
#  define Z_INC		izz += dzz; z_buf++
#endif

#ifdef A_MAP
#  define SCANMAP
#  define TEXCOORDS
#  define A_SRCADDR	((vv >> 16) << shifter) + (uu >> 16)
#  define A_ASRCADDR    (((uu >> 16) + a_uofs) & ander_w) + \
			 (((vv >> shifter_h) + a_vofs) & ander_h)
#else
#  define A_SRCADDR	((uu >> 16) & ander_w) + ((vv >> shifter_h) & ander_h)
#  define A_ASRCADDR    srcaddr
#endif

#define SCANLOOP \
  do									\
  {									\
    IF_ZBUF_OK								\
    {									\
      Z_FILL;								\
      unsigned srcaddr = A_SRCADDR;					\
      uint8 alpha = Scan.AlphaMap [A_ASRCADDR];				\
      A_IPIXTYPE src = A_PIXEL_PREPROC(A_COLORMAP (srcTex [srcaddr]));	\
      A_IPIXTYPE dst = A_PIXEL_PREPROC(A_DSTTEX);			\
      unsigned src_rb = src & A_MASK_RB;				\
      unsigned dst_rb = dst & A_MASK_RB;				\
      dst_rb = dst_rb + (alpha * (src_rb - dst_rb) >> A_NORMSHIFT);	\
      unsigned src_g = src & A_MASK_G;					\
      unsigned dst_g = dst & A_MASK_G;					\
      dst_g = dst_g + (alpha * (src_g - dst_g) >> A_NORMSHIFT);		\
      *_dest = A_PIXEL_POSTPROC(					\
        (dst_rb & A_MASK_RB) | (dst_g & A_MASK_G));			\
    }									\
    _dest++;								\
    uu += duu;								\
    vv += dvv;								\
    Z_INC;								\
  }									\
  while (_dest <= _destend)

#define SCANFUNC A_SCANFUNC
#include "scanln.inc"
#undef IF_ZBUF_OK

#else // A_PI

void A_SCANFUNC (void *dest, int len, uint32 *zbuff,
  int32 u, int32 du, int32 v, int32 dv, float z, float dz,
  unsigned char *bitmap, int bitmap_log2w)
{
  A_PIXTYPE *_dest = (A_PIXTYPE *)dest;
  A_PIXTYPE *_destend = _dest + len;
#ifdef A_MAP
  int a_uofs = Scan.amap_uofs;
  int a_vofs = Scan.amap_vofs;
#endif
  while (_dest < _destend)
  {
    const uint32 zFixed = csQfixed24 (z);
#if defined (A_ZUSE)
    if (zFixed > *zbuff)
#endif
    {
#if defined (A_ZUSE) || defined (A_ZFIL)
      *zbuff = zFixed;
#endif
      unsigned srcaddr = ((v >> 16) << bitmap_log2w) + (u >> 16);
      uint8 alpha = Scan.AlphaMap [srcaddr];
      A_IPIXTYPE src = A_PIXEL_PREPROC(A_COLORMAP (bitmap [srcaddr]));
      A_IPIXTYPE dst = A_PIXEL_PREPROC(A_DSTTEX);
      unsigned src_rb = src & A_MASK_RB;
      unsigned dst_rb = dst & A_MASK_RB;
      dst_rb = dst_rb + (alpha * (src_rb - dst_rb) >> A_NORMSHIFT);
      unsigned src_g = src & A_MASK_G;
      unsigned dst_g = dst & A_MASK_G;
      dst_g = dst_g + (alpha * (src_g - dst_g) >> A_NORMSHIFT);
      *_dest = A_PIXEL_POSTPROC(
        (dst_rb & A_MASK_RB) | (dst_g & A_MASK_G));
      u += du; v += dv;
    }
#if defined (A_ZUSE) || defined (A_ZFIL)
    zbuff++;
    z += dz;
#endif
    _dest++;
  }
}

#endif // A_PI

#undef A_PIXEL_POSTPROC
#undef A_PIXEL_PREPROC
#undef A_SCANFUNC
#undef A_MAP
#undef A_PI
#undef A_R5G5B5
#undef A_R5G6B5
#undef A_R8G8B8
#undef A_MASK_RB
#undef A_MASK_G
#undef A_NORMSHIFT
#undef A_COLORMAP
#undef A_SRCADDR
#undef A_ASRCADDR
#undef A_DSTTEX
#undef A_IPIXTYPE
#undef A_PIXTYPE
#undef A_ZUSE
#undef A_ZFILL
#undef A_ZNONE
#undef A_ZTEST
#undef Z_FILL
#undef Z_INC
