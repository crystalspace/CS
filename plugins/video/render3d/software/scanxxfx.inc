/*  -*- Mode: C++; -*- */
/*
    Crystal Space 3D engine
    Copyright (C) 2001 by Jorrit Tyberghein

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
    This include file contains common implementation for some routines that
    are quite the same in all 8-, 16- and 32bpp modes. They define routines
    that use special FX blending modes.
    The following macros are expected to be defined prior to including
    this file:

	SCAN16/SCAN32	The number of bits per pixel (16, 32)
	COLORMAP
	PI_R?G?B?
*/

#if defined (SCAN16)
#  define PIXTYPE uint16
#  if defined (PI_R5G6B5)
#    define SCAN_BPP 16_565
#  else
#    define SCAN_BPP 16_555
#  endif
#elif defined (SCAN32)
#  define PIXTYPE uint32
#  define SCAN_BPP 32
#else
#  error "No valid SCANXX macro defined!"
#endif

#include "pixtype.inc"
#include "blend.inc"

#define SCAN_EXPAND(x,y)	csScan_ ## x ## _ ## y
#define SCAN_NAME(x, y)		SCAN_EXPAND (x, y)

// The following code is taken from scanpi2.inc. Look there for comments.
#define DO_BLEND(texel) \
  csRGBpixel rgbtexel = *(csRGBpixel*)&texel;			  \
  unsigned rs = (rgbtexel.red   << (PI_RB_BLEND * 2 - 8)) & PI_RM_BLEND;  \
  unsigned gs = (rgbtexel.green << (PI_GB_BLEND * 2 - 8)) & PI_GM_BLEND;  \
  unsigned bs = (rgbtexel.blue  << (PI_BB_BLEND * 2 - 8)) & PI_BM_BLEND;  \
  unsigned pixel = *_dest;						  \
  unsigned rd = (pixel & PI_RM) >> PI_RS_DST_BLEND;			  \
  unsigned gd = (pixel & PI_GM) >> PI_GS_DST_BLEND;			  \
  unsigned bd = (pixel & PI_BM) >> PI_BS_DST_BLEND;			  \
  rs = unsigned (Scan.BlendTable [PI_RBT_OFS + rs + rd]);		  \
  gs = unsigned (Scan.BlendTable [PI_GBT_OFS + gs + gd]);		  \
  bs = unsigned (Scan.BlendTable [PI_BBT_OFS + bs + bd]);		  \
  *_dest = (rs << PI_RS) | (gs << PI_GS) | (bs << PI_BS)

//------------------------------------------------------------------

#ifndef NO_scan_tex_fx_znone

#define SCANFUNC SCAN_NAME (SCAN_BPP, scan_tex_fx_znone)
#define SCANLOOP \
  do									\
  {									\
    uint32 texel = srcTex [((uu >> 16) & ander_w) +			\
      ((vv >> shifter_h) & ander_h)];					\
    DO_BLEND(texel);							\
    _dest++;								\
    uu += duu;								\
    vv += dvv;								\
  }									\
  while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_scan_tex_fx_znone

//------------------------------------------------------------------

#ifndef NO_scan_tex_fx_zfil

#define SCANFUNC SCAN_NAME (SCAN_BPP, scan_tex_fx_zfil)
#define SCANLOOP \
  do									\
  {									\
    uint32 texel = srcTex [((uu >> 16) & ander_w) +			\
      ((vv >> shifter_h) & ander_h)];					\
    DO_BLEND(texel);							\
    _dest++;								\
    uu += duu;								\
    vv += dvv;								\
  }									\
  while (_dest <= _destend)
#define SCANEND \
  do									\
  {									\
    *z_buffer++ = izz;							\
    izz += dzz;								\
  }									\
  while (z_buffer <= lastZbuf)
#include "scanln.inc"

#endif // NO_scan_tex_fx_zfil

//------------------------------------------------------------------

#ifndef NO_scan_tex_fx_zuse

#define SCANFUNC SCAN_NAME (SCAN_BPP, scan_tex_fx_zuse)
#define SCANLOOP \
  do									\
  {									\
    if (izz >= *z_buffer)						\
    {									\
      *z_buffer = izz;							\
      uint32 texel = srcTex [((uu >> 16) & ander_w) +			\
        ((vv >> shifter_h) & ander_h)];					\
      DO_BLEND(texel);							\
    }									\
    _dest++;								\
    z_buffer++;								\
    uu += duu;								\
    vv += dvv;								\
    izz += dzz;								\
  }									\
  while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_scan_tex_fx_zuse

//------------------------------------------------------------------

#ifndef NO_scan_tex_fx_ztest

#define SCANFUNC SCAN_NAME (SCAN_BPP, scan_tex_fx_ztest)
#define SCANLOOP \
  do									\
  {									\
    if (izz >= *z_buffer)						\
    {									\
      uint32 texel = srcTex [((uu >> 16) & ander_w) +			\
        ((vv >> shifter_h) & ander_h)];					\
      DO_BLEND(texel);							\
    }									\
    _dest++;								\
    z_buffer++;								\
    uu += duu;								\
    vv += dvv;								\
    izz += dzz;								\
  }									\
  while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_scan_tex_fx_ztest

//------------------------------------------------------------------

#ifndef NO_scan_map_fx_znone

#define SCANFUNC SCAN_NAME (SCAN_BPP, scan_map_fx_znone)
#define SCANMAP
#define SCANLOOP \
  do									\
  {									\
    PIXTYPE pix = srcTex [((vv >> 16) << shifter) + (uu >> 16)];	\
    uint8 texel = pix;							\
    DO_BLEND(texel);							\
    _dest++;								\
    uu += duu;								\
    vv += dvv;								\
  }									\
  while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_scan_map_fx_znone

//------------------------------------------------------------------

#ifndef NO_scan_map_fx_zfil

#define SCANFUNC SCAN_NAME (SCAN_BPP, scan_map_fx_zfil)
#define SCANMAP
#define SCANLOOP \
  do									\
  {									\
    PIXTYPE pix = srcTex [((vv >> 16) << shifter) + (uu >> 16)];	\
    uint8 texel = pix;							\
    DO_BLEND(texel);							\
    _dest++;								\
    uu += duu;								\
    vv += dvv;								\
  }									\
  while (_dest <= _destend)
#define SCANEND \
  do									\
  {									\
    *z_buffer++ = izz;							\
    izz += dzz;								\
  }									\
  while (z_buffer <= lastZbuf)
#include "scanln.inc"

#endif // NO_scan_map_fx_zfil

//------------------------------------------------------------------

#ifndef NO_scan_map_fx_zuse

#define SCANFUNC SCAN_NAME (SCAN_BPP, scan_map_fx_zuse)
#define SCANMAP
#define SCANLOOP \
  do									\
  {									\
    if (izz >= *z_buffer)						\
    {									\
      *z_buffer = izz;							\
      PIXTYPE pix = srcTex [((vv >> 16) << shifter) + (uu >> 16)];	\
      uint8 texel = pix;						\
      DO_BLEND(texel);							\
    }									\
    _dest++;								\
    z_buffer++;								\
    uu += duu;								\
    vv += dvv;								\
    izz += dzz;								\
  }									\
  while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_scan_map_fx_zuse

//------------------------------------------------------------------

#ifndef NO_scan_map_fx_ztest

#define SCANFUNC SCAN_NAME (SCAN_BPP, scan_map_fx_ztest)
#define SCANMAP
#define SCANLOOP \
  do									\
  {									\
    if (izz >= *z_buffer)						\
    {									\
      PIXTYPE pix = srcTex [((vv >> 16) << shifter) + (uu >> 16)];	\
      uint8 texel = pix;						\
      DO_BLEND(texel);							\
    }									\
    _dest++;								\
    z_buffer++;								\
    uu += duu;								\
    vv += dvv;								\
    izz += dzz;								\
  }									\
  while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_scan_map_fx_ztest

#undef PIXTYPE
#undef SCAN_BPP

#include "unblend.inc"

