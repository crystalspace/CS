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
#include "cs3d/software/scan.h"
#include "cs3d/software/scan16.h"
#include "cs3d/software/tables.h"
#include "ipolygon.h"
#include "ilghtmap.h"

#include "sttest.h"

extern unsigned char* priv_to_global;
UShort* Scan16::pal_table;

#if !defined(NO_ASSEMBLER)

#  if defined(PROC_INTEL)
#    if defined(COMP_GCC)
#      include "cs3d/software/i386/scanln16.h"
#    elif defined(COMP_VC)
#      include "cs3d/software/i386/scanln16vc.h"
#    endif //COMP_???
#  endif //PROC_INTEL

#endif //!NO_ASSEMBLER

#ifndef NO_draw_scanline_map

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC draw_scanline_map
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
#include "scanln16.inc"

#endif // NO_draw_scanline_map

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_filter

extern int filter_bf;

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC draw_scanline_map_filter
#define SCANMAP 1
#define SCANLOOP \
    int filter_du, filter_dv;						\
      while(_dest<=_destend&&((vv<BAILOUT_CONSTANT||uu<BAILOUT_CONSTANT)||(vv>=Scan::th2fp-BAILOUT_CONSTANT||uu>=Scan::tw2fp-BAILOUT_CONSTANT)))\
      {                                                                   \
        *_dest++ = srcTex[((vv>>16)<<shifter) + (uu>>16)];                \
        uu += duu;							\
        vv += dvv;							\
      }                                                                   \
      while ((_dest <= _destend)&&!((vv<BAILOUT_CONSTANT||uu<BAILOUT_CONSTANT)||(vv>=Scan::th2fp-BAILOUT_CONSTANT||uu>=Scan::tw2fp-BAILOUT_CONSTANT)))\
      {									\
        if ((((long)_dest) & filter_bf) != 0)				\
        {									\
          if ((uu&0xffff) < 64*256) filter_du = -1;			\
          else if ((uu&0xffff) > 192*256) filter_du = 1;			\
          else filter_du = 0;						\
          if ((vv&0xffff) < 64*256) filter_dv = -1;			\
          else if ((vv&0xffff) > 192*256) filter_dv = 1;			\
          else filter_dv = 0;						\
        }									\
        else filter_du = filter_dv = 0;					\
        *_dest++ = srcTex[(((vv>>16)+filter_dv)<<shifter) + ((uu>>16)+filter_du)];\
        uu += duu;							\
        vv += dvv;							\
      }									\
      while(_dest<=_destend)                                              \
      {                                                                   \
        *_dest++ = srcTex[((vv>>16)<<shifter) + (uu>>16)];                \
        uu += duu;							\
        vv += dvv;							\
    }

#define SCANEND \
    do									\
    {									\
      *z_buffer++ = izz;						\
      izz += dzz;							\
    }									\
    while (z_buffer <= lastZbuf)
#include "scanln16.inc"

#endif // NO_draw_scanline_map_filter

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_filter2

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC draw_scanline_map_filter2
#define SCANMAP 1
#define SCANLOOP \
      unsigned int w,gb;		\
      unsigned short color,r;		\
      while(_dest<=_destend)\
      {                                                                   \
        int addr=(((vv>>16))<<shifter)+(uu>>16);\
        int _=((uu&X_AND_FILTER)>>(14-LOG2_STEPS_X))+((vv&Y_AND_FILTER)>>((14-LOG2_STEPS_X)-LOG2_STEPS_Y));\
\
        w=tables.another_mul_table[_+0];\
        color=srcTex[addr];				\
        r=tables.color_565_table[NUM_LIGHT_INTENSITIES*2048+w+(color>>11)];\
        gb=tables.color_565_table[w+(color&2047)];	\
\
        w=tables.another_mul_table[_+2];\
	color=srcTex[addr+1];				\
        r+=tables.color_565_table[NUM_LIGHT_INTENSITIES*2048+w+(color>>11)];\
        gb+=tables.color_565_table[w+(color&2047)];	\
\
        w=tables.another_mul_table[_+1];\
	color=srcTex[addr+(1<<shifter)];		\
        r+=tables.color_565_table[NUM_LIGHT_INTENSITIES*2048+w+(color>>11)];\
        gb+=tables.color_565_table[w+(color&2047)];	\
\
        w=tables.another_mul_table[_+3];\
	color=srcTex[addr+(1<<shifter)+1];		\
        r+=tables.color_565_table[NUM_LIGHT_INTENSITIES*2048+w+(color>>11)];\
        gb+=tables.color_565_table[w+(color&2047)];	\
\
        *_dest++ = (r<<(11-LOG2_NUM_LIGHT_INTENSITIES))|(gb>>LOG2_NUM_LIGHT_INTENSITIES);                \
        uu += duu;							\
        vv += dvv;							\
      } /* fclose(fo) */

#define SCANEND \
    do									\
    {									\
      *z_buffer++ = izz;						\
      izz += dzz;							\
    }									\
    while (z_buffer <= lastZbuf)
#include "scanln16.inc"

#endif // NO_draw_scanline_map_filter2

//------------------------------------------------------------------

#ifndef NO_draw_scanline_z_buf_map

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC draw_scanline_z_buf_map
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
#include "scanln16.inc"

#endif // NO_draw_scanline_z_buf_map

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_alpha50

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC draw_scanline_map_alpha50
#define SCANMAP 1
#define SCANLOOP \
    do									\
    {									\
      UShort a = *_dest;						\
      UShort b = srcTex[((vv>>16)<<shifter) + (uu>>16)];		\
      *_dest++ = ((a & Scan::alpha_mask) >> 1) + ((b & Scan::alpha_mask) >> 1);	\
      uu += duu;							\
      vv += dvv;							\
    }									\
    while (_dest <= _destend)
#include "scanln16.inc"

#endif // NO_draw_scanline_map_alpha50

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_alpha_555

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC draw_scanline_map_alpha_555
#define SCANMAP 1
#define SCANLOOP \
    do									\
    {									\
      UShort a = *_dest;						\
      UShort b = srcTex[((vv>>16)<<shifter) + (uu>>16)];		\
      int r1 = a >> 10;	\
      int g1 = (a >> 5) & 0x1f;	\
      int b1 = a & 0x1f;	\
      int r2 = b >> 10;	\
      int g2 = (b >> 5) & 0x1f;	\
      int b2 = b & 0x1f;	\
      r1 = ((256-Scan::alpha_fact)*r1 + r2*Scan::alpha_fact) >> 8;	\
      g1 = ((256-Scan::alpha_fact)*g1 + g2*Scan::alpha_fact) >> 8;	\
      b1 = ((256-Scan::alpha_fact)*b1 + b2*Scan::alpha_fact) >> 8;	\
      *_dest++ = (r1<<10) | (g1<<5) | b1;				\
      uu += duu;							\
      vv += dvv;							\
    }									\
    while (_dest <= _destend)
#include "scanln16.inc"

#endif // NO_draw_scanline_map_alpha_555

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_alpha_565

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC draw_scanline_map_alpha_565
#define SCANMAP 1
#define SCANLOOP \
    do									\
    {									\
      UShort a = *_dest;						\
      UShort b = srcTex[((vv>>16)<<shifter) + (uu>>16)];		\
      int r1 = a >> 11;							\
      int g1 = (a >> 5) & 0x3f;						\
      int b1 = a & 0x1f;						\
      int r2 = b >> 11;							\
      int g2 = (b >> 5) & 0x3f;						\
      int b2 = b & 0x1f;						\
      r1 = ((256-Scan::alpha_fact)*r1 + r2*Scan::alpha_fact) >> 8;	\
      g1 = ((256-Scan::alpha_fact)*g1 + g2*Scan::alpha_fact) >> 8;	\
      b1 = ((256-Scan::alpha_fact)*b1 + b2*Scan::alpha_fact) >> 8;	\
      *_dest++ = (r1<<11) | (g1<<5) | b1;				\
      uu += duu;							\
      vv += dvv;							\
    }									\
    while (_dest <= _destend)
#include "scanln16.inc"

#endif // NO_draw_scanline_map_alpha_565

//------------------------------------------------------------------

#ifndef NO_draw_scanline_transp_map

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC draw_scanline_transp_map
#define SCANMAP 1
#define SCANLOOP \
    do									\
    {									\
      UShort c = srcTex[((vv>>16)<<shifter) + (uu>>16)];		\
      if (c)								\
      {									\
        *_dest++ = c;							\
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
#include "scanln16.inc"

#endif // NO_draw_scanline_transp_map

//------------------------------------------------------------------

#ifndef NO_draw_scanline

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC draw_scanline
#define SCANLOOP \
    do									\
    {									\
      *_dest++ = pal_table[srcTex[((uu>>16)&ander_w) + ((vv>>shifter_h)&ander_h)]];\
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
#include "scanln16.inc"

#endif // NO_draw_scanline

//------------------------------------------------------------------

#ifndef NO_draw_scanline_z_buf

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC draw_scanline_z_buf
#define SCANLOOP \
    do									\
    {									\
      if (izz >= (int)(*z_buffer))					\
      {									\
        *_dest++ = pal_table[srcTex[((uu>>16)&ander_w) + ((vv>>shifter_h)&ander_h)]];\
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
#include "scanln16.inc"

#endif // NO_draw_scanline_z_buf

//------------------------------------------------------------------

#ifndef NO_draw_scanline_z_buf_private

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC draw_scanline_z_buf_private
#define SCANLOOP \
    do									\
    {									\
      if (izz >= (int)(*z_buffer))					\
      {									\
        *_dest++ = pal_table[priv_to_global[srcTex[((uu>>16)&ander_w) + ((vv>>shifter_h)&ander_h)]]];\
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
#include "scanln16.inc"

#endif // NO_draw_scanline_z_buf_private

//------------------------------------------------------------------

#ifndef NO_draw_scanline_private

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC draw_scanline_private
#define SCANLOOP \
    do									\
    {									\
      *_dest++ = pal_table[priv_to_global[srcTex[((uu>>16)&ander_w) + ((vv>>shifter_h)&ander_h)]]];\
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
#include "scanln16.inc"

#endif // NO_draw_scanline_private

//------------------------------------------------------------------

#ifndef NO_draw_scanline_flat

void Scan16::draw_scanline_flat (int xx, unsigned char* d,
			       unsigned long* z_buf,
			       float inv_z, float u_div_z, float v_div_z)
{
  (void)u_div_z; (void)v_div_z;
  int color = Scan::flat_color;
  long izz = QInt24 (inv_z);
  long dzz = QInt24 (Scan::M);
  UShort* _dest = (UShort*)d;
  UShort* _destend = _dest + xx-1;
  do
  {
    *_dest++ = color;
    *z_buf++ = izz;
    izz += dzz;
  }
  while (_dest <= _destend);
}

#endif // NO_draw_scanline_flat

//------------------------------------------------------------------

#ifndef NO_draw_scanline_z_buf_flat

void Scan16::draw_scanline_z_buf_flat (int xx, unsigned char* d,
			       unsigned long* z_buf,
			       float inv_z, float u_div_z, float v_div_z)
{
  (void)u_div_z; (void)v_div_z;
  int color = Scan::flat_color;
  long izz = QInt24 (inv_z);
  long dzz = QInt24 (Scan::M);
  UShort* _dest = (UShort*)d;
  UShort* _destend = _dest + xx-1;
  do
  {
    if (izz >= (int)(*z_buf))
    {
      *_dest++ = color;
      *z_buf++ = izz;
    }
    else
    {
      _dest++;
      z_buf++;
    }
    izz += dzz;
  }
  while (_dest <= _destend);
}

#endif // NO_draw_scanline_z_buf_flat

//------------------------------------------------------------------

#ifndef NO_draw_scanline_fog_555

void Scan16::draw_scanline_fog_555 (int xx, unsigned char* d,
                              unsigned long* z_buf,
                              float inv_z, float u_div_z, float v_div_z)
{
  if (xx <= 0) return;
  (void)u_div_z; (void)v_div_z;
  UShort* _dest = (UShort*)d;
  UShort* _destend = _dest + xx-1;
  int dzz, izz;
  izz = QInt24 (inv_z);
  dzz = QInt24 (Scan::M);

  ULong fog_dens = Scan::fog_density;
  int fog_r = 256+Scan::fog_red;
  int fog_g = 256+Scan::fog_green;
  int fog_b = 256+Scan::fog_blue;

  do
  {
    int dens_dist = tables.exp_table_2[fog_dens / *(z_buf) - fog_dens / izz];
    int r = (*_dest) >> 10;
    int g = ((*_dest) >> 5) & 0x1f;
    int b = (*_dest) & 0x1f;

    r += tables.mul_table[dens_dist+fog_r - r];
    g += tables.mul_table[dens_dist+fog_g - g];
    b += tables.mul_table[dens_dist+fog_b - b];

    *_dest++ = (r<<10) | (g<<5) | b;
    z_buf++;
    izz += dzz;
  }
  while (_dest <= _destend);
}

#endif // NO_draw_scanline_fog_555

//------------------------------------------------------------------

#ifndef NO_draw_scanline_fog_565

void Scan16::draw_scanline_fog_565 (int xx, unsigned char* d,
                              unsigned long* z_buf,
                              float inv_z, float u_div_z, float v_div_z)
{
  if (xx <= 0) return;
  (void)u_div_z; (void)v_div_z;
  UShort* _dest = (UShort*)d;
  UShort* _destend = _dest + xx-1;
  int dzz, izz;
  izz = QInt24 (inv_z);
  dzz = QInt24 (Scan::M);

  ULong fog_dens = Scan::fog_density;
  int fog_r = 256+Scan::fog_red;
  int fog_g = 256+Scan::fog_green;
  int fog_b = 256+Scan::fog_blue;

  do
  {
    int dens_dist = tables.exp_table_2[fog_dens / *(z_buf) - fog_dens / izz];
    int r = (*_dest) >> 11;
    int g = ((*_dest) >> 5) & 0x3f;
    int b = (*_dest) & 0x1f;

    r += tables.mul_table[dens_dist+fog_r - r];
    g += tables.mul_table[dens_dist+fog_g - g];
    b += tables.mul_table[dens_dist+fog_b - b];

    *_dest++ = (r<<11) | (g<<5) | b;
    z_buf++;
    izz += dzz;
  }
  while (_dest <= _destend);
}

#endif // NO_draw_scanline_fog_565

//------------------------------------------------------------------

#ifndef NO_draw_scanline_fog_view_555

void Scan16::draw_scanline_fog_view_555 (int xx, unsigned char* d,
                              unsigned long* z_buf,
                              float inv_z, float u_div_z, float v_div_z)
{
  if (xx <= 0) return;
  (void)u_div_z; (void)v_div_z; (void)inv_z;
  UShort* _dest = (UShort*)d;
  UShort* _destend = _dest + xx-1;

  ULong fog_dens = Scan::fog_density;
  int fog_r = 256+Scan::fog_red;
  int fog_g = 256+Scan::fog_green;
  int fog_b = 256+Scan::fog_blue;

  do
  {
    int dens_dist = tables.exp_table_2[fog_dens / *(z_buf)];
    int r = (*_dest) >> 10;
    int g = ((*_dest) >> 5) & 0x1f;
    int b = (*_dest) & 0x1f;

    r += tables.mul_table[dens_dist+fog_r - r];
    g += tables.mul_table[dens_dist+fog_g - g];
    b += tables.mul_table[dens_dist+fog_b - b];

    *_dest++ = (r<<10) | (g<<5) | b;
    z_buf++;
  }
  while (_dest <= _destend);
}

#endif // NO_draw_scanline_fog_view_555

//------------------------------------------------------------------

#ifndef NO_draw_scanline_fog_view_565

void Scan16::draw_scanline_fog_view_565 (int xx, unsigned char* d,
                              unsigned long* z_buf,
                              float inv_z, float u_div_z, float v_div_z)
{
  if (xx <= 0) return;
  (void)u_div_z; (void)v_div_z; (void)inv_z;
  UShort* _dest = (UShort*)d;
  UShort* _destend = _dest + xx-1;

  ULong fog_dens = Scan::fog_density;
  int fog_r = 256+Scan::fog_red;
  int fog_g = 256+Scan::fog_green;
  int fog_b = 256+Scan::fog_blue;

  do
  {
    int dens_dist = tables.exp_table_2[fog_dens / *(z_buf)];
    int r = (*_dest) >> 11;
    int g = ((*_dest) >> 5) & 0x3f;
    int b = (*_dest) & 0x1f;

    r += tables.mul_table[dens_dist+fog_r - r];
    g += tables.mul_table[dens_dist+fog_g - g];
    b += tables.mul_table[dens_dist+fog_b - b];

    *_dest++ = (r<<11) | (g<<5) | b;
    z_buf++;
  }
  while (_dest <= _destend);
}

#endif // NO_draw_scanline_fog_view_565

//------------------------------------------------------------------

#ifndef NO_draw_scanline_fog_plane_555

void Scan16::draw_scanline_fog_plane_555 (int xx, unsigned char* d,
                              unsigned long* z_buf,
                              float inv_z, float u_div_z, float v_div_z)
{
  if (xx <= 0) return;
  (void)u_div_z; (void)v_div_z;
  UShort* _dest = (UShort*)d;
  UShort* _destend = _dest + xx-1;
  int izz;
  izz = QInt24 (inv_z);

  ULong fog_dens = Scan::fog_density;
  int fog_r = 256+Scan::fog_red;
  int fog_g = 256+Scan::fog_green;
  int fog_b = 256+Scan::fog_blue;

  long dist1 = QInt16 (1. / (1.+.3));
  long dist2 = QInt16 (1. / (1.+0));

  do
  {
    int dens_dist = tables.exp_table_2[fog_dens / dist1 - fog_dens / dist2];
    int r = (*_dest) >> 10;
    int g = ((*_dest) >> 5) & 0x1f;
    int b = (*_dest) & 0x1f;

    r += tables.mul_table[dens_dist+fog_r - r];
    g += tables.mul_table[dens_dist+fog_g - g];
    b += tables.mul_table[dens_dist+fog_b - b];

    *_dest++ = (r<<10) | (g<<5) | b;
    z_buf++;
  }
  while (_dest <= _destend);
}

#endif // NO_draw_scanline_fog_plane_555

//------------------------------------------------------------------

#ifndef NO_draw_scanline_fog_plane_565

void Scan16::draw_scanline_fog_plane_565 (int xx, unsigned char* d,
                              unsigned long* z_buf,
                              float inv_z, float u_div_z, float v_div_z)
{
}

#endif // NO_draw_scanline_fog_plane_565

//------------------------------------------------------------------

#ifndef NO_draw_scanline_zfill_only

void Scan16::draw_scanline_zfill_only (int xx, unsigned char* d,
                              unsigned long* z_buf,
                              float inv_z, float u_div_z, float v_div_z)
{
  (void)u_div_z; (void)v_div_z; (void)inv_z; (void)d;
  ULong* lastZbuf = z_buf + xx-1;
  int dzz, izz;
  izz = QInt24 (inv_z);
  dzz = QInt24 (Scan::M);

  do
  {
    *z_buf++ = izz;
    izz += dzz;
  }
  while (z_buf <= lastZbuf);
}

#endif // NO_draw_scanline_zfill_only

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline

void Scan16::draw_pi_scanline (void *dest, int len, long *zbuff, long u, long du,
  long v, long dv, long z, long dz, unsigned char *bitmap, int bitmap_log2w)
{
  UShort *_dest = (UShort *)dest;
  UShort *_destend = _dest + len;
  while (_dest < _destend)
  {
    if (z >= *zbuff)
    {
      *_dest = pal_table[*(bitmap + ((v >> 16) << bitmap_log2w) + (u >> 16))];
      *zbuff = z;
    }
    _dest++;
    zbuff++;
    u += du;
    v += dv;
    z += dz;
  } /* endwhile */
}

#endif // NO_draw_pi_scanline

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_zfill

void Scan16::draw_pi_scanline_zfill (void *dest, int len, long *zbuff, long u, long du,
  long v, long dv, long z, long dz, unsigned char *bitmap, int bitmap_log2w)
{
  UShort *_dest = (UShort *)dest;
  UShort *_destend = _dest + len;
  while (_dest < _destend)
  {
    *_dest++ = pal_table[*(bitmap + ((v >> 16) << bitmap_log2w) + (u >> 16))];
    *zbuff++ = z;
    u += du;
    v += dv;
    z += dz;
  } /* endwhile */
}

#endif // NO_draw_pi_scanline_zfill

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_gouroud_zfill_555

void Scan16::draw_pi_scanline_gouroud_zfill_555 (void *dest, int len, long *zbuff, long u, long du,
  long v, long dv, long z, long dz, unsigned char *bitmap, int bitmap_log2w,
  long r, long g, long b, long dr, long dg, long db)
{
  UShort *_dest = (UShort *)dest;
  UShort *_destend = _dest + len;
  UShort tex;
  int r1, g1, b1;
  while (_dest < _destend)
  {
    tex = pal_table[*(bitmap + ((v >> 16) << bitmap_log2w) + (u >> 16))];
    r1 = tex >> 10;
    g1 = (tex >> 5) & 0x1f;
    b1 = tex & 0x1f;
    r1 = (r1*r) >> (16+5);
    g1 = (g1*g) >> (16+5);
    b1 = (b1*b) >> (16+5);
    *_dest++ = (r1<<10) | (g1<<5) | b1;
    *zbuff++ = z;
    u += du; v += dv; z += dz;
    r += dr; g += dg; b += db;
  } /* endwhile */
}

#endif // NO_draw_pi_scanline_gouroud_zfill_555

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_gouroud_zfill_565

void Scan16::draw_pi_scanline_gouroud_zfill_565 (void *dest, int len, long *zbuff, long u, long du,
  long v, long dv, long z, long dz, unsigned char *bitmap, int bitmap_log2w,
  long r, long g, long b, long dr, long dg, long db)
{
  UShort *_dest = (UShort *)dest;
  UShort *_destend = _dest + len;
  UShort tex;
  int r1, g1, b1;
  while (_dest < _destend)
  {
    tex = pal_table[*(bitmap + ((v >> 16) << bitmap_log2w) + (u >> 16))];
    r1 = tex >> 11;
    g1 = (tex >> 5) & 0x3f;
    b1 = tex & 0x1f;
    r1 = (r1*r) >> (16+5);
    g1 = (g1*g) >> (16+6);
    b1 = (b1*b) >> (16+5);
    *_dest++ = (r1<<11) | (g1<<5) | b1;
    *zbuff++ = z;
    u += du; v += dv; z += dz;
    r += dr; g += dg; b += db;
  } /* endwhile */
}

#endif // NO_draw_pi_scanline_gouroud_zfill_565

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_gouroud_555

void Scan16::draw_pi_scanline_gouroud_555 (void *dest, int len, long *zbuff, long u, long du,
  long v, long dv, long z, long dz, unsigned char *bitmap, int bitmap_log2w,
  long r, long g, long b, long dr, long dg, long db)
{
  UShort *_dest = (UShort *)dest;
  UShort *_destend = _dest + len;
  UShort tex;
  int r1, g1, b1;
  while (_dest < _destend)
  {
    if (z >= *zbuff)
    {
      tex = pal_table[*(bitmap + ((v >> 16) << bitmap_log2w) + (u >> 16))];
      r1 = tex >> 10;
      g1 = (tex >> 5) & 0x1f;
      b1 = tex & 0x1f;
      r1 = (r1*r) >> (16+5);
      g1 = (g1*g) >> (16+5);
      b1 = (b1*b) >> (16+5);
      *_dest = (r1<<10) | (g1<<5) | b1;
      *zbuff = z;
    }
    _dest++;
    zbuff++;
    u += du; v += dv; z += dz;
    r += dr; g += dg; b += db;
  } /* endwhile */
}

#endif // NO_draw_pi_scanline_gouroud_555

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_gouroud_565

void Scan16::draw_pi_scanline_gouroud_565 (void *dest, int len, long *zbuff, long u, long du,
  long v, long dv, long z, long dz, unsigned char *bitmap, int bitmap_log2w,
  long r, long g, long b, long dr, long dg, long db)
{
  UShort *_dest = (UShort *)dest;
  UShort *_destend = _dest + len;
  UShort tex;
  int r1, g1, b1;
  while (_dest < _destend)
  {
    if (z >= *zbuff)
    {
      tex = pal_table[*(bitmap + ((v >> 16) << bitmap_log2w) + (u >> 16))];
      r1 = tex >> 11;
      g1 = (tex >> 5) & 0x3f;
      b1 = tex & 0x1f;
      r1 = (r1*r) >> (16+5);
      g1 = (g1*g) >> (16+6);
      b1 = (b1*b) >> (16+5);
      *_dest = (r1<<11) | (g1<<5) | b1;
      *zbuff = z;
    }
    _dest++;
    zbuff++;
    u += du; v += dv; z += dz;
    r += dr; g += dg; b += db;
  } /* endwhile */
}

#endif // NO_draw_pi_scanline_gouroud_565

//---------------------------------------------------------------------------

#ifdef DO_MMX
#ifndef NO_mmx_draw_scanline

void Scan16::mmx_draw_scanline (int /*xx*/, unsigned char* /*d*/,
                              unsigned long* /*z_buf*/,
                              float /*inv_z*/, float /*u_div_z*/, float /*v_div_z*/)
{
}

#endif // NO_mmx_draw_scanline

//------------------------------------------------------------------

#ifndef NO_mmx_draw_scanline_map

void Scan16::mmx_draw_scanline_map (int /*xx*/, unsigned char* /*d*/,
                              unsigned long* /*z_buf*/,
                              float /*inv_z*/, float /*u_div_z*/, float /*v_div_z*/)
{
}

#endif // NO_mmx_draw_scanline_map

//------------------------------------------------------------------

#ifndef NO_mmx_draw_pi_scanline

void Scan16::mmx_draw_pi_scanline (void* /*dest*/, int /*len*/, long* /*zbuff*/, long /*u*/, long /*du*/,
  long /*v*/, long /*dv*/, long /*z*/, long /*dz*/, unsigned char* /*bitmap*/, int /*bitmap_log2w*/)
{
}

#endif // NO_mmx_draw_pi_scanline

//------------------------------------------------------------------
#endif // DO_MMX
