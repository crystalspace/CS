/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#include "sysdef.h"
#include "qint.h"
#include "scan.h"
#include "tables.h"
#include "ipolygon.h"
#include "ilghtmap.h"

#include "sttest.h"

//------------------------------------------------------------------

#ifndef NO_draw_scanline_flat_zfil

void csScan_32_draw_scanline_flat_zfil (int xx, unsigned char* d,
  unsigned long* zbuf, float inv_z, float u_div_z, float v_div_z)
{
  (void)u_div_z; (void)v_div_z;
  int color = Scan.FlatColor;
  long izz = QInt24 (inv_z);
  long dzz = QInt24 (Scan.M);
  ULong* _dest = (ULong*)d;
  ULong* _destend = _dest + xx-1;
  do
  {
    *_dest++ = color;
    *zbuf++ = izz;
    izz += dzz;
  }
  while (_dest <= _destend);
}

#endif // NO_draw_scanline_flat_zfil

//------------------------------------------------------------------

#ifndef NO_draw_scanline_flat_zuse

void csScan_32_draw_scanline_flat_zuse (int xx, unsigned char* d,
  unsigned long* zbuf, float inv_z, float u_div_z, float v_div_z)
{
  (void)u_div_z; (void)v_div_z;
  int color = Scan.FlatColor;
  long izz = QInt24 (inv_z);
  long dzz = QInt24 (Scan.M);
  ULong* _dest = (ULong*)d;
  ULong* _destend = _dest + xx-1;
  do
  {
    if (izz >= (int)(*zbuf))
    {
      *_dest++ = color;
      *zbuf++ = izz;
    }
    else
    {
      _dest++;
      zbuf++;
    }
    izz += dzz;
  }
  while (_dest <= _destend);
}

#endif // NO_draw_scanline_flat_zuse

//------------------------------------------------------------------

#ifndef NO_draw_scanline_tex_zfil

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC csScan_32_draw_scanline_tex_zfil
#define SCANLOOP \
    do									\
    {									\
      *_dest++ = srcTex[((uu>>16)&ander_w) + ((vv>>shifter_h)&ander_h)];\
      uu += duu;							\
      vv += dvv;							\
    }									\
    while (_dest <= _destend)
#define SCANEND \
    do									\
    {									\
      *z_buffer++ = izz;						\
      izz += dzz;							\
    }									\
    while (z_buffer <= lastZbuf)
#include "scanln32.inc"

#endif // NO_draw_scanline_tex_zfil

//------------------------------------------------------------------

#ifndef NO_draw_scanline_tex_zuse

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC csScan_32_draw_scanline_tex_zuse
#define SCANLOOP \
    do									\
    {									\
      if (izz >= (int)(*z_buffer))					\
      {									\
        *_dest++ = srcTex[((uu>>16)&ander_w) + ((vv>>shifter_h)&ander_h)];\
	*z_buffer++ = izz;						\
      }									\
      else								\
      {									\
        _dest++;							\
        z_buffer++;							\
      }									\
      uu += duu;							\
      vv += dvv;							\
      izz += dzz;							\
    }									\
    while (_dest <= _destend)
#include "scanln32.inc"

#endif // NO_draw_scanline_tex_zuse

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_zfil

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC csScan_32_draw_scanline_map_zfil
#define SCANMAP 1
#define SCANLOOP \
    do									\
    {									\
      *_dest++ = srcTex[((vv>>16)<<shifter) + (uu>>16)];		\
      uu += duu;							\
      vv += dvv;							\
    }									\
    while (_dest <= _destend)
#define SCANEND \
    do									\
    {									\
      *z_buffer++ = izz;						\
      izz += dzz;							\
    }									\
    while (z_buffer <= lastZbuf)
#include "scanln32.inc"

#endif // NO_draw_scanline_map_zfil

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_zuse

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC csScan_32_draw_scanline_map_zuse
#define SCANMAP 1
#define SCANLOOP \
    do									\
    {									\
      if (izz >= (int)(*z_buffer))					\
      {									\
	*_dest++ = srcTex[((vv>>16)<<shifter) + (uu>>16)];		\
	*z_buffer++ = izz;						\
      }									\
      else								\
      {									\
        _dest++;							\
        z_buffer++;							\
      }									\
      uu += duu;							\
      vv += dvv;							\
      izz += dzz;							\
    }									\
    while (_dest <= _destend)
#include "scanln32.inc"

#endif // NO_draw_scanline_map_zuse

//------------------------------------------------------------------

#ifndef NO_draw_scanline_fog

void csScan_32_draw_scanline_fog (int xx, unsigned char* d,
  unsigned long* zbuf, float inv_z, float u_div_z, float v_div_z)
{
  if (xx <= 0) return;
  (void)u_div_z; (void)v_div_z;
  ULong* _dest = (ULong*)d;
  ULong* _destend = _dest + xx-1;
  unsigned long izz = QInt24 (inv_z);
  int dzz = QInt24 (Scan.M);

  ULong fog_dens = Scan.FogDensity;
  int fog_r = 256+Scan.FogR;
  int fog_g = 256+Scan.FogG;
  int fog_b = 256+Scan.FogB;

  do
  {
    if (izz < *zbuf)
    {
      _dest++;
      izz+=dzz;
      zbuf++;
    }
    else
    {
      int dens_dist = tables.exp_table_2[fog_dens / *(zbuf) - fog_dens / izz];
      int r = (*_dest) >> 16;
      int g = ((*_dest) >> 8) & 0xff;
      int b = (*_dest) & 0xff;

      r += tables.mul_table[dens_dist+fog_r - r];
      g += tables.mul_table[dens_dist+fog_g - g];
      b += tables.mul_table[dens_dist+fog_b - b];

      *_dest++ = (r<<16) | (g<<8) | b;
      zbuf++;
      izz += dzz;
    }
  }
  while (_dest <= _destend);
}

#endif // NO_draw_scanline_fog

//------------------------------------------------------------------

#ifndef NO_draw_scanline_fog_view

void csScan_32_draw_scanline_fog_view (int xx, unsigned char* d,
  unsigned long* zbuf, float inv_z, float u_div_z, float v_div_z)
{
  if (xx <= 0) return;
  (void)u_div_z; (void)v_div_z; (void)inv_z;
  ULong* _dest = (ULong*)d;
  ULong* _destend = _dest + xx-1;

  ULong fog_dens = Scan.FogDensity;
  int fog_r = 256+Scan.FogR;
  int fog_g = 256+Scan.FogG;
  int fog_b = 256+Scan.FogB;

  do
  {
    int dens_dist = tables.exp_table_2[fog_dens / *(zbuf)];
    int r = (*_dest) >> 16;
    int g = ((*_dest) >> 8) & 0xff;
    int b = (*_dest) & 0xff;

    r += tables.mul_table[dens_dist+fog_r - r];
    g += tables.mul_table[dens_dist+fog_g - g];
    b += tables.mul_table[dens_dist+fog_b - b];

    *_dest++ = (r<<16) | (g<<8) | b;
    zbuf++;
  }
  while (_dest <= _destend);
}

#endif // NO_draw_scanline_fog_view

//------------------------------------------------------------------

#ifndef NO_draw_scanline_fog_plane

void csScan_32_draw_scanline_fog_plane (int xx, unsigned char* d,
  unsigned long* zbuf, float inv_z, float u_div_z, float v_div_z)
{
  if (xx <= 0) return;
  (void)u_div_z; (void)v_div_z;
  UShort* _dest = (UShort*)d;
  UShort* _destend = _dest + xx-1;
  int izz;
  izz = QInt24 (inv_z);

  ULong fog_dens = Scan.FogDensity;
  int fog_r = 256+Scan.FogR;
  int fog_g = 256+Scan.FogG;
  int fog_b = 256+Scan.FogB;

  long dist = QInt24 (1. / 1.);
  int dens_dist = tables.exp_table_2[fog_dens / dist];
//printf ("fog_dens=%d dist=%d dens_dist=%d\n", fog_dens, dist, dens_dist);

  do
  {
    int r = (*_dest) >> 16;
    int g = ((*_dest) >> 8) & 0xff;
    int b = (*_dest) & 0xff;

    r += tables.mul_table[dens_dist+fog_r - r];
    g += tables.mul_table[dens_dist+fog_g - g];
    b += tables.mul_table[dens_dist+fog_b - b];

    *_dest++ = (r<<16) | (g<<8) | b;
    zbuf++;
  }
  while (_dest <= _destend);
}

#endif // NO_draw_scanline_fog_plane

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_tex_zfil

#define PI_SCANFUNC csScan_32_draw_pi_scanline_tex_zfil
#define ZFILL
#define BPP32
#include "scanpi.inc"

/*
void csScan_32_draw_pi_scanline_tex_zfil (void *dest, int len,
  unsigned long *zbuff, long u, long du, long v, long dv,
  unsigned long z, long dz, unsigned char *bitmap, int bitmap_log2w)
{
  ULong *_dest = (ULong *)dest;
  ULong *_destend = _dest + len;
  ULong *bm = (ULong*)bitmap;
  ULong tex;
  while (_dest < _destend)
  {
    tex = *(bm + ((v >> 16) << bitmap_log2w) + (u >> 16));
    *_dest++ = tex;
    *zbuff++ = z;
    u += du; v += dv; z += dz;
  } 
}
*/

#endif // NO_draw_pi_scanline_tex_zfil

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_tex_zuse

#define PI_SCANFUNC csScan_32_draw_pi_scanline_tex_zuse
#define ZUSE
#define BPP32
#include "scanpi.inc"

/*
void csScan_32_draw_pi_scanline_tex_zuse (void *dest, int len,
  unsigned long *zbuff, long u, long du, long v, long dv,
  unsigned long z, long dz, unsigned char *bitmap, int bitmap_log2w)
{
  ULong *_dest = (ULong *)dest;
  ULong *_destend = _dest + len;
  ULong * bm = (ULong*)bitmap;
  ULong tex;
  while (_dest < _destend)
  {
    if (z >= *zbuff)
    {
      tex = *(bm + ((v >> 16) << bitmap_log2w) + (u >> 16));
      *_dest = tex;
      *zbuff = z;
    }
    _dest++;
    zbuff++;
    u += du; v += dv; z += dz;
  } 
}
*/

#endif // NO_draw_pi_scanline_tex_zuse

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_tex_gouraud_zfil

#define PI_SCANFUNC csScan_32_draw_pi_scanline_tex_gouraud_zfil
#define ZFILL
#define GOURAUD
#define BPP32
#include "scanpi.inc"

/*
void csScan_32_draw_pi_scanline_tex_gouraud_zfil (void *dest, int len,
  unsigned long *zbuff, long u, long du, long v, long dv,
  unsigned long z, long dz, unsigned char *bitmap, int bitmap_log2w,
  long r, long g, long b, long dr, long dg, long db)
{
  ULong *_dest = (ULong *)dest;
  ULong *_destend = _dest + len;
  ULong *bm = (ULong*)bitmap;
  ULong tex;
  ULong r1, g1, b1;
  while (_dest < _destend)
  {
    tex = *(bm + ((v >> 16) << bitmap_log2w) + (u >> 16));
    r1 = tex >> 16;
    g1 = (tex >> 8) & 0xff;
    b1 = tex & 0xff;
    r1 = (r1*(ULong)r) >> (16+8);
    g1 = (g1*(ULong)g) >> (16+8);
    b1 = (b1*(ULong)b) >> (16+8);
    *_dest++ = (r1<<16) | (g1<<8) | b1;
    *zbuff++ = z;
    u += du; v += dv; z += dz;
    r += dr; g += dg; b += db;
  } 
}
*/

#endif // NO_draw_pi_scanline_tex_gouraud_zfil

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_tex_gouraud_zuse

#define PI_SCANFUNC csScan_32_draw_pi_scanline_tex_gouraud_zuse
#define ZUSE
#define GOURAUD
#define BPP32
#include "scanpi.inc"

/*
void csScan_32_draw_pi_scanline_tex_gouraud_zuse (void *dest, int len,
  unsigned long *zbuff, long u, long du, long v, long dv,
  unsigned long z, long dz, unsigned char *bitmap, int bitmap_log2w,
  long r, long g, long b, long dr, long dg, long db)
{
  ULong *_dest = (ULong *)dest;
  ULong *_destend = _dest + len;
  ULong * bm = (ULong*)bitmap;
  ULong tex;
  ULong r1, g1, b1;
  while (_dest < _destend)
  {
    if (z >= *zbuff)
    {
      tex = *(bm + ((v >> 16) << bitmap_log2w) + (u >> 16));
      r1 = tex >> 16;
      g1 = (tex >> 8) & 0xff;
      b1 = tex & 0xff;
      r1 = (r1*(ULong)r) >> (16+8);
      g1 = (g1*(ULong)g) >> (16+8);
      b1 = (b1*(ULong)b) >> (16+8);
      *_dest = (r1<<16) | (g1<<8) | b1;
      *zbuff = z;
    }
    _dest++;
    zbuff++;
    u += du; v += dv; z += dz;
    r += dr; g += dg; b += db;
  } 
}
*/

#endif // NO_draw_pi_scanline_tex_gouraud_zuse

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_flat_gouraud_zfil

#define PI_SCANFUNC csScan_32_draw_pi_scanline_flat_gouraud_zfil
#define FLAT
#define ZFILL
#define GOURAUD
#define BPP32
#include "scanpi.inc"

/*
void csScan_32_draw_pi_scanline_flat_gouraud_zfil (void *dest, int len,
  unsigned long *zbuff, long u, long du, long v, long dv,
  unsigned long z, long dz, unsigned char *bitmap, int bitmap_log2w,
  long r, long g, long b, long dr, long dg, long db)
{
  (void)u; (void)v; (void)du; (void)dv; (void)bitmap; (void)bitmap_log2w;
  ULong *_dest = (ULong *)dest;
  ULong *_destend = _dest + len;
  ULong r1, g1, b1;
  while (_dest < _destend)
  {
    r1 = ((ULong)r) >> 16;
    g1 = ((ULong)g) >> 16;
    b1 = ((ULong)b) >> 16;
    *_dest++ = (r1<<16) | (g1<<8) | b1;
    *zbuff++ = z;
    z += dz;
    r += dr; g += dg; b += db;
  } 
}
*/

#endif // NO_draw_pi_scanline_flat_gouraud_zfil

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_flat_gouraud_zuse

#define PI_SCANFUNC csScan_32_draw_pi_scanline_flat_gouraud_zuse
#define FLAT
#define ZUSE
#define GOURAUD
#define BPP32
#include "scanpi.inc"

/*
void csScan_32_draw_pi_scanline_flat_gouraud_zuse (void *dest, int len,
  unsigned long *zbuff, long u, long du, long v, long dv,
  unsigned long z, long dz, unsigned char *bitmap, int bitmap_log2w,
  long r, long g, long b, long dr, long dg, long db)
{
  (void)u; (void)v; (void)du; (void)dv; (void)bitmap; (void)bitmap_log2w;
  ULong *_dest = (ULong *)dest;
  ULong *_destend = _dest + len;
  ULong r1, g1, b1;
  while (_dest < _destend)
  {
    if (z >= *zbuff)
    {
      r1 = ((ULong)r) >> 16;
      g1 = ((ULong)g) >> 16;
      b1 = ((ULong)b) >> 16;
      *_dest = (r1<<16) | (g1<<8) | b1;
      *zbuff = z;
    }
    _dest++;
    zbuff++;
    z += dz;
    r += dr; g += dg; b += db;
  } 
}
*/

#endif // NO_draw_pi_scanline_flat_gouraud_zuse

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_flat_zfil

#define PI_SCANFUNC csScan_32_draw_pi_scanline_flat_zfil
#define FLAT
#define ZFILL
#define BPP32
#include "scanpi.inc"

/*
void csScan_32_draw_pi_scanline_flat_zfil (void *dest, int len,
  unsigned long *zbuff, long u, long du, long v, long dv,
  unsigned long z, long dz, unsigned char *bitmap, int bitmap_log2w)
{
  (void)u; (void)v; (void)du; (void)dv; (void)bitmap; (void)bitmap_log2w;
  ULong *_dest = (ULong *)dest;
  ULong *_destend = _dest + len;
  while (_dest < _destend)
  {
    *_dest++ = 0xffffff;
    *zbuff++ = z;
    z += dz;
  } 
}
*/

#endif // NO_draw_pi_scanline_flat_zfil

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_flat_zuse

#define PI_SCANFUNC csScan_32_draw_pi_scanline_flat_zuse
#define FLAT
#define ZUSE
#define BPP32
#include "scanpi.inc"

/*
void csScan_32_draw_pi_scanline_flat_zuse (void *dest, int len,
  unsigned long *zbuff, long u, long du, long v, long dv,
  unsigned long z, long dz, unsigned char *bitmap, int bitmap_log2w)
{
  (void)u; (void)v; (void)du; (void)dv; (void)bitmap; (void)bitmap_log2w;
  ULong *_dest = (ULong *)dest;
  ULong *_destend = _dest + len;
  while (_dest < _destend)
  {
    if (z >= *zbuff)
    {
      *_dest = 0xffffff;
      *zbuff = z;
    }
    _dest++;
    zbuff++;
    z += dz;
  } 
}
*/

#endif // NO_draw_pi_scanline_flat_zuse

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_alpha50

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC csScan_32_draw_scanline_map_alpha50
#define SCANMAP 1
#define SCANLOOP \
    do									\
    {									\
      ULong a = *_dest;							\
      ULong b = srcTex[((vv>>16)<<shifter) + (uu>>16)];			\
      *_dest++ = ((a & 0xfefefe) >> 1) + ((b & 0xfefefe) >> 1);		\
      uu += duu;							\
      vv += dvv;							\
    }									\
    while (_dest <= _destend)
#include "scanln32.inc"

#endif // NO_draw_scanline_map_alpha50

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_alpha

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC csScan_32_draw_scanline_map_alpha
#define SCANMAP 1
#define SCANLOOP \
    do									\
    {									\
      ULong a = *_dest;							\
      ULong b = srcTex[((vv>>16)<<shifter) + (uu>>16)];			\
      int r1 = a >> 16;							\
      int g1 = (a >> 8) & 0xff;						\
      int b1 = a & 0xff;						\
      int r2 = b >> 16;							\
      int g2 = (b >> 8) & 0xff;						\
      int b2 = b & 0xff;						\
      r1 = ((256-Scan.AlphaFact)*r1 + r2*Scan.AlphaFact) >> 8;		\
      g1 = ((256-Scan.AlphaFact)*g1 + g2*Scan.AlphaFact) >> 8;		\
      b1 = ((256-Scan.AlphaFact)*b1 + b2*Scan.AlphaFact) >> 8;		\
      *_dest++ = (r1<<16) | (g1<<8) | b1;				\
      uu += duu;							\
      vv += dvv;							\
    }									\
    while (_dest <= _destend)
#include "scanln32.inc"

#endif // NO_draw_scanline_map_alpha

//------------------------------------------------------------------
