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

#include <stdarg.h>

#include "sysdef.h"
#include "qint.h"
#include "cscom/com.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "soft_g3d.h"
#include "scan.h"
#include "tcache.h"
#include "tcache16.h"
#include "tcache32.h"
#include "soft_txt.h"
#include "tables.h"
#include "ipolygon.h"
#include "isystem.h"
#include "igraph2d.h"
#include "ilghtmap.h"

#if defined (DO_MMX)
#  include "i386/cpuid.h"
#endif

//-------------------------- The indices into arrays of scanline routines ------

/*
 *  The rules for scanproc index name building:
 *  Curly brackets means optional name components
 *  Square brackets denote enforced name components
 *  Everything outside brackets is a must
 *
 *  SCANPROC{Persp}{Gouraud}_{Source_{Smode_}}{Zmode_}
 *
 *  Persp       = PI for perspective-incorrect routines
 *  Gouraud     = G for Gouraud-shading routines
 *  Source      = TEX for non-lightmapped textures
 *                MAP for lightmapped textures
 *                FLAT for flat-shaded
 *                FOG for drawing fog
 *  SMode       = KEY for "key-color" source pixel removal
 *                FILT for filtered texture (possibly bi-linear)
 *                GOURAUD for Gouraud-shading applied to the texture
 *  Zmode       = ZUSE for polys that are tested against Z-buffer (and fills)
 *                ZFIL for polys that just fills Z-buffer without testing
 *
 *  Example:
 *      SCANPROC_TEX_ZFIL_PRIV
 *              scanline procedure for drawing a non-lightmapped texture
 *              with Z-fill and with a private palette table
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
#define SCANPROC_MAP_FILT_ZFIL          0x06
#define SCANPROC_MAP_FILT_ZUSE          0x07
#define SCANPROC_TEX_KEY_ZFIL           0x08
#define SCANPROC_TEX_KEY_ZUSE           0x09
#define SCANPROC_MAP_KEY_ZFIL           0x0A
#define SCANPROC_MAP_KEY_ZUSE           0x0B
// these do not have "zuse" counterparts
#define SCANPROC_ZFIL                   0x10
#define SCANPROC_FOG                    0x11
#define SCANPROC_FOG_VIEW               0x12
#define SCANPROC_FOG_PLANE              0x13

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

#define BLENDTABLE_COPY                 0x00
#define BLENDTABLE_ADD                  0x01
#define BLENDTABLE_MULTIPLY             0x02
#define BLENDTABLE_MULTIPLY2            0x03
#define BLENDTABLE_ALPHA25              0x04
#define BLENDTABLE_ALPHA50              0x05
#define BLENDTABLE_ALPHA75              0x06
#define BLENDTABLE_ALPHA100             0x07

///---------------------------------------------------------------------------

IMPLEMENT_UNKNOWN (csGraphics3DSoftware)

#ifndef BUGGY_EGCS_COMPILER

BEGIN_INTERFACE_TABLE (csGraphics3DSoftware)
    IMPLEMENTS_INTERFACE (IGraphics3D)
#if defined(OS_NEXT) && defined(PROC_SPARC)
// FIXME: NextStep multiple-inheritance compiler crasher [sparc]
#else
    IMPLEMENTS_INTERFACE (IHaloRasterizer)
#endif
    IMPLEMENTS_COMPOSITE_INTERFACE_EX (IConfig, XConfig3DSoft)
END_INTERFACE_TABLE ()

#else

// this is a fake routine to circumvent the compiler bug. const removed to allow assignment
const INTERFACE_ENTRY *csGraphics3DSoftware::GetInterfaceTable ()
{
  static INTERFACE_ENTRY InterfaceTable[4];
  InterfaceTable[0].pIID = &IID_IGraphics3D;
  InterfaceTable[0].pfnFinder = ENTRY_IS_OFFSET;
  InterfaceTable[0].dwData = BASE_OFFSET(csGraphics3DSoftware, IGraphics3D);
  InterfaceTable[1].pIID = &IID_IHaloRasterizer;
  InterfaceTable[1].pfnFinder = ENTRY_IS_OFFSET;
  InterfaceTable[1].dwData = BASE_OFFSET(csGraphics3DSoftware, IHaloRasterizer);
  InterfaceTable[2].pIID = &IID_IConfig;
  InterfaceTable[2].pfnFinder = ENTRY_IS_OFFSET;
  InterfaceTable[2].dwData = COMPOSITE_OFFSET(csGraphics3DSoftware, IConfig, IXConfig3DSoft, m_xXConfig3DSoft);
  InterfaceTable[3].pIID = 0;
  InterfaceTable[3].pfnFinder = 0;
  InterfaceTable[3].dwData = 0;
  return InterfaceTable;
}

#endif

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

csGraphics3DSoftware::csGraphics3DSoftware (ISystem* piSystem) : m_piG2D(NULL)
{
  HRESULT hRes;
  CLSID clsid2dDriver;
  char *sz2DDriver = SOFTWARE_2D_DRIVER;        // "crystalspace.graphics2d.xxx"
  IGraphics2DFactory* piFactory = NULL;
  tcache = NULL;
  txtmgr = NULL;

  m_piSystem = piSystem;

  hRes = csCLSIDFromProgID( &sz2DDriver, &clsid2dDriver );

  if (FAILED(hRes))
  {
    SysPrintf(MSG_FATAL_ERROR, "FATAL: Cannot open \"%s\" 2D Graphics driver", sz2DDriver);
    exit(0);
  }

  hRes = csCoGetClassObject( clsid2dDriver, CLSCTX_INPROC_SERVER, NULL, (REFIID)IID_IGraphics2DFactory, (void**)&piFactory );
  if (FAILED(hRes))
  {
    SysPrintf(MSG_FATAL_ERROR, "Error! Couldn't create 2D graphics driver instance.");
    exit(0);
  }

  hRes = piFactory->CreateInstance( (REFIID)IID_IGraphics2D, m_piSystem, (void**)&m_piG2D );
  if (FAILED(hRes))
  {
    SysPrintf(MSG_FATAL_ERROR, "Error! Couldn't create 2D graphics driver instance.");
    exit(0);
  }

  FINAL_RELEASE( piFactory );

  CHK (txtmgr = new csTextureManagerSoftware (m_piSystem, m_piG2D));

  zdist_mipmap1 = 12;
  zdist_mipmap2 = 24;
  zdist_mipmap3 = 40;

#ifdef DO_MMX
  do_mmx = true;
#endif
  do_lighting = true;
  do_transp = true;
  do_textured = true;
  do_debug = false;
  do_interlaced = -1;
  ilace_fastmove = false;
  do_texel_filt = false;
  do_bilin_filt = false;
  do_transp = true;
  do_textured = true;
  rstate_mipmap = 0;
  rstate_gouraud = true;
  rstate_specular = true;
  rstate_dither = false;

  dbg_max_polygons_to_draw = 2000000000;        // After 2 billion polygons we give up :-)

  fogMode = G3DFOGMETHOD_ZBUFFER;
  //fogMode = G3DFOGMETHOD_PLANES;
}

csGraphics3DSoftware::~csGraphics3DSoftware ()
{
  Close ();
  CHK (delete [] z_buffer);
  FINAL_RELEASE (m_piG2D);

  while (fog_buffers)
  {
    FogBuffer* n = fog_buffers->next;
    CHK (delete fog_buffers);
    fog_buffers = n;
  }

  CHK (delete [] line_table);
  CHK (delete txtmgr);
}

void csGraphics3DSoftware::ScanSetup ()
{
  // Select the right scanline drawing functions
  memset (&ScanProc, 0, sizeof (ScanProc));
  memset (&ScanProcPI, 0, sizeof (ScanProcPI));
  memset (&ScanProcPIG, 0, sizeof (ScanProcPIG));
  memset (&ScanProcPIFX, 0, sizeof (ScanProcPIFX));
  ScanProc_Alpha = NULL;

  bool PrivTex = (txtmgr->txtMode == TXT_PRIVATE);
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

      ScanProc [SCANPROC_TEX_ZFIL] = PrivTex ?
        csScan_8_draw_scanline_tex_priv_zfil :
#ifdef DO_MMX
        UseMMX ? csScan_8_mmx_draw_scanline_tex_zfil :
#endif
        csScan_8_draw_scanline_tex_zfil;
      ScanProc [SCANPROC_TEX_ZUSE] = PrivTex ?
        csScan_8_draw_scanline_tex_priv_zuse :
        csScan_8_draw_scanline_tex_zuse;

      ScanProc [SCANPROC_MAP_ZFIL] =
#ifdef DO_MMX
        UseMMX ? csScan_8_mmx_draw_scanline_map_zfil :
#endif
        csScan_8_draw_scanline_map_zfil;
      ScanProc [SCANPROC_MAP_ZUSE] = csScan_8_draw_scanline_map_zuse;

      ScanProc [SCANPROC_MAP_FILT_ZFIL] = csScan_8_draw_scanline_map_filt_zfil;
//    ScanProc [SCANPROC_MAP_FILT_ZUSE] = csScan_8_draw_scanline_map_filt_zuse;

      ScanProc [SCANPROC_TEX_KEY_ZFIL] = PrivTex ?
        csScan_8_draw_scanline_tex_priv_key_zfil :
        csScan_8_draw_scanline_tex_key_zfil;
//    ScanProc [SCANPROC_TEX_KEY_ZUSE] = PrivTex ?
//      csScan_8_draw_scanline_tex_priv_key_zuse :
//      csScan_8_draw_scanline_tex_key_zuse;
      ScanProc [SCANPROC_MAP_KEY_ZFIL] = csScan_8_draw_scanline_map_key_zfil;
//    ScanProc [SCANPROC_MAP_KEY_ZUSE] = csScan_8_draw_scanline_map_key_zuse;

//    ScanProc [SCANPROC_FOG] = csScan_8_draw_scanline_fog;
//    ScanProc [SCANPROC_FOG_VIEW] = csScan_8_draw_scanline_fog_view;
//    ScanProc [SCANPROC_FOG_PLANE] = csScan_8_draw_scanline_fog_plane;

//    ScanProcPI [SCANPROCPI_FLAT_ZFIL] = csScan_8_draw_pi_scanline_flat_zfil;
//    ScanProcPI [SCANPROCPI_FLAT_ZUSE] = csScan_8_draw_pi_scanline_flat_zuse;
      ScanProcPI [SCANPROCPI_TEX_ZFIL] = csScan_8_draw_pi_scanline_tex_zfil;
      ScanProcPI [SCANPROCPI_TEX_ZUSE] =
#ifdef DO_MMX
        UseMMX ? csScan_8_mmx_draw_pi_scanline_tex_zuse :
#endif
        csScan_8_draw_pi_scanline_tex_zuse;

//    ScanProcPIG [SCANPROCPI_FLAT_GOURAUD_ZFIL] = csScan_8_draw_pi_scanline_flat_gouraud_zfil;
//    ScanProcPIG [SCANPROCPI_FLAT_GOURAUD_ZUSE] = csScan_8_draw_pi_scanline_flat_gouraud_zuse;
//    ScanProcPIG [SCANPROCPI_TEX_GOURAUD_ZFIL] = csScan_8_draw_pi_scanline_tex_gouraud_zfil;
//    ScanProcPIG [SCANPROCPI_TEX_GOURAUD_ZUSE] = csScan_8_draw_pi_scanline_tex_gouraud_zuse;

      if (do_transp)
        ScanProc_Alpha = ScanProc_8_Alpha;

      ScanProcPIFX[SCANPROCPIFX_ZUSE]        = csScan_8_draw_pifx_scanline_dummy;
      ScanProcPIFX[SCANPROCPIFX_ZFIL]        = csScan_8_draw_pifx_scanline_dummy;
      ScanProcPIFX[SCANPROCPIFX_TRANSP_ZUSE] = csScan_8_draw_pifx_scanline_dummy;
      ScanProcPIFX[SCANPROCPIFX_TRANSP_ZFIL] = csScan_8_draw_pifx_scanline_dummy;
      break;

    case 2:
      ScanProc [SCANPROC_FLAT_ZFIL] = csScan_16_draw_scanline_flat_zfil;
      ScanProc [SCANPROC_FLAT_ZUSE] = csScan_16_draw_scanline_flat_zuse;

      ScanProc [SCANPROC_TEX_ZFIL] = PrivTex ?
        csScan_16_draw_scanline_tex_priv_zfil :
#ifdef DO_MMX
        UseMMX ? csScan_16_mmx_draw_scanline_tex_zfil :
#endif
        csScan_16_draw_scanline_tex_zfil;
      ScanProc [SCANPROC_TEX_ZUSE] = PrivTex ?
        csScan_16_draw_scanline_tex_priv_zuse :
        csScan_16_draw_scanline_tex_zuse;

      ScanProc [SCANPROC_MAP_ZFIL] =
#ifdef DO_MMX
        UseMMX ? csScan_16_mmx_draw_scanline_map_zfil :
#endif
        csScan_16_draw_scanline_map_zfil;
      ScanProc [SCANPROC_MAP_ZUSE] = csScan_16_draw_scanline_map_zuse;

      ScanProc [SCANPROC_MAP_FILT_ZFIL] = do_bilin_filt ?
        csScan_16_draw_scanline_map_filt2_zfil :
        csScan_16_draw_scanline_map_filt_zfil;
//    ScanProc [SCANPROC_MAP_FILT_ZUSE] = do_bilin_filt ?
//      csScan_16_draw_scanline_map_filt2_zuse :
//      csScan_16_draw_scanline_map_filt_zuse;

//    ScanProc [SCANPROC_TEX_KEY_ZFIL] = PrivTex ?
//      csScan_16_draw_scanline_tex_priv_key_zfil :
//      csScan_16_draw_scanline_tex_key_zfil;
//    ScanProc [SCANPROC_TEX_KEY_ZUSE] = PrivTex ?
//      csScan_16_draw_scanline_tex_priv_key_zuse :
//      csScan_16_draw_scanline_tex_key_zuse;
      ScanProc [SCANPROC_MAP_KEY_ZFIL] = csScan_16_draw_scanline_map_key_zfil;
//    ScanProc [SCANPROC_MAP_KEY_ZUSE] = csScan_16_draw_scanline_map_key_zuse;

      ScanProc [SCANPROC_FOG] = (pfmt.GreenBits == 5) ?
        csScan_16_draw_scanline_fog_555 :
        csScan_16_draw_scanline_fog_565;
      ScanProc [SCANPROC_FOG_VIEW] = (pfmt.GreenBits == 5) ?
        csScan_16_draw_scanline_fog_view_555 :
        csScan_16_draw_scanline_fog_view_565;
      ScanProc [SCANPROC_FOG_PLANE] = (pfmt.GreenBits == 5) ?
        csScan_16_draw_scanline_fog_plane_555 :
        csScan_16_draw_scanline_fog_plane_565;

      ScanProcPI [SCANPROCPI_FLAT_ZFIL] = csScan_16_draw_pi_scanline_flat_zfil;
      ScanProcPI [SCANPROCPI_FLAT_ZUSE] = csScan_16_draw_pi_scanline_flat_zuse;
      ScanProcPI [SCANPROCPI_TEX_ZFIL] = csScan_16_draw_pi_scanline_tex_zfil;
      ScanProcPI [SCANPROCPI_TEX_ZUSE] =
#ifdef DO_MMX
        UseMMX ? csScan_16_mmx_draw_pi_scanline_tex_zuse :
#endif
        csScan_16_draw_pi_scanline_tex_zuse;

      ScanProcPIG [SCANPROCPI_FLAT_GOURAUD_ZFIL] = (pfmt.GreenBits == 5) ?
        csScan_16_draw_pi_scanline_flat_gouraud_zfil_565 :
        csScan_16_draw_pi_scanline_flat_gouraud_zfil_555;
      ScanProcPIG [SCANPROCPI_FLAT_GOURAUD_ZUSE] = (pfmt.GreenBits == 5) ?
        csScan_16_draw_pi_scanline_flat_gouraud_zuse_555 :
        csScan_16_draw_pi_scanline_flat_gouraud_zuse_565;
      ScanProcPIG [SCANPROCPI_TEX_GOURAUD_ZFIL] = (pfmt.GreenBits == 5) ?
        csScan_16_draw_pi_scanline_tex_gouraud_zfil_555 :
        csScan_16_draw_pi_scanline_tex_gouraud_zfil_565;
      ScanProcPIG [SCANPROCPI_TEX_GOURAUD_ZUSE] = (pfmt.GreenBits == 5) ?
        csScan_16_draw_pi_scanline_tex_gouraud_zuse_555 :
        csScan_16_draw_pi_scanline_tex_gouraud_zuse_565;

      ScanProcPIFX[SCANPROCPIFX_ZUSE] = (pfmt.GreenBits == 5) ?
          csScan_16_draw_pifx_scanline_zuse_555 :
          csScan_16_draw_pifx_scanline_zuse_565;
      ScanProcPIFX[SCANPROCPIFX_ZFIL] = (pfmt.GreenBits == 5) ?
          csScan_16_draw_pifx_scanline_zfil_555 :
          csScan_16_draw_pifx_scanline_zfil_565;
      ScanProcPIFX[SCANPROCPIFX_TRANSP_ZUSE] = (pfmt.GreenBits == 5) ?
          csScan_16_draw_pifx_scanline_transp_zuse_555 :
          csScan_16_draw_pifx_scanline_transp_zuse_565;
      ScanProcPIFX[SCANPROCPIFX_TRANSP_ZFIL] = (pfmt.GreenBits == 5) ?
          csScan_16_draw_pifx_scanline_transp_zfil_555 :
          csScan_16_draw_pifx_scanline_transp_zfil_565;

      if (do_transp)
        ScanProc_Alpha = ScanProc_16_Alpha;
      break;

    case 4:
      ScanProc [SCANPROC_FLAT_ZFIL] = csScan_32_draw_scanline_flat_zfil;
      ScanProc [SCANPROC_FLAT_ZUSE] = csScan_32_draw_scanline_flat_zuse;

      ScanProc [SCANPROC_TEX_ZFIL] = csScan_32_draw_scanline_tex_zfil;
      ScanProc [SCANPROC_TEX_ZUSE] = csScan_32_draw_scanline_tex_zuse;

      ScanProc [SCANPROC_MAP_ZFIL] = csScan_32_draw_scanline_map_zfil;
      ScanProc [SCANPROC_MAP_ZUSE] = csScan_32_draw_scanline_map_zuse;

//    ScanProc [SCANPROC_MAP_FILT_ZFIL] = do_bilin_filt ?
//      csScan_32_draw_scanline_map_filt2_zfil :
//      csScan_32_draw_scanline_map_filt_zfil;
//    ScanProc [SCANPROC_MAP_FILT_ZUSE] = do_bilin_filt ?
//      csScan_32_draw_scanline_map_filt2_zuse :
//      csScan_32_draw_scanline_map_filt_zuse;

//    ScanProc [SCANPROC_TEX_KEY_ZFIL] = PrivTex ?
//      csScan_32_draw_scanline_tex_priv_key_zfil :
//      csScan_32_draw_scanline_tex_key_zfil;
//    ScanProc [SCANPROC_TEX_KEY_ZUSE] = PrivTex ?
//      csScan_32_draw_scanline_tex_priv_key_zuse :
//      csScan_32_draw_scanline_tex_key_zuse;
//    ScanProc [SCANPROC_MAP_KEY_ZFIL] = csScan_32_draw_scanline_map_key_zfil;
//    ScanProc [SCANPROC_MAP_KEY_ZUSE] = csScan_32_draw_scanline_map_key_zuse;

      ScanProc [SCANPROC_FOG] = (pfmt.RedShift == 0) ?
        csScan_32_draw_scanline_fog_BGR :
        csScan_32_draw_scanline_fog_RGB;
      ScanProc [SCANPROC_FOG_VIEW] = (pfmt.RedShift == 0) ?
        csScan_32_draw_scanline_fog_view_BGR :
        csScan_32_draw_scanline_fog_view_RGB;
      ScanProc [SCANPROC_FOG_PLANE] = (pfmt.RedShift == 0) ?
        csScan_32_draw_scanline_fog_plane_BGR :
        csScan_32_draw_scanline_fog_plane_RGB;

      ScanProcPI [SCANPROCPI_FLAT_ZFIL] = csScan_32_draw_pi_scanline_flat_zfil;
      ScanProcPI [SCANPROCPI_FLAT_ZUSE] = csScan_32_draw_pi_scanline_flat_zuse;
      ScanProcPI [SCANPROCPI_TEX_ZFIL] = csScan_32_draw_pi_scanline_tex_zfil;
      ScanProcPI [SCANPROCPI_TEX_ZUSE] = csScan_32_draw_pi_scanline_tex_zuse;

      ScanProcPIG [SCANPROCPI_FLAT_GOURAUD_ZFIL] = csScan_32_draw_pi_scanline_flat_gouraud_zfil;
      ScanProcPIG [SCANPROCPI_FLAT_GOURAUD_ZUSE] = csScan_32_draw_pi_scanline_flat_gouraud_zuse;
      ScanProcPIG [SCANPROCPI_TEX_GOURAUD_ZFIL] = csScan_32_draw_pi_scanline_tex_gouraud_zfil;
      ScanProcPIG [SCANPROCPI_TEX_GOURAUD_ZUSE] = csScan_32_draw_pi_scanline_tex_gouraud_zuse;

      ScanProcPIFX[SCANPROCPIFX_ZUSE]        = csScan_32_draw_pifx_scanline_zuse;
      ScanProcPIFX[SCANPROCPIFX_ZFIL]        = csScan_32_draw_pifx_scanline_zfil;
      ScanProcPIFX[SCANPROCPIFX_TRANSP_ZUSE] = csScan_32_draw_pifx_scanline_transp_zuse;
      ScanProcPIFX[SCANPROCPIFX_TRANSP_ZFIL] = csScan_32_draw_pifx_scanline_transp_zfil;

      if (do_transp)
        ScanProc_Alpha = ScanProc_32_Alpha;
      break;
  } /* endswitch */

  int MaxColorcomponent = 64; // (pfmt.PixelBytes == 4) ? 256 : 32;

  // For DrawPolygonFX we will need to do some blending of textures with current
  // screen content.
  int i;
  for (i = 0; i < 8; i++)
    m_BlendingTable [i] = new UByte [MaxColorcomponent * MaxColorcomponent];

  for (int src = 0; src<MaxColorcomponent; src++)
  {
    for (int dest = 0; dest<MaxColorcomponent; dest++)
    {
      // The index in the table is a combintation of src pixel and dest pixel
      int index = dest * MaxColorcomponent + src;

      // Calculate all the available blendingmodes supported.
      m_BlendingTable [BLENDTABLE_COPY     ][index] =  src;
      m_BlendingTable [BLENDTABLE_ADD      ][index] =  dest + src;
      m_BlendingTable [BLENDTABLE_MULTIPLY ][index] = (dest * src)/MaxColorcomponent;
      m_BlendingTable [BLENDTABLE_MULTIPLY2][index] = (dest * src * 2)/MaxColorcomponent;
      m_BlendingTable [BLENDTABLE_ALPHA25  ][index] =  dest/4 + (src*3)/4;
      m_BlendingTable [BLENDTABLE_ALPHA50  ][index] =  dest/2 + src/2;
      m_BlendingTable [BLENDTABLE_ALPHA75  ][index] = (dest*3)/4 + src/4;
      m_BlendingTable [BLENDTABLE_ALPHA100 ][index] =  dest;

      for (i = 0; i < 8; i++)
        if (m_BlendingTable[i][index] >= MaxColorcomponent)
          m_BlendingTable[i][index] = MaxColorcomponent-1;
    }
  }
}

csDrawScanline* csGraphics3DSoftware::ScanProc_8_Alpha
  (csGraphics3DSoftware *This, int alpha)
{
  TextureTablesAlpha *lt_alpha = This->txtmgr->lt_alpha;

  if (alpha < 13)
    return NULL;
  if (alpha < 37)
  {
    Scan.AlphaMap = lt_alpha->alpha_map25;
    return csScan_8_draw_scanline_map_alpha1;
  }
  if (alpha >= 37 && alpha < 63)
  {
    Scan.AlphaMap = lt_alpha->alpha_map50;
    return csScan_8_draw_scanline_map_alpha1;
  }
  if (alpha >= 63 && alpha < 87)
  {
    Scan.AlphaMap = lt_alpha->alpha_map25;
    return csScan_8_draw_scanline_map_alpha2;
  }
  // completely opaque
  return csScan_8_draw_scanline_map_zfil;
}

csDrawScanline* csGraphics3DSoftware::ScanProc_16_Alpha
  (csGraphics3DSoftware *This, int alpha)
{
  Scan.AlphaMask = This->txtmgr->alpha_mask;
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

STDMETHODIMP csGraphics3DSoftware::Initialize ()
{
  tables.Initialize ();

  m_piG2D->Initialize ();
  txtmgr->InitSystem ();

  z_buffer = NULL;
  z_buf_mode = ZBuf_None;

  width = height = -1;

  fog_buffers = NULL;
  line_table = NULL;
  return S_OK;
}

STDMETHODIMP csGraphics3DSoftware::Open (char *Title)
{
  DrawMode = 0;

  IGraphicsInfo* piGI = NULL;

  VERIFY_SUCCESS (m_piG2D->QueryInterface((IID&)IID_IGraphicsInfo, (void**)&piGI));

  if (FAILED (m_piG2D->Open (Title)))
  {
      SysPrintf (MSG_FATAL_ERROR, "Error opening Graphics2D context.");
      FINAL_RELEASE (piGI);
      // set "not opened" flag
      width = height = -1;

      return E_UNEXPECTED;
  }

  int nWidth, nHeight;
  bool bFullScreen;

  piGI->GetWidth (nWidth);
  piGI->GetHeight (nHeight);
  piGI->GetIsFullScreen (bFullScreen);
  piGI->GetPixelFormat (&pfmt);

  width = nWidth;
  height = nHeight;
  width2 = nWidth/2;
  height2 = nHeight/2;
  SetDimensions (width, height);

  SysPrintf(MSG_INITIALIZATION, "Using %s mode at resolution %dx%d.\n",
            bFullScreen ? "full screen" : "windowed", nWidth, nHeight);

  if (pfmt.PixelBytes == 4)
  {
    SysPrintf (MSG_INITIALIZATION, "Using truecolor mode with %d bytes per pixel and %d:%d:%d RGB mode.\n",
          pfmt.PixelBytes, pfmt.RedBits, pfmt.GreenBits, pfmt.BlueBits);

    CHK (tcache = new TextureCache32 (&pfmt));
    pixel_shift = 2;
  }
  else if (pfmt.PixelBytes == 2)
  {
    SysPrintf (MSG_INITIALIZATION, "Using truecolor mode with %d bytes per pixel and %d:%d:%d RGB mode.\n",
          pfmt.PixelBytes, pfmt.RedBits, pfmt.GreenBits, pfmt.BlueBits);

    CHK (tcache = new TextureCache16 (&pfmt));
    pixel_shift = 1;
  }
  else
  {
    SysPrintf (MSG_INITIALIZATION, "Using palette mode with 1 byte per pixel (256 colors).\n");
    CHK (tcache = new TextureCache (&pfmt));
    pixel_shift = 0;
  }
  tcache->set_cache_size (-1);

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

  FINAL_RELEASE (piGI);

  ScanSetup ();
  return S_OK;
}

STDMETHODIMP csGraphics3DSoftware::Close()
{
  CHK (delete tcache); tcache = NULL;

  if ((width == height) && (width == -1))
    return S_OK;
  HRESULT rc = m_piG2D->Close();
  width = height = -1;
  return rc;
}

STDMETHODIMP csGraphics3DSoftware::SetDimensions (int width, int height)
{
  csGraphics3DSoftware::width = width;
  csGraphics3DSoftware::height = height;
  csGraphics3DSoftware::width2 = width/2;
  csGraphics3DSoftware::height2 = height/2;

  CHK (delete [] z_buffer);
  CHK (z_buffer = new unsigned long [width*height]);
  z_buf_size = sizeof (unsigned long)*width*height;

  CHK (delete [] line_table);
  CHK (line_table = new UByte* [height+1]);

  return S_OK;
}

STDMETHODIMP csGraphics3DSoftware::BeginDraw (int DrawFlags)
{
  //ASSERT( m_piG2D );

  // if 2D graphics is not locked, lock it
  if ((DrawFlags & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
   && (!(DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))))
  {
    if (FAILED (m_piG2D->BeginDraw()))
      return E_UNEXPECTED;
  }

  if (DrawFlags & CSDRAW_CLEARZBUFFER)
    memset (z_buffer, 0, z_buf_size);

  if (DrawFlags & CSDRAW_CLEARSCREEN)
    m_piG2D->Clear (0);

  if (DrawFlags & CSDRAW_3DGRAPHICS)
  {
    // Initialize the line table.
    int i;
    for (i = 0 ; i < height ; i++)
      m_piG2D->GetPixelAt (0, i, &line_table[i]);
    dbg_current_polygon = 0;
  }

  DrawMode = DrawFlags;

  return S_OK;
}

STDMETHODIMP csGraphics3DSoftware::FinishDraw ()
{
  //ASSERT( m_piG2D );

  if (DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
  {
    m_piG2D->FinishDraw ();
  }
  DrawMode = 0;
  return S_OK;
}

STDMETHODIMP csGraphics3DSoftware::Print (csRect *area)
{
  m_piG2D->Print (area);
  if (do_interlaced != -1)
    do_interlaced ^= 1;
  return S_OK;
}

STDMETHODIMP csGraphics3DSoftware::SetZBufMode (ZBufMode mode)
{
  z_buf_mode = mode;
  return S_OK;
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

HRESULT csGraphics3DSoftware::DrawPolygonFlat (G3DPolygonDPF& poly)
{
  int i;
  int max_i, min_i;
  float max_y, min_y;
  unsigned char *d;
  unsigned long *z_buf;
  float inv_aspect = poly.inv_aspect;

  if (poly.num < 3) return S_FALSE;

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
  if (num_vertices < 3) return S_FALSE;

  // For debugging: is we reach the maximum number of polygons to draw we simply stop.
  dbg_current_polygon++;
  if (dbg_current_polygon == dbg_max_polygons_to_draw-1) return E_FAIL;
  if (dbg_current_polygon >= dbg_max_polygons_to_draw-1) return S_OK;

  IPolygonTexture *tex = NULL;
  ILightMap *lm = NULL;
  if (do_lighting)
  {
    tex = poly.poly_texture[0];
    tex->GetLightMap (&lm);
  }
  csTextureMMSoftware* txt_mm = (csTextureMMSoftware*)GetcsTextureMMFromITextureHandle (poly.txt_handle);
  int mean_color_idx = txt_mm->get_mean_color_idx ();
  if (lm)
  {
    // Lighted polygon
    int lr, lg, lb;
    lm->GetMeanLighting (lr, lg, lb);
    FINAL_RELEASE (lm);
    FINAL_RELEASE (tex);
    if (pfmt.PixelBytes >= 2)
    {
      // Make lighting a little bit brighter because average
      // lighting is really dark otherwise.
      lr = lr<<2; if (lr > 255) lr = 255;
      lg = lg<<2; if (lg > 255) lg = 255;
      lb = lb<<2; if (lb > 255) lb = 255;

      int r, g, b;
      r = (mean_color_idx&pfmt.RedMask)>>pfmt.RedShift;
      g = (mean_color_idx&pfmt.GreenMask)>>pfmt.GreenShift;
      b = (mean_color_idx&pfmt.BlueMask)>>pfmt.BlueShift;
      r = (r*lr)>>8;
      g = (g*lg)>>8;
      b = (b*lb)>>8;
      mean_color_idx = (r<<pfmt.RedShift) | (g<<pfmt.GreenShift) | (b<<pfmt.BlueShift);
    }
    else
    {
      // Make lighting a little bit brighter because average
      // lighting is really dark otherwise.
      lr = lr<<1; if (lr > 255) lr = 255;
      lg = lg<<1; if (lg > 255) lg = 255;
      lb = lb<<1; if (lb > 255) lb = 255;

      PalIdxLookup* lt_light = txtmgr->lt_light;
      unsigned char* true_to_pal = txtmgr->lt_pal->true_to_pal;

      if (txtmgr->txtMode == TXT_GLOBAL)
      {
        PalIdxLookup* pil = lt_light+mean_color_idx;
        mean_color_idx = true_to_pal[pil->red[lr] | pil->green[lg] | pil->blue[lb]];
      }
      else
      {
        unsigned char * rgb, * rgb_values = txt_mm->get_colormap_private ();
        rgb = rgb_values + (mean_color_idx << 2);
        mean_color_idx = true_to_pal[lt_light[*rgb].red[lr] |
                   lt_light[*(rgb+1)].green[lg] |
                   lt_light[*(rgb+2)].blue[lb]];
      }
    }
  }

  Scan.FlatColor = mean_color_idx;
  Scan.M = M;

  // Select the right scanline drawing function.
  if (do_transp && (Scan.Texture->get_transparent ()) || poly.alpha)
    return S_OK;
  int scan_index = SCANPROC_FLAT_ZFIL;
  if (z_buf_mode == ZBuf_Use)
    scan_index++;
  csDrawScanline* dscan = ScanProc [scan_index];
  if (!dscan)
    goto finish;                        // Nothing to do.

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
	  goto finish;
        scanR1 = scanR2;
	if (++scanR2 >= poly.num)
	  scanR2 = 0;

        leave = false;
        fyR = QRound (poly.vertices [scanR2].sy);
        if (sy <= fyR)
          continue;

        float dyR = (poly.vertices [scanR1].sy - poly.vertices [scanR2].sy);
        if (dyR > 0)
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

finish:
  return S_OK;
}

STDMETHODIMP csGraphics3DSoftware::DrawPolygon (G3DPolygonDP& poly)
{
  if (!do_textured)
    return DrawPolygonFlat (poly);

  int i;
  float P1, P2, P3, P4;
  float Q1, Q2, Q3, Q4;
  int max_i, min_i;
  float max_y, min_y;
  float min_z;
  unsigned char *d;
  unsigned long *z_buf;
  float inv_aspect = poly.inv_aspect;

  if (poly.num < 3)
    return S_FALSE;

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
  min_i = max_i = 0;
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
    if (inv_z > min_z) min_z = inv_z;
    // theoretically we should do sqrt(dx^2+dy^2) here, but
    // we can approximate it by just abs(dx)+abs(dy)
    if ((fabs (poly.vertices [i].sx - poly.vertices [i - 1].sx)
       + fabs (poly.vertices [i].sy - poly.vertices [i - 1].sy)) > VERTEX_NEAR_THRESHOLD)
      num_vertices++;
  }

  // if this is a 'degenerate' polygon, skip it.
  if (num_vertices < 3) return S_FALSE;

  // For debugging: is we reach the maximum number of polygons to draw we simply stop.
  dbg_current_polygon++;
  if (dbg_current_polygon == dbg_max_polygons_to_draw-1) return E_FAIL;
  if (dbg_current_polygon >= dbg_max_polygons_to_draw-1) return S_OK;

  // Correct 1/z -> z.
  min_z = 1/min_z;

  // Mipmapping.
  int mipmap;
  if (poly.uses_mipmaps == false ||  rstate_mipmap == 1)
    mipmap = 0;
  else if (rstate_mipmap == 0)
  {
    if (min_z < zdist_mipmap1) mipmap =  0;
    else if (min_z < zdist_mipmap2) mipmap = 1;
    else if (min_z < zdist_mipmap3) mipmap = 2;
    else mipmap = 3;
  }
  else
    mipmap = rstate_mipmap - 1;
  IPolygonTexture *tex = poly.poly_texture[mipmap];
  csTextureMMSoftware *txt_mm = (csTextureMMSoftware*)GetcsTextureMMFromITextureHandle (poly.txt_handle);
  csTexture *txt_unl = txt_mm->get_texture (mipmap);

  // Initialize our static drawing information and cache
  // the texture in the texture cache (if this is not already the case).
  // If we are using the sub-texture optimization then we just allocate
  // the needed memory in the cache but don't do any calculations yet.
  int subtex_size;
  tex->GetSubtexSize (subtex_size);
  if (do_lighting)
  {
    if (subtex_size) CacheInitTexture (tex);
    else CacheTexture (tex);
  }

  csScan_InitDraw (this, tex, txt_mm, txt_unl);

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

  P1 *= Scan.tw; P2 *= Scan.tw; P3 *= Scan.tw; P4 *= Scan.tw;
  Q1 *= Scan.th; Q2 *= Scan.th; Q3 *= Scan.th; Q4 *= Scan.th;
  P4 -= Scan.fdu; Q4 -= Scan.fdv;

  // Precompute everything so that we can calculate (u,v) (texture space
  // coordinates) for every (sx,sy) (screen space coordinates). We make
  // use of the fact that 1/z, u/z and v/z are linear in screen space.
  float J1, J2, J3, K1, K2, K3;
  if (ABS (Dc) < SMALL_D)
  {
    // The Dc component of the plane of the polygon is too small.
    J1 = J2 = J3 = 0;
    K1 = K2 = K3 = 0;
  } else
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
  Scan.dM = M*Scan.InterpolStep;
  Scan.dJ1 = J1*Scan.InterpolStep;
  Scan.dK1 = K1*Scan.InterpolStep;

  // Select the right scanline drawing function.
  bool tex_transp = Scan.Texture->get_transparent ();
  int  scan_index = -2;
  csDrawScanline* dscan = NULL;

  if (Scan.tmap2)
  {
    if (do_transp && tex_transp)
      scan_index = SCANPROC_MAP_KEY_ZFIL;
    else if (ScanProc_Alpha && poly.alpha)
      dscan = ScanProc_Alpha (this, poly.alpha);
    else if ((do_texel_filt || do_bilin_filt) && mipmap == 0)
      scan_index = SCANPROC_MAP_FILT_ZFIL;
    else
      scan_index = SCANPROC_MAP_ZFIL;
  }
  else
  {
    int txtMode = txtmgr->txtMode;
    Scan.PaletteTable = txtmgr->lt_pal->pal_to_true;
    if (txtMode == TXT_PRIVATE)
      Scan.PrivToGlobal = Scan.Texture->get_private_to_global ();
    if (do_transp && tex_transp)
      scan_index = SCANPROC_TEX_KEY_ZFIL;
    else
      scan_index = SCANPROC_TEX_ZFIL;
  } /* endif */
  if (z_buf_mode == ZBuf_Use)
    scan_index++;
  if (!dscan)
    if ((scan_index < 0) || !(dscan = ScanProc [scan_index]))
      goto finish;              // nothing to do

  // If sub-texture optimization is enabled we will convert the 2D screen polygon
  // to texture space and then triangulate this texture polygon to cache all
  // needed sub-textures.
  // Note that we will not do this checking if there are no dirty sub-textures.
  // This is an optimization.
  int nDirty;
  tex->GetNumberDirtySubTex (nDirty);

  if (do_lighting && subtex_size && nDirty)
  {
    // To test if the idea is feasible I'll first implement a more naive
    // algorithm here. I will search the bounding box in texture space
    // that is visible and cache all sub-textures inside that bounding box.

    // @@@ When everything works this algorithm needs to be rewritten so
    // that only the visible shape is used. One way to do this would be
    // to triangulate and do this thing a triangle at a time.
    float min_u = 0, max_u = 0, min_v = 0, max_v = 0;
    for (int vertex = 0; vertex < poly.num; vertex++)
    {
      float cx = poly.vertices [vertex].sx - float (width2);
      // apply sub-pixel correction
      int nextvert = (vertex == 0) ? poly.num - 1 : vertex - 1;
      float y1 = poly.vertices [vertex].sy;
      float y2 = poly.vertices [nextvert].sy;
      int iy = QRound (y1);
      if (iy != QRound (y2))
      {
        float dx = (poly.vertices [nextvert].sx - poly.vertices [vertex].sx) / (y1 - y2);
        cx += dx * (poly.vertices [vertex].sy - (float (iy) - 0.5));
      } /* endif */
      cx = float (QRound (cx));

      float cy = (float (iy) - 0.5) - float (height2);
      float inv_z = M * cx + N * cy + O;
      float u_div_z = J1 * cx + J2 * cy + J3;
      float v_div_z = K1 * cx + K2 * cy + K3;
      float z = 1. / inv_z;
      float u = u_div_z * z;
      float v = v_div_z * z;

      if (vertex == 0)
        min_u = max_u = u, min_v = max_v = v;
      else
      {
        if (u < min_u) min_u = u; else if (u > max_u) max_u = u;
        if (v < min_v) min_v = v; else if (v > max_v) max_v = v;
      }
    } /* endfor */
    if (min_u < 0) min_u = 0; else if (max_u > Scan.tw2) max_u = Scan.tw2;
    if (min_v < 0) min_v = 0; else if (max_v > Scan.th2) max_v = Scan.th2;

    if ((min_u < Scan.tw2)
     && (min_v < Scan.th2)
     && (max_u > 0)
     && (max_v > 0))
      CacheRectTexture (tex, QInt (min_u), QInt (min_v), QInt (max_u), QInt (max_v));
  }

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
          goto finish;
        scanR1 = scanR2;
	if (++scanR2 >= poly.num)
	  scanR2 = 0;

        leave = false;
        fyR = QRound (poly.vertices [scanR2].sy);
        if (sy <= fyR)
          continue;

        float dyR = (poly.vertices [scanR1].sy - poly.vertices [scanR2].sy);
        if (dyR > 0)
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

        // Sub-pixel U & V correction
        float deltaX = (float)xL - sxL;

        //m_piG2D->GetPixelAt(xL, screenY, &d);
        d = line_table [screenY] + (xL << pixel_shift);
        z_buf = z_buffer + width * screenY + xL;

        // Select the right filter depending if we are drawing an odd or even line.
        // This is only used by draw_scanline_map_filt_zfil currently and is still
        // experimental.
        extern int filter_bf;
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

finish:
  FINAL_RELEASE( tex );
  return S_OK;
}

#if 0 // ----------------------------------------------------------------------
// Experimental system to try to decrease the bluriness when moving
// quickly. It's not very beautiful (there are artifacts) so I've disabled
// it for the moment
// This code should be called just after the if containing dscan().
      else if (ilace_fastmove && yy < yy1)
      {
        // If there was a fast movement and we are using interlacing
        // then copy the previous line to this one to minimize blurring.
        // (only do this if we are not at the first line).
        xxL = QRound (sxL+width2);
        if (xxL < 0) xxL = 0;
        int xxR = QRound (sxR+width2);
        if (xxR >= width) xxR = width-1;

        //int xx = QRound (sxR-sxL);
        int xx = xxR-xxL;
        if (xx > 0)
        {
          d = Graph2D->GetPixelAt (xxL, height-yy);
          unsigned char* d2 = Graph2D->GetPixelAt (xxL, height-yy-1);
          unsigned char* last_d = d + xx;
          do
          {
            *d++ = *d2++;
          } while (d <= last_d);
        }
      }
#endif // ---------------------------------------------------------------------

STDMETHODIMP csGraphics3DSoftware::DrawPolygonDebug (G3DPolygonDP& poly)
{
  (void)poly;
  return S_OK;
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

STDMETHODIMP csGraphics3DSoftware::OpenFogObject (CS_ID id, csFog* fog)
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
  return S_OK;
}

STDMETHODIMP csGraphics3DSoftware::CloseFogObject (CS_ID id)
{
  FogBuffer* fb = find_fog_buffer (id);
  if (!fb)
  {
    SysPrintf (MSG_INTERNAL_ERROR, "ENGINE FAILURE! Try to close a non-open fog object!\n");
    return E_FAIL;
  }
  if (fb->next) fb->next->prev = fb->prev;
  if (fb->prev) fb->prev->next = fb->next;
  else fog_buffers = fb->next;
  CHK (delete fb);
  return S_OK;
}

STDMETHODIMP csGraphics3DSoftware::AddFogPolygon (CS_ID id, G3DPolygonAFP& poly, int fog_type)
{
  int i;
  int max_i, min_i;
  float max_y, min_y;
  unsigned char *d;
  unsigned long *z_buf;
  float inv_aspect = poly.inv_aspect;

  if (poly.num < 3)
    return S_FALSE;
  if (fogMode == G3DFOGMETHOD_PLANES && (fog_type == CS_FOG_FRONT || fog_type == CS_FOG_BACK))
    return S_FALSE;
  if (fogMode == G3DFOGMETHOD_ZBUFFER && fog_type == CS_FOG_PLANE)
    return S_FALSE;

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
  else if (fog_type == CS_FOG_PLANE)
  {
    // We are drawing planed fog. In this case our z is already known and fixed.
    // We put it in 'O'.
    M = 0;
    N = 0;
    O = 1./poly.fog_plane_z;
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
  if (num_vertices < 3) return S_FALSE;

  FogBuffer* fb = find_fog_buffer (id);
  if (!fb)
  {
    SysPrintf (MSG_INTERNAL_ERROR, "ENGINE FAILURE! Fog object not open!\n");
    exit (0);
  }

  Scan.FogDensity = QInt (fb->density * 100);
  if (pfmt.PalEntries == 0)
  {
    Scan.FogR = QInt (fb->red * ((1 << pfmt.RedBits) - 1)) << pfmt.RedShift;
    Scan.FogG = QInt (fb->green * ((1 << pfmt.GreenBits) - 1)) << pfmt.GreenShift;
    Scan.FogB = QInt (fb->blue * ((1 << pfmt.BlueBits) - 1)) << pfmt.BlueShift;
  }
  else
  {
    Scan.FogR = QInt (fb->red * 255);
    Scan.FogG = QInt (fb->green * 255);
    Scan.FogB = QInt (fb->blue * 255);
  }

  // Steps for interpolating horizontally accross a scanline.
  Scan.M = M;
  Scan.dM = M*Scan.InterpolStep;

  // Select the right scanline drawing function.
  csDrawScanline* dscan = NULL;
  int scan_index = fog_type == CS_FOG_FRONT ?
    SCANPROC_FOG : fog_type == CS_FOG_BACK ?
    SCANPROC_ZFIL : fog_type == CS_FOG_VIEW ?
    SCANPROC_FOG_VIEW : fog_type == CS_FOG_PLANE ?
    SCANPROC_FOG_PLANE : -1;

  if ((scan_index < 0) || !(dscan = ScanProc [scan_index]))
    goto finish;   // Nothing to do.

  //@@@ Optimization note! We should have a seperate loop for CS_FOG_VIEW
  // and CS_FOG_PLANE as they are much simpler and do not require
  // the calculations for z. This would make things more efficient.

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
          goto finish;
        scanR1 = scanR2;
	if (++scanR2 >= poly.num)
	  scanR2 = 0;

        leave = false;
        fyR = QRound (poly.vertices [scanR2].sy);
        if (sy <= fyR)
          continue;

        float dyR = (poly.vertices [scanR1].sy - poly.vertices [scanR2].sy);
        if (dyR > 0)
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
      if ((sy & 1) != do_interlaced && sxR >= 0 && sxL < width && screenY >= 0 && screenY < height)
#endif
      {
        // Compute the rounded screen coordinates of horizontal strip
        xL = QRound (sxL);
        xR = QRound (sxR);

        // Sub-pixel U & V correction
        float deltaX = (float)xL - sxL;

        //m_piG2D->GetPixelAt(xL, screenY, &d);
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

finish:
  return S_OK;
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
  int redFact;
  int greenFact;
  int blueFact;
  int twfp;
  int thfp;
  float tw;
  float th;
  unsigned char *bm;
  int shf_w;
  bool transparent;
  bool textured;
  bool do_gouraud;
  DPFXMixMode mixmode;
  UByte* BlendingTable;
  float r, g, b;
  csDrawPIScanline*        drawline;
  csDrawPIScanlineGouraud* drawline_gouraud;
  csDrawPIScanlineFX*      drawline_fx;
} pqinfo;

STDMETHODIMP csGraphics3DSoftware::StartPolygonQuick (ITextureHandle* handle,
  bool gouraud)
{
  csTextureMMSoftware* txt_mm;
  csTexture* txt_unl;
  pqinfo.textured = true;
  if (!do_lighting) gouraud = false;
  if (!do_textured) pqinfo.textured = false;
  if (!handle) pqinfo.textured = false;
  pqinfo.do_gouraud = rstate_gouraud && (gouraud || !handle);

  int itw, ith;

  if (handle)
  {
    txt_mm = (csTextureMMSoftware*)GetcsTextureMMFromITextureHandle (handle);
    txt_unl = txt_mm->get_texture (0);
    pqinfo.bm = txt_unl->get_bitmap8 ();
    itw = txt_unl->get_width ();
    ith = txt_unl->get_height ();
    pqinfo.shf_w = txt_unl->get_w_shift ();

    pqinfo.tw = (float)itw;
    pqinfo.th = (float)ith;
    pqinfo.twfp = QInt16 (pqinfo.tw);
    pqinfo.thfp = QInt16 (pqinfo.th);
  }

  Scan.AlphaMask = txtmgr->alpha_mask;
  Scan.PaletteTable = txtmgr->lt_pal->pal_to_true;

  pqinfo.redFact = (1 << pfmt.RedBits) - 1;
  pqinfo.greenFact = (1 << pfmt.GreenBits) - 1;
  pqinfo.blueFact = (1 << pfmt.BlueBits) - 1;

  // Select draw scanline routine
  int scan_index = pqinfo.textured ? SCANPROCPI_TEX_ZFIL : SCANPROCPI_FLAT_ZFIL;
  if (z_buf_mode == ZBuf_Use)
    scan_index++;
  pqinfo.drawline = ScanProcPI [scan_index];
  pqinfo.drawline_gouraud = ScanProcPIG [scan_index];
  if (!pqinfo.drawline_gouraud)
    pqinfo.do_gouraud = false;

  return S_OK;
}

STDMETHODIMP csGraphics3DSoftware::FinishPolygonQuick ()
{
  return S_OK;
}

#define EPS   0.0001

STDMETHODIMP csGraphics3DSoftware::DrawPolygonQuick (G3DPolygonDPQ& poly)
{
  int i;
  bool gouraud = pqinfo.do_gouraud;
  bool textured = pqinfo.textured;

  if (!pqinfo.drawline && !pqinfo.drawline_gouraud)
    return S_OK;

  float flat_r, flat_g, flat_b;
  if (poly.txt_handle)
    flat_r = flat_g = flat_b = 1;
  else
  {
    flat_r = poly.flat_color_r;
    flat_g = poly.flat_color_g;
    flat_b = poly.flat_color_b;
  }

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
  for (i = 0 ; i < poly.num ; i++)
  {
    uu[i] = pqinfo.tw * poly.vertices [i].u;
    vv[i] = pqinfo.th * poly.vertices [i].v;
    iz[i] = poly.vertices [i].z;
    rr[i] = pqinfo.redFact * (flat_r * poly.vertices [i].r);
    gg[i] = pqinfo.greenFact * (flat_g * poly.vertices [i].g);
    bb[i] = pqinfo.blueFact * (flat_b * poly.vertices [i].b);
    if (poly.vertices [i].sy > top_y)
    {
      top_y = poly.vertices [i].sy;
      top = i;
    }
    if (poly.vertices [i].sy < bot_y)
    {
      bot_y = poly.vertices [i].sy;
      bot = i;
    }
  }

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
    return S_OK;

  float inv_dd = 1 / dd;
  int du = 0, dv = 0;
  if (textured)
  {
    float uu0 = pqinfo.tw * poly.vertices [0].u;
    float uu1 = pqinfo.tw * poly.vertices [1].u;
    float uu2 = pqinfo.tw * poly.vertices [last].u;
    du = QInt16 (((uu0 - uu2) * (poly.vertices [1].sy - poly.vertices [last].sy)
                - (uu1 - uu2) * (poly.vertices [0].sy - poly.vertices [last].sy)) * inv_dd);
    float vv0 = pqinfo.th * poly.vertices [0].v;
    float vv1 = pqinfo.th * poly.vertices [1].v;
    float vv2 = pqinfo.th * poly.vertices [last].v;
    dv = QInt16 (((vv0 - vv2) * (poly.vertices [1].sy - poly.vertices [last].sy)
                - (vv1 - vv2) * (poly.vertices [0].sy - poly.vertices [last].sy)) * inv_dd);
  }
  float iz0 = poly.vertices [0].z;
  float iz1 = poly.vertices [1].z;
  float iz2 = poly.vertices [last].z;
  int dz = QInt24 (((iz0 - iz2) * (poly.vertices [1].sy - poly.vertices [last].sy)
                  - (iz1 - iz2) * (poly.vertices [0].sy - poly.vertices [last].sy)) * inv_dd);
  long dr = 0, dg = 0, db = 0;
  if (gouraud)
  {
    float rr0 = pqinfo.redFact * (flat_r * poly.vertices [0].r);
    float rr1 = pqinfo.redFact * (flat_r * poly.vertices [1].r);
    float rr2 = pqinfo.redFact * (flat_r * poly.vertices [last].r);
    dr = QInt16 (((rr0 - rr2) * (poly.vertices [1].sy - poly.vertices [last].sy)
                - (rr1 - rr2) * (poly.vertices [0].sy - poly.vertices [last].sy)) * inv_dd);
    float gg0 = pqinfo.greenFact * (flat_g * poly.vertices [0].g);
    float gg1 = pqinfo.greenFact * (flat_g * poly.vertices [1].g);
    float gg2 = pqinfo.greenFact * (flat_g * poly.vertices [last].g);
    dg = QInt16 (((gg0 - gg2) * (poly.vertices [1].sy - poly.vertices [last].sy)
                - (gg1 - gg2) * (poly.vertices [0].sy - poly.vertices [last].sy)) * inv_dd);
    float bb0 = pqinfo.blueFact * (flat_b * poly.vertices [0].b);
    float bb1 = pqinfo.blueFact * (flat_b * poly.vertices [1].b);
    float bb2 = pqinfo.blueFact * (flat_b * poly.vertices [last].b);
    db = QInt16 (((bb0 - bb2) * (poly.vertices [1].sy - poly.vertices [last].sy)
                - (bb1 - bb2) * (poly.vertices [0].sy - poly.vertices [last].sy)) * inv_dd);
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
          goto finish;
        scanR1 = scanR2;
	if (++scanR2 >= poly.num)
	  scanR2 = 0;

        leave = false;
        fyR = QRound (poly.vertices [scanR2].sy);
        if (sy <= fyR)
	  continue;

        float dyR = poly.vertices [scanR1].sy - poly.vertices [scanR2].sy;
        if (dyR > 0)
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
          goto finish;
        scanL1 = scanL2;
	if (--scanL2 < 0)
	  scanL2 = poly.num - 1;

        leave = false;
        fyL = QRound (poly.vertices [scanL2].sy);
	if (sy <= fyL)
	  continue;

        float dyL = poly.vertices [scanL1].sy - poly.vertices [scanL2].sy;
        if (dyL > 0)
        {
          float inv_dyL = 1/dyL;
          dxdyL = QInt16 ((poly.vertices [scanL2].sx - poly.vertices [scanL1].sx) * inv_dyL);
          if (textured)
          {
            dudyL = QInt16 ((uu[scanL2] - uu[scanL1]) * inv_dyL);
            dvdyL = QInt16 ((vv[scanL2] - vv[scanL1]) * inv_dyL);
          }
          dzdyL = QInt24 ((iz[scanL2] - iz[scanL1]) * inv_dyL);
          if (gouraud)
          {
            drdyL = QInt16 ((rr[scanL2] - rr[scanL1]) * inv_dyL);
            dgdyL = QInt16 ((gg[scanL2] - gg[scanL1]) * inv_dyL);
            dbdyL = QInt16 ((bb[scanL2] - bb[scanL1]) * inv_dyL);
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

          if (textured)
          {
            uL = QInt16 (uu [scanL1] + (uu [scanL2] - uu [scanL1]) * Factor);
            vL = QInt16 (vv [scanL1] + (vv [scanL2] - vv [scanL1]) * Factor);
          }
          zL = QInt24 (iz [scanL1] + (iz [scanL2] - iz [scanL1]) * Factor);
          if (gouraud)
          {
            rL = QInt16 (rr [scanL1] + (rr [scanL2] - rr [scanL1]) * Factor);
            gL = QInt16 (gg [scanL1] + (gg [scanL2] - gg [scanL1]) * Factor);
            bL = QInt16 (bb [scanL1] + (bb [scanL2] - bb [scanL1]) * Factor);
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
    if (!textured)
    {
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
            register unsigned long *zbuff = z_buffer + width * screenY + xl;
            unsigned char* pixel_at = line_table[screenY] + (xl << pixel_shift);

            if (gouraud)
              pqinfo.drawline_gouraud (pixel_at, xr-xl, zbuff, 0, 0,
                    0, 0, zL, dz, NULL, 0, rL, gL, bL, dr, dg, db);
            else
              pqinfo.drawline (pixel_at, xr - xl, zbuff, 0, 0,
                      0, 0, zL, dz, NULL, 0);
          } /* endif */
        }

        xL += dxdyL; xR += dxdyR; zL += dzdyL;// zR += dzdyR;

        if(gouraud)
          rL += drdyL, gL += dgdyL, bL += dbdyL;

        sy--;
        screenY++;
      }
    }
    else
    {
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
            register unsigned long *zbuff = z_buffer + width * screenY + xl;

            int l=xr-xl;
            // Check for texture overflows
            int uu = uL, vv = vL;
            int duu = du, dvv = dv;
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

            unsigned char *pixel_at = line_table [screenY] + (xl << pixel_shift);
            if (gouraud)
              pqinfo.drawline_gouraud (pixel_at, xr - xl, zbuff, uu, duu,
                    vv, dvv, zL, dz, pqinfo.bm, pqinfo.shf_w, rL, gL, bL, dr, dg, db);
            else
              pqinfo.drawline (pixel_at, xr - xl, zbuff, uu, duu,
                    vv, dvv, zL, dz, pqinfo.bm, pqinfo.shf_w);
          }
        }

        xL += dxdyL; xR += dxdyR;
        uL += dudyL; vL += dvdyL; zL += dzdyL;
        if (gouraud)
          rL += drdyL, gL += dgdyL, bL += dbdyL;

        sy--;
        screenY++;
      }
    }
  }

finish:
  return S_OK;
}

STDMETHODIMP csGraphics3DSoftware::StartPolygonFX(ITextureHandle* handle,
                                                  DPFXMixMode mode,
                                                  float alpha,
                                                  bool gouraud)
{
  pqinfo.do_gouraud = rstate_gouraud && gouraud;
  pqinfo.mixmode    = mode;

  csTextureMMSoftware* txt_mm;
  csTexture* txt_unl;
  pqinfo.textured = true;

  if (mode == FX_Copy && !pqinfo.transparent)
  {
    if (!do_lighting) gouraud = false;
    if (!do_textured) pqinfo.textured = false;
    if (!handle) pqinfo.textured = false;
    pqinfo.do_gouraud = rstate_gouraud && (gouraud || !handle);
  }

  int itw, ith;

  if (handle)
  {
    txt_mm = (csTextureMMSoftware*)GetcsTextureMMFromITextureHandle (handle);
    txt_unl = txt_mm->get_texture (0);
    pqinfo.bm = txt_unl->get_bitmap8 ();
    itw = txt_unl->get_width ();
    ith = txt_unl->get_height ();
    pqinfo.shf_w = txt_unl->get_w_shift ();

    pqinfo.tw          = (float)itw;
    pqinfo.th          = (float)ith;
    pqinfo.twfp        = QInt16 (pqinfo.tw);
    pqinfo.thfp        = QInt16 (pqinfo.th);
    pqinfo.transparent = txt_mm->get_transparent();
  }

  Scan.AlphaMask = txtmgr->alpha_mask;
  Scan.PaletteTable = txtmgr->lt_pal->pal_to_true;

  pqinfo.redFact = (1 << pfmt.RedBits) - 1;
  pqinfo.greenFact = (1 << pfmt.GreenBits) - 1;
  pqinfo.blueFact = (1 << pfmt.BlueBits) - 1;

  // Select draw scanline routine
  int scan_index = pqinfo.textured ? SCANPROCPI_TEX_ZFIL : SCANPROCPI_FLAT_ZFIL;
  if (z_buf_mode == ZBuf_Use)
    scan_index++;
  pqinfo.drawline = ScanProcPI [scan_index];
  pqinfo.drawline_gouraud = ScanProcPIG [scan_index];
  if (!pqinfo.drawline_gouraud)
    pqinfo.do_gouraud = false;

  scan_index = (z_buf_mode == ZBuf_Use) ? SCANPROCPIFX_ZUSE : SCANPROCPIFX_ZFIL;
  if (pqinfo.transparent) scan_index += 0x02;
  pqinfo.drawline_fx = ScanProcPIFX[scan_index];

  switch (mode)
  {
    case FX_Add:
      pqinfo.BlendingTable = m_BlendingTable[BLENDTABLE_ADD];
      break;
    case FX_Multiply:
      pqinfo.BlendingTable = m_BlendingTable[BLENDTABLE_MULTIPLY];
      break;
    case FX_Multiply2:
      pqinfo.BlendingTable = m_BlendingTable[BLENDTABLE_MULTIPLY2];
      break;
    case FX_Alpha:
      if (alpha<0.05)
      {
        pqinfo.BlendingTable = m_BlendingTable[BLENDTABLE_COPY];
      }
      else if (alpha<0.375)
      {
        pqinfo.BlendingTable = m_BlendingTable[BLENDTABLE_ALPHA25];
      }
      else if (alpha<0.625)
      {
        pqinfo.BlendingTable = m_BlendingTable[BLENDTABLE_ALPHA50];
      }
      else if (alpha<0.95)
      {
        pqinfo.BlendingTable = m_BlendingTable[BLENDTABLE_ALPHA75];
      }
      else
      {
        pqinfo.BlendingTable = m_BlendingTable[BLENDTABLE_ALPHA100];
      }
      break;
    case FX_Copy:
    default:
      pqinfo.BlendingTable = m_BlendingTable[BLENDTABLE_COPY];
      //pqinfo.BlendingTable = m_BlendingTable[BLENDTABLE_ADD];
      break;
  }

  return S_OK;
}

STDMETHODIMP csGraphics3DSoftware::FinishPolygonFX()
{
  return S_OK;
}

STDMETHODIMP csGraphics3DSoftware::DrawPolygonFX(G3DPolygonDPFX& poly)
{
  int i;
  bool gouraud  = pqinfo.do_gouraud;
  bool textured = pqinfo.textured;

  if (!pqinfo.drawline && !pqinfo.drawline_gouraud)
    return S_OK;

  float flat_r, flat_g, flat_b;
  if (poly.txt_handle)
    flat_r = flat_g = flat_b = 1;
  else
  {
    flat_r = poly.vertices[0].r;
    flat_g = poly.vertices[0].g;
    flat_b = poly.vertices[0].b;
  }

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
  for (i = 0 ; i < poly.num ; i++)
  {
    uu[i] = pqinfo.tw * poly.vertices [i].u;
    vv[i] = pqinfo.th * poly.vertices [i].v;
    iz[i] = poly.vertices [i].z;
    rr[i] = pqinfo.redFact * (flat_r * poly.vertices [i].r);
    gg[i] = pqinfo.greenFact * (flat_g * poly.vertices [i].g);
    bb[i] = pqinfo.blueFact * (flat_b * poly.vertices [i].b);
    if (poly.vertices [i].sy > top_y)
    {
      top_y = poly.vertices [i].sy;
      top = i;
    }
    if (poly.vertices [i].sy < bot_y)
    {
      bot_y = poly.vertices [i].sy;
      bot = i;
    }
  }

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
    return S_OK;

  float inv_dd = 1 / dd;
  int du = 0, dv = 0;
  if (textured)
  {
    float uu0 = pqinfo.tw * poly.vertices [0].u;
    float uu1 = pqinfo.tw * poly.vertices [1].u;
    float uu2 = pqinfo.tw * poly.vertices [last].u;
    du = QInt16 (((uu0 - uu2) * (poly.vertices [1].sy - poly.vertices [last].sy)
                - (uu1 - uu2) * (poly.vertices [0].sy - poly.vertices [last].sy)) * inv_dd);
    float vv0 = pqinfo.th * poly.vertices [0].v;
    float vv1 = pqinfo.th * poly.vertices [1].v;
    float vv2 = pqinfo.th * poly.vertices [last].v;
    dv = QInt16 (((vv0 - vv2) * (poly.vertices [1].sy - poly.vertices [last].sy)
                - (vv1 - vv2) * (poly.vertices [0].sy - poly.vertices [last].sy)) * inv_dd);
  }
  float iz0 = poly.vertices [0].z;
  float iz1 = poly.vertices [1].z;
  float iz2 = poly.vertices [last].z;
  int dz = QInt24 (((iz0 - iz2) * (poly.vertices [1].sy - poly.vertices [last].sy)
                  - (iz1 - iz2) * (poly.vertices [0].sy - poly.vertices [last].sy)) * inv_dd);
  long dr = 0, dg = 0, db = 0;
  if (gouraud)
  {
    float rr0 = pqinfo.redFact * (flat_r * poly.vertices [0].r);
    float rr1 = pqinfo.redFact * (flat_r * poly.vertices [1].r);
    float rr2 = pqinfo.redFact * (flat_r * poly.vertices [last].r);
    dr = QInt16 (((rr0 - rr2) * (poly.vertices [1].sy - poly.vertices [last].sy)
                - (rr1 - rr2) * (poly.vertices [0].sy - poly.vertices [last].sy)) * inv_dd);
    float gg0 = pqinfo.greenFact * (flat_g * poly.vertices [0].g);
    float gg1 = pqinfo.greenFact * (flat_g * poly.vertices [1].g);
    float gg2 = pqinfo.greenFact * (flat_g * poly.vertices [last].g);
    dg = QInt16 (((gg0 - gg2) * (poly.vertices [1].sy - poly.vertices [last].sy)
                - (gg1 - gg2) * (poly.vertices [0].sy - poly.vertices [last].sy)) * inv_dd);
    float bb0 = pqinfo.blueFact * (flat_b * poly.vertices [0].b);
    float bb1 = pqinfo.blueFact * (flat_b * poly.vertices [1].b);
    float bb2 = pqinfo.blueFact * (flat_b * poly.vertices [last].b);
    db = QInt16 (((bb0 - bb2) * (poly.vertices [1].sy - poly.vertices [last].sy)
                - (bb1 - bb2) * (poly.vertices [0].sy - poly.vertices [last].sy)) * inv_dd);
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
          goto finish;
        scanR1 = scanR2;
	if (++scanR2 >= poly.num)
	  scanR2 = 0;

        leave = false;
	fyR = QRound (poly.vertices [scanR2].sy);
	if (sy <= fyR)
	  continue;

        float dyR = poly.vertices [scanR1].sy - poly.vertices [scanR2].sy;
        if (dyR > 0)
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
          goto finish;
        scanL1 = scanL2;
	if (--scanL2 < 0)
	  scanL2 = poly.num - 1;

        leave = false;
	fyL = QRound (poly.vertices [scanL2].sy);
	if (sy <= fyL)
	  continue;

        float dyL = poly.vertices [scanL1].sy - poly.vertices [scanL2].sy;
        if (dyL > 0)
        {
          float inv_dyL = 1/dyL;
          dxdyL = QInt16 ((poly.vertices [scanL2].sx - poly.vertices [scanL1].sx) * inv_dyL);
          if (textured)
          {
            dudyL = QInt16 ((uu[scanL2] - uu[scanL1]) * inv_dyL);
            dvdyL = QInt16 ((vv[scanL2] - vv[scanL1]) * inv_dyL);
          }
          dzdyL = QInt24 ((iz[scanL2] - iz[scanL1]) * inv_dyL);
          if (gouraud)
          {
            drdyL = QInt16 ((rr[scanL2] - rr[scanL1]) * inv_dyL);
            dgdyL = QInt16 ((gg[scanL2] - gg[scanL1]) * inv_dyL);
            dbdyL = QInt16 ((bb[scanL2] - bb[scanL1]) * inv_dyL);
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

          if (textured)
          {
            uL = QInt16 (uu [scanL1] + (uu [scanL2] - uu [scanL1]) * Factor);
            vL = QInt16 (vv [scanL1] + (vv [scanL2] - vv [scanL1]) * Factor);
          }
          zL = QInt24 (iz [scanL1] + (iz [scanL2] - iz [scanL1]) * Factor);
          if (gouraud)
          {
            rL = QInt16 (rr [scanL1] + (rr [scanL2] - rr [scanL1]) * Factor);
            gL = QInt16 (gg [scanL1] + (gg [scanL2] - gg [scanL1]) * Factor);
            bL = QInt16 (bb [scanL1] + (bb [scanL2] - bb [scanL1]) * Factor);
          }
        } /* endif */
        leave = false;
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
    if (!textured)
    {
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
            register unsigned long *zbuff = z_buffer + width * screenY + xl;
            unsigned char* pixel_at = line_table[screenY] + (xl << pixel_shift);

            if (pqinfo.mixmode == FX_Copy && !pqinfo.transparent)
            {
              if (gouraud)
                pqinfo.drawline_gouraud (pixel_at, xr-xl, zbuff, 0, 0,
                      0, 0, zL, dz, NULL, 0, rL, gL, bL, dr, dg, db);
              else
                pqinfo.drawline (pixel_at, xr - xl, zbuff, 0, 0,
                        0, 0, zL, dz, NULL, 0);
            }
            else
            {
              if (gouraud)
              {
                pqinfo.drawline_fx (pixel_at, xr-xl, zbuff, 0, 0,
                      0, 0, zL, dz, NULL, 0,
                      rL, gL, bL, dr, dg, db,
                      pqinfo.BlendingTable);
              }
              else
              {
                pqinfo.drawline_fx (pixel_at, xr-xl, zbuff, 0, 0,
                      0, 0, zL, dz, NULL, 0,
                      pqinfo.redFact << 16,
                      pqinfo.greenFact << 16,
                      pqinfo.blueFact << 16,
                      0, 0, 0,
                      pqinfo.BlendingTable);
              }
            }
          } /* endif */
        }

        xL += dxdyL; xR += dxdyR; zL += dzdyL;// zR += dzdyR;

        if(gouraud)
          rL += drdyL, gL += dgdyL, bL += dbdyL;

        sy--;
        screenY++;
      }
    }
    else
    {
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
            register unsigned long *zbuff = z_buffer + width * screenY + xl;

            int l=xr-xl;
            // Check for texture overflows
            int uu = uL, vv = vL;
            int duu = du, dvv = dv;
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

            unsigned char *pixel_at = line_table [screenY] + (xl << pixel_shift);

            if (pqinfo.mixmode == FX_Copy && !pqinfo.transparent)
            {
              if (gouraud)
                pqinfo.drawline_gouraud (pixel_at, xr - xl, zbuff, uu, duu,
                      vv, dvv, zL, dz, pqinfo.bm, pqinfo.shf_w, rL, gL, bL, dr, dg, db);
              else
                pqinfo.drawline (pixel_at, xr - xl, zbuff, uu, duu,
                      vv, dvv, zL, dz, pqinfo.bm, pqinfo.shf_w);
            }
            else
            {
              if (gouraud)
              {
                pqinfo.drawline_fx (pixel_at, xr - xl, zbuff, uu, duu,
                      vv, dvv, zL, dz, pqinfo.bm, pqinfo.shf_w,
                      rL, gL, bL, dr, dg, db,
                      pqinfo.BlendingTable);
              }
              else
              {
                pqinfo.drawline_fx (pixel_at, xr - xl, zbuff, uu, duu,
                      vv, dvv, zL, dz, pqinfo.bm, pqinfo.shf_w,
                      pqinfo.redFact << 16,
                      pqinfo.greenFact << 16,
                      pqinfo.blueFact << 16,
                      0, 0, 0,
                      pqinfo.BlendingTable);
              }
            }
          }
        }

        xL += dxdyL; xR += dxdyR;
        uL += dudyL; vL += dvdyL; zL += dzdyL;
        if (gouraud)
          rL += drdyL, gL += dgdyL, bL += dbdyL;

        sy--;
        screenY++;
      }
    }
  }

finish:
  return S_OK;
};

STDMETHODIMP csGraphics3DSoftware::CacheTexture (IPolygonTexture* texture)
{
  tcache->use_texture (texture, txtmgr);
  return S_OK;
}

void csGraphics3DSoftware::CacheInitTexture (IPolygonTexture* texture)
{
  tcache->init_texture (texture, txtmgr);
}

void csGraphics3DSoftware::CacheSubTexture (IPolygonTexture* texture, int u, int v)
{
  tcache->use_sub_texture (texture, txtmgr, u, v);
}

void csGraphics3DSoftware::CacheRectTexture (IPolygonTexture* tex,
        int minu, int minv, int maxu, int maxv)
{
  int subtex_size;
  tex->GetSubtexSize (subtex_size);

  int iu, iv;
  for (iu = minu ; iu < maxu ; iu += subtex_size)
  {
    for (iv = minv ; iv < maxv ; iv += subtex_size)
        tcache->use_sub_texture (tex, txtmgr, iu, iv);
    tcache->use_sub_texture (tex, txtmgr, iu, maxv);
  }
  for (iv = minv ; iv < maxv ; iv += subtex_size)
      tcache->use_sub_texture (tex, txtmgr, maxu, iv);
  tcache->use_sub_texture (tex, txtmgr, maxu, maxv);
}

STDMETHODIMP csGraphics3DSoftware::UncacheTexture (IPolygonTexture* texture)
{
  (void)texture;
  return E_NOTIMPL;
}

STDMETHODIMP csGraphics3DSoftware::SetRenderState (G3D_RENDERSTATEOPTION op,
  long value)
{
  switch (op)
  {
    case G3DRENDERSTATE_NOTHING:
      return S_OK;
    case G3DRENDERSTATE_ZBUFFERTESTENABLE:
      if (value)
      {
         if (z_buf_mode == ZBuf_Test)
           return S_OK;
         if (z_buf_mode == ZBuf_None)
           z_buf_mode = ZBuf_Test;
         else if (z_buf_mode == ZBuf_Fill)
           z_buf_mode = ZBuf_Use;
      }
      else
      {
         if (z_buf_mode == ZBuf_Fill)
           return S_OK;
         if (z_buf_mode == ZBuf_Use)
           z_buf_mode = ZBuf_Fill;
         else if (z_buf_mode == ZBuf_Test)
           z_buf_mode = ZBuf_None;
      }
      break;
    case G3DRENDERSTATE_ZBUFFERFILLENABLE:
      if (value)
      {
        if (z_buf_mode == ZBuf_Fill)
          return S_OK;
        if (z_buf_mode == ZBuf_None)
          z_buf_mode = ZBuf_Fill;
        else if (z_buf_mode == ZBuf_Test)
          z_buf_mode = ZBuf_Use;
      }
      else
      {
        if (z_buf_mode == ZBuf_Test)
          return S_OK;
        if (z_buf_mode == ZBuf_Use)
          z_buf_mode = ZBuf_Test;
        else if (z_buf_mode == ZBuf_Fill)
          z_buf_mode = ZBuf_None;
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
      do_bilin_filt = value;
      ScanSetup ();
      break;
    case G3DRENDERSTATE_TRILINEARMAPPINGENABLE:
      do_bilin_filt = value;
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
      return E_FAIL;
#endif
    case G3DRENDERSTATE_INTERLACINGENABLE:
      if (m_piG2D->DoubleBuffer (!value) != S_OK)
        return E_FAIL;
      do_interlaced = value ? 0 : -1;
      break;
    case G3DRENDERSTATE_INTERPOLATIONSTEP:
      Scan.InterpolMode = value;
      break;
    case G3DRENDERSTATE_DEBUGENABLE:
      do_debug = value;
      break;
    case G3DRENDERSTATE_FILTERINGENABLE:
      do_texel_filt = value;
      ScanSetup ();
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
      return E_INVALIDARG;
  }

  return S_OK;
}

STDMETHODIMP csGraphics3DSoftware::GetRenderState(G3D_RENDERSTATEOPTION op, long& retval)
{
  switch (op)
  {
    case G3DRENDERSTATE_NOTHING:
      retval = 0;
      break;
    case G3DRENDERSTATE_ZBUFFERTESTENABLE:
      retval = (bool)(z_buf_mode & ZBuf_Test);
      break;
    case G3DRENDERSTATE_ZBUFFERFILLENABLE:
      retval = (bool)(z_buf_mode & ZBuf_Fill);
      break;
    case G3DRENDERSTATE_DITHERENABLE:
      retval = rstate_dither;
      break;
    case G3DRENDERSTATE_SPECULARENABLE:
      retval = rstate_specular;
      break;
    case G3DRENDERSTATE_BILINEARMAPPINGENABLE:
      retval = do_texel_filt;
      break;
    case G3DRENDERSTATE_TRILINEARMAPPINGENABLE:
      retval = do_bilin_filt;
      break;
    case G3DRENDERSTATE_TRANSPARENCYENABLE:
      retval = do_transp;
      break;
    case G3DRENDERSTATE_MIPMAPENABLE:
      retval = rstate_mipmap;
      break;
    case G3DRENDERSTATE_TEXTUREMAPPINGENABLE:
      retval = do_textured;
      break;
    case G3DRENDERSTATE_MMXENABLE:
#ifdef DO_MMX
      retval = do_mmx;
#else
      retval = 0;
#endif
      break;
    case G3DRENDERSTATE_INTERLACINGENABLE:
      retval = do_interlaced == -1 ? false : true;
      break;
    case G3DRENDERSTATE_INTERPOLATIONSTEP:
      retval = Scan.InterpolMode;
      break;
    case G3DRENDERSTATE_DEBUGENABLE:
      retval = do_debug;
      break;
    case G3DRENDERSTATE_FILTERINGENABLE:
      retval = do_texel_filt;
      break;
    case G3DRENDERSTATE_LIGHTINGENABLE:
      retval = do_lighting;
      break;
    case G3DRENDERSTATE_MAXPOLYGONSTODRAW:
      retval = dbg_max_polygons_to_draw;
      break;
    case G3DRENDERSTATE_GOURAUDENABLE:
      retval = rstate_gouraud;
      break;
    default:
      retval = 0;
      return E_INVALIDARG;
  }

  return S_OK;
}

STDMETHODIMP csGraphics3DSoftware::GetCaps(G3D_CAPS *caps)
{
  if (!caps)
    return E_INVALIDARG;

  caps->ColorModel = G3DCOLORMODEL_RGB;
  caps->CanClip = false;
  caps->SupportsArbitraryMipMapping = true;
  caps->BitDepth = 8;
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
  caps->fog = G3D_FOGMETHOD((int)G3DFOGMETHOD_ZBUFFER | (int)G3DFOGMETHOD_PLANES);

  return 1;
}

STDMETHODIMP csGraphics3DSoftware::ClearCache()
{
  if (tcache) tcache->clear ();
  return S_OK;
}

STDMETHODIMP csGraphics3DSoftware::SetCacheSize (long size)
{
  if (tcache) tcache->set_cache_size (size);
  else TextureCache::set_default_cache_size (size);
  return S_OK;
}

STDMETHODIMP csGraphics3DSoftware::DumpCache()
{
  if (tcache) tcache->dump();
  return S_OK;
}

STDMETHODIMP csGraphics3DSoftware::DrawLine (csVector3& v1, csVector3& v2, float fov, int color)
{
  if (v1.z < SMALL_Z && v2.z < SMALL_Z) return S_FALSE;

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

  m_piG2D->DrawLine (px1, py1, px2, py2, color);

  return S_OK;
}

void csGraphics3DSoftware::SysPrintf (int mode, char* szMsg, ...)
{
  char buf[1024];
  va_list arg;

  va_start (arg, szMsg);
  vsprintf (buf, szMsg, arg);
  va_end (arg);

  m_piSystem->Print (mode, buf);
}

// IHaloRasterizer Implementation //

// NOTE!!! This only works in 16-bit mode!!!
STDMETHODIMP csGraphics3DSoftware::DrawHalo(csVector3* pCenter, float fIntensity, HALOINFO haloInfo)
{
  if (pfmt.PixelBytes != 2)
    return S_FALSE;

  int izz = QInt24 (1.0f / pCenter->z);
  HRESULT hRes = S_OK;

  if (haloInfo == NULL)
    return E_INVALIDARG;

  int hdiv3 = height / 3;

  if (pCenter->x > width || pCenter->x < 0 || pCenter->y > height || pCenter->y < 0 )
    hRes=S_FALSE;
  else
  {
    unsigned long zb = z_buffer[(int)pCenter->x + (width * (int)pCenter->y)];

    // first, do a z-test to make sure the halo is visible
    if (izz < (int)zb)
      hRes = S_FALSE;
  }

  // now, draw the halo
  unsigned short* pBuffer = ((csG3DSoftwareHaloInfo*)haloInfo)->pbuf;
  unsigned char* pAlphaBuffer = ((csG3DSoftwareHaloInfo*)haloInfo)->palpha;

  int nx = QInt(pCenter->x) - (hdiv3 >> 1),
      ny = QInt(pCenter->y) - (hdiv3 >> 1);

  int x, y;
  int hh = hdiv3, hw = hdiv3;

  if (fIntensity <= 0.0f)
    return S_FALSE;

  if (nx >= width || ny >= height)
    return S_FALSE;

  if (nx < 0)
  {
    hw += nx;    // decrease the width
    nx = 0;         // clip to screen boundaries
  }

  if (ny < 0)
  {
    hh += ny;
    ny = 0;
  }

  if (nx + hw > width)
    hw -= (nx + hw) - width;

  if (ny + hh > height)
    hh -= (ny + hh) - height;

  int startx = nx - (QInt(pCenter->x) - (hdiv3 >> 1)),
      starty = ny - (QInt(pCenter->y) - (hdiv3 >> 1));

  int br1, bg1, bb1,
      br2, bg2, bb2;

  int red_shift, green_mask;
  if (pfmt.GreenBits == 5)
  {
    red_shift = 10;
    green_mask = 0x1f;
  }
  else
  {
    red_shift = 11;
    green_mask = 0x3f;
  }

  for (y=0; y < hh; y++)
  {
    unsigned short b, p;

    unsigned short* pScreen;
    unsigned short* pBufY;
    unsigned char* pAlphaBufY;

    //m_piG2D->GetPixelAt(nx, ny + y, (unsigned char**)&pScreen);
    pScreen = (unsigned short*)(line_table[ny+y] + (nx << pixel_shift));
    pBufY = &pBuffer[startx + (hdiv3 * (starty + y))];
    pAlphaBufY = &pAlphaBuffer[startx + (hdiv3 * (starty + y))];

    for (x=0; x < hw; x++)
    {
      b = pBufY[x];

      int a = QInt((float)pAlphaBufY[x] * fIntensity);
      int na = 256-a;

      p = pScreen[x];

      br1 = b >> red_shift;
      bg1 = (b >> 5) & green_mask;
      bb1 = b & 0x1f;

      br2 = p >> red_shift;
      bg2 = (p >> 5) & green_mask;
      bb2 = p & 0x1f;

      br1 = (a*br1 + br2*na) >> 8;
      bg1 = (a*bg1 + bg2*na) >> 8;
      bb1 = (a*bb1 + bb2*na) >> 8;

      pScreen[x] = (br1<<red_shift) | (bg1<<5) | bb1;
    }
  }

  return hRes;
}

STDMETHODIMP csGraphics3DSoftware::CreateHalo(float r, float g, float b, HALOINFO* pRetVal)
{
  m_piG2D->AddRef();
  csHaloDrawer halo(m_piG2D, r, g, b);

  CHK (csG3DSoftwareHaloInfo* retval = new csG3DSoftwareHaloInfo());

  retval->pbuf = halo.GetBuffer();
  retval->palpha = halo.GetAlphaBuffer();

  *pRetVal = (HALOINFO)retval;

  return S_OK;
}

STDMETHODIMP csGraphics3DSoftware::DestroyHalo(HALOINFO haloInfo)
{
  CHK (delete [] ((csG3DSoftwareHaloInfo*)haloInfo)->pbuf);
  CHK (delete [] ((csG3DSoftwareHaloInfo*)haloInfo)->palpha);

  CHK (delete (csG3DSoftwareHaloInfo*)haloInfo);
  return S_OK;
}

STDMETHODIMP csGraphics3DSoftware::TestHalo (csVector3* pCenter)
{
  int izz = QInt24 (1.0f / pCenter->z);

  if (pCenter->x > width || pCenter->x < 0 || pCenter->y > height || pCenter->y < 0  )
    return S_FALSE;

  unsigned long zb = z_buffer[(int)pCenter->x + (width * (int)pCenter->y)];

  if (izz < (int)zb)
    return S_FALSE;

  return S_OK;
}

/////////////

// csHaloDrawer implementation //

csGraphics3DSoftware::csHaloDrawer::csHaloDrawer(IGraphics2D* piG2D, float r, float g, float b)
{
  mpBuffer = NULL;
  mpAlphaBuffer = NULL;

  IGraphicsInfo* piGI;

  piG2D->QueryInterface(IID_IGraphicsInfo, (void**)&piGI);
  mpiG2D = piG2D;

  piGI->GetWidth(mWidth);
  piGI->GetHeight(mHeight);

  csPixelFormat pfmt;
  piGI->GetPixelFormat (&pfmt);

  if (pfmt.PixelBytes != 2)
  {
    red_shift = 0;
    FINAL_RELEASE (piGI);
    return;
  }

  if (pfmt.GreenBits == 5)
  {
    red_shift = 10;
    green_mask = 0x1f;
    not_green_bits = 3;
  }
  else
  {
    red_shift = 11;
    green_mask = 0x3f;
    not_green_bits = 2;
  }

  int dim = mHeight / 3;

  // point variables
  int x = 0;
  int y = dim / 2;

  CHK (mpBuffer = new unsigned short[dim*dim]);
  CHK (mpAlphaBuffer = new unsigned char[dim*dim]);
  memset(mpBuffer, 0, dim*dim*sizeof(short));
  memset(mpAlphaBuffer, 0, dim*dim*sizeof(char));

  mBufferWidth = dim;
  mDim = dim;

  mRed = r; mGreen = b; mBlue = b;
  mx = my = dim / 2;

  int i,j;

  float power = 1.0 / 2.5;
  float power_dist = pow (dim * dim / 4, power);

#define LEVEL_OF_DISTORTION   5

  for (i = 0, y = -dim / 2; i < dim; i++, y++)
  {
    for(j = 0, x = -dim / 2; j < dim; j++, x++)
    {
      float dist = pow (x * x + y * y, power);
      if (dist > power_dist)
        continue;
      int alpha = QRound (255 * cos (0.5 * M_PI * dist / power_dist) + 0.5);

      alpha += rand () % (2 * LEVEL_OF_DISTORTION + 1) - LEVEL_OF_DISTORTION;
      if (alpha < 0)
        alpha = 0;
      if (alpha > 255)
        alpha = 255;

      int zr = QRound (r * alpha);
      int zg = QRound (g * alpha);
      int zb = QRound (b * alpha);

      zr >>= 3; zg >>= not_green_bits; zb >>= 3;

      unsigned short c = (zr << red_shift) | (zg << 5) | zb;//(zr << 10) | (zg << 5) | zb;
      mpBuffer[j+i*dim]=c;
      mpAlphaBuffer[j+i*dim]=alpha;
/*    unsigned short final_color=(alpha<<24)|c;
      mpBuffer[j+i*dim]=final_color;*/
    }
  }

#if 0
  ////// Draw the outer rim //////

  drawline_outerrim(-y, y, x);

  while (true)
  {
    if (d < 0)
      d += 2 * x + 3;
    else
    {
      d += 2 * (x - y) + 5;
      y--;
      if (y <= x)
        break;

      drawline_outerrim(-x, x, y);
      drawline_outerrim(-x, x, -y);
    }
    x++;

    drawline_outerrim(-y, y, x);
    drawline_outerrim(-y, y, -x);
  }

  ////// Draw the inner core /////

  x=0;
  y=dim/3;
  d = 1 - y;

  mDim = QInt (dim/1.5);

  mRatioRed = (r - (r/3.f)) / y;
  mRatioGreen = (g - (g/3.f)) / y;
  mRatioBlue = (b - (b/3.f)) / y;

  drawline_innerrim(-y, y, x);

  while (true)
  {
    if (d < 0)
      d += 2 * x + 3;
    else
    {
      d += 2 * (x - y) + 5;
      y--;
      if (y <= x)
        break;

      drawline_innerrim(-x, x, y);
      drawline_innerrim(-x, x, -y);
    }
    x++;
    drawline_innerrim(-y, y, x);
    drawline_innerrim(-y, y, -x);
  }

  ///// Draw the vertical lines /////

  // DAN: this doesn't look right yet.
#if 0
  int y1, y2;

  // the vertical line has a constant height,
  // until the halo itself is of a constant height,
  // at which point the vertical line decreases.

  y1 = my - mWidth/10;
  y2 = my + mWidth/10;

  if (dim < mWidth / 6)
  {
    int q = mWidth/6 - dim;
    y1 += q;
    y2 -= q;
  }

  drawline_vertical(mx, y1, y2);
#endif

#endif

  FINAL_RELEASE (piGI);
}

csGraphics3DSoftware::csHaloDrawer::~csHaloDrawer()
{
  FINAL_RELEASE (mpiG2D);
}

void csGraphics3DSoftware::csHaloDrawer::drawline_vertical(int /*x*/, int y1, int y2)
{
  int i;
  unsigned short* buf;
  unsigned char* abuf;

  int r = (int)(mRed/2.5f * 256.0f) >> 3;
  int g = (int)(mGreen/2.5f * 256.0f) >> not_green_bits;
  int b = (int)(mBlue/2.5f * 256.0f) >> 3;

  int c = (r << red_shift) | (g << 5) | b;

  while (y1 < y2)
  {
    buf = &mpBuffer[(mx-1) + (mBufferWidth * y1++)];
    abuf = &mpAlphaBuffer[(mx-1) + (mBufferWidth * y1++)];

    for(i=0; i<3; i++)
    {
        buf[i] = c;
        abuf[i] = 0;
    }
  }

}

void csGraphics3DSoftware::csHaloDrawer::drawline_outerrim(int x1, int x2, int y)
{
  if (x1 == x2) return;

  int r = (int)(mRed / 3.5f * 256.0f);
  int g = (int)(mGreen / 3.5f * 256.0f);
  int b = (int)(mBlue / 3.5f * 256.0f);

  int a = QInt((r + g + b) / 3);

  // stopx makes sure we don't overrdraw when drawing the inner core.
  // maybe there's something faster than a sqrt... - DAN
  // @@@ JORRIT: had to make some changes to prevent overflows!
  float sq = (mDim/3.0)*(mDim/3.0) - ((double)y*(double)y);
  int stopx = 0;
  if (sq > 0) stopx = (int)sqrt (sq);

  r >>= 3; g >>= not_green_bits; b >>= 3;

  unsigned short* bufy;
  unsigned char*  abufy;

  x1 += mx;
  x2 += mx;
  y += my;

  bufy = &mpBuffer[y * mBufferWidth];
  abufy = &mpAlphaBuffer[y * mBufferWidth];

  if (stopx)
  {
    while (x1 <= (mx - stopx) + 2)
    {
      bufy[x1] = (r << red_shift) | (g << 5) | b;
      abufy[x1++] = a;
    }

    x1 = mx + stopx - 2;

    while (x1 <= x2)
    {
      bufy[x1] = (r << red_shift) | (g << 5) | b;
      abufy[x1++] = a;
    }
  }
  else
  {
    while (x1 <= x2)
    {
      bufy[x1] = (r << red_shift) | (g << 5) | b;
      abufy[x1++] = a;
    }

  }
}

void csGraphics3DSoftware::csHaloDrawer::drawline_innerrim(int x1, int x2, int y)
{
  float w2 = x2 - x1;
  unsigned short* bufy;
  unsigned char* abufy;

  x1 += mx;
  x2 += mx;
  y += my;

  if (y >= mHeight || y <= 0) return;
  bufy = &mpBuffer[y * mBufferWidth];
  abufy = &mpAlphaBuffer[y * mBufferWidth];

  if (w2 == 0.0f) return;
  w2 /= 2.0f;

  int halfx = QInt (x1 + w2);

  float ir, ig, ib, ia;

  float rlow = mRed / 4.5f;
  float glow = mGreen / 4.5f;
  float blow = mBlue / 4.5f;

  if (y <= my)
  {
    int iy = y - (my - (mDim / 2));

    ir = (iy * mRatioRed + rlow) * 256;
    ig = (iy * mRatioGreen + glow) * 256;
    ib = (iy * mRatioBlue + blow) * 256;
    ia = (ir + ig + ib) / 3.0f;
  }
  else
  {
    int iy = (my + (mDim/2)) - y;

    ir = (iy * mRatioRed + rlow) * 256;
    ig = (iy * mRatioGreen + glow) * 256;
    ib = (iy * mRatioBlue + blow) * 256;
    ia = (ir + ig + ib) / 3.0f;
  }

  float r = rlow * 256;
  float g = glow * 256;
  float b = blow * 256;
  float a = (r + g + b) / 3.0f;

  if (a < 0) a = 0;

  if (ir > 245) ir = 245;
  if (ig > 245) ig = 245;
  if (ib > 245) ib = 245;
  if (ia > 250) ia = 250;

  float rdelta = (ir - r) / w2;
  float gdelta = (ig - g) / w2;
  float bdelta = (ib - b) / w2;
  float adelta = (ia - a) / w2;

  int br, bg, bb;

  unsigned short p;
  int inta;

  while (x1 <= halfx)
  {
    p = bufy[x1];

    inta = QInt(a);

    br = QInt(r) >> 3;
    bg = QInt(g) >> not_green_bits;
    bb = QInt(b) >> 3;

    bufy[x1] = (br<<red_shift) | (bg<<5) | bb;
    abufy[x1++] = inta;

    r += rdelta; g+=gdelta; b+=bdelta; a+=adelta;
  }

  while(x1 <= x2)
  {
    p = bufy[x1];

    inta = QInt(a);

    br = QInt(r) >> 3;
    bg = QInt(g) >> not_green_bits;
    bb = QInt(b) >> 3;

    bufy[x1] = (br<<red_shift) | (bg<<5) | bb;
    abufy[x1++] = inta;

    r -= rdelta; g -= gdelta; b -= bdelta; a -= adelta;
  }
}

//---------------------------------------------------------------------------

// IXConfig3DSoft implementation

IMPLEMENT_COMPOSITE_UNKNOWN (csGraphics3DSoftware, XConfig3DSoft)

csOptionDescription IXConfig3DSoft::config_options[] =
{
  { 0, "ilace", "Interlacing", CSVAR_BOOL },
  { 1, "light", "Texture lighting", CSVAR_BOOL },
  { 2, "transp", "Transparent textures", CSVAR_BOOL },
  { 3, "txtmap", "Texture mapping", CSVAR_BOOL },
  { 4, "txtfilt", "Texture filtering", CSVAR_BOOL },
  { 5, "bifilt", "Bilinear filtering", CSVAR_BOOL },
  { 6, "mmx", "MMX support", CSVAR_BOOL },
  { 7, "gamma", "Gamma value", CSVAR_FLOAT },
  { 8, "dmipmap1", "Mipmap distance 1", CSVAR_FLOAT },
  { 9, "dmipmap2", "Mipmap distance 2", CSVAR_FLOAT },
  { 10, "dmipmap3", "Mipmap distance 3", CSVAR_FLOAT },
  { 11, "gouraud", "Gouraud shading", CSVAR_BOOL },
};

#define NUM_OPTIONS 12

STDMETHODIMP IXConfig3DSoft::SetOption (int id, csVariant* value)
{
  METHOD_PROLOGUE (csGraphics3DSoftware, XConfig3DSoft);
  if (value->type != config_options[id].type) return E_FAIL;
  switch (id)
  {
    case 0: pThis->do_interlaced = value->v.bVal ? 0 : -1; break;
    case 1: pThis->do_lighting = value->v.bVal; break;
    case 2: pThis->do_transp = value->v.bVal; break;
    case 3: pThis->do_textured = value->v.bVal; break;
    case 4: pThis->do_texel_filt = value->v.bVal; break;
    case 5: pThis->do_bilin_filt = value->v.bVal; break;
#ifdef DO_MMX
    case 6: pThis->do_mmx = value->v.bVal; break;
#endif
    case 7: pThis->txtmgr->Gamma = value->v.fVal; break;
    case 8: pThis->zdist_mipmap1 = value->v.fVal; break;
    case 9: pThis->zdist_mipmap2 = value->v.fVal; break;
    case 10: pThis->zdist_mipmap3 = value->v.fVal; break;
    case 11: pThis->rstate_gouraud = value->v.bVal; break;
    default: return E_FAIL;
  }
  pThis->ScanSetup ();
  return S_OK;
}

STDMETHODIMP IXConfig3DSoft::GetOption (int id, csVariant* value)
{
  METHOD_PROLOGUE (csGraphics3DSoftware, XConfig3DSoft);
  value->type = config_options[id].type;
  switch (id)
  {
    case 0: value->v.bVal = pThis->do_interlaced != -1; break;
    case 1: value->v.bVal = pThis->do_lighting; break;
    case 2: value->v.bVal = pThis->do_transp; break;
    case 3: value->v.bVal = pThis->do_textured; break;
    case 4: value->v.bVal = pThis->do_texel_filt; break;
    case 5: value->v.bVal = pThis->do_bilin_filt; break;
#ifdef DO_MMX
    case 6: value->v.bVal = pThis->do_mmx; break;
#endif
    case 7: value->v.fVal = pThis->txtmgr->Gamma; break;
    case 8: value->v.fVal = pThis->zdist_mipmap1; break;
    case 9: value->v.fVal = pThis->zdist_mipmap2; break;
    case 10: value->v.fVal = pThis->zdist_mipmap3; break;
    case 11: value->v.bVal = pThis->rstate_gouraud; break;
    default: return E_FAIL;
  }
  return S_OK;
}

STDMETHODIMP IXConfig3DSoft::GetNumberOptions (int& num)
{
//METHOD_PROLOGUE (csGraphics3DSoftware, XConfig3DSoft);
  num = NUM_OPTIONS;
  return S_OK;
}

STDMETHODIMP IXConfig3DSoft::GetOptionDescription (int idx, csOptionDescription* option)
{
//METHOD_PROLOGUE (csGraphics3DSoftware, XConfig3DSoft);
  if (idx < 0 || idx >= NUM_OPTIONS) return E_FAIL;
  *option = config_options[idx];
  return S_OK;
}

//---------------------------------------------------------------------------

