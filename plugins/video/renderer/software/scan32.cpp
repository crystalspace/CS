/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

#include "cssysdef.h"
#include "qint.h"
#include "scan.h"

#include "sttest.h"

#define SCAN32
#define COLORMAP	((ULong *)Scan.PaletteTable)

//--//--//--//--//--//--//--//--//--//--//--/ assembler implementations --//--//

#if defined (PROC_X86) && defined (DO_NASM)
#  include "i386/scan32a.h"
#endif // PROC_X86

//--//--//--//--//--//--//--//--//--//--//--//--//--//--//--/ scan_XXXX --//--//

#include "scanxx.inc"

//------------------------------------------------------------------

#ifndef NO_scan_fog

void csScan_32_scan_fog (int xx, unsigned char* d,
  unsigned long* z_buf, float inv_z, float u_div_z, float v_div_z)
{
  if (xx <= 0) return;
  (void)u_div_z; (void)v_div_z;
  ULong* _dest = (ULong*)d;
  ULong* _destend = _dest + xx;
  unsigned long izz = QInt24 (inv_z);
  int dzz = QInt24 (Scan.M);
  ULong fog_pix = R8G8B8_PIXEL_POSTPROC(Scan.FogR | Scan.FogG | Scan.FogB);
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
        ULong pix = R8G8B8_PIXEL_PREPROC (*_dest);
        register ULong r = (fd * ((pix & 0x00ff0000) - Scan.FogR) >> 8) + Scan.FogR;
        register ULong g = (fd * ((pix & 0x0000ff00) - Scan.FogG) >> 8) + Scan.FogG;
        register ULong b = (fd * ((pix & 0x000000ff) - Scan.FogB) >> 8) + Scan.FogB;
        *_dest = R8G8B8_PIXEL_POSTPROC ((r & 0x00ff0000) | (g & 0x0000ff00) | (b & 0x000000ff));
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

#endif // NO_scan_fog

//------------------------------------------------------------------

#ifndef NO_scan_fog_view

void csScan_32_scan_fog_view (int xx, unsigned char* d,
  unsigned long* z_buf, float inv_z, float u_div_z, float v_div_z)
{
  if (xx <= 0) return;
  (void)u_div_z; (void)v_div_z; (void)inv_z;
  ULong* _dest = (ULong*)d;
  ULong* _destend = _dest + xx;
  ULong fog_pix = R8G8B8_PIXEL_POSTPROC(Scan.FogR | Scan.FogG | Scan.FogB);
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
        ULong pix = R8G8B8_PIXEL_PREPROC (*_dest);
        register ULong r = (fd * ((pix & 0x00ff0000) - Scan.FogR) >> 8) + Scan.FogR;
        register ULong g = (fd * ((pix & 0x0000ff00) - Scan.FogG) >> 8) + Scan.FogG;
        register ULong b = (fd * ((pix & 0x000000ff) - Scan.FogB) >> 8) + Scan.FogB;
        *_dest = R8G8B8_PIXEL_POSTPROC ((r & 0x00ff0000) | (g & 0x0000ff00) | (b & 0x000000ff));
      }
      else
        *_dest = fog_pix;
    } /* endif */
    _dest++;
    z_buf++;
  }
  while (_dest < _destend);
}

#endif // NO_scan_fog_view

//------------------------------------------------------------------

#ifndef NO_scan_map_fixalpha50

#define SCANFUNC csScan_32_scan_map_fixalpha50
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

#endif // NO_scan_map_fixalpha50

//------------------------------------------------------------------

#ifndef NO_scan_map_fixalpha

// Note that we don't actually need separate procedures for RGB and BGR
// encodings: the cause is that R,G and B variables below are named so
// not because they will always contain respective values, but just
// for cleaner understanding how the procedure works.
#define SCANFUNC csScan_32_scan_map_fixalpha
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

#endif // NO_scan_map_fixalpha

//------------------------------------------------------------------

#ifndef NO_scan_map_filt2_zfil

/*
    Strangely enough, but 16 interpolation steps are quite enough
    even for 32bpp modes... We also use here the technique used in
    16-bit modes (see scan16.cpp) and to my surprise I solved the
    puzzle with same 6 multiplies per pixel as in 16-bit modes (!)
*/

#define SCANFUNC csScan_32_scan_map_filt2_zfil
#define SCANMAP
#define SCANLOOP							\
    if ((duu > 0xffff) || (dvv > 0xffff))				\
      do								\
      {									\
        *_dest++ = srcTex [((vv >> 16) << shifter) + (uu >> 16)];	\
        uu += duu;							\
        vv += dvv;							\
      }									\
      while (_dest <= _destend);					\
    else								\
      do								\
      {									\
        unsigned addr = (((vv >> 16)) << shifter) + (uu >> 16);		\
        unsigned pl = R8G8B8_PIXEL_PREPROC (srcTex [addr]);		\
        unsigned pr = R8G8B8_PIXEL_PREPROC (srcTex [addr + 1]);		\
        unsigned c, u = (uu >> 12) & 0x0f;				\
        c = pl & 0x00ff00ff;						\
        unsigned rbt = (c << 4) + u * ((pr & 0x00ff00ff) - c);		\
        c = pl & 0x0000ff00;						\
        unsigned gt  = (c << 4) + u * ((pr & 0x0000ff00) - c);		\
        pl = R8G8B8_PIXEL_PREPROC (srcTex [addr + Scan.tw2]);		\
        pr = R8G8B8_PIXEL_PREPROC (srcTex [addr + Scan.tw2 + 1]);	\
        c = pl & 0x00ff00ff;						\
        unsigned rbb = (c << 4) + u * ((pr & 0x00ff00ff) - c);		\
        c = pl & 0x0000ff00;						\
        unsigned gb  = (c << 4) + u * ((pr & 0x0000ff00) - c);		\
        unsigned v = (vv >> 12) & 0x0f;					\
        rbb = (((rbt << 4) + v * (rbb - rbt)) >> 8) & 0x00ff00ff;	\
        gb  = (((gt << 4)  + v * (gb  - gt )) >> 8) & 0x0000ff00;	\
        *_dest++ = R8G8B8_PIXEL_POSTPROC (rbb | gb);			\
        uu += duu;							\
        vv += dvv;							\
      }									\
      while (_dest <= _destend);

#define SCANEND \
    do									\
    {									\
      *z_buffer++ = izz;						\
      izz += dzz;							\
    }									\
    while (z_buffer <= lastZbuf)
#include "scanln.inc"

#endif // NO_scan_map_filt2_zfil

//------------------------------------------------------------------

#ifndef NO_scan_map_filt2_zuse

#define SCANFUNC csScan_32_scan_map_filt2_zuse
#define SCANMAP
#define SCANLOOP							\
    if ((duu > 0xffff) || (dvv > 0xffff))				\
      do								\
      {									\
        if (izz >= *z_buffer)						\
        {								\
          *z_buffer = izz;						\
          *_dest = srcTex [((vv >> 16) << shifter) + (uu >> 16)];	\
        }								\
        _dest++;							\
        z_buffer++;							\
        uu += duu;							\
        vv += dvv;							\
        izz += dzz;							\
      }									\
      while (_dest <= _destend);					\
    else								\
      do								\
      {									\
        if (izz >= *z_buffer)						\
        {								\
          *z_buffer = izz;						\
          unsigned addr = (((vv >> 16)) << shifter) + (uu >> 16);	\
          unsigned pl = R8G8B8_PIXEL_PREPROC (srcTex [addr]);		\
          unsigned pr = R8G8B8_PIXEL_PREPROC (srcTex [addr + 1]);	\
          unsigned c, u = (uu >> 12) & 0x0f;				\
          c = pl & 0x00ff00ff;						\
          unsigned rbt = (c << 4) + u * ((pr & 0x00ff00ff) - c);	\
          c = pl & 0x0000ff00;						\
          unsigned gt  = (c << 4) + u * ((pr & 0x0000ff00) - c);	\
          pl = R8G8B8_PIXEL_PREPROC (srcTex [addr + Scan.tw2]);		\
          pr = R8G8B8_PIXEL_PREPROC (srcTex [addr + Scan.tw2 + 1]);	\
          c = pl & 0x00ff00ff;						\
          unsigned rbb = (c << 4) + u * ((pr & 0x00ff00ff) - c);	\
          c = pl & 0x0000ff00;						\
          unsigned gb  = (c << 4) + u * ((pr & 0x0000ff00) - c);	\
          unsigned v = (vv >> 12) & 0x0f;				\
          rbb = (((rbt << 4) + v * (rbb - rbt)) >> 8) & 0x00ff00ff;	\
          gb  = (((gt << 4)  + v * (gb  - gt )) >> 8) & 0x0000ff00;	\
          *_dest = R8G8B8_PIXEL_POSTPROC (rbb | gb);			\
        }								\
        _dest++;							\
        z_buffer++;							\
        uu += duu;							\
        vv += dvv;							\
        izz += dzz;							\
      }									\
      while (_dest <= _destend);
#include "scanln.inc"

#endif // NO_scan_map_filt2_zuse

//------------------------------------------------------------------

#ifndef NO_scan_map_alpha_znone

#define A_SCANFUNC csScan_32_scan_map_alpha_znone
#define A_ZNONE
#define A_MAP
#define A_R8G8B8
#include "scanalph.inc"

#endif // NO_scan_map_alpha_znone

//------------------------------------------------------------------

#ifndef NO_scan_map_alpha_zfil

#define A_SCANFUNC csScan_32_scan_map_alpha_zfil
#define A_ZFIL
#define A_MAP
#define A_R8G8B8
#include "scanalph.inc"

#endif // NO_scan_map_alpha_zfil

//------------------------------------------------------------------

#ifndef NO_scan_map_alpha_zuse

#define A_SCANFUNC csScan_32_scan_map_alpha_zuse
#define A_ZUSE
#define A_MAP
#define A_R8G8B8
#include "scanalph.inc"

#endif // NO_scan_map_alpha_zuse

//------------------------------------------------------------------

#ifndef NO_scan_map_alpha_ztest

#define A_SCANFUNC csScan_32_scan_map_alpha_ztest
#define A_ZTEST
#define A_MAP
#define A_R8G8B8
#include "scanalph.inc"

#endif // NO_scan_map_alpha_ztest

//------------------------------------------------------------------

#ifndef NO_scan_tex_alpha_znone

#define A_SCANFUNC csScan_32_scan_tex_alpha_znone
#define A_ZNONE
#define A_R8G8B8
#include "scanalph.inc"

#endif // NO_scan_tex_alpha_znone

//------------------------------------------------------------------

#ifndef NO_scan_tex_alpha_zfil

#define A_SCANFUNC csScan_32_scan_tex_alpha_zfil
#define A_ZFIL
#define A_R8G8B8
#include "scanalph.inc"

#endif // NO_scan_tex_alpha_zfil

//------------------------------------------------------------------

#ifndef NO_scan_tex_alpha_zuse

#define A_SCANFUNC csScan_32_scan_tex_alpha_zuse
#define A_ZUSE
#define A_R8G8B8
#include "scanalph.inc"

#endif // NO_scan_tex_alpha_zuse

//------------------------------------------------------------------

#ifndef NO_scan_tex_alpha_ztest

#define A_SCANFUNC csScan_32_scan_tex_alpha_ztest
#define A_ZTEST
#define A_R8G8B8
#include "scanalph.inc"

#endif // NO_scan_tex_alpha_ztest

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_alpha_znone

#define A_SCANFUNC csScan_32_scan_pi_tex_alpha_znone
#define A_PI
#define A_R8G8B8
#include "scanalph.inc"

#endif // NO_scan_pi_tex_alpha_znone

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_alpha_zfil

#define A_SCANFUNC csScan_32_scan_pi_tex_alpha_zfil
#define A_PI
#define A_R8G8B8
#define A_ZFIL
#include "scanalph.inc"

#endif // NO_scan_pi_tex_alpha_zfil

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_alpha_zuse

#define A_SCANFUNC csScan_32_scan_pi_tex_alpha_zuse
#define A_PI
#define A_R8G8B8
#define A_ZUSE
#include "scanalph.inc"

#endif // NO_scan_pi_tex_alpha_zuse

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_alpha_ztest

#define A_SCANFUNC csScan_32_scan_pi_tex_alpha_ztest
#define A_PI
#define A_R8G8B8
#define A_ZTEST
#include "scanalph.inc"

#endif // NO_scan_pi_tex_alpha_ztest

//------------------------------------------------------------------

#ifndef NO_scan_pi_flat_znone

#define PI_SCANFUNC csScan_32_scan_pi_flat_znone
#define PI_FLAT
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_flat_znone

//------------------------------------------------------------------

#ifndef NO_scan_pi_flat_zfil

#define PI_SCANFUNC csScan_32_scan_pi_flat_zfil
#define PI_ZFILL
#define PI_FLAT
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_flat_zfil

//------------------------------------------------------------------

#ifndef NO_scan_pi_flat_zuse

#define PI_SCANFUNC csScan_32_scan_pi_flat_zuse
#define PI_ZUSE
#define PI_FLAT
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_flat_zuse

//------------------------------------------------------------------

#ifndef NO_scan_pi_flat_ztest

#define PI_SCANFUNC csScan_32_scan_pi_flat_ztest
#define PI_ZTEST
#define PI_FLAT
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_flat_ztest

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_znone

#define PI_SCANFUNC csScan_32_scan_pi_tex_znone
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_znone

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_zfil

#define PI_SCANFUNC csScan_32_scan_pi_tex_zfil
#define PI_ZFILL
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_zfil

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_zuse

#define PI_SCANFUNC csScan_32_scan_pi_tex_zuse
#define PI_ZUSE
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_zuse

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_ztest

#define PI_SCANFUNC csScan_32_scan_pi_tex_ztest
#define PI_ZTEST
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_ztest

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_key_znone

#define PI_SCANFUNC csScan_32_scan_pi_tex_key_znone
#define PI_COLORKEY
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_key_znone

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_key_zfil

#define PI_SCANFUNC csScan_32_scan_pi_tex_key_zfil
#define PI_ZFILL
#define PI_COLORKEY
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_key_zfil

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_key_zuse

#define PI_SCANFUNC csScan_32_scan_pi_tex_key_zuse
#define PI_ZUSE
#define PI_COLORKEY
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_key_zuse

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_key_ztest

#define PI_SCANFUNC csScan_32_scan_pi_tex_key_ztest
#define PI_ZTEST
#define PI_COLORKEY
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_key_ztest

//------------------------------------------------------------------

#ifndef NO_scan_pi_flat_fx_znone

#define PI_SCANFUNC csScan_32_scan_pi_flat_fx_znone
#define PI_FLAT
#define PI_BLEND
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_flat_fx_znone

//------------------------------------------------------------------

#ifndef NO_scan_pi_flat_fx_zfil

#define PI_SCANFUNC csScan_32_scan_pi_flat_fx_zfil
#define PI_ZFILL
#define PI_FLAT
#define PI_BLEND
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_flat_fx_zfil

//------------------------------------------------------------------

#ifndef NO_scan_pi_flat_fx_zuse

#define PI_SCANFUNC csScan_32_scan_pi_flat_fx_zuse
#define PI_ZUSE
#define PI_FLAT
#define PI_BLEND
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_flat_fx_zuse

//------------------------------------------------------------------

#ifndef NO_scan_pi_flat_fx_ztest

#define PI_SCANFUNC csScan_32_scan_pi_flat_fx_ztest
#define PI_ZTEST
#define PI_FLAT
#define PI_BLEND
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_flat_fx_ztest

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_fx_znone

#define PI_SCANFUNC csScan_32_scan_pi_tex_fx_znone
#define PI_BLEND
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_fx_znone

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_fx_zfil

#define PI_SCANFUNC csScan_32_scan_pi_tex_fx_zfil
#define PI_ZFILL
#define PI_BLEND
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_fx_zfil

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_fx_zuse

#define PI_SCANFUNC csScan_32_scan_pi_tex_fx_zuse
#define PI_ZUSE
#define PI_BLEND
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_fx_zuse

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_fx_ztest

#define PI_SCANFUNC csScan_32_scan_pi_tex_fx_ztest
#define PI_ZTEST
#define PI_BLEND
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_fx_ztest

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_fxkey_znone

#define PI_SCANFUNC csScan_32_scan_pi_tex_fxkey_znone
#define PI_COLORKEY
#define PI_BLEND
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_fxkey_znone

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_fxkey_zfil

#define PI_SCANFUNC csScan_32_scan_pi_tex_fxkey_zfil
#define PI_ZFILL
#define PI_COLORKEY
#define PI_BLEND
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_fxkey_zfil

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_fxkey_zuse

#define PI_SCANFUNC csScan_32_scan_pi_tex_fxkey_zuse
#define PI_ZUSE
#define PI_COLORKEY
#define PI_BLEND
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_fxkey_zuse

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_fxkey_ztest

#define PI_SCANFUNC csScan_32_scan_pi_tex_fxkey_ztest
#define PI_ZTEST
#define PI_COLORKEY
#define PI_BLEND
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_fxkey_ztest

//------------------------------------------------------------------

#ifndef NO_scan_pi_flat_gou_znone

#define PI_SCANFUNC csScan_32_scan_pi_flat_gou_znone
#define PI_FLAT
#define PI_GOURAUD
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_flat_gou_znone

//------------------------------------------------------------------

#ifndef NO_scan_pi_flat_gou_zfil

#define PI_SCANFUNC csScan_32_scan_pi_flat_gou_zfil
#define PI_ZFILL
#define PI_FLAT
#define PI_GOURAUD
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_flat_gou_zfil

//------------------------------------------------------------------

#ifndef NO_scan_pi_flat_gou_zuse

#define PI_SCANFUNC csScan_32_scan_pi_flat_gou_zuse
#define PI_ZUSE
#define PI_FLAT
#define PI_GOURAUD
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_flat_gou_zuse

//------------------------------------------------------------------

#ifndef NO_scan_pi_flat_gou_ztest

#define PI_SCANFUNC csScan_32_scan_pi_flat_gou_ztest
#define PI_ZTEST
#define PI_FLAT
#define PI_GOURAUD
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_flat_gou_ztest

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_gou_znone

#define PI_SCANFUNC csScan_32_scan_pi_tex_gou_znone
#define PI_GOURAUD
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_gou_znone

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_gou_zfil

#define PI_SCANFUNC csScan_32_scan_pi_tex_gou_zfil
#define PI_ZFILL
#define PI_GOURAUD
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_gou_zfil

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_gou_zuse

#define PI_SCANFUNC csScan_32_scan_pi_tex_gou_zuse
#define PI_ZUSE
#define PI_GOURAUD
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_gou_zuse

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_gou_ztest

#define PI_SCANFUNC csScan_32_scan_pi_tex_gou_ztest
#define PI_ZTEST
#define PI_GOURAUD
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_gou_ztest

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_goukey_znone

#define PI_SCANFUNC csScan_32_scan_pi_tex_goukey_znone
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_goukey_znone

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_goukey_zfil

#define PI_SCANFUNC csScan_32_scan_pi_tex_goukey_zfil
#define PI_ZFILL
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_goukey_zfil

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_goukey_zuse

#define PI_SCANFUNC csScan_32_scan_pi_tex_goukey_zuse
#define PI_ZUSE
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_goukey_zuse

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_goukey_ztest

#define PI_SCANFUNC csScan_32_scan_pi_tex_goukey_ztest
#define PI_ZTEST
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_goukey_ztest

//------------------------------------------------------------------

#ifndef NO_scan_pi_flat_goufx_znone

#define PI_SCANFUNC csScan_32_scan_pi_flat_goufx_znone
#define PI_FLAT
#define PI_GOURAUD
#define PI_BLEND
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_flat_goufx_znone

//------------------------------------------------------------------

#ifndef NO_scan_pi_flat_goufx_zfil

#define PI_SCANFUNC csScan_32_scan_pi_flat_goufx_zfil
#define PI_FLAT
#define PI_ZFILL
#define PI_GOURAUD
#define PI_BLEND
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_flat_goufx_zfil

//------------------------------------------------------------------

#ifndef NO_scan_pi_flat_goufx_zuse

#define PI_SCANFUNC csScan_32_scan_pi_flat_goufx_zuse
#define PI_FLAT
#define PI_ZUSE
#define PI_GOURAUD
#define PI_BLEND
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_flat_goufx_zuse

//------------------------------------------------------------------

#ifndef NO_scan_pi_flat_goufx_ztest

#define PI_SCANFUNC csScan_32_scan_pi_flat_goufx_ztest
#define PI_FLAT
#define PI_ZTEST
#define PI_GOURAUD
#define PI_BLEND
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_flat_goufx_ztest

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_goufx_znone

#define PI_SCANFUNC csScan_32_scan_pi_tex_goufx_znone
#define PI_GOURAUD
#define PI_BLEND
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_goufx_znone

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_goufx_zfil

#define PI_SCANFUNC csScan_32_scan_pi_tex_goufx_zfil
#define PI_ZFILL
#define PI_GOURAUD
#define PI_BLEND
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_goufx_zfil

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_goufx_zuse

#define PI_SCANFUNC csScan_32_scan_pi_tex_goufx_zuse
#define PI_ZUSE
#define PI_GOURAUD
#define PI_BLEND
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_goufx_zuse

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_goufx_ztest

#define PI_SCANFUNC csScan_32_scan_pi_tex_goufx_ztest
#define PI_ZTEST
#define PI_GOURAUD
#define PI_BLEND
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_goufx_ztest

//------------------------------------------------------------------


#ifndef NO_scan_pi_tex_goufxkey_znone

#define PI_SCANFUNC csScan_32_scan_pi_tex_goufxkey_znone
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_BLEND
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_goufxkey_znone

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_goufxkey_zfil

#define PI_SCANFUNC csScan_32_scan_pi_tex_goufxkey_zfil
#define PI_ZFILL
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_BLEND
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_goufxkey_zfil

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_goufxkey_zuse

#define PI_SCANFUNC csScan_32_scan_pi_tex_goufxkey_zuse
#define PI_ZUSE
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_BLEND
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_goufxkey_zuse

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_goufxkey_ztest

#define PI_SCANFUNC csScan_32_scan_pi_tex_goufxkey_ztest
#define PI_ZTEST
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_BLEND
#define PI_R8G8B8
#include "scanpi.inc"

#endif // NO_scan_pi_tex_goufxkey_ztest

//------------------------------------------------------------------
