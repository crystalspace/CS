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

#include <stdarg.h>

#define CS_SYSDEF_PROVIDE_SOFTWARE2D
#include "cssysdef.h"
#include "qint.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/transfrm.h"
#include "csgeom/polyclip.h"
#include "csgeom/plane3.h"
#include "scan.h"
#include "tcache.h"
#include "soft_txt.h"
#include "isys/system.h"
#include "ivideo/graph2d.h"
#include "imesh/thing/lightmap.h"	//@@@!!!
#include "sft3dcom.h"
#include "iutil/cfgfile.h"
#include "iutil/cmdline.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "isys/event.h"

#if defined (DO_MMX)
#  include "video/renderer/software/i386/cpuid.h"
#endif

int csGraphics3DSoftwareCommon::filter_bf = 1;

//-------------------------- The indices into arrays of scanline routines ------

/*
 *  The rules for scanproc index name building:
 *  Curly brackets means optional name components
 *  Square brackets denote enforced name components
 *  Everything outside brackets is a must
 *
 *  SCANPROC_{Persp_}{Source_}{Effects_}{Zmode}
 *
 *  Persp       = PI for perspective-incorrect routines
 *  Source      = TEX for non-lightmapped textures
 *                MAP for lightmapped textures
 *                FLAT for flat-shaded
 *                FOG for drawing fog
 *  Effects     = GOU for Gouraud-shading applied to the texture
 *                KEY for "key-color" source pixel removal
 *                FX if routine supports table-driven effects
 *                FXKEY for both FX and KEY effects
 *                ALPHA for alpha-mapped textures
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
#define SCANPROC_FLAT_ZNONE             0x00
#define SCANPROC_FLAT_ZFIL              0x01
#define SCANPROC_FLAT_ZUSE              0x02
#define SCANPROC_FLAT_ZTEST             0x03
#define SCANPROC_MAP_ZNONE              0x04
#define SCANPROC_MAP_ZFIL               0x05
#define SCANPROC_MAP_ZUSE               0x06
#define SCANPROC_MAP_ZTEST              0x07
#define SCANPROC_MAP_KEY_ZNONE          0x08
#define SCANPROC_MAP_KEY_ZFIL           0x09
#define SCANPROC_MAP_KEY_ZUSE           0x0a
#define SCANPROC_MAP_KEY_ZTEST          0x0b
#define SCANPROC_TEX_ZNONE              0x0c
#define SCANPROC_TEX_ZFIL               0x0d
#define SCANPROC_TEX_ZUSE               0x0e
#define SCANPROC_TEX_ZTEST              0x0f
#define SCANPROC_TEX_KEY_ZNONE          0x10
#define SCANPROC_TEX_KEY_ZFIL           0x11
#define SCANPROC_TEX_KEY_ZUSE           0x12
#define SCANPROC_TEX_KEY_ZTEST          0x13
#define SCANPROC_TEX_ALPHA_ZNONE        0x14
#define SCANPROC_TEX_ALPHA_ZFIL         0x15
#define SCANPROC_TEX_ALPHA_ZUSE         0x16
#define SCANPROC_TEX_ALPHA_ZTEST        0x17
#define SCANPROC_MAP_ALPHA_ZNONE        0x18
#define SCANPROC_MAP_ALPHA_ZFIL         0x19
#define SCANPROC_MAP_ALPHA_ZUSE         0x1a
#define SCANPROC_MAP_ALPHA_ZTEST        0x1b
#define SCANPROC_TEX_FX_ZNONE           0x1c
#define SCANPROC_TEX_FX_ZFIL            0x1d
#define SCANPROC_TEX_FX_ZUSE            0x1e
#define SCANPROC_TEX_FX_ZTEST           0x1f
#define SCANPROC_MAP_FX_ZNONE           0x20
#define SCANPROC_MAP_FX_ZFIL            0x21
#define SCANPROC_MAP_FX_ZUSE            0x22
#define SCANPROC_MAP_FX_ZTEST           0x23
// these do not have "zuse" counterparts
#define SCANPROC_ZFIL                   0x24
#define SCANPROC_FOG                    0x25
#define SCANPROC_FOG_VIEW               0x26

// The following routines have a different prototype

// Flat-shaded perspective-incorrect routines
#define SCANPROC_PI_FLAT_ZNONE          0x00
#define SCANPROC_PI_FLAT_ZFIL           0x01
#define SCANPROC_PI_FLAT_ZUSE           0x02
#define SCANPROC_PI_FLAT_ZTEST          0x03
// Textured flat-shaded polygons
#define SCANPROC_PI_TEX_ZNONE           0x04
#define SCANPROC_PI_TEX_ZFIL            0x05
#define SCANPROC_PI_TEX_ZUSE            0x06
#define SCANPROC_PI_TEX_ZTEST           0x07
#define SCANPROC_PI_TEX_KEY_ZNONE       0x08
#define SCANPROC_PI_TEX_KEY_ZFIL        0x09
#define SCANPROC_PI_TEX_KEY_ZUSE        0x0a
#define SCANPROC_PI_TEX_KEY_ZTEST       0x0b
// Textured flat-shaded polygons with tiling
#define SCANPROC_PI_TILE_TEX_ZNONE      0x0c
#define SCANPROC_PI_TILE_TEX_ZFIL       0x0d
#define SCANPROC_PI_TILE_TEX_ZUSE       0x0e
#define SCANPROC_PI_TILE_TEX_ZTEST      0x0f
#define SCANPROC_PI_TILE_TEX_KEY_ZNONE  0x10
#define SCANPROC_PI_TILE_TEX_KEY_ZFIL   0x11
#define SCANPROC_PI_TILE_TEX_KEY_ZUSE   0x12
#define SCANPROC_PI_TILE_TEX_KEY_ZTEST  0x13
// Scanline drawing routines with flat shading + effects.
#define SCANPROC_PI_FLAT_FX_ZNONE       0x14
#define SCANPROC_PI_FLAT_FX_ZFIL        0x15
#define SCANPROC_PI_FLAT_FX_ZUSE        0x16
#define SCANPROC_PI_FLAT_FX_ZTEST       0x17
#define SCANPROC_PI_TEX_FX_ZNONE        0x18
#define SCANPROC_PI_TEX_FX_ZFIL         0x19
#define SCANPROC_PI_TEX_FX_ZUSE         0x1a
#define SCANPROC_PI_TEX_FX_ZTEST        0x1b
#define SCANPROC_PI_TEX_FXKEY_ZNONE     0x1c
#define SCANPROC_PI_TEX_FXKEY_ZFIL      0x1d
#define SCANPROC_PI_TEX_FXKEY_ZUSE      0x1e
#define SCANPROC_PI_TEX_FXKEY_ZTEST     0x1f
#define SCANPROC_PI_TILE_TEX_FX_ZNONE   0x20
#define SCANPROC_PI_TILE_TEX_FX_ZFIL    0x21
#define SCANPROC_PI_TILE_TEX_FX_ZUSE    0x22
#define SCANPROC_PI_TILE_TEX_FX_ZTEST   0x23
#define SCANPROC_PI_TILE_TEX_FXKEY_ZNONE 0x24
#define SCANPROC_PI_TILE_TEX_FXKEY_ZFIL 0x25
#define SCANPROC_PI_TILE_TEX_FXKEY_ZUSE 0x26
#define SCANPROC_PI_TILE_TEX_FXKEY_ZTEST 0x27
// Perspective-incorrect flat-shaded alpha-mapped texture
#define SCANPROC_PI_TEX_ALPHA_ZNONE     0x28
#define SCANPROC_PI_TEX_ALPHA_ZFIL      0x29
#define SCANPROC_PI_TEX_ALPHA_ZUSE      0x2a
#define SCANPROC_PI_TEX_ALPHA_ZTEST     0x2b

// Gouraud-shaded PI routines should have same indices
// as their non-Gouraud counterparts. Every routine except
// flat-shaded ones have two versions: without table-driven
// effects (FX) and one with them.
#define SCANPROC_PI_FLAT_GOU_ZNONE           0x00
#define SCANPROC_PI_FLAT_GOU_ZFIL            0x01
#define SCANPROC_PI_FLAT_GOU_ZUSE            0x02
#define SCANPROC_PI_FLAT_GOU_ZTEST           0x03
// Textured Gouraud-shaded polygons
#define SCANPROC_PI_TEX_GOU_ZNONE            0x04
#define SCANPROC_PI_TEX_GOU_ZFIL             0x05
#define SCANPROC_PI_TEX_GOU_ZUSE             0x06
#define SCANPROC_PI_TEX_GOU_ZTEST            0x07
#define SCANPROC_PI_TEX_GOUKEY_ZNONE         0x08
#define SCANPROC_PI_TEX_GOUKEY_ZFIL          0x09
#define SCANPROC_PI_TEX_GOUKEY_ZUSE          0x0a
#define SCANPROC_PI_TEX_GOUKEY_ZTEST         0x0b
// Textured Gouraud-shaded polygons with tiling
#define SCANPROC_PI_TILE_TEX_GOU_ZNONE       0x0c
#define SCANPROC_PI_TILE_TEX_GOU_ZFIL        0x0d
#define SCANPROC_PI_TILE_TEX_GOU_ZUSE        0x0e
#define SCANPROC_PI_TILE_TEX_GOU_ZTEST       0x0f
#define SCANPROC_PI_TILE_TEX_GOUKEY_ZNONE    0x10
#define SCANPROC_PI_TILE_TEX_GOUKEY_ZFIL     0x11
#define SCANPROC_PI_TILE_TEX_GOUKEY_ZUSE     0x12
#define SCANPROC_PI_TILE_TEX_GOUKEY_ZTEST    0x13
// Scanline drawing routines with Gouraud shading + effects.
#define SCANPROC_PI_FLAT_GOUFX_ZNONE         0x14
#define SCANPROC_PI_FLAT_GOUFX_ZFIL          0x15
#define SCANPROC_PI_FLAT_GOUFX_ZUSE          0x16
#define SCANPROC_PI_FLAT_GOUFX_ZTEST         0x17
#define SCANPROC_PI_TEX_GOUFX_ZNONE          0x18
#define SCANPROC_PI_TEX_GOUFX_ZFIL           0x19
#define SCANPROC_PI_TEX_GOUFX_ZUSE           0x1a
#define SCANPROC_PI_TEX_GOUFX_ZTEST          0x1b
#define SCANPROC_PI_TEX_GOUFXKEY_ZNONE       0x1c
#define SCANPROC_PI_TEX_GOUFXKEY_ZFIL        0x1d
#define SCANPROC_PI_TEX_GOUFXKEY_ZUSE        0x1e
#define SCANPROC_PI_TEX_GOUFXKEY_ZTEST       0x1f
#define SCANPROC_PI_TILE_TEX_GOUFX_ZNONE     0x20
#define SCANPROC_PI_TILE_TEX_GOUFX_ZFIL      0x21
#define SCANPROC_PI_TILE_TEX_GOUFX_ZUSE      0x22
#define SCANPROC_PI_TILE_TEX_GOUFX_ZTEST     0x23
#define SCANPROC_PI_TILE_TEX_GOUFXKEY_ZNONE  0x24
#define SCANPROC_PI_TILE_TEX_GOUFXKEY_ZFIL   0x25
#define SCANPROC_PI_TILE_TEX_GOUFXKEY_ZUSE   0x26
#define SCANPROC_PI_TILE_TEX_GOUFXKEY_ZTEST  0x27

///---------------------------------------------------------------------------
SCF_IMPLEMENT_IBASE(csGraphics3DSoftwareCommon)
  SCF_IMPLEMENTS_INTERFACE(iGraphics3D)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGraphics3DSoftwareCommon::eiPlugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csGraphics3DSoftwareCommon::csGraphics3DSoftwareCommon (iBase* parent) :
  G2D (NULL)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);

  tcache = NULL;
  texman = NULL;
  vbufmgr = NULL;
  partner = NULL;
  clipper = NULL;
  cliptype = CS_CLIPPER_NONE;
  do_near_plane = false;

#ifdef DO_MMX
  do_mmx = true;
#endif
  do_lighting = true;
  do_alpha = true;
  do_textured = true;
  do_interlaced = -1;
  ilace_fastmove = false;
  bilinear_filter = 0;
  do_smaller_rendering = false;
  smaller_buffer = NULL;
  pixel_shift = 0;
  rstate_mipmap = 0;
  do_gouraud = true;
  Gamma = QInt16 (1.0);

  dbg_max_polygons_to_draw = 2000000000; // After 2 billion polygons we give up :-)

  z_buffer = NULL;
  line_table = NULL;

  Caps.CanClip = false;
  Caps.minTexHeight = 2;
  Caps.minTexWidth = 2;
  Caps.maxTexHeight = 1024;
  Caps.maxTexWidth = 1024;
  Caps.fog = G3DFOGMETHOD_ZBUFFER;
  Caps.NeedsPO2Maps = false;
  Caps.MaxAspectRatio = 32768;
  width = height = -1;
  partner = NULL;
  is_for_procedural_textures = false;

  dpfx_valid = false;

  object_reg = NULL;
}

csGraphics3DSoftwareCommon::~csGraphics3DSoftwareCommon ()
{
  Close ();
  if (G2D) G2D->DecRef ();
  if (partner) partner->DecRef ();
  if (clipper)
  {
    clipper->DecRef ();
    clipper = NULL;
    cliptype = CS_CLIPPER_NONE;
  }
}

bool csGraphics3DSoftwareCommon::Initialize (iObjectRegistry* p)
{
  object_reg = p;
  iSystem* sys = CS_GET_SYSTEM (object_reg);	//@@@
  sys->CallOnEvents (&scfiPlugin, CSMASK_Broadcast);
  return true;
}

bool csGraphics3DSoftwareCommon::HandleEvent (iEvent& Event)
{
  if (Event.Type == csevBroadcast)
    switch (Event.Command.Code)
    {
      case cscmdSystemOpen:
      {
        Open ();
        return true;
      }
      case cscmdSystemClose:
      {
        Close ();
        return true;
      }
    }
  return false;
}

void csGraphics3DSoftwareCommon::NewInitialize ()
{
  config.AddConfig(object_reg, "/config/soft3d.cfg");
  iCommandLineParser* cmdline = CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser);
  do_smaller_rendering = config->GetBool ("Video.Software.Smaller", false);
  mipmap_coef = config->GetFloat ("Video.Software.TextureManager.MipmapCoef", 1.3);
  do_interlaced = config->GetBool ("Video.Software.Interlacing", false) ? 0 : -1;

  const char *gamma = cmdline->GetOption ("gamma");
  if (!gamma) gamma = config->GetStr ("Video.Software.Gamma", "1");
  float fGamma;
  sscanf (gamma, "%f", &fGamma);
  Gamma = QInt16 (fGamma);

#ifdef DO_MMX
  do_mmx = config->GetBool ("Video.Software.MMX", true);
#endif
}

void csGraphics3DSoftwareCommon::SharedInitialize(csGraphics3DSoftwareCommon *p)
{
  // Avoid reading in a config file from the hard-drive
  partner = p;
  partner->IncRef ();
  do_smaller_rendering = p->do_smaller_rendering;
  mipmap_coef = p->mipmap_coef;
  do_interlaced = p->do_interlaced;
  Gamma = p->Gamma;
#ifdef DO_MMX
  do_mmx = p->do_mmx;
#endif
}

void csGraphics3DSoftwareCommon::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  iReporter* rep = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (rep)
  {
    rep->ReportV (severity, "crystalspace.video.software", msg, arg);
  }
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

bool csGraphics3DSoftwareCommon::Open ()
{
  if (!G2D->Open ())
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error opening Graphics2D context.");
    // set "not opened" flag
    width = height = -1;
    return false;
  }

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

  if (pfmt.PixelBytes == 4)
    pixel_shift = 2;
  else if (pfmt.PixelBytes == 2)
    pixel_shift = 1;
  else
    pixel_shift = 0;

  DrawMode = 0;
  SetDimensions (G2D->GetWidth (), G2D->GetHeight ());
  z_buf_mode = CS_ZBUF_NONE;

  int i;
  for (i = 0; i < MAX_INDEXED_FOG_TABLES; i++)
    fog_tables [i].table = NULL;

  return true;
}

bool csGraphics3DSoftwareCommon::NewOpen ()
{
#if defined (DO_MMX)
  int family, features;
  char vendor [13];
  csDetectCPU (&family, vendor, &features);
  cpu_mmx = (features & CPUx86_FEATURE_MMX) != 0;
  Report (CS_REPORTER_SEVERITY_NOTIFY,
  	"%d %s CPU detected; FPU (%s) MMX (%s) CMOV (%s)",
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

  fog_buffers = NULL;

  // Create the texture manager
  texman = new csTextureManagerSoftware (object_reg, this, config);
  texman->SetPixelFormat (pfmt);

  // Create the vertex buffer manager.
  vbufmgr = new csVertexBufferManager (object_reg);

  tcache = new csTextureCacheSoftware (texman);
  const char *cache_size = config->GetStr
        ("Video.Software.TextureManager.Cache", NULL);
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
      Report (CS_REPORTER_SEVERITY_NOTIFY,
        "Invalid cache size specified, using default");
      csize = DEFAULT_CACHE_SIZE;
    }
  }
  tcache->set_cache_size (csize);

  ScanSetup ();

  SetRenderState (G3DRENDERSTATE_INTERLACINGENABLE, do_interlaced == 0);
  SetRenderState (G3DRENDERSTATE_GAMMACORRECTION, Gamma);

  return true;
}

bool csGraphics3DSoftwareCommon::SharedOpen ()
{
  pixel_shift = partner->pixel_shift;
  fog_buffers = partner->fog_buffers;
  alpha_mask = partner->alpha_mask;
  z_buf_mode = partner->z_buf_mode;
#if defined (DO_MMX)
  cpu_mmx = partner->cpu_mmx;
#endif
  texman = partner->texman;
  vbufmgr = partner->vbufmgr;
  tcache = partner->tcache;
  ScanSetup ();
  SetRenderState (G3DRENDERSTATE_INTERLACINGENABLE, do_interlaced == 0);
  SetRenderState (G3DRENDERSTATE_GAMMACORRECTION, Gamma);
  return true;
}

void csGraphics3DSoftwareCommon::ScanSetup ()
{
  // Select the right scanline drawing functions
  memset (&ScanProc, 0, sizeof (ScanProc));
  memset (&ScanProcPI, 0, sizeof (ScanProcPI));
  memset (&ScanProcPIG, 0, sizeof (ScanProcPIG));
  ScanProc_Alpha = NULL;

#ifdef DO_MMX
  bool UseMMX = (cpu_mmx && do_mmx);
#endif

  // Bits-per-pixel independent routine
  ScanProc [SCANPROC_ZFIL] = csScan_scan_zfil;

  switch (pfmt.PixelBytes)
  {
    case 1:
      if (do_alpha) ScanProc_Alpha = ScanProc_8_Alpha;

      ScanProc [SCANPROC_FLAT_ZNONE] = csScan_8_scan_flat_znone;
      ScanProc [SCANPROC_FLAT_ZFIL] = csScan_8_scan_flat_zfil;
      ScanProc [SCANPROC_FLAT_ZUSE] = csScan_8_scan_flat_zuse;
      ScanProc [SCANPROC_FLAT_ZTEST] = csScan_8_scan_flat_ztest;

      ScanProc [SCANPROC_TEX_ZNONE] = csScan_8_scan_tex_znone;
      ScanProc [SCANPROC_TEX_ZFIL] =
#ifdef DO_MMX
        UseMMX ? csScan_8_mmx_scan_tex_zfil :
#endif
        csScan_8_scan_tex_zfil;
      ScanProc [SCANPROC_TEX_ZUSE] = csScan_8_scan_tex_zuse;
      ScanProc [SCANPROC_TEX_ZTEST] = csScan_8_scan_tex_ztest;

      ScanProc [SCANPROC_MAP_ZNONE] = csScan_8_scan_map_znone;
      ScanProc [SCANPROC_MAP_ZFIL] =
        bilinear_filter ? csScan_8_scan_map_filt_zfil :
#ifdef DO_MMX
        UseMMX ? csScan_8_mmx_scan_map_zfil :
#endif
        csScan_8_scan_map_zfil;
      ScanProc [SCANPROC_MAP_ZUSE] = csScan_8_scan_map_zuse;
      ScanProc [SCANPROC_MAP_ZTEST] = csScan_8_scan_map_ztest;

      ScanProc [SCANPROC_TEX_KEY_ZNONE] = csScan_8_scan_tex_key_znone;
      ScanProc [SCANPROC_TEX_KEY_ZFIL] = csScan_8_scan_tex_key_zfil;
      ScanProc [SCANPROC_TEX_KEY_ZUSE] = csScan_8_scan_tex_key_zuse;
      ScanProc [SCANPROC_TEX_KEY_ZTEST] = csScan_8_scan_tex_key_ztest;
      ScanProc [SCANPROC_MAP_KEY_ZNONE] = csScan_8_scan_map_key_znone;
      ScanProc [SCANPROC_MAP_KEY_ZFIL] = csScan_8_scan_map_key_zfil;
      ScanProc [SCANPROC_MAP_KEY_ZUSE] = csScan_8_scan_map_key_zuse;
      ScanProc [SCANPROC_MAP_KEY_ZTEST] = csScan_8_scan_map_key_ztest;

      ScanProc [SCANPROC_TEX_ALPHA_ZNONE] = csScan_8_scan_tex_alpha_znone;
      ScanProc [SCANPROC_TEX_ALPHA_ZFIL] = csScan_8_scan_tex_alpha_zfil;
      ScanProc [SCANPROC_TEX_ALPHA_ZUSE] = csScan_8_scan_tex_alpha_zuse;
      ScanProc [SCANPROC_TEX_ALPHA_ZTEST] = csScan_8_scan_tex_alpha_ztest;
      ScanProc [SCANPROC_MAP_ALPHA_ZNONE] = csScan_8_scan_map_alpha_znone;
      ScanProc [SCANPROC_MAP_ALPHA_ZFIL] = csScan_8_scan_map_alpha_zfil;
      ScanProc [SCANPROC_MAP_ALPHA_ZUSE] = csScan_8_scan_map_alpha_zuse;
      ScanProc [SCANPROC_MAP_ALPHA_ZTEST] = csScan_8_scan_map_alpha_ztest;

      ScanProc [SCANPROC_TEX_FX_ZNONE] = csScan_8_scan_tex_fx_znone;
      ScanProc [SCANPROC_TEX_FX_ZFIL] = csScan_8_scan_tex_fx_zfil;
      ScanProc [SCANPROC_TEX_FX_ZUSE] = csScan_8_scan_tex_fx_zuse;
      ScanProc [SCANPROC_TEX_FX_ZTEST] = csScan_8_scan_tex_fx_ztest;
      ScanProc [SCANPROC_MAP_FX_ZNONE] = csScan_8_scan_map_fx_znone;
      ScanProc [SCANPROC_MAP_FX_ZFIL] = csScan_8_scan_map_fx_zfil;
      ScanProc [SCANPROC_MAP_FX_ZUSE] = csScan_8_scan_map_fx_zuse;
      ScanProc [SCANPROC_MAP_FX_ZTEST] = csScan_8_scan_map_fx_ztest;

      ScanProc [SCANPROC_FOG] = csScan_8_scan_fog;
      ScanProc [SCANPROC_FOG_VIEW] = csScan_8_scan_fog_view;

      ScanProcPI [SCANPROC_PI_FLAT_ZNONE] = csScan_8_scan_pi_flat_znone;
      ScanProcPI [SCANPROC_PI_FLAT_ZFIL] = csScan_8_scan_pi_flat_zfil;
      ScanProcPI [SCANPROC_PI_FLAT_ZUSE] = csScan_8_scan_pi_flat_zuse;
      ScanProcPI [SCANPROC_PI_FLAT_ZTEST] = csScan_8_scan_pi_flat_ztest;
      ScanProcPI [SCANPROC_PI_TEX_ZNONE] = csScan_8_scan_pi_tex_znone;
      ScanProcPI [SCANPROC_PI_TEX_ZFIL] = csScan_8_scan_pi_tex_zfil;
      ScanProcPI [SCANPROC_PI_TEX_ZUSE] = csScan_8_scan_pi_tex_zuse;
      ScanProcPI [SCANPROC_PI_TEX_ZTEST] = csScan_8_scan_pi_tex_ztest;
      ScanProcPI [SCANPROC_PI_TEX_KEY_ZNONE] = csScan_8_scan_pi_tex_key_znone;
      ScanProcPI [SCANPROC_PI_TEX_KEY_ZFIL] = csScan_8_scan_pi_tex_key_zfil;
      ScanProcPI [SCANPROC_PI_TEX_KEY_ZUSE] = csScan_8_scan_pi_tex_key_zuse;
      ScanProcPI [SCANPROC_PI_TEX_KEY_ZTEST] = csScan_8_scan_pi_tex_key_ztest;
      ScanProcPI [SCANPROC_PI_TILE_TEX_ZNONE] = csScan_8_scan_pi_tile_tex_znone;
      ScanProcPI [SCANPROC_PI_TILE_TEX_ZFIL] = csScan_8_scan_pi_tile_tex_zfil;
      ScanProcPI [SCANPROC_PI_TILE_TEX_ZUSE] = csScan_8_scan_pi_tile_tex_zuse;
      ScanProcPI [SCANPROC_PI_TILE_TEX_ZTEST] = csScan_8_scan_pi_tile_tex_ztest;
      ScanProcPI [SCANPROC_PI_TILE_TEX_KEY_ZNONE] = csScan_8_scan_pi_tile_tex_key_znone;
      ScanProcPI [SCANPROC_PI_TILE_TEX_KEY_ZFIL] = csScan_8_scan_pi_tile_tex_key_zfil;
      ScanProcPI [SCANPROC_PI_TILE_TEX_KEY_ZUSE] = csScan_8_scan_pi_tile_tex_key_zuse;
      ScanProcPI [SCANPROC_PI_TILE_TEX_KEY_ZTEST] = csScan_8_scan_pi_tile_tex_key_ztest;
      ScanProcPI [SCANPROC_PI_FLAT_FX_ZNONE] = csScan_8_scan_pi_flat_fx_znone;
      ScanProcPI [SCANPROC_PI_FLAT_FX_ZFIL] = csScan_8_scan_pi_flat_fx_zfil;
      ScanProcPI [SCANPROC_PI_FLAT_FX_ZUSE] = csScan_8_scan_pi_flat_fx_zuse;
      ScanProcPI [SCANPROC_PI_FLAT_FX_ZTEST] = csScan_8_scan_pi_flat_fx_ztest;
      ScanProcPI [SCANPROC_PI_TEX_FX_ZNONE] = csScan_8_scan_pi_tex_fx_znone;
      ScanProcPI [SCANPROC_PI_TEX_FX_ZFIL] = csScan_8_scan_pi_tex_fx_zfil;
      ScanProcPI [SCANPROC_PI_TEX_FX_ZUSE] = csScan_8_scan_pi_tex_fx_zuse;
      ScanProcPI [SCANPROC_PI_TEX_FX_ZTEST] = csScan_8_scan_pi_tex_fx_ztest;
      ScanProcPI [SCANPROC_PI_TEX_FXKEY_ZNONE] = csScan_8_scan_pi_tex_fxkey_znone;
      ScanProcPI [SCANPROC_PI_TEX_FXKEY_ZFIL] = csScan_8_scan_pi_tex_fxkey_zfil;
      ScanProcPI [SCANPROC_PI_TEX_FXKEY_ZUSE] = csScan_8_scan_pi_tex_fxkey_zuse;
      ScanProcPI [SCANPROC_PI_TEX_FXKEY_ZTEST] = csScan_8_scan_pi_tex_fxkey_ztest;
      ScanProcPI [SCANPROC_PI_TILE_TEX_FX_ZNONE] = csScan_8_scan_pi_tile_tex_fx_znone;
      ScanProcPI [SCANPROC_PI_TILE_TEX_FX_ZFIL] = csScan_8_scan_pi_tile_tex_fx_zfil;
      ScanProcPI [SCANPROC_PI_TILE_TEX_FX_ZUSE] = csScan_8_scan_pi_tile_tex_fx_zuse;
      ScanProcPI [SCANPROC_PI_TILE_TEX_FX_ZTEST] = csScan_8_scan_pi_tile_tex_fx_ztest;
      ScanProcPI [SCANPROC_PI_TILE_TEX_FXKEY_ZNONE] = csScan_8_scan_pi_tile_tex_fxkey_znone;
      ScanProcPI [SCANPROC_PI_TILE_TEX_FXKEY_ZFIL] = csScan_8_scan_pi_tile_tex_fxkey_zfil;
      ScanProcPI [SCANPROC_PI_TILE_TEX_FXKEY_ZUSE] = csScan_8_scan_pi_tile_tex_fxkey_zuse;
      ScanProcPI [SCANPROC_PI_TILE_TEX_FXKEY_ZTEST] = csScan_8_scan_pi_tile_tex_fxkey_ztest;
      ScanProcPI [SCANPROC_PI_TEX_ALPHA_ZNONE] = csScan_8_scan_pi_tex_alpha_znone;
      ScanProcPI [SCANPROC_PI_TEX_ALPHA_ZFIL] = csScan_8_scan_pi_tex_alpha_zfil;
      ScanProcPI [SCANPROC_PI_TEX_ALPHA_ZUSE] = csScan_8_scan_pi_tex_alpha_zuse;
      ScanProcPI [SCANPROC_PI_TEX_ALPHA_ZTEST] = csScan_8_scan_pi_tex_alpha_ztest;

      ScanProcPIG [SCANPROC_PI_FLAT_GOU_ZNONE] = csScan_8_scan_pi_flat_gou_znone;
      ScanProcPIG [SCANPROC_PI_FLAT_GOU_ZFIL] = csScan_8_scan_pi_flat_gou_zfil;
      ScanProcPIG [SCANPROC_PI_FLAT_GOU_ZUSE] = csScan_8_scan_pi_flat_gou_zuse;
      ScanProcPIG [SCANPROC_PI_FLAT_GOU_ZTEST] = csScan_8_scan_pi_flat_gou_ztest;
      ScanProcPIG [SCANPROC_PI_TEX_GOU_ZNONE] = csScan_8_scan_pi_tex_gou_znone;
      ScanProcPIG [SCANPROC_PI_TEX_GOU_ZFIL] = csScan_8_scan_pi_tex_gou_zfil;
      ScanProcPIG [SCANPROC_PI_TEX_GOU_ZUSE] = csScan_8_scan_pi_tex_gou_zuse;
      ScanProcPIG [SCANPROC_PI_TEX_GOU_ZTEST] = csScan_8_scan_pi_tex_gou_ztest;
      ScanProcPIG [SCANPROC_PI_TEX_GOUKEY_ZNONE] = csScan_8_scan_pi_tex_goukey_znone;
      ScanProcPIG [SCANPROC_PI_TEX_GOUKEY_ZFIL] = csScan_8_scan_pi_tex_goukey_zfil;
      ScanProcPIG [SCANPROC_PI_TEX_GOUKEY_ZUSE] = csScan_8_scan_pi_tex_goukey_zuse;
      ScanProcPIG [SCANPROC_PI_TEX_GOUKEY_ZTEST] = csScan_8_scan_pi_tex_goukey_ztest;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOU_ZNONE] = csScan_8_scan_pi_tile_tex_gou_znone;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOU_ZFIL] = csScan_8_scan_pi_tile_tex_gou_zfil;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOU_ZUSE] = csScan_8_scan_pi_tile_tex_gou_zuse;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOU_ZTEST] = csScan_8_scan_pi_tile_tex_gou_ztest;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUKEY_ZNONE] = csScan_8_scan_pi_tile_tex_goukey_znone;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUKEY_ZFIL] = csScan_8_scan_pi_tile_tex_goukey_zfil;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUKEY_ZUSE] = csScan_8_scan_pi_tile_tex_goukey_zuse;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUKEY_ZTEST] = csScan_8_scan_pi_tile_tex_goukey_ztest;
      ScanProcPIG [SCANPROC_PI_FLAT_GOUFX_ZNONE] = csScan_8_scan_pi_flat_goufx_znone;
      ScanProcPIG [SCANPROC_PI_FLAT_GOUFX_ZFIL] = csScan_8_scan_pi_flat_goufx_zfil;
      ScanProcPIG [SCANPROC_PI_FLAT_GOUFX_ZUSE] = csScan_8_scan_pi_flat_goufx_zuse;
      ScanProcPIG [SCANPROC_PI_FLAT_GOUFX_ZTEST] = csScan_8_scan_pi_flat_goufx_ztest;
      ScanProcPIG [SCANPROC_PI_TEX_GOUFX_ZNONE] = csScan_8_scan_pi_tex_goufx_znone;
      ScanProcPIG [SCANPROC_PI_TEX_GOUFX_ZFIL] = csScan_8_scan_pi_tex_goufx_zfil;
      ScanProcPIG [SCANPROC_PI_TEX_GOUFX_ZUSE] = csScan_8_scan_pi_tex_goufx_zuse;
      ScanProcPIG [SCANPROC_PI_TEX_GOUFX_ZTEST] = csScan_8_scan_pi_tex_goufx_ztest;
      ScanProcPIG [SCANPROC_PI_TEX_GOUFXKEY_ZNONE] = csScan_8_scan_pi_tex_goufxkey_znone;
      ScanProcPIG [SCANPROC_PI_TEX_GOUFXKEY_ZFIL] = csScan_8_scan_pi_tex_goufxkey_zfil;
      ScanProcPIG [SCANPROC_PI_TEX_GOUFXKEY_ZUSE] = csScan_8_scan_pi_tex_goufxkey_zuse;
      ScanProcPIG [SCANPROC_PI_TEX_GOUFXKEY_ZTEST] = csScan_8_scan_pi_tex_goufxkey_ztest;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUFX_ZNONE] = csScan_8_scan_pi_tile_tex_goufx_znone;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUFX_ZFIL] = csScan_8_scan_pi_tile_tex_goufx_zfil;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUFX_ZUSE] = csScan_8_scan_pi_tile_tex_goufx_zuse;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUFX_ZTEST] = csScan_8_scan_pi_tile_tex_goufx_ztest;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUFXKEY_ZNONE] = csScan_8_scan_pi_tile_tex_goufxkey_znone;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUFXKEY_ZFIL] = csScan_8_scan_pi_tile_tex_goufxkey_zfil;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUFXKEY_ZUSE] = csScan_8_scan_pi_tile_tex_goufxkey_zuse;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUFXKEY_ZTEST] = csScan_8_scan_pi_tile_tex_goufxkey_ztest;
      break;

    case 2:
      if (do_alpha) ScanProc_Alpha = ScanProc_16_Alpha;

      ScanProc [SCANPROC_FLAT_ZNONE] = csScan_16_scan_flat_znone;
      ScanProc [SCANPROC_FLAT_ZFIL] = csScan_16_scan_flat_zfil;
      ScanProc [SCANPROC_FLAT_ZUSE] = csScan_16_scan_flat_zuse;
      ScanProc [SCANPROC_FLAT_ZTEST] = csScan_16_scan_flat_ztest;

      ScanProc [SCANPROC_TEX_ZNONE] = csScan_16_scan_tex_znone;
      ScanProc [SCANPROC_TEX_ZFIL] =
#ifdef DO_MMX
        UseMMX ? csScan_16_mmx_scan_tex_zfil :
#endif
        csScan_16_scan_tex_zfil;
      ScanProc [SCANPROC_TEX_ZUSE] = csScan_16_scan_tex_zuse;
      ScanProc [SCANPROC_TEX_ZTEST] = csScan_16_scan_tex_ztest;

      ScanProc [SCANPROC_MAP_ZNONE] =
        bilinear_filter == 2 ?
        (pfmt.GreenBits == 5 ?
          csScan_16_555_scan_map_filt2_znone :
          csScan_16_565_scan_map_filt2_znone) :
        bilinear_filter == 1 ? csScan_16_scan_map_filt_znone :
        csScan_16_scan_map_znone;
      ScanProc [SCANPROC_MAP_ZFIL] =
        bilinear_filter == 2 ?
        (pfmt.GreenBits == 5 ?
          csScan_16_555_scan_map_filt2_zfil :
          csScan_16_565_scan_map_filt2_zfil) :
        bilinear_filter == 1 ? csScan_16_scan_map_filt_zfil :
#ifdef DO_MMX
        UseMMX ? csScan_16_mmx_scan_map_zfil :
#endif
        csScan_16_scan_map_zfil;
      ScanProc [SCANPROC_MAP_ZUSE] =
        bilinear_filter == 2 ?
        (pfmt.GreenBits == 5 ?
          csScan_16_555_scan_map_filt2_zuse :
          csScan_16_565_scan_map_filt2_zuse) :
        csScan_16_scan_map_zuse;
      ScanProc [SCANPROC_MAP_ZTEST] =
        bilinear_filter == 2 ?
        (pfmt.GreenBits == 5 ?
          csScan_16_555_scan_map_filt2_ztest :
          csScan_16_565_scan_map_filt2_ztest) :
        csScan_16_scan_map_ztest;

      ScanProc [SCANPROC_TEX_KEY_ZNONE] = csScan_16_scan_tex_key_znone;
      ScanProc [SCANPROC_TEX_KEY_ZFIL] = csScan_16_scan_tex_key_zfil;
      ScanProc [SCANPROC_TEX_KEY_ZUSE] = csScan_16_scan_tex_key_zuse;
      ScanProc [SCANPROC_TEX_KEY_ZTEST] = csScan_16_scan_tex_key_ztest;
      ScanProc [SCANPROC_MAP_KEY_ZNONE] = csScan_16_scan_map_key_znone;
      ScanProc [SCANPROC_MAP_KEY_ZFIL] = csScan_16_scan_map_key_zfil;
      ScanProc [SCANPROC_MAP_KEY_ZUSE] = csScan_16_scan_map_key_zuse;
      ScanProc [SCANPROC_MAP_KEY_ZTEST] = csScan_16_scan_map_key_ztest;

      ScanProc [SCANPROC_TEX_ALPHA_ZNONE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_tex_alpha_znone :
        csScan_16_565_scan_tex_alpha_znone;
      ScanProc [SCANPROC_TEX_ALPHA_ZFIL] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_tex_alpha_zfil :
        csScan_16_565_scan_tex_alpha_zfil;
      ScanProc [SCANPROC_TEX_ALPHA_ZUSE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_tex_alpha_zuse :
        csScan_16_565_scan_tex_alpha_zuse;
      ScanProc [SCANPROC_TEX_ALPHA_ZTEST] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_tex_alpha_ztest :
        csScan_16_565_scan_tex_alpha_ztest;
      ScanProc [SCANPROC_MAP_ALPHA_ZNONE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_map_alpha_znone :
        csScan_16_565_scan_map_alpha_znone;
      ScanProc [SCANPROC_MAP_ALPHA_ZFIL] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_map_alpha_zfil :
        csScan_16_565_scan_map_alpha_zfil;
      ScanProc [SCANPROC_MAP_ALPHA_ZUSE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_map_alpha_zuse :
        csScan_16_565_scan_map_alpha_zuse;
      ScanProc [SCANPROC_MAP_ALPHA_ZTEST] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_map_alpha_ztest :
        csScan_16_565_scan_map_alpha_ztest;

      ScanProc [SCANPROC_TEX_FX_ZNONE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_tex_fx_znone :
        csScan_16_565_scan_tex_fx_znone;
      ScanProc [SCANPROC_TEX_FX_ZFIL] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_tex_fx_zfil :
        csScan_16_565_scan_tex_fx_zfil;
      ScanProc [SCANPROC_TEX_FX_ZUSE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_tex_fx_zuse :
        csScan_16_565_scan_tex_fx_zuse;
      ScanProc [SCANPROC_TEX_FX_ZTEST] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_tex_fx_ztest :
        csScan_16_565_scan_tex_fx_ztest;
      ScanProc [SCANPROC_MAP_FX_ZNONE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_map_fx_znone :
        csScan_16_565_scan_map_fx_znone;
      ScanProc [SCANPROC_MAP_FX_ZFIL] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_map_fx_zfil :
        csScan_16_565_scan_map_fx_zfil;
      ScanProc [SCANPROC_MAP_FX_ZUSE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_map_fx_zuse :
        csScan_16_565_scan_map_fx_zuse;
      ScanProc [SCANPROC_MAP_FX_ZTEST] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_map_fx_ztest :
        csScan_16_565_scan_map_fx_ztest;

      ScanProc [SCANPROC_FOG] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_fog :
        csScan_16_565_scan_fog;
      ScanProc [SCANPROC_FOG_VIEW] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_fog_view :
        csScan_16_565_scan_fog_view;

      ScanProcPI [SCANPROC_PI_FLAT_ZNONE] = csScan_16_scan_pi_flat_znone;
      ScanProcPI [SCANPROC_PI_FLAT_ZFIL] = csScan_16_scan_pi_flat_zfil;
      ScanProcPI [SCANPROC_PI_FLAT_ZUSE] = csScan_16_scan_pi_flat_zuse;
      ScanProcPI [SCANPROC_PI_FLAT_ZTEST] = csScan_16_scan_pi_flat_ztest;
      ScanProcPI [SCANPROC_PI_TEX_ZNONE] = csScan_16_scan_pi_tex_znone;
      ScanProcPI [SCANPROC_PI_TEX_ZFIL] = csScan_16_scan_pi_tex_zfil;
      ScanProcPI [SCANPROC_PI_TEX_ZUSE] =
#ifdef DO_MMX
        UseMMX ? csScan_16_mmx_scan_pi_tex_zuse :
#endif
        csScan_16_scan_pi_tex_zuse;
      ScanProcPI [SCANPROC_PI_TEX_ZTEST] = csScan_16_scan_pi_tex_ztest;
      ScanProcPI [SCANPROC_PI_TEX_KEY_ZNONE] = csScan_16_scan_pi_tex_key_znone;
      ScanProcPI [SCANPROC_PI_TEX_KEY_ZFIL] = csScan_16_scan_pi_tex_key_zfil;
      ScanProcPI [SCANPROC_PI_TEX_KEY_ZUSE] = csScan_16_scan_pi_tex_key_zuse;
      ScanProcPI [SCANPROC_PI_TEX_KEY_ZTEST] = csScan_16_scan_pi_tex_key_ztest;
      ScanProcPI [SCANPROC_PI_TILE_TEX_ZNONE] = csScan_16_scan_pi_tile_tex_znone;
      ScanProcPI [SCANPROC_PI_TILE_TEX_ZFIL] = csScan_16_scan_pi_tile_tex_zfil;
      ScanProcPI [SCANPROC_PI_TILE_TEX_ZUSE] = csScan_16_scan_pi_tile_tex_zuse;
      ScanProcPI [SCANPROC_PI_TILE_TEX_ZTEST] = csScan_16_scan_pi_tile_tex_ztest;
      ScanProcPI [SCANPROC_PI_TILE_TEX_KEY_ZNONE] = csScan_16_scan_pi_tile_tex_key_znone;
      ScanProcPI [SCANPROC_PI_TILE_TEX_KEY_ZFIL] = csScan_16_scan_pi_tile_tex_key_zfil;
      ScanProcPI [SCANPROC_PI_TILE_TEX_KEY_ZUSE] = csScan_16_scan_pi_tile_tex_key_zuse;
      ScanProcPI [SCANPROC_PI_TILE_TEX_KEY_ZTEST] = csScan_16_scan_pi_tile_tex_key_ztest;

      ScanProcPI [SCANPROC_PI_FLAT_FX_ZNONE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_flat_fx_znone :
        csScan_16_565_scan_pi_flat_fx_znone;
      ScanProcPI [SCANPROC_PI_FLAT_FX_ZFIL] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_flat_fx_zfil :
        csScan_16_565_scan_pi_flat_fx_zfil;
      ScanProcPI [SCANPROC_PI_FLAT_FX_ZUSE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_flat_fx_zuse :
        csScan_16_565_scan_pi_flat_fx_zuse;
      ScanProcPI [SCANPROC_PI_FLAT_FX_ZTEST] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_flat_fx_ztest :
        csScan_16_565_scan_pi_flat_fx_ztest;
      ScanProcPI [SCANPROC_PI_TEX_FX_ZNONE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_fx_znone :
        csScan_16_565_scan_pi_tex_fx_znone;
      ScanProcPI [SCANPROC_PI_TEX_FX_ZFIL] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_fx_zfil :
        csScan_16_565_scan_pi_tex_fx_zfil;
      ScanProcPI [SCANPROC_PI_TEX_FX_ZUSE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_fx_zuse :
        csScan_16_565_scan_pi_tex_fx_zuse;
      ScanProcPI [SCANPROC_PI_TEX_FX_ZTEST] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_fx_ztest :
        csScan_16_565_scan_pi_tex_fx_ztest;
      ScanProcPI [SCANPROC_PI_TEX_FXKEY_ZNONE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_fxkey_znone :
        csScan_16_565_scan_pi_tex_fxkey_znone;
      ScanProcPI [SCANPROC_PI_TEX_FXKEY_ZFIL] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_fxkey_zfil :
        csScan_16_565_scan_pi_tex_fxkey_zfil;
      ScanProcPI [SCANPROC_PI_TEX_FXKEY_ZUSE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_fxkey_zuse :
        csScan_16_565_scan_pi_tex_fxkey_zuse;
      ScanProcPI [SCANPROC_PI_TEX_FXKEY_ZTEST] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_fxkey_ztest :
        csScan_16_565_scan_pi_tex_fxkey_ztest;
      ScanProcPI [SCANPROC_PI_TILE_TEX_FX_ZNONE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tile_tex_fx_znone :
        csScan_16_565_scan_pi_tile_tex_fx_znone;
      ScanProcPI [SCANPROC_PI_TILE_TEX_FX_ZFIL] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tile_tex_fx_zfil :
        csScan_16_565_scan_pi_tile_tex_fx_zfil;
      ScanProcPI [SCANPROC_PI_TILE_TEX_FX_ZUSE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tile_tex_fx_zuse :
        csScan_16_565_scan_pi_tile_tex_fx_zuse;
      ScanProcPI [SCANPROC_PI_TILE_TEX_FX_ZTEST] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tile_tex_fx_ztest :
        csScan_16_565_scan_pi_tile_tex_fx_ztest;
      ScanProcPI [SCANPROC_PI_TILE_TEX_FXKEY_ZNONE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tile_tex_fxkey_znone :
        csScan_16_565_scan_pi_tile_tex_fxkey_znone;
      ScanProcPI [SCANPROC_PI_TILE_TEX_FXKEY_ZFIL] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tile_tex_fxkey_zfil :
        csScan_16_565_scan_pi_tile_tex_fxkey_zfil;
      ScanProcPI [SCANPROC_PI_TILE_TEX_FXKEY_ZUSE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tile_tex_fxkey_zuse :
        csScan_16_565_scan_pi_tile_tex_fxkey_zuse;
      ScanProcPI [SCANPROC_PI_TILE_TEX_FXKEY_ZTEST] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tile_tex_fxkey_ztest :
        csScan_16_565_scan_pi_tile_tex_fxkey_ztest;
      ScanProcPI [SCANPROC_PI_TEX_ALPHA_ZNONE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_alpha_znone :
        csScan_16_565_scan_pi_tex_alpha_znone;
      ScanProcPI [SCANPROC_PI_TEX_ALPHA_ZFIL] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_alpha_zfil :
        csScan_16_565_scan_pi_tex_alpha_zfil;
      ScanProcPI [SCANPROC_PI_TEX_ALPHA_ZUSE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_alpha_zuse :
        csScan_16_565_scan_pi_tex_alpha_zuse;
      ScanProcPI [SCANPROC_PI_TEX_ALPHA_ZTEST] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_alpha_ztest :
        csScan_16_565_scan_pi_tex_alpha_ztest;

      ScanProcPIG [SCANPROC_PI_FLAT_GOU_ZNONE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_flat_gou_znone :
        csScan_16_565_scan_pi_flat_gou_znone;
      ScanProcPIG [SCANPROC_PI_FLAT_GOU_ZFIL] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_flat_gou_zfil :
        csScan_16_565_scan_pi_flat_gou_zfil;
      ScanProcPIG [SCANPROC_PI_FLAT_GOU_ZUSE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_flat_gou_zuse :
        csScan_16_565_scan_pi_flat_gou_zuse;
      ScanProcPIG [SCANPROC_PI_FLAT_GOU_ZTEST] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_flat_gou_ztest :
        csScan_16_565_scan_pi_flat_gou_ztest;
      ScanProcPIG [SCANPROC_PI_TEX_GOU_ZNONE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_gou_znone :
        csScan_16_565_scan_pi_tex_gou_znone;
      ScanProcPIG [SCANPROC_PI_TEX_GOU_ZFIL] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_gou_zfil :
        csScan_16_565_scan_pi_tex_gou_zfil;
      ScanProcPIG [SCANPROC_PI_TEX_GOU_ZUSE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_gou_zuse :
        csScan_16_565_scan_pi_tex_gou_zuse;
      ScanProcPIG [SCANPROC_PI_TEX_GOU_ZTEST] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_gou_ztest :
        csScan_16_565_scan_pi_tex_gou_ztest;
      ScanProcPIG [SCANPROC_PI_TEX_GOUKEY_ZNONE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_goukey_znone :
        csScan_16_565_scan_pi_tex_goukey_znone;
      ScanProcPIG [SCANPROC_PI_TEX_GOUKEY_ZFIL] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_goukey_zfil :
        csScan_16_565_scan_pi_tex_goukey_zfil;
      ScanProcPIG [SCANPROC_PI_TEX_GOUKEY_ZUSE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_goukey_zuse :
        csScan_16_565_scan_pi_tex_goukey_zuse;
      ScanProcPIG [SCANPROC_PI_TEX_GOUKEY_ZTEST] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_goukey_ztest :
        csScan_16_565_scan_pi_tex_goukey_ztest;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOU_ZNONE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tile_tex_gou_znone :
        csScan_16_565_scan_pi_tile_tex_gou_znone;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOU_ZFIL] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tile_tex_gou_zfil :
        csScan_16_565_scan_pi_tile_tex_gou_zfil;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOU_ZUSE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tile_tex_gou_zuse :
        csScan_16_565_scan_pi_tile_tex_gou_zuse;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOU_ZTEST] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tile_tex_gou_ztest :
        csScan_16_565_scan_pi_tile_tex_gou_ztest;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUKEY_ZNONE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tile_tex_goukey_znone :
        csScan_16_565_scan_pi_tile_tex_goukey_znone;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUKEY_ZFIL] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tile_tex_goukey_zfil :
        csScan_16_565_scan_pi_tile_tex_goukey_zfil;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUKEY_ZUSE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tile_tex_goukey_zuse :
        csScan_16_565_scan_pi_tile_tex_goukey_zuse;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUKEY_ZTEST] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tile_tex_goukey_ztest :
        csScan_16_565_scan_pi_tile_tex_goukey_ztest;
      ScanProcPIG [SCANPROC_PI_FLAT_GOUFX_ZNONE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_flat_goufx_znone :
        csScan_16_565_scan_pi_flat_goufx_znone;
      ScanProcPIG [SCANPROC_PI_FLAT_GOUFX_ZFIL] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_flat_goufx_zfil :
        csScan_16_565_scan_pi_flat_goufx_zfil;
      ScanProcPIG [SCANPROC_PI_FLAT_GOUFX_ZUSE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_flat_goufx_zuse :
        csScan_16_565_scan_pi_flat_goufx_zuse;
      ScanProcPIG [SCANPROC_PI_FLAT_GOUFX_ZTEST] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_flat_goufx_ztest :
        csScan_16_565_scan_pi_flat_goufx_ztest;
      ScanProcPIG [SCANPROC_PI_TEX_GOUFX_ZNONE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_goufx_znone :
        csScan_16_565_scan_pi_tex_goufx_znone;
      ScanProcPIG [SCANPROC_PI_TEX_GOUFX_ZFIL] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_goufx_zfil :
        csScan_16_565_scan_pi_tex_goufx_zfil;
      ScanProcPIG [SCANPROC_PI_TEX_GOUFX_ZUSE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_goufx_zuse :
        csScan_16_565_scan_pi_tex_goufx_zuse;
      ScanProcPIG [SCANPROC_PI_TEX_GOUFX_ZTEST] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_goufx_ztest :
        csScan_16_565_scan_pi_tex_goufx_ztest;
      ScanProcPIG [SCANPROC_PI_TEX_GOUFXKEY_ZNONE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_goufxkey_znone :
        csScan_16_565_scan_pi_tex_goufxkey_znone;
      ScanProcPIG [SCANPROC_PI_TEX_GOUFXKEY_ZFIL] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_goufxkey_zfil :
        csScan_16_565_scan_pi_tex_goufxkey_zfil;
      ScanProcPIG [SCANPROC_PI_TEX_GOUFXKEY_ZUSE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_goufxkey_zuse :
        csScan_16_565_scan_pi_tex_goufxkey_zuse;
      ScanProcPIG [SCANPROC_PI_TEX_GOUFXKEY_ZTEST] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tex_goufxkey_ztest :
        csScan_16_565_scan_pi_tex_goufxkey_ztest;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUFX_ZNONE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tile_tex_goufx_znone :
        csScan_16_565_scan_pi_tile_tex_goufx_znone;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUFX_ZFIL] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tile_tex_goufx_zfil :
        csScan_16_565_scan_pi_tile_tex_goufx_zfil;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUFX_ZUSE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tile_tex_goufx_zuse :
        csScan_16_565_scan_pi_tile_tex_goufx_zuse;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUFX_ZTEST] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tile_tex_goufx_ztest :
        csScan_16_565_scan_pi_tile_tex_goufx_ztest;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUFXKEY_ZNONE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tile_tex_goufxkey_znone :
        csScan_16_565_scan_pi_tile_tex_goufxkey_znone;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUFXKEY_ZFIL] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tile_tex_goufxkey_zfil :
        csScan_16_565_scan_pi_tile_tex_goufxkey_zfil;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUFXKEY_ZUSE] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tile_tex_goufxkey_zuse :
        csScan_16_565_scan_pi_tile_tex_goufxkey_zuse;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUFXKEY_ZTEST] = (pfmt.GreenBits == 5) ?
        csScan_16_555_scan_pi_tile_tex_goufxkey_ztest :
        csScan_16_565_scan_pi_tile_tex_goufxkey_ztest;
      break;

    case 4:
      if (do_alpha) ScanProc_Alpha = ScanProc_32_Alpha;

      ScanProc [SCANPROC_FLAT_ZNONE] = csScan_32_scan_flat_znone;
      ScanProc [SCANPROC_FLAT_ZFIL] = csScan_32_scan_flat_zfil;
      ScanProc [SCANPROC_FLAT_ZUSE] = csScan_32_scan_flat_zuse;
      ScanProc [SCANPROC_FLAT_ZTEST] = csScan_32_scan_flat_ztest;

      ScanProc [SCANPROC_TEX_ZNONE] = csScan_32_scan_tex_znone;
      ScanProc [SCANPROC_TEX_ZFIL] =
#if defined (DO_MMX)
        UseMMX ? csScan_32_mmx_scan_tex_zfil :
#endif
        csScan_32_scan_tex_zfil;
      ScanProc [SCANPROC_TEX_ZUSE] = csScan_32_scan_tex_zuse;
      ScanProc [SCANPROC_TEX_ZTEST] = csScan_32_scan_tex_ztest;

      ScanProc [SCANPROC_MAP_ZNONE] =
        csScan_32_scan_map_znone;
      ScanProc [SCANPROC_MAP_ZFIL] =
        bilinear_filter == 2 ? csScan_32_scan_map_filt2_zfil :
#if defined (DO_MMX)
        UseMMX ? csScan_32_mmx_scan_map_zfil :
#endif
        csScan_32_scan_map_zfil;
      ScanProc [SCANPROC_MAP_ZUSE] =
        bilinear_filter == 2 ? csScan_32_scan_map_filt2_zuse :
        csScan_32_scan_map_zuse;
      ScanProc [SCANPROC_MAP_ZTEST] =
        csScan_32_scan_map_ztest;

      ScanProc [SCANPROC_TEX_KEY_ZNONE] = csScan_32_scan_tex_key_znone;
      ScanProc [SCANPROC_TEX_KEY_ZFIL] = csScan_32_scan_tex_key_zfil;
      ScanProc [SCANPROC_TEX_KEY_ZUSE] = csScan_32_scan_tex_key_zuse;
      ScanProc [SCANPROC_TEX_KEY_ZTEST] = csScan_32_scan_tex_key_ztest;
      ScanProc [SCANPROC_MAP_KEY_ZNONE] = csScan_32_scan_map_key_znone;
      ScanProc [SCANPROC_MAP_KEY_ZFIL] = csScan_32_scan_map_key_zfil;
      ScanProc [SCANPROC_MAP_KEY_ZUSE] = csScan_32_scan_map_key_zuse;
      ScanProc [SCANPROC_MAP_KEY_ZTEST] = csScan_32_scan_map_key_ztest;

      ScanProc [SCANPROC_TEX_ALPHA_ZNONE] = csScan_32_scan_tex_alpha_znone;
      ScanProc [SCANPROC_TEX_ALPHA_ZFIL] = csScan_32_scan_tex_alpha_zfil;
      ScanProc [SCANPROC_TEX_ALPHA_ZUSE] = csScan_32_scan_tex_alpha_zuse;
      ScanProc [SCANPROC_TEX_ALPHA_ZTEST] = csScan_32_scan_tex_alpha_ztest;
      ScanProc [SCANPROC_MAP_ALPHA_ZNONE] = csScan_32_scan_map_alpha_znone;
      ScanProc [SCANPROC_MAP_ALPHA_ZFIL] = csScan_32_scan_map_alpha_zfil;
      ScanProc [SCANPROC_MAP_ALPHA_ZUSE] = csScan_32_scan_map_alpha_zuse;
      ScanProc [SCANPROC_MAP_ALPHA_ZTEST] = csScan_32_scan_map_alpha_ztest;

      ScanProc [SCANPROC_TEX_FX_ZNONE] = csScan_32_scan_tex_fx_znone;
      ScanProc [SCANPROC_TEX_FX_ZFIL] = csScan_32_scan_tex_fx_zfil;
      ScanProc [SCANPROC_TEX_FX_ZUSE] = csScan_32_scan_tex_fx_zuse;
      ScanProc [SCANPROC_TEX_FX_ZTEST] = csScan_32_scan_tex_fx_ztest;
      ScanProc [SCANPROC_MAP_FX_ZNONE] = csScan_32_scan_map_fx_znone;
      ScanProc [SCANPROC_MAP_FX_ZFIL] = csScan_32_scan_map_fx_zfil;
      ScanProc [SCANPROC_MAP_FX_ZUSE] = csScan_32_scan_map_fx_zuse;
      ScanProc [SCANPROC_MAP_FX_ZTEST] = csScan_32_scan_map_fx_ztest;

      ScanProc [SCANPROC_FOG] = csScan_32_scan_fog;
      ScanProc [SCANPROC_FOG_VIEW] = csScan_32_scan_fog_view;

      ScanProcPI [SCANPROC_PI_FLAT_ZNONE] = csScan_32_scan_pi_flat_znone;
      ScanProcPI [SCANPROC_PI_FLAT_ZFIL] = csScan_32_scan_pi_flat_zfil;
      ScanProcPI [SCANPROC_PI_FLAT_ZUSE] = csScan_32_scan_pi_flat_zuse;
      ScanProcPI [SCANPROC_PI_FLAT_ZTEST] = csScan_32_scan_pi_flat_ztest;
      ScanProcPI [SCANPROC_PI_TEX_ZNONE] = csScan_32_scan_pi_tex_znone;
      ScanProcPI [SCANPROC_PI_TEX_ZFIL] = csScan_32_scan_pi_tex_zfil;
      ScanProcPI [SCANPROC_PI_TEX_ZUSE] = csScan_32_scan_pi_tex_zuse;
      ScanProcPI [SCANPROC_PI_TEX_ZTEST] = csScan_32_scan_pi_tex_ztest;
      ScanProcPI [SCANPROC_PI_TEX_KEY_ZNONE] = csScan_32_scan_pi_tex_key_znone;
      ScanProcPI [SCANPROC_PI_TEX_KEY_ZFIL] = csScan_32_scan_pi_tex_key_zfil;
      ScanProcPI [SCANPROC_PI_TEX_KEY_ZUSE] = csScan_32_scan_pi_tex_key_zuse;
      ScanProcPI [SCANPROC_PI_TEX_KEY_ZTEST] = csScan_32_scan_pi_tex_key_ztest;
      ScanProcPI [SCANPROC_PI_TILE_TEX_ZNONE] = csScan_32_scan_pi_tile_tex_znone;
      ScanProcPI [SCANPROC_PI_TILE_TEX_ZFIL] = csScan_32_scan_pi_tile_tex_zfil;
      ScanProcPI [SCANPROC_PI_TILE_TEX_ZUSE] = csScan_32_scan_pi_tile_tex_zuse;
      ScanProcPI [SCANPROC_PI_TILE_TEX_ZTEST] = csScan_32_scan_pi_tile_tex_ztest;
      ScanProcPI [SCANPROC_PI_TILE_TEX_KEY_ZNONE] = csScan_32_scan_pi_tile_tex_key_znone;
      ScanProcPI [SCANPROC_PI_TILE_TEX_KEY_ZFIL] = csScan_32_scan_pi_tile_tex_key_zfil;
      ScanProcPI [SCANPROC_PI_TILE_TEX_KEY_ZUSE] = csScan_32_scan_pi_tile_tex_key_zuse;
      ScanProcPI [SCANPROC_PI_TILE_TEX_KEY_ZTEST] = csScan_32_scan_pi_tile_tex_key_ztest;
      ScanProcPI [SCANPROC_PI_FLAT_FX_ZNONE] = csScan_32_scan_pi_flat_fx_znone;
      ScanProcPI [SCANPROC_PI_FLAT_FX_ZFIL] = csScan_32_scan_pi_flat_fx_zfil;
      ScanProcPI [SCANPROC_PI_FLAT_FX_ZUSE] = csScan_32_scan_pi_flat_fx_zuse;
      ScanProcPI [SCANPROC_PI_FLAT_FX_ZTEST] = csScan_32_scan_pi_flat_fx_ztest;
      ScanProcPI [SCANPROC_PI_TEX_FX_ZNONE] = csScan_32_scan_pi_tex_fx_znone;
      ScanProcPI [SCANPROC_PI_TEX_FX_ZFIL] = csScan_32_scan_pi_tex_fx_zfil;
      ScanProcPI [SCANPROC_PI_TEX_FX_ZUSE] = csScan_32_scan_pi_tex_fx_zuse;
      ScanProcPI [SCANPROC_PI_TEX_FX_ZTEST] = csScan_32_scan_pi_tex_fx_ztest;
      ScanProcPI [SCANPROC_PI_TEX_FXKEY_ZNONE] = csScan_32_scan_pi_tex_fxkey_znone;
      ScanProcPI [SCANPROC_PI_TEX_FXKEY_ZFIL] = csScan_32_scan_pi_tex_fxkey_zfil;
      ScanProcPI [SCANPROC_PI_TEX_FXKEY_ZUSE] = csScan_32_scan_pi_tex_fxkey_zuse;
      ScanProcPI [SCANPROC_PI_TEX_FXKEY_ZTEST] = csScan_32_scan_pi_tex_fxkey_ztest;
      ScanProcPI [SCANPROC_PI_TILE_TEX_FX_ZNONE] = csScan_32_scan_pi_tile_tex_fx_znone;
      ScanProcPI [SCANPROC_PI_TILE_TEX_FX_ZFIL] = csScan_32_scan_pi_tile_tex_fx_zfil;
      ScanProcPI [SCANPROC_PI_TILE_TEX_FX_ZUSE] = csScan_32_scan_pi_tile_tex_fx_zuse;
      ScanProcPI [SCANPROC_PI_TILE_TEX_FX_ZTEST] = csScan_32_scan_pi_tile_tex_fx_ztest;
      ScanProcPI [SCANPROC_PI_TILE_TEX_FXKEY_ZNONE] = csScan_32_scan_pi_tile_tex_fxkey_znone;
      ScanProcPI [SCANPROC_PI_TILE_TEX_FXKEY_ZFIL] = csScan_32_scan_pi_tile_tex_fxkey_zfil;
      ScanProcPI [SCANPROC_PI_TILE_TEX_FXKEY_ZUSE] = csScan_32_scan_pi_tile_tex_fxkey_zuse;
      ScanProcPI [SCANPROC_PI_TILE_TEX_FXKEY_ZTEST] = csScan_32_scan_pi_tile_tex_fxkey_ztest;
      ScanProcPI [SCANPROC_PI_TEX_ALPHA_ZNONE] = csScan_32_scan_pi_tex_alpha_znone;
      ScanProcPI [SCANPROC_PI_TEX_ALPHA_ZFIL] = csScan_32_scan_pi_tex_alpha_zfil;
      ScanProcPI [SCANPROC_PI_TEX_ALPHA_ZUSE] = csScan_32_scan_pi_tex_alpha_zuse;
      ScanProcPI [SCANPROC_PI_TEX_ALPHA_ZTEST] = csScan_32_scan_pi_tex_alpha_ztest;

      ScanProcPIG [SCANPROC_PI_FLAT_GOU_ZNONE] = csScan_32_scan_pi_flat_gou_znone;
      ScanProcPIG [SCANPROC_PI_FLAT_GOU_ZFIL] = csScan_32_scan_pi_flat_gou_zfil;
      ScanProcPIG [SCANPROC_PI_FLAT_GOU_ZUSE] = csScan_32_scan_pi_flat_gou_zuse;
      ScanProcPIG [SCANPROC_PI_FLAT_GOU_ZTEST] = csScan_32_scan_pi_flat_gou_ztest;
      ScanProcPIG [SCANPROC_PI_TEX_GOU_ZNONE] = csScan_32_scan_pi_tex_gou_znone;
      ScanProcPIG [SCANPROC_PI_TEX_GOU_ZFIL] = csScan_32_scan_pi_tex_gou_zfil;
      ScanProcPIG [SCANPROC_PI_TEX_GOU_ZUSE] = csScan_32_scan_pi_tex_gou_zuse;
      ScanProcPIG [SCANPROC_PI_TEX_GOU_ZTEST] = csScan_32_scan_pi_tex_gou_ztest;
      ScanProcPIG [SCANPROC_PI_TEX_GOUKEY_ZNONE] = csScan_32_scan_pi_tex_goukey_znone;
      ScanProcPIG [SCANPROC_PI_TEX_GOUKEY_ZFIL] = csScan_32_scan_pi_tex_goukey_zfil;
      ScanProcPIG [SCANPROC_PI_TEX_GOUKEY_ZUSE] = csScan_32_scan_pi_tex_goukey_zuse;
      ScanProcPIG [SCANPROC_PI_TEX_GOUKEY_ZTEST] = csScan_32_scan_pi_tex_goukey_ztest;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOU_ZNONE] = csScan_32_scan_pi_tile_tex_gou_znone;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOU_ZFIL] = csScan_32_scan_pi_tile_tex_gou_zfil;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOU_ZUSE] = csScan_32_scan_pi_tile_tex_gou_zuse;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOU_ZTEST] = csScan_32_scan_pi_tile_tex_gou_ztest;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUKEY_ZNONE] = csScan_32_scan_pi_tile_tex_goukey_znone;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUKEY_ZFIL] = csScan_32_scan_pi_tile_tex_goukey_zfil;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUKEY_ZUSE] = csScan_32_scan_pi_tile_tex_goukey_zuse;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUKEY_ZTEST] = csScan_32_scan_pi_tile_tex_goukey_ztest;
      ScanProcPIG [SCANPROC_PI_FLAT_GOUFX_ZNONE] = csScan_32_scan_pi_flat_goufx_znone;
      ScanProcPIG [SCANPROC_PI_FLAT_GOUFX_ZFIL] = csScan_32_scan_pi_flat_goufx_zfil;
      ScanProcPIG [SCANPROC_PI_FLAT_GOUFX_ZUSE] = csScan_32_scan_pi_flat_goufx_zuse;
      ScanProcPIG [SCANPROC_PI_FLAT_GOUFX_ZTEST] = csScan_32_scan_pi_flat_goufx_ztest;
      ScanProcPIG [SCANPROC_PI_TEX_GOUFX_ZNONE] = csScan_32_scan_pi_tex_goufx_znone;
      ScanProcPIG [SCANPROC_PI_TEX_GOUFX_ZFIL] = csScan_32_scan_pi_tex_goufx_zfil;
      ScanProcPIG [SCANPROC_PI_TEX_GOUFX_ZUSE] = csScan_32_scan_pi_tex_goufx_zuse;
      ScanProcPIG [SCANPROC_PI_TEX_GOUFX_ZTEST] = csScan_32_scan_pi_tex_goufx_ztest;
      ScanProcPIG [SCANPROC_PI_TEX_GOUFXKEY_ZNONE] = csScan_32_scan_pi_tex_goufxkey_znone;
      ScanProcPIG [SCANPROC_PI_TEX_GOUFXKEY_ZFIL] = csScan_32_scan_pi_tex_goufxkey_zfil;
      ScanProcPIG [SCANPROC_PI_TEX_GOUFXKEY_ZUSE] = csScan_32_scan_pi_tex_goufxkey_zuse;
      ScanProcPIG [SCANPROC_PI_TEX_GOUFXKEY_ZTEST] = csScan_32_scan_pi_tex_goufxkey_ztest;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUFX_ZNONE] = csScan_32_scan_pi_tile_tex_goufx_znone;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUFX_ZFIL] = csScan_32_scan_pi_tile_tex_goufx_zfil;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUFX_ZUSE] = csScan_32_scan_pi_tile_tex_goufx_zuse;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUFX_ZTEST] = csScan_32_scan_pi_tile_tex_goufx_ztest;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUFXKEY_ZNONE] = csScan_32_scan_pi_tile_tex_goufxkey_znone;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUFXKEY_ZFIL] = csScan_32_scan_pi_tile_tex_goufxkey_zfil;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUFXKEY_ZUSE] = csScan_32_scan_pi_tile_tex_goufxkey_zuse;
      ScanProcPIG [SCANPROC_PI_TILE_TEX_GOUFXKEY_ZTEST] = csScan_32_scan_pi_tile_tex_goufxkey_ztest;
      break;
  } /* endswitch */

  static int o_rbits = -1, o_gbits, o_bbits;
  if ((o_rbits != pfmt.RedBits)
   || (o_gbits != pfmt.GreenBits)
   || (o_bbits != pfmt.BlueBits))
    /// make blending tables
    if(!is_for_procedural_textures) /// if this is not procedural manager
    {
      csScan_CalcBlendTables (Scan.BlendingTable, o_rbits = pfmt.RedBits, 
        o_gbits = pfmt.GreenBits, o_bbits = pfmt.BlueBits);
    }
    else 
    {
      csScan_CalcBlendTables (Scan.BlendingTableProc, o_rbits = pfmt.RedBits, 
        o_gbits = pfmt.GreenBits, o_bbits = pfmt.BlueBits);
    }
}

csDrawScanline* csGraphics3DSoftwareCommon::ScanProc_8_Alpha
  (csGraphics3DSoftwareCommon *This, int alpha)
{
  csAlphaTables *alpha_tables = This->texman->alpha_tables;

  if (alpha < 32)
    return NULL;
  else if (alpha < 96)
  {
    Scan.AlphaMap = alpha_tables->alpha_map25;
    return csScan_8_scan_map_fixalpha2;
  }
  else if (alpha < 160)
  {
    Scan.AlphaMap = alpha_tables->alpha_map50;
    return csScan_8_scan_map_fixalpha1;
  }
  else if (alpha < 224)
  {
    Scan.AlphaMap = alpha_tables->alpha_map25;
    return csScan_8_scan_map_fixalpha1;
  }
  else
    // completely opaque
    return csScan_8_scan_map_zfil;
}

csDrawScanline* csGraphics3DSoftwareCommon::ScanProc_16_Alpha
  (csGraphics3DSoftwareCommon *This, int alpha)
{
  Scan.AlphaMask = This->alpha_mask;
  Scan.AlphaFact = alpha;

  // In 16 bits mode we can get max 32 levels of transparency

  // completely transparent?
  if (alpha <= 256/32)
    return NULL;
  // approximate alpha from 47% to 53% with fast 50% routine
  if ((alpha >= 128 - 256/32) && (alpha <= 128 + 256/32))
    return csScan_16_scan_map_fixalpha50;
  // completely opaque?
  if (alpha >= 256 - 256/32)
    return csScan_16_scan_map_zfil;
  // general case
  if (This->pfmt.GreenBits == 5)
    return csScan_16_555_scan_map_fixalpha;
  // General case
  return csScan_16_565_scan_map_fixalpha;
}

csDrawScanline* csGraphics3DSoftwareCommon::ScanProc_32_Alpha
  (csGraphics3DSoftwareCommon* /*This*/, int alpha)
{
  Scan.AlphaFact = alpha;

  // completely transparent?
  if (alpha <= 1)
    return NULL;
  // for 50% use fast routine
  else if (alpha >= 127 && alpha <= 129)
    return csScan_32_scan_map_fixalpha50;
  // completely opaque?
  else if (alpha >= 254)
    return csScan_32_scan_map_zfil;
  // general case
  return csScan_32_scan_map_fixalpha;
}

void csGraphics3DSoftwareCommon::Close ()
{
  if ((width == height) && (width == -1))
    return;

  int i;
  for (i = 0; i < MAX_INDEXED_FOG_TABLES; i++)
    if (fog_tables [i].table)
      delete [] fog_tables [i].table;

  while (fog_buffers)
  {
    FogBuffer* n = fog_buffers->next;
    delete fog_buffers;
    fog_buffers = n;
  }
  if (!partner)
  { 
    delete tcache; 
    tcache = NULL; 
    texman->Clear();
    texman->DecRef(); texman = NULL;
    vbufmgr->DecRef (); vbufmgr = NULL;
  }
  if (clipper) 
  { 
    clipper->DecRef ();
    clipper = NULL;
    cliptype = CS_CLIPPER_NONE;
  }

  delete [] z_buffer; z_buffer = NULL;
  delete [] smaller_buffer; smaller_buffer = NULL;
  delete [] line_table; line_table = NULL;

  G2D->Close ();
  width = height = -1;
}

void csGraphics3DSoftwareCommon::SetDimensions (int nwidth, int nheight)
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

  delete [] smaller_buffer;
  smaller_buffer = NULL;
  if (do_smaller_rendering)
  {
    smaller_buffer = new UByte [(width*height) * pfmt.PixelBytes];
  }

  delete [] z_buffer;
  z_buffer = new uint32 [width*height];
  z_buf_size = sizeof (uint32)*width*height;

  delete [] line_table;
  line_table = new UByte* [height+1];
}

void csGraphics3DSoftwareCommon::SetClipper (iClipper2D* clip, int cliptype)
{
  if (clip) clip->IncRef ();
  if (clipper) clipper->DecRef ();
  clipper = clip;
  if (!clipper) cliptype = CS_CLIPPER_NONE;
  csGraphics3DSoftwareCommon::cliptype = cliptype;
}

bool csGraphics3DSoftwareCommon::BeginDraw (int DrawFlags)
{
  dpfx_valid = false;

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

void csGraphics3DSoftwareCommon::Print (csRect *area)
{
  G2D->Print (area);
  if (do_interlaced != -1)
    do_interlaced ^= 1;
  if (tcache)
    tcache->frameno++;
}


void csGraphics3DSoftwareCommon::FinishDraw ()
{
  if (DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
    G2D->FinishDraw ();
  DrawMode = 0;
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

inline static void SelectInterpolationStep (float M)
{
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
}

void csGraphics3DSoftwareCommon::DrawPolygonFlat (G3DPolygonDPF& poly)
{
  int i;
  int max_i, min_i;
  float max_y, min_y;
  unsigned char *d;
  uint32 *z_buf;

  if (poly.num < 3) return;

  // Get the plane normal of the polygon. Using this we can calculate
  // '1/z' at every screen space point.
  float Ac, Bc, Cc, Dc, inv_Dc;
  Ac = poly.normal.A ();
  Bc = poly.normal.B ();
  Cc = poly.normal.C ();
  Dc = poly.normal.D ();

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
    // Sometimes double precision in the clipper is not enough. Do an epsilon fuzz
    // so not to reject cases when bounds exceeded by less than epsilon. smgh
    if (((poly.vertices[i].sx + EPSILON) < 0) ||
	((poly.vertices[i].sx - EPSILON) > width))
      return;

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
    if ((ABS (poly.vertices [i].sx - poly.vertices [i - 1].sx)
       + ABS (poly.vertices [i].sy - poly.vertices [i - 1].sy)) > VERTEX_NEAR_THRESHOLD)
      num_vertices++;
    // the above does not catch cases like this:
    // p1   p2   p3
    // x----x----x    a degenerated hexagon :)  norman
    // p6   p5   p4   
  }

  if (((min_y + EPSILON) < 0) ||
      ((max_y - EPSILON) > height))
    return;

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

  csRGBpixel color;
  iTextureHandle *txt_handle = poly.mat_handle->GetTexture ();
  if (txt_handle)
    txt_handle->GetMeanColor (color.red, color.green, color.blue);
  else
    poly.mat_handle->GetFlatColor (color);

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
      Scan.FlatColor = texman->encode_rgb ((color.red * lr) >> 8,
        (color.green * lg) >> 8, (color.blue * lb) >> 8);
    else
      Scan.FlatColor = texman->find_rgb ((color.red * lr) >> 8,
        (color.green * lg) >> 8, (color.blue * lb) >> 8);
  }
  else if (pfmt.PixelBytes >= 2)
    Scan.FlatColor = texman->encode_rgb (color.red, color.green, color.blue);
  else
    Scan.FlatColor = texman->find_rgb (color.red, color.green, color.blue);

  Scan.M = M;

  int alpha = poly.mixmode & CS_FX_MASK_ALPHA;
  // Select the right scanline drawing function.
  if (do_alpha && (alpha || txt_handle->GetKeyColor ()))
    return;
  int scan_index = SCANPROC_FLAT_ZNONE;
  if (z_buf_mode == CS_ZBUF_FILL) scan_index++;
  else if (z_buf_mode == CS_ZBUF_USE) scan_index += 2;
  else if (z_buf_mode == CS_ZBUF_TEST) scan_index += 3;
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

    screenY = height - sy;

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

void csGraphics3DSoftwareCommon::DrawPolygon (G3DPolygonDP& poly)
{
  if (z_buf_mode == CS_ZBUF_FILLONLY)
  {
    DrawFogPolygon (0, poly, CS_FOG_BACK);
    return;
  }

  if (!do_textured || !poly.mat_handle->GetTexture ())
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
  uint32 *z_buf;

  if (poly.num < 3)
    return;

  // Get the plane normal of the polygon. Using this we can calculate
  // '1/z' at every screen space point.
  float Ac, Bc, Cc, Dc, inv_Dc;
  Ac = poly.normal.A ();
  Bc = poly.normal.B ();
  Cc = poly.normal.C ();
  Dc = poly.normal.D ();

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

  // Sometimes double precision in the clipper is not enough. Do an epsilon fuzz
  // so not to reject cases when bounds exceeded by less than epsilon. smgh
  if (((poly.vertices[0].sx + EPSILON) < 0) ||
      ((poly.vertices[0].sx - EPSILON) > width))
    return;

  min_i = max_i = min_z_i = 0;
  min_y = max_y = poly.vertices[0].sy;

  float t = (M == 0.f ? N : N/M);
  bool M_neg = (M<0.f);
  bool M_zero = (M == 0.f);
  min_z = (M_zero ? 0 : poly.vertices[0].sx) + t*poly.vertices[0].sy;

  // count 'real' number of vertices
  int num_vertices = 1;
  for (i = 1 ; i < poly.num ; i++)
  {
    if (((poly.vertices[i].sx + EPSILON) < 0) ||
	((poly.vertices[i].sx - EPSILON) > width))
      return;

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
    float inv_z = (M_zero ? 0 : poly.vertices[i].sx) + t*poly.vertices[i].sy;

    if ((inv_z > min_z) ^ M_neg)
    {
      min_z = inv_z;
      min_z_i = i;
    }
    // theoretically we should do sqrt(dx^2+dy^2) here, but
    // we can approximate it by just abs(dx)+abs(dy)
    if ((ABS (poly.vertices [i].sx - poly.vertices [i - 1].sx)
       + ABS (poly.vertices [i].sy - poly.vertices [i - 1].sy))
       	> VERTEX_NEAR_THRESHOLD)
      num_vertices++;
  }

  if (((min_y + EPSILON) < 0) ||
      ((max_y - EPSILON) > height))
    return;

  min_z = M * (poly.vertices[min_z_i].sx - width2)
        + N * (poly.vertices[min_z_i].sy - height2) + O;

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
  csTextureHandleSoftware *tex_mm =
    (csTextureHandleSoftware *)poly.mat_handle->GetTexture ()
    	->GetPrivateObject ();

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

      float _P1 = P1*m0w, _P2 = P2*m0w, _P3 = P3*m0w, _P4 = P4*m0w - fdu;
      float _Q1 = Q1*m0h, _Q2 = Q2*m0h, _Q3 = Q3*m0h, _Q4 = Q4*m0h - fdv;

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
  csTextureSoftware *txt_unl = (csTextureSoftware *)tex_mm->get_texture (mipmap);

  // Check if polygon has a lightmap (i.e. if it is lighted)
  bool has_lightmap = tex->GetLightMap () && do_lighting;
  if (has_lightmap)
  {
    // If there is a lightmap we check if the size of the lighted
    // texture would not exceed 1000000 bytes. In that case we
    // revert to unlighted texture mapping.
    long size = tex->GetWidth () * tex->GetHeight ();
    if (size > 1000000) has_lightmap = false;
  }

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

  SelectInterpolationStep (M);

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
        float z = M * cx + N * cy + O;			\
        if (ABS (z) > SMALL_EPSILON)			\
        {						\
          z = 1 / z;					\
          float u = (J1 * cx + J2 * cy + J3) * z;	\
          float v = (K1 * cx + K2 * cy + K3) * z;	\
							\
          if (u < u_min) u_min = u;			\
          if (u > u_max) u_max = u;			\
          if (v < v_min) v_min = v;			\
          if (v > v_max) v_max = v;			\
        }						\
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
    tcache->fill_texture (mipmap, tex, tex_mm,  u_min, v_min, u_max, v_max);
  }
  csScan_InitDraw (mipmap, this, tex, tex_mm, txt_unl);

  // Select the right scanline drawing function.
  bool tex_keycolor = tex_mm->GetKeyColor ();
  csDrawScanline* dscan;

  // poly.alpha old way or with mixmode!!!???
  int alpha = poly.mixmode & CS_FX_MASK_ALPHA;
  if (!alpha && poly.mixmode != CS_FX_COPY)
  {
    int scan_index = Scan.bitmap2 ?
    	SCANPROC_MAP_FX_ZNONE :
	SCANPROC_TEX_FX_ZNONE;
    if (z_buf_mode == CS_ZBUF_FILL) scan_index++;
    else if (z_buf_mode == CS_ZBUF_USE) scan_index += 2;
    else if (z_buf_mode == CS_ZBUF_TEST) scan_index += 3;
    dscan = ScanProc [scan_index];
    UInt mode = poly.mixmode;
    Scan.PaletteTable = tex_mm->GetPaletteToGlobal ();
    Scan.TexturePalette = tex_mm->GetColorMap ();
    Scan.PrivateCMap = tex_mm->GetPaletteToGlobal8 ();
    Scan.BlendTable = NULL;
    // array to select blend tables from
    unsigned char **BlendingTable = Scan.BlendingTable; 
    switch (mode & CS_FX_MASK_MIXMODE)
    {
      case CS_FX_ADD:
        Scan.BlendTable = BlendingTable [BLENDTABLE_ADD];
        break;
      case CS_FX_MULTIPLY:
        Scan.BlendTable = BlendingTable [BLENDTABLE_MULTIPLY];
        break;
      case CS_FX_MULTIPLY2:
        Scan.BlendTable = BlendingTable [BLENDTABLE_MULTIPLY2];
        break;
      case CS_FX_ALPHA:
      {
      	// @@@ Can currently not happen!
        if (alpha < 12)
          mode = (mode & ~CS_FX_MASK_MIXMODE) | CS_FX_COPY;
        else if (alpha < 96)
          Scan.BlendTable = BlendingTable [BLENDTABLE_ALPHA25];
        else if (alpha < 160)
          Scan.BlendTable = BlendingTable [BLENDTABLE_ALPHA50];
        else if (alpha < 244)
          Scan.BlendTable = BlendingTable [BLENDTABLE_ALPHA75];
        //else
          // goto zfill_only; @@@ Not supported here!!!
        break;
      }
      case CS_FX_TRANSPARENT:
        //@@@ Not supported!
        break;
    }
  }
  else if (!alpha || !Scan.bitmap2 || !ScanProc_Alpha || !do_alpha)
  {
    int scan_index = Scan.bitmap2 ? SCANPROC_MAP_ZNONE : SCANPROC_TEX_ZNONE;
    if (z_buf_mode == CS_ZBUF_FILL) scan_index++;
    else if (z_buf_mode == CS_ZBUF_USE) scan_index += 2;
    else if (z_buf_mode == CS_ZBUF_TEST) scan_index += 3;
    if (do_alpha)
      if (tex_keycolor)
        scan_index += 4;
      else if ((Scan.AlphaMap = txt_unl->get_alphamap ()))
      {
        scan_index = Scan.bitmap2
		? SCANPROC_MAP_ALPHA_ZNONE
		: SCANPROC_TEX_ALPHA_ZNONE;
        if (z_buf_mode == CS_ZBUF_FILL) scan_index++;
        else if (z_buf_mode == CS_ZBUF_USE) scan_index += 2;
        else if (z_buf_mode == CS_ZBUF_TEST) scan_index += 3;
      }
    dscan = ScanProc [scan_index];
  }
  else
    dscan = ScanProc_Alpha (this, alpha);

  sxL = sxR = dxL = dxR = 0; // Avoid warnings about "uninitialized variables"
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

    screenY = height - sy;

    while (sy > fin_y)
    {
      if ((sy & 1) != do_interlaced)
      {
        // Compute the rounded screen coordinates of horizontal strip
        xL = QRound (sxL);
        xR = QRound (sxR);

        // Sub-pixel U & V correction
        float deltaX = (float)xL - sxL;

        d = line_table [screenY] + (xL << pixel_shift);
        z_buf = z_buffer + width * screenY + xL;

        // Select the right filter depending if we are drawing an odd or
	// even line. This is only used by scan_map_filt_zfil currently and
	// is still experimental.
        if (sy & 1) filter_bf = 3; else filter_bf = 1;

        // do not draw the rightmost pixel - it will be covered
        // by neightbour polygon's left bound
        dscan (xR - xL, d, z_buf,
		inv_z + deltaX * M,
		u_div_z + deltaX * J1,
		v_div_z + deltaX * K1);
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
  ;
//if ((tex_mm->GetFlags () & CS_TEXTURE_PROC) == CS_TEXTURE_PROC)
//  tcache->uncache_texture (0, tex);
}

void csGraphics3DSoftwareCommon::DrawPolygonDebug (G3DPolygonDP& poly)
{
  (void)poly;
}

FogBuffer* csGraphics3DSoftwareCommon::find_fog_buffer (CS_ID id)
{
  FogBuffer* f = fog_buffers;
  while (f)
  {
    if (f->id == id) return f;
    f = f->next;
  }
  return NULL;
}

void csGraphics3DSoftwareCommon::OpenFogObject (CS_ID id, csFog* fog)
{
  FogBuffer* fb = new FogBuffer ();
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

void csGraphics3DSoftwareCommon::CloseFogObject (CS_ID id)
{
  FogBuffer* fb = find_fog_buffer (id);
  if (!fb)
  {
    Report (CS_REPORTER_SEVERITY_BUG,
    	"ENGINE FAILURE! Try to close a non-open fog object!");
    return;
  }
  if (fb->next) fb->next->prev = fb->prev;
  if (fb->prev) fb->prev->next = fb->next;
  else fog_buffers = fb->next;
  delete fb;
}

void csGraphics3DSoftwareCommon::DrawFogPolygon (CS_ID id,
	G3DPolygonDFP& poly, int fog_type)
{
  int i;
  int max_i, min_i;
  float max_y, min_y;
  unsigned char *d;
  uint32 *z_buf;

  if (poly.num < 3)
    return;

  float M = 0, N = 0, O = 0;
  if (fog_type == CS_FOG_FRONT || fog_type == CS_FOG_BACK)
  {
    // Get the plane normal of the polygon. Using this we can calculate
    // '1/z' at every screen space point.
    float Ac, Bc, Cc, Dc, inv_Dc;
    Ac = poly.normal.A ();
    Bc = poly.normal.B ();
    Cc = poly.normal.C ();
    Dc = poly.normal.D ();

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
      inv_Dc = 1 / Dc;
      M = -Ac * inv_Dc * inv_aspect;
      N = -Bc * inv_Dc * inv_aspect;
      O = -Cc * inv_Dc;
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
    if ((ABS (poly.vertices [i].sx - poly.vertices [i - 1].sx)
       + ABS (poly.vertices [i].sy - poly.vertices [i - 1].sy))
       	> VERTEX_NEAR_THRESHOLD)
      num_vertices++;
  }

  // if this is a 'degenerate' polygon, skip it
  if (num_vertices < 3)
    return;

  if (fog_type != CS_FOG_BACK)
  {
    FogBuffer* fb = find_fog_buffer (id);
    if (!fb)
    {
      Report (CS_REPORTER_SEVERITY_BUG,
      	"ENGINE FAILURE! Fog object not open!");
      exit (0);
    }

    Scan.FogDensity = QRound (fb->density * 100);
    if (pfmt.PalEntries == 0)
    {
      Scan.FogR = QRound (fb->red * ((1 << pfmt.RedBits) - 1))
      	<< pfmt.RedShift;
      Scan.FogG = QRound (fb->green * ((1 << pfmt.GreenBits) - 1))
      	<< pfmt.GreenShift;
      Scan.FogB = QRound (fb->blue * ((1 << pfmt.BlueBits) - 1))
      	<< pfmt.BlueShift;

      if (pfmt.PixelBytes == 4)
      {
        // trick: in 32-bit modes set FogR,G,B so that "R" uses bits 16-23,
        // "G" uses bits 8-15 and "B" uses bits 0-7. This is to accomodate
        // different pixel encodings such as RGB, BGR, RBG and so on...
        ULong r = (R8G8B8_SHIFT_ADJUST(pfmt.RedShift) == 16) ? Scan.FogR :
          (R8G8B8_SHIFT_ADJUST(pfmt.GreenShift) == 16) ? Scan.FogG : Scan.FogB;
        ULong g = (R8G8B8_SHIFT_ADJUST(pfmt.RedShift) == 8) ? Scan.FogR :
          (R8G8B8_SHIFT_ADJUST(pfmt.GreenShift) == 8) ? Scan.FogG : Scan.FogB;
        ULong b = (R8G8B8_SHIFT_ADJUST(pfmt.RedShift) == 0) ? Scan.FogR :
          (R8G8B8_SHIFT_ADJUST(pfmt.GreenShift) == 0) ? Scan.FogG : Scan.FogB;
        Scan.FogR = R8G8B8_PIXEL_PREPROC(r);
        Scan.FogG = R8G8B8_PIXEL_PREPROC(g);
        Scan.FogB = R8G8B8_PIXEL_PREPROC(b);
      }
      Scan.FogPix = Scan.FogR | Scan.FogG | Scan.FogB;
    }
    else
    {
      Scan.FogR = QRound (fb->red * 255);
      Scan.FogG = QRound (fb->green * 255);
      Scan.FogB = QRound (fb->blue * 255);
      Scan.Fog8 = BuildIndexedFogTable ();
      Scan.FogPix = texman->find_rgb (Scan.FogR, Scan.FogG, Scan.FogB);
    }
  }

  SelectInterpolationStep (M);

  // Steps for interpolating horizontally accross a scanline.
  Scan.M = M;
  Scan.dM = M*Scan.InterpolStep;

  // Select the right scanline drawing function.
  csDrawScanline* dscan = NULL;
  int scan_index =
  	fog_type == CS_FOG_FRONT ? SCANPROC_FOG :
	fog_type == CS_FOG_BACK ? SCANPROC_ZFIL :
	fog_type == CS_FOG_VIEW ? SCANPROC_FOG_VIEW :
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

  int scanL1, scanL2, scanR1, scanR2; // scan vertex left/right start/final
  float sxL, sxR, dxL, dxR;           // scanline X left/right and deltas
  int sy, fyL, fyR;                   // scanline Y, final Y left, final Y right
  int xL, xR;
  int screenY;

  sxL = sxR = dxL = dxR = 0; // Avoid warnings about "uninitialized variables"
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

    screenY = height - sy;

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
  iMaterialHandle *mat_handle;
  int redFact, greenFact, blueFact;
  int max_r, max_g, max_b;
  int twfp, thfp;
  float tw, th;
  unsigned char *bm;
  int shf_w;
  bool keycolor;
  bool textured;
  bool tiling;
  UInt mixmode;
  csDrawPIScanline *drawline;
  csDrawPIScanlineGouraud *drawline_gouraud;
} pqinfo;

#define EPS   0.0001

void csGraphics3DSoftwareCommon::RealStartPolygonFX (iMaterialHandle* handle,
  UInt mode, bool use_fog)
{
  if (!dpfx_valid ||
  	use_fog != dpfx_use_fog ||
	handle != dpfx_mat_handle ||
	z_buf_mode != dpfx_z_buf_mode ||
	mode != dpfx_mixmode)
  {
    dpfx_valid = true;
    dpfx_use_fog = use_fog;
    dpfx_mat_handle = handle;
    dpfx_z_buf_mode = z_buf_mode;
    dpfx_mixmode = mode;
  }
  else return;

  if (!do_gouraud || !do_lighting)
    mode &= ~CS_FX_GOURAUD;

  pqinfo.mat_handle = handle;

  iTextureHandle *txt_handle = handle ? handle->GetTexture () : NULL;
  if (txt_handle)
  {
    csTextureHandleSoftware *tex_mm = (csTextureHandleSoftware*)txt_handle->GetPrivateObject ();
    csTextureSoftware *txt_unl = (csTextureSoftware *)tex_mm->get_texture (0);
    csScan_InitDrawFX (tex_mm, txt_unl);
    pqinfo.bm = txt_unl->get_bitmap ();
    pqinfo.tw = txt_unl->get_width ();
    pqinfo.th = txt_unl->get_height ();
    pqinfo.shf_w = txt_unl->get_w_shift ();
    pqinfo.twfp = QInt16 (pqinfo.tw) - 1;
    pqinfo.thfp = QInt16 (pqinfo.th) - 1;
    pqinfo.keycolor = tex_mm->GetKeyColor ();
    pqinfo.textured = do_textured;
    pqinfo.tiling = !!(mode & CS_FX_TILING);
  }
  else
    pqinfo.textured = false;

  Scan.AlphaMask = alpha_mask;

  Scan.BlendTable = NULL;
  // array to select blend tables from
  unsigned char **BlendingTable = Scan.BlendingTable; 
  if(is_for_procedural_textures) // proc manager uses its own blend tables
    BlendingTable = Scan.BlendingTableProc;
  pqinfo.drawline = NULL;
  pqinfo.drawline_gouraud = NULL;

  if (pqinfo.textured && Scan.AlphaMap)
  {
    int scan_index = 
      (z_buf_mode == CS_ZBUF_USE) ? SCANPROC_PI_TEX_ALPHA_ZUSE :
      (z_buf_mode == CS_ZBUF_FILL) ? SCANPROC_PI_TEX_ALPHA_ZFIL :
      (z_buf_mode == CS_ZBUF_TEST) ? SCANPROC_PI_TEX_ALPHA_ZTEST :
      SCANPROC_PI_TEX_ALPHA_ZNONE;
    pqinfo.drawline = ScanProcPI [scan_index];
  }
  switch (mode & CS_FX_MASK_MIXMODE)
  {
    case CS_FX_ADD:
      Scan.BlendTable = BlendingTable [BLENDTABLE_ADD];
      break;
    case CS_FX_MULTIPLY:
      Scan.BlendTable = BlendingTable [BLENDTABLE_MULTIPLY];
      break;
    case CS_FX_MULTIPLY2:
      Scan.BlendTable = BlendingTable [BLENDTABLE_MULTIPLY2];
      break;
    case CS_FX_ALPHA:
    {
      int alpha = mode & CS_FX_MASK_ALPHA;
      if (alpha < 12)
        mode = (mode & ~CS_FX_MASK_MIXMODE) | CS_FX_COPY;
      else if (alpha < 96)
        Scan.BlendTable = BlendingTable [BLENDTABLE_ALPHA25];
      else if (alpha < 160)
        Scan.BlendTable = BlendingTable [BLENDTABLE_ALPHA50];
      else if (alpha < 244)
        Scan.BlendTable = BlendingTable [BLENDTABLE_ALPHA75];
      else
        goto zfill_only;
      break;
    }
    case CS_FX_TRANSPARENT:
zfill_only:
      mode &= ~CS_FX_GOURAUD;
      pqinfo.drawline = (z_buf_mode == CS_ZBUF_USE) ? NULL : csScan_scan_pi_zfil;
      break;
    default:
      break;
  }

  // Select draw scanline routines
  int scan_index = pqinfo.textured ? SCANPROC_PI_TEX_ZNONE : SCANPROC_PI_FLAT_ZNONE;
  if (z_buf_mode == CS_ZBUF_FILL) scan_index++;
  else if (z_buf_mode == CS_ZBUF_USE) scan_index += 2;
  else if (z_buf_mode == CS_ZBUF_TEST) scan_index += 3;
  if (pqinfo.textured && pqinfo.keycolor)
    scan_index += 4;
  if ((mode & CS_FX_MASK_MIXMODE) != CS_FX_COPY)
    scan_index += 20;
  if (pqinfo.textured && (mode & CS_FX_TILING))
    scan_index += 8;
  if (!pqinfo.drawline)
    pqinfo.drawline = ScanProcPI [scan_index];
  if (mode & CS_FX_GOURAUD)
    pqinfo.drawline_gouraud = ScanProcPIG [scan_index];

  pqinfo.mixmode = mode;
  // We use #.16 fixed-point format for R,G,B factors
  // where # is the number of bits per component (with the exception of
  // 32bpp modes/textured where we use (#-2).16 format).
  int shift_amount =
    ((pfmt.PixelBytes == 4) && (Scan.BlendTable || pqinfo.textured)) ? 6 : 8;
  pqinfo.redFact   = (((pfmt.RedMask >> pfmt.RedShift) + 1)     << shift_amount) - 1;
  pqinfo.greenFact = (((pfmt.GreenMask >> pfmt.GreenShift) + 1) << shift_amount) - 1;
  pqinfo.blueFact  = (((pfmt.BlueMask >> pfmt.BlueShift) + 1)   << shift_amount) - 1;

  pqinfo.max_r = (1 << (pfmt.RedBits   + shift_amount + 8)) - 1;
  pqinfo.max_g = (1 << (pfmt.GreenBits + shift_amount + 8)) - 1;
  pqinfo.max_b = (1 << (pfmt.BlueBits  + shift_amount + 8)) - 1;
}

void csGraphics3DSoftwareCommon::DrawPolygonFX (G3DPolygonDPFX& poly)
{
  RealStartPolygonFX (poly.mat_handle, poly.mixmode, poly.use_fog);

  if (!pqinfo.drawline && !pqinfo.drawline_gouraud)
    return;

  // Determine the R/G/B of the polygon's flat color
  if (pqinfo.textured)
    Scan.FlatRGB.Set (255, 255, 255);
  else
  {
    if (pqinfo.mat_handle)
      pqinfo.mat_handle->GetFlatColor (Scan.FlatRGB);
    else
      Scan.FlatRGB.Set (poly.flat_color_r, poly.flat_color_g, poly.flat_color_b);
  }

  // Get the same value as a pixel-format-encoded value
  if (pfmt.PixelBytes >= 2)
    Scan.FlatColor = texman->encode_rgb (Scan.FlatRGB.red, Scan.FlatRGB.green,
      Scan.FlatRGB.blue);
  else
    Scan.FlatColor = texman->find_rgb (Scan.FlatRGB.red, Scan.FlatRGB.green,
      Scan.FlatRGB.blue);

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
    rr[i] = poly.vertices [i].r * pqinfo.redFact   * Scan.FlatRGB.red;
    if (poly.vertices [i].g > 2.0) poly.vertices [i].g = 2.0;
    if (poly.vertices [i].g < 0.0) poly.vertices [i].g = 0.0;
    gg[i] = poly.vertices [i].g * pqinfo.greenFact * Scan.FlatRGB.green;
    if (poly.vertices [i].b > 2.0) poly.vertices [i].b = 2.0;
    if (poly.vertices [i].b < 0.0) poly.vertices [i].b = 0.0;
    bb[i] = poly.vertices [i].b * pqinfo.blueFact  * Scan.FlatRGB.blue;
    if (poly.vertices [i].sy > top_y)
      top_y = poly.vertices [top = i].sy;
    if (poly.vertices [i].sy < bot_y)
      bot_y = poly.vertices [bot = i].sy;
  }

  // If the polygon exceeds the screen, it is an engine failure
  if (((bot_y + EPSILON) < 0) ||
      ((top_y - EPSILON) > height))
    return;

  //-----
  // Scan from top to bottom.
  // The following structure contains all the data for one side
  // of the scanline conversion. 'L' is responsible for the left
  // side, 'R' for the right side respectively.
  //-----
  struct
  {
    // Start and final vertex number
    char sv, fv;
    // The X coordinates and its per-scanline delta; also the final Y coordinate
    int x, dxdy, fy;
    // The `U/V/Z' texture coordinates and their per-scanline delta
    int u, dudy, v, dvdy, z, dzdy;
    // The `R/G/B' colors and their per-scanline delta
    int r, drdy, g, dgdy, b, dbdy;
  } L,R;

// Start of code to stop MSVC bitching about uninitialized variables
  L.sv = R.sv = top;
  L.fv = R.fv = top;
  int sy = L.fy = R.fy = QRound (poly.vertices [top].sy);

  L.x = R.x = 0;
  L.dxdy = R.dxdy = 0;

  L.u = R.u = 0;
  L.dudy = R.dudy = 0;
  L.v = R.v = 0;
  L.dvdy = R.dvdy = 0;
  L.z = R.z = 0;
  L.dzdy = R.dzdy = 0;

  L.r = R.r = 0;
  L.drdy = R.drdy = 0;
  L.g = R.g = 0;
  L.dgdy = R.dgdy = 0;
  L.b = R.b = 0;
  L.dbdy = R.dbdy = 0;
// End of MSVC specific code

  // Decide whenever we should use Gouraud or flat (faster) routines
  bool do_gouraud = (pqinfo.drawline_gouraud != NULL)
    && (pqinfo.mixmode & CS_FX_GOURAUD);

  //-----
  // Main scanline loop.
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
      if (sy <= R.fy)
      {
        // Check first if polygon has been finished
        if (R.fv == bot)
          return;
        R.sv = R.fv;
	if (++R.fv >= poly.num)
	  R.fv = 0;

        leave = false;
	R.fy = QRound (poly.vertices [R.fv].sy);
	if (sy <= R.fy)
	  continue;

        float dyR = poly.vertices [R.sv].sy - poly.vertices [R.fv].sy;
        if (dyR)
        {
          float inv_dyR = 1 / dyR;
          R.x = QInt16 (poly.vertices [R.sv].sx);
          R.dxdy = QInt16 (
            (poly.vertices [R.fv].sx - poly.vertices [R.sv].sx) * inv_dyR);
          R.dzdy = QInt24 ((iz [R.fv] - iz [R.sv]) * inv_dyR);
          if (pqinfo.textured)
          {
            R.dudy = QInt16 ((uu [R.fv] - uu [R.sv]) * inv_dyR);
            R.dvdy = QInt16 ((vv [R.fv] - vv [R.sv]) * inv_dyR);
          }
          if (pqinfo.mixmode & CS_FX_GOURAUD)
          {
            R.drdy = QRound ((rr [R.fv] - rr [R.sv]) * inv_dyR);
            R.dgdy = QRound ((gg [R.fv] - gg [R.sv]) * inv_dyR);
            R.dbdy = QRound ((bb [R.fv] - bb [R.sv]) * inv_dyR);
          }

          // horizontal pixel correction
          float deltaX = (R.dxdy * 1/65536.) *
            (poly.vertices [R.sv].sy - (float (sy) - 0.5));
          R.x += QInt16 (deltaX);

          // apply sub-pixel accuracy factor
          float Factor;
          if (poly.vertices [R.fv].sx != poly.vertices [R.sv].sx)
            Factor = deltaX / (poly.vertices [R.fv].sx - poly.vertices [R.sv].sx);
          else
            Factor = 0;
          if (pqinfo.textured)
          {
            R.u = QInt16 (uu [R.sv] + (uu [R.fv] - uu [R.sv]) * Factor);
            R.v = QInt16 (vv [R.sv] + (vv [R.fv] - vv [R.sv]) * Factor);
          }
          R.z = QInt24 (iz [R.sv] + (iz [R.fv] - iz [R.sv]) * Factor);
          if (pqinfo.mixmode & CS_FX_GOURAUD)
          {
            R.r = QRound (rr [R.sv] + (rr [R.fv] - rr [R.sv]) * Factor);
            R.g = QRound (gg [R.sv] + (gg [R.fv] - gg [R.sv]) * Factor);
            R.b = QRound (bb [R.sv] + (bb [R.fv] - bb [R.sv]) * Factor);
          }
        } /* endif */
      } /* endif */
      if (sy <= L.fy)
      {
        if (L.fv == bot)
          return;
        L.sv = L.fv;
	if (--L.fv < 0)
	  L.fv = poly.num - 1;

        leave = false;
	L.fy = QRound (poly.vertices [L.fv].sy);
	if (sy <= L.fy)
	  continue;

        float dyL = poly.vertices [L.sv].sy - poly.vertices [L.fv].sy;
        if (dyL)
        {
          float inv_dyL = 1 / dyL;
          L.x = QInt16 (poly.vertices [L.sv].sx);
          L.dxdy = QInt16 (
            (poly.vertices [L.fv].sx - poly.vertices [L.sv].sx) * inv_dyL);
          L.dzdy = QInt24 ((iz [L.fv] - iz [L.sv]) * inv_dyL);
          if (pqinfo.textured)
          {
            L.dudy = QInt16 ((uu [L.fv] - uu [L.sv]) * inv_dyL);
            L.dvdy = QInt16 ((vv [L.fv] - vv [L.sv]) * inv_dyL);
          }
          if (pqinfo.mixmode & CS_FX_GOURAUD)
          {
            L.drdy = QRound ((rr [L.fv] - rr [L.sv]) * inv_dyL);
            L.dgdy = QRound ((gg [L.fv] - gg [L.sv]) * inv_dyL);
            L.dbdy = QRound ((bb [L.fv] - bb [L.sv]) * inv_dyL);
          }

          // horizontal pixel correction
          float deltaX = (L.dxdy * 1/65536.) *
            (poly.vertices [L.sv].sy - (float (sy) - 0.5));
          L.x += QInt16 (deltaX);

          // apply sub-pixel accuracy factor
          float Factor;
          if (poly.vertices [L.fv].sx != poly.vertices [L.sv].sx)
            Factor = deltaX / (poly.vertices [L.fv].sx - poly.vertices [L.sv].sx);
          else
            Factor = 0;
          if (pqinfo.textured)
          {
            L.u = QInt16 (uu [L.sv] + (uu [L.fv] - uu [L.sv]) * Factor);
            L.v = QInt16 (vv [L.sv] + (vv [L.fv] - vv [L.sv]) * Factor);
          }
          L.z = QInt24 (iz [L.sv] + (iz [L.fv] - iz [L.sv]) * Factor);
          if (pqinfo.mixmode & CS_FX_GOURAUD)
          {
            L.r = QRound (rr [L.sv] + (rr [L.fv] - rr [L.sv]) * Factor);
            L.g = QRound (gg [L.sv] + (gg [L.fv] - gg [L.sv]) * Factor);
            L.b = QRound (bb [L.sv] + (bb [L.fv] - bb [L.sv]) * Factor);
          }
        } /* endif */
      } /* endif */
    } while (!leave); /* enddo */

    //-----
    // Now draw a trapezoid.
    //-----
    int fin_y;
    if (L.fy > R.fy)
      fin_y = L.fy;
    else
      fin_y = R.fy;

    int screenY = height - sy;
    while (sy > fin_y)
    {
      if ((sy & 1) != do_interlaced)
      {
        //-----
        // Draw one scanline.
        //-----
        int xl = round16 (L.x);
        int xr = round16 (R.x);

        if (xr > xl)
        {
          int l = xr - xl;
          float inv_l = 1. / l;

          int dzz = QRound ((R.z - L.z) * inv_l);
          int uu = 0, duu = 0, vv = 0, dvv = 0;
          if (pqinfo.textured)
          {
            int span_u = R.u - L.u;
            int span_v = R.v - L.v;
            uu = L.u; duu = QInt (span_u * inv_l);
            vv = L.v; dvv = QInt (span_v * inv_l);

	    if (!pqinfo.tiling)
	    {
              // Check for texture overflows
              if (uu < 0) uu = 0; if (uu > pqinfo.twfp) uu = pqinfo.twfp;
              if (vv < 0) vv = 0; if (vv > pqinfo.thfp) vv = pqinfo.thfp;

              int tmpu = uu + span_u;
              if (tmpu < 0 || tmpu > pqinfo.twfp)
              {
                if (tmpu < 0) tmpu = 0; if (tmpu > pqinfo.twfp) tmpu = pqinfo.twfp;
                duu = QInt ((tmpu - uu) * inv_l);
              }
              int tmpv = vv + span_v;
              if (tmpv < 0 || tmpv > pqinfo.thfp)
              {
                if (tmpv < 0) tmpv = 0; if (tmpv > pqinfo.thfp) tmpv = pqinfo.thfp;
                dvv = QInt ((tmpv - vv) * inv_l);
              }
	    }
          }

          // R,G,B brightness can underflow due to subpixel correction
          // Underflow will cause visual artifacts while small overflows
          // will be neutralized by our "clamp to 1.0" circuit.
          int rr = 0, drr = 0, gg = 0, dgg = 0, bb = 0, dbb = 0;
          bool clamp = false;
          if (pqinfo.mixmode & CS_FX_GOURAUD)
          {
            int span_r = R.r - L.r;
            int span_g = R.g - L.g;
            int span_b = R.b - L.b;
            rr = L.r, drr = QInt (span_r * inv_l);
            gg = L.g, dgg = QInt (span_g * inv_l);
            bb = L.b, dbb = QInt (span_b * inv_l);

            if (rr < 0) rr = 0;
            if (gg < 0) gg = 0;
            if (bb < 0) bb = 0;

            int tmp = rr + span_r;
            if (tmp < 0) drr = - QInt (rr * inv_l);
            clamp |= (rr > pqinfo.max_r) || (tmp > pqinfo.max_r);
            tmp = gg + span_g;
            if (tmp < 0) dgg = - QInt (gg * inv_l);
            clamp |= (gg > pqinfo.max_g) || (tmp > pqinfo.max_g);
            tmp = bb + span_b;
            if (tmp < 0) dbb = - QInt (bb * inv_l);
            clamp |= (bb > pqinfo.max_b) || (tmp > pqinfo.max_b);
          }

          uint32 *zbuff = z_buffer + width * screenY + xl;
          unsigned char *dest = line_table [screenY] + (xl << pixel_shift);

          if (do_gouraud)
            pqinfo.drawline_gouraud (dest, l, zbuff, uu, duu, vv, dvv,
              L.z, dzz, pqinfo.bm, pqinfo.shf_w, rr, gg, bb, drr, dgg, dbb, clamp);
          else
            pqinfo.drawline (dest, l, zbuff, uu, duu, vv, dvv,
              L.z, dzz, pqinfo.bm, pqinfo.shf_w);
        }
      }

      L.x += L.dxdy; R.x += R.dxdy;
      L.z += L.dzdy; R.z += R.dzdy;
      if (pqinfo.textured)
        L.u += L.dudy, L.v += L.dvdy,
        R.u += R.dudy, R.v += R.dvdy;
      if (pqinfo.mixmode & CS_FX_GOURAUD)
        L.r += L.drdy, L.g += L.dgdy, L.b += L.dbdy,
        R.r += R.drdy, R.g += R.dgdy, R.b += R.dbdy;

      sy--;
      screenY++;
    }
  }
}

void csGraphics3DSoftwareCommon::DrawTriangleMesh (G3DTriangleMesh& mesh)
{
  DefaultDrawTriangleMesh (mesh, this, o2c, clipper,
	cliptype, aspect, width2, height2);
}

unsigned char *csGraphics3DSoftwareCommon::BuildIndexedFogTable ()
{
  static int usage = 0;
  usage++;

  // first look if a fog table with given R,G,B has already been built
  int i, j;
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
    for (j = 1; j <= 32; j++)
      dest [(j - 1) * 256 + i] = texman->find_rgb (
        Scan.FogR + (int (j * (r - Scan.FogR)) >> 5),
        Scan.FogG + (int (j * (g - Scan.FogG)) >> 5),
        Scan.FogB + (int (j * (b - Scan.FogB)) >> 5));
  }

  fog_tables [fi].lastuse = usage;
  return fog_tables [fi].table;
}

bool csGraphics3DSoftwareCommon::SetRenderState (G3D_RENDERSTATEOPTION op,
  long value)
{
  switch (op)
  {
    case G3DRENDERSTATE_ZBUFFERMODE:
      z_buf_mode = csZBufMode (value);
      break;
    case G3DRENDERSTATE_DITHERENABLE:
      texman->dither_textures = value;
      break;
    case G3DRENDERSTATE_BILINEARMAPPINGENABLE:
      bilinear_filter = value ? 1 : 0;
      ScanSetup ();
      break;
    case G3DRENDERSTATE_TRILINEARMAPPINGENABLE:
      bilinear_filter = value ? 2 : 0;
      ScanSetup ();
      break;
    case G3DRENDERSTATE_TRANSPARENCYENABLE:
      do_alpha = value;
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
      CS_ASSERT (G2D);
      if (!value)
      {
	G2D->DoubleBuffer (true);
	do_interlaced = -1;
      }
      else
      {
        if (!G2D->DoubleBuffer (false))
          return false;
        do_interlaced = 0;
      }
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
      do_gouraud = value;
      break;
    case G3DRENDERSTATE_GAMMACORRECTION:
      CS_ASSERT (texman);
      texman->SetGamma ((Gamma = value) / 65536.);
      if (tcache) tcache->Clear ();
      break;
    default:
      return false;
  }

  return true;
}

long csGraphics3DSoftwareCommon::GetRenderState(G3D_RENDERSTATEOPTION op)
{
  switch (op)
  {
    case G3DRENDERSTATE_ZBUFFERMODE:
      return z_buf_mode;
    case G3DRENDERSTATE_DITHERENABLE:
      return texman->dither_textures;
    case G3DRENDERSTATE_BILINEARMAPPINGENABLE:
      return bilinear_filter == 1 ? 1 : 0;
    case G3DRENDERSTATE_TRILINEARMAPPINGENABLE:
      return bilinear_filter == 2 ? 1 : 0;
    case G3DRENDERSTATE_TRANSPARENCYENABLE:
      return do_alpha;
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
      return do_gouraud;
    case G3DRENDERSTATE_GAMMACORRECTION:
      return Gamma;
    default:
      return 0;
  }
}

void csGraphics3DSoftwareCommon::ClearCache()
{
  if (tcache) tcache->Clear ();
}

void csGraphics3DSoftwareCommon::RemoveFromCache (iPolygonTexture* poly_texture)
{
  if (tcache)
  {
    tcache->uncache_texture (0, poly_texture);
    tcache->uncache_texture (1, poly_texture);
    tcache->uncache_texture (2, poly_texture);
    tcache->uncache_texture (3, poly_texture);
  }
}

void csGraphics3DSoftwareCommon::DumpCache()
{
  if (tcache) tcache->dump (this);
}

void csGraphics3DSoftwareCommon::DrawLine (const csVector3& v1, const csVector3& v2,
	float fov, int color)
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
  int py1 = height - QInt (y1 * iz1 + (height/2));
  float iz2 = fov/z2;
  int px2 = QInt (x2 * iz2 + (width/2));
  int py2 = height - QInt (y2 * iz2 + (height/2));

  G2D->DrawLine (px1, py1, px2, py2, color);
}

float csGraphics3DSoftwareCommon::GetZBuffValue (int x, int y)
{
  unsigned long zbf = z_buffer [x + y * width];
  if (!zbf) return 1000000000.;
  return 16777216.0 / float (zbf);
}
