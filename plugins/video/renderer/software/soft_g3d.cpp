/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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

#include <stdarg.h>

#include "sysdef.h"
#include "qint.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/transfrm.h"
#include "csgeom/polyclip.h"
#include "csutil/inifile.h"
#include "soft_g3d.h"
#include "scan.h"
#include "tcache.h"
#include "soft_txt.h"
#include "ipolygon.h"
#include "isystem.h"
#include "igraph2d.h"
#include "ilghtmap.h"

#if defined (DO_MMX)
#  include "cs3d/software/i386/cpuid.h"
#endif

#define SysPrintf System->Printf

int csGraphics3DSoftware::filter_bf = 1;

//-------------------------- The indices into arrays of scanline routines ------

/*
 *  The rules for scanproc index name building:
 *  Curly brackets means optional name components
 *  Square brackets denote enforced name components
 *  Everything outside brackets is a must
 *
 *  SCANPROC{Persp}{Gouraud}_{Source_{Smode_}}{Zmode}
 *
 *  Persp       = PI for perspective-incorrect routines
 *  Gouraud     = G for Gouraud-shading routines
 *  Source      = TEX for non-lightmapped textures
 *                MAP for lightmapped textures
 *                FLAT for flat-shaded
 *                FOG for drawing fog
 *  SMode       = KEY for "key-color" source pixel removal
 *                GOURAUD for Gouraud-shading applied to the texture
 *  Zmode       = ZUSE for polys that are tested against Z-buffer (and fills)
 *                ZFIL for polys that just fills Z-buffer without testing
 *
 *  Example:
 *      SCANPROC_TEX_ZFIL
 *              scanline procedure for drawing a non-lightmapped
 *              texture with Z-fill
 *  Note:
 *      For easier runtime decisions odd indices use Z buffer
 *      while even indices fills Z-buffer (if appropiate)
 */
#define SCANPROC_FLAT_ZFIL              0x00
#define SCANPROC_FLAT_ZUSE              0x01
#define SCANPROC_TEX_ZFIL               0x02
#define SCANPROC_TEX_ZUSE               0x03
#define SCANPROC_MAP_ZFIL               0x04
#define SCANPROC_MAP_ZUSE               0x05
#define SCANPROC_TEX_KEY_ZFIL           0x06
#define SCANPROC_TEX_KEY_ZUSE           0x07
#define SCANPROC_MAP_KEY_ZFIL           0x08
#define SCANPROC_MAP_KEY_ZUSE           0x09
// these do not have "zuse" counterparts
#define SCANPROC_ZFIL                   0x10
#define SCANPROC_FOG                    0x11
#define SCANPROC_FOG_VIEW               0x12

#define SCANPROCPI_FLAT_ZFIL            0x00
#define SCANPROCPI_FLAT_ZUSE            0x01
#define SCANPROCPI_TEX_ZFIL             0x02
#define SCANPROCPI_TEX_ZUSE             0x03

// Gouraud-shaded PI routines should have same indices
// as their non-Gouraud counterparts
#define SCANPROCPI_FLAT_GOURAUD_ZFIL    0x00
#define SCANPROCPI_FLAT_GOURAUD_ZUSE    0x01
#define SCANPROCPI_TEX_GOURAUD_ZFIL     0x02
#define SCANPROCPI_TEX_GOURAUD_ZUSE     0x03

// Pointers to scanline drawing with effects.
#define SCANPROCPIFX_ZUSE               0x00
#define SCANPROCPIFX_ZFIL               0x01
#define SCANPROCPIFX_TRANSP_ZUSE        0x02
#define SCANPROCPIFX_TRANSP_ZFIL        0x03

///---------------------------------------------------------------------------

IMPLEMENT_FACTORY (csGraphics3DSoftware)

EXPORT_CLASS_TABLE (soft3d)
  EXPORT_CLASS (csGraphics3DSoftware, "crystalspace.graphics3d.software",
    "Software 3D graphics driver for Crystal Space")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE (csGraphics3DSoftware)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics3D)
  IMPLEMENTS_EMBEDDED_INTERFACE (iConfig)
IMPLEMENT_IBASE_END

#if defined (OS_LINUX)
char* get_software_2d_driver ()
{
  if (getenv ("GGI_DISPLAY"))
    return SOFTWARE_2D_DRIVER_GGI;
  else if (getenv ("DISPLAY"))
    return SOFTWARE_2D_DRIVER_XLIB;
  else
    return SOFTWARE_2D_DRIVER_SVGA;
}
#elif defined (OS_UNIX) && !defined (OS_BE)
// by the way, other unices has SVGALib support too... through GGI ;-)
char* get_software_2d_driver ()
{
  if (getenv ("GGI_DISPLAY"))
    return SOFTWARE_2D_DRIVER_GGI;
  else
    return SOFTWARE_2D_DRIVER_XLIB;
}
#endif

csGraphics3DSoftware::csGraphics3DSoftware (iBase *iParent) : G2D (NULL)
{
  CONSTRUCT_IBASE (iParent);
  CONSTRUCT_EMBEDDED_IBASE (scfiConfig);

  config = new csIniFile ("soft3d.cfg");

  tcache = NULL;
  texman = NULL;

  clipper = NULL;

#ifdef DO_MMX
  do_mmx = true;
#endif
  do_lighting = true;
  do_transp = true;
  do_textured = true;
  do_interlaced = -1;
  ilace_fastmove = false;
  do_bilin_filt = 0;
  do_transp = true;
  do_textured = true;
  do_smaller_rendering = false;
  smaller_buffer = NULL;
  rstate_mipmap = 0;
  rstate_gouraud = true;
  rstate_specular = true;
  rstate_dither = false;

  dbg_max_polygons_to_draw = 2000000000;        // After 2 billion polygons we give up :-)

  fogMode = G3DFOGMETHOD_ZBUFFER;
  //fogMode = G3DFOGMETHOD_VERTEX;

  z_buffer = NULL;
  line_table = NULL;

  csScan_Initialize ();
}

csGraphics3DSoftware::~csGraphics3DSoftware ()
{
  Close ();
  csScan_Finalize ();
  CHK (delete texman);
  CHK (delete [] z_buffer);
  CHK (delete [] smaller_buffer);
  CHK (delete [] line_table);
  CHK (delete config;)
  if (G2D) G2D->DecRef ();
  if (System) System->DecRef ();
}

bool csGraphics3DSoftware::Initialize (iSystem *iSys)
{
  System = iSys;
  // @@@ See above in destructor.
  //(System = iSys)->IncRef ();

  width = height = -1;

  if (!System->RegisterDriver ("iGraphics3D", this))
    return false;

  const char *driver = config->GetStr ("Hardware", "Driver2D", SOFTWARE_2D_DRIVER);
  G2D = LOAD_PLUGIN (System, driver, iGraphics2D);
  if (!G2D)
    return false;

  CHK (texman = new csTextureManagerSoftware (System, G2D, config));

#ifdef DO_MMX
  do_mmx = config->GetYesNo ("Hardware", "MMX", true);
#endif
  do_smaller_rendering = config->GetYesNo ("Hardware", "SMALLER", false);
  SetRenderState (G3DRENDERSTATE_INTERLACINGENABLE,
    config->GetYesNo ("Hardware", "INTERLACING", false));

  mipmap_coef = config->GetFloat ("TextureManager", "MIPMAP_COEF", 1.3);

  return true;
}

void csGraphics3DSoftware::ScanSetup ()
{
  // Select the right scanline drawing functions
  memset (&ScanProc, 0, sizeof (ScanProc));
  memset (&ScanProcPI, 0, sizeof (ScanProcPI));
  memset (&ScanProcPIG, 0, sizeof (ScanProcPIG));
  memset (&ScanProcPIFX, 0, sizeof (ScanProcPIFX));
  ScanProc_Alpha = NULL;

#ifdef DO_MMX
  bool UseMMX = (cpu_mmx && do_mmx);
#endif

  // In the following unimplemented routines are just commented out
  // Since the arrays are zeroed above this is effectively a NULL assignment.
  ScanProc [SCANPROC_ZFIL] = csScan_draw_scanline_zfil;
  switch (pfmt.PixelBytes)
  {
    case 1:
      ScanProc [SCANPROC_FLAT_ZFIL] = csScan_8_draw_scanline_flat_zfil;
      ScanProc [SCANPROC_FLAT_ZUSE] = csScan_8_draw_scanline_flat_zuse;

      ScanProc [SCANPROC_TEX_ZFIL] =
#ifdef DO_MMX
        UseMMX ? csScan_8_mmx_draw_scanline_tex_zfil :
#endif
        csScan_8_draw_scanline_tex_zfil;
      ScanProc [SCANPROC_TEX_ZUSE] = csScan_8_draw_scanline_tex_zuse;

      ScanProc [SCANPROC_MAP_ZFIL] =
        do_bilin_filt ? csScan_8_draw_scanline_map_filt_zfil :
#ifdef DO_MMX
        UseMMX ? csScan_8_mmx_draw_scanline_map_zfil :
#endif
        csScan_8_draw_scanline_map_zfil;
      ScanProc [SCANPROC_MAP_ZUSE] = csScan_8_draw_scanline_map_zuse;

      ScanProc [SCANPROC_TEX_KEY_ZFIL] = csScan_8_draw_scanline_tex_key_zfil;
      ScanProc [SCANPROC_TEX_KEY_ZUSE] = csScan_8_draw_scanline_tex_key_zuse;
      ScanProc [SCANPROC_MAP_KEY_ZFIL] = csScan_8_draw_scanline_map_key_zfil;
      ScanProc [SCANPROC_MAP_KEY_ZUSE] = csScan_8_draw_scanline_map_key_zuse;

      ScanProc [SCANPROC_FOG] = csScan_8_draw_scanline_fog;
      ScanProc [SCANPROC_FOG_VIEW] = csScan_8_draw_scanline_fog_view;

      ScanProcPI [SCANPROCPI_FLAT_ZFIL] = csScan_8_draw_pi_scanline_flat_zfil;
      ScanProcPI [SCANPROCPI_FLAT_ZUSE] = csScan_8_draw_pi_scanline_flat_zuse;
      ScanProcPI [SCANPROCPI_TEX_ZFIL] = csScan_8_draw_pi_scanline_tex_zfil;
      ScanProcPI [SCANPROCPI_TEX_ZUSE] = csScan_8_draw_pi_scanline_tex_zuse;

      ScanProcPIG [SCANPROCPI_FLAT_GOURAUD_ZFIL] = csScan_8_draw_pi_scanline_flat_gouraud_zfil;
      ScanProcPIG [SCANPROCPI_FLAT_GOURAUD_ZUSE] = csScan_8_draw_pi_scanline_flat_gouraud_zuse;
      ScanProcPIG [SCANPROCPI_TEX_GOURAUD_ZFIL] = csScan_8_draw_pi_scanline_tex_gouraud_zfil;
      ScanProcPIG [SCANPROCPI_TEX_GOURAUD_ZUSE] = csScan_8_draw_pi_scanline_tex_gouraud_zuse;

      if (do_transp)
        ScanProc_Alpha = ScanProc_8_Alpha;

      ScanProcPIFX [SCANPROCPIFX_ZUSE] = csScan_8_draw_pifx_scanline_zuse;
      ScanProcPIFX [SCANPROCPIFX_ZFIL] = csScan_8_draw_pifx_scanline_zfil;
      ScanProcPIFX [SCANPROCPIFX_TRANSP_ZUSE] = csScan_8_draw_pifx_scanline_transp_zuse;
      ScanProcPIFX [SCANPROCPIFX_TRANSP_ZFIL] = csScan_8_draw_pifx_scanline_transp_zfil;
      break;

    case 2:
      ScanProc [SCANPROC_FLAT_ZFIL] = csScan_16_draw_scanline_flat_zfil;
      ScanProc [SCANPROC_FLAT_ZUSE] = csScan_16_draw_scanline_flat_zuse;

      ScanProc [SCANPROC_TEX_ZFIL] =
#ifdef DO_MMX
        UseMMX ? csScan_16_mmx_draw_scanline_tex_zfil :
#endif
        csScan_16_draw_scanline_tex_zfil;
      ScanProc [SCANPROC_TEX_ZUSE] = csScan_16_draw_scanline_tex_zuse;

      ScanProc [SCANPROC_MAP_ZFIL] =
        do_bilin_filt == 2 ? csScan_16_draw_scanline_map_filt2_zfil :
        do_bilin_filt == 1 ? csScan_16_draw_scanline_map_filt_zfil :
#ifdef DO_MMX
        UseMMX ? csScan_16_mmx_draw_scanline_map_zfil :
#endif
        csScan_16_draw_scanline_map_zfil;
      ScanProc [SCANPROC_MAP_ZUSE] = csScan_16_draw_scanline_map_zuse;

      ScanProc [SCANPROC_TEX_KEY_ZFIL] = csScan_16_draw_scanline_tex_key_zfil;
      ScanProc [SCANPROC_TEX_KEY_ZUSE] = csScan_16_draw_scanline_tex_key_zuse;
      ScanProc [SCANPROC_MAP_KEY_ZFIL] = csScan_16_draw_scanline_map_key_zfil;
      ScanProc [SCANPROC_MAP_KEY_ZUSE] = csScan_16_draw_scanline_map_key_zuse;

      ScanProc [SCANPROC_FOG] = (pfmt.GreenBits == 5) ?
        csScan_16_draw_scanline_fog_555 :
        csScan_16_draw_scanline_fog_565;
      ScanProc [SCANPROC_FOG_VIEW] = (pfmt.GreenBits == 5) ?
        csScan_16_draw_scanline_fog_view_555 :
        csScan_16_draw_scanline_fog_view_565;

      ScanProcPI [SCANPROCPI_FLAT_ZFIL] = csScan_16_draw_pi_scanline_flat_zfil;
      ScanProcPI [SCANPROCPI_FLAT_ZUSE] = csScan_16_draw_pi_scanline_flat_zuse;
      ScanProcPI [SCANPROCPI_TEX_ZFIL] = csScan_16_draw_pi_scanline_tex_zfil;
      ScanProcPI [SCANPROCPI_TEX_ZUSE] =
#ifdef DO_MMX
        UseMMX ? csScan_16_mmx_draw_pi_scanline_tex_zuse :
#endif
        csScan_16_draw_pi_scanline_tex_zuse;

      ScanProcPIG [SCANPROCPI_FLAT_GOURAUD_ZFIL] = (pfmt.GreenBits == 5) ?
        csScan_16_draw_pi_scanline_flat_gouraud_zfil_555 :
        csScan_16_draw_pi_scanline_flat_gouraud_zfil_565;
      ScanProcPIG [SCANPROCPI_FLAT_GOURAUD_ZUSE] = (pfmt.GreenBits == 5) ?
        csScan_16_draw_pi_scanline_flat_gouraud_zuse_555 :
        csScan_16_draw_pi_scanline_flat_gouraud_zuse_565;
      ScanProcPIG [SCANPROCPI_TEX_GOURAUD_ZFIL] = (pfmt.GreenBits == 5) ?
        csScan_16_draw_pi_scanline_tex_gouraud_zfil_555 :
        csScan_16_draw_pi_scanline_tex_gouraud_zfil_565;
      ScanProcPIG [SCANPROCPI_TEX_GOURAUD_ZUSE] = (pfmt.GreenBits == 5) ?
        csScan_16_draw_pi_scanline_tex_gouraud_zuse_555 :
        csScan_16_draw_pi_scanline_tex_gouraud_zuse_565;

      ScanProcPIFX [SCANPROCPIFX_ZUSE] = (pfmt.GreenBits == 5) ?
          csScan_16_draw_pifx_scanline_zuse_555 :
          csScan_16_draw_pifx_scanline_zuse_565;
      ScanProcPIFX [SCANPROCPIFX_ZFIL] = (pfmt.GreenBits == 5) ?
          csScan_16_draw_pifx_scanline_zfil_555 :
          csScan_16_draw_pifx_scanline_zfil_565;
      ScanProcPIFX [SCANPROCPIFX_TRANSP_ZUSE] = (pfmt.GreenBits == 5) ?
          csScan_16_draw_pifx_scanline_transp_zuse_555 :
          csScan_16_draw_pifx_scanline_transp_zuse_565;
      ScanProcPIFX [SCANPROCPIFX_TRANSP_ZFIL] = (pfmt.GreenBits == 5) ?
          csScan_16_draw_pifx_scanline_transp_zfil_555 :
          csScan_16_draw_pifx_scanline_transp_zfil_565;

      if (do_transp)
        ScanProc_Alpha = ScanProc_16_Alpha;
      break;

    case 4:
      ScanProc [SCANPROC_FLAT_ZFIL] = csScan_32_draw_scanline_flat_zfil;
      ScanProc [SCANPROC_FLAT_ZUSE] = csScan_32_draw_scanline_flat_zuse;

      ScanProc [SCANPROC_TEX_ZFIL] =
#if defined (DO_MMX) && defined (DO_NASM)
        UseMMX ? csScan_32_mmx_draw_scanline_tex_zfil :
#endif
        csScan_32_draw_scanline_tex_zfil;
      ScanProc [SCANPROC_TEX_ZUSE] = csScan_32_draw_scanline_tex_zuse;

      ScanProc [SCANPROC_MAP_ZFIL] =
#if defined (DO_MMX) && defined (DO_NASM)
        UseMMX ? csScan_32_mmx_draw_scanline_map_zfil :
#endif
        csScan_32_draw_scanline_map_zfil;
      ScanProc [SCANPROC_MAP_ZUSE] = csScan_32_draw_scanline_map_zuse;

      ScanProc [SCANPROC_TEX_KEY_ZFIL] = csScan_32_draw_scanline_tex_key_zfil;
      ScanProc [SCANPROC_TEX_KEY_ZUSE] = csScan_32_draw_scanline_tex_key_zuse;
      ScanProc [SCANPROC_MAP_KEY_ZFIL] = csScan_32_draw_scanline_map_key_zfil;
      ScanProc [SCANPROC_MAP_KEY_ZUSE] = csScan_32_draw_scanline_map_key_zuse;

      ScanProc [SCANPROC_FOG] = csScan_32_draw_scanline_fog;
      ScanProc [SCANPROC_FOG_VIEW] = csScan_32_draw_scanline_fog_view;

      ScanProcPI [SCANPROCPI_FLAT_ZFIL] = csScan_32_draw_pi_scanline_flat_zfil;
      ScanProcPI [SCANPROCPI_FLAT_ZUSE] = csScan_32_draw_pi_scanline_flat_zuse;
      ScanProcPI [SCANPROCPI_TEX_ZFIL] = csScan_32_draw_pi_scanline_tex_zfil;
      ScanProcPI [SCANPROCPI_TEX_ZUSE] = csScan_32_draw_pi_scanline_tex_zuse;

      ScanProcPIG [SCANPROCPI_FLAT_GOURAUD_ZFIL] = csScan_32_draw_pi_scanline_flat_gouraud_zfil;
      ScanProcPIG [SCANPROCPI_FLAT_GOURAUD_ZUSE] = csScan_32_draw_pi_scanline_flat_gouraud_zuse;
      ScanProcPIG [SCANPROCPI_TEX_GOURAUD_ZFIL] = csScan_32_draw_pi_scanline_tex_gouraud_zfil;
      ScanProcPIG [SCANPROCPI_TEX_GOURAUD_ZUSE] = csScan_32_draw_pi_scanline_tex_gouraud_zuse;

      ScanProcPIFX [SCANPROCPIFX_ZUSE] = csScan_32_draw_pifx_scanline_zuse;
      ScanProcPIFX [SCANPROCPIFX_ZFIL] = csScan_32_draw_pifx_scanline_zfil;
      ScanProcPIFX [SCANPROCPIFX_TRANSP_ZUSE] = csScan_32_draw_pifx_scanline_transp_zuse;
      ScanProcPIFX [SCANPROCPIFX_TRANSP_ZFIL] = csScan_32_draw_pifx_scanline_transp_zfil;

      if (do_transp)
        ScanProc_Alpha = ScanProc_32_Alpha;
      break;
  } /* endswitch */

  static int o_rbits = -1, o_gbits, o_bbits;
  if ((o_rbits != pfmt.RedBits)
   || (o_gbits != pfmt.GreenBits)
   || (o_bbits != pfmt.BlueBits))
    csScan_CalcBlendTables (o_rbits = pfmt.RedBits, o_gbits = pfmt.GreenBits,
      o_bbits = pfmt.BlueBits);
}

csDrawScanline* csGraphics3DSoftware::ScanProc_8_Alpha
  (csGraphics3DSoftware *This, int alpha)
{
  csAlphaTables *alpha_tables = This->texman->alpha_tables;

  if (alpha < 13)
    return NULL;
  if (alpha < 37)
  {
    Scan.AlphaMap = alpha_tables->alpha_map25;
    return csScan_8_draw_scanline_map_alpha2;
  }
  if (alpha >= 37 && alpha < 63)
  {
    Scan.AlphaMap = alpha_tables->alpha_map50;
    return csScan_8_draw_scanline_map_alpha1;
  }
  if (alpha >= 63 && alpha < 87)
  {
    Scan.AlphaMap = alpha_tables->alpha_map25;
    return csScan_8_draw_scanline_map_alpha1;
  }
  // completely opaque
  return csScan_8_draw_scanline_map_zfil;
}

csDrawScanline* csGraphics3DSoftware::ScanProc_16_Alpha
  (csGraphics3DSoftware *This, int alpha)
{
  Scan.AlphaMask = This->alpha_mask;
  Scan.AlphaFact = (alpha * 256) / 100;

  // completely transparent?
  if (alpha <= 100/32)
    return NULL;
  // approximate alpha from 47% to 53% with fast 50% routine
  if ((alpha >= 50 - 100/32) && (alpha <= 50 + 100/32))
    return csScan_16_draw_scanline_map_alpha50;
  // completely opaque?
  if (alpha >= 100 - 100/32)
    return csScan_16_draw_scanline_map_zfil;
  // general case
  if (This->pfmt.GreenBits == 5)
    return csScan_16_draw_scanline_map_alpha_555;
  else
    return csScan_16_draw_scanline_map_alpha_565;
}

csDrawScanline* csGraphics3DSoftware::ScanProc_32_Alpha
  (csGraphics3DSoftware* /*This*/, int alpha)
{
  Scan.AlphaFact = (alpha * 256) / 100;

  // completely transparent?
  if (alpha <= 1)
    return NULL;
  // for 50% use fast routine
  if (alpha == 50)
    return csScan_32_draw_scanline_map_alpha50;
  // completely opaque?
  if (alpha >= 99)
    return csScan_32_draw_scanline_map_zfil;
  // general case
  return csScan_32_draw_scanline_map_alpha;
}

bool csGraphics3DSoftware::Open (const char *Title)
{
  DrawMode = 0;

  if (!G2D->Open (Title))
  {
      SysPrintf (MSG_FATAL_ERROR, "Error opening Graphics2D context.\n");
      // set "not opened" flag
      width = height = -1;

      return false;
  }

  int nWidth = G2D->GetWidth ();
  int nHeight = G2D->GetHeight ();
  bool bFullScreen = G2D->GetFullScreen ();
  pfmt = *G2D->GetPixelFormat ();
  if (pfmt.PalEntries)
  {
    // If we don't have truecolor we simulate 5:6:5 bits
    // for R:G:B in the masks anyway because we still need the
    // 16-bit format for our light mixing
    pfmt.RedShift   = RGB2PAL_BITS_G + RGB2PAL_BITS_B;
    pfmt.GreenShift = RGB2PAL_BITS_B;
    pfmt.BlueShift  = 0;
    pfmt.RedMask    = ((1 << RGB2PAL_BITS_G) - 1) << pfmt.RedShift;
    pfmt.GreenMask  = ((1 << RGB2PAL_BITS_G) - 1) << pfmt.GreenShift;
    pfmt.BlueMask   = ((1 << RGB2PAL_BITS_B) - 1);
    pfmt.RedBits    = RGB2PAL_BITS_R;
    pfmt.GreenBits  = RGB2PAL_BITS_G;
    pfmt.BlueBits   = RGB2PAL_BITS_B;
  }
  texman->SetPixelFormat (pfmt);

  SetDimensions (nWidth, nHeight);

  SysPrintf(MSG_INITIALIZATION, "Using %s mode %dx%d (internal rendering at %dx%d).\n",
            bFullScreen ? "full screen" : "windowed", nWidth, nHeight, width, height);

  if (pfmt.PixelBytes == 4)
  {
    SysPrintf (MSG_INITIALIZATION, "Using truecolor mode with %d bytes per pixel and %d:%d:%d RGB mode.\n",
          pfmt.PixelBytes, pfmt.RedBits, pfmt.GreenBits, pfmt.BlueBits);

    pixel_shift = 2;
  }
  else if (pfmt.PixelBytes == 2)
  {
    SysPrintf (MSG_INITIALIZATION, "Using truecolor mode with %d bytes per pixel and %d:%d:%d RGB mode.\n",
          pfmt.PixelBytes, pfmt.RedBits, pfmt.GreenBits, pfmt.BlueBits);

    pixel_shift = 1;
  }
  else
  {
    SysPrintf (MSG_INITIALIZATION, "Using palette mode with 1 byte per pixel (256 colors).\n");
    pixel_shift = 0;
  }

  CHK (tcache = new csTextureCacheSoftware (texman));
  const char *cache_size = config->GetStr ("TextureManager", "CACHE", NULL);
  int csize = DEFAULT_CACHE_SIZE;
  if (cache_size)
  {
    char suffix [100];
    sscanf (cache_size, "%d%s", &csize, suffix);
    if (!strcasecmp (suffix, "KP"))
      csize *= 1024 * pfmt.PixelBytes;
    else if (!strcasecmp (suffix, "MP"))
      csize *= 1024 * 1024 * pfmt.PixelBytes;
    else if (!strcasecmp (suffix, "KB"))
      csize *= 1024;
    else if (!strcasecmp (suffix, "MB"))
      csize *= 1024 * 1024;
    else
      csize = 0;

    if (!csize)
    {
      SysPrintf (MSG_INITIALIZATION, "Invalid cache size specified, using default\n");
      csize = DEFAULT_CACHE_SIZE;
    }
  }
  tcache->set_cache_size (csize);

#if defined (DO_MMX)
  int family, features;
  char vendor [13];
  csDetectCPU (&family, vendor, &features);
  cpu_mmx = (features & CPUx86_FEATURE_MMX) != 0;
  SysPrintf (MSG_INITIALIZATION, "%d %s CPU detected; FPU (%s) MMX (%s) CMOV (%s)\n",
    family, vendor,
    (features & CPUx86_FEATURE_FPU) ? "yes" : "no",
    (features & CPUx86_FEATURE_MMX) ? "yes" : "no",
    (features & CPUx86_FEATURE_CMOV) ? "yes" : "no");
#endif

  alpha_mask = 0;
  alpha_mask |= 1 << (pfmt.RedShift);
  alpha_mask |= 1 << (pfmt.GreenShift);
  alpha_mask |= 1 << (pfmt.BlueShift);
  alpha_mask = ~alpha_mask;

  z_buf_mode = CS_ZBUF_NONE;
  fog_buffers = NULL;
  for (int i = 0; i < MAX_INDEXED_FOG_TABLES; i++)
    fog_tables [i].table = NULL;

  ScanSetup ();
  return true;
}

void csGraphics3DSoftware::Close()
{
  if ((width == height) && (width == -1))
    return;

  for (int i = 0; i < MAX_INDEXED_FOG_TABLES; i++)
    if (fog_tables [i].table)
      delete [] fog_tables [i].table;

  while (fog_buffers)
  {
    FogBuffer* n = fog_buffers->next;
    CHK (delete fog_buffers);
    fog_buffers = n;
  }

  CHK (delete tcache); tcache = NULL;
  CHK (delete clipper); clipper = NULL;

  G2D->Close ();
  width = height = -1;
}

void csGraphics3DSoftware::SetDimensions (int nwidth, int nheight)
{
  display_width = nwidth;
  display_height = nheight;
  if (do_smaller_rendering)
  {
    width = nwidth/2;
    height = nheight/2;
  }
  else
  {
    width = nwidth;
    height = nheight;
  }
  width2 = width/2;
  height2 = height/2;

  CHK (delete [] smaller_buffer);
  smaller_buffer = NULL;
  if (do_smaller_rendering)
  {
    CHK (smaller_buffer = new UByte [(width*height) * pfmt.PixelBytes]);
  }

  CHK (delete [] z_buffer);
  CHK (z_buffer = new unsigned long [width*height]);
  z_buf_size = sizeof (unsigned long)*width*height;

  CHK (delete [] line_table);
  CHK (line_table = new UByte* [height+1]);
}

void csGraphics3DSoftware::SetClipper (csVector2* vertices, int num_vertices)
{
  CHK (delete clipper);
  clipper = NULL;
  if (!vertices) return;
  // @@@ This could be better! We are using a general polygon clipper
  // even in cases where a box clipper would be better. We should
  // have a special SetBoxClipper call in iGraphics3D.
  CHK (clipper = new csPolygonClipper (vertices, num_vertices, false, true));
}

bool csGraphics3DSoftware::BeginDraw (int DrawFlags)
{
  if ((G2D->GetWidth() != display_width) || 
      (G2D->GetHeight() != display_height)) 
    SetDimensions (G2D->GetWidth(), G2D->GetHeight());

  // if 2D graphics is not locked, lock it
  if ((DrawFlags & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
   && (!(DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))))
  {
    if (!G2D->BeginDraw())
      return false;
  }

  if (DrawFlags & CSDRAW_CLEARZBUFFER)
    memset (z_buffer, 0, z_buf_size);

  if (DrawFlags & CSDRAW_CLEARSCREEN)
    G2D->Clear (0);

  if (DrawFlags & CSDRAW_3DGRAPHICS)
  {
    // Initialize the line table.
    int i;
    for (i = 0 ; i < height ; i++)
      if (do_smaller_rendering)
        line_table[i] = smaller_buffer + ((i*width)*pfmt.PixelBytes);
      else
        line_table[i] = G2D->GetPixelAt (0, i);
    dbg_current_polygon = 0;
  }
  else if (DrawMode & CSDRAW_3DGRAPHICS)
  {
    // Finished 3D drawing. If we are simulating the flush output to real frame buffer.
    if (do_smaller_rendering)
    {
      int x, y;
      switch (pfmt.PixelBytes)
      {
        case 2:
	  if (pfmt.GreenBits == 5)
            for (y = 0 ; y < height ; y++)
            {
              UShort* src = (UShort*)line_table[y];
	      UShort* dst1 = (UShort*)G2D->GetPixelAt (0, y+y);
              UShort* dst2 = (UShort*)G2D->GetPixelAt (0, y+y+1);
              for (x = 0 ; x < width ; x++)
	      {
	        dst1[x+x] = src[x];
	        dst1[x+x+1] = ((src[x]&0x7bde)>>1) + ((src[x+1]&0x7bde)>>1);
	        dst2[x+x] = ((src[x]&0x7bde)>>1) + ((src[x+width]&0x7bde)>>1);
	        dst2[x+x+1] = ((dst1[x+x+1]&0x7bde)>>1) + ((dst2[x+x]&0x7bde)>>1);
	      }
            }
	  else
            for (y = 0 ; y < height ; y++)
            {
              UShort* src = (UShort*)line_table[y];
	      UShort* dst1 = (UShort*)G2D->GetPixelAt (0, y+y);
              UShort* dst2 = (UShort*)G2D->GetPixelAt (0, y+y+1);
              for (x = 0 ; x < width ; x++)
	      {
	        dst1[x+x] = src[x];
	        dst1[x+x+1] = ((src[x]&0xf7de)>>1) + ((src[x+1]&0xf7de)>>1);
	        dst2[x+x] = ((src[x]&0xf7de)>>1) + ((src[x+width]&0xf7de)>>1);
	        dst2[x+x+1] = ((dst1[x+x+1]&0xf7de)>>1) + ((dst2[x+x]&0xf7de)>>1);
	      }
            }
	  break;
        case 4:
          for (y = 0 ; y < height ; y++)
          {
            ULong* src = (ULong*)line_table[y];
	    ULong* dst1 = (ULong*)G2D->GetPixelAt (0, y+y);
            ULong* dst2 = (ULong*)G2D->GetPixelAt (0, y+y+1);
            for (x = 0 ; x < width ; x++)
	    {
	      dst1[x+x] = src[x];
	      dst1[x+x+1] = ((src[x]&0xfefefe)>>1) + ((src[x+1]&0xfefefe)>>1);
	      dst2[x+x] = ((src[x]&0xfefefe)>>1) + ((src[x+width]&0xfefefe)>>1);
	      dst2[x+x+1] = ((dst1[x+x+1]&0xfefefe)>>1) + ((dst2[x+x]&0xfefefe)>>1);
	    }
          }
	  break;
      }
    }
  }

  DrawMode = DrawFlags;

  return true;
}

void csGraphics3DSoftware::FinishDraw ()
{
  if (DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
    G2D->FinishDraw ();
  DrawMode = 0;
}

void csGraphics3DSoftware::Print (csRect *area)
{
  G2D->Print (area);
  if (do_interlaced != -1)
    do_interlaced ^= 1;
}

void csGraphics3DSoftware::SetZBufMode (G3DZBufMode mode)
{
  z_buf_mode = mode;
}

#define SMALL_D 0.01

/**
 * The engine often generates "empty" polygons, for example
 * (2, 2) - (317,2) - (317,2) - (2, 2)
 * To avoid too much computations, DrawPolygon detects such polygons by
 * counting the number of "real" vertices (i.e. the number of vertices,
 * distance between which is bigger that some amount). The "right" formula
 * for distance is sqrt(dX^2 + dY^2) but to avoid root and multiply
 * DrawPolygon checks abs(dX) + abs(dY). This is enough.
 */
#define VERTEX_NEAR_THRESHOLD   0.001

/*
 * For the four interpolation modes.
 */
struct
{
  int step1, shift1;
  int step2, shift2;
  int step3, shift3;
  int step4, shift4;
} inter_modes[4] =
{
  { 128, 7, 64, 6, 32, 5, 16, 4 },      // Selective
  { 32, 5, 32, 5, 32, 5, 32, 5 },       // 32-steps
  { 16, 4, 16, 4, 16, 4, 16, 4 },       // 16-steps
  { 8, 3, 8, 3, 8, 3, 8, 3 }            // 8-steps
};

void csGraphics3DSoftware::DrawPolygonFlat (G3DPolygonDPF& poly)
{
  int i;
  int max_i, min_i;
  float max_y, min_y;
  unsigned char *d;
  unsigned long *z_buf;
  float inv_aspect = poly.inv_aspect;

  if (poly.num < 3) return;

  // Get the plane normal of the polygon. Using this we can calculate
  // '1/z' at every screen space point.
  float Ac, Bc, Cc, Dc, inv_Dc;
  Ac = poly.normal.A;
  Bc = poly.normal.B;
  Cc = poly.normal.C;
  Dc = poly.normal.D;

  float M, N, O;
  if (ABS (Dc) < SMALL_D) Dc = -SMALL_D;
  if (ABS (Dc) < SMALL_D)
  {
    // The Dc component of the plane normal is too small. This means that
    // the plane of the polygon is almost perpendicular to the eye of the
    // viewer. In this case, nothing much can be seen of the plane anyway
    // so we just take one value for the entire polygon.
    M = 0;
    N = 0;
    // For O choose the transformed z value of one vertex.
    // That way Z buffering should at least work.
    O = 1 / poly.z_value;
  }
  else
  {
    inv_Dc = 1/Dc;
    M = -Ac * inv_Dc * inv_aspect;
    N = -Bc * inv_Dc * inv_aspect;
    O = -Cc * inv_Dc;
  }

  // Compute the min_y and max_y for this polygon in screen space coordinates.
  // We are going to use these to scan the polygon from top to bottom.
  min_i = max_i = 0;
  min_y = max_y = poly.vertices[0].sy;
  // count 'real' number of vertices
  int num_vertices = 1;
  for (i = 1 ; i < poly.num ; i++)
  {
    if (poly.vertices[i].sy > max_y)
    {
      max_y = poly.vertices[i].sy;
      max_i = i;
    }
    else if (poly.vertices[i].sy < min_y)
    {
      min_y = poly.vertices[i].sy;
      min_i = i;
    }
    // theoretically we should do sqrt(dx^2+dy^2) here, but
    // we can approximate it by just abs(dx)+abs(dy)
    if ((fabs (poly.vertices [i].sx - poly.vertices [i - 1].sx)
       + fabs (poly.vertices [i].sy - poly.vertices [i - 1].sy)) > VERTEX_NEAR_THRESHOLD)
      num_vertices++;
  }

  // if this is a 'degenerate' polygon, skip it.
  if (num_vertices < 3)
    return;

  // For debugging: is we reach the maximum number of polygons to draw we simply stop.
  dbg_current_polygon++;
  if (dbg_current_polygon == dbg_max_polygons_to_draw-1)
    return;
  if (dbg_current_polygon >= dbg_max_polygons_to_draw-1)
    return;

  iPolygonTexture *tex = NULL;
  iLightMap *lm = NULL;
  if (do_lighting)
  {
    tex = poly.poly_texture;
    lm = tex->GetLightMap ();
  }

  UByte mean_r, mean_g, mean_b;
  poly.txt_handle->GetMeanColor (mean_r, mean_g, mean_b);

  if (lm)
  {
    // Lighted polygon
    int lr, lg, lb;
    lm->GetMeanLighting (lr, lg, lb);

    // Make lighting a little bit brighter because average
    // lighting is really dark otherwise.
    lr = lr << 1; if (lr > 255) lr = 255;
    lg = lg << 1; if (lg > 255) lg = 255;
    lb = lb << 1; if (lb > 255) lb = 255;

    if (pfmt.PixelBytes >= 2)
      Scan.FlatColor = texman->encode_rgb ((mean_r * lr) >> 8,
        (mean_g * lg) >> 8, (mean_b * lb) >> 8);
    else
      Scan.FlatColor = texman->find_rgb ((mean_r * lr) >> 8,
        (mean_g * lg) >> 8, (mean_b * lb) >> 8);
  }
  else if (pfmt.PixelBytes >= 2)
    Scan.FlatColor = texman->encode_rgb (mean_r, mean_g, mean_b);
  else
    Scan.FlatColor = texman->find_rgb (mean_r, mean_g, mean_b);

  Scan.M = M;

  // Select the right scanline drawing function.
  if (do_transp
   && (poly.alpha || (poly.txt_handle && poly.txt_handle->GetTransparent ())))
    return;
  int scan_index = SCANPROC_FLAT_ZFIL;
  if (z_buf_mode == CS_ZBUF_USE)
    scan_index++;
  csDrawScanline* dscan = ScanProc [scan_index];
  if (!dscan)
    return;				// Nothing to do.

  // Scan both sides of the polygon at once.
  // We start with two pointers at the top (as seen in y-inverted
  // screen-space: bottom of display) and advance them until both
  // join together at the bottom. The way this algorithm works, this
  // should happen automatically; the left pointer is only advanced
  // when it is further away from the bottom than the right pointer
  // and vice versa.
  // Using this we effectively partition our polygon in trapezoids
  // with at most two triangles (one at the top and one at the bottom).

  int scanL1, scanL2, scanR1, scanR2;   // scan vertex left/right start/final
  float sxL, sxR, dxL, dxR;             // scanline X left/right and deltas
  int sy, fyL, fyR;                     // scanline Y, final Y left, final Y right
  int xL, xR;
  int screenY;

  sxL = sxR = dxL = dxR = 0;            // avoid GCC warnings about "uninitialized variables"
  scanL2 = scanR2 = max_i;
  sy = fyL = fyR = QRound (poly.vertices [scanL2].sy);

  for ( ; ; )
  {
    //-----
    // We have reached the next segment. Recalculate the slopes.
    //-----
    bool leave;
    do
    {
      leave = true;
      if (sy <= fyR)
      {
        // Check first if polygon has been finished
        if (scanR2 == min_i)
	  return;
        scanR1 = scanR2;
	if (++scanR2 >= poly.num)
	  scanR2 = 0;

        leave = false;
        fyR = QRound (poly.vertices [scanR2].sy);
        if (sy <= fyR)
          continue;

        float dyR = (poly.vertices [scanR1].sy - poly.vertices [scanR2].sy);
        if (dyR)
        {
          sxR = poly.vertices [scanR1].sx;
          dxR = (poly.vertices [scanR2].sx - sxR) / dyR;
          // horizontal pixel correction
          sxR += dxR * (poly.vertices [scanR1].sy - (float (sy) - 0.5));
        } /* endif */
      } /* endif */
      if (sy <= fyL)
      {
        scanL1 = scanL2;
	if (--scanL2 < 0)
	  scanL2 = poly.num - 1;

        leave = false;
        fyL = QRound (poly.vertices [scanL2].sy);
        if (sy <= fyL)
          continue;

        float dyL = (poly.vertices [scanL1].sy - poly.vertices [scanL2].sy);
        if (dyL)
        {
          sxL = poly.vertices [scanL1].sx;
          dxL = (poly.vertices [scanL2].sx - sxL) / dyL;
          // horizontal pixel correction
          sxL += dxL * (poly.vertices [scanL1].sy - (float (sy) - 0.5));
        } /* endif */
      } /* endif */
    } while (!leave); /* enddo */

    // Steps for interpolating vertically over scanlines.
    float vd_inv_z = - dxL * M + N;

    float cx = (sxL - float (width2));
    float cy = (float (sy) - 0.5 - float (height2));
    float inv_z = M * cx + N * cy + O;

    // Find the trapezoid top (or bottom in inverted Y coordinates)
    int fin_y;
    if (fyL > fyR)
      fin_y = fyL;
    else
      fin_y = fyR;

    screenY = height - 1 - sy;

    while (sy > fin_y)
    {
      //@@@ Normally I would not need to have to check against screen
      // boundaries but apparantly there are cases where this test is
      // needed (maybe a bug in the clipper?). I have to look at this later.
#if 1
      if ((sy & 1) != do_interlaced)
#else
      if (((sy & 1) != do_interlaced) && (sxR >= 0) && (sxL < width) && (screenY >= 0) && (screenY < height))
#endif
      {
        // Compute the rounded screen coordinates of horizontal strip
        xL = QRound (sxL);
        xR = QRound (sxR);

        d = line_table [screenY] + (xL << pixel_shift);
        z_buf = z_buffer + width * screenY + xL;

        // do not draw the rightmost pixel - it will be covered
        // by neightbour polygon's left bound
        dscan (xR - xL, d, z_buf, inv_z, 0, 0);
      }

      sxL += dxL;
      sxR += dxR;
      inv_z -= vd_inv_z;
      sy--;
      screenY++;
    } /* endwhile */
  } /* endfor */
}

void csGraphics3DSoftware::DrawPolygon (G3DPolygonDP& poly)
{
  if (!do_textured)
  {
    DrawPolygonFlat (poly);
    return;
  }

  int i;
  float P1, P2, P3, P4;
  float Q1, Q2, Q3, Q4;
  int max_i, min_i, min_z_i;
  float max_y, min_y;
  float min_z;
  unsigned char *d;
  unsigned long *z_buf;
  float inv_aspect = poly.inv_aspect;

  if (poly.num < 3)
    return;

  // Get the plane normal of the polygon. Using this we can calculate
  // '1/z' at every screen space point.
  float Ac, Bc, Cc, Dc, inv_Dc;
  Ac = poly.normal.A;
  Bc = poly.normal.B;
  Cc = poly.normal.C;
  Dc = poly.normal.D;

  float M, N, O;
  if (ABS (Dc) < SMALL_D) Dc = -SMALL_D;
  if (ABS (Dc) < SMALL_D)
  {
    // The Dc component of the plane normal is too small. This means that
    // the plane of the polygon is almost perpendicular to the eye of the
    // viewer. In this case, nothing much can be seen of the plane anyway
    // so we just take one value for the entire polygon.
    M = 0;
    N = 0;
    // For O choose the transformed z value of one vertex.
    // That way Z buffering should at least work.
    O = 1/poly.z_value;
  }
  else
  {
    inv_Dc = 1/Dc;
    M = -Ac*inv_Dc*inv_aspect;
    N = -Bc*inv_Dc*inv_aspect;
    O = -Cc*inv_Dc;
  }

  // Compute the min_y and max_y for this polygon in screen space coordinates.
  // We are going to use these to scan the polygon from top to bottom.
  // Also compute the min_z in camera space coordinates. This is going to be
  // used for mipmapping.
  min_i = max_i = min_z_i = 0;
  min_y = max_y = poly.vertices[0].sy;
  min_z = M * (poly.vertices[0].sx - width2)
        + N * (poly.vertices[0].sy - height2) + O;
  // count 'real' number of vertices
  int num_vertices = 1;
  for (i = 1 ; i < poly.num ; i++)
  {
    if (poly.vertices[i].sy > max_y)
    {
      max_y = poly.vertices[i].sy;
      max_i = i;
    }
    else if (poly.vertices[i].sy < min_y)
    {
      min_y = poly.vertices[i].sy;
      min_i = i;
    }
    float inv_z = M * (poly.vertices[i].sx - width2)
                + N * (poly.vertices[i].sy - height2) + O;
    if (inv_z > min_z)
    {
      min_z = inv_z;
      min_z_i = i;
    }
    // theoretically we should do sqrt(dx^2+dy^2) here, but
    // we can approximate it by just abs(dx)+abs(dy)
    if ((fabs (poly.vertices [i].sx - poly.vertices [i - 1].sx)
       + fabs (poly.vertices [i].sy - poly.vertices [i - 1].sy)) > VERTEX_NEAR_THRESHOLD)
      num_vertices++;
  }

  // if this is a 'degenerate' polygon, skip it.
  if (num_vertices < 3)
    return;

  // For debugging: is we reach the maximum number of polygons to draw we simply stop.
  dbg_current_polygon++;
  if (dbg_current_polygon == dbg_max_polygons_to_draw-1)
    return;
  if (dbg_current_polygon >= dbg_max_polygons_to_draw-1)
    return;

  // @@@ The texture transform matrix is currently written as T = M*(C-V)
  // (with V being the transform vector, M the transform matrix, and C
  // the position in camera space coordinates. It would be better (more
  // suitable for the following calculations) if it would be written
  // as T = M*C - V.
  P1 = poly.plane.m_cam2tex->m11;
  P2 = poly.plane.m_cam2tex->m12;
  P3 = poly.plane.m_cam2tex->m13;
  P4 = - (P1 * poly.plane.v_cam2tex->x
        + P2 * poly.plane.v_cam2tex->y
        + P3 * poly.plane.v_cam2tex->z);
  Q1 = poly.plane.m_cam2tex->m21;
  Q2 = poly.plane.m_cam2tex->m22;
  Q3 = poly.plane.m_cam2tex->m23;
  Q4 = - (Q1 * poly.plane.v_cam2tex->x
        + Q2 * poly.plane.v_cam2tex->y
        + Q3 * poly.plane.v_cam2tex->z);

  iPolygonTexture *tex = poly.poly_texture;
  csTextureMMSoftware *tex_mm = (csTextureMMSoftware *)poly.txt_handle->GetPrivateObject ();
  float fdu, fdv;
  if (tex)
  {
    fdu = tex->GetFDU ();
    fdv = tex->GetFDV ();
  }
  else
  {
    fdu = 0;
    fdv = 0;
  }

  // Now we're in the right shape to determine the mipmap level.
  // We'll use the following formula to determine the required level of
  // mipmap: we'll take the x and y screen coordinates of nearest point.
  // Now we compute the difference between u (x + 1, y + 1) and u (x, y),
  // and between v (x + 1, y + 1) and v (x, y). We're using the following
  // formulas in our texture mapping to compute u, v an z values at any
  // given x, y screen point:
  //
  //   u/z (x, y) = J1 * x + J2 * y + J3
  //   v/z (x, y) = K1 * x + K2 * y + K3
  //   1/z (x, y) = M * x + N * y + O
  //
  // Thus u (x, y) = u/z (x, y) / 1/z (x, y). Similary for v (x, y).
  // Also we know that at nearest polygon point (where we will research
  // for texel density) the 1/z value is already stored in min_z variable
  // (i.e. M * x + N * y + O = min_z). Thus we compute the formula for
  // u (x + 1, y + 1) - u (x, y):
  //
  //   J1 * (x + 1) + J2 * (y + 1) + J3     J1 * x + J2 * y + J3
  //  ---------------------------------- - ---------------------- =
  //    M * (x + 1) + N * (x + 1) + O        M * x + N * y + O
  // 
  //  min_z*(J1*(x+1) + J2*(y+1) + J3) - (min_z + M + N)*(J1*x + J2*y + J3)
  //  --------------------------------------------------------------------- =
  //                        min_z * (min_z + M + N)
  //
  //   min_z * (J1 + J2) - (M + N) * (J1 * x + J2 * y + J3)
  //  ------------------------------------------------------
  //                     min_z * (min_z + M + N)
  //
  // Thus we can compute delta U and delta V (we'll refer them as du and dv),
  // the amount of texels we should move in texture space if we move by 1 in
  // x and y direction.
  // 
  // Now we should take sqrt (du^2 + dv^2) and decide the mipmap level
  // depending on this value. We can ommit the square root and work with the
  // squared value. Thus, we can select the required mipmap level depending
  // on this value this way:
  //
  //   if <= 2*2, mipmap level 0
  //   if <= 4*4, mipmap level 1
  //   if <= 8*8, mipmap level 2
  //   if above, mipmap level 3

  // Mipmapping.
  int mipmap;
  if (rstate_mipmap == 1)
    mipmap = 0;
  else if (rstate_mipmap == 0)
  {
    if (!min_z)
      mipmap = 0;
    else if (ABS (Dc) < SMALL_D)
      mipmap = 3;
    else
    {
      // Mipmap level 0 size
      int m0w = tex_mm->get_texture (0)->get_width ();
      int m0h = tex_mm->get_texture (0)->get_height ();

      float _P1 = P1 * m0w, _P2 = P2 * m0w, _P3 = P3 * m0w, _P4 = P4 * m0w - fdu;
      float _Q1 = Q1 * m0h, _Q2 = Q2 * m0h, _Q3 = Q3 * m0h, _Q4 = Q4 * m0h - fdv;

      float J1 = _P1 * inv_aspect + _P4 * M;
      float J2 = _P2 * inv_aspect + _P4 * N;
      float J3 = _P3              + _P4 * O;
      float K1 = _Q1 * inv_aspect + _Q4 * M;
      float K2 = _Q2 * inv_aspect + _Q4 * N;
      float K3 = _Q3              + _Q4 * O;

      float x = poly.vertices [min_z_i].sx - width2;
      float y = poly.vertices [min_z_i].sy - height2;

      float du = (min_z * (J1 + J2) - (M + N) * (J1 * x + J2 * y + J3)) /
                 (min_z * (min_z + M + N));
      float dv = (min_z * (K1 + K2) - (M + N) * (K1 * x + K2 * y + K3)) /
                 (min_z * (min_z + M + N));

      float mipmap_sel = mipmap_coef * (du * du + dv * dv);

      // Now look which mipmap we should use
      if (mipmap_sel <= 2 * 2)
        mipmap = 0;
      else if (mipmap_sel <= 4 * 4)
        mipmap = 1;
      else if (mipmap_sel <= 8 * 8)
        mipmap = 2;
      else
        mipmap = 3;
    }
  }
  else
    mipmap = rstate_mipmap - 1;

  // If mipmap is too small or not available, use the mipmap level
  // that is still visible or available ...
  int shf_u = tex->GetShiftU () - mipmap;
  if (shf_u < 0) mipmap += shf_u;
  if (mipmap < 0) mipmap = 0;
  while (mipmap && !tex_mm->get_texture (mipmap))
    mipmap--;

  if (mipmap)
  {
    int duv = (1 << mipmap);
    fdu /= duv;
    fdv /= duv;
  }

  // Now get the unlighted texture corresponding to mipmap level we choosen
  csTexture *txt_unl = tex_mm->get_texture (mipmap);

  // Check if polygon has a lightmap (i.e. if it is lighted)
  bool has_lightmap = tex->GetLightMap () && do_lighting;

  // Continue with texture mapping
  Scan.tw = txt_unl->get_width ();
  Scan.th = txt_unl->get_height ();

  P1 *= Scan.tw; P2 *= Scan.tw; P3 *= Scan.tw; P4 *= Scan.tw;
  Q1 *= Scan.th; Q2 *= Scan.th; Q3 *= Scan.th; Q4 *= Scan.th;
  P4 -= fdu; Q4 -= fdv;

  // Precompute everything so that we can calculate (u,v) (texture space
  // coordinates) for every (sx,sy) (screen space coordinates). We make
  // use of the fact that 1/z, u/z and v/z are linear in screen space.
  float J1, J2, J3, K1, K2, K3;
  if (ABS (Dc) < SMALL_D)
  {
    // The Dc component of the plane of the polygon is too small.
    J1 = J2 = J3 = 0;
    K1 = K2 = K3 = 0;
  }
  else
  {
    J1 = P1 * inv_aspect + P4 * M;
    J2 = P2 * inv_aspect + P4 * N;
    J3 = P3              + P4 * O;
    K1 = Q1 * inv_aspect + Q4 * M;
    K2 = Q2 * inv_aspect + Q4 * N;
    K3 = Q3              + Q4 * O;
  }

  // Select the right interpolation factor based on the z-slope of our
  // polygon. This will greatly increase the speed of polygons which are
  // horizontally constant in z.
  if (ABS (M) < .000001)
  {
    Scan.InterpolStep = inter_modes[Scan.InterpolMode].step1;
    Scan.InterpolShift = inter_modes[Scan.InterpolMode].shift1;
  }
  else if (ABS (M) < .00005)
  {
    Scan.InterpolStep = inter_modes[Scan.InterpolMode].step2;
    Scan.InterpolShift = inter_modes[Scan.InterpolMode].shift2;
  }
  else if (ABS (M) < .001)
  {
    Scan.InterpolStep = inter_modes[Scan.InterpolMode].step3;
    Scan.InterpolShift = inter_modes[Scan.InterpolMode].shift3;
  }
  else
  {
    Scan.InterpolStep = inter_modes[Scan.InterpolMode].step4;
    Scan.InterpolShift = inter_modes[Scan.InterpolMode].shift4;
  }

  // Steps for interpolating horizontally accross a scanline.
  Scan.M = M;
  Scan.J1 = J1;
  Scan.K1 = K1;
  Scan.dM = M * Scan.InterpolStep;
  Scan.dJ1 = J1 * Scan.InterpolStep;
  Scan.dK1 = K1 * Scan.InterpolStep;

  // Scan both sides of the polygon at once.
  // We start with two pointers at the top (as seen in y-inverted
  // screen-space: bottom of display) and advance them until both
  // join together at the bottom. The way this algorithm works, this
  // should happen automatically; the left pointer is only advanced
  // when it is further away from the bottom than the right pointer
  // and vice versa.
  // Using this we effectively partition our polygon in trapezoids
  // with at most two triangles (one at the top and one at the bottom).

  int scanL1, scanL2, scanR1, scanR2;   // scan vertex left/right start/final
  float sxL, sxR, dxL, dxR;             // scanline X left/right and deltas
  int sy, fyL, fyR;                     // scanline Y, final Y left, final Y right
  int xL, xR;
  int screenY;

  // If we have a lighted polygon, prepare the texture
  if (has_lightmap)
  {
    sxL = sxR = dxL = dxR = 0;
    scanL2 = scanR2 = max_i;
    sy = fyL = fyR = QRound (poly.vertices [scanL2].sy);

    // Find the largest texture rectangle that is going to be displayed
    float u_min = +99999999;
    float v_min = +99999999;
    float u_max = -99999999;
    float v_max = -99999999;

    // Do a quick polygon scan at the edges to find mi/max u and v
    for ( ; ; )
    {
      bool leave;
      do
      {
        leave = true;
        if (sy <= fyR)
        {
          // Check first if polygon has been finished
          if (scanR2 == min_i)
            goto texr_done;
          scanR1 = scanR2;
          if (++scanR2 >= poly.num)
            scanR2 = 0;

          leave = false;
          fyR = QRound (poly.vertices [scanR2].sy);
          if (sy <= fyR)
            continue;

          float dyR = (poly.vertices [scanR1].sy - poly.vertices [scanR2].sy);
          if (dyR)
          {
            sxR = poly.vertices [scanR1].sx;
            dxR = (poly.vertices [scanR2].sx - sxR) / dyR;
            // horizontal pixel correction
            sxR += dxR * (poly.vertices [scanR1].sy - (float (sy) - 0.5));
          }
        }
        if (sy <= fyL)
        {
          scanL1 = scanL2;
          if (--scanL2 < 0)
            scanL2 = poly.num - 1;

          leave = false;
          fyL = QRound (poly.vertices [scanL2].sy);
          if (sy <= fyL)
            continue;

          float dyL = (poly.vertices [scanL1].sy - poly.vertices [scanL2].sy);
          if (dyL)
          {
            sxL = poly.vertices [scanL1].sx;
            dxL = (poly.vertices [scanL2].sx - sxL) / dyL;
            // horizontal pixel correction
            sxL += dxL * (poly.vertices [scanL1].sy - (float (sy) - 0.5));
          }
        }
      } while (!leave);

      // Compute U/V and check against bounding box
#define CHECK(sx)					\
      {							\
        float cx = sx - float (width2);			\
        float z = 1 / (M * cx + N * cy + O);		\
        float u = (J1 * cx + J2 * cy + J3) * z;		\
        float v = (K1 * cx + K2 * cy + K3) * z;		\
							\
        if (u < u_min) u_min = u;			\
        if (u > u_max) u_max = u;			\
        if (v < v_min) v_min = v;			\
        if (v > v_max) v_max = v;			\
      }

      float cy = (float (sy) - 0.5 - float (height2));
      // Due to sub-pixel correction we can arrive half of pixel left or right
      CHECK (sxL - 0.5);
      CHECK (sxR + 0.5);

      // Find the trapezoid top (or bottom in inverted Y coordinates)
      int old_sy = sy;
      if (fyL > fyR)
        sy = fyL;
      else
        sy = fyR;

      sxL += dxL * (old_sy - sy);
      sxR += dxR * (old_sy - sy);

      cy = (float (sy) - 0.5 - float (height2));
      CHECK (sxL);
      CHECK (sxR);
#undef CHECK
    }
texr_done:

    tcache->fill_texture (mipmap, tex, u_min, v_min, u_max, v_max);
  }
  csScan_InitDraw (mipmap, this, tex, tex_mm, txt_unl);

  // Select the right scanline drawing function.
  bool tex_transp = tex_mm->GetTransparent ();
  int  scan_index = -2;
  csDrawScanline* dscan = NULL;

  if (Scan.bitmap2)
  {
    if (do_transp && tex_transp)
      scan_index = SCANPROC_MAP_KEY_ZFIL;
    else if (ScanProc_Alpha && poly.alpha)
      dscan = ScanProc_Alpha (this, poly.alpha);
    else
      scan_index = SCANPROC_MAP_ZFIL;
  }
  else
  {
    Scan.PaletteTable = Scan.Texture->GetPaletteToGlobal ();
    if (do_transp && tex_transp)
      scan_index = SCANPROC_TEX_KEY_ZFIL;
    else
      scan_index = SCANPROC_TEX_ZFIL;
  } /* endif */
  if (z_buf_mode == CS_ZBUF_USE)
    scan_index++;
  if (!dscan)
    if ((scan_index < 0) || !(dscan = ScanProc [scan_index]))
      return; // nothing to do

  sxL = sxR = dxL = dxR = 0;            // avoid GCC warnings about "uninitialized variables"
  scanL2 = scanR2 = max_i;
  sy = fyL = fyR = QRound (poly.vertices [scanL2].sy);

  for ( ; ; )
  {
    //-----
    // We have reached the next segment. Recalculate the slopes.
    //-----
    bool leave;
    do
    {
      leave = true;
      if (sy <= fyR)
      {
        // Check first if polygon has been finished
        if (scanR2 == min_i)
          return;
        scanR1 = scanR2;
	if (++scanR2 >= poly.num)
	  scanR2 = 0;

        leave = false;
        fyR = QRound (poly.vertices [scanR2].sy);
        if (sy <= fyR)
          continue;

        float dyR = (poly.vertices [scanR1].sy - poly.vertices [scanR2].sy);
        if (dyR)
        {
          sxR = poly.vertices [scanR1].sx;
          dxR = (poly.vertices [scanR2].sx - sxR) / dyR;
          // horizontal pixel correction
          sxR += dxR * (poly.vertices [scanR1].sy - (float (sy) - 0.5));
        }
      }
      if (sy <= fyL)
      {
        scanL1 = scanL2;
	if (--scanL2 < 0)
	  scanL2 = poly.num - 1;

        leave = false;
        fyL = QRound (poly.vertices [scanL2].sy);
        if (sy <= fyL)
          continue;

        float dyL = (poly.vertices [scanL1].sy - poly.vertices [scanL2].sy);
        if (dyL)
        {
          sxL = poly.vertices [scanL1].sx;
          dxL = (poly.vertices [scanL2].sx - sxL) / dyL;
          // horizontal pixel correction
          sxL += dxL * (poly.vertices [scanL1].sy - (float (sy) - 0.5));
        }
      }
    } while (!leave);

    // Steps for interpolating vertically over scanlines.
    float vd_inv_z = - dxL * M + N;
    float vd_u_div_z = - dxL * J1 + J2;
    float vd_v_div_z = - dxL * K1 + K2;

    float cx = (sxL - float (width2));
    float cy = (float (sy) - 0.5 - float (height2));
    float inv_z = M * cx + N * cy + O;
    float u_div_z = J1 * cx + J2 * cy + J3;
    float v_div_z = K1 * cx + K2 * cy + K3;

    // Find the trapezoid top (or bottom in inverted Y coordinates)
    int fin_y;
    if (fyL > fyR)
      fin_y = fyL;
    else
      fin_y = fyR;

    screenY = height - 1 - sy;

    while (sy > fin_y)
    {
      if ((sy & 1) != do_interlaced)
      {
        // Compute the rounded screen coordinates of horizontal strip
        xL = QRound (sxL);
        xR = QRound (sxR);

        // Sub-pixel U & V correction
        float deltaX = (float)xL - sxL;

        //G2D->GetPixelAt(xL, screenY, &d);
        d = line_table [screenY] + (xL << pixel_shift);
        z_buf = z_buffer + width * screenY + xL;

        // Select the right filter depending if we are drawing an odd or even line.
        // This is only used by draw_scanline_map_filt_zfil currently and is still
        // experimental.
        if (sy & 1) filter_bf = 3; else filter_bf = 1;

        // do not draw the rightmost pixel - it will be covered
        // by neightbour polygon's left bound
        dscan (xR - xL, d, z_buf, inv_z + deltaX * M, u_div_z + deltaX * J1, v_div_z + deltaX * K1);
      }

      sxL += dxL;
      sxR += dxR;
      inv_z -= vd_inv_z;
      u_div_z -= vd_u_div_z;
      v_div_z -= vd_v_div_z;
      sy--;
      screenY++;
    } /* endwhile */
  } /* endfor */
}

void csGraphics3DSoftware::DrawPolygonDebug (G3DPolygonDP& poly)
{
  (void)poly;
}

FogBuffer* csGraphics3DSoftware::find_fog_buffer (CS_ID id)
{
  FogBuffer* f = fog_buffers;
  while (f)
  {
    if (f->id == id) return f;
    f = f->next;
  }
  return NULL;
}

void csGraphics3DSoftware::OpenFogObject (CS_ID id, csFog* fog)
{
  CHK (FogBuffer* fb = new FogBuffer ());
  fb->next = fog_buffers;
  fb->prev = NULL;
  fb->id = id;
  fb->density = fog->density;
  fb->red = fog->red;
  fb->green = fog->green;
  fb->blue = fog->blue;
  if (fog_buffers) fog_buffers->prev = fb;
  fog_buffers = fb;
}

void csGraphics3DSoftware::CloseFogObject (CS_ID id)
{
  FogBuffer* fb = find_fog_buffer (id);
  if (!fb)
  {
    SysPrintf (MSG_INTERNAL_ERROR, "ENGINE FAILURE! Try to close a non-open fog object!\n");
    return;
  }
  if (fb->next) fb->next->prev = fb->prev;
  if (fb->prev) fb->prev->next = fb->next;
  else fog_buffers = fb->next;
  CHK (delete fb);
}

void csGraphics3DSoftware::AddFogPolygon (CS_ID id, G3DPolygonAFP& poly, int fog_type)
{
  int i;
  int max_i, min_i;
  float max_y, min_y;
  unsigned char *d;
  unsigned long *z_buf;
  float inv_aspect = poly.inv_aspect;

  if (poly.num < 3)
    return;
  if (fogMode == G3DFOGMETHOD_VERTEX)
    return;

  float M = 0, N = 0, O = 0;
  if (fog_type == CS_FOG_FRONT || fog_type == CS_FOG_BACK)
  {
    // Get the plane normal of the polygon. Using this we can calculate
    // '1/z' at every screen space point.
    float Ac, Bc, Cc, Dc, inv_Dc;
    Ac = poly.normal.A;
    Bc = poly.normal.B;
    Cc = poly.normal.C;
    Dc = poly.normal.D;

    if (ABS (Dc) < SMALL_D)
    {
      // The Dc component of the plane normal is too small. This means that
      // the plane of the polygon is almost perpendicular to the eye of the
      // viewer. In this case, nothing much can be seen of the plane anyway
      // so we just take one value for the entire polygon.
      M = 0;
      N = 0;
      // For O choose the transformed z value of one vertex.
      // That way Z buffering should at least work.
      O = 1;
    }
    else
    {
      inv_Dc = 1/Dc;
      M = -Ac*inv_Dc*inv_aspect;
      N = -Bc*inv_Dc*inv_aspect;
      O = -Cc*inv_Dc;
    }
  }

  // Compute the min_y and max_y for this polygon in screen space coordinates.
  // We are going to use these to scan the polygon from top to bottom.
  min_i = max_i = 0;
  min_y = max_y = poly.vertices[0].sy;
  // count 'real' number of vertices
  int num_vertices = 1;
  for (i = 1 ; i < poly.num ; i++)
  {
    if (poly.vertices[i].sy > max_y)
    {
      max_y = poly.vertices[i].sy;
      max_i = i;
    }
    else if (poly.vertices[i].sy < min_y)
    {
      min_y = poly.vertices[i].sy;
      min_i = i;
    }
    // theoretically we should do here sqrt(dx^2+dy^2), but
    // we can approximate it just by abs(dx)+abs(dy)
    if ((fabs (poly.vertices [i].sx - poly.vertices [i - 1].sx)
       + fabs (poly.vertices [i].sy - poly.vertices [i - 1].sy)) > VERTEX_NEAR_THRESHOLD)
      num_vertices++;
  }

  // if this is a 'degenerate' polygon, skip it
  if (num_vertices < 3)
    return;

  FogBuffer* fb = find_fog_buffer (id);
  if (!fb)
  {
    SysPrintf (MSG_INTERNAL_ERROR, "ENGINE FAILURE! Fog object not open!\n");
    exit (0);
  }

  Scan.FogDensity = QRound (fb->density * 100);
  if (pfmt.PalEntries == 0)
  {
    Scan.FogR = QRound (fb->red * ((1 << pfmt.RedBits) - 1)) << pfmt.RedShift;
    Scan.FogG = QRound (fb->green * ((1 << pfmt.GreenBits) - 1)) << pfmt.GreenShift;
    Scan.FogB = QRound (fb->blue * ((1 << pfmt.BlueBits) - 1)) << pfmt.BlueShift;
    Scan.FogPix = Scan.FogR | Scan.FogG | Scan.FogB;

    if (pfmt.PixelBytes == 4)
    {
      // trick: in 32-bit modes set FogR,G,B so that "R" uses bits 16-23,
      // "G" uses bits 8-15 and "B" uses bits 0-7. This is to accomodate
      // different pixel encodings such as RGB, BGR, RBG and so on...
#ifdef TOP8BITS_R8G8B8_USED
#  define X 8
#else
#  define X 0
#endif
      unsigned long r = (pfmt.RedShift == 16+X) ? Scan.FogR :
        (pfmt.GreenShift == 16+X) ? Scan.FogG : Scan.FogB;
      unsigned long g = (pfmt.RedShift == 8+X) ? Scan.FogR :
        (pfmt.GreenShift == 8+X) ? Scan.FogG : Scan.FogB;
      unsigned long b = (pfmt.RedShift == 0+X) ? Scan.FogR :
        (pfmt.GreenShift == 0+X) ? Scan.FogG : Scan.FogB;
#undef X
      Scan.FogR = r; Scan.FogG = g; Scan.FogB = b;
    }
  }
  else
  {
    Scan.FogR = QRound (fb->red * 255);
    Scan.FogG = QRound (fb->green * 255);
    Scan.FogB = QRound (fb->blue * 255);
    Scan.Fog8 = BuildIndexedFogTable ();
    Scan.FogPix = texman->find_rgb (Scan.FogR, Scan.FogG, Scan.FogB);
  }

  // Steps for interpolating horizontally accross a scanline.
  Scan.M = M;
  Scan.dM = M*Scan.InterpolStep;

  // Select the right scanline drawing function.
  csDrawScanline* dscan = NULL;
  int scan_index =
  	fog_type == CS_FOG_FRONT ?  SCANPROC_FOG :
	fog_type == CS_FOG_BACK ?  SCANPROC_ZFIL :
	fog_type == CS_FOG_VIEW ?  SCANPROC_FOG_VIEW :
	-1;

  if ((scan_index < 0) || !(dscan = ScanProc [scan_index]))
    return;   // Nothing to do.

  //@@@ Optimization note! We should have a seperate loop for CS_FOG_VIEW
  // as that is much simpler and does not require the calculations for z.
  // This would make things more efficient.

  // Scan both sides of the polygon at once.
  // We start with two pointers at the top (as seen in y-inverted
  // screen-space: bottom of display) and advance them until both
  // join together at the bottom. The way this algorithm works, this
  // should happen automatically; the left pointer is only advanced
  // when it is further away from the bottom than the right pointer
  // and vice versa.
  // Using this we effectively partition our polygon in trapezoids
  // with at most two triangles (one at the top and one at the bottom).

  int scanL1, scanL2, scanR1, scanR2;   // scan vertex left/right start/final
  float sxL, sxR, dxL, dxR;             // scanline X left/right and deltas
  int sy, fyL, fyR;                     // scanline Y, final Y left, final Y right
  int xL, xR;
  int screenY;

  sxL = sxR = dxL = dxR = 0;            // avoid GCC warnings about "uninitialized variables"
  scanL2 = scanR2 = max_i;
  sy = fyL = fyR = QRound (poly.vertices [scanL2].sy);

  for ( ; ; )
  {
    //-----
    // We have reached the next segment. Recalculate the slopes.
    //-----
    bool leave;
    do
    {
      leave = true;
      if (sy <= fyR)
      {
        // Check first if polygon has been finished
        if (scanR2 == min_i)
          return;
        scanR1 = scanR2;
	if (++scanR2 >= poly.num)
	  scanR2 = 0;

        leave = false;
        fyR = QRound (poly.vertices [scanR2].sy);
        if (sy <= fyR)
          continue;

        float dyR = (poly.vertices [scanR1].sy - poly.vertices [scanR2].sy);
        if (dyR)
        {
          sxR = poly.vertices [scanR1].sx;
          dxR = (poly.vertices [scanR2].sx - sxR) / dyR;
          // horizontal pixel correction
          sxR += dxR * (poly.vertices [scanR1].sy - (float (sy) - 0.5));
        } /* endif */
      } /* endif */
      if (sy <= fyL)
      {
        scanL1 = scanL2;
	if (--scanL2 < 0)
	  scanL2 = poly.num - 1;

        leave = false;
        fyL = QRound (poly.vertices [scanL2].sy);
        if (sy <= fyL)
          continue;

        float dyL = (poly.vertices [scanL1].sy - poly.vertices [scanL2].sy);
        if (dyL)
        {
          sxL = poly.vertices [scanL1].sx;
          dxL = (poly.vertices [scanL2].sx - sxL) / dyL;
          // horizontal pixel correction
          sxL += dxL * (poly.vertices [scanL1].sy - (float (sy) - 0.5));
        } /* endif */
      } /* endif */
    } while (!leave); /* enddo */

    // Steps for interpolating vertically over scanlines.
    float vd_inv_z = - dxL * M + N;

    float cx = (sxL - float (width2));
    float cy = (float (sy) - 0.5 - float (height2));
    float inv_z = M * cx + N * cy + O;

    // Find the trapezoid top (or bottom in inverted Y coordinates)
    int fin_y;
    if (fyL > fyR)
      fin_y = fyL;
    else
      fin_y = fyR;

    screenY = height - 1 - sy;

    while (sy > fin_y)
    {
      if ((sy & 1) != do_interlaced)
      {
        // Compute the rounded screen coordinates of horizontal strip
        xL = QRound (sxL);
        xR = QRound (sxR);

        // Sub-pixel U & V correction
        float deltaX = (float)xL - sxL;

        //G2D->GetPixelAt(xL, screenY, &d);
        d = line_table[screenY] + (xL << pixel_shift);
        z_buf = z_buffer + width * screenY + xL;

        // do not draw the rightmost pixel - it will be covered
        // by neightbour polygon's left bound
        dscan (xR - xL, d, z_buf, inv_z + deltaX * M, 0, 0);
      }

      sxL += dxL;
      sxR += dxR;
      inv_z -= vd_inv_z;
      sy--;
      screenY++;
    } /* endwhile */
  } /* endfor */
}

// Calculate round (f) of a 16:16 fixed pointer number
// and return a long integer.
inline long round16 (long f)
{
  return (f + 0x8000) >> 16;
}

// The static global variable that holds current PolygonQuick settings
static struct
{
  int redFact, greenFact, blueFact;
  int max_r, max_g, max_b;
  int twfp, thfp;
  float tw, th;
  unsigned char *bm;
  int shf_w;
  bool transparent;
  bool textured;
  UInt mixmode;
  csDrawPIScanline *drawline;
  csDrawPIScanlineGouraud *drawline_gouraud;
} pqinfo;

#define EPS   0.0001

void csGraphics3DSoftware::StartPolygonFX (iTextureHandle* handle,
  UInt mode)
{
  if (!rstate_gouraud || !do_lighting)
    mode &= ~CS_FX_GOURAUD;

  if (handle)
  {
    csTextureMMSoftware *tex_mm = (csTextureMMSoftware*)handle->GetPrivateObject ();
    csTexture *txt_unl = tex_mm->get_texture (0);
    pqinfo.bm = (UByte *)txt_unl->get_bitmap ();
    pqinfo.tw = txt_unl->get_width ();
    pqinfo.th = txt_unl->get_height ();
    pqinfo.shf_w = txt_unl->get_w_shift ();
    pqinfo.twfp = QInt16 (pqinfo.tw) - 1;
    pqinfo.thfp = QInt16 (pqinfo.th) - 1;
    pqinfo.transparent = tex_mm->GetTransparent ();
    pqinfo.textured = do_textured;
    Scan.PaletteTable = tex_mm->GetPaletteToGlobal ();
    Scan.TexturePalette = tex_mm->GetColorMap ();
  }
  else
    pqinfo.textured = false;

  Scan.inv_cmap = texman->inv_cmap;

  Scan.AlphaMask = alpha_mask;

  // Select draw scanline routine
  int scan_index = pqinfo.textured ? SCANPROCPI_TEX_ZFIL : SCANPROCPI_FLAT_ZFIL;
  if (z_buf_mode == CS_ZBUF_USE)
    scan_index++;
  pqinfo.drawline = ScanProcPI [scan_index];

  csDrawPIScanlineGouraud *gouraud_proc = ScanProcPIG [scan_index];
  if ((mode & CS_FX_MASK_MIXMODE) == CS_FX_COPY)
    pqinfo.drawline_gouraud = gouraud_proc;
  else
  {
    scan_index = (z_buf_mode == CS_ZBUF_USE) ? SCANPROCPIFX_ZUSE : SCANPROCPIFX_ZFIL;
    if (pqinfo.transparent) scan_index += 2;
    pqinfo.drawline_gouraud = ScanProcPIFX [scan_index];
  }

  Scan.BlendTable = NULL;
  switch (mode & CS_FX_MASK_MIXMODE)
  {
    case CS_FX_ADD:
      Scan.BlendTable = Scan.BlendingTable [BLENDTABLE_ADD];
      break;
    case CS_FX_MULTIPLY:
      Scan.BlendTable = Scan.BlendingTable [BLENDTABLE_MULTIPLY];
      break;
    case CS_FX_MULTIPLY2:
      Scan.BlendTable = Scan.BlendingTable [BLENDTABLE_MULTIPLY2];
      break;
    case CS_FX_ALPHA:
    {
      int alpha = mode & CS_FX_MASK_ALPHA;
      if (alpha < 12)
        //Please _dont't_ optimize this _again_! You can't replace a 
        //BLENDTABLE_ALPHA00 with a pqinfo.drawline_gouraud = gouraud_proc,
        //because gouraud_proc does not correctly handle colorkeying.
        //so either fix gouraud_proc or leave this BLENDTABLE_ALPHA00 alone
        //Thomas Hieber, Feb. 10th 2000
        Scan.BlendTable = Scan.BlendingTable [BLENDTABLE_ALPHA00];
      else if (alpha < 96)
        Scan.BlendTable = Scan.BlendingTable [BLENDTABLE_ALPHA25];
      else if (alpha < 160)
        Scan.BlendTable = Scan.BlendingTable [BLENDTABLE_ALPHA50];
      else if (alpha < 244)
        Scan.BlendTable = Scan.BlendingTable [BLENDTABLE_ALPHA75];
      else
      {
        mode &= ~CS_FX_GOURAUD;
        pqinfo.drawline = csScan_draw_pi_scanline_zfil;
      }
      break;
    }
    case CS_FX_TRANSPARENT:
      mode &= ~CS_FX_GOURAUD;
      pqinfo.drawline = csScan_draw_pi_scanline_zfil;
      break;
    case CS_FX_COPY:
    default:
      pqinfo.drawline_gouraud = gouraud_proc;
      break;
  }

  // Once again check for availability of gouraud procedure
  if (!pqinfo.drawline_gouraud)
  {
    mode &= ~(CS_FX_GOURAUD | CS_FX_MASK_MIXMODE);
    Scan.BlendTable = NULL;
  }

  pqinfo.mixmode = mode;
  // We use #.16 fixed-point format for R,G,B factors
  // where # is the number of bits per component (with the exception of
  // 32bpp modes/textured where we use (#-2).16 format).
  int shift_amount =
    ((pfmt.PixelBytes == 4) && (Scan.BlendTable || pqinfo.textured)) ? 6 : 8;
  pqinfo.redFact   = (pfmt.RedMask >> pfmt.RedShift)     << shift_amount;
  pqinfo.greenFact = (pfmt.GreenMask >> pfmt.GreenShift) << shift_amount;
  pqinfo.blueFact  = (pfmt.BlueMask >> pfmt.BlueShift)   << shift_amount;

  pqinfo.max_r = (1 << (pfmt.RedBits   + shift_amount + 8)) - 1;
  pqinfo.max_g = (1 << (pfmt.GreenBits + shift_amount + 8)) - 1;
  pqinfo.max_b = (1 << (pfmt.BlueBits  + shift_amount + 8)) - 1;
}

void csGraphics3DSoftware::FinishPolygonFX()
{
}

void csGraphics3DSoftware::DrawPolygonFX (G3DPolygonDPFX& poly)
{
  if (!pqinfo.drawline && !pqinfo.drawline_gouraud)
    return;

  //-----
  // Calculate constant du,dv,dz.
  //
  //          (At-Ct)*(By-Cy) - (Bt-Ct)*(Ay-Cy)
  //  dt/dx = ---------------------------------
  //          (Ax-Cx)*(By-Cy) - (Bx-Cx)*(Ay-Cy)
  //-----

  int last;
  float dd = 0;
  for (last = 2; last < poly.num; last++)
  {
    dd = (poly.vertices [0].sx - poly.vertices [last].sx) *
         (poly.vertices [1].sy - poly.vertices [last].sy) -
         (poly.vertices [1].sx - poly.vertices [last].sx) *
         (poly.vertices [0].sy - poly.vertices [last].sy);
    if (dd < 0)
      break;
  }

  // Rejection of back-faced polygons
  if ((last == poly.num) || (dd == 0))
    return;

  int flat_r, flat_g, flat_b;
  if (pqinfo.textured)
    flat_r = flat_g = flat_b = 255;
  else
  {
    flat_r = poly.flat_color_r;
    flat_g = poly.flat_color_g;
    flat_b = poly.flat_color_b;
  }
  Scan.FlatRGB = RGBPixel (flat_r, flat_g, flat_b);

  //-----
  // Get the values from the polygon for more conveniant local access.
  // Also look for the top-most and bottom-most vertices.
  //-----
  float uu[64], vv[64], iz[64];
  float rr[64], gg[64], bb[64];
  int top, bot;
  float top_y = -99999;
  float bot_y = 99999;
  top = bot = 0;                        // avoid GCC complains
  int i;
  for (i = 0 ; i < poly.num ; i++)
  {
    uu[i] = pqinfo.tw * poly.vertices [i].u;
    vv[i] = pqinfo.th * poly.vertices [i].v;
    iz[i] = poly.vertices [i].z;
    if (poly.vertices [i].r > 2.0) poly.vertices [i].r = 2.0;
    if (poly.vertices [i].r < 0.0) poly.vertices [i].r = 0.0;
    rr[i] = poly.vertices [i].r * pqinfo.redFact   * flat_r;
    if (poly.vertices [i].g > 2.0) poly.vertices [i].g = 2.0;
    if (poly.vertices [i].g < 0.0) poly.vertices [i].g = 0.0;
    gg[i] = poly.vertices [i].g * pqinfo.greenFact * flat_g;
    if (poly.vertices [i].b > 2.0) poly.vertices [i].b = 2.0;
    if (poly.vertices [i].b < 0.0) poly.vertices [i].b = 0.0;
    bb[i] = poly.vertices [i].b * pqinfo.blueFact  * flat_b;
    if (poly.vertices [i].sy > top_y)
      top_y = poly.vertices [top = i].sy;
    if (poly.vertices [i].sy < bot_y)
      bot_y = poly.vertices [bot = i].sy;
  }

  float inv_dd = 1 / dd;
  int dz = QInt24 (((iz [0] - iz [last]) * (poly.vertices [1].sy - poly.vertices [last].sy)
                  - (iz [1] - iz [last]) * (poly.vertices [0].sy - poly.vertices [last].sy)) * inv_dd);
  int du = 0, dv = 0;
  if (pqinfo.textured)
  {
    du = QInt16 (((uu [0] - uu [last]) * (poly.vertices [1].sy - poly.vertices [last].sy)
                - (uu [1] - uu [last]) * (poly.vertices [0].sy - poly.vertices [last].sy)) * inv_dd);
    dv = QInt16 (((vv [0] - vv [last]) * (poly.vertices [1].sy - poly.vertices [last].sy)
                - (vv [1] - vv [last]) * (poly.vertices [0].sy - poly.vertices [last].sy)) * inv_dd);
  }
  long dr = 0, dg = 0, db = 0;
  if (pqinfo.mixmode & CS_FX_GOURAUD)
  {
    dr = QRound (((rr [0] - rr [last]) * (poly.vertices [1].sy - poly.vertices [last].sy)
                - (rr [1] - rr [last]) * (poly.vertices [0].sy - poly.vertices [last].sy)) * inv_dd);
    dg = QRound (((gg [0] - gg [last]) * (poly.vertices [1].sy - poly.vertices [last].sy)
                - (gg [1] - gg [last]) * (poly.vertices [0].sy - poly.vertices [last].sy)) * inv_dd);
    db = QRound (((bb [0] - bb [last]) * (poly.vertices [1].sy - poly.vertices [last].sy)
                - (bb [1] - bb [last]) * (poly.vertices [0].sy - poly.vertices [last].sy)) * inv_dd);
  }

  //-----
  // Scan from top to bottom.
  // scanL1-scanL2 is the left segment to scan.
  // scanR1-scanR2 is the right segment to scan.
  //-----
  int scanL1, scanL2 = top;
  int scanR1, scanR2 = top;

  int xL = 0, xR = 0, dxdyL = 0, dxdyR = 0;
  int uL = 0, vL = 0, zL = 0, rL = 0, gL = 0, bL = 0;
  int dudyL = 0, dvdyL = 0, dzdyL = 0, drdyL = 0, dgdyL = 0, dbdyL = 0;

  int sy, fyL, fyR;
  sy = fyL = fyR = QRound (poly.vertices [top].sy);

  // Decide whenever we should use Gouraud or flat (faster) routines
  bool use_gouraud = (pqinfo.drawline_gouraud != NULL)
    && ((pqinfo.mixmode & CS_FX_GOURAUD)
     || (pqinfo.mixmode & CS_FX_MASK_MIXMODE) != CS_FX_COPY);
  if (use_gouraud && !(pqinfo.mixmode & CS_FX_GOURAUD))
  {
    rL = pqinfo.redFact   << 8;
    gL = pqinfo.greenFact << 8;
    bL = pqinfo.blueFact  << 8;
  } /* endif */

  //-----
  // The loop.
  //-----
  for ( ; ; )
  {
    //-----
    // We have reached the next segment. Recalculate the slopes.
    //-----
    bool leave;
    do
    {
      leave = true;
      if (sy <= fyR)
      {
        // Check first if polygon has been finished
        if (scanR2 == bot)
          return;
        scanR1 = scanR2;
	if (++scanR2 >= poly.num)
	  scanR2 = 0;

        leave = false;
	fyR = QRound (poly.vertices [scanR2].sy);
	if (sy <= fyR)
	  continue;

        float dyR = poly.vertices [scanR1].sy - poly.vertices [scanR2].sy;
        if (dyR)
        {
          xR = QInt16 (poly.vertices [scanR1].sx);
          dxdyR = QInt16 ((poly.vertices [scanR2].sx - poly.vertices [scanR1].sx) / dyR);
          // horizontal pixel correction
          xR += QRound (dxdyR * (poly.vertices [scanR1].sy -
            ((float)QRound (poly.vertices [scanR1].sy) - 0.5)));
        } /* endif */
      } /* endif */
      if (sy <= fyL)
      {
        if (scanL2 == bot)
          return;
        scanL1 = scanL2;
	if (--scanL2 < 0)
	  scanL2 = poly.num - 1;

        leave = false;
	fyL = QRound (poly.vertices [scanL2].sy);
	if (sy <= fyL)
	  continue;

        float dyL = poly.vertices [scanL1].sy - poly.vertices [scanL2].sy;
        if (dyL)
        {
          float inv_dyL = 1/dyL;
          dxdyL = QInt16 ((poly.vertices [scanL2].sx - poly.vertices [scanL1].sx) * inv_dyL);
          if (pqinfo.textured)
          {
            dudyL = QInt16 ((uu[scanL2] - uu[scanL1]) * inv_dyL);
            dvdyL = QInt16 ((vv[scanL2] - vv[scanL1]) * inv_dyL);
          }
          dzdyL = QInt24 ((iz[scanL2] - iz[scanL1]) * inv_dyL);
          if (pqinfo.mixmode & CS_FX_GOURAUD)
          {
            drdyL = QRound ((rr [scanL2] - rr [scanL1]) * inv_dyL);
            dgdyL = QRound ((gg [scanL2] - gg [scanL1]) * inv_dyL);
            dbdyL = QRound ((bb [scanL2] - bb [scanL1]) * inv_dyL);
          }
          xL = QInt16 (poly.vertices [scanL1].sx);

          // horizontal pixel correction
          float deltaY = poly.vertices [scanL1].sy - (float (sy) - 0.5);
          float deltaX = (dxdyL / 65536.) * deltaY;
          xL += QInt16 (deltaX);

          // apply sub-pixel accuracy factor
          float Factor;
          if (poly.vertices [scanL2].sx != poly.vertices [scanL1].sx)
            Factor = deltaX / (poly.vertices [scanL2].sx - poly.vertices [scanL1].sx);
          else
            Factor = 0;

          if (pqinfo.textured)
          {
            uL = QInt16 (uu [scanL1] + (uu [scanL2] - uu [scanL1]) * Factor);
            vL = QInt16 (vv [scanL1] + (vv [scanL2] - vv [scanL1]) * Factor);
          }
          zL = QInt24 (iz [scanL1] + (iz [scanL2] - iz [scanL1]) * Factor);
          if (pqinfo.mixmode & CS_FX_GOURAUD)
          {
            rL = QRound (rr [scanL1] + (rr [scanL2] - rr [scanL1]) * Factor);
            gL = QRound (gg [scanL1] + (gg [scanL2] - gg [scanL1]) * Factor);
            bL = QRound (bb [scanL1] + (bb [scanL2] - bb [scanL1]) * Factor);
          }
        } /* endif */
      } /* endif */
    } while (!leave); /* enddo */

    //-----
    // Now draw a trapezoid.
    //-----
    int fin_y;
    if (fyL > fyR)
      fin_y = fyL;
    else
      fin_y = fyR;

    int screenY = height - 1 - sy;
    while (sy > fin_y)
    {
      if ((sy & 1) != do_interlaced)
      {
        //-----
        // Draw one scanline.
        //-----
        int xl = round16 (xL);
        int xr = round16 (xR);

        if (xr > xl)
        {
          int l = xr - xl;

          int uu = uL, vv = vL;
          int duu = du, dvv = dv;

          if (pqinfo.textured)
          {
            // Check for texture overflows
            if (uu < 0) uu = 0; if (uu > pqinfo.twfp) uu = pqinfo.twfp;
            if (vv < 0) vv = 0; if (vv > pqinfo.thfp) vv = pqinfo.thfp;

            int tmpu = uu + du * l;
            int tmpv = vv + dv * l;
            if (tmpu < 0 || tmpu > pqinfo.twfp)
            {
              if (tmpu < 0) tmpu = 0; if (tmpu > pqinfo.twfp) tmpu = pqinfo.twfp;
              duu = (tmpu - uu) / l;
            }
            if (tmpv < 0 || tmpv > pqinfo.thfp)
            {
              if (tmpv < 0) tmpv = 0; if (tmpv > pqinfo.thfp) tmpv = pqinfo.thfp;
              dvv = (tmpv - vv) / l;
            }
          }

          // R,G,B brightness can underflow due to pixel subcorrection
          // Underflow will cause visual artifacts while small overflows
          // will be neutralized by our "clamp to 1.0" circuit.
          int rr = rL, gg = gL, bb = bL;
          int drr = dr; int dgg = dg; int dbb = db;
          bool clamp = false;
          if (pqinfo.mixmode & CS_FX_GOURAUD)
          {
            if (rr < 0) rr = 0;
            if (gg < 0) gg = 0;
            if (bb < 0) bb = 0;

            int tmp = rr + drr * l;
            if (tmp < 0) drr = - (rr / l);
            clamp |= (rr > pqinfo.max_r) || (tmp > pqinfo.max_r);
            tmp = gg + dgg * l;
            if (tmp < 0) dgg = - (gg / l);
            clamp |= (gg > pqinfo.max_g) || (tmp > pqinfo.max_g);
            tmp = bb + dbb * l;
            if (tmp < 0) dbb = - (bb / l);
            clamp |= (bb > pqinfo.max_b) || (tmp > pqinfo.max_b);
          }

          unsigned long *zbuff = z_buffer + width * screenY + xl;
          unsigned char *dest = line_table [screenY] + (xl << pixel_shift);

          if (use_gouraud)
            pqinfo.drawline_gouraud (dest, l, zbuff, uu, duu, vv, dvv,
              zL, dz, pqinfo.bm, pqinfo.shf_w, rr, gg, bb, drr, dgg, dbb, clamp);
          else
            pqinfo.drawline (dest, l, zbuff, uu, duu, vv, dvv,
              zL, dz, pqinfo.bm, pqinfo.shf_w);
        }
      }

      xL += dxdyL; xR += dxdyR; zL += dzdyL;
      if (pqinfo.textured)
        uL += dudyL, vL += dvdyL;
      if (pqinfo.mixmode & CS_FX_GOURAUD)
        rL += drdyL, gL += dgdyL, bL += dbdyL;

      sy--;
      screenY++;
    }
  }
}


unsigned char *csGraphics3DSoftware::BuildIndexedFogTable ()
{
  static int usage = 0;
  usage++;

  // first look if a fog table with given R,G,B has already been built
  int i;
  for (i = 0; i < MAX_INDEXED_FOG_TABLES; i++)
    if (fog_tables [i].table
     && (fog_tables [i].r == Scan.FogR)
     && (fog_tables [i].g == Scan.FogG)
     && (fog_tables [i].b == Scan.FogB))
    {
      fog_tables [i].lastuse = usage;
      return fog_tables [i].table;
    }

  // We have to build this table: find a free slot
  // in fog tables or free the least recently used one
  int fi = -1, lr = -1;
  for (i = 0; i < MAX_INDEXED_FOG_TABLES; i++)
    if (!fog_tables [i].table)
    {
      fi = i;
      break;
    }
    else if (usage - fog_tables [i].lastuse > lr)
    {
      fi = i;
      lr = usage - fog_tables [i].lastuse;
    }

  if (fog_tables [fi].table)
    delete [] fog_tables [fi].table;
  fog_tables [fi].table = new unsigned char [32 * 4096];
  if (!fog_tables [fi].table)
    return NULL;

  unsigned char *dest = fog_tables [fi].table;
  for (i = 0; i < 256; i++)
  {
    int r = texman->cmap [i].red;
    int g = texman->cmap [i].green;
    int b = texman->cmap [i].blue;
    for (int j = 1; j <= 32; j++)
      dest [(j - 1) * 256 + i] = texman->find_rgb (
        Scan.FogR + ((j * (r - Scan.FogR)) >> 5),
        Scan.FogG + ((j * (g - Scan.FogG)) >> 5),
        Scan.FogB + ((j * (b - Scan.FogB)) >> 5));
  }

  fog_tables [fi].lastuse = usage;
  return fog_tables [fi].table;
}

bool csGraphics3DSoftware::SetRenderState (G3D_RENDERSTATEOPTION op,
  long value)
{
  switch (op)
  {
    case G3DRENDERSTATE_NOTHING:
      return true;
    case G3DRENDERSTATE_ZBUFFERTESTENABLE:
      if (value)
      {
         if (z_buf_mode == CS_ZBUF_TEST)
           return true;
         if (z_buf_mode == CS_ZBUF_NONE)
           z_buf_mode = CS_ZBUF_TEST;
         else if (z_buf_mode == CS_ZBUF_FILL)
           z_buf_mode = CS_ZBUF_USE;
      }
      else
      {
         if (z_buf_mode == CS_ZBUF_FILL)
           return true;
         if (z_buf_mode == CS_ZBUF_USE)
           z_buf_mode = CS_ZBUF_FILL;
         else if (z_buf_mode == CS_ZBUF_TEST)
           z_buf_mode = CS_ZBUF_NONE;
      }
      break;
    case G3DRENDERSTATE_ZBUFFERFILLENABLE:
      if (value)
      {
        if (z_buf_mode == CS_ZBUF_FILL)
          return true;
        if (z_buf_mode == CS_ZBUF_NONE)
          z_buf_mode = CS_ZBUF_FILL;
        else if (z_buf_mode == CS_ZBUF_TEST)
          z_buf_mode = CS_ZBUF_USE;
      }
      else
      {
        if (z_buf_mode == CS_ZBUF_TEST)
          return true;
        if (z_buf_mode == CS_ZBUF_USE)
          z_buf_mode = CS_ZBUF_TEST;
        else if (z_buf_mode == CS_ZBUF_FILL)
          z_buf_mode = CS_ZBUF_NONE;
      }
      break;
    case G3DRENDERSTATE_DITHERENABLE:
      rstate_dither = value;
      ScanSetup ();
      break;
    case G3DRENDERSTATE_SPECULARENABLE:
      rstate_specular = value;
      ScanSetup ();
      break;
    case G3DRENDERSTATE_BILINEARMAPPINGENABLE:
      do_bilin_filt = value ? 1 : 0;
      ScanSetup ();
      break;
    case G3DRENDERSTATE_TRILINEARMAPPINGENABLE:
      do_bilin_filt = value ? 2 : 0;
      ScanSetup ();
      break;
    case G3DRENDERSTATE_TRANSPARENCYENABLE:
      do_transp = value;
      ScanSetup ();
      break;
    case G3DRENDERSTATE_MIPMAPENABLE:
      rstate_mipmap = value;
      break;
    case G3DRENDERSTATE_TEXTUREMAPPINGENABLE:
      do_textured = value;
      ScanSetup ();
      break;
    case G3DRENDERSTATE_MMXENABLE:
#ifdef DO_MMX
      do_mmx = value;
      ScanSetup ();
      break;
#else
      return false;
#endif
    case G3DRENDERSTATE_INTERLACINGENABLE:
      if (!G2D->DoubleBuffer (!value))
        return false;
      do_interlaced = value ? 0 : -1;
      break;
    case G3DRENDERSTATE_INTERPOLATIONSTEP:
      Scan.InterpolMode = value;
      break;
    case G3DRENDERSTATE_LIGHTINGENABLE:
      do_lighting = value;
      break;
    case G3DRENDERSTATE_MAXPOLYGONSTODRAW:
      dbg_max_polygons_to_draw = value;
      if (dbg_max_polygons_to_draw < 0) dbg_max_polygons_to_draw = 0;
      break;
    case G3DRENDERSTATE_GOURAUDENABLE:
      rstate_gouraud = value;
      break;
    default:
      return false;
  }

  return true;
}

long csGraphics3DSoftware::GetRenderState(G3D_RENDERSTATEOPTION op)
{
  switch (op)
  {
    case G3DRENDERSTATE_NOTHING:
      return 0;
    case G3DRENDERSTATE_ZBUFFERTESTENABLE:
      return (bool)(z_buf_mode & CS_ZBUF_TEST);
    case G3DRENDERSTATE_ZBUFFERFILLENABLE:
      return (bool)(z_buf_mode & CS_ZBUF_FILL);
    case G3DRENDERSTATE_DITHERENABLE:
      return rstate_dither;
    case G3DRENDERSTATE_SPECULARENABLE:
      return rstate_specular;
    case G3DRENDERSTATE_BILINEARMAPPINGENABLE:
      return do_bilin_filt == 1 ? 1 : 0;
    case G3DRENDERSTATE_TRILINEARMAPPINGENABLE:
      return do_bilin_filt == 2 ? 1 : 0;
    case G3DRENDERSTATE_TRANSPARENCYENABLE:
      return do_transp;
    case G3DRENDERSTATE_MIPMAPENABLE:
      return rstate_mipmap;
    case G3DRENDERSTATE_TEXTUREMAPPINGENABLE:
      return do_textured;
    case G3DRENDERSTATE_MMXENABLE:
#ifdef DO_MMX
      return do_mmx;
#else
      return 0;
#endif
    case G3DRENDERSTATE_INTERLACINGENABLE:
      return do_interlaced == -1 ? false : true;
    case G3DRENDERSTATE_INTERPOLATIONSTEP:
      return Scan.InterpolMode;
    case G3DRENDERSTATE_LIGHTINGENABLE:
      return do_lighting;
    case G3DRENDERSTATE_MAXPOLYGONSTODRAW:
      return dbg_max_polygons_to_draw;
    case G3DRENDERSTATE_GOURAUDENABLE:
      return rstate_gouraud;
    default:
      return 0;
  }
}

void csGraphics3DSoftware::GetCaps(G3D_CAPS *caps)
{
  if (!caps)
    return;

  caps->ColorModel = G3DCOLORMODEL_RGB;
  caps->CanClip = false;
  caps->SupportsArbitraryMipMapping = true;
  caps->BitDepth = pfmt.PixelBytes * 8;
  caps->ZBufBitDepth = 32;
  caps->minTexHeight = 2;
  caps->minTexWidth = 2;
  caps->maxTexHeight = 1024;
  caps->maxTexWidth = 1024;
  caps->PrimaryCaps.RasterCaps = G3DRASTERCAPS_SUBPIXEL;
  caps->PrimaryCaps.canBlend = true;
  caps->PrimaryCaps.ShadeCaps = G3DRASTERCAPS_LIGHTMAP;
  caps->PrimaryCaps.PerspectiveCorrects = true;
  caps->PrimaryCaps.FilterCaps = G3D_FILTERCAPS((int)G3DFILTERCAPS_NEAREST | (int)G3DFILTERCAPS_MIPNEAREST);
  caps->fog = G3D_FOGMETHOD((int)G3DFOGMETHOD_ZBUFFER);
}

void csGraphics3DSoftware::ClearCache()
{
  if (tcache) tcache->Clear ();
}

void csGraphics3DSoftware::RemoveFromCache (iPolygonTexture* poly_texture)
{
  if (tcache)
  {
    tcache->uncache_texture (0, poly_texture);
    tcache->uncache_texture (1, poly_texture);
    tcache->uncache_texture (2, poly_texture);
    tcache->uncache_texture (3, poly_texture);
  }
}

void csGraphics3DSoftware::DumpCache()
{
  if (tcache) tcache->dump (this);
}

void csGraphics3DSoftware::DrawLine (csVector3& v1, csVector3& v2, float fov, int color)
{
  if (v1.z < SMALL_Z && v2.z < SMALL_Z)
    return;

  float x1 = v1.x, y1 = v1.y, z1 = v1.z;
  float x2 = v2.x, y2 = v2.y, z2 = v2.z;

  if (z1 < SMALL_Z)
  {
    // x = t*(x2-x1)+x1;
    // y = t*(y2-y1)+y1;
    // z = t*(z2-z1)+z1;
    float t = (SMALL_Z-z1) / (z2-z1);
    x1 = t*(x2-x1)+x1;
    y1 = t*(y2-y1)+y1;
    z1 = SMALL_Z;
  }
  else if (z2 < SMALL_Z)
  {
    // x = t*(x2-x1)+x1;
    // y = t*(y2-y1)+y1;
    // z = t*(z2-z1)+z1;
    float t = (SMALL_Z-z1) / (z2-z1);
    x2 = t*(x2-x1)+x1;
    y2 = t*(y2-y1)+y1;
    z2 = SMALL_Z;
  }

  float iz1 = fov/z1;
  int px1 = QInt (x1 * iz1 + (width/2));
  int py1 = height - 1 - QInt (y1 * iz1 + (height/2));
  float iz2 = fov/z2;
  int px2 = QInt (x2 * iz2 + (width/2));
  int py2 = height - 1 - QInt (y2 * iz2 + (height/2));

  G2D->DrawLine (px1, py1, px2, py2, color);
}

float csGraphics3DSoftware::GetZbuffValue (int x, int y)
{
  unsigned long zbf = z_buffer [x + y * width];
  if (!zbf) return 0;
  return 16777216.0 / float (zbf);
}

//---------------------------------------------------------------------------

IMPLEMENT_EMBEDDED_IBASE (csGraphics3DSoftware::csSoftConfig)
  IMPLEMENTS_INTERFACE (iConfig)
IMPLEMENT_EMBEDDED_IBASE_END

#define NUM_OPTIONS 8

static const csOptionDescription config_options [NUM_OPTIONS] =
{
  { 0, "ilace", "Interlacing", CSVAR_BOOL },
  { 1, "light", "Texture lighting", CSVAR_BOOL },
  { 2, "transp", "Transparent textures", CSVAR_BOOL },
  { 3, "txtmap", "Texture mapping", CSVAR_BOOL },
  { 4, "mmx", "MMX support", CSVAR_BOOL },
  { 5, "gamma", "Gamma value", CSVAR_FLOAT },
  { 6, "gouraud", "Gouraud shading", CSVAR_BOOL },
  { 7, "smaller", "Smaller rendering", CSVAR_BOOL },
};

bool csGraphics3DSoftware::csSoftConfig::SetOption (int id, csVariant* value)
{
  if (value->type != config_options[id].type)
    return false;
  switch (id)
  {
    case 0: scfParent->do_interlaced = value->v.b ? 0 : -1; break;
    case 1: scfParent->do_lighting = value->v.b; break;
    case 2: scfParent->do_transp = value->v.b; break;
    case 3: scfParent->do_textured = value->v.b; break;
#ifdef DO_MMX
    case 4: scfParent->do_mmx = value->v.b; break;
#endif
    case 5: scfParent->texman->Gamma = value->v.f; break;
    case 6: scfParent->rstate_gouraud = value->v.b; break;
    case 7: scfParent->do_smaller_rendering = value->v.b; break;
    default: return false;
  }
  scfParent->ScanSetup ();
  return true;
}

bool csGraphics3DSoftware::csSoftConfig::GetOption (int id, csVariant* value)
{
  value->type = config_options[id].type;
  switch (id)
  {
    case 0: value->v.b = scfParent->do_interlaced != -1; break;
    case 1: value->v.b = scfParent->do_lighting; break;
    case 2: value->v.b = scfParent->do_transp; break;
    case 3: value->v.b = scfParent->do_textured; break;
#ifdef DO_MMX
    case 4: value->v.b = scfParent->do_mmx; break;
#endif
    case 5: value->v.f = scfParent->texman->Gamma; break;
    case 6: value->v.b = scfParent->rstate_gouraud; break;
    case 7: value->v.b = scfParent->do_smaller_rendering; break;
    default: return false;
  }
  return true;
}

bool csGraphics3DSoftware::csSoftConfig::GetOptionDescription
  (int idx, csOptionDescription* option)
{
  if (idx < 0 || idx >= NUM_OPTIONS)
    return false;
  *option = config_options[idx];
  return true;
}

