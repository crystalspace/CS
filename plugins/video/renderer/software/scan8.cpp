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
#include "tcache.h"
#include "isystem.h"
#include "ipolygon.h"

#include "sttest.h"

/// The only instance of this structure lives here
csScanSetup Scan =
{
  16,			// InterpolStep
  4,			// InterpolShift
  INTER_MODE_SMART	// InterpolMode
};

//--//--//--//--//--//--//--//--//--//--//--/ assembler implementations --//--//

#if !defined(NO_ASSEMBLER)

#  if defined(PROC_M68K)
#    if defined(COMP_GCC)
#      include "m68k/scanln8.h"
#    endif // COMP_GCC
#  endif // PROC_M68K

#  if defined (PROC_INTEL)
#    if defined (DO_NASM)
#      include "i386/scan8a.h"
#    elif defined (COMP_GCC)
#      include "i386/scanln8.h"
#    elif defined (COMP_VC) || defined (COMP_WCC)
#      include "i386/scanln8vc.h"
#    endif // COMP_???
#  endif // PROC_INTEL

#endif //!NO_ASSEMBLER

//--//--//--//--//--//--//--//--//--//--//--//--//-- draw_scanline_XXXX --//--//

#ifndef NO_draw_scanline_map_zfil

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC csScan_8_draw_scanline_map_zfil
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
#include "scanln8.inc"

#endif // NO_draw_scanline_map_zfil

//------------------------------------------------------------------

int filter_bf;

#ifndef NO_draw_scanline_map_filt_zfil

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC csScan_8_draw_scanline_map_filt_zfil
#define SCANMAP 1
#define SCANLOOP \
    int filter_bf_shifted=filter_bf>>1,filter_du, filter_dv;            \
    while(_dest<=_destend&&((vv<BAILOUT_CONSTANT||uu<BAILOUT_CONSTANT)||(vv>=Scan.th2fp-BAILOUT_CONSTANT||uu>=Scan.tw2fp-BAILOUT_CONSTANT)))\
    {                                                                   \
      *_dest++ = srcTex[((vv>>16)<<shifter) + (uu>>16)];                \
      uu += duu;                                                        \
      vv += dvv;                                                        \
    }                                                                   \
    while ((_dest <= _destend)&&!((vv<BAILOUT_CONSTANT||uu<BAILOUT_CONSTANT)||(vv>=Scan.th2fp-BAILOUT_CONSTANT||uu>=Scan.tw2fp-BAILOUT_CONSTANT)))\
    {                                                                   \
      if (((((long)_dest)) & filter_bf_shifted) != 0)                   \
      {                                                                 \
        if ((uu&0xffff) < 64*256) filter_du = -1;                       \
        else if ((uu&0xffff) > 192*256) filter_du = 1;                  \
        else filter_du = 0;                                             \
        if ((vv&0xffff) < 64*256) filter_dv = -1;                       \
        else if ((vv&0xffff) > 192*256) filter_dv = 1;                  \
        else filter_dv = 0;                                             \
      }                                                                 \
      else filter_du = filter_dv = 0;                                   \
      *_dest++ = srcTex[(((vv>>16)+filter_dv)<<shifter) + ((uu>>16)+filter_du)];\
      uu += duu;                                                        \
      vv += dvv;                                                        \
    }                                                                   \
    while(_dest<=_destend)                                              \
    {                                                                   \
      *_dest++ = srcTex[((vv>>16)<<shifter) + (uu>>16)];                \
      uu += duu;                                                        \
      vv += dvv;                                                        \
    }

#define SCANEND \
    do                                                                  \
    {                                                                   \
      *z_buffer++ = izz;                                                \
      izz += dzz;                                                       \
    }                                                                   \
    while (z_buffer <= lastZbuf)
#include "scanln8.inc"

#endif // NO_draw_scanline_map_filt_zfil

//------------------------------------------------------------------

#ifndef NO_draw_scanline_tex_zfil

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC csScan_8_draw_scanline_tex_zfil
#define SCANLOOP \
    do                                                                  \
    {                                                                   \
      *_dest++ = srcTex[((uu>>16)&ander_w) + ((vv>>shifter_h)&ander_h)];\
      uu += duu;                                                        \
      vv += dvv;                                                        \
    }                                                                   \
    while (_dest <= _destend)
#define SCANEND \
    do                                                                  \
    {                                                                   \
      *z_buffer++ = izz;                                                \
      izz += dzz;                                                       \
    }                                                                   \
    while (z_buffer <= lastZbuf)
#include "scanln8.inc"

#endif // NO_draw_scanline_tex_zfil

//------------------------------------------------------------------

#ifndef NO_draw_scanline_tex_priv_zfil

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC csScan_8_draw_scanline_tex_priv_zfil
#define SCANLOOP \
    do                                                                  \
    {                                                                   \
      *_dest++ = Scan.PrivToGlobal[srcTex[((uu>>16)&ander_w) + ((vv>>shifter_h)&ander_h)]];        \
      uu += duu;                                                        \
      vv += dvv;                                                        \
    }                                                                   \
    while (_dest <= _destend)
#define SCANEND \
    do                                                                  \
    {                                                                   \
      *z_buffer++ = izz;							\
      izz += dzz;                                                       \
    }                                                                   \
    while (z_buffer <= lastZbuf)
#include "scanln8.inc"

#endif // NO_draw_scanline_tex_priv_zfil

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_alpha1

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC csScan_8_draw_scanline_map_alpha1
#define SCANMAP 1
#define SCANLOOP \
    do                                                                  \
    {                                                                   \
      *_dest = Scan.AlphaMap[*_dest][srcTex[((vv>>16)<<shifter) + (uu>>16)]];\
      _dest++;                                                          \
      uu += duu;                                                        \
      vv += dvv;                                                        \
    }                                                                   \
    while (_dest <= _destend)
#include "scanln8.inc"

#endif // NO_draw_scanline_map_alpha1

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_alpha2

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC csScan_8_draw_scanline_map_alpha2
#define SCANMAP 1
#define SCANLOOP \
    do                                                                  \
    {                                                                   \
      *_dest = Scan.AlphaMap[srcTex[((vv>>16)<<shifter) + (uu>>16)]][*_dest];\
      _dest++;                                                          \
      uu += duu;                                                        \
      vv += dvv;                                                        \
    }                                                                   \
    while (_dest <= _destend)
#include "scanln8.inc"

#endif // NO_draw_scanline_map_alpha2

//------------------------------------------------------------------

#ifndef NO_draw_scanline_tex_key_zfil

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC csScan_8_draw_scanline_tex_key_zfil
#define SCANLOOP \
    unsigned char c;                                                    \
    do                                                                  \
    {                                                                   \
      c = srcTex[((uu>>16)&ander_w) + ((vv>>shifter_h)&ander_h)];	\
      if (c)                                                            \
      {                                                                 \
        *_dest++ = c;                                                   \
        *z_buffer++ = izz;						\
      }                                                                 \
      else                                                              \
      {                                                                 \
        _dest++;                                                        \
        z_buffer++;							\
      }                                                                 \
      uu += duu;                                                        \
      vv += dvv;                                                        \
      izz += dzz;                                                       \
    }                                                                   \
    while (_dest <= _destend)
#include "scanln8.inc"

#endif // draw_scanline_tex_key_zfil

//------------------------------------------------------------------

#ifndef NO_draw_scanline_tex_priv_key_zfil

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC csScan_8_draw_scanline_tex_priv_key_zfil
#define SCANLOOP \
    do                                                                  \
    {                                                                   \
      unsigned char c = srcTex[((uu>>16)&ander_w) +                     \
        ((vv>>shifter_h)&ander_h)];                                     \
      if (c)                                                            \
      {                                                                 \
        *_dest++ = Scan.PrivToGlobal[c];				\
        *z_buffer++ = izz;						\
      }                                                                 \
      else                                                              \
      {                                                                 \
        _dest++;                                                        \
        z_buffer++;							\
      }                                                                 \
      uu += duu;                                                        \
      vv += dvv;                                                        \
      izz += dzz;                                                       \
    }                                                                   \
    while (_dest <= _destend)
#include "scanln8.inc"

#endif // NO_draw_scanline_tex_priv_key_zfil

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_key_zfil

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC csScan_8_draw_scanline_map_key_zfil
#define SCANMAP 1
#define SCANLOOP \
    do                                                                  \
    {                                                                   \
      unsigned char c = srcTex[((vv>>16)<<shifter) + (uu>>16)];         \
      if (c)                                                            \
      {                                                                 \
        *_dest++ = c;                                                   \
        *z_buffer++ = izz;						\
      }                                                                 \
      else                                                              \
      {                                                                 \
        _dest++;                                                        \
        z_buffer++;							\
      }                                                                 \
      uu += duu;                                                        \
      vv += dvv;                                                        \
      izz += dzz;                                                       \
    }                                                                   \
    while (_dest <= _destend)
#include "scanln8.inc"

#endif // NO_draw_scanline_map_key_zfil

//------------------------------------------------------------------

#ifndef NO_draw_scanline_tex_zuse

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC csScan_8_draw_scanline_tex_zuse
#define SCANLOOP \
    do                                                                  \
    {                                                                   \
      if (izz >= (int)(*z_buffer))					\
      {                                                                 \
        *_dest++ = srcTex[((uu>>16)&ander_w) + ((vv>>shifter_h)&ander_h)];\
        *z_buffer++ = izz;						\
      }                                                                 \
      else                                                              \
      {                                                                 \
        _dest++;                                                        \
        z_buffer++;							\
      }                                                                 \
      uu += duu;                                                        \
      vv += dvv;                                                        \
      izz += dzz;                                                       \
    }                                                                   \
    while (_dest <= _destend)
#include "scanln8.inc"

#endif // NO_draw_scanline_tex_zuse

//------------------------------------------------------------------

#ifndef NO_draw_scanline_tex_priv_zuse

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC csScan_8_draw_scanline_tex_priv_zuse
#define SCANLOOP \
    do                                                                  \
    {                                                                   \
      if (izz >= (int)(*z_buffer))					\
      {                                                                 \
        *_dest++ = Scan.PrivToGlobal[srcTex[((uu>>16)&ander_w) + ((vv>>shifter_h)&ander_h)]];      \
        *z_buffer++ = izz;						\
      }                                                                 \
      else                                                              \
      {                                                                 \
        _dest++;                                                        \
        z_buffer++;							\
      }                                                                 \
      uu += duu;                                                        \
      vv += dvv;                                                        \
      izz += dzz;                                                       \
    }                                                                   \
    while (_dest <= _destend)
#include "scanln8.inc"

#endif // NO_draw_scanline_tex_priv_zuse

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_zuse

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC csScan_8_draw_scanline_map_zuse
#define SCANMAP 1
#define SCANLOOP \
    do                                                                  \
    {                                                                   \
      if (izz >= (int)(*z_buffer))					\
      {                                                                 \
        *_dest++ = srcTex[((vv>>16)<<shifter) + (uu>>16)];              \
        *z_buffer++ = izz;						\
      }                                                                 \
      else                                                              \
      {                                                                 \
        _dest++;                                                        \
        z_buffer++;							\
      }                                                                 \
      uu += duu;                                                        \
      vv += dvv;                                                        \
      izz += dzz;                                                       \
    }                                                                   \
    while (_dest <= _destend)
#include "scanln8.inc"

#endif // NO_draw_scanline_map_zuse

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_light

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC csScan_8_draw_scanline_map_light
#define SCANMAP 1
#define SCANLOOP \
    do                                                                  \
    {                                                                   \
      *_dest++ = Scan.LightTable[srcTex[((vv>>16)<<shifter) + (uu>>16)]];\
      uu += duu;                                                        \
      vv += dvv;                                                        \
    }                                                                   \
    while (_dest <= _destend)
#define SCANEND \
    do                                                                  \
    {                                                                   \
      *z_buffer++ = izz;							\
      izz += dzz;                                                       \
    }                                                                   \
    while (z_buffer <= lastZbuf)
#include "scanln8.inc"

#endif // NO_draw_scanline_map_light

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_light_zuse

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC csScan_8_draw_scanline_map_light_zuse
#define SCANMAP 1
#define SCANLOOP \
    do                                                                  \
    {                                                                   \
      if (izz >= (int)(*z_buffer))					\
      {                                                                 \
        *_dest++ = Scan.LightTable[srcTex[((vv>>16)<<shifter) + (uu>>16)]];\
        *z_buffer++ = izz;						\
      }                                                                 \
      else                                                              \
      {                                                                 \
        _dest++;                                                        \
        z_buffer++;							\
      }                                                                 \
      uu += duu;                                                        \
      vv += dvv;                                                        \
      izz += dzz;                                                       \
    }                                                                   \
    while (_dest <= _destend)
#include "scanln8.inc"

#endif // NO_draw_scanline_map_light_zuse

//------------------------------------------------------------------

#ifndef NO_draw_scanline_flat_zfil

void csScan_8_draw_scanline_flat_zfil (int xx, unsigned char* d,
  unsigned long *z_buf, float inv_z, float u_div_z, float v_div_z)
{
  (void)u_div_z; (void)v_div_z;
  int color = Scan.FlatColor;
  long izz = QInt24 (inv_z);
  long dzz = QInt24 (Scan.M);
  UByte* _dest = (UByte*)d;
  UByte* _destend = _dest + xx-1;
  do
  {
    *_dest++ = color;
    *z_buf++ = izz;
    izz += dzz;
  }
  while (_dest <= _destend);
}

#endif // NO_draw_scanline_flat_zfil

//------------------------------------------------------------------

#ifndef NO_draw_scanline_flat_zuse

void csScan_8_draw_scanline_flat_zuse (int xx, unsigned char* d,
  unsigned long *z_buf, float inv_z, float u_div_z, float v_div_z)
{
  (void)u_div_z; (void)v_div_z;
  int color = Scan.FlatColor;
  long izz = QInt24 (inv_z);
  long dzz = QInt24 (Scan.M);
  UByte* _dest = (UByte*)d;
  UByte* _destend = _dest + xx-1;
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

#endif // NO_draw_scanline_flat_zuse

//------------------------------------------------------------------

#ifndef NO_draw_scanline_fog

void csScan_8_draw_scanline_fog (int xx, unsigned char* d,
  unsigned long *z_buf, float inv_z, float u_div_z, float v_div_z)
{
  if (xx <= 0) return;
  (void)u_div_z; (void)v_div_z;
  unsigned char *_dest = (unsigned char *)d;
  unsigned char *_destend = _dest + xx;
  unsigned long izz = QInt24 (inv_z);
  int dzz = QInt24 (Scan.M);
  ULong fog_dens = Scan.FogDensity;
  unsigned char fog_pix = Scan.FogIndex;

  do
  {
    int fd;
    unsigned long izb = *z_buf;
    if (izz >= 0x1000000)
    {
      // izz exceeds our 1/x table, so compute fd aproximatively and go on.
      // This happens seldom, only when we're very close to fog, but not
      // inside it; however we should handle this case as well.
      if ((izb < 0x1000000) && (izz > izb))
      {
        fd = fog_dens * (tables.one_div_z [izb >> 12] - (tables.one_div_z [izz >> 20] >> 8)) >> 12;
        goto fd_done;
      }
    }
    else if (izz > izb)
    {
      fd = fog_dens * (tables.one_div_z [izb >> 12] - tables.one_div_z [izz >> 12]) >> 12;
fd_done:
      if (fd < EXP_16_SIZE)
        *_dest = Scan.Fog8 [(tables.exp_16 [fd] << 8) | *_dest];
      else
        *_dest = fog_pix;
    }
    _dest++;
    z_buf++;
    izz += dzz;
  }
  while (_dest < _destend);
}

#endif // NO_draw_scanline_fog

//------------------------------------------------------------------

#ifndef NO_draw_scanline_fog_view

void csScan_8_draw_scanline_fog_view (int xx, unsigned char* d,
  unsigned long *z_buf, float inv_z, float u_div_z, float v_div_z)
{
  if (xx <= 0) return;
  (void)u_div_z; (void)v_div_z; (void)inv_z;
  unsigned char *_dest = (unsigned char *)d;
  unsigned char *_destend = _dest + xx;
  ULong fog_dens = Scan.FogDensity;
  unsigned char fog_pix = Scan.FogIndex;

  do
  {
    unsigned long izb = *z_buf;
    if (izb < 0x1000000)
    {
      int fd = fog_dens * tables.one_div_z [izb >> 12] >> 12;
      if (fd < EXP_16_SIZE)
        *_dest = Scan.Fog8 [(tables.exp_16 [fd] << 8) | *_dest];
      else
        *_dest = fog_pix;
    }
    _dest++;
    z_buf++;
  }
  while (_dest < _destend);
}

#endif // NO_draw_scanline_fog_view

//------------------------------------------------------------------

#if 0
void csScan_8_light_scanline (int xx, int uu, int vv, unsigned char* d,
  float d1, float d2, float dd1, float dd2, float dd_u, float da_u,
  float dd_v, float da_v, int lu, int lv, int sq_rad)
{
  int i;
  float r, u1, v1;
  int uu1, vv1, duu, dvv;
  int sqd;
  unsigned char* lt;

  while (xx > 0)
  {
    d1 -= dd1;
    d2 += dd2;
    r = d1 / d2;

    i = Scan.InterpolStep;
    if (xx < i) i = xx;
    xx -= i;

    u1 = r*dd_u + da_u;
    v1 = r*dd_v + da_v;

    uu1 = QInt16 (u1);
    vv1 = QInt16 (v1);

    duu = (uu1-uu)/Scan.InterpolStep;
    dvv = (vv1-vv)/Scan.InterpolStep;

    while (i-- > 0)
    {
      sqd = ((uu>>16)-lu)*((uu>>16)-lu) + ((vv>>16)-lv)*((vv>>16)-lv);
      sqd = sq_rad-sqd;
      if (sqd > 0)
      {
        if (sqd > (255-NORMAL_LIGHT_LEVEL)) sqd = 255-NORMAL_LIGHT_LEVEL;
        lt = textures->get_light_table (NORMAL_LIGHT_LEVEL+sqd);
        *d++ = lt[*d];
      }
      uu += duu;
      vv += dvv;
    }
  }
}
#endif

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_tex_zuse

void csScan_8_draw_pi_scanline_tex_zuse (void *dest, int len,
  unsigned long *zbuff, long u, long du, long v, long dv,
  unsigned long z, long dz, unsigned char *bitmap, int bitmap_log2w)
{
  unsigned char *_dest = (unsigned char *)dest;
  unsigned char *_destend = _dest + len;
  while (_dest < _destend)
  {
    if (z >= *zbuff)
    {
      *_dest = *(bitmap + ((v >> 16) << bitmap_log2w) + (u >> 16));
      *zbuff = z;
    }
    _dest++;
    zbuff++;
    u += du;
    v += dv;
    z += dz;
  } /* endwhile */
}

#endif // NO_draw_pi_scanline_tex_zuse

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_tex_zfil

void csScan_8_draw_pi_scanline_tex_zfil (void *dest, int len,
  unsigned long *zbuff, long u, long du, long v, long dv,
  unsigned long z, long dz, unsigned char *bitmap, int bitmap_log2w)
{
  unsigned char *_dest = (unsigned char *)dest;
  unsigned char *_destend = _dest + len;
  while (_dest < _destend)
  {
    *_dest++ = *(bitmap + ((v >> 16) << bitmap_log2w) + (u >> 16));
    *zbuff++ = z;
    u += du;
    v += dv;
    z += dz;
  } /* endwhile */
}

#endif // NO_draw_pi_scanline_tex_zfil

void csScan_8_draw_pifx_scanline_dummy
  (void *dest, int len, unsigned long *zbuff, long u, long du, long v, long dv,
   unsigned long z, long dz, unsigned char *bitmap, int bitmap_log2w,
   ULong r, ULong g, ULong b, long dr, long dg, long db,
   UByte* BlendingTable)
{
   (void) dest;(void) len;(void) zbuff;
   (void) u;(void) du;(void) v;(void) dv;(void) z;(void) dz;
   (void) *bitmap;(void) bitmap_log2w;
   (void) r; (void) g; (void) b; (void) dr; (void) dg; (void) db;
   (void) BlendingTable;
}
