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
#include "tcache.h"
#include "isystem.h"
#include "ipolygon.h"
#include "soft_g3d.h"

#include "sttest.h"

#define SCAN8
#define COLORMAP	((UByte *)Scan.PaletteTable)

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

#include "scanxx.inc"

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_filt_zfil

#define SCANFUNC csScan_8_draw_scanline_map_filt_zfil
#define SCANMAP
#define SCANLOOP \
    int filter_bf_shifted=csGraphics3DSoftware::filter_bf>>1,filter_du, filter_dv;            \
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
#include "scanln.inc"

#endif // NO_draw_scanline_map_filt_zfil

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_alpha1

#define SCANFUNC csScan_8_draw_scanline_map_alpha1
#define SCANMAP
#define SCANLOOP \
    do                                                                  \
    {                                                                   \
      *_dest = Scan.AlphaMap [(unsigned (*_dest) << 8) +		\
        srcTex [((vv >> 16) << shifter) + (uu >> 16)]];			\
      _dest++;                                                          \
      uu += duu;                                                        \
      vv += dvv;                                                        \
    }                                                                   \
    while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_draw_scanline_map_alpha1

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_alpha2

#define SCANFUNC csScan_8_draw_scanline_map_alpha2
#define SCANMAP
#define SCANLOOP \
    do                                                                  \
    {                                                                   \
      *_dest = Scan.AlphaMap [(*_dest + unsigned (			\
        srcTex [((vv >> 16) << shifter) + (uu >> 16)])) << 8];		\
      _dest++;                                                          \
      uu += duu;                                                        \
      vv += dvv;                                                        \
    }                                                                   \
    while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_draw_scanline_map_alpha2

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
  unsigned long dzz = QInt24 (Scan.M);
  ULong fog_dens = Scan.FogDensity;
  unsigned char fog_pix = Scan.FogPix;

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
      if (fd < EXP_32_SIZE)
        *_dest = Scan.Fog8 [(Scan.exp_16 [fd] << 8) | *_dest];
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
  unsigned char fog_pix = Scan.FogPix;

  do
  {
    unsigned long izb = *z_buf;
    if (izb < 0x1000000)
    {
      int fd = fog_dens * Scan.one_div_z [izb >> 12] >> 12;
      if (fd < EXP_32_SIZE)
        *_dest = Scan.Fog8 [(Scan.exp_16 [fd] << 8) | *_dest];
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

#ifndef NO_draw_pi_scanline_tex_zuse

#define PI_SCANFUNC csScan_8_draw_pi_scanline_tex_zuse
#define PI_ZUSE
#define PI_INDEX8
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_tex_zuse

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_tex_zfil

#define PI_SCANFUNC csScan_8_draw_pi_scanline_tex_zfil
#define PI_ZFILL
#define PI_INDEX8
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_tex_zfil

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_tex_gouraud_zfil

#define PI_SCANFUNC csScan_8_draw_pi_scanline_tex_gouraud_zfil
#define PI_ZFILL
#define PI_GOURAUD
#define PI_INDEX8
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_tex_gouraud_zfil

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_tex_gouraud_zuse

#define PI_SCANFUNC csScan_8_draw_pi_scanline_tex_gouraud_zuse
#define PI_ZUSE
#define PI_GOURAUD
#define PI_INDEX8
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_tex_gouraud_zuse

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_flat_zuse

#define PI_SCANFUNC csScan_8_draw_pi_scanline_flat_zuse
#define PI_ZUSE
#define PI_FLAT
#define PI_INDEX8
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_flat_zuse

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_flat_zfil

#define PI_SCANFUNC csScan_8_draw_pi_scanline_flat_zfil
#define PI_ZFILL
#define PI_FLAT
#define PI_INDEX8
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_flat_zfil

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_flat_gouraud_zfil

#define PI_SCANFUNC csScan_8_draw_pi_scanline_flat_gouraud_zfil
#define PI_ZFILL
#define PI_FLAT
#define PI_GOURAUD
#define PI_INDEX8
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_flat_gouraud_zfil

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_flat_gouraud_zuse

#define PI_SCANFUNC csScan_8_draw_pi_scanline_flat_gouraud_zuse
#define PI_ZUSE
#define PI_FLAT
#define PI_GOURAUD
#define PI_INDEX8
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_flat_gouraud_zuse

//------------------------------------------------------------------

#ifndef NO_draw_pifx_scanline_zfil

#define PI_SCANFUNC csScan_8_draw_pifx_scanline_zfil
#define PI_ZFILL
#define PI_GOURAUD
#define PI_INDEX8
#define PI_BLEND
#include "scanpi.inc"

#endif // NO_draw_pifx_scanline_zfil

//------------------------------------------------------------------

#ifndef NO_draw_pifx_scanline_zuse

#define PI_SCANFUNC csScan_8_draw_pifx_scanline_zuse
#define PI_ZUSE
#define PI_GOURAUD
#define PI_INDEX8
#define PI_BLEND
#include "scanpi.inc"

#endif // NO_draw_pifx_scanline_zuse

//------------------------------------------------------------------

#ifndef NO_draw_pifx_scanline_transp_zfil

#define PI_SCANFUNC csScan_8_draw_pifx_scanline_transp_zfil
#define PI_ZFILL
#define PI_GOURAUD
#define PI_INDEX8
#define PI_COLORKEY
#define PI_BLEND
#include "scanpi.inc"

#endif // NO_draw_pifx_scanline_transp_zfil

//------------------------------------------------------------------

#ifndef NO_draw_pifx_scanline_transp_zuse

#define PI_SCANFUNC csScan_8_draw_pifx_scanline_transp_zuse
#define PI_ZUSE
#define PI_GOURAUD
#define PI_INDEX8
#define PI_COLORKEY
#define PI_BLEND
#include "scanpi.inc"

#endif // NO_draw_pifx_scanline_transp_zuse

//------------------------------------------------------------------
