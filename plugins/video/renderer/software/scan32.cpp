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

#include "sttest.h"

#define SCAN32
#define COLORMAP	((ULong *)Scan.PaletteTable)

//--//--//--//--//--//--//--//--//--//--//--/ assembler implementations --//--//

#if !defined(NO_ASSEMBLER)
#  if defined(PROC_INTEL) && defined (DO_NASM)
#    include "i386/scan32a.h"
#  endif //PROC_INTEL
#endif //!NO_ASSEMBLER

//--//--//--//--//--//--//--//--//--//--//--//--//-- draw_scanline_XXXX --//--//

#include "scanxx.inc"

//------------------------------------------------------------------

#ifndef NO_draw_scanline_fog

void csScan_32_draw_scanline_fog (int xx, unsigned char* d,
  unsigned long* z_buf, float inv_z, float u_div_z, float v_div_z)
{
  if (xx <= 0) return;
  (void)u_div_z; (void)v_div_z;
  ULong* _dest = (ULong*)d;
  ULong* _destend = _dest + xx;
  unsigned long izz = QInt24 (inv_z);
  int dzz = QInt24 (Scan.M);
  ULong fog_pix = Scan.FogR | Scan.FogG | Scan.FogB;
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
        register int r = (fd * ((*_dest & 0x00ff0000) - Scan.FogR) >> 8) + Scan.FogR;
        register int g = (fd * ((*_dest & 0x0000ff00) - Scan.FogG) >> 8) + Scan.FogG;
        register int b = (fd * ((*_dest & 0x000000ff) - Scan.FogB) >> 8) + Scan.FogB;
        *_dest = (r & 0x00ff0000) | (g & 0x0000ff00) | b;
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

#endif // NO_draw_scanline_fog

//------------------------------------------------------------------

#ifndef NO_draw_scanline_fog_view

void csScan_32_draw_scanline_fog_view (int xx, unsigned char* d,
  unsigned long* z_buf, float inv_z, float u_div_z, float v_div_z)
{
  if (xx <= 0) return;
  (void)u_div_z; (void)v_div_z; (void)inv_z;
  ULong* _dest = (ULong*)d;
  ULong* _destend = _dest + xx;
  ULong fog_pix = Scan.FogR | Scan.FogG | Scan.FogB;
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
        register int r = (fd * ((*_dest & 0x00ff0000) - Scan.FogR) >> 8) + Scan.FogR;
        register int g = (fd * ((*_dest & 0x0000ff00) - Scan.FogG) >> 8) + Scan.FogG;
        register int b = (fd * ((*_dest & 0x000000ff) - Scan.FogB) >> 8) + Scan.FogB;
        *_dest = (r & 0x00ff0000) | (g & 0x0000ff00) | b;
      }
      else
        *_dest = fog_pix;
    } /* endif */
    _dest++;
    z_buf++;
  }
  while (_dest < _destend);
}

#endif // NO_draw_scanline_fog_view

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_alpha50

#define SCANFUNC csScan_32_draw_scanline_map_alpha50
#define SCANMAP
#define SCANLOOP \
    do									\
    {									\
      ULong a = *_dest;							\
      ULong b = srcTex [((vv >> 16) << shifter) + (uu >> 16)];		\
      *_dest++ = ((a & 0xfefefefe) >> 1) + ((b & 0xfefefefe) >> 1);	\
      uu += duu;							\
      vv += dvv;							\
    }									\
    while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_draw_scanline_map_alpha50

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_alpha

// Note that we don't actually need separate procedures for RGB and BGR
// encodings: the cause is that R,G and B variables below are named so
// not because they will always contain respective values, but just
// for cleaner understanding how the procedure works.
#define SCANFUNC csScan_32_draw_scanline_map_alpha
#define SCANMAP
#define SCANLOOP \
    do									\
    {									\
      ULong tex = srcTex [((vv >> 16) << shifter) + (uu >> 16)];	\
      int tr = *_dest & 0xff0000;					\
      int tg = *_dest & 0x00ff00;					\
      int tb = *_dest & 0x0000ff;					\
      int r = (Scan.AlphaFact * ((tex & 0xff0000) - tr) >> 8) + tr;	\
      int g = (Scan.AlphaFact * ((tex & 0x00ff00) - tg) >> 8) + tg;	\
      int b = (Scan.AlphaFact * ((tex & 0x0000ff) - tb) >> 8) + tb;	\
      *_dest++ = (r & 0xff0000) | (g & 0x00ff00) | b;			\
      uu += duu;							\
      vv += dvv;							\
    }									\
    while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_draw_scanline_map_alpha

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_tex_zfil

#define PI_SCANFUNC csScan_32_draw_pi_scanline_tex_zfil
#define PI_ZFILL
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_tex_zfil

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_tex_zuse

#define PI_SCANFUNC csScan_32_draw_pi_scanline_tex_zuse
#define PI_ZUSE
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_tex_zuse

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_tex_gouraud_zfil

#define PI_SCANFUNC csScan_32_draw_pi_scanline_tex_gouraud_zfil
#define PI_ZFILL
#define PI_GOURAUD
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_tex_gouraud_zfil

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_tex_gouraud_zuse

#define PI_SCANFUNC csScan_32_draw_pi_scanline_tex_gouraud_zuse
#define PI_ZUSE
#define PI_GOURAUD
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_tex_gouraud_zuse

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_flat_gouraud_zfil

#define PI_SCANFUNC csScan_32_draw_pi_scanline_flat_gouraud_zfil
#define PI_FLAT
#define PI_ZFILL
#define PI_GOURAUD
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_flat_gouraud_zfil

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_flat_gouraud_zuse

#define PI_SCANFUNC csScan_32_draw_pi_scanline_flat_gouraud_zuse
#define PI_FLAT
#define PI_ZUSE
#define PI_GOURAUD
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_flat_gouraud_zuse

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_flat_zfil

#define PI_SCANFUNC csScan_32_draw_pi_scanline_flat_zfil
#define PI_FLAT
#define PI_ZFILL
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_flat_zfil

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_flat_zuse

#define PI_SCANFUNC csScan_32_draw_pi_scanline_flat_zuse
#define PI_FLAT
#define PI_ZUSE
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_draw_pi_scanline_flat_zuse

//------------------------------------------------------------------

#ifndef NO_draw_pifx_scanline_zfil

#define PI_SCANFUNC csScan_32_draw_pifx_scanline_zfil
#define PI_ZFILL
#define PI_GOURAUD
#define PI_R8G8B8
#define PI_BLEND
#include "scanpi.inc"

#endif

//------------------------------------------------------------------

#ifndef NO_draw_pifx_scanline_zuse

#define PI_SCANFUNC csScan_32_draw_pifx_scanline_zuse
#define PI_ZUSE
#define PI_GOURAUD
#define PI_R8G8B8
#define PI_BLEND
#include "scanpi.inc"

#endif

//------------------------------------------------------------------

#ifndef NO_draw_pifx_scanline_transp_zfil

#define PI_SCANFUNC csScan_32_draw_pifx_scanline_transp_zfil
#define PI_ZFILL
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_R8G8B8
#define PI_BLEND
#include "scanpi.inc"

#endif

//------------------------------------------------------------------

#ifndef NO_draw_pifx_scanline_transp_zuse

#define PI_SCANFUNC csScan_32_draw_pifx_scanline_transp_zuse
#define PI_ZUSE
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_R8G8B8
#define PI_BLEND
#include "scanpi.inc"

#endif

//------------------------------------------------------------------
