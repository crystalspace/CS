/*
    Copyright (C) 1998-2000 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CS_SCAN_H__
#define __CS_SCAN_H__

#include "csutil/scf.h"
#include "csgfx/rgbpixel.h"

struct csPolyTextureMapping;
  
namespace cspluginSoft3d
{

//---//---//---//---//---//---//---//---//---//---/ Forward declarations //---//

class csSoftwareTexture;
class csSoftwareTextureHandle;
class csSoftwareGraphics3DCommon;
class csSoftRendererLightmap;

//---//---//---//---//---//---//---//---//---//---//---//---// Constants //---//

#define INTER_MODE_SMART	0
#define INTER_MODE_STEP8	1
#define INTER_MODE_STEP16	2
#define INTER_MODE_STEP32	3

// Specifies boundary where texel filtering does not occur
#define BAILOUT_CONSTANT	32768

// A coefficient for planar fog density: bigger is denser
#define PLANAR_FOG_DENSITY_COEF	6

/// At this point csQround (255 * exp (-float (i) / 256.)) reaches zero
#define EXP_256_SIZE			1420
/// Same for csQround (32 * exp (-float (i) / 256.))
#define EXP_32_SIZE			1065

/// Blending table indices
#define BLENDTABLE_ADD                  0x00
#define BLENDTABLE_MULTIPLY             0x01
#define BLENDTABLE_MULTIPLY2            0x02
#define BLENDTABLE_ALPHA25              0x03
#define BLENDTABLE_ALPHA50              0x04
#define BLENDTABLE_ALPHA75              0x05
#define NUMBLENDINGTABLES               6

//---//---//---//---//---//---//---//---//---// Special Pixel Processing //---//

/*
 * These macros allow special processing of 32-bit pixel data.  For performance
 * reasons, the software renderer has a fixed notion about the layout of an RGB
 * pixel (BGRA on little-endian, and ABGR on big-endian).  If you need to store
 * textures and have the final rendered image use a different pixel layout,
 * then you can use these macros to transform the layout to and from the the
 * canonical layout expected by the software renderer.  The PREPROC macro
 * should be defined to transform your custom pixel layout into the renderer's
 * canonical form, and the POSTPROC macro should transform the data in the
 * reverse direction.  The SHIFT_ADJUST macro should be used to transform pixel
 * data shift amounts from your custom form to the canonical form.  To really
 * understand how these macros affect the rendering process, examine the source
 * code which invokes them.
 */
#define R8G8B8_PIXEL_PREPROC(x)  (x)
#define R8G8B8_PIXEL_POSTPROC(x) (x)
#define R8G8B8_SHIFT_ADJUST(x)   (x)

//---//---//---//---//---//---//---//---//---//---//---/ Precomputed data /---//

/*
 * Structure with precalculated data<p>
 * There is generally one global structure of this type.
 * However, to make assembler life easier we do it in a structure
 * rather than defining many static variables.<p>
 * <b>WARNING:</p> Do not forget to synchronise all changes
 * made to this structure with i386/scan.ash!
 */
struct csScanSetup
{
  /// Interpolation step for semi-perspective correct texture mapping
  int InterpolStep;
  /// Interpolation step (shift-value) for semi-perspective correct texture mapping
  int InterpolShift;
  /// Interpolation mode
  int InterpolMode;

  /// Fog color
  uint32 FogR;
  uint32 FogG;
  uint32 FogB;
  /// The fog pixel (R|G|B for true/hicolor) or index into palette of fog color
  uint32 FogPix;
  /// Fog density
  unsigned int FogDensity;

  /// A pointer to the texture.
  csSoftwareTextureHandle *Texture;

  /// The lighted texture bitmap from the texture cache.
  unsigned char *bitmap2;
  /// Width of the texture from the texture cache.
  int tw2;
  /// Height of the texture from the texture cache.
  int th2;
  /// Texture width in 16:16 fixed-point format - 1
  int tw2fp;
  /// Texture height in 16:16 fixed-point format - 1
  int th2fp;
  /// the difference of U-coordinates between cached texture and alpha map
  int amap_uofs;
  /// the difference of V-coordinates between cached texture and alpha map
  int amap_vofs;

  /// The unlighted texture bitmap.
  unsigned char *bitmap;
  /// Width of unlighted texture.
  int tw;
  /// Height of unlighted texture.
  int th;

  /**
   * The following fields are used by the polygon drawer
   * and contain information fot calculating the 1/z, u/z, and v/z linear
   * equations.
   */

  /// Difference value for every horizontal dx.
  float M;
  /// Difference value for every horizontal dx.
  float J1;
  /// Difference value for every horizontal dx.
  float K1;
  /// Difference for every 16 pixels.
  float dM;
  /// Difference for every 16 pixels.
  float dJ1;
  /// Difference for every 16 pixels.
  float dK1;

  /// Mean color value.
  int FlatColor;
  /// R/G/B components of flatcolor
  csRGBpixel FlatRGB;

  /// Alpha mask for 16-bit renderer.
  unsigned int AlphaMask;
  /// General alpha factor for 16-bit renderer (0 to 255).
  int AlphaFact;

  /// log2 (texture_u)
  int shf_u;

  /**
   * The following fields are only used when drawing
   * unlighted tiled textures (the scan_..._map_...
   * routines don't need them).
   */

  /// log2(texture_width)
  int shf_w;
  /// 1 << shf_w - 1
  int and_w;
  /// log2(texture_height)
  int shf_h;
  /// 1 << shf_h - 1
  int and_h;
  /// U at the origin of the texture
  int min_u;
  /// V at the origin of the texture
  int min_v;

  /// Actual texture palette
  csRGBpixel *TexturePalette;
  /// 8-bit to native pixel format conversion table
  void *PaletteTable;
  /// Set up by poly renderer to alpha blending table
  uint8 *AlphaMap;

  /// Current blending table
  unsigned char *BlendTable;

  /**
   * A table of 4096 1/z values where z is in fixed-point 0.12 format
   * Used in fog routines to get the real value of Z. The result is
   * an 8.24 fixed-point number.
   */
  unsigned int *one_div_z;

  /**
   * A table of exp(x) in the range 0..255; x == 0..EXP_256_SIZE
   */
  unsigned char *exp_256;

  /**
   * This table contains eight subtables - one for each available blending
   * mode (see BLENDTABLE_XXX constants). Since SRC can be twice brighter
   * than maximum, BlendingTable has a twice larger SRC dimension (i.e.
   * it is a (BLENDTABLE_MAX*2 x BLENDTABLE_MAX) matrix);
   */
  unsigned char *BlendingTable [NUMBLENDINGTABLES];

  /// the blending table for proc. textures.
  unsigned char *BlendingTableProc [NUMBLENDINGTABLES];
};

// The following should be accessible from assembly, so avoid C++ mangling
extern "C"
{

/// The only instance of this structure
extern csScanSetup Scan;

/// The interface definition for all scan_XXX routines
typedef void (csDrawScanline)
  (int xx, unsigned char* d, uint32* z_buf, float inv_z,
   float u_div_z, float v_div_z);
/// The interface definition for all scan_pi_XXX routines
typedef void (csDrawPIScanline)
  (void *dest, int len, uint32 *zbuff, int32 u, int32 du, int32 v, int32 dv,
   float z, float dz, unsigned char *bitmap, int bitmap_log2w);
/// The interface definition for all scan_pi_XXX_gou_XXX routines
typedef void (csDrawPIScanlineGouraud)
  (void *dest, int len, uint32 *zbuff, int32 u, int32 du, int32 v, int32 dv,
   float z, float dz, unsigned char *bitmap, int bitmap_log2w,
   uint32 r, uint32 g, uint32 b, int32 dr, int32 dg, int32 db, bool clamp);

//---//---//---//---//---//---//---//---//---//---//---//---//- Routines //---//

/// Initialize all required tables
void csScan_Initialize ();
/// Free all tables
void csScan_Finalize ();
/// Calculate blending tables (should be called each time pixel format changes)
void csScan_CalcBlendTables (unsigned char *BlendingTable[], int rbits,
  int gbits, int bbits);
/// Initialize the scanline variables
void csScan_InitDraw (int MipMap, csSoftwareGraphics3DCommon* g3d,
  csPolyTextureMapping* tmapping,
  csSoftRendererLightmap* rlm, csSoftwareTextureHandle* texture,
  csSoftwareTexture* untxt);
/// Initialize the scanline variables for DrawPolygonFX
void csScan_InitDrawFX (csSoftwareTextureHandle* texture, csSoftwareTexture *untxt);
/// Pixel-depth independent routine
csDrawScanline csScan_scan_zfil;
/// Pixel-depth independent routine
csDrawPIScanline csScan_scan_pi_zfil;

/*
 * The following macro will declare all variations of a
 * perspective-correct scanline routine.
 */
#define SCAN_ROUTINE(pfmt,features)					\
csDrawScanline csScan_##pfmt##_scan_##features##_znone;			\
csDrawScanline csScan_##pfmt##_scan_##features##_zfil;			\
csDrawScanline csScan_##pfmt##_scan_##features##_zuse;			\
csDrawScanline csScan_##pfmt##_scan_##features##_ztest;

/*
 * The following macro is used to define all variations of a
 * perspective-incorrect scanline routine
 */
#define PI_SCAN_ROUTINE(pfmt,features)					\
csDrawPIScanline csScan_##pfmt##_scan_pi_##features##_znone;		\
csDrawPIScanline csScan_##pfmt##_scan_pi_##features##_zfil;		\
csDrawPIScanline csScan_##pfmt##_scan_pi_##features##_zuse;		\
csDrawPIScanline csScan_##pfmt##_scan_pi_##features##_ztest;

/*
 * This macros is used to declare several variations of a Gouraud-shaded
 * perspective-incorrect scanline routine.
 */
#define PIG_SCAN_ROUTINE(pfmt,features)					\
csDrawPIScanlineGouraud csScan_##pfmt##_scan_pi_##features##_znone;	\
csDrawPIScanlineGouraud csScan_##pfmt##_scan_pi_##features##_zfil;	\
csDrawPIScanlineGouraud csScan_##pfmt##_scan_pi_##features##_zuse;	\
csDrawPIScanlineGouraud csScan_##pfmt##_scan_pi_##features##_ztest;

//---//---//---//---//---//---//---//---//---//- 16-bit drawing routines //---//

/// Draw one horizontal scanline with no texture mapping
SCAN_ROUTINE (16, flat)
/// Draw one horizontal scanline (no lighting).
SCAN_ROUTINE (16, tex)
/// Draw one horizontal scanline (lighting).
SCAN_ROUTINE (16, map)
/// Draw one horizontal scanline (transparent and no lighting).
SCAN_ROUTINE (16, tex_key)
/// Draw one horizontal scanline (transparent with lighting).
SCAN_ROUTINE (16, map_key)
/// Draw one horizontal scanline (alphamap and no lighting).
SCAN_ROUTINE (16_555, tex_alpha)
SCAN_ROUTINE (16_565, tex_alpha)
/// Draw one horizontal scanline (alphamap with lighting).
SCAN_ROUTINE (16_555, map_alpha)
SCAN_ROUTINE (16_565, map_alpha)
/// Draw one horizontal scanline (fx and no lighting).
SCAN_ROUTINE (16_555, tex_fx)
SCAN_ROUTINE (16_565, tex_fx)
/// Draw one horizontal scanline (fx with lighting).
SCAN_ROUTINE (16_555, map_fx)
SCAN_ROUTINE (16_565, map_fx)

/// Draw one horizontal scanline (lighting and filtering).
csDrawScanline csScan_16_scan_map_filt_ztest;
/// Draw one horizontal scanline (lighting and filtering).
csDrawScanline csScan_16_scan_map_filt_znone;
/// Draw one horizontal scanline (lighting and filtering).
csDrawScanline csScan_16_scan_map_filt_zfil;
/// Draw one horizontal scanline (lighting and more filtering).
csDrawScanline csScan_16_555_scan_map_filt2_znone;
csDrawScanline csScan_16_565_scan_map_filt2_znone;
/// Draw one horizontal scanline (lighting and more filtering).
csDrawScanline csScan_16_555_scan_map_filt2_zfil;
csDrawScanline csScan_16_565_scan_map_filt2_zfil;
/// Draw one horizontal scanline (lighting and more filtering, Z-check).
csDrawScanline csScan_16_555_scan_map_filt2_zuse;
csDrawScanline csScan_16_565_scan_map_filt2_zuse;
/// Draw one horizontal scanline (lighting and more filtering, Z-check).
csDrawScanline csScan_16_555_scan_map_filt2_ztest;
csDrawScanline csScan_16_565_scan_map_filt2_ztest;

/// Draw one horizontal scanline for fog.
csDrawScanline csScan_16_555_scan_fog;
csDrawScanline csScan_16_565_scan_fog;
/// Draw one horizontal scanline for fog assuming the camera is in fog.
csDrawScanline csScan_16_555_scan_fog_view;
csDrawScanline csScan_16_565_scan_fog_view;

/// Draw one horizontal scanline (lighting and fixed alpha transparency).
csDrawScanline csScan_16_scan_map_fixalpha50;
/// Draw one horizontal scanline (lighting and fixed alpha transparency with alphamap).
csDrawScanline csScan_16_555_scan_map_fixalpha50_alphamap;
csDrawScanline csScan_16_565_scan_map_fixalpha50_alphamap;
/// Draw one horizontal scanline (lighting and fixed alpha transparency and keycolor).
csDrawScanline csScan_16_scan_map_fixalpha50_key;
/// Draw one horizontal scanline (lighting and fixed alpha transparency). General case.
csDrawScanline csScan_16_555_scan_map_fixalpha;
csDrawScanline csScan_16_565_scan_map_fixalpha;
/// Draw one horizontal scanline (lighting and fixed alpha transparency and keycolor). General case.
csDrawScanline csScan_16_555_scan_map_fixalpha_key;
csDrawScanline csScan_16_565_scan_map_fixalpha_key;
/// Draw one horizontal scanline (lighting and fixed alpha transparency and alphamap). General case.
csDrawScanline csScan_16_555_scan_map_fixalpha_alphamap;
csDrawScanline csScan_16_565_scan_map_fixalpha_alphamap;

/// Draw a flat-lighted perspective-incorrect line
PI_SCAN_ROUTINE (16, flat)
/// Draw a perspective-incorrect texture mapped polygon scanline
PI_SCAN_ROUTINE (16, tex)
/// Draw a perspective-incorrect texture mapped polygon scanline with color keying
PI_SCAN_ROUTINE (16, tex_key)
/// Draw a perspective-incorrect texture mapped polygon scanline
PI_SCAN_ROUTINE (16, tile_tex)
/// Draw a perspective-incorrect texture mapped polygon scanline with color keying
PI_SCAN_ROUTINE (16, tile_tex_key)
/// Draw a flat-lighted perspective-incorrect with table-driven effects
PI_SCAN_ROUTINE (16_555, flat_fx)
PI_SCAN_ROUTINE (16_565, flat_fx)
/// Draw a perspective-incorrect texture mapped polygon scanline with table-driven effects
PI_SCAN_ROUTINE (16_555, tex_fx)
PI_SCAN_ROUTINE (16_565, tex_fx)
/// Draw a perspective-incorrect texture mapped polygon scanline with table-driven effects and color keying
PI_SCAN_ROUTINE (16_555, tex_fxkey)
PI_SCAN_ROUTINE (16_565, tex_fxkey)
/// Draw a perspective-incorrect texture mapped polygon scanline with table-driven effects
PI_SCAN_ROUTINE (16_555, tile_tex_fx)
PI_SCAN_ROUTINE (16_565, tile_tex_fx)
/// Draw a perspective-incorrect texture mapped polygon scanline with table-driven effects and color keying
PI_SCAN_ROUTINE (16_555, tile_tex_fxkey)
PI_SCAN_ROUTINE (16_565, tile_tex_fxkey)
/// Draw a flat-shaded alpha-mapped texture
PI_SCAN_ROUTINE (16_555, tex_alpha)
PI_SCAN_ROUTINE (16_565, tex_alpha)

/// Draw a single-color Gouraud-shaded polygon
PIG_SCAN_ROUTINE (16_555, flat_gou)
PIG_SCAN_ROUTINE (16_565, flat_gou)
/// Draw a perspective-incorrect texture mapped polygon scanline with Gouraud shading
PIG_SCAN_ROUTINE (16_555, tex_gou)
PIG_SCAN_ROUTINE (16_565, tex_gou)
/// Perspective-incorrect textured polygon with Gouraud shading and color keying
PIG_SCAN_ROUTINE (16_555, tex_goukey)
PIG_SCAN_ROUTINE (16_565, tex_goukey)
/// Draw a perspective-incorrect texture mapped polygon scanline with Gouraud shading
PIG_SCAN_ROUTINE (16_555, tile_tex_gou)
PIG_SCAN_ROUTINE (16_565, tile_tex_gou)
/// Perspective-incorrect textured polygon with Gouraud shading and color keying
PIG_SCAN_ROUTINE (16_555, tile_tex_goukey)
PIG_SCAN_ROUTINE (16_565, tile_tex_goukey)
/// Draw a single-color Gouraud-shaded polygon with table-driven effects
PIG_SCAN_ROUTINE (16_555, flat_goufx)
PIG_SCAN_ROUTINE (16_565, flat_goufx)
/// Draw a perspective-incorrect textured polygon scanline with table-driven effects
PIG_SCAN_ROUTINE (16_555, tex_goufx)
PIG_SCAN_ROUTINE (16_565, tex_goufx)
/// Draw a perspective-incorrect polygon scanline with various effects and color keying
PIG_SCAN_ROUTINE (16_555, tex_goufxkey)
PIG_SCAN_ROUTINE (16_565, tex_goufxkey)
/// Draw a perspective-incorrect textured polygon scanline with table-driven effects
PIG_SCAN_ROUTINE (16_555, tile_tex_goufx)
PIG_SCAN_ROUTINE (16_565, tile_tex_goufx)
/// Draw a perspective-incorrect polygon scanline with various effects and color keying
PIG_SCAN_ROUTINE (16_555, tile_tex_goufxkey)
PIG_SCAN_ROUTINE (16_565, tile_tex_goufxkey)

#ifdef CS_HAVE_MMX
/// Draw one horizontal scanline (lighting) using MMX
csDrawScanline csScan_16_mmx_scan_map_zfil;
/// Draw one horizontal scanline (no lighting).
csDrawScanline csScan_16_mmx_scan_tex_zfil;
/// Draw a perspective-incorrect texture mapped polygon scanline
csDrawPIScanline csScan_16_mmx_scan_pi_tex_zuse;
#endif

//---//---//---//---//---//---//---//---//---//- 32-bit scanline drawers //---//

/// Draw one horizontal scanline with no texture mapping
SCAN_ROUTINE (32, flat)
/// Draw one horizontal scanline (no lighting).
SCAN_ROUTINE (32, tex)
/// Draw one horizontal scanline (lighting).
SCAN_ROUTINE (32, map)
/// Draw one horizontal scanline (transparent and no lighting).
SCAN_ROUTINE (32, tex_key)
/// Draw one horizontal scanline (transparent with lighting).
SCAN_ROUTINE (32, map_key)
/// Draw one horizontal scanline (alphamap and no lighting).
SCAN_ROUTINE (32, tex_alpha)
/// Draw one horizontal scanline (alphamap+key and no lighting).
SCAN_ROUTINE (32, tex_alphakey)
/// Draw one horizontal scanline (alphamap with lighting).
SCAN_ROUTINE (32, map_alpha)
/// Draw one horizontal scanline (alphamap+key with lighting).
SCAN_ROUTINE (32, map_alphakey)
/// Draw one horizontal scanline (fx and no lighting).
SCAN_ROUTINE (32, tex_fx)
/// Draw one horizontal scanline (fx with lighting).
SCAN_ROUTINE (32, map_fx)

/// Same as above but do bilinear filtering (Z-fill)
csDrawScanline csScan_32_scan_map_filt2_znone;
/// Same as above but do bilinear filtering (Z-fill)
csDrawScanline csScan_32_scan_map_filt2_zfil;
/// Same as above but do bilinear filtering (Z-check)
csDrawScanline csScan_32_scan_map_filt2_zuse;
/// Same as above but do bilinear filtering (Z-check)
csDrawScanline csScan_32_scan_map_filt2_ztest;

/// Draw one horizontal scanline for fog
csDrawScanline csScan_32_scan_fog;
/// Draw one horizontal scanline for fog assuming the camera is in fog
csDrawScanline csScan_32_scan_fog_view;

/// Draw one horizontal scanline (lighting and fixed alpha transparency).
csDrawScanline csScan_32_scan_map_fixalpha50;
/// Draw one horizontal scanline (lighting and fixed alpha transparency with keycolor).
csDrawScanline csScan_32_scan_map_fixalpha50_key;
/// Draw one horizontal scanline (lighting and fixed alpha transparency with alphamap).
csDrawScanline csScan_32_scan_map_fixalpha50_alphamap;
/// Draw one horizontal scanline (lighting and fixed alpha transparency). General case.
csDrawScanline csScan_32_scan_map_fixalpha;
/// Draw one horizontal scanline (lighting and fixed alpha transparency with keycolor). General case.
csDrawScanline csScan_32_scan_map_fixalpha_key;
/// Draw one horizontal scanline (lighting and fixed alpha transparency with alphamap). General case.
csDrawScanline csScan_32_scan_map_fixalpha_alphamap;

/// Draw a flat-lighted perspective-incorrect line
PI_SCAN_ROUTINE (32, flat)
/// Draw a perspective-incorrect texture mapped polygon scanline
PI_SCAN_ROUTINE (32, tex)
/// Draw a perspective-incorrect texture mapped polygon scanline with color keying
PI_SCAN_ROUTINE (32, tex_key)
/// Draw a perspective-incorrect texture mapped polygon scanline
PI_SCAN_ROUTINE (32, tile_tex)
/// Draw a perspective-incorrect texture mapped polygon scanline with color keying
PI_SCAN_ROUTINE (32, tile_tex_key)
/// Draw a flat-lighted perspective-incorrect with table-driven effects
PI_SCAN_ROUTINE (32, flat_fx)
/// Draw a perspective-incorrect texture mapped polygon scanline with table-driven effects
PI_SCAN_ROUTINE (32, tex_fx)
/// Draw a perspective-incorrect texture mapped polygon scanline with table-driven effects and color keying
PI_SCAN_ROUTINE (32, tex_fxkey)
/// Draw a perspective-incorrect texture mapped polygon scanline with table-driven effects
PI_SCAN_ROUTINE (32, tile_tex_fx)
/// Draw a perspective-incorrect texture mapped polygon scanline with table-driven effects and color keying
PI_SCAN_ROUTINE (32, tile_tex_fxkey)
/// Draw a flat-shaded alpha-mapped texture
PI_SCAN_ROUTINE (32, tex_alpha)

/// Draw a single-color Gouraud-shaded polygon
PIG_SCAN_ROUTINE (32, flat_gou)
/// Draw a perspective-incorrect texture mapped polygon scanline with Gouraud shading
PIG_SCAN_ROUTINE (32, tex_gou)
/// Perspective-incorrect textured polygon with Gouraud shading and color keying
PIG_SCAN_ROUTINE (32, tex_goukey)
/// Draw a perspective-incorrect texture mapped polygon scanline with Gouraud shading
PIG_SCAN_ROUTINE (32, tile_tex_gou)
/// Perspective-incorrect textured polygon with Gouraud shading and color keying
PIG_SCAN_ROUTINE (32, tile_tex_goukey)
/// Draw a single-color Gouraud-shaded polygon with table-driven effects
PIG_SCAN_ROUTINE (32, flat_goufx)
/// Draw a perspective-incorrect textured polygon scanline with table-driven effects
PIG_SCAN_ROUTINE (32, tex_goufx)
/// Draw a perspective-incorrect polygon scanline with various effects and color keying
PIG_SCAN_ROUTINE (32, tex_goufxkey)
/// Draw a perspective-incorrect textured polygon scanline with table-driven effects
PIG_SCAN_ROUTINE (32, tile_tex_goufx)
/// Draw a perspective-incorrect polygon scanline with various effects and color keying
PIG_SCAN_ROUTINE (32, tile_tex_goufxkey)

#if defined (CS_HAVE_MMX)
/// Draw a perspective-incorrect texture mapped polygon scanline using MMX
csDrawScanline csScan_32_mmx_scan_map_zfil;
/// Draw one horizontal scanline (no lighting) using MMX
csDrawScanline csScan_32_mmx_scan_tex_zfil;
#endif

} // extern "C"

} // namespace cspluginSoft3d

#endif // __CS_SCAN_H__
