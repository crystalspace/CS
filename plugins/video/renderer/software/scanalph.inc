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
	A_INDEX8,
	A_R5G5B5,
	A_R5G6B5,
	A_R8G8B8
	  - The corresponding pixel format.

*/

#if defined (A_INDEX8)
#  define INV_COLORMAP(x) Scan.inv_cmap [x]
#else
#  define INV_COLORMAP(x) x
#endif

// Intermediate pixel format
#if defined (A_R5G5B5) || defined (A_R5G6B5) || defined (A_INDEX8)
#  define A_IPIXTYPE	uint16
#else
#  define A_IPIXTYPE	uint32
#endif

#if defined (A_INDEX8)
#  define A_MASK_RB	((((1 << RGB2PAL_BITS_R) - 1) << (RGB2PAL_BITS_G + RGB2PAL_BITS_B)) \
                        | (1 << RGB2PAL_BITS_B) - 1)
#  define A_MASK_G	(((1 << RGB2PAL_BITS_G) - 1) << RGB2PAL_BITS_B)
#  if defined (A_MAP)
#    define A_COLORMAP(x)	Scan.GlobalCMap [x]
#  else
#    define A_COLORMAP(x)	Scan.PrivateCMap [x]
#  endif
#  define A_DSTTEX	Scan.GlobalCMap [*_dest]
#  define A_NORMSHIFT	5
#  define A_PIXTYPE	uint8
#elif defined (A_R5G5B5)
#  define A_MASK_RB	0x00007c1f
#  define A_MASK_G	0x000003e0
#  if defined (A_MAP)
#    define A_COLORMAP(x)	x
#  else
#    define A_COLORMAP(x)	COLORMAP [x]
#  endif
#  define A_DSTTEX	*_dest
#  define A_NORMSHIFT	5
#  define A_PIXTYPE	uint16
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

#ifdef A_MAP
#  define SCANMAP
#  define A_SRCADDR	((vv >> 16) << shifter) + (uu >> 16)
#else
#  define A_SRCADDR	((uu >> 16) & ander_w) + ((vv >> shifter_h) & ander_h)
#endif

#ifndef A_PI

#define SCANLOOP \
  do									\
  {									\
    unsigned srcaddr = A_SRCADDR;					\
    uint8 alpha = Scan.AlphaMap [srcaddr];				\
    A_IPIXTYPE src = A_COLORMAP (srcTex [srcaddr]);			\
    A_IPIXTYPE dst = A_DSTTEX;						\
    unsigned src_rb = src & A_MASK_RB;					\
    unsigned dst_rb = dst & A_MASK_RB;					\
    dst_rb = dst_rb + (alpha * (src_rb - dst_rb) >> A_NORMSHIFT);	\
    unsigned src_g = src & A_MASK_G;					\
    unsigned dst_g = dst & A_MASK_G;					\
    dst_g = dst_g + (alpha * (src_g - dst_g) >> A_NORMSHIFT);		\
    *_dest++ = INV_COLORMAP ((dst_rb & A_MASK_RB) | (dst_g & A_MASK_G));\
    uu += duu;								\
    vv += dvv;								\
  }									\
  while (_dest <= _destend)
#define SCANFUNC A_SCANFUNC
#include "scanln.inc"

#else // A_PI

void A_SCANFUNC (void *dest, int len, unsigned long* /*zbuff*/,
  long u, long du, long v, long dv, unsigned long /*z*/, long /*dz*/,
  unsigned char *bitmap, int bitmap_log2w)
{
  A_PIXTYPE *_dest = (A_PIXTYPE *)dest;
  A_PIXTYPE *_destend = _dest + len;
  while (_dest < _destend)
  {
    unsigned srcaddr = ((v >> 16) << bitmap_log2w) + (u >> 16);
    uint8 alpha = Scan.AlphaMap [srcaddr];
    A_IPIXTYPE src = A_COLORMAP (bitmap [srcaddr]);
    A_IPIXTYPE dst = A_DSTTEX;
    unsigned src_rb = src & A_MASK_RB;
    unsigned dst_rb = dst & A_MASK_RB;
    dst_rb = dst_rb + (alpha * (src_rb - dst_rb) >> A_NORMSHIFT);
    unsigned src_g = src & A_MASK_G;
    unsigned dst_g = dst & A_MASK_G;
    dst_g = dst_g + (alpha * (src_g - dst_g) >> A_NORMSHIFT);
    *_dest++ = INV_COLORMAP ((dst_rb & A_MASK_RB) | (dst_g & A_MASK_G));
    u += du; v += dv;
  }
}

#endif // A_PI

#undef A_SCANFUNC
#undef A_MAP
#undef A_PI
#undef A_INDEX8
#undef A_R5G5B5
#undef A_R5G6B5
#undef A_R8G8B8
#undef A_MASK_RB
#undef A_MASK_G
#undef A_NORMSHIFT
#undef A_COLORMAP
#undef A_SRCADDR
#undef A_DSTTEX
#undef A_IPIXTYPE
#undef A_PIXTYPE
