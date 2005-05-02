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
#include "csqint.h"
#include "scan.h"
#include "soft_g3d.h"

#include "sttest.h"

#define SCAN16
#define COLORMAP	((uint16 *)Scan.PaletteTable)

//--//--//--//--//--//--//--//--//--//--//--/ assembler implementations --//--//

#if defined (CS_PROCESSOR_X86) && (CS_PROCESSOR_SIZE == 32) && defined (CS_HAVE_NASM)
#  include "i386/scan16a.h"
#endif // CS_PROCESSOR_X86

#if defined (CS_LITTLE_ENDIAN)
#  define LEFT(x)	(x & 0xffff)
#  define RIGHT(x)	(x >> 16)
#elif defined (CS_BIG_ENDIAN)
#  define LEFT(x)	(x >> 16)
#  define RIGHT(x)	(x & 0xffff)
#endif

//--//--//--//--//--//--//--//--//--//--//--//--//--//--/ scan_XXXX --//--//--//

#include "scanxx.inc"

#define PI_R5G6B5
#include "scanxxfx.inc"
#undef  PI_R5G6B5
#define PI_R5G5B5
#include "scanxxfx.inc"
#undef  PI_R5G5B5

//----------------------------------------------------------------------------//

#ifndef NO_scan_map_filt_znone

#define SCANFUNC csScan_16_scan_map_filt_znone
#define SCANMAP
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
      if ((((int32)(intptr_t)_dest) & csSoftwareGraphics3DCommon::filter_bf) != 0) \
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
    }									\
  uu = uu1;								\
  vv = vv1;
#include "scanln.inc"

#endif // NO_scan_map_filt_znone

//------------------------------------------------------------------

#ifndef NO_565_scan_map_filt2_znone

/*
  We process two pixels at a time (in a single CPU register):
     +--------+---------+    +--------+---------+
  T: |top-left|top-right| B: |bot-left|bot-right|
     +--------+---------+    +--------+---------+
  We isolate the r/g/b components in both pixels at once and multiply
  the double word with the V fractional value. Then we combine the left
  and right halves. We use 16 gradations for interpolation, which is
  quite enough for 16-bit modes.
  Using one reg has a drawback: the subtraction operation do not perform
  independent subtractions from both components; the least significant
  bit of left pixel component could be garbled by right pixel's influence.
*/

#define SCANFUNC csScan_16_565_scan_map_filt2_znone
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
        unsigned top = *(unsigned *)&srcTex [addr];			\
        unsigned bot = *(unsigned *)&srcTex [addr + Scan.tw2];		\
        unsigned c, v = (vv >> 12) & 0x0f;				\
        c = (top & 0x001f001f);						\
        unsigned b = (c << 4) + v * ((bot & 0x001f001f) - c);		\
        /* b == <7:0> <5.4:bl> <7:0> <5.4:br> */			\
        c = (top & 0x07e007e0);						\
        unsigned g = c + (int (v * ((bot & 0x07e007e0) - c)) >> 4);	\
        /* g == <6:0> <6.4:gl> <6:0> <6.4:gr> */			\
        c = (top & 0xf800f800) >> 8;					\
        unsigned r = (c << 4) + v * (((bot & 0xf800f800) >> 8) - c);	\
        /* r == <4:0> <5.7:rl> <4:0> <5.7:rr> */			\
        unsigned u = (uu >> 12) & 0x0f;					\
        c = LEFT (b);							\
        b = ((c << 4) + u * (RIGHT (b) - c) + 0x0080) >> 8;		\
        c = LEFT (g);							\
        g = (c + ((u * (RIGHT (g) - c) + 0x0100) >> 4)) & 0x07e0;	\
        c = LEFT (r);							\
        r = (((c << 4) + (u * (RIGHT (r) - c)) + 0x0400) & 0xf800);	\
        *_dest++ = r | g | b;						\
        uu += duu;							\
        vv += dvv;							\
      }									\
      while (_dest <= _destend);
#include "scanln.inc"

#endif // NO_565_scan_map_filt2_znone

//------------------------------------------------------------------

#ifndef NO_555_scan_map_filt2_znone

#define SCANFUNC csScan_16_555_scan_map_filt2_znone
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
        unsigned top = *(unsigned *)&srcTex [addr];			\
        unsigned bot = *(unsigned *)&srcTex [addr + Scan.tw2];		\
        unsigned c, v = (vv >> 12) & 0x0f;				\
        c = (top & 0x001f001f);						\
        unsigned b = (c << 4) + v * ((bot & 0x001f001f) - c);		\
        /* b == <7:0> <5.4:bl> <7:0> <5.4:br> */			\
        c = (top & 0x03e003e0);						\
        unsigned g = c + (int (v * ((bot & 0x03e003e0) - c)) >> 4);	\
        /* g == <7:0> <5.4:gl> <7:0> <5.4:gr> */			\
        c = (top & 0x7c007c00) >> 8;					\
        unsigned r = (c << 4) + v * (((bot & 0x7c007c00) >> 8) - c);	\
        /* r == <4:0> <5.7:rl> <4:0> <5.7:rr> */			\
        unsigned u = (uu >> 12) & 0x0f;					\
        c = LEFT (b);							\
        b = ((c << 4) + u * (RIGHT (b) - c) + 0x0080) >> 8;		\
        c = LEFT (g);							\
        g = (c + ((u * (RIGHT (g) - c) + 0x0100) >> 4)) & 0x03e0;	\
        c = LEFT (r);							\
        r = (((c << 4) + (u * (RIGHT (r) - c)) + 0x0200) & 0x7c00);	\
        *_dest++ = r | g | b;						\
        uu += duu;							\
        vv += dvv;							\
      }									\
      while (_dest <= _destend);
#include "scanln.inc"

#endif // NO_555_scan_map_filt2_znone

//------------------------------------------------------------------

#ifndef NO_565_scan_map_filt2_ztest

#define SCANFUNC csScan_16_565_scan_map_filt2_ztest
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
        unsigned addr = (((vv >> 16)) << shifter) + (uu >> 16);		\
        unsigned top = *(unsigned *)&srcTex [addr];			\
        unsigned bot = *(unsigned *)&srcTex [addr + Scan.tw2];		\
        unsigned c, v = (vv >> 12) & 0x0f;				\
        c = (top & 0x001f001f);						\
        unsigned b = (c << 4) + v * ((bot & 0x001f001f) - c);		\
        /* b == <7:0> <5.4:bl> <7:0> <5.4:br> */			\
        c = (top & 0x07e007e0);						\
        unsigned g = c + (int (v * ((bot & 0x07e007e0) - c)) >> 4);	\
        /* g == <6:0> <6.4:gl> <6:0> <6.4:gr> */			\
        c = (top & 0xf800f800) >> 8;					\
        unsigned r = (c << 4) + v * (((bot & 0xf800f800) >> 8) - c);	\
        /* r == <4:0> <5.7:rl> <4:0> <5.7:rr> */			\
        unsigned u = (uu >> 12) & 0x0f;					\
        c = LEFT (b);							\
        b = ((c << 4) + u * (RIGHT (b) - c) + 0x0080) >> 8;		\
        c = LEFT (g);							\
        g = (c + ((u * (RIGHT (g) - c) + 0x0100) >> 4)) & 0x07e0;	\
        c = LEFT (r);							\
        r = (((c << 4) + (u * (RIGHT (r) - c)) + 0x0400) & 0xf800);	\
        *_dest = r | g | b;						\
        }								\
        _dest++;							\
        z_buffer++;							\
        uu += duu;							\
        vv += dvv;							\
        izz += dzz;							\
      }									\
      while (_dest <= _destend);
#include "scanln.inc"

#endif // NO_565_scan_map_filt2_ztest

//------------------------------------------------------------------

#ifndef NO_555_scan_map_filt2_ztest

#define SCANFUNC csScan_16_555_scan_map_filt2_ztest
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
        unsigned addr = (((vv >> 16)) << shifter) + (uu >> 16);		\
        unsigned top = *(unsigned *)&srcTex [addr];			\
        unsigned bot = *(unsigned *)&srcTex [addr + Scan.tw2];		\
        unsigned c, v = (vv >> 12) & 0x0f;				\
        c = (top & 0x001f001f);						\
        unsigned b = (c << 4) + v * ((bot & 0x001f001f) - c);		\
        /* b == <7:0> <5.4:bl> <7:0> <5.4:br> */			\
        c = (top & 0x03e003e0);						\
        unsigned g = c + (int (v * ((bot & 0x03e003e0) - c)) >> 4);	\
        /* g == <7:0> <5.4:gl> <7:0> <5.4:gr> */			\
        c = (top & 0x7c007c00) >> 8;					\
        unsigned r = (c << 4) + v * (((bot & 0x7c007c00) >> 8) - c);	\
        /* r == <4:0> <5.7:rl> <4:0> <5.7:rr> */			\
        unsigned u = (uu >> 12) & 0x0f;					\
        c = LEFT (b);							\
        b = ((c << 4) + u * (RIGHT (b) - c) + 0x0080) >> 8;		\
        c = LEFT (g);							\
        g = (c + ((u * (RIGHT (g) - c) + 0x0100) >> 4)) & 0x03e0;	\
        c = LEFT (r);							\
        r = (((c << 4) + (u * (RIGHT (r) - c)) + 0x0200) & 0x7c00);	\
        *_dest = r | g | b;						\
        }								\
        _dest++;							\
        z_buffer++;							\
        uu += duu;							\
        vv += dvv;							\
        izz += dzz;							\
      }									\
      while (_dest <= _destend);
#include "scanln.inc"

#endif // NO_555_scan_map_filt2_ztest

//----------------------------------------------------------------------------//

#ifndef NO_scan_map_filt_zfil

#define SCANFUNC csScan_16_scan_map_filt_zfil
#define SCANMAP
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
      if ((((int32)(intptr_t)_dest) & csSoftwareGraphics3DCommon::filter_bf) != 0) \
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
    }									\
  uu = uu1;								\
  vv = vv1;

#define SCANEND \
    do									\
    {									\
      *z_buffer++ = izz;						\
      izz += dzz;							\
    }									\
    while (z_buffer <= lastZbuf)
#include "scanln.inc"

#endif // NO_scan_map_filt_zfil

//------------------------------------------------------------------

#ifndef NO_565_scan_map_filt2_zfil

/*
  We process two pixels at a time (in a single CPU register):
     +--------+---------+    +--------+---------+
  T: |top-left|top-right| B: |bot-left|bot-right|
     +--------+---------+    +--------+---------+
  We isolate the r/g/b components in both pixels at once and multiply
  the double word with the V fractional value. Then we combine the left
  and right halves. We use 16 gradations for interpolation, which is
  quite enough for 16-bit modes.
  Using one reg has a drawback: the subtraction operation do not perform
  independent subtractions from both components; the least significant
  bit of left pixel component could be garbled by right pixel's influence.
*/

#define SCANFUNC csScan_16_565_scan_map_filt2_zfil
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
        unsigned top = *(unsigned *)&srcTex [addr];			\
        unsigned bot = *(unsigned *)&srcTex [addr + Scan.tw2];		\
        unsigned c, v = (vv >> 12) & 0x0f;				\
        c = (top & 0x001f001f);						\
        unsigned b = (c << 4) + v * ((bot & 0x001f001f) - c);		\
        /* b == <7:0> <5.4:bl> <7:0> <5.4:br> */			\
        c = (top & 0x07e007e0);						\
        unsigned g = c + (int (v * ((bot & 0x07e007e0) - c)) >> 4);	\
        /* g == <6:0> <6.4:gl> <6:0> <6.4:gr> */			\
        c = (top & 0xf800f800) >> 8;					\
        unsigned r = (c << 4) + v * (((bot & 0xf800f800) >> 8) - c);	\
        /* r == <4:0> <5.7:rl> <4:0> <5.7:rr> */			\
        unsigned u = (uu >> 12) & 0x0f;					\
        c = LEFT (b);							\
        b = ((c << 4) + u * (RIGHT (b) - c) + 0x0080) >> 8;		\
        c = LEFT (g);							\
        g = (c + ((u * (RIGHT (g) - c) + 0x0100) >> 4)) & 0x07e0;	\
        c = LEFT (r);							\
        r = (((c << 4) + (u * (RIGHT (r) - c)) + 0x0400) & 0xf800);	\
        *_dest++ = r | g | b;						\
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

#endif // NO_565_scan_map_filt2_zfil

//------------------------------------------------------------------

#ifndef NO_555_scan_map_filt2_zfil

#define SCANFUNC csScan_16_555_scan_map_filt2_zfil
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
        unsigned top = *(unsigned *)&srcTex [addr];			\
        unsigned bot = *(unsigned *)&srcTex [addr + Scan.tw2];		\
        unsigned c, v = (vv >> 12) & 0x0f;				\
        c = (top & 0x001f001f);						\
        unsigned b = (c << 4) + v * ((bot & 0x001f001f) - c);		\
        /* b == <7:0> <5.4:bl> <7:0> <5.4:br> */			\
        c = (top & 0x03e003e0);						\
        unsigned g = c + (int (v * ((bot & 0x03e003e0) - c)) >> 4);	\
        /* g == <7:0> <5.4:gl> <7:0> <5.4:gr> */			\
        c = (top & 0x7c007c00) >> 8;					\
        unsigned r = (c << 4) + v * (((bot & 0x7c007c00) >> 8) - c);	\
        /* r == <4:0> <5.7:rl> <4:0> <5.7:rr> */			\
        unsigned u = (uu >> 12) & 0x0f;					\
        c = LEFT (b);							\
        b = ((c << 4) + u * (RIGHT (b) - c) + 0x0080) >> 8;		\
        c = LEFT (g);							\
        g = (c + ((u * (RIGHT (g) - c) + 0x0100) >> 4)) & 0x03e0;	\
        c = LEFT (r);							\
        r = (((c << 4) + (u * (RIGHT (r) - c)) + 0x0200) & 0x7c00);	\
        *_dest++ = r | g | b;						\
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

#endif // NO_555_scan_map_filt2_zfil

//------------------------------------------------------------------

#ifndef NO_565_scan_map_filt2_zuse

#define SCANFUNC csScan_16_565_scan_map_filt2_zuse
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
        unsigned addr = (((vv >> 16)) << shifter) + (uu >> 16);		\
        unsigned top = *(unsigned *)&srcTex [addr];			\
        unsigned bot = *(unsigned *)&srcTex [addr + Scan.tw2];		\
        unsigned c, v = (vv >> 12) & 0x0f;				\
        c = (top & 0x001f001f);						\
        unsigned b = (c << 4) + v * ((bot & 0x001f001f) - c);		\
        /* b == <7:0> <5.4:bl> <7:0> <5.4:br> */			\
        c = (top & 0x07e007e0);						\
        unsigned g = c + (int (v * ((bot & 0x07e007e0) - c)) >> 4);	\
        /* g == <6:0> <6.4:gl> <6:0> <6.4:gr> */			\
        c = (top & 0xf800f800) >> 8;					\
        unsigned r = (c << 4) + v * (((bot & 0xf800f800) >> 8) - c);	\
        /* r == <4:0> <5.7:rl> <4:0> <5.7:rr> */			\
        unsigned u = (uu >> 12) & 0x0f;					\
        c = LEFT (b);							\
        b = ((c << 4) + u * (RIGHT (b) - c) + 0x0080) >> 8;		\
        c = LEFT (g);							\
        g = (c + ((u * (RIGHT (g) - c) + 0x0100) >> 4)) & 0x07e0;	\
        c = LEFT (r);							\
        r = (((c << 4) + (u * (RIGHT (r) - c)) + 0x0400) & 0xf800);	\
        *_dest = r | g | b;						\
        }								\
        _dest++;							\
        z_buffer++;							\
        uu += duu;							\
        vv += dvv;							\
        izz += dzz;							\
      }									\
      while (_dest <= _destend);
#include "scanln.inc"

#endif // NO_565_scan_map_filt2_zuse

//------------------------------------------------------------------

#ifndef NO_555_scan_map_filt2_zuse

#define SCANFUNC csScan_16_555_scan_map_filt2_zuse
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
        unsigned addr = (((vv >> 16)) << shifter) + (uu >> 16);		\
        unsigned top = *(unsigned *)&srcTex [addr];			\
        unsigned bot = *(unsigned *)&srcTex [addr + Scan.tw2];		\
        unsigned c, v = (vv >> 12) & 0x0f;				\
        c = (top & 0x001f001f);						\
        unsigned b = (c << 4) + v * ((bot & 0x001f001f) - c);		\
        /* b == <7:0> <5.4:bl> <7:0> <5.4:br> */			\
        c = (top & 0x03e003e0);						\
        unsigned g = c + (int (v * ((bot & 0x03e003e0) - c)) >> 4);	\
        /* g == <7:0> <5.4:gl> <7:0> <5.4:gr> */			\
        c = (top & 0x7c007c00) >> 8;					\
        unsigned r = (c << 4) + v * (((bot & 0x7c007c00) >> 8) - c);	\
        /* r == <4:0> <5.7:rl> <4:0> <5.7:rr> */			\
        unsigned u = (uu >> 12) & 0x0f;					\
        c = LEFT (b);							\
        b = ((c << 4) + u * (RIGHT (b) - c) + 0x0080) >> 8;		\
        c = LEFT (g);							\
        g = (c + ((u * (RIGHT (g) - c) + 0x0100) >> 4)) & 0x03e0;	\
        c = LEFT (r);							\
        r = (((c << 4) + (u * (RIGHT (r) - c)) + 0x0200) & 0x7c00);	\
        *_dest = r | g | b;						\
        }								\
        _dest++;							\
        z_buffer++;							\
        uu += duu;							\
        vv += dvv;							\
        izz += dzz;							\
      }									\
      while (_dest <= _destend);
#include "scanln.inc"

#endif // NO_555_scan_map_filt2_zuse

//------------------------------------------------------------------

#ifndef NO_scan_map_fixalpha50

#define SCANFUNC csScan_16_scan_map_fixalpha50
#define SCANMAP
#define SCANLOOP \
    do									\
    {									\
      uint16 tex = srcTex [((vv >> 16) << shifter) + (uu >> 16)];	\
      *_dest = ((*_dest & Scan.AlphaMask) >> 1) + ((tex & Scan.AlphaMask) >> 1);\
      uu += duu;							\
      vv += dvv;							\
      _dest++;								\
    }									\
    while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_scan_map_fixalpha50

//------------------------------------------------------------------

#ifndef NO_scan_map_fixalpha50_key

#define SCANFUNC csScan_16_scan_map_fixalpha50_key
#define SCANMAP
#define SCANLOOP \
    do									\
    {									\
      uint16 tex = srcTex [((vv >> 16) << shifter) + (uu >> 16)];	\
      if (tex)								\
        *_dest = ((*_dest & Scan.AlphaMask) >> 1) + ((tex & Scan.AlphaMask) >> 1);\
      uu += duu;							\
      vv += dvv;							\
      _dest++;								\
    }									\
    while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_scan_map_fixalpha50_key

//------------------------------------------------------------------

#ifndef NO_555_scan_map_fixalpha

#define SCANFUNC csScan_16_555_scan_map_fixalpha
#define SCANMAP
#define SCANLOOP \
    do									\
    {									\
      uint16 tex = srcTex [((vv >> 16) << shifter) + (uu >> 16)];	\
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

#endif // NO_555_scan_map_fixalpha

//------------------------------------------------------------------

#ifndef NO_555_scan_map_fixalpha_key

#define SCANFUNC csScan_16_555_scan_map_fixalpha_key
#define SCANMAP
#define SCANLOOP \
    do									\
    {									\
      uint16 tex = srcTex [((vv >> 16) << shifter) + (uu >> 16)];	\
      if (tex)								\
      {									\
        int tr = *_dest & 0x7c00;					\
        int tg = *_dest & 0x03e0;					\
        int tb = *_dest & 0x001f;					\
        int r = (Scan.AlphaFact * ((tex & 0x7c00) - tr) >> 8) + tr;	\
        int g = (Scan.AlphaFact * ((tex & 0x03e0) - tg) >> 8) + tg;	\
        int b = (Scan.AlphaFact * ((tex & 0x001f) - tb) >> 8) + tb;	\
        *_dest++ = (r & 0x7c00) | (g & 0x03e0) | b;			\
      }									\
      else								\
        ++_dest;							\
      uu += duu;							\
      vv += dvv;							\
    }									\
    while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_555_scan_map_fixalpha_key

//------------------------------------------------------------------

#ifndef NO_555_scan_map_fixalpha_alphamap

#define SCANFUNC csScan_16_555_scan_map_fixalpha_alphamap
#define SCANMAP
#define TEXCOORDS
#define SCANLOOP \
    do									\
    {									\
      uint16 tex = srcTex [((vv >> 16) << shifter) + (uu >> 16)];	\
      uint8 alpha = (Scan.AlphaFact *					\
        Scan.AlphaMap[((uu >> 16) & ander_w) + ((vv >> shifter_h) & ander_h)]>>5);\
      int tr = *_dest & 0x7c00;						\
      int tg = *_dest & 0x03e0;						\
      int tb = *_dest & 0x001f;						\
      int r = (alpha * ((tex & 0x7c00) - tr) >> 8) + tr;	\
      int g = (alpha * ((tex & 0x03e0) - tg) >> 8) + tg;	\
      int b = (alpha * ((tex & 0x001f) - tb) >> 8) + tb;	\
      *_dest++ = (r & 0x7c00) | (g & 0x03e0) | b;			\
      uu += duu;							\
      vv += dvv;							\
    }									\
    while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_555_scan_map_fixalpha_alphamap

//------------------------------------------------------------------

#ifndef NO_555_scan_map_fixalpha50_alphamap

#define SCANFUNC csScan_16_555_scan_map_fixalpha50_alphamap
#define SCANMAP
#define TEXCOORDS
#define SCANLOOP \
    do									\
    {									\
      uint16 tex = srcTex [((vv >> 16) << shifter) + (uu >> 16)];	\
      uint8 alpha = Scan.AlphaMap[((uu >> 16) & ander_w) + ((vv >> shifter_h) & ander_h)]<<2;\
      int tr = *_dest & 0x7c00;						\
      int tg = *_dest & 0x03e0;						\
      int tb = *_dest & 0x001f;						\
      int r = (alpha * ((tex & 0x7c00) - tr) >> 8) + tr;	\
      int g = (alpha * ((tex & 0x03e0) - tg) >> 8) + tg;	\
      int b = (alpha * ((tex & 0x001f) - tb) >> 8) + tb;	\
      *_dest++ = (r & 0x7c00) | (g & 0x03e0) | b;			\
      uu += duu;							\
      vv += dvv;							\
    }									\
    while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_555_scan_map_fixalpha50_alphamap

//------------------------------------------------------------------

#ifndef NO_565_scan_map_fixalpha

#define SCANFUNC csScan_16_565_scan_map_fixalpha
#define SCANMAP
#define SCANLOOP \
    do									\
    {									\
      uint16 tex = srcTex [((vv >> 16) << shifter) + (uu >> 16)];	\
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

#endif // NO_565_scan_map_fixalpha

//------------------------------------------------------------------

#ifndef NO_565_scan_map_fixalpha_key

#define SCANFUNC csScan_16_565_scan_map_fixalpha_key
#define SCANMAP
#define SCANLOOP \
    do									\
    {									\
      uint16 tex = srcTex [((vv >> 16) << shifter) + (uu >> 16)];	\
      if (tex)								\
      {									\
        int tr = *_dest & 0xf800;					\
        int tg = *_dest & 0x07e0;					\
        int tb = *_dest & 0x001f;					\
        int r = (Scan.AlphaFact * ((tex & 0xf800) - tr) >> 8) + tr;	\
        int g = (Scan.AlphaFact * ((tex & 0x07e0) - tg) >> 8) + tg;	\
        int b = (Scan.AlphaFact * ((tex & 0x001f) - tb) >> 8) + tb;	\
        *_dest++ = (r & 0xf800) | (g & 0x07e0) | b;			\
      }									\
      else								\
        ++_dest;							\
      uu += duu;							\
      vv += dvv;							\
    }									\
    while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_565_scan_map_fixalpha_key

//------------------------------------------------------------------

#ifndef NO_565_scan_map_fixalpha_alphamap

#define SCANFUNC csScan_16_565_scan_map_fixalpha_alphamap
#define SCANMAP
#define TEXCOORDS
#define SCANLOOP \
    do									\
    {									\
      uint16 tex = srcTex [((vv >> 16) << shifter) + (uu >> 16)];	\
      uint8 alpha = (Scan.AlphaFact *					\
        Scan.AlphaMap[((uu >> 16) & ander_w) + ((vv >> shifter_h) & ander_h)]>>5);\
      int tr = *_dest & 0xf800;						\
      int tg = *_dest & 0x07e0;						\
      int tb = *_dest & 0x001f;						\
      int r = (alpha * ((tex & 0xf800) - tr) >> 8) + tr;	\
      int g = (alpha * ((tex & 0x07e0) - tg) >> 8) + tg;	\
      int b = (alpha * ((tex & 0x001f) - tb) >> 8) + tb;	\
      *_dest++ = (r & 0xf800) | (g & 0x07e0) | b;			\
      uu += duu;							\
      vv += dvv;							\
    }									\
    while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_565_scan_map_fixalpha_alphamap

//------------------------------------------------------------------


#ifndef NO_565_scan_map_fixalpha50_alphamap

#define SCANFUNC csScan_16_565_scan_map_fixalpha50_alphamap
#define SCANMAP
#define TEXCOORDS
#define SCANLOOP \
    do									\
    {									\
      uint16 tex = srcTex [((vv >> 16) << shifter) + (uu >> 16)];	\
      uint8 alpha = Scan.AlphaMap[((uu >> 16) & ander_w) + ((vv >> shifter_h) & ander_h)]<<2;\
      int tr = *_dest & 0xf800;						\
      int tg = *_dest & 0x07e0;						\
      int tb = *_dest & 0x001f;						\
      int r = (alpha * ((tex & 0xf800) - tr) >> 8) + tr;	\
      int g = (alpha * ((tex & 0x07e0) - tg) >> 8) + tg;	\
      int b = (alpha * ((tex & 0x001f) - tb) >> 8) + tb;	\
      *_dest++ = (r & 0xf800) | (g & 0x07e0) | b;			\
      uu += duu;							\
      vv += dvv;							\
    }									\
    while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_565_scan_map_fixalpha50_alphamap

//------------------------------------------------------------------

#ifndef NO_555_scan_fog

void csScan_16_555_scan_fog (int xx, unsigned char* d,
  uint32* z_buf, float inv_z, float u_div_z, float v_div_z)
{
  if (xx <= 0) return;
  (void)u_div_z; (void)v_div_z;
  uint16 *_dest = (uint16 *)d;
  uint16 *_destend = _dest + xx;
  uint32 izz = csQfixed24 (inv_z);
  int dzz = csQfixed24 (Scan.M);
  uint16 fog_pix = Scan.FogR | Scan.FogG | Scan.FogB;
  uint32 fog_dens = Scan.FogDensity;

  do
  {
    int fd;
    uint32 izb = *z_buf;
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

#endif // NO_555_scan_fog

//------------------------------------------------------------------

#ifndef NO_565_scan_fog

void csScan_16_565_scan_fog (int xx, unsigned char* d,
  uint32* z_buf, float inv_z, float u_div_z, float v_div_z)
{
  if (xx <= 0) return;
  (void)u_div_z; (void)v_div_z;
  uint16 *_dest = (uint16 *)d;
  uint16 *_destend = _dest + xx;
  uint32 izz = csQfixed24 (inv_z);
  int dzz = csQfixed24 (Scan.M);
  uint16 fog_pix = Scan.FogR | Scan.FogG | Scan.FogB;
  uint32 fog_dens = Scan.FogDensity;

  do
  {
    int fd;
    uint32 izb = *z_buf;
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

#endif // NO_565_scan_fog

//------------------------------------------------------------------

#ifndef NO_555_scan_fog_view

void csScan_16_555_scan_fog_view (int xx, unsigned char* d,
  uint32* z_buf, float inv_z, float u_div_z, float v_div_z)
{
  if (xx <= 0) return;
  (void)u_div_z; (void)v_div_z; (void)inv_z;
  uint16* _dest = (uint16*)d;
  uint16* _destend = _dest + xx;
  uint16 fog_pix = Scan.FogR | Scan.FogG | Scan.FogB;
  uint32 fog_dens = Scan.FogDensity;

  do
  {
    uint32 izb = *z_buf;
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

#endif // NO_555_scan_fog_view

//------------------------------------------------------------------

#ifndef NO_565_scan_fog_view

void csScan_16_565_scan_fog_view (int xx, unsigned char* d,
  uint32* z_buf, float inv_z, float u_div_z, float v_div_z)
{
  if (xx <= 0) return;
  (void)u_div_z; (void)v_div_z; (void)inv_z;
  uint16* _dest = (uint16*)d;
  uint16* _destend = _dest + xx;
  uint16 fog_pix = Scan.FogR | Scan.FogG | Scan.FogB;
  uint32 fog_dens = Scan.FogDensity;

  do
  {
    uint32 izb = *z_buf;
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

#endif // NO_565_scan_fog_view

//------------------------------------------------------------------

#ifndef NO_555_scan_map_alpha_znone

#define A_SCANFUNC csScan_16_555_scan_map_alpha_znone
#define A_ZNONE
#define A_MAP
#define A_R5G5B5
#include "scanalph.inc"

#endif // NO_555_scan_map_alpha_znone

//------------------------------------------------------------------

#ifndef NO_555_scan_map_alpha_zfil

#define A_SCANFUNC csScan_16_555_scan_map_alpha_zfil
#define A_ZFIL
#define A_MAP
#define A_R5G5B5
#include "scanalph.inc"

#endif // NO_555_scan_map_alpha_zfil

//------------------------------------------------------------------

#ifndef NO_555_scan_map_alpha_zuse

#define A_SCANFUNC csScan_16_555_scan_map_alpha_zuse
#define A_ZUSE
#define A_MAP
#define A_R5G5B5
#include "scanalph.inc"

#endif // NO_555_scan_map_alpha_zuse

//------------------------------------------------------------------

#ifndef NO_555_scan_map_alpha_ztest

#define A_SCANFUNC csScan_16_555_scan_map_alpha_ztest
#define A_ZTEST
#define A_MAP
#define A_R5G5B5
#include "scanalph.inc"

#endif // NO_555_scan_map_alpha_ztest

//------------------------------------------------------------------

#ifndef NO_565_scan_map_alpha_znone

#define A_SCANFUNC csScan_16_565_scan_map_alpha_znone
#define A_ZNONE
#define A_MAP
#define A_R5G6B5
#include "scanalph.inc"

#endif // NO_565_scan_map_alpha_znone

//------------------------------------------------------------------

#ifndef NO_565_scan_map_alpha_zfil

#define A_SCANFUNC csScan_16_565_scan_map_alpha_zfil
#define A_ZFIL
#define A_MAP
#define A_R5G6B5
#include "scanalph.inc"

#endif // NO_565_scan_map_alpha_zfil

//------------------------------------------------------------------

#ifndef NO_565_scan_map_alpha_zuse

#define A_SCANFUNC csScan_16_565_scan_map_alpha_zuse
#define A_ZUSE
#define A_MAP
#define A_R5G6B5
#include "scanalph.inc"

#endif // NO_565_scan_map_alpha_zuse

//------------------------------------------------------------------

#ifndef NO_565_scan_map_alpha_ztest

#define A_SCANFUNC csScan_16_565_scan_map_alpha_ztest
#define A_ZTEST
#define A_MAP
#define A_R5G6B5
#include "scanalph.inc"

#endif // NO_565_scan_map_alpha_ztest

//------------------------------------------------------------------

#ifndef NO_555_scan_tex_alpha_znone

#define A_SCANFUNC csScan_16_555_scan_tex_alpha_znone
#define A_ZNONE
#define A_R5G5B5
#include "scanalph.inc"

#endif // NO_555_scan_tex_alpha_znone

//------------------------------------------------------------------

#ifndef NO_555_scan_tex_alpha_zfil

#define A_SCANFUNC csScan_16_555_scan_tex_alpha_zfil
#define A_ZFIL
#define A_R5G5B5
#include "scanalph.inc"

#endif // NO_555_scan_tex_alpha_zfil

//------------------------------------------------------------------

#ifndef NO_555_scan_tex_alpha_zuse

#define A_SCANFUNC csScan_16_555_scan_tex_alpha_zuse
#define A_ZUSE
#define A_R5G5B5
#include "scanalph.inc"

#endif // NO_555_scan_tex_alpha_zuse

//------------------------------------------------------------------

#ifndef NO_555_scan_tex_alpha_ztest

#define A_SCANFUNC csScan_16_555_scan_tex_alpha_ztest
#define A_ZTEST
#define A_R5G5B5
#include "scanalph.inc"

#endif // NO_555_scan_tex_alpha_ztest

//------------------------------------------------------------------

#ifndef NO_565_scan_tex_alpha_znone

#define A_SCANFUNC csScan_16_565_scan_tex_alpha_znone
#define A_ZNONE
#define A_R5G6B5
#include "scanalph.inc"

#endif // NO_565_scan_tex_alpha_znone

//------------------------------------------------------------------

#ifndef NO_565_scan_tex_alpha_zfil

#define A_SCANFUNC csScan_16_565_scan_tex_alpha_zfil
#define A_ZFIL
#define A_R5G6B5
#include "scanalph.inc"

#endif // NO_565_scan_tex_alpha_zfil

//------------------------------------------------------------------

#ifndef NO_565_scan_tex_alpha_zuse

#define A_SCANFUNC csScan_16_565_scan_tex_alpha_zuse
#define A_ZUSE
#define A_R5G6B5
#include "scanalph.inc"

#endif // NO_565_scan_tex_alpha_zuse

//------------------------------------------------------------------

#ifndef NO_565_scan_tex_alpha_ztest

#define A_SCANFUNC csScan_16_565_scan_tex_alpha_ztest
#define A_ZTEST
#define A_R5G6B5
#include "scanalph.inc"

#endif // NO_565_scan_tex_alpha_ztest

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_alpha_znone

#define A_SCANFUNC csScan_16_555_scan_pi_tex_alpha_znone
#define A_PI
#define A_R5G5B5
#include "scanalph.inc"

#endif // NO_555_scan_pi_tex_alpha_znone

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_alpha_zfil

#define A_SCANFUNC csScan_16_555_scan_pi_tex_alpha_zfil
#define A_PI
#define A_R5G5B5
#define A_ZFIL
#include "scanalph.inc"

#endif // NO_555_scan_pi_tex_alpha_zfil

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_alpha_zuse

#define A_SCANFUNC csScan_16_555_scan_pi_tex_alpha_zuse
#define A_PI
#define A_R5G5B5
#define A_ZUSE
#include "scanalph.inc"

#endif // NO_555_scan_pi_tex_alpha_zuse

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_alpha_ztest

#define A_SCANFUNC csScan_16_555_scan_pi_tex_alpha_ztest
#define A_PI
#define A_R5G5B5
#define A_ZTEST
#include "scanalph.inc"

#endif // NO_555_scan_pi_tex_alpha_ztest

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_alpha_znone

#define A_SCANFUNC csScan_16_565_scan_pi_tex_alpha_znone
#define A_PI
#define A_R5G6B5
#include "scanalph.inc"

#endif // NO_565_scan_pi_tex_alpha_znone

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_alpha_zfil

#define A_SCANFUNC csScan_16_565_scan_pi_tex_alpha_zfil
#define A_PI
#define A_R5G6B5
#define A_ZFIL
#include "scanalph.inc"

#endif // NO_565_scan_pi_tex_alpha_zfil

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_alpha_zuse

#define A_SCANFUNC csScan_16_565_scan_pi_tex_alpha_zuse
#define A_PI
#define A_R5G6B5
#define A_ZUSE
#include "scanalph.inc"

#endif // NO_565_scan_pi_tex_alpha_zuse

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_alpha_ztest

#define A_SCANFUNC csScan_16_565_scan_pi_tex_alpha_ztest
#define A_PI
#define A_R5G6B5
#define A_ZTEST
#include "scanalph.inc"

#endif // NO_565_scan_pi_tex_alpha_ztest

//------------------------------------------------------------------

#ifndef NO_scan_pi_flat_znone

#define PI_SCANFUNC csScan_16_scan_pi_flat_znone
#define PI_FLAT
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_scan_pi_flat_znone

//------------------------------------------------------------------

#ifndef NO_scan_pi_flat_zfil

#define PI_SCANFUNC csScan_16_scan_pi_flat_zfil
#define PI_ZFILL
#define PI_FLAT
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_scan_pi_flat_zfil

//------------------------------------------------------------------

#ifndef NO_scan_pi_flat_zuse

#define PI_SCANFUNC csScan_16_scan_pi_flat_zuse
#define PI_ZUSE
#define PI_FLAT
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_scan_pi_flat_zuse

//------------------------------------------------------------------

#ifndef NO_scan_pi_flat_ztest

#define PI_SCANFUNC csScan_16_scan_pi_flat_ztest
#define PI_ZTEST
#define PI_FLAT
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_scan_pi_flat_ztest

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_znone

#define PI_SCANFUNC csScan_16_scan_pi_tex_znone
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_scan_pi_tile_tex_znone
#define PI_TILE
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_scan_pi_tex_znone

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_zfil

#define PI_SCANFUNC csScan_16_scan_pi_tex_zfil
#define PI_ZFILL
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_scan_pi_tile_tex_zfil
#define PI_TILE
#define PI_ZFILL
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_scan_pi_tex_zfil

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_zuse

#define PI_SCANFUNC csScan_16_scan_pi_tex_zuse
#define PI_ZUSE
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_scan_pi_tex_zuse

#ifndef NO_scan_pi_tile_tex_zuse

#define PI_SCANFUNC csScan_16_scan_pi_tile_tex_zuse
#define PI_TILE
#define PI_ZUSE
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_scan_pi_tile_tex_zuse

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_ztest

#define PI_SCANFUNC csScan_16_scan_pi_tex_ztest
#define PI_ZTEST
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_scan_pi_tile_tex_ztest
#define PI_TILE
#define PI_ZTEST
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_scan_pi_tex_ztest

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_key_znone

#define PI_SCANFUNC csScan_16_scan_pi_tex_key_znone
#define PI_COLORKEY
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_scan_pi_tile_tex_key_znone
#define PI_TILE
#define PI_COLORKEY
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_scan_pi_tex_key_znone

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_key_zfil

#define PI_SCANFUNC csScan_16_scan_pi_tex_key_zfil
#define PI_ZFILL
#define PI_COLORKEY
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_scan_pi_tile_tex_key_zfil
#define PI_TILE
#define PI_ZFILL
#define PI_COLORKEY
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_scan_pi_tex_key_zfil

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_key_zuse

#define PI_SCANFUNC csScan_16_scan_pi_tex_key_zuse
#define PI_ZUSE
#define PI_COLORKEY
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_scan_pi_tile_tex_key_zuse
#define PI_TILE
#define PI_ZUSE
#define PI_COLORKEY
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_scan_pi_tex_key_zuse

//------------------------------------------------------------------

#ifndef NO_scan_pi_tex_key_ztest

#define PI_SCANFUNC csScan_16_scan_pi_tex_key_ztest
#define PI_ZTEST
#define PI_COLORKEY
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_scan_pi_tile_tex_key_ztest
#define PI_TILE
#define PI_ZTEST
#define PI_COLORKEY
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_scan_pi_tex_key_ztest

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_flat_fx_znone

#define PI_SCANFUNC csScan_16_555_scan_pi_flat_fx_znone
#define PI_FLAT
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_flat_fx_none

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_flat_fx_znone

#define PI_SCANFUNC csScan_16_565_scan_pi_flat_fx_znone
#define PI_FLAT
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_flat_fx_znone

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_flat_fx_zfil

#define PI_SCANFUNC csScan_16_555_scan_pi_flat_fx_zfil
#define PI_ZFILL
#define PI_FLAT
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_flat_fx_zfil

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_flat_fx_zfil

#define PI_SCANFUNC csScan_16_565_scan_pi_flat_fx_zfil
#define PI_ZFILL
#define PI_FLAT
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_flat_fx_zfil

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_flat_fx_zuse

#define PI_SCANFUNC csScan_16_555_scan_pi_flat_fx_zuse
#define PI_ZUSE
#define PI_FLAT
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_flat_fx_zuse

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_flat_fx_zuse

#define PI_SCANFUNC csScan_16_565_scan_pi_flat_fx_zuse
#define PI_ZUSE
#define PI_FLAT
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_flat_fx_zuse

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_flat_fx_ztest

#define PI_SCANFUNC csScan_16_555_scan_pi_flat_fx_ztest
#define PI_ZTEST
#define PI_FLAT
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_flat_fx_ztest

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_flat_fx_ztest

#define PI_SCANFUNC csScan_16_565_scan_pi_flat_fx_ztest
#define PI_ZTEST
#define PI_FLAT
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_flat_fx_ztest

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_fx_znone

#define PI_SCANFUNC csScan_16_555_scan_pi_tex_fx_znone
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_555_scan_pi_tile_tex_fx_znone
#define PI_TILE
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_tex_fx_znone

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_fx_znone

#define PI_SCANFUNC csScan_16_565_scan_pi_tex_fx_znone
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_565_scan_pi_tile_tex_fx_znone
#define PI_TILE
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_tex_fx_znone

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_fx_zfil

#define PI_SCANFUNC csScan_16_555_scan_pi_tex_fx_zfil
#define PI_ZFILL
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_555_scan_pi_tile_tex_fx_zfil
#define PI_TILE
#define PI_ZFILL
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_tex_fx_zfil

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_fx_zfil

#define PI_SCANFUNC csScan_16_565_scan_pi_tex_fx_zfil
#define PI_ZFILL
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_565_scan_pi_tile_tex_fx_zfil
#define PI_TILE
#define PI_ZFILL
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_tex_fx_zfil

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_fx_zuse

#define PI_SCANFUNC csScan_16_555_scan_pi_tex_fx_zuse
#define PI_ZUSE
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_555_scan_pi_tile_tex_fx_zuse
#define PI_TILE
#define PI_ZUSE
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_tex_fx_zuse

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_fx_zuse

#define PI_SCANFUNC csScan_16_565_scan_pi_tex_fx_zuse
#define PI_ZUSE
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_565_scan_pi_tile_tex_fx_zuse
#define PI_TILE
#define PI_ZUSE
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_tex_fx_zuse

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_fx_ztest

#define PI_SCANFUNC csScan_16_555_scan_pi_tex_fx_ztest
#define PI_ZTEST
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_555_scan_pi_tile_tex_fx_ztest
#define PI_TILE
#define PI_ZTEST
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_tex_fx_ztest

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_fx_ztest

#define PI_SCANFUNC csScan_16_565_scan_pi_tex_fx_ztest
#define PI_ZTEST
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_565_scan_pi_tile_tex_fx_ztest
#define PI_TILE
#define PI_ZTEST
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_tex_fx_ztest

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_fxkey_znone

#define PI_SCANFUNC csScan_16_555_scan_pi_tex_fxkey_znone
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_555_scan_pi_tile_tex_fxkey_znone
#define PI_TILE
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_tex_fxkey_znone

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_fxkey_znone

#define PI_SCANFUNC csScan_16_565_scan_pi_tex_fxkey_znone
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_565_scan_pi_tile_tex_fxkey_znone
#define PI_TILE
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_tex_fxkey_znone

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_fxkey_zfil

#define PI_SCANFUNC csScan_16_555_scan_pi_tex_fxkey_zfil
#define PI_ZFILL
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_555_scan_pi_tile_tex_fxkey_zfil
#define PI_TILE
#define PI_ZFILL
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_tex_fxkey_zfil

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_fxkey_zfil

#define PI_SCANFUNC csScan_16_565_scan_pi_tex_fxkey_zfil
#define PI_ZFILL
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_565_scan_pi_tile_tex_fxkey_zfil
#define PI_TILE
#define PI_ZFILL
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_tex_fxkey_zfil

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_fxkey_zuse

#define PI_SCANFUNC csScan_16_555_scan_pi_tex_fxkey_zuse
#define PI_ZUSE
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_555_scan_pi_tile_tex_fxkey_zuse
#define PI_TILE
#define PI_ZUSE
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_tex_fxkey_zuse

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_fxkey_zuse

#define PI_SCANFUNC csScan_16_565_scan_pi_tex_fxkey_zuse
#define PI_ZUSE
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_565_scan_pi_tile_tex_fxkey_zuse
#define PI_TILE
#define PI_ZUSE
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_tex_fxkey_zuse

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_fxkey_ztest

#define PI_SCANFUNC csScan_16_555_scan_pi_tex_fxkey_ztest
#define PI_ZTEST
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_555_scan_pi_tile_tex_fxkey_ztest
#define PI_TILE
#define PI_ZTEST
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_tex_fxkey_ztest

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_fxkey_ztest

#define PI_SCANFUNC csScan_16_565_scan_pi_tex_fxkey_ztest
#define PI_ZTEST
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_565_scan_pi_tile_tex_fxkey_ztest
#define PI_TILE
#define PI_ZTEST
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_tex_fxkey_ztest

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_flat_gou_znone

#define PI_SCANFUNC csScan_16_555_scan_pi_flat_gou_znone
#define PI_FLAT
#define PI_GOURAUD
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_flat_gou_znone

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_flat_gou_znone

#define PI_SCANFUNC csScan_16_565_scan_pi_flat_gou_znone
#define PI_FLAT
#define PI_GOURAUD
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_flat_gou_znone

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_flat_gou_zfil

#define PI_SCANFUNC csScan_16_555_scan_pi_flat_gou_zfil
#define PI_ZFILL
#define PI_FLAT
#define PI_GOURAUD
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_flat_gou_zfil

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_flat_gou_zfil

#define PI_SCANFUNC csScan_16_565_scan_pi_flat_gou_zfil
#define PI_ZFILL
#define PI_FLAT
#define PI_GOURAUD
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_flat_gou_zfil

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_flat_gou_zuse

#define PI_SCANFUNC csScan_16_555_scan_pi_flat_gou_zuse
#define PI_ZUSE
#define PI_FLAT
#define PI_GOURAUD
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_flat_gou_zuse

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_flat_gou_zuse

#define PI_SCANFUNC csScan_16_565_scan_pi_flat_gou_zuse
#define PI_ZUSE
#define PI_FLAT
#define PI_GOURAUD
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_flat_gou_zuse

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_flat_gou_ztest

#define PI_SCANFUNC csScan_16_555_scan_pi_flat_gou_ztest
#define PI_ZTEST
#define PI_FLAT
#define PI_GOURAUD
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_flat_gou_ztest

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_flat_gou_ztest

#define PI_SCANFUNC csScan_16_565_scan_pi_flat_gou_ztest
#define PI_ZTEST
#define PI_FLAT
#define PI_GOURAUD
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_flat_gou_ztest

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_gou_znone

#define PI_SCANFUNC csScan_16_555_scan_pi_tex_gou_znone
#define PI_GOURAUD
#define PI_R5G5B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_555_scan_pi_tile_tex_gou_znone
#define PI_TILE
#define PI_GOURAUD
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_tex_gou_znone

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_gou_znone

#define PI_SCANFUNC csScan_16_565_scan_pi_tex_gou_znone
#define PI_GOURAUD
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_565_scan_pi_tile_tex_gou_znone
#define PI_TILE
#define PI_GOURAUD
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_tex_gou_znone

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_gou_zfil

#define PI_SCANFUNC csScan_16_555_scan_pi_tex_gou_zfil
#define PI_ZFILL
#define PI_GOURAUD
#define PI_R5G5B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_555_scan_pi_tile_tex_gou_zfil
#define PI_TILE
#define PI_ZFILL
#define PI_GOURAUD
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_tex_gou_zfil

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_gou_zfil

#define PI_SCANFUNC csScan_16_565_scan_pi_tex_gou_zfil
#define PI_ZFILL
#define PI_GOURAUD
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_565_scan_pi_tile_tex_gou_zfil
#define PI_TILE
#define PI_ZFILL
#define PI_GOURAUD
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_tex_gou_zfil

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_gou_zuse

#define PI_SCANFUNC csScan_16_555_scan_pi_tex_gou_zuse
#define PI_ZUSE
#define PI_GOURAUD
#define PI_R5G5B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_555_scan_pi_tile_tex_gou_zuse
#define PI_TILE
#define PI_ZUSE
#define PI_GOURAUD
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_tex_gou_zuse

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_gou_zuse

#define PI_SCANFUNC csScan_16_565_scan_pi_tex_gou_zuse
#define PI_ZUSE
#define PI_GOURAUD
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_565_scan_pi_tile_tex_gou_zuse
#define PI_TILE
#define PI_ZUSE
#define PI_GOURAUD
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_tex_gou_zuse

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_gou_ztest

#define PI_SCANFUNC csScan_16_555_scan_pi_tex_gou_ztest
#define PI_ZTEST
#define PI_GOURAUD
#define PI_R5G5B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_555_scan_pi_tile_tex_gou_ztest
#define PI_TILE
#define PI_ZTEST
#define PI_GOURAUD
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_tex_gou_ztest

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_gou_ztest

#define PI_SCANFUNC csScan_16_565_scan_pi_tex_gou_ztest
#define PI_ZTEST
#define PI_GOURAUD
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_565_scan_pi_tile_tex_gou_ztest
#define PI_TILE
#define PI_ZTEST
#define PI_GOURAUD
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_tex_gou_ztest

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_goukey_znone

#define PI_SCANFUNC csScan_16_555_scan_pi_tex_goukey_znone
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_R5G5B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_555_scan_pi_tile_tex_goukey_znone
#define PI_TILE
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_tex_goukey_znone

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_goukey_znone

#define PI_SCANFUNC csScan_16_565_scan_pi_tex_goukey_znone
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_565_scan_pi_tile_tex_goukey_znone
#define PI_TILE
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_tex_goukey_znone

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_goukey_zfil

#define PI_SCANFUNC csScan_16_555_scan_pi_tex_goukey_zfil
#define PI_ZFILL
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_R5G5B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_555_scan_pi_tile_tex_goukey_zfil
#define PI_TILE
#define PI_ZFILL
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_tex_goukey_zfil

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_goukey_zfil

#define PI_SCANFUNC csScan_16_565_scan_pi_tex_goukey_zfil
#define PI_ZFILL
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_565_scan_pi_tile_tex_goukey_zfil
#define PI_TILE
#define PI_ZFILL
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_tex_goukey_zfil

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_goukey_zuse

#define PI_SCANFUNC csScan_16_555_scan_pi_tex_goukey_zuse
#define PI_ZUSE
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_R5G5B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_555_scan_pi_tile_tex_goukey_zuse
#define PI_TILE
#define PI_ZUSE
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_tex_goukey_zuse

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_goukey_zuse

#define PI_SCANFUNC csScan_16_565_scan_pi_tex_goukey_zuse
#define PI_ZUSE
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_565_scan_pi_tile_tex_goukey_zuse
#define PI_TILE
#define PI_ZUSE
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_tex_goukey_zuse

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_goukey_ztest

#define PI_SCANFUNC csScan_16_555_scan_pi_tex_goukey_ztest
#define PI_ZTEST
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_R5G5B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_555_scan_pi_tile_tex_goukey_ztest
#define PI_TILE
#define PI_ZTEST
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_tex_goukey_ztest

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_goukey_ztest

#define PI_SCANFUNC csScan_16_565_scan_pi_tex_goukey_ztest
#define PI_ZTEST
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_565_scan_pi_tile_tex_goukey_ztest
#define PI_TILE
#define PI_ZTEST
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_tex_goukey_ztest

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_flat_goufx_znone

#define PI_SCANFUNC csScan_16_555_scan_pi_flat_goufx_znone
#define PI_FLAT
#define PI_GOURAUD
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_flat_goufx_znone

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_flat_goufx_znone

#define PI_SCANFUNC csScan_16_565_scan_pi_flat_goufx_znone
#define PI_FLAT
#define PI_GOURAUD
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_flat_goufx_znone

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_flat_goufx_zfil

#define PI_SCANFUNC csScan_16_555_scan_pi_flat_goufx_zfil
#define PI_ZFILL
#define PI_FLAT
#define PI_GOURAUD
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_flat_goufx_zfil

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_flat_goufx_zfil

#define PI_SCANFUNC csScan_16_565_scan_pi_flat_goufx_zfil
#define PI_ZFILL
#define PI_FLAT
#define PI_GOURAUD
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_flat_goufx_zfil

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_flat_goufx_zuse

#define PI_SCANFUNC csScan_16_555_scan_pi_flat_goufx_zuse
#define PI_ZUSE
#define PI_FLAT
#define PI_GOURAUD
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_flat_goufx_zuse

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_flat_goufx_zuse

#define PI_SCANFUNC csScan_16_565_scan_pi_flat_goufx_zuse
#define PI_ZUSE
#define PI_FLAT
#define PI_GOURAUD
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_flat_goufx_zuse

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_flat_goufx_ztest

#define PI_SCANFUNC csScan_16_555_scan_pi_flat_goufx_ztest
#define PI_ZTEST
#define PI_FLAT
#define PI_GOURAUD
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_flat_goufx_ztest

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_flat_goufx_ztest

#define PI_SCANFUNC csScan_16_565_scan_pi_flat_goufx_ztest
#define PI_ZTEST
#define PI_FLAT
#define PI_GOURAUD
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_flat_goufx_ztest

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_goufx_znone

#define PI_SCANFUNC csScan_16_555_scan_pi_tex_goufx_znone
#define PI_GOURAUD
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_555_scan_pi_tile_tex_goufx_znone
#define PI_TILE
#define PI_GOURAUD
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_tex_goufx_znone

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_goufx_znone

#define PI_SCANFUNC csScan_16_565_scan_pi_tex_goufx_znone
#define PI_GOURAUD
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_565_scan_pi_tile_tex_goufx_znone
#define PI_TILE
#define PI_GOURAUD
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_tex_goufx_znone

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_goufx_zfil

#define PI_SCANFUNC csScan_16_555_scan_pi_tex_goufx_zfil
#define PI_ZFILL
#define PI_GOURAUD
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_555_scan_pi_tile_tex_goufx_zfil
#define PI_TILE
#define PI_ZFILL
#define PI_GOURAUD
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_tex_goufx_zfil

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_goufx_zfil

#define PI_SCANFUNC csScan_16_565_scan_pi_tex_goufx_zfil
#define PI_ZFILL
#define PI_GOURAUD
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_565_scan_pi_tile_tex_goufx_zfil
#define PI_TILE
#define PI_ZFILL
#define PI_GOURAUD
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_tex_goufx_zfil

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_goufx_zuse

#define PI_SCANFUNC csScan_16_555_scan_pi_tex_goufx_zuse
#define PI_ZUSE
#define PI_GOURAUD
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_555_scan_pi_tile_tex_goufx_zuse
#define PI_TILE
#define PI_ZUSE
#define PI_GOURAUD
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_tex_goufx_zuse

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_goufx_zuse

#define PI_SCANFUNC csScan_16_565_scan_pi_tex_goufx_zuse
#define PI_ZUSE
#define PI_GOURAUD
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_565_scan_pi_tile_tex_goufx_zuse
#define PI_TILE
#define PI_ZUSE
#define PI_GOURAUD
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_tex_goufx_zuse

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_goufx_ztest

#define PI_SCANFUNC csScan_16_555_scan_pi_tex_goufx_ztest
#define PI_ZTEST
#define PI_GOURAUD
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_555_scan_pi_tile_tex_goufx_ztest
#define PI_TILE
#define PI_ZTEST
#define PI_GOURAUD
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_tex_goufx_ztest

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_goufx_ztest

#define PI_SCANFUNC csScan_16_565_scan_pi_tex_goufx_ztest
#define PI_ZTEST
#define PI_GOURAUD
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_565_scan_pi_tile_tex_goufx_ztest
#define PI_TILE
#define PI_ZTEST
#define PI_GOURAUD
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_tex_goufx_ztest

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_goufxkey_znone

#define PI_SCANFUNC csScan_16_555_scan_pi_tex_goufxkey_znone
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_555_scan_pi_tile_tex_goufxkey_znone
#define PI_TILE
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_tex_goufxkey_znone

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_goufxkey_znone

#define PI_SCANFUNC csScan_16_565_scan_pi_tex_goufxkey_znone
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_565_scan_pi_tile_tex_goufxkey_znone
#define PI_TILE
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_tex_goufxkey_znone

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_goufxkey_zfil

#define PI_SCANFUNC csScan_16_555_scan_pi_tex_goufxkey_zfil
#define PI_ZFILL
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_555_scan_pi_tile_tex_goufxkey_zfil
#define PI_TILE
#define PI_ZFILL
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_tex_goufxkey_zfil

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_goufxkey_zfil

#define PI_SCANFUNC csScan_16_565_scan_pi_tex_goufxkey_zfil
#define PI_ZFILL
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_565_scan_pi_tile_tex_goufxkey_zfil
#define PI_TILE
#define PI_ZFILL
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_tex_goufxkey_zfil

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_goufxkey_zuse

#define PI_SCANFUNC csScan_16_555_scan_pi_tex_goufxkey_zuse
#define PI_ZUSE
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_555_scan_pi_tile_tex_goufxkey_zuse
#define PI_TILE
#define PI_ZUSE
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_tex_goufxkey_zuse

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_goufxkey_zuse

#define PI_SCANFUNC csScan_16_565_scan_pi_tex_goufxkey_zuse
#define PI_ZUSE
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_565_scan_pi_tile_tex_goufxkey_zuse
#define PI_TILE
#define PI_ZUSE
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_tex_goufxkey_zuse

//------------------------------------------------------------------

#ifndef NO_555_scan_pi_tex_goufxkey_ztest

#define PI_SCANFUNC csScan_16_555_scan_pi_tex_goufxkey_ztest
#define PI_ZTEST
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_555_scan_pi_tile_tex_goufxkey_ztest
#define PI_TILE
#define PI_ZTEST
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G5B5
#include "scanpi.inc"

#endif // NO_555_scan_pi_tex_goufxkey_ztest

//------------------------------------------------------------------

#ifndef NO_565_scan_pi_tex_goufxkey_ztest

#define PI_SCANFUNC csScan_16_565_scan_pi_tex_goufxkey_ztest
#define PI_ZTEST
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"
#define PI_SCANFUNC csScan_16_565_scan_pi_tile_tex_goufxkey_ztest
#define PI_TILE
#define PI_ZTEST
#define PI_GOURAUD
#define PI_COLORKEY
#define PI_BLEND
#define PI_R5G6B5
#include "scanpi.inc"

#endif // NO_565_scan_pi_tex_goufxkey_ztest

//------------------------------------------------------------------
