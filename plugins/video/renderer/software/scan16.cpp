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
#include "ipolygon.h"
#include "ilghtmap.h"

#include "sttest.h"

#define SCAN16

//--//--//--//--//--//--//--//--//--//--//--/ assembler implementations --//--//

#if !defined (NO_ASSEMBLER)

#  if defined(PROC_INTEL)
#    if defined (DO_NASM)
#      include "i386/scan16a.h"
#    elif defined(COMP_GCC)
#      include "i386/scanln16.h"
#    elif defined(COMP_VC)
#      include "i386/scanln16vc.h"
#    endif //COMP_???
#  endif //PROC_INTEL

#endif //!NO_ASSEMBLER


//--//--//--//--//--//--//--//--//--//--//--//--//-- draw_scanline_XXXX --//--//

#ifndef NO_draw_scanline_map_zfil

#define SCANFUNC csScan_16_draw_scanline_map_zfil
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
#include "scanln.inc"

#endif // NO_draw_scanline_map_zfil

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_filt_zfil

extern int filter_bf;

#define SCANFUNC csScan_16_draw_scanline_map_filt_zfil
#define SCANMAP 1
#define SCANLOOP \
    int filter_du, filter_dv;						\
      while(_dest<=_destend&&((vv<BAILOUT_CONSTANT||uu<BAILOUT_CONSTANT)||(vv>=Scan.th2fp-BAILOUT_CONSTANT||uu>=Scan.tw2fp-BAILOUT_CONSTANT)))\
      {                                                                   \
        *_dest++ = srcTex[((vv>>16)<<shifter) + (uu>>16)];                \
        uu += duu;							\
        vv += dvv;							\
      }                                                                   \
      while ((_dest <= _destend)&&!((vv<BAILOUT_CONSTANT||uu<BAILOUT_CONSTANT)||(vv>=Scan.th2fp-BAILOUT_CONSTANT||uu>=Scan.tw2fp-BAILOUT_CONSTANT)))\
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
#include "scanln.inc"

#endif // NO_draw_scanline_map_filt_zfil

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_filt2_zfil

#define SCANFUNC csScan_16_draw_scanline_map_filt2_zfil
#define SCANMAP 1
#define SCANLOOP \
      unsigned int w,gb;		\
      unsigned short color,r;		\
      while(_dest<=_destend)\
      {                                                                   \
        int addr=(((vv>>16))<<shifter)+(uu>>16);\
        int _=((uu&X_AND_FILTER)>>(14-LOG2_STEPS_X))+((vv&Y_AND_FILTER)>>((14-LOG2_STEPS_X)-LOG2_STEPS_Y));\
\
        w=Scan.filter_mul_table[_+0];\
        color=srcTex[addr];				\
        r=Scan.color_565_table[NUM_LIGHT_INTENSITIES*2048+w+(color>>11)];\
        gb=Scan.color_565_table[w+(color&2047)];	\
\
        w=Scan.filter_mul_table[_+2];\
	color=srcTex[addr+1];				\
        r+=Scan.color_565_table[NUM_LIGHT_INTENSITIES*2048+w+(color>>11)];\
        gb+=Scan.color_565_table[w+(color&2047)];	\
\
        w=Scan.filter_mul_table[_+1];\
	color=srcTex[addr+(1<<shifter)];		\
        r+=Scan.color_565_table[NUM_LIGHT_INTENSITIES*2048+w+(color>>11)];\
        gb+=Scan.color_565_table[w+(color&2047)];	\
\
        w=Scan.filter_mul_table[_+3];\
	color=srcTex[addr+(1<<shifter)+1];		\
        r+=Scan.color_565_table[NUM_LIGHT_INTENSITIES*2048+w+(color>>11)];\
        gb+=Scan.color_565_table[w+(color&2047)];	\
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
#include "scanln.inc"

#endif // NO_draw_scanline_map_filt2_zfil

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_zuse

#define SCANFUNC csScan_16_draw_scanline_map_zuse
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
#include "scanln.inc"

#endif // NO_draw_scanline_map_zuse

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_alpha50

#define SCANFUNC csScan_16_draw_scanline_map_alpha50
#define SCANMAP 1
#define SCANLOOP \
    do									\
    {									\
      UShort tex = srcTex [((vv >> 16) << shifter) + (uu >> 16)];	\
      *_dest++ = ((*_dest & Scan.AlphaMask) >> 1) + ((tex & Scan.AlphaMask) >> 1);\
      uu += duu;							\
      vv += dvv;							\
    }									\
    while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_draw_scanline_map_alpha50

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_alpha_555

#define SCANFUNC csScan_16_draw_scanline_map_alpha_555
#define SCANMAP 1
#define SCANLOOP \
    do									\
    {									\
      UShort tex = srcTex [((vv >> 16) << shifter) + (uu >> 16)];	\
      int tr = *_dest & 0x7c00;						\
      int tg = *_dest & 0x03e0;						\
      int tb = *_dest & 0x001f;						\
      int r = (Scan.AlphaFact * ((tex & 0x7c00) - tr) >> 8) + tr;	\
      int g = (Scan.AlphaFact * ((tex & 0x03e0) - tg) >> 8) + tg;	\
      int b = (Scan.AlphaFact * ((tex & 0x001f) - tb) >> 8) + tb;	\
      *_dest++ = (r & 0x7c00) | (g & 0x03e0) | b;			\
      uu += duu;							\
      vv += dvv;							\
    }									\
    while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_draw_scanline_map_alpha_555

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_alpha_565

#define SCANFUNC csScan_16_draw_scanline_map_alpha_565
#define SCANMAP 1
#define SCANLOOP \
    do									\
    {									\
      UShort tex = srcTex [((vv >> 16) << shifter) + (uu >> 16)];	\
      int tr = *_dest & 0xf800;						\
      int tg = *_dest & 0x07e0;						\
      int tb = *_dest & 0x001f;						\
      int r = (Scan.AlphaFact * ((tex & 0xf800) - tr) >> 8) + tr;	\
      int g = (Scan.AlphaFact * ((tex & 0x07e0) - tg) >> 8) + tg;	\
      int b = (Scan.AlphaFact * ((tex & 0x001f) - tb) >> 8) + tb;	\
      *_dest++ = (r & 0xf800) | (g & 0x07e0) | b;			\
      uu += duu;							\
      vv += dvv;							\
    }									\
    while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_draw_scanline_map_alpha_565

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_key_zfil

#define SCANFUNC csScan_16_draw_scanline_map_key_zfil
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
#include "scanln.inc"

#endif // NO_draw_scanline_map_key_zfil

//------------------------------------------------------------------

#ifndef NO_draw_scanline_tex_zfil

#define SCANFUNC csScan_16_draw_scanline_tex_zfil
#define SCANLOOP \
    do									\
    {									\
      *_dest++ = Scan.PaletteTable[srcTex[((uu>>16)&ander_w) + ((vv>>shifter_h)&ander_h)]];\
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
#include "scanln.inc"

#endif // NO_draw_scanline_tex_zfil

//------------------------------------------------------------------

#ifndef NO_draw_scanline_tex_zuse

#define SCANFUNC csScan_16_draw_scanline_tex_zuse
#define SCANLOOP \
    do									\
    {									\
      if (izz >= (int)(*z_buffer))					\
      {									\
        *_dest++ = Scan.PaletteTable[srcTex[((uu>>16)&ander_w) + ((vv>>shifter_h)&ander_h)]];\
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
#include "scanln.inc"

#endif // NO_draw_scanline_tex_zuse

//------------------------------------------------------------------

#ifndef NO_draw_scanline_tex_priv_zuse

#define SCANFUNC csScan_16_draw_scanline_tex_priv_zuse
#define SCANLOOP \
    do									\
    {									\
      if (izz >= (int)(*z_buffer))					\
      {									\
        *_dest++ = Scan.PaletteTable[Scan.PrivToGlobal[srcTex[((uu>>16)&ander_w) + ((vv>>shifter_h)&ander_h)]]];\
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
#include "scanln.inc"

#endif // NO_draw_scanline_tex_priv_zuse

//------------------------------------------------------------------

#ifndef NO_draw_scanline_tex_priv_zfil

#define SCANFUNC csScan_16_draw_scanline_tex_priv_zfil
#define SCANLOOP \
    do									\
    {									\
      *_dest++ = Scan.PaletteTable[Scan.PrivToGlobal[srcTex[((uu>>16)&ander_w) + ((vv>>shifter_h)&ander_h)]]];\
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
#include "scanln.inc"

#endif // NO_draw_scanline_tex_priv_zfil

//------------------------------------------------------------------

#ifndef NO_draw_scanline_flat_zfil

void csScan_16_draw_scanline_flat_zfil (int xx, unsigned char* d,
  unsigned long* z_buf, float inv_z, float u_div_z, float v_div_z)
{
  (void)u_div_z; (void)v_div_z;
  int color = Scan.FlatColor;
  long izz = QInt24 (inv_z);
  long dzz = QInt24 (Scan.M);
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

#endif // NO_draw_scanline_flat_zfil

//------------------------------------------------------------------

#ifndef NO_draw_scanline_flat_zuse

void csScan_16_draw_scanline_flat_zuse (int xx, unsigned char* d,
  unsigned long* z_buf, float inv_z, float u_div_z, float v_div_z)
{
  (void)u_div_z; (void)v_div_z;
  int color = Scan.FlatColor;
  long izz = QInt24 (inv_z);
  long dzz = QInt24 (Scan.M);
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

#endif // NO_draw_scanline_flat_zuse

//------------------------------------------------------------------

#ifndef NO_draw_scanline_fog_555

void csScan_16_draw_scanline_fog_555 (int xx, unsigned char* d,
  unsigned long* z_buf, float inv_z, float u_div_z, float v_div_z)
{
  if (xx <= 0) return;
  (void)u_div_z; (void)v_div_z;
  UShort *_dest = (UShort *)d;
  UShort *_destend = _dest + xx;
  unsigned long izz = QInt24 (inv_z);
  int dzz = QInt24 (Scan.M);
  UShort fog_pix = Scan.FogR | Scan.FogG | Scan.FogB;
  ULong fog_dens = Scan.FogDensity;

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
        fd = fog_dens * (Scan.one_div_z [izb >> 12] - (Scan.one_div_z [izz >> 20] >> 8)) >> 12;
        goto fd_done;
      }
    }
    else if (izz > izb)
    {
      fd = fog_dens * (Scan.one_div_z [izb >> 12] - Scan.one_div_z [izz >> 12]) >> 12;
fd_done:
      if (fd < EXP_256_SIZE)
      {
        fd = Scan.exp_256 [fd];
        register int r = (fd * ((*_dest & 0x7c00) - Scan.FogR) >> 8) + Scan.FogR;
        register int g = (fd * ((*_dest & 0x03e0) - Scan.FogG) >> 8) + Scan.FogG;
        register int b = (fd * ((*_dest & 0x001f) - Scan.FogB) >> 8) + Scan.FogB;
        *_dest = (r & 0x7c00) | (g & 0x03e0) | b;
      }
      else
        *_dest = fog_pix;
    }
    _dest++;
    z_buf++;
    izz += dzz;
  }
  while (_dest < _destend);
}

#endif // NO_draw_scanline_fog_555

//------------------------------------------------------------------

#ifndef NO_draw_scanline_fog_565

void csScan_16_draw_scanline_fog_565 (int xx, unsigned char* d,
  unsigned long* z_buf, float inv_z, float u_div_z, float v_div_z)
{
  if (xx <= 0) return;
  (void)u_div_z; (void)v_div_z;
  UShort *_dest = (UShort *)d;
  UShort *_destend = _dest + xx;
  unsigned long izz = QInt24 (inv_z);
  int dzz = QInt24 (Scan.M);
  UShort fog_pix = Scan.FogR | Scan.FogG | Scan.FogB;
  ULong fog_dens = Scan.FogDensity;

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
        fd = fog_dens * (Scan.one_div_z [izb >> 12] - (Scan.one_div_z [izz >> 20] >> 8)) >> 12;
        goto fd_done;
      }
    }
    else if (izz > izb)
    {
      fd = fog_dens * (Scan.one_div_z [izb >> 12] - Scan.one_div_z [izz >> 12]) >> 12;
fd_done:
      if (fd < EXP_256_SIZE)
      {
        fd = Scan.exp_256 [fd];
        register int r = (fd * ((*_dest & 0xf800) - Scan.FogR) >> 8) + Scan.FogR;
        register int g = (fd * ((*_dest & 0x07e0) - Scan.FogG) >> 8) + Scan.FogG;
        register int b = (fd * ((*_dest & 0x001f) - Scan.FogB) >> 8) + Scan.FogB;
        *_dest = (r & 0xf800) | (g & 0x07e0) | b;
      }
      else
        *_dest = fog_pix;
    }
    _dest++;
    z_buf++;
    izz += dzz;
  }
  while (_dest < _destend);
}

#endif // NO_draw_scanline_fog_565

//------------------------------------------------------------------

#ifndef NO_draw_scanline_fog_view_555

void csScan_16_draw_scanline_fog_view_555 (int xx, unsigned char* d,
  unsigned long* z_buf, float inv_z, float u_div_z, float v_div_z)
{
  if (xx <= 0) return;
  (void)u_div_z; (void)v_div_z; (void)inv_z;
  UShort* _dest = (UShort*)d;
  UShort* _destend = _dest + xx;
  UShort fog_pix = Scan.FogR | Scan.FogG | Scan.FogB;
  ULong fog_dens = Scan.FogDensity;

  do
  {
    unsigned long izb = *z_buf;
    if (izb < 0x1000000)
    {
      int fd = fog_dens * Scan.one_div_z [izb >> 12] >> 12;
      if (fd < EXP_256_SIZE)
      {
        fd = Scan.exp_256 [fd];
        register int r = (fd * ((*_dest & 0x7c00) - Scan.FogR) >> 8) + Scan.FogR;
        register int g = (fd * ((*_dest & 0x03e0) - Scan.FogG) >> 8) + Scan.FogG;
        register int b = (fd * ((*_dest & 0x001f) - Scan.FogB) >> 8) + Scan.FogB;
        *_dest = (r & 0x7c00) | (g & 0x03e0) | b;
      }
      else
        *_dest = fog_pix;
    } /* endif */
    _dest++;
    z_buf++;
  }
  while (_dest < _destend);
}

#endif // NO_draw_scanline_fog_view_555

//------------------------------------------------------------------

#ifndef NO_draw_scanline_fog_view_565

void csScan_16_draw_scanline_fog_view_565 (int xx, unsigned char* d,
  unsigned long* z_buf, float inv_z, float u_div_z, float v_div_z)
{
  if (xx <= 0) return;
  (void)u_div_z; (void)v_div_z; (void)inv_z;
  UShort* _dest = (UShort*)d;
  UShort* _destend = _dest + xx;
  UShort fog_pix = Scan.FogR | Scan.FogG | Scan.FogB;
  ULong fog_dens = Scan.FogDensity;

  do
  {
    unsigned long izb = *z_buf;
    if (izb < 0x1000000)
    {
      int fd = fog_dens * Scan.one_div_z [izb >> 12] >> 12;
      if (fd < EXP_256_SIZE)
      {
        fd = Scan.exp_256 [fd];
        register int r = (fd * ((*_dest & 0xf800) - Scan.FogR) >> 8) + Scan.FogR;
        register int g = (fd * ((*_dest & 0x07e0) - Scan.FogG) >> 8) + Scan.FogG;
        register int b = (fd * ((*_dest & 0x001f) - Scan.FogB) >> 8) + Scan.FogB;
        *_dest = (r & 0xf800) | (g & 0x07e0) | b;
      }
      else
        *_dest = fog_pix;
    } /* endif */
    _dest++;
    z_buf++;
  }
  while (_dest < _destend);
}

#endif // NO_draw_scanline_fog_view_565

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_tex_zuse

#define PI_SCANFUNC csScan_16_draw_pi_scanline_tex_zuse
#define PI_ZUSE
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_tex_zuse

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_tex_zfil

#define PI_SCANFUNC csScan_16_draw_pi_scanline_tex_zfil
#define PI_ZFILL
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_tex_zfil

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_tex_gouraud_zfil_555

#define PI_SCANFUNC csScan_16_draw_pi_scanline_tex_gouraud_zfil_555
#define PI_ZFILL
#define PI_GOURAUD
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_tex_gouraud_zfil_555

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_tex_gouraud_zfil_565

#define PI_SCANFUNC csScan_16_draw_pi_scanline_tex_gouraud_zfil_565
#define PI_ZFILL
#define PI_GOURAUD
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_tex_gouraud_zfil_565

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_tex_gouraud_zuse_555

#define PI_SCANFUNC csScan_16_draw_pi_scanline_tex_gouraud_zuse_555
#define PI_ZUSE
#define PI_GOURAUD
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_tex_gouraud_zuse_555

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_tex_gouraud_zuse_565

#define PI_SCANFUNC csScan_16_draw_pi_scanline_tex_gouraud_zuse_565
#define PI_ZUSE
#define PI_GOURAUD
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_tex_gouraud_zuse_565

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_flat_zuse

#define PI_SCANFUNC csScan_16_draw_pi_scanline_flat_zuse
#define PI_ZUSE
#define PI_FLAT
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_flat_zuse

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_flat_zfil

#define PI_SCANFUNC csScan_16_draw_pi_scanline_flat_zfil
#define PI_ZFILL
#define PI_FLAT
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_flat_zfil

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_flat_gouraud_zfil_555

#define PI_SCANFUNC csScan_16_draw_pi_scanline_flat_gouraud_zfil_555
#define PI_ZFILL
#define PI_FLAT
#define PI_GOURAUD
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_flat_gouraud_zfil_555

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_flat_gouraud_zfil_565

#define PI_SCANFUNC csScan_16_draw_pi_scanline_flat_gouraud_zfil_565
#define PI_ZFILL
#define PI_FLAT
#define PI_GOURAUD
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_flat_gouraud_zfil_565

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_flat_gouraud_zuse_555

#define PI_SCANFUNC csScan_16_draw_pi_scanline_flat_gouraud_zuse_555
#define PI_ZUSE
#define PI_FLAT
#define PI_GOURAUD
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_flat_gouraud_zuse_555

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_flat_gouraud_zuse_565

#define PI_SCANFUNC csScan_16_draw_pi_scanline_flat_gouraud_zuse_565
#define PI_ZUSE
#define PI_FLAT
#define PI_GOURAUD
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_flat_gouraud_zuse_565

//------------------------------------------------------------------

#ifndef NO_draw_pifx_scanline_zfil_555

#define PI_SCANFUNC csScan_16_draw_pifx_scanline_zfil_555
#define PI_ZFILL
#define PI_GOURAUD
#define PI_R5G5B5
#define PI_BLEND
#include "scanpi.inc"

#endif

//------------------------------------------------------------------

#ifndef NO_draw_pifx_scanline_zfil_565

#define PI_SCANFUNC csScan_16_draw_pifx_scanline_zfil_565
#define PI_ZFILL
#define PI_GOURAUD
#define PI_R5G6B5
#define PI_BLEND
#include "scanpi.inc"

#endif

//------------------------------------------------------------------

#ifndef NO_draw_pifx_scanline_zuse_555

#define PI_SCANFUNC csScan_16_draw_pifx_scanline_zuse_555
#define PI_ZUSE
#define PI_GOURAUD
#define PI_R5G5B5
#define PI_BLEND
#include "scanpi.inc"

#endif

//------------------------------------------------------------------

#ifndef NO_draw_pifx_scanline_zuse_565

#define PI_SCANFUNC csScan_16_draw_pifx_scanline_zuse_565
#define PI_ZUSE
#define PI_GOURAUD
#define PI_R5G6B5
#define PI_BLEND
#include "scanpi.inc"

#endif

//------------------------------------------------------------------

#ifndef NO_draw_pifx_scanline_transp_zfil_555

#define PI_SCANFUNC csScan_16_draw_pifx_scanline_transp_zfil_555
#define PI_ZFILL
#define PI_GOURAUD
#define PI_R5G5B5
#define PI_COLORKEY
#define PI_BLEND
#include "scanpi.inc"

#endif

//------------------------------------------------------------------

#ifndef NO_draw_pifx_scanline_transp_zfil_565

#define PI_SCANFUNC csScan_16_draw_pifx_scanline_transp_zfil_565
#define PI_ZFILL
#define PI_GOURAUD
#define PI_R5G6B5
#define PI_COLORKEY
#define PI_BLEND
#include "scanpi.inc"

#endif

//------------------------------------------------------------------

#ifndef NO_draw_pifx_scanline_transp_zuse_555

#define PI_SCANFUNC csScan_16_draw_pifx_scanline_transp_zuse_555
#define PI_ZUSE
#define PI_GOURAUD
#define PI_R5G5B5
#define PI_COLORKEY
#define PI_BLEND
#include "scanpi.inc"

#endif

//------------------------------------------------------------------

#ifndef NO_draw_pifx_scanline_transp_zuse_565

#define PI_SCANFUNC csScan_16_draw_pifx_scanline_transp_zuse_565
#define PI_ZUSE
#define PI_GOURAUD
#define PI_R5G6B5
#define PI_COLORKEY
#define PI_BLEND
#include "scanpi.inc"

#endif

//------------------------------------------------------------------
