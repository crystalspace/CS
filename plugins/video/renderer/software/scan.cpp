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
#include "cs3d/software/soft_g3d.h"
#include "cs3d/software/soft_txt.h"
#include "cs3d/software/tcache.h"
#include "isystem.h"
#include "ipolygon.h"

#include "sttest.h"

//--//--//--//--//--//--//--//--//--//--//--//--//-- Static variables //--//--//

int Scan::INTERPOL_STEP = 16;
int Scan::INTERPOL_SHFT = 4;
int Scan::inter_mode = INTER_MODE_SMART;
csTextureMMSoftware* Scan::texture;
IPolygon3D* Scan::poly;
unsigned char* Scan::tmap;
int Scan::flat_color;
int Scan::tw;
int Scan::th;
int Scan::shf_w, Scan::and_w;
int Scan::shf_h, Scan::and_h;
UShort Scan::alpha_mask = 0;
int Scan::alpha_fact;

int Scan::fog_red, Scan::fog_green, Scan::fog_blue;
unsigned long Scan::fog_density;

unsigned char* Scan::tmap2;
int Scan::tw2;
int Scan::th2;
int Scan::tw2fp;
int Scan::th2fp;
float Scan::fdu;
float Scan::fdv;
int Scan::shf_u;

float Scan::dM, Scan::dJ1, Scan::dK1;
float Scan::M, Scan::J1, Scan::K1;

float Scan::debug_sx1L;
float Scan::debug_sx1R;
float Scan::debug_sx2L;
float Scan::debug_sx2R;
float Scan::debug_sxL;
float Scan::debug_sxR;
float Scan::debug_sy;
float Scan::debug_sy1;
float Scan::debug_sy2;
float Scan::debug_inv_z;
float Scan::debug_u_div_z;
float Scan::debug_v_div_z;
float Scan::debug_N;
float Scan::debug_O;
float Scan::debug_J2;
float Scan::debug_J3;
float Scan::debug_K2;
float Scan::debug_K3;

int volatile Scan::debug_xx;
int volatile Scan::debug_uu;
int volatile Scan::debug_vv;
int volatile Scan::debug_uu1;
int volatile Scan::debug_vv1;
int volatile Scan::debug_duu;
int volatile Scan::debug_dvv;

// Set up by poly renderer to local->global palette translation table
unsigned char* priv_to_global;
// Set up by poly renderer to alpha blending table
RGB8map* alpha_map;
// SJI - dynamic lighting
unsigned char *Scan::curLightTable; // SJI dynamic light

//--//--//--//--//--//--//--//--//--//--//--/ assembler implementations --//--//

#if !defined(NO_ASSEMBLER)

#  if defined(PROC_M68K)
#    if defined(COMP_GCC)
#      include "cs3d/software/m68k/scanln8.h"
#    endif //COMP_GCC
#  endif //PROC_M68K

#  if defined (PROC_INTEL)
#    if defined (COMP_GCC)
#      include "cs3d/software/i386/scanln8.h"
#    elif defined (COMP_VC) || defined (COMP_WCC)
#      include "cs3d/software/i386/scanln8vc.h"
#    endif //COMP_???
#  endif //PROC_INTEL

#endif //!NO_ASSEMBLER

//--//--//--//--//--//--//--//--//--//--//--//--//-- draw_scanline_XXXX --//--//

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
#include "scanline.inc"


#endif // NO_draw_scanline_map

//------------------------------------------------------------------

int filter_bf;

#ifndef NO_draw_scanline_map_filter

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC draw_scanline_map_filter
#define SCANMAP 1
#define SCANLOOP \
    int filter_bf_shifted=filter_bf>>1,filter_du, filter_dv;            \
    while(_dest<=_destend&&((vv<BAILOUT_CONSTANT||uu<BAILOUT_CONSTANT)||(vv>=Scan::th2fp-BAILOUT_CONSTANT||uu>=Scan::tw2fp-BAILOUT_CONSTANT)))\
    {                                                                   \
      *_dest++ = srcTex[((vv>>16)<<shifter) + (uu>>16)];                \
      uu += duu;							\
      vv += dvv;							\
    }                                                                   \
    while ((_dest <= _destend)&&!((vv<BAILOUT_CONSTANT||uu<BAILOUT_CONSTANT)||(vv>=Scan::th2fp-BAILOUT_CONSTANT||uu>=Scan::tw2fp-BAILOUT_CONSTANT)))\
    {									\
      if (((((long)_dest)) & filter_bf_shifted) != 0)			\
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
#include "scanline.inc"

#endif // NO_draw_scanline_map_filter

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
#include "scanline.inc"

#endif // NO_draw_scanline

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
      *_dest++ = priv_to_global[srcTex[((uu>>16)&ander_w) + ((vv>>shifter_h)&ander_h)]];	\
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
#include "scanline.inc"

#endif // NO_draw_scanline_private

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_alpha1

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC draw_scanline_map_alpha1
#define SCANMAP 1
#define SCANLOOP \
    do									\
    {									\
      *_dest = alpha_map[*_dest][srcTex[((vv>>16)<<shifter) + (uu>>16)]];\
      _dest++;								\
      uu += duu;							\
      vv += dvv;							\
    }									\
    while (_dest <= _destend)
#include "scanline.inc"

#endif // NO_draw_scanline_map_alpha1

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_alpha2

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC draw_scanline_map_alpha2
#define SCANMAP 1
#define SCANLOOP \
    do									\
    {									\
      *_dest = alpha_map[srcTex[((vv>>16)<<shifter) + (uu>>16)]][*_dest];\
      _dest++;								\
      uu += duu;							\
      vv += dvv;							\
    }									\
    while (_dest <= _destend)
#include "scanline.inc"

#endif // NO_draw_scanline_map_alpha2

//------------------------------------------------------------------

#ifndef NO_draw_scanline_transp

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC draw_scanline_transp
#define SCANLOOP \
    unsigned char c;							\
    do									\
    {									\
      c = srcTex[((uu>>16)&ander_w) + ((vv>>shifter_h)&ander_h)];\
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
#include "scanline.inc"

#endif // draw_scanline_transp

//------------------------------------------------------------------

#ifndef NO_draw_scanline_transp_private

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC draw_scanline_transp_private
#define SCANLOOP \
    do									\
    {									\
      unsigned char c = srcTex[((uu>>16)&ander_w) +			\
        ((vv>>shifter_h)&ander_h)];					\
      if (c)								\
      {									\
        *_dest++ = priv_to_global[c];					\
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
#include "scanline.inc"

#endif // NO_draw_scanline_transp_private

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
      unsigned char c = srcTex[((vv>>16)<<shifter) + (uu>>16)];		\
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
#include "scanline.inc"

#endif // NO_draw_scanline_transp_map

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
#include "scanline.inc"

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
	*_dest++ = priv_to_global[srcTex[((uu>>16)&ander_w) + ((vv>>shifter_h)&ander_h)]];	\
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
#include "scanline.inc"

#endif // NO_draw_scanline_z_buf_private

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
#include "scanline.inc"

#endif // NO_draw_scanline_z_buf_map

//------------------------------------------------------------------

#ifndef NO_draw_scanline_map_light

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC draw_scanline_map_light
#define SCANMAP 1
#define SCANLOOP \
    do									\
    {									\
      *_dest++ = curLightTable[srcTex[((vv>>16)<<shifter) + (uu>>16)]];	\
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
#include "scanline.inc"

#endif // NO_draw_scanline_map_light

//------------------------------------------------------------------

#ifndef NO_draw_scanline_z_buf_map_light

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC draw_scanline_z_buf_map_light
#define SCANMAP 1
#define SCANLOOP \
    do									\
    {									\
      if (izz >= (int)(*z_buffer))					\
      {									\
	*_dest++ = curLightTable[srcTex[((vv>>16)<<shifter) + (uu>>16)]];\
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
#include "scanline.inc"

#endif // NO_draw_scanline_z_buf_map_light

//------------------------------------------------------------------

#ifndef NO_draw_scanline_flat

void Scan::draw_scanline_flat (int xx, unsigned char* d,
			       unsigned long* z_buf,
			       float inv_z, float u_div_z, float v_div_z)
{
  (void)u_div_z; (void)v_div_z;
  int color = flat_color;
  long izz = QInt24 (inv_z);
  long dzz = QInt24 (M);
  while (xx > 0)
  {

    *d++ = color;
    xx--;
    *z_buf++ = izz;
    izz += dzz;
  }
}

#endif // NO_draw_scanline_flat

//------------------------------------------------------------------

#ifndef NO_draw_scanline_z_buf_flat

void Scan::draw_scanline_z_buf_flat (int xx, unsigned char* d,
			       unsigned long* z_buf,
			       float inv_z, float u_div_z, float v_div_z)
{
  (void)u_div_z; (void)v_div_z;
  int i;
  int izz1, dzz, izz;
  int color = flat_color;

  izz = QInt24 (inv_z);

  while (xx > 0)
  {
    inv_z += dM;

    i = INTERPOL_STEP;
    if (xx < i) i = xx;
    xx -= i;

    izz1 = QInt24 (inv_z);
    dzz = (izz1-izz)/INTERPOL_STEP;

    while (i-- > 0)
    {
      if (izz >= (int)(*z_buf))
      {
	*d++ = color;
	*z_buf++ = izz;
      }
      else
      {
        d++;
        z_buf++;
      }
      izz += dzz;
    }
  }
}

#endif // NO_draw_scanline_z_buf_flat

//------------------------------------------------------------------

#ifndef NO_draw_scanline_fog

void Scan::draw_scanline_fog (int xx, unsigned char* d,
                              unsigned long* z_buf,
                              float inv_z, float u_div_z, float v_div_z)
{
  //@@@ Not implemented yet!
  (void)xx; (void)d; (void)z_buf; (void)inv_z; (void)u_div_z; (void)v_div_z;
}

#endif // NO_draw_scanline_fog

//------------------------------------------------------------------

#if 0
void Scan::light_scanline (int xx,
			   int uu, int vv,
			   unsigned char* d,
			   float d1, float d2, float dd1, float dd2,
			   float dd_u, float da_u, float dd_v, float da_v,
			   int lu, int lv, int sq_rad)
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

    i = INTERPOL_STEP;
    if (xx < i) i = xx;
    xx -= i;

    u1 = r*dd_u + da_u;
    v1 = r*dd_v + da_v;

    uu1 = QInt16 (u1);
    vv1 = QInt16 (v1);

    duu = (uu1-uu)/INTERPOL_STEP;
    dvv = (vv1-vv)/INTERPOL_STEP;

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

#ifndef NO_draw_pi_scanline

void Scan::draw_pi_scanline (void *dest, int len,
  long *zbuff, long u, long du, long v, long dv, long z, long dz,
  unsigned char *bitmap, int bitmap_log2w)
{
  unsigned char *_dest = (unsigned char *)dest;
  unsigned char *_destend = _dest + len;
  while (_dest < _destend)
  {
#if 0
    if ((u>>16) < 0 || (u>>16) > 256 || (v>>16) < 0 || (v>>16) > 256)
    {
        printf(MSG_DEBUG_0, "OVERFLOW u=%d v=%d\n", u, v);
        exit(1);
    }
#endif
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

#endif // NO_draw_pi_scanline

//------------------------------------------------------------------

#ifndef NO_draw_pi_scanline_zfill

void Scan::draw_pi_scanline_zfill (void *dest, int len,
  long *zbuff, long u, long du, long v, long dv, long z, long dz,
  unsigned char *bitmap, int bitmap_log2w)
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

#endif // NO_draw_pi_scanline_zfill

//------------------------------------------------------------------

#ifndef NO_mmx_draw_scanline

#ifdef DO_MMX
void Scan::mmx_draw_scanline (int xx, unsigned char* d,
                              unsigned long* z_buf,
                              float inv_z, float u_div_z, float v_div_z)
{
}
#endif

#endif // NO_mmx_draw_scanline

//------------------------------------------------------------------

#ifndef NO_mmx_draw_scanline_map

#ifdef DO_MMX
void Scan::mmx_draw_scanline_map (int xx, unsigned char* d,
                              unsigned long* z_buf,
                              float inv_z, float u_div_z, float v_div_z)
{
}
#endif

#endif // NO_mmx_draw_scanline_map

//------------------------------------------------------------------

#ifndef NO_mmx_draw_pi_scanline

#ifdef DO_MMX
void Scan::mmx_draw_pi_scanline (void *dest, int len, long *zbuff, long u, long du,
  long v, long dv, long z, long dz, unsigned char *bitmap, int bitmap_log2w)
{
}
#endif

#endif // NO_mmx_draw_pi_scanline

//------------------------------------------------------------------

void Scan::init_draw (csGraphics3DSoftware* g3d, IPolygon3D* p, IPolygonTexture* tex,
	csTextureMMSoftware* texture, csTexture* untxt)
{
  poly = p;

  Scan::texture = texture;
  tw = untxt->get_width ();
  th = untxt->get_height ();
  tmap = untxt->get_bitmap8 ();
  shf_w = untxt->get_w_shift ();
  and_w = untxt->get_w_mask ();
  shf_h = untxt->get_h_shift ();
  and_h = untxt->get_h_mask ();

  flat_color = texture->get_mean_color_idx ();

  if (g3d->do_lighting)
  {
    void* td = NULL;
    tex->GetTCacheData (&td);
    if (td)
    {
      TCacheLightedTexture* tclt = (TCacheLightedTexture*)td;
      tmap2 = tclt->get_tmap8 ();
    }
    else tmap2 = NULL;	// Not a lighted texture.

    tex->GetFDU (fdu);
    tex->GetFDV (fdv);
  }
  else
  {
    tmap2 = NULL;
    fdu = fdv = 0;
  }
  tex->GetWidth (tw2);
  tex->GetHeight (th2);

#ifdef STUPID_TEST
  tw2fp = (tw2 << 16) - 1;
  th2fp = (th2 << 16) - 1;
#endif
  tex->GetShiftU (shf_u);

  and_h <<= shf_w;
  shf_h = 16 - shf_w;
}

void Scan::dump (csGraphics3DSoftware* pG3D)
{
  IPolygonSet* piPS;
  char* szName;
  char* szParentName;

  poly->GetName(&szName);
  poly->GetParent(&piPS);
  piPS->GetName(&szParentName);
  FINAL_RELEASE(piPS);

  pG3D->SysPrintf (MSG_DEBUG_0, "------------------------------------------------\n");
  pG3D->SysPrintf (MSG_DEBUG_0, "Drawing polygon '%s' in '%s'.\n", szName, szParentName);
  if (tmap2)
  {
    pG3D->SysPrintf (MSG_DEBUG_0, "Using a texture from the texture cache.\n");
    pG3D->SysPrintf (MSG_DEBUG_0, "  Width=%d, height=%d\n", tw2, th2);
    pG3D->SysPrintf (MSG_DEBUG_0, "  fdu=%f, fdv=%f\n", fdu, fdv);
  }
  pG3D->SysPrintf (MSG_DEBUG_0, "The original unlighted texture:\n");
  pG3D->SysPrintf (MSG_DEBUG_0, "  Width=%d, height=%d\n", tw, th);
  pG3D->SysPrintf (MSG_DEBUG_0, "\n");
  pG3D->SysPrintf (MSG_DEBUG_0, "shf_u=%d, shf_w=%d, shf_h=%d\n", shf_u, shf_w, shf_h);
  pG3D->SysPrintf (MSG_DEBUG_0, "and_w=%d, and_h=%d\n", and_w, and_h);
  pG3D->SysPrintf (MSG_DEBUG_0, "\n");
  pG3D->SysPrintf (MSG_DEBUG_0, "M=%f, J1=%f, K1=%f\n", M, J1, K1);
  pG3D->SysPrintf (MSG_DEBUG_0, "dM=%f, dJ1=%f, dK1=%f\n", dM, dJ1, dK1);
  if (pG3D->do_debug)
  {
    pG3D->SysPrintf (MSG_DEBUG_0, "More information (debug info is enabled):\n");
    pG3D->SysPrintf (MSG_DEBUG_0, "  Trapezoid:\n");
    pG3D->SysPrintf (MSG_DEBUG_0, "      (%f,%f) - (%f,%f)\n", debug_sx1L, debug_sy1, debug_sx1R, debug_sy1);
    pG3D->SysPrintf (MSG_DEBUG_0, "      (%f,%f) - (%f,%f)\n", debug_sx2L, debug_sy2, debug_sx2R, debug_sy2);
    pG3D->SysPrintf (MSG_DEBUG_0, "  sy=%f, sxL=%f, sxR=%f\n", debug_sy, debug_sxL, debug_sxR);
    pG3D->SysPrintf (MSG_DEBUG_0, "  inv_z=%f, u_div_z=%f, v_div_z=%f\n",
    	debug_inv_z, debug_u_div_z, debug_v_div_z);
    pG3D->SysPrintf (MSG_DEBUG_0, "  1/z = (%f * sx) + (%f * sy) + %f\n", M, debug_N, debug_O);
    pG3D->SysPrintf (MSG_DEBUG_0, "  u/z = (%f * sx) + (%f * sy) + %f\n", J1, debug_J2, debug_J3);
    pG3D->SysPrintf (MSG_DEBUG_0, "  v/z = (%f * sx) + (%f * sy) + %f\n", K1, debug_K2, debug_K3);
    pG3D->SysPrintf (MSG_DEBUG_0, "  draw_scanline info:\n");
    pG3D->SysPrintf (MSG_DEBUG_0, "    xx=%d\n", debug_xx);
    pG3D->SysPrintf (MSG_DEBUG_0, "    uu=%d\n", debug_uu);
    pG3D->SysPrintf (MSG_DEBUG_0, "    vv=%d\n", debug_vv);
    pG3D->SysPrintf (MSG_DEBUG_0, "    uu1=%d\n", debug_uu1);
    pG3D->SysPrintf (MSG_DEBUG_0, "    vv1=%d\n", debug_vv1);
    pG3D->SysPrintf (MSG_DEBUG_0, "    duu=%d\n", debug_duu);
    pG3D->SysPrintf (MSG_DEBUG_0, "    dvv=%d\n", debug_dvv);
  }
  pG3D->SysPrintf (MSG_DEBUG_0, "------------------------------------------------\n");
}

//---------------------------------------------------------------------------
