/*
    This include file contains common implementation for some routines that
    are quite the same in all 8-, 16- and 32bpp modes. The following macros
    are expected to be defined prior to including this file:

	SCAN8/SCAN16/SCAN32	The number of bits per pixel (8, 16, 32)
*/

#if defined (SCAN8)
#  define PIXTYPE UByte
#  define SCAN_BPP 8
#elif defined (SCAN16)
#  define PIXTYPE UShort
#  define SCAN_BPP 16
#elif defined (SCAN32)
#  define PIXTYPE ULong
#  define SCAN_BPP 32
#else
#  error "No valid SCANXX macro defined!"
#endif

#define SCAN_EXPAND(x,y)	csScan_ ## x ## _ ## y
#define SCAN_NAME(x, y)		SCAN_EXPAND (x, y)

#ifndef NO_draw_scanline_flat_zfil

void SCAN_NAME (SCAN_BPP, draw_scanline_flat_zfil) (int xx, unsigned char* d,
  unsigned long* z_buf, float inv_z, float u_div_z, float v_div_z PIXEL_ADJUST)
{
  (void)u_div_z; (void)v_div_z;
  int color = Scan.FlatColor;
  long izz = QInt24 (inv_z);
  long dzz = QInt24 (Scan.M);
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

#endif // NO_draw_scanline_flat_zfil

//------------------------------------------------------------------

#ifndef NO_draw_scanline_flat_zuse

void SCAN_NAME (SCAN_BPP, draw_scanline_flat_zuse) (int xx, unsigned char* d,
  unsigned long* z_buf, float inv_z, float u_div_z, float v_div_z PIXEL_ADJUST)
{
  (void)u_div_z; (void)v_div_z;
  int color = Scan.FlatColor;
  unsigned long izz = QInt24 (inv_z);
  unsigned long dzz = QInt24 (Scan.M);
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

#endif // NO_draw_scanline_flat_zuse

//------------------------------------------------------------------

#ifndef NO_draw_scanline_tex_zfil

#define SCANFUNC SCAN_NAME (SCAN_BPP, draw_scanline_tex_zfil)
#define SCANLOOP \
    do									\
    {									\
      *_dest++ = COLORMAP [srcTex [((uu >> 16) & ander_w) +		\
        ((vv >> shifter_h) & ander_h)]];				\
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

#define SCANFUNC SCAN_NAME (SCAN_BPP, draw_scanline_tex_zuse)
#define SCANLOOP \
    do									\
    {									\
      if (izz >= *z_buffer)						\
      {									\
	*z_buffer = izz;						\
        *_dest = COLORMAP [srcTex [((uu >> 16) & ander_w) +		\
          ((vv >> shifter_h) & ander_h)]];				\
      }									\
      _dest++;								\
      z_buffer++;							\
      uu += duu;							\
      vv += dvv;							\
      izz += dzz;							\
    }									\
    while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_draw_scanline_tex_zuse

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_zfil

#define SCANFUNC SCAN_NAME (SCAN_BPP, draw_scanline_map_zfil)
#define SCANMAP
#define SCANLOOP \
    do									\
    {									\
      *_dest++ = srcTex [((vv >> 16) << shifter) + (uu >> 16)];		\
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

#ifndef NO_draw_scanline_map_zuse

#define SCANFUNC SCAN_NAME (SCAN_BPP, draw_scanline_map_zuse)
#define SCANMAP
#define SCANLOOP \
    do									\
    {									\
      if (izz >= *z_buffer)						\
      {									\
	*z_buffer = izz;						\
	*_dest = srcTex [((vv >> 16) << shifter) + (uu >> 16)];		\
      }									\
      _dest++;								\
      z_buffer++;							\
      uu += duu;							\
      vv += dvv;							\
      izz += dzz;							\
    }									\
    while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_draw_scanline_map_zuse

//------------------------------------------------------------------

#ifndef NO_draw_scanline_tex_key_zfil

#define SCANFUNC SCAN_NAME (SCAN_BPP, draw_scanline_tex_key_zfil)
#define SCANLOOP \
    do                                                                  \
    {                                                                   \
      UByte c = srcTex [((uu >> 16) & ander_w) +			\
        ((vv >> shifter_h) & ander_h)];					\
      if (c)                                                            \
      {                                                                 \
        *_dest = COLORMAP [c];						\
        *z_buffer = izz;						\
      }                                                                 \
      _dest++;								\
      z_buffer++;							\
      uu += duu;                                                        \
      vv += dvv;                                                        \
      izz += dzz;                                                       \
    }                                                                   \
    while (_dest <= _destend)
#include "scanln.inc"

#endif // draw_scanline_tex_key_zfil

//------------------------------------------------------------------

#ifndef NO_draw_scanline_tex_key_zuse

#define SCANFUNC SCAN_NAME (SCAN_BPP, draw_scanline_tex_key_zuse)
#define SCANLOOP \
    do                                                                  \
    {                                                                   \
      if (izz >= *z_buffer)						\
      {									\
        UByte c = srcTex [((uu >> 16) & ander_w) +			\
          ((vv >> shifter_h) & ander_h)];				\
        if (!c) goto transp_pixel;					\
	*z_buffer = izz;						\
        *_dest = COLORMAP [c];						\
      }                                                                 \
transp_pixel:								\
      z_buffer++;							\
      _dest++;								\
      uu += duu;                                                        \
      vv += dvv;                                                        \
      izz += dzz;                                                       \
    }                                                                   \
    while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_draw_scanline_tex_key_zuse

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_key_zfil

#define SCANFUNC SCAN_NAME (SCAN_BPP, draw_scanline_map_key_zfil)
#define SCANMAP
#define SCANLOOP \
    do									\
    {									\
      PIXTYPE c = srcTex [((vv >> 16) << shifter) + (uu >> 16)];	\
      if (c)								\
      {									\
	*z_buffer = izz;						\
        *_dest = c;							\
      }									\
      _dest++;								\
      z_buffer++;							\
      uu += duu;							\
      vv += dvv;							\
      izz += dzz;							\
    }									\
    while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_draw_scanline_map_key_zfil

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_key_zuse

#define SCANFUNC SCAN_NAME (SCAN_BPP, draw_scanline_map_key_zuse)
#define SCANMAP
#define SCANLOOP \
    do                                                                  \
    {                                                                   \
      if (izz >= *z_buffer)						\
      {									\
        UByte c = srcTex [((vv >> 16) << shifter) + (uu >> 16)];	\
        if (!c) goto transp_pixel;					\
        *z_buffer = izz;						\
        *_dest = c;							\
      }                                                                 \
transp_pixel:								\
      _dest++;								\
      z_buffer++;							\
      uu += duu;                                                        \
      vv += dvv;                                                        \
      izz += dzz;                                                       \
    }                                                                   \
    while (_dest <= _destend)
#include "scanln.inc"

#endif // NO_draw_scanline_map_key_zuse
