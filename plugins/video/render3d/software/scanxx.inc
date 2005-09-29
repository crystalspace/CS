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
    This include file contains common implementation for some routines that
    are quite the same in all 8-, 16- and 32bpp modes. The following macros
    are expected to be defined prior to including this file:

	SCAN16/SCAN32	The number of bits per pixel (16, 32)
	COLORMAP
*/

#if defined (SCAN16)
#  define PIXTYPE uint16
#  define SCAN_BPP 16
#elif defined (SCAN32)
#  define PIXTYPE uint32
#  define SCAN_BPP 32
#else
#  error "No valid SCANXX macro defined!"
#endif

#define SCAN_EXPAND(x,y)	csScan_ ## x ## _ ## y
#define SCAN_NAME(x, y)		SCAN_EXPAND (x, y)

#ifndef NO_scan_flat_zfil

void SCAN_NAME (SCAN_BPP, scan_flat_zfil) (int xx, unsigned char* d,
  uint32* z_buf, float inv_z, float u_div_z, float v_div_z)
{
  (void)u_div_z; (void)v_div_z;
  int color = Scan.FlatColor;
  int32 izz = csQfixed24 (inv_z);
  int32 dzz = csQfixed24 (Scan.M);
  PIXTYPE *_dest = (PIXTYPE *)d;
  PIXTYPE *_destend = _dest + xx - 1;
  do
  {
    *_dest++ = color;
    *z_buf++ = izz;
    izz += dzz;
  }
  while (_dest <= _destend);
}

#endif // NO_scan_flat_zfil

//------------------------------------------------------------------

#ifndef NO_scan_flat_znone

void SCAN_NAME (SCAN_BPP, scan_flat_znone) (int xx, unsigned char* d,
  uint32* z_buf, float inv_z, float u_div_z, float v_div_z)
{
  (void)u_div_z; (void)v_div_z; (void)inv_z; (void)z_buf;
  int color = Scan.FlatColor;
  PIXTYPE *_dest = (PIXTYPE *)d;
  PIXTYPE *_destend = _dest + xx - 1;
  do
  {
    *_dest++ = color;
  }
  while (_dest <= _destend);
}

#endif // NO_scan_flat_znone

//------------------------------------------------------------------

#ifndef NO_scan_flat_zuse

void SCAN_NAME (SCAN_BPP, scan_flat_zuse) (int xx, unsigned char* d,
  uint32* z_buf, float inv_z, float u_div_z, float v_div_z)
{
  (void)u_div_z; (void)v_div_z;
  int color = Scan.FlatColor;
  uint32 izz = csQfixed24 (inv_z);
  uint32 dzz = csQfixed24 (Scan.M);
  PIXTYPE *_dest = (PIXTYPE *)d;
  PIXTYPE *_destend = _dest + xx - 1;
  do
  {
    if (izz >= *z_buf)
    {
      *z_buf = izz;
      *_dest = color;
    }
    _dest++;
    z_buf++;
    izz += dzz;
  }
  while (_dest <= _destend);
}

#endif // NO_scan_flat_zuse

//------------------------------------------------------------------

#ifndef NO_scan_flat_ztest

void SCAN_NAME (SCAN_BPP, scan_flat_ztest) (int xx, unsigned char* d,
  uint32* z_buf, float inv_z, float u_div_z, float v_div_z)
{
  (void)u_div_z; (void)v_div_z;
  int color = Scan.FlatColor;
  uint32 izz = csQfixed24 (inv_z);
  uint32 dzz = csQfixed24 (Scan.M);
  PIXTYPE *_dest = (PIXTYPE *)d;
  PIXTYPE *_destend = _dest + xx - 1;
  do
  {
    if (izz >= *z_buf)
      *_dest = color;
    _dest++;
    z_buf++;
    izz += dzz;
  }
  while (_dest <= _destend);
}

#endif // NO_scan_flat_ztest

//------------------------------------------------------------------

#ifndef NO_scan_tex_znone

#define SCANFUNC SCAN_NAME (SCAN_BPP, scan_tex_znone)
#define SCANLOOP \
  do									\
  {									\
    *_dest++ = srcTex [((uu >> 16) & ander_w) +				\
      ((vv >> shifter_h) & ander_h)];					\
    uu += duu;								\
    vv += dvv;								\
  }									\
  while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_scan_tex_znone

//------------------------------------------------------------------

#ifndef NO_scan_tex_zfil

#define SCANFUNC SCAN_NAME (SCAN_BPP, scan_tex_zfil)
#define SCANLOOP \
  do									\
  {									\
    *_dest++ = srcTex [((uu >> 16) & ander_w) +				\
      ((vv >> shifter_h) & ander_h)];					\
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

#endif // NO_scan_tex_zfil

//------------------------------------------------------------------

#ifndef NO_scan_tex_zuse

#define SCANFUNC SCAN_NAME (SCAN_BPP, scan_tex_zuse)
#define SCANLOOP \
  do									\
  {									\
    if (izz >= *z_buffer)						\
    {									\
      *z_buffer = izz;							\
      *_dest = srcTex [((uu >> 16) & ander_w) +				\
        ((vv >> shifter_h) & ander_h)];					\
    }									\
    _dest++;								\
    z_buffer++;								\
    uu += duu;								\
    vv += dvv;								\
    izz += dzz;								\
  }									\
  while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_scan_tex_zuse

//------------------------------------------------------------------

#ifndef NO_scan_tex_ztest

#define SCANFUNC SCAN_NAME (SCAN_BPP, scan_tex_ztest)
#define SCANLOOP \
  do									\
  {									\
    if (izz >= *z_buffer)						\
    {									\
      *_dest = srcTex [((uu >> 16) & ander_w) +				\
        ((vv >> shifter_h) & ander_h)];					\
    }									\
    _dest++;								\
    z_buffer++;								\
    uu += duu;								\
    vv += dvv;								\
    izz += dzz;								\
  }									\
  while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_scan_tex_ztest

//------------------------------------------------------------------

#ifndef NO_scan_map_znone

#define SCANFUNC SCAN_NAME (SCAN_BPP, scan_map_znone)
#define SCANMAP
#define SCANLOOP \
  do									\
  {									\
    *_dest++ = srcTex [((vv >> 16) << shifter) + (uu >> 16)];		\
    uu += duu;								\
    vv += dvv;								\
  }									\
  while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_scan_map_znone

//------------------------------------------------------------------

#ifndef NO_scan_map_zfil

#define SCANFUNC SCAN_NAME (SCAN_BPP, scan_map_zfil)
#define SCANMAP
#define SCANLOOP \
  do									\
  {									\
    *_dest++ = srcTex [((vv >> 16) << shifter) + (uu >> 16)];		\
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

#endif // NO_scan_map_zfil

//------------------------------------------------------------------

#ifndef NO_scan_map_zuse

#define SCANFUNC SCAN_NAME (SCAN_BPP, scan_map_zuse)
#define SCANMAP
#define SCANLOOP \
  do									\
  {									\
    if (izz >= *z_buffer)						\
    {									\
      *z_buffer = izz;							\
      *_dest = srcTex [((vv >> 16) << shifter) + (uu >> 16)];		\
    }									\
    _dest++;								\
    z_buffer++;								\
    uu += duu;								\
    vv += dvv;								\
    izz += dzz;								\
  }									\
  while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_scan_map_zuse

//------------------------------------------------------------------

#ifndef NO_scan_map_ztest

#define SCANFUNC SCAN_NAME (SCAN_BPP, scan_map_ztest)
#define SCANMAP
#define SCANLOOP \
  do									\
  {									\
    if (izz >= *z_buffer)						\
    {									\
      *_dest = srcTex [((vv >> 16) << shifter) + (uu >> 16)];		\
    }									\
    _dest++;								\
    z_buffer++;								\
    uu += duu;								\
    vv += dvv;								\
    izz += dzz;								\
  }									\
  while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_scan_map_ztest

//------------------------------------------------------------------

#ifndef NO_scan_tex_key_znone

#define SCANFUNC SCAN_NAME (SCAN_BPP, scan_tex_key_znone)
#define SCANLOOP \
  do									\
  {									\
    uint32 c = srcTex [((uu >> 16) & ander_w) +				\
      ((vv >> shifter_h) & ander_h)];					\
    if (c)								\
    {									\
      *_dest = c;							\
    }									\
    _dest++;								\
    uu += duu;								\
    vv += dvv;								\
  }									\
  while (_dest <= _destend)
#include "scanln.inc"

#endif // scan_tex_key_znone

//------------------------------------------------------------------


#ifndef NO_scan_tex_key_zfil

#define SCANFUNC SCAN_NAME (SCAN_BPP, scan_tex_key_zfil)
#define SCANLOOP \
  do									\
  {									\
    uint32 c = srcTex [((uu >> 16) & ander_w) +				\
      ((vv >> shifter_h) & ander_h)];					\
    if (c)								\
    {									\
      *_dest = c;							\
      *z_buffer = izz;							\
    }									\
    _dest++;								\
    z_buffer++;								\
    uu += duu;								\
    vv += dvv;								\
    izz += dzz;								\
  }									\
  while (_dest <= _destend)
#include "scanln.inc"

#endif // scan_tex_key_zfil

//------------------------------------------------------------------

#ifndef NO_scan_tex_key_zuse

#define SCANFUNC SCAN_NAME (SCAN_BPP, scan_tex_key_zuse)
#define SCANLOOP \
  do									\
  {									\
    if (izz >= *z_buffer)						\
    {									\
      uint32 c = srcTex [((uu >> 16) & ander_w) +			\
        ((vv >> shifter_h) & ander_h)];					\
      if (!c) goto transp_pixel;					\
      *z_buffer = izz;							\
      *_dest = c;							\
    }									\
transp_pixel:								\
    z_buffer++;								\
    _dest++;								\
    uu += duu;								\
    vv += dvv;								\
    izz += dzz;								\
  }									\
  while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_scan_tex_key_zuse

//------------------------------------------------------------------

#ifndef NO_scan_tex_key_ztest

#define SCANFUNC SCAN_NAME (SCAN_BPP, scan_tex_key_ztest)
#define SCANLOOP \
  do									\
  {									\
    if (izz >= *z_buffer)						\
    {									\
      uint32 c = srcTex [((uu >> 16) & ander_w) +			\
        ((vv >> shifter_h) & ander_h)];					\
      if (!c) goto transp_pixel;					\
      *_dest = c;							\
    }									\
transp_pixel:								\
    z_buffer++;								\
    _dest++;								\
    uu += duu;								\
    vv += dvv;								\
    izz += dzz;								\
  }									\
  while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_scan_tex_key_ztest

//------------------------------------------------------------------

#ifndef NO_scan_map_key_znone

#define SCANFUNC SCAN_NAME (SCAN_BPP, scan_map_key_znone)
#define SCANMAP
#define SCANLOOP \
  do									\
  {									\
    PIXTYPE c = srcTex [((vv >> 16) << shifter) + (uu >> 16)];		\
    if (c)								\
      *_dest = c;							\
    _dest++;								\
    uu += duu;								\
    vv += dvv;								\
  }									\
  while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_scan_map_key_znone

//------------------------------------------------------------------

#ifndef NO_scan_map_key_zfil

#define SCANFUNC SCAN_NAME (SCAN_BPP, scan_map_key_zfil)
#define SCANMAP
#define SCANLOOP \
  do									\
  {									\
    PIXTYPE c = srcTex [((vv >> 16) << shifter) + (uu >> 16)];		\
    if (c)								\
    {									\
      *z_buffer = izz;							\
      *_dest = c;							\
    }									\
    _dest++;								\
    z_buffer++;								\
    uu += duu;								\
    vv += dvv;								\
    izz += dzz;								\
  }									\
  while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_scan_map_key_zfil

//------------------------------------------------------------------

#ifndef NO_scan_map_key_zuse

#define SCANFUNC SCAN_NAME (SCAN_BPP, scan_map_key_zuse)
#define SCANMAP
#define SCANLOOP \
  do									\
  {									\
    if (izz >= *z_buffer)						\
    {									\
      PIXTYPE c = srcTex [((vv >> 16) << shifter) + (uu >> 16)];		\
      if (!c) goto transp_pixel;					\
      *z_buffer = izz;							\
      *_dest = c;							\
    }									\
transp_pixel:								\
    _dest++;								\
    z_buffer++;								\
    uu += duu;								\
    vv += dvv;								\
    izz += dzz;								\
  }									\
  while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_scan_map_key_zuse

//------------------------------------------------------------------

#ifndef NO_scan_map_key_ztest

#define SCANFUNC SCAN_NAME (SCAN_BPP, scan_map_key_ztest)
#define SCANMAP
#define SCANLOOP \
  do									\
  {									\
    if (izz >= *z_buffer)						\
    {									\
      PIXTYPE c = srcTex [((vv >> 16) << shifter) + (uu >> 16)];		\
      if (!c) goto transp_pixel;					\
      *_dest = c;							\
    }									\
transp_pixel:								\
    _dest++;								\
    z_buffer++;								\
    uu += duu;								\
    vv += dvv;								\
    izz += dzz;								\
  }									\
  while (_dest <= _destend)
#include "scanln.inc"

#undef PIXTYPE
#undef SCAN_BPP

#endif // NO_scan_map_key_ztest

