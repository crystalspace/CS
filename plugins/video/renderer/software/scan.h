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

#ifndef SCANNER_H
#define SCANNER_H

#include "types.h"
#include "cscom/com.h"

#define INTER_MODE_SMART 0
#define INTER_MODE_STEP8 1
#define INTER_MODE_STEP16 2
#define INTER_MODE_STEP32 3

// @@@ Specifies boundary where texel filtering does not occur -- D.D.
#define BAILOUT_CONSTANT 32768

class csGraphics3DSoftware;
class csTexture;
class csTextureMMSoftware;
class csTextureManagerSoftware;
interface IPolygon3D;
interface IPolygonTexture;

/**
 * This structure is used to put information for the scanline drawer in a global
 * structure (for efficiency).
 */
class Scan
{
public:
  /// Interpolation step for semi-perspective correct texture mapping.
  static int INTERPOL_STEP;
  /// Interpolation step (shift-value) for semi-perspective correct texture mapping.
  static int INTERPOL_SHFT;

  /// Interpolation mode.
  static int inter_mode;

  /// Fog color
  static int fog_red, fog_green, fog_blue;
  /// Fog density
  static unsigned long fog_density;

  /// A pointer to the texture.
  static csTextureMMSoftware* texture;

  /// The lighted texture bitmap from the texture cache.
  static unsigned char* tmap2;
  /// Width of the texture from the texture cache.
  static int tw2;
  /// Height of the texture from the texture cache.
  static int th2;
  /// Difference with u for untiled textures.
  static float fdu;
  /// Difference with v for untiled textures.
  static float fdv;
  /// Texture width in 16:16 fixed-point format - 1
  static int tw2fp;
  /// Texture height in 16:16 fixed-point format - 1
  static int th2fp;

  /// The unlighted texture bitmap.
  static unsigned char* tmap;
  /// Width of unlighted texture.
  static int tw;
  /// Height of unlighted texture.
  static int th;

  /// SJI: dynamic lighting.
  static unsigned char* curLightTable;

  // The following fields are used by the polygon drawer
  // and contain information fot calculating the 1/z, u/z, and v/z linear
  // equations.
  /// Difference value for every horizontal dx.
  static float M;
  /// Difference value for every horizontal dx.
  static float J1;
  /// Difference value for every horizontal dx.
  static float K1;
  /// Difference for every 16 pixels.
  static float dM;
  /// Difference for every 16 pixels.
  static float dJ1;
  /// Difference for every 16 pixels.
  static float dK1;

  /// Mean color value.
  static int flat_color;

  /// Alpha mask for 16-bit renderer.
  static UShort alpha_mask;
  /// General alpha factor for 16-bit renderer (0 to 255).
  static int alpha_fact;

  ///
  static int shf_u;

  // The following fields are only used when drawing
  // unlighted tiled textures (the draw_scanline_..._map_...
  // routines don't need them).
  ///
  static int shf_w;
  ///
  static int and_w;
  ///
  static int shf_h;
  ///
  static int and_h;

  // The following information is used only by Scan::dump()
  // in order to do some debugging whenever the program crashes.
  static IPolygon3D* poly;
  static float debug_sx1L;
  static float debug_sx1R;
  static float debug_sx2L;
  static float debug_sx2R;
  static float debug_sy1;
  static float debug_sy2;
  static float debug_sy;
  static float debug_sxL;
  static float debug_sxR;
  static float debug_inv_z;
  static float debug_u_div_z;
  static float debug_v_div_z;
  static float debug_N;
  static float debug_O;
  static float debug_J2;
  static float debug_J3;
  static float debug_K2;
  static float debug_K3;
  static int volatile debug_xx, debug_uu, debug_vv, debug_uu1, debug_vv1, debug_duu, debug_dvv;

public:

  /// Draw one horizontal scanline with no texture mapping.
  static void draw_scanline_flat (int xx, unsigned char* d, unsigned long* z_buf,
  			  float inv_z, float u_div_z, float v_div_z);
  /// Draw one horizontal scanline with no texture mapping (but use Z buffer).
  static void draw_scanline_z_buf_flat (int xx, unsigned char* d, unsigned long* z_buf,
  			  float inv_z, float u_div_z, float v_div_z);
  /// Draw one horizontal scanline (no lighting).
  static void draw_scanline (int xx, unsigned char* d, unsigned long* z_buf,
  			  float inv_z, float u_div_z, float v_div_z);
  /// Draw one horizontal scanline (no lighting, private mode).
  static void draw_scanline_private (int xx, unsigned char* d, unsigned long* z_buf,
  			  float inv_z, float u_div_z, float v_div_z);
  /// Draw one horizontal scanline for fog.
  static void draw_scanline_fog (int xx, unsigned char* d, unsigned long* z_buf,
  			  float inv_z, float u_div_z, float v_div_z);
  /// Draw one horizontal scanline (lighting and alpha transparency).
  static void draw_scanline_map_alpha1 (int xx, unsigned char* d, unsigned long* z_buf,
			  float inv_z, float u_div_z, float v_div_z);
  /// Draw one horizontal scanline (lighting and alpha transparency).
  static void draw_scanline_map_alpha2 (int xx, unsigned char* d, unsigned long* z_buf,
			  float inv_z, float u_div_z, float v_div_z);
  /// Draw one horizontal scanline (lighting).
  static void draw_scanline_map (int xx, unsigned char* d, unsigned long* z_buf,
			  float inv_z, float u_div_z, float v_div_z);
  /// Draw one horizontal scanline (lighting and filtering).
  static void draw_scanline_map_filter (int xx, unsigned char* d, unsigned long* z_buf,
			  float inv_z, float u_div_z, float v_div_z);
  /// Draw one horizontal scanline (transparent and no lighting).
  static void draw_scanline_transp (int xx, unsigned char* d, unsigned long* z_buf,
			  float inv_z, float u_div_z, float v_div_z);
  /// Draw one horizontal scanline (transparent and no lighting, private mode).
  static void draw_scanline_transp_private (int xx, unsigned char* d, unsigned long* z_buf,
			  float inv_z, float u_div_z, float v_div_z);
  /// Draw one horizontal scanline (transparent with lighting).
  static void draw_scanline_transp_map (int xx, unsigned char* d, unsigned long* z_buf,
			  float inv_z, float u_div_z, float v_div_z);
  /// Draw one horizontal scanline (Z buffer and no lighting).
  static void draw_scanline_z_buf (int xx, unsigned char* d, unsigned long* z_buf,
			  float inv_z, float u_div_z, float v_div_z);
  /// Draw one horizontal scanline (Z buffer and no lighting, private mode).
  static void draw_scanline_z_buf_private (int xx, unsigned char* d, unsigned long* z_buf,
			  float inv_z, float u_div_z, float v_div_z);
  /// Draw one horizontal scanline (Z buffer and lighting).
  static void draw_scanline_z_buf_map (int xx, unsigned char* d, unsigned long* z_buf,
			  float inv_z, float u_div_z, float v_div_z);
  /// Draw one horizontal scanline (lighting and uniform lighting).
  static void draw_scanline_map_light (int xx, unsigned char* d, unsigned long* z_buf,
			  float inv_z, float u_div_z, float v_div_z);
  /// Draw one horizontal scanline (Z buffer, lighting and uniform lighting).
  static void draw_scanline_z_buf_map_light (int xx, unsigned char* d, unsigned long* z_buf,
			  float inv_z, float u_div_z, float v_div_z);

#ifdef DO_MMX
  /// Draw one horizontal scanline (lighting) using MMX
  static void mmx_draw_scanline_map (int xx, unsigned char* d, unsigned long* z_buf,
			  float inv_z, float u_div_z, float v_div_z);
  /// Draw one horizontal scanline (no lighting) using MMX
  static void mmx_draw_scanline (int xx, unsigned char* d, unsigned long* z_buf,
  			  float inv_z, float u_div_z, float v_div_z);
#endif

  ///
  static void init_draw (csGraphics3DSoftware* g3d, IPolygon3D* p, IPolygonTexture* tex,
  	csTextureMMSoftware* texture, csTexture* untxt);
  /// Do a debugging dump.
  static void dump (csGraphics3DSoftware* pG3D);

  /*
   * The following methods are used by DrawPolygonQuick() and do not require
   * perspective-correct texture mapping. They do not require init_draw ()
   * to be called before using.
   */
  /// Draw a perspective-incorrect texture mapped polygon scanline
  static void draw_pi_scanline (void *dest, int len, long *zbuff,
                                long u, long du, long v, long dv, long z, long dz,
                                unsigned char *bitmap, int bitmap_log2w);
  /// Draw a perspective-incorrect texture mapped polygon scanline. Z fill only
  static void draw_pi_scanline_zfill (void *dest, int len, long *zbuff,
                                long u, long du, long v, long dv, long z, long dz,
                                unsigned char *bitmap, int bitmap_log2w);
#ifdef DO_MMX
  /// Draw a perspective-incorrect texture mapped polygon scanline using MMX
  static void mmx_draw_pi_scanline (void *dest, int len, long *zbuff,
                                long u, long du, long v, long dv, long z, long dz,
                                unsigned char *bitmap, int bitmap_log2w);
#endif
};

#endif /*SCANNER_H*/
