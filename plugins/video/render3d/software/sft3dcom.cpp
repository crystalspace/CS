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

#include "cssysdef.h"

#include "csqint.h"

#include "csgeom/math.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/plane3.h"
#include "csgeom/poly2d.h"
#include "csgeom/polyclip.h"
#include "csgeom/transfrm.h"
#include "csgeom/tri.h"
#include "csgfx/renderbuffer.h"
#include "cstool/rbuflock.h"
#include "csutil/cscolor.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/event.h"
#include "csutil/scfstrset.h"
#include "csutil/sysfunc.h"
#include "iutil/cfgfile.h"
#include "iutil/cmdline.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"
#include "ivideo/graph2d.h"
#include "ivideo/rendermesh.h"

#include "scan.h"
#include "sft3dcom.h"
#include "soft_txt.h"
#include "tcache.h"

#include "clipper.h"
#include "clip_znear.h"
#include "clip_iclipper.h"

#if defined (CS_HAVE_MMX)
#  include "plugins/video/render3d/software/i386/cpuid.h"
#endif

namespace cspluginSoft3d
{

int csSoftwareGraphics3DCommon::filter_bf = 1;

#include "scanindex.h"

///---------------------------------------------------------------------------
SCF_IMPLEMENT_IBASE(csSoftwareGraphics3DCommon)
  SCF_IMPLEMENTS_INTERFACE(iGraphics3D)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSoftwareGraphics3DCommon::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSoftwareGraphics3DCommon::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

csSoftwareGraphics3DCommon::csSoftwareGraphics3DCommon (iBase* parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);

  tcache = 0;
  scfiEventHandler = 0;
  texman = 0;
  //vbufmgr = 0;
  partner = 0;
  clipper = 0;
  cliptype = CS_CLIPPER_NONE;
  do_near_plane = false;

#ifdef CS_HAVE_MMX
  do_mmx = true;
#endif
  do_lighting = true;
  do_alpha = true;
  do_textured = true;
  do_interlaced = -1;
  ilace_fastmove = false;
  bilinear_filter = 0;
  do_smaller_rendering = false;
  smaller_buffer = 0;
  pixel_shift = 0;
  rstate_mipmap = 0;
  do_gouraud = true;

  dbg_max_polygons_to_draw = 2000000000; // After 2 billion polygons we give up :-)

  z_buffer = 0;
  line_table = 0;

  Caps.CanClip = true;
  Caps.minTexHeight = 2;
  Caps.minTexWidth = 2;
  Caps.maxTexHeight = 1024;
  Caps.maxTexWidth = 1024;
  Caps.MaxAspectRatio = 32768;
  Caps.NeedsPO2Maps = true;
  Caps.SupportsPointSprites = false;
  Caps.DestinationAlpha = false;
  Caps.StencilShadows = false;

  width = height = -1;
  partner = 0;
  is_for_procedural_textures = false;

  dpfx_valid = false;

  object_reg = 0;

  memset (activebuffers, 0, sizeof (activebuffers));
  activeTex = 0;

  clipportal_dirty = true;
  clipportal_floating = 0;

  scrapIndicesSize = 0;
  scrapVerticesSize = 0;
}

csSoftwareGraphics3DCommon::~csSoftwareGraphics3DCommon ()
{
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
    if (q != 0)
      q->RemoveListener (scfiEventHandler);
    scfiEventHandler->DecRef ();
  }

  Close ();
  if (partner) partner->DecRef ();
  if (clipper)
  {
    clipper->DecRef ();
    clipper = 0;
    cliptype = CS_CLIPPER_NONE;
  }

  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

bool csSoftwareGraphics3DCommon::Initialize (iObjectRegistry* p)
{
  object_reg = p;
  if (!scfiEventHandler)
    scfiEventHandler = new EventHandler (this);
  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (q != 0)
    q->RegisterListener (scfiEventHandler, CSMASK_Broadcast);

  strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.shared.stringset", iStringSet);
  string_vertices = strings->Request ("vertices");
  string_texture_coordinates = strings->Request ("texture coordinates");
  string_normals = strings->Request ("normals");
  string_colors = strings->Request ("colors");
  string_indices = strings->Request ("indices");
  string_texture_diffuse = strings->Request (CS_MATERIAL_TEXTURE_DIFFUSE);
  string_texture_lightmap = strings->Request ("tex lightmap");
  string_world2camera = strings->Request ("world2camera transform");

  return true;
}

bool csSoftwareGraphics3DCommon::HandleEvent (iEvent& Event)
{
  if (Event.Type == csevBroadcast)
    switch (csCommandEventHelper::GetCode(&Event))
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

void csSoftwareGraphics3DCommon::NewInitialize ()
{
  config.AddConfig(object_reg, "/config/soft3d.cfg");
  do_smaller_rendering = config->GetBool ("Video.Software.Smaller", false);
  mipmap_coef = config->GetFloat ("Video.Software.TextureManager.MipmapCoef", 1.3f);
  do_interlaced = config->GetBool ("Video.Software.Interlacing", false) ? 0 : -1;

#ifdef CS_HAVE_MMX
  do_mmx = config->GetBool ("Video.Software.MMX", true);
#endif
}

void csSoftwareGraphics3DCommon::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (object_reg, iReporter));
  if (rep)
    rep->ReportV (severity, "crystalspace.video.software", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

bool csSoftwareGraphics3DCommon::Open ()
{
  if (!G2D->Open ())
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error opening Graphics2D context.");
    // set "not opened" flag
    width = height = -1;
    return false;
  }

  pfmt = *G2D->GetPixelFormat ();

  if (pfmt.PixelBytes == 4)
    pixel_shift = 2;
  else if (pfmt.PixelBytes == 2)
    pixel_shift = 1;
  else
  {
    Report (CS_REPORTER_SEVERITY_ERROR,
    	"8-bit palette mode no longer works in the software renderer!");
    return false;
  }

  DrawMode = 0;
  SetDimensions (G2D->GetWidth (), G2D->GetHeight ());
  z_buf_mode = CS_ZBUF_NONE;

  csRef<iPluginManager> plugin_mgr (
    CS_QUERY_REGISTRY (object_reg, iPluginManager));

  CS_QUERY_REGISTRY_PLUGIN(shadermgr, object_reg,
    "crystalspace.graphics3d.shadermanager", iShaderManager);

  return true;
}

bool csSoftwareGraphics3DCommon::NewOpen ()
{
#if 0//defined (CS_HAVE_MMX)
  int family, features;
  char vendor [13];
  csDetectCPU (&family, vendor, &features);
  /* Higher 686 processors like P4 report higher cpu family, although they're
     still 686 architecture, so limit that until higher architecture follows
     Can someone test on IA64 or Xeon ? */
  if (family > 686) family = 686;
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

  //fog_buffers = 0;

  // Create the texture manager
  texman = new csSoftwareTextureManager (object_reg, this, config);
  texman->SetPixelFormat (pfmt);

  // Create the vertex buffer manager.
  //vbufmgr = new csPolArrayVertexBufferManager (object_reg);

  tcache = new csSoftwareTextureCache (texman);
  const char *cache_size = config->GetStr
        ("Video.Software.TextureManager.Cache", 0);
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

  //SetRenderState (G3DRENDERSTATE_INTERLACINGENABLE, do_interlaced == 0);

  polyrast.Init (pfmt, ScanProcPI, ScanProcPIG, texman, width, height,
    z_buffer, line_table);

  return true;
}

bool csSoftwareGraphics3DCommon::SharedOpen ()
{
  pixel_shift = partner->pixel_shift;
  //fog_buffers = partner->fog_buffers;
  alpha_mask = partner->alpha_mask;
  z_buf_mode = partner->z_buf_mode;
#if defined (CS_HAVE_MMX)
  cpu_mmx = partner->cpu_mmx;
#endif
  texman = partner->texman;
  //vbufmgr = partner->vbufmgr;
  tcache = partner->tcache;
  ScanSetup ();
  //SetRenderState (G3DRENDERSTATE_INTERLACINGENABLE, do_interlaced == 0);
  return true;
}

void csSoftwareGraphics3DCommon::ScanSetup ()
{
  // Select the right scanline drawing functions
  memset (&ScanProc, 0, sizeof (ScanProc));
  memset (&ScanProcPI, 0, sizeof (ScanProcPI));
  memset (&ScanProcPIG, 0, sizeof (ScanProcPIG));
  ScanProc_Alpha = 0;

#ifdef CS_HAVE_MMX
  //bool UseMMX = (cpu_mmx && do_mmx);
#endif

  // Bits-per-pixel independent routine
  ScanProc [SCANPROC_ZFIL] = csScan_scan_zfil;

  switch (pfmt.PixelBytes)
  {
    case 2:
      if (do_alpha) ScanProc_Alpha = ScanProc_16_Alpha;

      ScanProc [SCANPROC_FLAT_ZNONE] = csScan_16_scan_flat_znone;
      ScanProc [SCANPROC_FLAT_ZFIL] = csScan_16_scan_flat_zfil;
      ScanProc [SCANPROC_FLAT_ZUSE] = csScan_16_scan_flat_zuse;
      ScanProc [SCANPROC_FLAT_ZTEST] = csScan_16_scan_flat_ztest;

      ScanProc [SCANPROC_TEX_ZNONE] = csScan_16_scan_tex_znone;
      ScanProc [SCANPROC_TEX_ZFIL] =
/*#ifdef CS_HAVE_MMX
        UseMMX ? csScan_16_mmx_scan_tex_zfil :
#endif*/
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
/*#ifdef CS_HAVE_MMX
        UseMMX ? csScan_16_mmx_scan_map_zfil :
#endif*/
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
/*#ifdef CS_HAVE_MMX
        UseMMX ? csScan_16_mmx_scan_pi_tex_zuse :
#endif*/
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
/*#if defined (CS_HAVE_MMX)
        UseMMX ? csScan_32_mmx_scan_tex_zfil :
#endif*/
        csScan_32_scan_tex_zfil;
      ScanProc [SCANPROC_TEX_ZUSE] = csScan_32_scan_tex_zuse;
      ScanProc [SCANPROC_TEX_ZTEST] = csScan_32_scan_tex_ztest;

      ScanProc [SCANPROC_MAP_ZNONE] =
        csScan_32_scan_map_znone;
      ScanProc [SCANPROC_MAP_ZFIL] =
        bilinear_filter == 2 ? csScan_32_scan_map_filt2_zfil :
/*#if defined (CS_HAVE_MMX)
        UseMMX ? csScan_32_mmx_scan_map_zfil :
#endif*/
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

csDrawScanline* csSoftwareGraphics3DCommon::ScanProc_16_Alpha
  (csSoftwareGraphics3DCommon *This, int alpha, bool keycolor, bool alphamap)
{
  csDrawScanline* const ScanProcs[24] = {
    0, csScan_16_scan_map_fixalpha50, csScan_16_scan_map_zfil, csScan_16_565_scan_map_fixalpha,
    0, csScan_16_scan_map_fixalpha50_key, csScan_16_scan_map_key_zfil, csScan_16_565_scan_map_fixalpha_key,
    0, csScan_16_565_scan_map_fixalpha50_alphamap, csScan_16_565_scan_map_alpha_zfil, csScan_16_565_scan_map_fixalpha_alphamap,
    0, csScan_16_scan_map_fixalpha50, csScan_16_scan_map_zfil, csScan_16_555_scan_map_fixalpha,
    0, csScan_16_scan_map_fixalpha50_key, csScan_16_scan_map_key_zfil, csScan_16_555_scan_map_fixalpha_key,
    0, csScan_16_555_scan_map_fixalpha50_alphamap, csScan_16_555_scan_map_alpha_zfil, csScan_16_555_scan_map_fixalpha_alphamap
  };

  Scan.AlphaMask = This->alpha_mask;
  Scan.AlphaFact = alpha;

  // In 16 bits mode we can get max 32 levels of transparency

  int scanproc = 3;

  // completely transparent?
  if (alpha <= 256/32)
    scanproc = 0;
  // approximate alpha from 47% to 53% with fast 50% routine
  if ((alpha >= 128 - 256/32) && (alpha <= 128 + 256/32))
    scanproc = 1;
  // completely opaque?
  if (alpha >= 256 - 256/32)
    scanproc = 2;
  if (keycolor) 
    scanproc += 4;
  else if (alphamap) 
    scanproc += 8;
  if (This->pfmt.GreenBits == 5)
    scanproc += 12;
  return ScanProcs[scanproc];
}

csDrawScanline* csSoftwareGraphics3DCommon::ScanProc_32_Alpha
  (csSoftwareGraphics3DCommon* /*This*/, int alpha, bool keycolor, bool alphamap)
{
  csDrawScanline* const ScanProcs[12] = {
    0, csScan_32_scan_map_fixalpha50, csScan_32_scan_map_zfil, csScan_32_scan_map_fixalpha,
    0, csScan_32_scan_map_fixalpha50_key, csScan_32_scan_map_key_zfil, csScan_32_scan_map_fixalpha_key,
    0, csScan_32_scan_map_fixalpha50_alphamap, csScan_32_scan_map_alpha_zfil, csScan_32_scan_map_fixalpha_alphamap};

  Scan.AlphaFact = alpha;

  int scanproc = 3;

  // completely transparent?
  if (alpha <= 1)
    scanproc = 0;
  // for 50% use fast routine
  else if (alpha >= 127 && alpha <= 129)
    scanproc = 1;
  // completely opaque?
  else if (alpha >= 254)
    scanproc = 2;
  if (keycolor) 
    scanproc += 4;
  else if (alphamap) 
    scanproc += 8;
  return ScanProcs[scanproc];
}

void csSoftwareGraphics3DCommon::Close ()
{
  if ((width == height) && (width == -1))
    return;

  /*while (fog_buffers)
  {
    FogBuffer* n = fog_buffers->next;
    delete fog_buffers;
    fog_buffers = n;
  }*/
  if (!partner)
  {
    delete tcache;
    tcache = 0;
    texman->Clear();
    texman->DecRef(); texman = 0;
    //vbufmgr->DecRef (); vbufmgr = 0;
  }
  if (clipper)
  {
    clipper->DecRef ();
    clipper = 0;
    cliptype = CS_CLIPPER_NONE;
  }

  delete [] z_buffer; z_buffer = 0;
  delete [] smaller_buffer; smaller_buffer = 0;
  delete [] line_table; line_table = 0;

  G2D->Close ();
  width = height = -1;
}

void csSoftwareGraphics3DCommon::SetDimensions (int nwidth, int nheight)
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
  smaller_buffer = 0;
  if (do_smaller_rendering)
  {
    smaller_buffer = new uint8 [(width*height) * pfmt.PixelBytes];
  }

  delete [] z_buffer;
  z_buffer = new uint32 [width*height];
  z_buf_size = sizeof (uint32)*width*height;

  delete [] line_table;
  line_table = new uint8* [height+1];
}

void csSoftwareGraphics3DCommon::SetClipper (iClipper2D* clip, int cliptype)
{
  if (clip) clip->IncRef ();
  if (clipper) clipper->DecRef ();
  clipper = clip;
  if (!clipper) cliptype = CS_CLIPPER_NONE;
  csSoftwareGraphics3DCommon::cliptype = cliptype;
}

bool csSoftwareGraphics3DCommon::BeginDraw (int DrawFlags)
{
  dpfx_valid = false;

  clipportal_dirty = true;
  clipportal_floating = 0;
  CS_ASSERT (clipportal_stack.Length () == 0);

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

  // Initialize the line table.
  int i;
  for (i = 0 ; i < height ; i++)
    if (do_smaller_rendering)
      line_table[i] = smaller_buffer + ((i*width)*pfmt.PixelBytes);
    else
      line_table[i] = G2D->GetPixelAt (0, i);

  if (render_target)
  {
    int txt_w, txt_h;
    render_target->GetRendererDimensions (txt_w, txt_h);
    if (!rt_cliprectset)
    {
      G2D->GetClipRect (rt_old_minx, rt_old_miny, rt_old_maxx, rt_old_maxy);
      G2D->SetClipRect (-1, -1, txt_w+1, txt_h+1);
      rt_cliprectset = true;
    }

    if (!rt_onscreen)
    {
      int txt_w, txt_h;
      render_target->GetRendererDimensions (txt_w, txt_h);
      csSoftwareTextureHandle* tex_mm = (csSoftwareTextureHandle *)
	    render_target->GetPrivateObject ();
      csSoftwareTexture *tex_0 = (csSoftwareTexture*)(tex_mm->get_texture (0));
      int x, y;
      uint8* bitmap = tex_0->bitmap;
      switch (pfmt.PixelBytes)
      {
	case 2:
	  {
	    uint16* pal2glob = (uint16*)(tex_mm->GetPaletteToGlobal ());
	    for (y = txt_h-1 ; y >= 0 ; y--)
	    {
              uint16* d = (uint16*)line_table[y];
	      for (x = 0 ; x < txt_w ; x++)
	      {
	        uint8 pix = *bitmap++;
		*d++ = pal2glob[pix];
	      }
	    }
	  }
	  break;
	case 4:
	  {
	    uint32* pal2glob = (uint32*)(tex_mm->GetPaletteToGlobal ());
	    for (y = txt_h-1 ; y >= 0 ; y--)
	    {
              uint32* d = (uint32*)line_table[y];
	      for (x = 0 ; x < txt_w ; x++)
	      {
	        uint8 pix = *bitmap++;
		*d++ = pal2glob[pix];
	      }
	    }
	  }
	  break;
      }
      rt_onscreen = true;
    }
  }

  if (DrawFlags & CSDRAW_CLEARZBUFFER)
    memset (z_buffer, 0, z_buf_size);

  if (DrawFlags & CSDRAW_CLEARSCREEN)
    G2D->Clear (0);

  if (DrawFlags & CSDRAW_3DGRAPHICS)
  {
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
              uint16* src = (uint16*)line_table[y];
              uint16* dst1 = (uint16*)G2D->GetPixelAt (0, y+y);
              uint16* dst2 = (uint16*)G2D->GetPixelAt (0, y+y+1);
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
              uint16* src = (uint16*)line_table[y];
              uint16* dst1 = (uint16*)G2D->GetPixelAt (0, y+y);
              uint16* dst2 = (uint16*)G2D->GetPixelAt (0, y+y+1);
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
            uint32* src = (uint32*)line_table[y];
            uint32* dst1 = (uint32*)G2D->GetPixelAt (0, y+y);
            uint32* dst2 = (uint32*)G2D->GetPixelAt (0, y+y+1);
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

void csSoftwareGraphics3DCommon::Print (csRect const* area)
{
  G2D->Print (area);
  if (do_interlaced != -1)
    do_interlaced ^= 1;
  if (tcache)
    tcache->frameno++;
}


void csSoftwareGraphics3DCommon::FinishDraw ()
{
  if (DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
    G2D->FinishDraw ();
  DrawMode = 0;

  if (render_target)
  {
    if (rt_cliprectset)
    {
      rt_cliprectset = false;
      G2D->SetClipRect (rt_old_minx, rt_old_miny, rt_old_maxx, rt_old_maxy);
    }

    if (rt_onscreen)
    {
      rt_onscreen = false;
      int txt_w, txt_h;
      render_target->GetRendererDimensions (txt_w, txt_h);
      csSoftwareTextureHandle* tex_mm = (csSoftwareTextureHandle *)
	    render_target->GetPrivateObject ();
      //tex_mm->DeleteMipmaps ();
      tex_mm->UpdateTexture ();
      csSoftwareTexture *tex_0 = (csSoftwareTexture*)(tex_mm->get_texture (0));
      int x, y;
      uint8* bitmap = tex_0->bitmap;
      switch (pfmt.PixelBytes)
      {
	case 2:
	  {
	    for (y = 0 ; y < txt_h ; y++)
	    {
              uint16* d = (uint16*)line_table[y];
	      for (x = 0 ; x < txt_w ; x++)
	      {
		uint16 pix = *d++;
		uint8 pix8;
		pix8 = (((pix & pfmt.RedMask) >> pfmt.RedShift) >>
			(pfmt.RedBits - 3)) << 5;
		pix8 |= (((pix & pfmt.GreenMask) >> pfmt.GreenShift) >>
			(pfmt.GreenBits - 3)) << 2;
		pix8 |= (((pix & pfmt.BlueMask) >> pfmt.BlueShift) >>
			(pfmt.BlueBits - 2));
		*bitmap++ = pix8;
	      }
	    }
	  }
	  break;
	case 4:
	  {
	    for (y = 0 ; y < txt_h ; y++)
	    {
              uint32* d = (uint32*)line_table[y];
	      for (x = 0 ; x < txt_w ; x++)
	      {
		uint32 pix = *d++;
		uint8 pix8;
		pix8 = (((pix & pfmt.RedMask) >> pfmt.RedShift) >> 5) << 5;
		pix8 |= (((pix & pfmt.GreenMask) >> pfmt.GreenShift) >> 5) << 2;
		pix8 |= (((pix & pfmt.BlueMask) >> pfmt.BlueShift) >> 6);
		*bitmap++ = pix8;
	      }
	    }
	  }
	  break;
      }
    }
  }
  render_target = 0;
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
struct csSft3DCom
{
  int step1, shift1;
  int step2, shift2;
  int step3, shift3;
  int step4, shift4;
};

static csSft3DCom inter_modes[4] =
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

void csSoftwareGraphics3DCommon::DrawPolygonFlat (G3DPolygonDPF& poly)
{
  size_t i;
  size_t max_i, min_i;
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
  if (ABS (Dc) < SMALL_D) Dc = (float)-SMALL_D;
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
  min_y = max_y = poly.vertices[0].y;
  // count 'real' number of vertices
  size_t num_vertices = 1;
  for (i = 1 ; i < poly.num ; i++)
  {
    // Sometimes double precision in the clipper is not enough.
    // Do an epsilon fuzz so not to reject cases when bounds exceeded
    // by less than epsilon.

    // Jorrit: Removed the test below because it causes polygons
    // to disappear.
    //if (((poly.vertices[i].x + EPSILON) < 0) ||
	//((poly.vertices[i].x - EPSILON) > width))
      //return;

    if (poly.vertices[i].y > max_y)
    {
      max_y = poly.vertices[i].y;
      max_i = i;
    }
    else if (poly.vertices[i].y < min_y)
    {
      min_y = poly.vertices[i].y;
      min_i = i;
    }
    // theoretically we should do sqrt(dx^2+dy^2) here, but
    // we can approximate it by just abs(dx)+abs(dy)
    if ((ABS (poly.vertices [i].x - poly.vertices [i - 1].x)
       + ABS (poly.vertices [i].y - poly.vertices [i - 1].y))
       	> VERTEX_NEAR_THRESHOLD)
      num_vertices++;
    // the above does not catch cases like this:
    // p1   p2   p3
    // x----x----x    a degenerated hexagon :)  norman
    // p6   p5   p4
  }

  // Jorrit: Removed the test below because it causes polygons
  // to disappear.
  //if (((min_y + EPSILON) < 0) ||
      //((max_y - EPSILON) > height))
    //return;

  // if this is a 'degenerate' polygon, skip it.
  if (num_vertices < 3)
    return;

  // For debugging: if we reach the maximum number of polygons to draw we
  // simply stop.
  dbg_current_polygon++;
  if (dbg_current_polygon == dbg_max_polygons_to_draw-1)
    return;
  if (dbg_current_polygon >= dbg_max_polygons_to_draw-1)
    return;

/*  if (do_lighting)
  {
    tex = poly.poly_texture;
    lm = tex->GetLightMap ();
  }*/

  csRGBpixel color;
  iTextureHandle *txt_handle = poly.mat->GetTexture ();
  poly.mat->GetFlatColor (color);

  /* @@@
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

    Scan.FlatColor = texman->encode_rgb ((color.red * lr) >> 8,
      (color.green * lg) >> 8, (color.blue * lb) >> 8);
  }
  */
  Scan.FlatColor = texman->encode_rgb (color.red, color.green, color.blue);

  Scan.M = M;

  int alpha = poly.mixmode & CS_FX_MASK_ALPHA;
  // Select the right scanline drawing function.
  if (do_alpha && (alpha || (txt_handle && txt_handle->GetKeyColor ())))
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
  float sxL, sxR, dxL, dxR;             // sl X left/right and deltas
  int sy, fyL, fyR;                     // sl Y, final Y left, final Y right
  int xL, xR;
  int screenY;

  sxL = sxR = dxL = dxR = 0;
  scanL2 = scanR2 = (int)max_i;
  sy = fyL = fyR = csQround (poly.vertices [scanL2].y);

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
        if (scanR2 == (int)min_i)
	  return;
        scanR1 = scanR2;
	if (++scanR2 >= (int)poly.num)
	  scanR2 = 0;

        leave = false;
        fyR = csQround (poly.vertices [scanR2].y);
        if (sy <= fyR)
          continue;

        float dyR = (poly.vertices [scanR1].y - poly.vertices [scanR2].y);
        if (dyR)
        {
          sxR = poly.vertices [scanR1].x;
          dxR = (poly.vertices [scanR2].x - sxR) / dyR;
          // horizontal pixel correction
          sxR += dxR * (poly.vertices [scanR1].y - (float (sy) - 0.5));
        } /* endif */
      } /* endif */
      if (sy <= fyL)
      {
        scanL1 = scanL2;
	if (--scanL2 < 0)
	  scanL2 = (int)poly.num - 1;

        leave = false;
        fyL = csQround (poly.vertices [scanL2].y);
        if (sy <= fyL)
          continue;

        float dyL = (poly.vertices [scanL1].y - poly.vertices [scanL2].y);
        if (dyL)
        {
          sxL = poly.vertices [scanL1].x;
          dxL = (poly.vertices [scanL2].x - sxL) / dyL;
          // horizontal pixel correction
          sxL += dxL * (poly.vertices [scanL1].y - (float (sy) - 0.5));
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
        xL = csQround (sxL);
        xR = csQround (sxR);

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

void csSoftwareGraphics3DCommon::DrawPolygonZFill (G3DPolygonDFP& poly)
{
  size_t i;
  size_t max_i, min_i;
  float max_y, min_y;
  unsigned char *d;
  uint32 *z_buf;

  if (poly.num < 3)
    return;

  float M, N, O;
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

  // Compute the min_y and max_y for this polygon in screen space coordinates.
  // We are going to use these to scan the polygon from top to bottom.
  min_i = max_i = 0;
  min_y = max_y = poly.vertices[0].y;
  // count 'real' number of vertices
  size_t num_vertices = 1;
  for (i = 1 ; i < poly.num ; i++)
  {
    if (poly.vertices[i].y > max_y)
    {
      max_y = poly.vertices[i].y;
      max_i = i;
    }
    else if (poly.vertices[i].y < min_y)
    {
      min_y = poly.vertices[i].y;
      min_i = i;
    }
    // theoretically we should do here sqrt(dx^2+dy^2), but
    // we can approximate it just by abs(dx)+abs(dy)
    if ((ABS (poly.vertices [i].x - poly.vertices [i - 1].x)
       + ABS (poly.vertices [i].y - poly.vertices [i - 1].y))
       	> VERTEX_NEAR_THRESHOLD)
      num_vertices++;
  }

  // if this is a 'degenerate' polygon, skip it
  if (num_vertices < 3)
    return;

  SelectInterpolationStep (M);

  // Steps for interpolating horizontally accross a scanline.
  Scan.M = M;
  Scan.dM = M*Scan.InterpolStep;

  // Select the right scanline drawing function.
  csDrawScanline* dscan = 0;
  int scan_index = SCANPROC_ZFIL;
  if ((scan_index < 0) || !(dscan = ScanProc [scan_index]))
    return;   // Nothing to do.

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
  scanL2 = scanR2 = (int)max_i;
  sy = fyL = fyR = csQround (poly.vertices [scanL2].y);

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
        if (scanR2 == (int)min_i)
          return;
        scanR1 = scanR2;
	if (++scanR2 >= (int)poly.num)
	  scanR2 = 0;

        leave = false;
        fyR = csQround (poly.vertices [scanR2].y);
        if (sy <= fyR)
          continue;

        float dyR = (poly.vertices [scanR1].y - poly.vertices [scanR2].y);
        if (dyR)
        {
          sxR = poly.vertices [scanR1].x;
          dxR = (poly.vertices [scanR2].x - sxR) / dyR;
          // horizontal pixel correction
          sxR += dxR * (poly.vertices [scanR1].y - (float (sy) - 0.5));
        } /* endif */
      } /* endif */
      if (sy <= fyL)
      {
        scanL1 = scanL2;
	if (--scanL2 < 0)
	  scanL2 = (int)poly.num - 1;

        leave = false;
        fyL = csQround (poly.vertices [scanL2].y);
        if (sy <= fyL)
          continue;

        float dyL = (poly.vertices [scanL1].y - poly.vertices [scanL2].y);
        if (dyL)
        {
          sxL = poly.vertices [scanL1].x;
          dxL = (poly.vertices [scanL2].x - sxL) / dyL;
          // horizontal pixel correction
          sxL += dxL * (poly.vertices [scanL1].y - (float (sy) - 0.5));
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
        xL = csQround (sxL);
        xR = csQround (sxR);

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

void csSoftwareGraphics3DCommon::DrawPolygon (G3DPolygonDP& poly)
{
#if 0
  if (z_buf_mode == CS_ZBUF_FILLONLY)
  {
    DrawPolygonZFill (poly);
    return;
  }

  /*if (!do_textured || !poly.mat_handle->GetTexture ())
  {
    DrawPolygonFlat (poly);
    return;
  }*/

  size_t i;
  float P1, P2, P3, P4;
  float Q1, Q2, Q3, Q4;
  size_t max_i, min_i, min_z_i;
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
  if (ABS (Dc) < SMALL_D) Dc = (float)-SMALL_D;
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

  // Jorrit: Removed the test below because it causes polygons
  // to disappear.
  //if (((poly.vertices[0].x + EPSILON) < 0) ||
      //((poly.vertices[0].x - EPSILON) > width))
    //return;

  min_i = max_i = min_z_i = 0;
  min_y = max_y = poly.vertices[0].y;

  float t = (M == 0.f ? N : N/M);
  bool M_neg = (M<0.f);
  bool M_zero = (M == 0.f);
  min_z = (M_zero ? 0 : poly.vertices[0].x) + t*poly.vertices[0].y;

  // count 'real' number of vertices
  size_t num_vertices = 1;
  for (i = 1 ; i < poly.num ; i++)
  {
    // Jorrit: Removed the test below because it causes polygons
    // to disappear.
    //if (((poly.vertices[i].x + EPSILON) < 0) ||
	//((poly.vertices[i].x - EPSILON) > width))
      //return;

    if (poly.vertices[i].y > max_y)
    {
      max_y = poly.vertices[i].y;
      max_i = i;
    }
    else if (poly.vertices[i].y < min_y)
    {
      min_y = poly.vertices[i].y;
      min_i = i;
    }
    float inv_z = (M_zero ? 0 : poly.vertices[i].x) + t*poly.vertices[i].y;

    if ((inv_z > min_z) ^ M_neg)
    {
      min_z = inv_z;
      min_z_i = i;
    }
    // theoretically we should do sqrt(dx^2+dy^2) here, but
    // we can approximate it by just abs(dx)+abs(dy)
    if ((ABS (poly.vertices [i].x - poly.vertices [i - 1].x)
       + ABS (poly.vertices [i].y - poly.vertices [i - 1].y))
       	> VERTEX_NEAR_THRESHOLD)
      num_vertices++;
  }

  // Jorrit: Removed the test below because it causes polygons
  // to disappear.
  //if (((min_y + EPSILON) < 0) ||
      //((max_y - EPSILON) > height))
    //return;

  min_z = M * (poly.vertices[min_z_i].x - width2)
        + N * (poly.vertices[min_z_i].y - height2) + O;

  // if this is a 'degenerate' polygon, skip it.
  if (num_vertices < 3)
    return;

  // For debugging: if we reach the maximum number of polygons to draw
  // we simply stop.
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
  P1 = poly.cam2tex.m_cam2tex->m11;
  P2 = poly.cam2tex.m_cam2tex->m12;
  P3 = poly.cam2tex.m_cam2tex->m13;
  P4 = - (P1 * poly.cam2tex.v_cam2tex->x
        + P2 * poly.cam2tex.v_cam2tex->y
        + P3 * poly.cam2tex.v_cam2tex->z);
  Q1 = poly.cam2tex.m_cam2tex->m21;
  Q2 = poly.cam2tex.m_cam2tex->m22;
  Q3 = poly.cam2tex.m_cam2tex->m23;
  Q4 = - (Q1 * poly.cam2tex.v_cam2tex->x
        + Q2 * poly.cam2tex.v_cam2tex->y
        + Q3 * poly.cam2tex.v_cam2tex->z);

  csPolyTextureMapping* tmapping = poly.texmap; 
  csSoftwareTextureHandle *tex_mm =
    (csSoftwareTextureHandle *)/*poly.mat_handle->GetTexture ()
    	->GetPrivateObject ()*/poly.txt_handle;
  csSoftRendererLightmap* srlm = (csSoftRendererLightmap*)poly.rlm;

  float fdu, fdv;
//  if (tex)
  {
    fdu = tmapping->GetFDU ();
    fdv = tmapping->GetFDV ();
  }
/*  else
  {
    fdu = 0;
    fdv = 0;
  }*/

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
  if ((rstate_mipmap == 1) || (tex_mm->GetFlags() & CS_TEXTURE_NOMIPMAPS))
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

      float x = poly.vertices [min_z_i].x - width2;
      float y = poly.vertices [min_z_i].y - height2;

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
  int shf_u = tmapping->GetShiftU () - mipmap;
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
  csSoftwareTexture *txt_unl = (csSoftwareTexture*)tex_mm->get_texture (
  	mipmap);

  // Check if polygon has a lightmap (i.e. if it is lighted)
  bool has_lightmap = srlm && !poly.do_fullbright && do_lighting;
  if (has_lightmap)
  {
    // If there is a lightmap we check if the size of the lighted
    // texture would not exceed MAX_LIGHTMAP_SIZE pixels. In that case we
    // revert to unlighted texture mapping.
    long size = tmapping->GetLitWidth () * tmapping->GetLitHeight ();
    if (size > MAX_LIGHTMAP_SIZE) has_lightmap = false;
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
  float sxL, sxR, dxL, dxR;             // sl X left/right and deltas
  int sy, fyL, fyR;                     // sl Y, final Y left, final Y right
  int xL, xR;
  int screenY;

  // If we have a lighted polygon, prepare the texture
  if (has_lightmap)
  {
    sxL = sxR = dxL = dxR = 0;
    scanL2 = scanR2 = (int)max_i;
    sy = fyL = fyR = csQround (poly.vertices [scanL2].y);

    // Find the largest texture rectangle that is going to be displayed
    float u_min = +99999999.0f;
    float v_min = +99999999.0f;
    float u_max = -99999999.0f;
    float v_max = -99999999.0f;

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
          if (scanR2 == (int)min_i)
            goto texr_done;
          scanR1 = scanR2;
          if (++scanR2 >= (int)poly.num)
            scanR2 = 0;

          leave = false;
          fyR = csQround (poly.vertices [scanR2].y);
          if (sy <= fyR)
            continue;

          float dyR = (poly.vertices [scanR1].y - poly.vertices [scanR2].y);
          if (dyR)
          {
            sxR = poly.vertices [scanR1].x;
            dxR = (poly.vertices [scanR2].x - sxR) / dyR;
            // horizontal pixel correction
            sxR += dxR * (poly.vertices [scanR1].y - (float (sy) - 0.5));
          }
        }
        if (sy <= fyL)
        {
          scanL1 = scanL2;
          if (--scanL2 < 0)
            scanL2 = (int)poly.num - 1;

          leave = false;
          fyL = csQround (poly.vertices [scanL2].y);
          if (sy <= fyL)
            continue;

          float dyL = (poly.vertices [scanL1].y - poly.vertices [scanL2].y);
          if (dyL)
          {
            sxL = poly.vertices [scanL1].x;
            dxL = (poly.vertices [scanL2].x - sxL) / dyL;
            // horizontal pixel correction
            sxL += dxL * (poly.vertices [scanL1].y - (float (sy) - 0.5));
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
    tcache->fill_texture (mipmap, tmapping, srlm, 
      /*tex, */tex_mm, u_min, v_min, u_max, v_max);   
  }
  csScan_InitDraw (mipmap, this, tmapping, srlm, tex_mm, txt_unl);

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
    uint mode = poly.mixmode;
    Scan.PaletteTable = tex_mm->GetPaletteToGlobal ();
    Scan.TexturePalette = tex_mm->GetColorMap ();
    Scan.BlendTable = 0;
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
    {
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
    }
    dscan = ScanProc [scan_index];
  }
  else
  {
    dscan = ScanProc_Alpha (this, alpha, tex_keycolor, (Scan.AlphaMap = txt_unl->get_alphamap ()));
  }

  if (!dscan) return;

  sxL = sxR = dxL = dxR = 0; // Avoid warnings about "uninitialized variables"
  scanL2 = scanR2 = (int)max_i;
  sy = fyL = fyR = csQround (poly.vertices [scanL2].y);

  if (do_alpha) 
  {
    // cached texture has different coords than original tex.
    Scan.amap_uofs = tmapping->GetIMinU() >> mipmap; 
    Scan.amap_vofs = tmapping->GetIMinV() >> mipmap; 
  }

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
        if (scanR2 == (int)min_i)
          goto finish;
        scanR1 = scanR2;
	if (++scanR2 >= (int)poly.num)
	  scanR2 = 0;

        leave = false;
        fyR = csQround (poly.vertices [scanR2].y);
        if (sy <= fyR)
          continue;

        float dyR = (poly.vertices [scanR1].y - poly.vertices [scanR2].y);
        if (dyR)
        {
          sxR = poly.vertices [scanR1].x;
          dxR = (poly.vertices [scanR2].x - sxR) / dyR;
          // horizontal pixel correction
          sxR += dxR * (poly.vertices [scanR1].y - (float (sy) - 0.5));
        }
      }
      if (sy <= fyL)
      {
        scanL1 = scanL2;
	if (--scanL2 < 0)
	  scanL2 = (int)poly.num - 1;

        leave = false;
        fyL = csQround (poly.vertices [scanL2].y);
        if (sy <= fyL)
          continue;

        float dyL = (poly.vertices [scanL1].y - poly.vertices [scanL2].y);
        if (dyL)
        {
          sxL = poly.vertices [scanL1].x;
          dxL = (poly.vertices [scanL2].x - sxL) / dyL;
          // horizontal pixel correction
          sxL += dxL * (poly.vertices [scanL1].y - (float (sy) - 0.5));
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
        xL = csQround (sxL);
        xR = csQround (sxR);

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
#endif
}

#if 0

void csSoftwareGraphics3DCommon::DrawPolygonDebug (G3DPolygonDP& poly)
{
  (void)poly;
}

FogBuffer* csSoftwareGraphics3DCommon::find_fog_buffer (CS_ID id)
{
  FogBuffer* f = fog_buffers;
  while (f)
  {
    if (f->id == id) return f;
    f = f->next;
  }
  return 0;
}

void csSoftwareGraphics3DCommon::OpenFogObject (CS_ID id, csFog* fog)
{
  FogBuffer* fb = new FogBuffer ();
  fb->next = fog_buffers;
  fb->prev = 0;
  fb->id = id;
  fb->density = fog->density;
  fb->red = fog->red;
  fb->green = fog->green;
  fb->blue = fog->blue;
  if (fog_buffers) fog_buffers->prev = fb;
  fog_buffers = fb;
}

void csSoftwareGraphics3DCommon::CloseFogObject (CS_ID id)
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

void csSoftwareGraphics3DCommon::DrawFogPolygon (CS_ID id,
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
  min_y = max_y = poly.vertices[0].y;
  // count 'real' number of vertices
  int num_vertices = 1;
  for (i = 1 ; i < poly.num ; i++)
  {
    if (poly.vertices[i].y > max_y)
    {
      max_y = poly.vertices[i].y;
      max_i = i;
    }
    else if (poly.vertices[i].y < min_y)
    {
      min_y = poly.vertices[i].y;
      min_i = i;
    }
    // theoretically we should do here sqrt(dx^2+dy^2), but
    // we can approximate it just by abs(dx)+abs(dy)
    if ((ABS (poly.vertices [i].x - poly.vertices [i - 1].x)
       + ABS (poly.vertices [i].y - poly.vertices [i - 1].y))
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
      return;
    }

    Scan.FogDensity = csQround (fb->density * 100);
    Scan.FogR = csQround (fb->red * ((1 << pfmt.RedBits) - 1))
      	<< pfmt.RedShift;
    Scan.FogG = csQround (fb->green * ((1 << pfmt.GreenBits) - 1))
      	<< pfmt.GreenShift;
    Scan.FogB = csQround (fb->blue * ((1 << pfmt.BlueBits) - 1))
      	<< pfmt.BlueShift;

    if (pfmt.PixelBytes == 4)
    {
      // trick: in 32-bit modes set FogR,G,B so that "R" uses bits 16-23,
      // "G" uses bits 8-15 and "B" uses bits 0-7. This is to accomodate
      // different pixel encodings such as RGB, BGR, RBG and so on...
      uint32 r = (R8G8B8_SHIFT_ADJUST(pfmt.RedShift) == 16) ? Scan.FogR :
          (R8G8B8_SHIFT_ADJUST(pfmt.GreenShift) == 16) ? Scan.FogG : Scan.FogB;
      uint32 g = (R8G8B8_SHIFT_ADJUST(pfmt.RedShift) == 8) ? Scan.FogR :
          (R8G8B8_SHIFT_ADJUST(pfmt.GreenShift) == 8) ? Scan.FogG : Scan.FogB;
      uint32 b = (R8G8B8_SHIFT_ADJUST(pfmt.RedShift) == 0) ? Scan.FogR :
          (R8G8B8_SHIFT_ADJUST(pfmt.GreenShift) == 0) ? Scan.FogG : Scan.FogB;
      Scan.FogR = R8G8B8_PIXEL_PREPROC(r);
      Scan.FogG = R8G8B8_PIXEL_PREPROC(g);
      Scan.FogB = R8G8B8_PIXEL_PREPROC(b);
    }
    Scan.FogPix = Scan.FogR | Scan.FogG | Scan.FogB;
  }

  SelectInterpolationStep (M);

  // Steps for interpolating horizontally accross a scanline.
  Scan.M = M;
  Scan.dM = M*Scan.InterpolStep;

  // Select the right scanline drawing function.
  csDrawScanline* dscan = 0;
  int scan_index =
  	fog_type == CS_FOG_FRONT ? SCANPROC_FOG :
	fog_type == CS_FOG_BACK ? SCANPROC_ZFIL :
	fog_type == CS_FOG_VIEW ? SCANPROC_FOG_VIEW :
	-1;

  if ((scan_index < 0) || !(dscan = ScanProc [scan_index]))
    return;   // Nothing to do.

  //@@@ Optimization note! We should have a separate loop for CS_FOG_VIEW
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
  sy = fyL = fyR = csQround (poly.vertices [scanL2].y);

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
        fyR = csQround (poly.vertices [scanR2].y);
        if (sy <= fyR)
          continue;

        float dyR = (poly.vertices [scanR1].y - poly.vertices [scanR2].y);
        if (dyR)
        {
          sxR = poly.vertices [scanR1].x;
          dxR = (poly.vertices [scanR2].x - sxR) / dyR;
          // horizontal pixel correction
          sxR += dxR * (poly.vertices [scanR1].y - (float (sy) - 0.5));
        } /* endif */
      } /* endif */
      if (sy <= fyL)
      {
        scanL1 = scanL2;
	if (--scanL2 < 0)
	  scanL2 = poly.num - 1;

        leave = false;
        fyL = csQround (poly.vertices [scanL2].y);
        if (sy <= fyL)
          continue;

        float dyL = (poly.vertices [scanL1].y - poly.vertices [scanL2].y);
        if (dyL)
        {
          sxL = poly.vertices [scanL1].x;
          dxL = (poly.vertices [scanL2].x - sxL) / dyL;
          // horizontal pixel correction
          sxL += dxL * (poly.vertices [scanL1].y - (float (sy) - 0.5));
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
        xL = csQround (sxL);
        xR = csQround (sxR);

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

void csSoftwareGraphics3DCommon::OpenPortal (G3DPolygonDFP* poly)
{
  (void)poly;
}

void csSoftwareGraphics3DCommon::ClosePortal ()
{
}
#endif

void csSoftwareGraphics3DCommon::OpenPortal (size_t numVertices, 
				 const csVector2* vertices,
				 const csPlane3& normal,
				 bool floating)
{
  csClipPortal* cp = new csClipPortal ();
  cp->poly = new csVector2[numVertices];
  memcpy (cp->poly, vertices, numVertices * sizeof (csVector2));
  cp->num_poly = (int)numVertices;
  cp->normal = normal;
  clipportal_stack.Push (cp);
  clipportal_dirty = true;

  // If we already have a floating portal then we increase the
  // number. Otherwise we start at one.
  if (clipportal_floating)
    clipportal_floating++;
  else if (floating)
    clipportal_floating = 1;
}

void csSoftwareGraphics3DCommon::ClosePortal (bool use_zfill_portal)
{
  if (clipportal_stack.Length () <= 0) return;
  csClipPortal* cp = clipportal_stack.Pop ();

  if (use_zfill_portal)
  {
    SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_FILLONLY);
    static G3DPolygonDP g3dpoly;
    g3dpoly.mixmode = CS_FX_COPY;
    int num_vertices = cp->num_poly;
    g3dpoly.num = num_vertices;
    memcpy (g3dpoly.vertices, cp->poly, num_vertices * sizeof (csVector2));
    g3dpoly.z_value = 10.0;	// @@@ Just a bad guess!
    g3dpoly.normal = cp->normal;
    DrawPolygonZFill (g3dpoly);
  }
  
  delete cp;
  clipportal_dirty = true;
  if (clipportal_floating > 0)
    clipportal_floating--;
}

#if 0
void csSoftwareGraphics3DCommon::DrawTriangleMesh (G3DTriangleMesh& mesh)
{
  iClipper2D* cl;
  if (mesh.clip_portal >= CS_CLIPPER_NONE)
    cl = clipper;
  else
    cl = 0;
  DefaultDrawTriangleMesh (mesh, this, o2c, cl,
	false /*lazyclip*/, aspect, width2, height2);
}

void csSoftwareGraphics3DCommon::DrawPolygonMesh (G3DPolygonMesh& mesh)
{
  iClipper2D* cl;
  if (mesh.clip_portal >= CS_CLIPPER_NONE)
    cl = clipper;
  else
    cl = 0;
  DefaultDrawPolygonMesh (mesh, this, o2c, cl,
	false /*lazyclip*/, aspect, width2, height2);
}

bool csSoftwareGraphics3DCommon::SetRenderState (G3D_RENDERSTATEOPTION op,
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
#ifdef CS_HAVE_MMX
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
    default:
      return false;
  }

  return true;
}

long csSoftwareGraphics3DCommon::GetRenderState(G3D_RENDERSTATEOPTION op)
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
#ifdef CS_HAVE_MMX
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
    default:
      return 0;
  }
}
#endif

void csSoftwareGraphics3DCommon::ClearCache()
{
  if (tcache) tcache->Clear ();
}

void csSoftwareGraphics3DCommon::RemoveFromCache (
	      iRendererLightmap* rlm)
{
  if (tcache)
  {
    csSoftRendererLightmap* srlm = 
      (csSoftRendererLightmap*)rlm;
    tcache->uncache_texture (0, srlm);
    tcache->uncache_texture (1, srlm);
    tcache->uncache_texture (2, srlm);
    tcache->uncache_texture (3, srlm);
  }
}

void csSoftwareGraphics3DCommon::DumpCache()
{
  if (tcache) tcache->dump (this);
}

void csSoftwareGraphics3DCommon::DrawLine (const csVector3& v1,
	const csVector3& v2, float fov, int color)
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
  int px1 = csQint (x1 * iz1 + (width/2));
  int py1 = height - csQint (y1 * iz1 + (height/2));
  float iz2 = fov/z2;
  int px2 = csQint (x2 * iz2 + (width/2));
  int py2 = height - csQint (y2 * iz2 + (height/2));

  G2D->DrawLine (px1, py1, px2, py2, color);
}

float csSoftwareGraphics3DCommon::GetZBuffValue (int x, int y)
{
  unsigned long zbf = z_buffer [x + y * width];
  if (!zbf) return 1000000000.;
  return 16777216.0 / float (zbf);
}

void csSoftwareGraphics3DCommon::SetRenderTarget (iTextureHandle* handle,
	bool persistent)
{
  render_target = handle;
  csSoftwareTextureHandle* tex_mm = (csSoftwareTextureHandle *)
	    render_target->GetPrivateObject ();
  tex_mm->Setup332Palette ();
  // We don't generate mipmaps or so...
  tex_mm->flags |= CS_TEXTURE_NOMIPMAPS;

  rt_onscreen = !persistent;
  rt_cliprectset = false;
}

/*void csSoftwareGraphics3DCommon::DrawMesh (csRenderMesh* mesh)
{
  iRenderBufferSource* source = mesh->buffersource;
  iRenderBuffer* indexbuf = source->GetRenderBuffer (
    strings->Request ("indices"));
  if (!indexbuf)
    return;

  uint32 *indices = (uint32*)indexbuf->Lock (CS_BUF_LOCK_NORMAL);

  do_lighting = false;

  void* locks[16];
  int i=0;
  for (i=0; i<16; ++i)
  {
    if (activebuffers[i] != 0)
      locks[i] = activebuffers[i]->Lock (CS_BUF_LOCK_NORMAL);
  }

  G3DTriangleMesh trimesh;
  trimesh.
  for (i=mesh->indexstart; i<mesh->indexend; i+=3)
  {
    G3DPolygonDPF poly;
    poly.do_fullbright = true;
    poly.mat_handle = mesh->material->GetMaterialHandle();
    poly.mixmode = CS_FX_COPY;
    poly.use_fog = false;
    csVector3 v[3];
    v[0] = csVector3 (((float*)locks[CS_VATTRIB_POSITION])[indices[i]*3],
                  ((float*)locks[CS_VATTRIB_POSITION])[indices[i]*3+1],
                  ((float*)locks[CS_VATTRIB_POSITION])[indices[i]*3+2]);
    v[1] = csVector3 (((float*)locks[CS_VATTRIB_POSITION])[indices[i+1]*3],
                  ((float*)locks[CS_VATTRIB_POSITION])[indices[i+1]*3+1],
                  ((float*)locks[CS_VATTRIB_POSITION])[indices[i+1]*3+2]);
    v[2] = csVector3 (((float*)locks[CS_VATTRIB_POSITION])[indices[i+2]*3],
                  ((float*)locks[CS_VATTRIB_POSITION])[indices[i+2]*3+1],
                  ((float*)locks[CS_VATTRIB_POSITION])[indices[i+2]*3+2]);

    for (int j=0; j<3; j++)
    {
      v[j] *= *mesh->transform;
      float iz = aspect / v[j].z;
      poly.vertices[j].x = v[j].x * iz + width2;
      poly.vertices[j].y = v[j].y * iz + height2;
    }
    poly.num = 3;
    poly.z_value = v[0].z;
    poly.normal = csPlane3(v[0], v[1], v[2]);
    poly.poly_texture = 0;
    z_buf_mode = CS_ZBUF_USE;
    //DrawPolygonFlat (poly);
    DrawTriangle (this, clipper, mesh, poly, 
      poly.vertices[0], poly.vertices[1], poly.vertices[2],)
  }
  
  indexbuf->Release ();
  for (i=0; i<16; ++i)
  {
    if (activebuffers[i] != 0)
      activebuffers[i]->Release ();
  }
}*/

/// The perspective corrected vertices.
typedef csDirtyAccessArray<csVector3> dtmesh_persp;
CS_IMPLEMENT_STATIC_VAR (Get_persp, dtmesh_persp, ())


static dtmesh_persp *persp = 0;

#if 0
static void DoAddPerspective (csPoly2D& dest, const csVector3& v, 
			      float aspect, float shift_x, float shift_y)
{
  float iz = aspect / v.z;
  csVector2 p;
  p.x = v.x * iz + shift_x;
  p.y = v.y * iz + shift_y;
  //dest.Push (p);
  dest.AddVertex (p);
  //dest.GetBoundingBox ().AddBoundingVertex (p);
}

static bool DoPolyPerspective (csVector3* verts, int num_verts, 
			       csPoly2D& dest, /*bool mirror,*/
			       float aspect, float shift_x, float shift_y,
			       const csPlane3& plane_cam)
{
  bool mirror = false;

  csVector3 *ind, *end = verts + num_verts;

  if (num_verts == 0) return false;
  dest.MakeEmpty ();

  // Classify all points as NORMAL (z>=SMALL_Z), NEAR (0<=z<SMALL_Z), or
  // BEHIND (z<0).  Use several processing algorithms: trivially accept if all
  // points are NORMAL, mixed process if some points are NORMAL and some
  // are not, special process if there are no NORMAL points, but some are
  // NEAR.  Assume that the polygon has already been culled if all points
  // are BEHIND.
  // Handle the trivial acceptance case:
  ind = verts;
  while (ind < end)
  {
    if (ind->z >= SMALL_Z)
      DoAddPerspective (dest, *ind, aspect, shift_x, shift_y);
    else
      break;
    ind++;
  }

  // Check if special or mixed processing is required
  if (ind == end) return true;

  // If we are processing a triangle (uv_coords != 0) then
  // we stop here because the triangle is only visible if all
  // vertices are visible (this is not exactly true but it is
  // easier this way! @@@ CHANGE IN FUTURE).

  csVector3 *exit = 0, *exitn = 0, *reenter = 0, *reentern = 0;
  csVector2 *evert = 0;

  if (ind == verts)
  {
    while (ind < end)
    {
      if (ind->z >= SMALL_Z)
      {
        reentern = ind;
        reenter = ind - 1;
        break;
      }

      ind++;
    }
  }
  else
  {
    exit = ind;
    exitn = ind - 1;
    evert = dest.GetLast ();
  }

  // Check if mixed processing is required
  if (exit || reenter)
  {
    bool needfinish = false;

    if (exit)
    {
      // we know where the polygon is no longer NORMAL, now we need to

      // to find out on which edge it becomes NORMAL again.
      while (ind < end)
      {
        if (ind->z >= SMALL_Z)
        {
          reentern = ind;
          reenter = ind - 1;
          break;
        }

        ind++;
      }

      if (ind == end)
      {
        reentern = verts;
        reenter = ind - 1;
      }
      else
        needfinish = true;
    } /* if (exit) */
    else
    {
      // we know where the polygon becomes NORMAL, now we need to
      // to find out on which edge it ceases to be NORMAL.
      while (ind < end)
      {
        if (ind->z >= SMALL_Z)
          DoAddPerspective (dest, *ind, aspect, shift_x, shift_y);
        else
        {
          exit = ind;
          exitn = ind - 1;
          break;
        }

        ind++;
      }

      if (ind == end)
      {
        exit = verts;
        exitn = ind - 1;
      }

      evert = dest.GetLast ();
    }

    // Add the NEAR points appropriately.
#define MAX_VALUE 1000000.
    // First, for the exit point.
    float ex, ey, epointx, epointy;
    ex = exitn->z * exit->x - exitn->x * exit->z;
    ey = exitn->z * exit->y - exitn->y * exit->z;
    if (ABS (ex) < SMALL_EPSILON && ABS (ey) < SMALL_EPSILON)
    {
      // Uncommon special case:  polygon passes through origin.
      //plane->WorldToCamera (trans, source[0]);  //@@@ Why is this needed???
      ex = plane_cam.A ();
      ey = plane_cam.B ();
      if (ABS (ex) < SMALL_EPSILON && ABS (ey) < SMALL_EPSILON)
      {
        // Downright rare case:  polygon near parallel with viewscreen.
        ex = exit->x - exitn->x;
        ey = exit->y - exitn->y;
      }
    }

    if (ABS (ex) > ABS (ey))
    {
      if (ex > 0)
        epointx = MAX_VALUE;
      else
        epointx = -MAX_VALUE;
      epointy = (epointx - evert->x) * ey / ex + evert->y;
    }
    else
    {
      if (ey > 0)
        epointy = MAX_VALUE;
      else
        epointy = -MAX_VALUE;
      epointx = (epointy - evert->y) * ex / ey + evert->x;
    }

    // Next, for the reentry point.
    float rx, ry, rpointx, rpointy;

    // Perspective correct the point.
    float iz = aspect / reentern->z;
    csVector2 rvert;
    rvert.x = reentern->x * iz + shift_x;
    rvert.y = reentern->y * iz + shift_y;

    if (reenter == exit && reenter->z > -SMALL_EPSILON)
    {
      rx = ex;
      ry = ey;
    }
    else
    {
      rx = reentern->z * reenter->x - reentern->x * reenter->z;
      ry = reentern->z * reenter->y - reentern->y * reenter->z;
    }

    if (ABS (rx) < SMALL_EPSILON && ABS (ry) < SMALL_EPSILON)
    {
      // Uncommon special case:  polygon passes through origin.
      //plane->WorldToCamera (trans, source[0]);  //@@@ Why is this needed?
      rx = plane_cam.A ();
      ry = plane_cam.B ();
      if (ABS (rx) < SMALL_EPSILON && ABS (ry) < SMALL_EPSILON)
      {
        // Downright rare case:  polygon near parallel with viewscreen.
        rx = reenter->x - reentern->x;
        ry = reenter->y - reentern->y;
      }
    }

    if (ABS (rx) > ABS (ry))
    {
      if (rx > 0)
        rpointx = MAX_VALUE;
      else
        rpointx = -MAX_VALUE;
      rpointy = (rpointx - rvert.x) * ry / rx + rvert.y;
    }
    else
    {
      if (ry > 0)
        rpointy = MAX_VALUE;
      else
        rpointy = -MAX_VALUE;
      rpointx = (rpointy - rvert.y) * rx / ry + rvert.x;
    }

#define QUADRANT(x, y)  ((y < x ? 1 : 0) ^ (x < -y ? 3 : 0))
#define MQUADRANT(x, y) ((y < x ? 3 : 0) ^ (x < -y ? 1 : 0))
    dest.AddVertex (epointx, epointy);
#if EXPERIMENTAL_BUG_FIX
    if (mirror)
    {
      int quad = MQUADRANT (epointx, epointy);
      int rquad = MQUADRANT (rpointx, rpointy);
      if (
        (quad == 0 && -epointx == epointy) ||
        (quad == 1 && epointx == epointy))
        quad++;
      if (
        (rquad == 0 && -rpointx == rpointy) ||
        (rquad == 1 && rpointx == rpointy))
        rquad++;
      while (quad != rquad)
      {
        epointx = float((quad & 2) ? MAX_VALUE : -MAX_VALUE);
        epointy = float((quad == 0 || quad == 3) ? MAX_VALUE : -MAX_VALUE);
        dest.AddVertex (epointx, epointy);
        quad = (quad + 1) & 3;
      }
    }
    else
    {
      int quad = QUADRANT (epointx, epointy);
      int rquad = QUADRANT (rpointx, rpointy);
      if (
        (quad == 0 && epointx == epointy) ||
        (quad == 1 && -epointx == epointy))
        quad++;
      if (
        (rquad == 0 && rpointx == rpointy) ||
        (rquad == 1 && -rpointx == rpointy))
        rquad++;
      while (quad != rquad)
      {
        epointx = float((quad & 2) ? -MAX_VALUE : MAX_VALUE);
        epointy = float((quad == 0 || quad == 3) ? MAX_VALUE : -MAX_VALUE);
        dest.AddVertex (epointx, epointy);
        quad = (quad + 1) & 3;
      }
    }
#endif
    dest.AddVertex (rpointx, rpointy);

    // Add the rest of the vertices, which are all NORMAL points.
    if (needfinish)
      while (ind < end) DoAddPerspective (dest, *ind++,
        aspect, shift_x, shift_y);
  } /* if (exit || reenter) */

  // Do special processing (all points are NEAR or BEHIND)
  else
  {
    if (mirror)
    {
      csVector3 *ind2 = end - 1;
      for (ind = verts; ind < end; ind2 = ind, ind++)
        if (
          (
            ind->x -
            ind2->x
          ) *
              (ind2->y) -
              (ind->y - ind2->y) *
              (ind2->x) > -SMALL_EPSILON)
          return false;
      dest.AddVertex (MAX_VALUE, -MAX_VALUE);
      dest.AddVertex (MAX_VALUE, MAX_VALUE);
      dest.AddVertex (-MAX_VALUE, MAX_VALUE);
      dest.AddVertex (-MAX_VALUE, -MAX_VALUE);
    }
    else
    {
      csVector3 *ind2 = end - 1;
      for (ind = verts; ind < end; ind2 = ind, ind++)
        if (
          (
            ind->x -
            ind2->x
          ) *
              (ind2->y) -
              (ind->y - ind2->y) *
              (ind2->x) < SMALL_EPSILON)
          return false;
      dest.AddVertex (-MAX_VALUE, -MAX_VALUE);
      dest.AddVertex (-MAX_VALUE, MAX_VALUE);
      dest.AddVertex (MAX_VALUE, MAX_VALUE);
      dest.AddVertex (MAX_VALUE, -MAX_VALUE);
    }
  }

  return true;

}
#endif

void csSoftwareGraphics3DCommon::DrawSimpleMesh (const csSimpleRenderMesh &mesh,
						 uint flags)
{
  csRef<csRenderBufferHolder> scrapBufferHolder;
  scrapBufferHolder.AttachNew (new csRenderBufferHolder);

  uint indexCount = mesh.indices ? mesh.indexCount : mesh.vertexCount;
  if (scrapIndicesSize < indexCount)
  {
    scrapIndices = csRenderBuffer::CreateIndexRenderBuffer (indexCount,
      CS_BUF_DYNAMIC, CS_BUFCOMP_UNSIGNED_INT, 0, 0);
    scrapIndicesSize = indexCount;
  }
  if (scrapVerticesSize < mesh.vertexCount)
  {
    scrapVertices = csRenderBuffer::CreateRenderBuffer (
      mesh.vertexCount,
      CS_BUF_DYNAMIC, CS_BUFCOMP_FLOAT, 3);
    scrapTexcoords = csRenderBuffer::CreateRenderBuffer (
      mesh.vertexCount,
      CS_BUF_DYNAMIC, CS_BUFCOMP_FLOAT, 2);
    scrapColors = csRenderBuffer::CreateRenderBuffer (
      mesh.vertexCount,
      CS_BUF_DYNAMIC, CS_BUFCOMP_FLOAT, 4);

    scrapVerticesSize = mesh.vertexCount;
  }

  csShaderVariable* sv;
  sv = scrapContext.GetVariableAdd (strings->Request ("indices"));
  if (mesh.indices)
  {
    scrapIndices->CopyInto (mesh.indices, mesh.indexCount);
  }
  else
  {
    csRenderBufferLock<uint> indexLock (scrapIndices);
    for (uint i = 0; i < mesh.vertexCount; i++)
      indexLock[(size_t)i] = i;
  }
  sv->SetValue (scrapIndices);
  scrapBufferHolder->SetRenderBuffer (CS_BUFFER_INDEX, scrapIndices);

  sv = scrapContext.GetVariableAdd (strings->Request ("vertices"));
  if (mesh.vertices)
  {
    scrapVertices->CopyInto (mesh.vertices, mesh.vertexCount);
    ActivateBuffer (CS_VATTRIB_POSITION, scrapVertices);
    scrapBufferHolder->SetRenderBuffer (CS_BUFFER_POSITION, scrapVertices);
  }
  else
  {
    DeactivateBuffer (CS_VATTRIB_POSITION);
  }
  sv = scrapContext.GetVariableAdd (strings->Request ("texture coordinates"));
  if (mesh.texcoords)
  {
    scrapTexcoords->CopyInto (mesh.texcoords, mesh.vertexCount);
    ActivateBuffer (CS_VATTRIB_TEXCOORD, scrapTexcoords);
    scrapBufferHolder->SetRenderBuffer (CS_BUFFER_TEXCOORD0, scrapTexcoords);
  }
  else
  {
    DeactivateBuffer (CS_VATTRIB_TEXCOORD);
  }
  sv = scrapContext.GetVariableAdd (strings->Request ("colors"));
  if (mesh.colors)
  {
    scrapColors->CopyInto (mesh.colors, mesh.vertexCount);
    scrapBufferHolder->SetRenderBuffer (CS_BUFFER_COLOR, scrapColors);
    ActivateBuffer (CS_VATTRIB_COLOR, scrapColors);
  }
  else
  {
    DeactivateBuffer (CS_VATTRIB_COLOR);
  }

  if (mesh.texture)
    ActivateTexture (mesh.texture);
  else
    DeactivateTexture ();

  csRenderMesh rmesh;
  //rmesh.z_buf_mode = mesh.z_buf_mode;
  rmesh.mixmode = mesh.mixmode;
  rmesh.clip_portal = 0;
  rmesh.clip_plane = 0;
  rmesh.clip_z_plane = 0;
  rmesh.do_mirror = false;
  rmesh.meshtype = mesh.meshtype;
  rmesh.indexstart = 0;
  rmesh.indexend = indexCount;
  rmesh.variablecontext = &scrapContext;
  rmesh.buffers = scrapBufferHolder;

  if (flags & csSimpleMeshScreenspace)
  {
    csReversibleTransform camtrans;

    const float vwf = (float)(width);
    const float vhf = (float)(height);

    camtrans.SetO2T (
      csMatrix3 (1.0f, 0.0f, 0.0f,
      0.0f, -1.0f, 0.0f,
      0.0f, 0.0f, 1.0f));
    camtrans.SetO2TTranslation (csVector3 (
      vwf / 2.0f, vhf / 2.0f, -aspect));

    SetWorldToCamera (camtrans.GetInverse ());
  }

  rmesh.object2world = mesh.object2world;

  csShaderVarStack stacks;
  shadermgr->PushVariables (stacks);
  scrapContext.PushVariables (stacks);

  if (mesh.alphaType.autoAlphaMode)
  {
    csAlphaMode::AlphaType autoMode = csAlphaMode::alphaNone;

    iTextureHandle* tex = 0;

    csShaderVariable* texVar =
      csGetShaderVariableFromStack (stacks, mesh.alphaType.autoModeTexture);

    if (texVar)
      texVar->GetValue (tex);

    if (tex == 0)
      tex = mesh.texture;
    if (tex != 0)
      autoMode = tex->GetAlphaType ();

    rmesh.alphaType = autoMode;
  }
  else
  {
    rmesh.alphaType = mesh.alphaType.alphaType;
  }

  SetZMode (mesh.z_buf_mode);
  DrawMesh (&rmesh, rmesh, stacks);
}

#if 0
void csSoftwareGraphics3DCommon::DrawPolysMesh (const csCoreRenderMesh* mesh,
    const csRenderMeshModes& modes,
    const csArray<csShaderVariable*> &stacks)
{

  /*
  iRenderBufferSource* source = mesh->buffersource;

  csSoftPolygonRenderer* polyRender = (csSoftPolygonRenderer*)source;
  */

  iRenderBuffer* indexbuf = (modes.buffers ? modes.buffers->GetRenderBuffer(CS_BUFFER_INDEX) : 0);
  size_t i;
  
  if (!indexbuf)
  {  
    csShaderVariable* indexBufSV = csGetShaderVariableFromStack (stacks, string_indices);
    CS_ASSERT (indexBufSV != 0);
    
    indexBufSV->GetValue (indexbuf);
  }
  CS_ASSERT(indexbuf);
  
  csSoftPolygonRenderer* polyRender = (csSoftPolygonRenderer*)indexbuf;

  csShaderVariable* texDiffuseSV = csGetShaderVariableFromStack (stacks, string_texture_diffuse);
  CS_ASSERT_MSG ("'tex diffuse' SV does not exist.", texDiffuseSV != 0);
  iTextureWrapper* texw = 0;
  texDiffuseSV->GetValue (texw);
  texw->Visit ();
  iTextureHandle* texh = 0;
  texDiffuseSV->GetValue (texh);
  CS_ASSERT_MSG ("A material has no diffuse texture attached.", texh);
  
  csShaderVariable* texLightmapSV = csGetShaderVariableFromStack (stacks, string_texture_lightmap);
  CS_ASSERT_MSG ("'tex lightmap' SV does not exist.",
    texLightmapSV != 0);

  iTextureHandle* lmh = 0;
  texLightmapSV->GetValue (lmh);
  //if (!lmh) return; // @@@ FIXME
  //CS_ASSERT(("Mesh did not supply lightmap", lmh));
  csSoftSuperLightmap* slm = 0;
  
  if (lmh)
  {
    slm = (csSoftSuperLightmap*)((csSoftwareTextureHandle*)lmh->GetCacheData());
  }

  /*
    - perspective
    - clip
   */

  G3DPolygonDP poly;
  poly.use_fog = false;
  poly.do_fullbright = false;
  poly.mixmode = modes.mixmode;
  z_buf_mode = modes.z_buf_mode;

  csReversibleTransform object2camera = w2c / mesh->object2world;

  for (i = 0; i < polyRender->polys.Length (); i++)
  {
    
    csPolygonRenderData* spoly = polyRender->polys[i];

    int numVerts = spoly->num_vertices;
    CS_ALLOC_STACK_ARRAY(csVector3, camVerts, numVerts);

    int v;
    int cnt_vis = 0;
    csVector3* obj_verts = *(spoly->p_obj_verts);
    for (v = 0; v < numVerts; v++)
    {
      camVerts[v] = object2camera.Other2This (obj_verts[spoly->vertices[v]]);
      if (camVerts[v].z >= 0)
      {
	cnt_vis++;
      }
    }
    if (cnt_vis == 0) continue;

    const csPlane3 &oplane = spoly->plane_obj;
    float cl = oplane.Classify (object2camera.GetOrigin ());
    //if (cl > EPSILON) continue;
    if ((mesh->do_mirror && cl <= 0) || ((!mesh->do_mirror) && cl >= 0)) continue;

    /* @@@ Portal clipping here? */

    csPlane3 plane_cam;
    object2camera.Other2This (oplane, camVerts[0], plane_cam);

    /* do perspective */
    csPoly2D p2d;
    DoPolyPerspective (camVerts, numVerts, p2d, 
      aspect, width2, height2,
      plane_cam);

    if (p2d.ClipAgainst (clipper))
    {
      poly.num = p2d.GetVertexCount ();
      memcpy (poly.vertices, p2d.GetVertices (), sizeof (csVector2) * poly.num);
      poly.z_value = camVerts[0].z;
      poly.normal = plane_cam;
      poly.txt_handle = texh;
      if (slm)
      {
        poly.rlm = slm->GetRlmForID (polyRender->rlmIDs[i]);
      }
      else
      {
        poly.rlm = 0;
      }
      
      //@@@@@ poly.mat_handle = spoly->material;
      // Adding the material handle to csPolygonRenderData is not
      // really an option but we need the material here. Perhaps use
      // same technique as OpenGL renderer uses?
      //CS_ASSERT (false);

      csMatrix3 m_o2t;
      csVector3 v_o2t;
      if (spoly->tmapping)
      {
        m_o2t = spoly->tmapping->GetO2T();
        v_o2t = spoly->tmapping->GetO2TTranslation();
      }
      else
      {
        CS_ASSERT (false);	// @@@ Support flat-shading!
      }
      csReversibleTransform obj2tex (m_o2t, v_o2t);

      
      csMatrix3 m_cam2tex = obj2tex.GetO2T () * object2camera.GetT2O ();
      csVector3 v_cam2tex = object2camera.Other2This (obj2tex.GetO2TTranslation ());
      
      poly.cam2tex.m_cam2tex = &m_cam2tex;
      poly.cam2tex.v_cam2tex = &v_cam2tex;

      poly.texmap = spoly->tmapping;

      DrawPolygon (poly);
    }
  }

/*  int indexPos = mesh->indexstart
  int polyIdx = 0;
  while (indexPos < mesh->indexend)
  {
    int numVerts = mesh->polyNumVerts[polyIdx];
    poly.num = numVerts;
    poly.mat_handle = mesh->material->GetMaterialHandle ();
    poly.texmap = mesh->polyTexMaps[polyIdx];

    indexPos += numVerts;
    polyIdx++;
  }*/

  /*indexbuf->Release ();
  for (i=0; i<numBuffers; ++i)
  {
    if (activebuffers[i] != 0)
      activebuffers[i]->Release ();
  }*/
}
#endif

class TriangleDrawer
{
  csSoftwareGraphics3DCommon& g3d;
  VertexBuffer clipInBuf[clipMaxBuffers];
  size_t clipInStride[clipMaxBuffers];
  VertexBuffer clipOutBuf[clipMaxBuffers];
  iRenderBuffer** activebuffers;
  ClipBuffersMask buffersMask;
  const csCoreRenderMesh* mesh;
  const csRenderMeshModes& modes;

  static const int outFloatsPerBuf = 16;
  float clipOut[clipMaxBuffers * outFloatsPerBuf];
  ClipMeatZNear clipZNear;
  BuffersClipper<ClipMeatZNear> bclipperZNear;
  csVector3 outPersp[4];

  void ProjectVertices (size_t rangeStart, size_t rangeEnd)
  {
    size_t num_vertices = rangeEnd + 1;

    persp->SetLength (num_vertices);
    const int width2 = g3d.width2;
    const int height2 = g3d.height2;
    const float aspect = g3d.aspect;
    csRenderBufferLock<csVector3, iRenderBuffer*> work_verts 
      (activebuffers[VATTR_SPEC(POSITION)], CS_BUF_LOCK_READ);
    // Perspective project.
    for (size_t i = rangeStart; i <= rangeEnd; i++)
    {
      if (work_verts[i].z >= SMALL_Z)
      {
	(*persp)[i].z = work_verts[i].z;
	float iz = aspect / (*persp)[i].z;
	(*persp)[i].x = work_verts[i].x * iz + width2;
	(*persp)[i].y = work_verts[i].y * iz + height2;
      }
      else
	(*persp)[i] = work_verts[i];
    }
  }
  void ClipAndDrawTriangle (const size_t* trivert)
  {
    //-----
    // Do backface culling. Note that this depends on the
    // mirroring of the current view.
    //-----
    const csVector2& pa = *(csVector2*)&outPersp[trivert[0]];
    const csVector2& pb = *(csVector2*)&outPersp[trivert[1]];
    const csVector2& pc = *(csVector2*)&outPersp[trivert[2]];
    float area = csMath2::Area2 (pa, pb, pc);
    if (!area) return;
    if (mesh->do_mirror)
    {
      if (area <= -SMALL_EPSILON) return;
    }
    else
    {
      if (area >= SMALL_EPSILON) return;
    }

    // Clip triangle. Note that the clipper doesn't care about the
    // orientation of the triangle vertices. It works just as well in
    // mirrored mode.

    /* You can only have as much clipped vertices as the sum of vertices in 
     * the original poly and those in the clipping poly... I think. */
    const size_t maxClipVertices = g3d.clipper->GetVertexCount() + 3;
    const size_t floatsPerBufPerVert = 4;
    ClipMeatiClipper meat;
    meat.Init (g3d.clipper, maxClipVertices);
    CS_ALLOC_STACK_ARRAY(float, out, 
      floatsPerBufPerVert * clipMaxBuffers * maxClipVertices);
    CS_ALLOC_STACK_ARRAY(csVector3, clippedPersp, maxClipVertices);
    VertexBuffer clipOutBuf[clipMaxBuffers];
    for (size_t i = 0; i < clipMaxBuffers; i++)
    {
      clipOutBuf[i].data = (uint8*)&out[i * floatsPerBufPerVert * maxClipVertices];
      const size_t c = clipInBuf[i].comp;
      clipOutBuf[i].comp = c;
      clipInStride[i] = c * sizeof(float);
    }

    BuffersClipper<ClipMeatiClipper> clip (meat);
    clip.Init (outPersp, clippedPersp,
      this->clipOutBuf, clipInStride, clipOutBuf, buffersMask);
    csTriangle tri;
    if (mesh->do_mirror)
    {
      tri.a = (int)trivert[2];
      tri.b = (int)trivert[1];
      tri.c = (int)trivert[0];
    }
    else
    {
      tri.a = (int)trivert[0];
      tri.b = (int)trivert[1];
      tri.c = (int)trivert[2];
    }
    size_t rescount = clip.DoClip (tri);
    if (rescount == 0) return;

    g3d.polyrast.DrawPolygonFX (rescount, clippedPersp, clipOutBuf, 
      buffersMask, g3d.activeTex, modes.mixmode | CS_FX_TILING | CS_FX_FLAT);
  }

public:
  TriangleDrawer (csSoftwareGraphics3DCommon& g3d,
    iRenderBuffer* activebuffers[], size_t rangeStart, 
    size_t rangeEnd, const csCoreRenderMesh* mesh,
    const csRenderMeshModes& modes) : g3d(g3d), 
    activebuffers(activebuffers), mesh(mesh), modes(modes),
    bclipperZNear(clipZNear)
  {
    const size_t bufNum = csMin (activeBufferCount, clipMaxBuffers);

    buffersMask = 0;
    for (size_t b = 0; b < bufNum; b++)
    {
      if (activebuffers[b] == 0) continue;

      iRenderBuffer* buf = activebuffers[b];
      buffersMask |= 1 << b;
      clipInBuf[b].data = (uint8*)buf->Lock (CS_BUF_LOCK_READ);
      clipInBuf[b].comp = buf->GetComponentCount();
      clipInStride[b] = buf->GetElementDistance();
      clipOutBuf[b].data = (uint8*)&clipOut[b * outFloatsPerBuf];
      clipOutBuf[b].comp = 4;
    }

    ProjectVertices (rangeStart, rangeEnd);

    clipZNear.Init (g3d.width2, g3d.height2, g3d.aspect);
    bclipperZNear.Init (persp->GetArray(), outPersp,
      clipInBuf, clipInStride, clipOutBuf, buffersMask);
  }

  ~TriangleDrawer()
  {
    const size_t bufNum = csMin (activeBufferCount, clipMaxBuffers);

    for (size_t b = 0; b < bufNum; b++)
    {
      if (activebuffers[b] != 0) activebuffers[b]->Release();
    }
  }

  void DrawTriangle (uint a, uint b, uint c)
  {
    csTriangle tri;
    tri.a = a;
    tri.b = b;
    tri.c = c;
    /* Small Z clipping. Also projects unprojected vertices (skipped in
     * ProjectVertices() due a Z coord too small) and will invert the Z
     * of the pespective verts. */
    size_t n = bclipperZNear.DoClip (tri);
    if (n == 0) return;
    CS_ASSERT((n>= 3) && (n <= 4));

    static const size_t trivert1[3] = { 0, 1, 2 };
    ClipAndDrawTriangle (trivert1);
    if (n == 4)
    {
      static const size_t trivert2[3] = { 0, 2, 3 };
      ClipAndDrawTriangle (trivert2);
    }
  }
};

void csSoftwareGraphics3DCommon::DrawMesh (const csCoreRenderMesh* mesh,
    const csRenderMeshModes& modes,
    const csArray<csShaderVariable*> &stacks)
{
#if 0
  if (mesh->meshtype == CS_MESHTYPE_POLYGON)
  {
    DrawPolysMesh (mesh, modes, stacks);
    return;
  }
#endif

  if (!persp)
  {
    persp= Get_persp();
  }

  iRenderBuffer* indexbuf = 
    (modes.buffers ? modes.buffers->GetRenderBuffer(CS_BUFFER_INDEX) : 0);

  if (!indexbuf)
  {
    csShaderVariable* indexBufSV = csGetShaderVariableFromStack (stacks, string_indices);
    CS_ASSERT (indexBufSV != 0);
    indexBufSV->GetValue (indexbuf);
  }
  CS_ASSERT(indexbuf);

  uint32 *indices = (uint32*)indexbuf->Lock (CS_BUF_LOCK_READ);

  csReversibleTransform object2camera = w2c / mesh->object2world;

  //z_buf_mode = CS_ZBUF_USE;
  z_buf_mode = modes.z_buf_mode;
  polyrast.SetZBufMode (z_buf_mode);


  // @@@ Currently we don't implement multi-texture
  // in the generic implementation. This is a todo...
  
  // Make sure we don't process too many vertices;
  size_t num_vertices = indexbuf->GetRangeEnd()+1;

  // Update work tables.
  if (num_vertices > persp->Capacity ())
  {
    persp->SetCapacity (num_vertices);
  }
  persp->SetLength (num_vertices);

  const size_t rangeStart = indexbuf->GetRangeStart();
  const size_t rangeEnd = indexbuf->GetRangeEnd();

  if (!object2camera.IsIdentity())
  {
    if (!translatedVerts.IsValid()
      || (translatedVerts->GetElementCount() <= rangeEnd))
    {
      translatedVerts = csRenderBuffer::CreateRenderBuffer (
	rangeEnd + 1, CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 3);
    }

    csRenderBufferLock<csVector3, iRenderBuffer*> f1 
      (activebuffers[VATTR_SPEC(POSITION)], CS_BUF_LOCK_READ);
    if (!f1) return; 
    csVector3* tr_verts = 
      (csVector3*)translatedVerts->Lock (CS_BUF_LOCK_NORMAL);

    size_t i;
    for (i = rangeStart; i <= rangeEnd; i++)
      tr_verts[i] = object2camera * f1[i];

    translatedVerts->Release();

    activebuffers[VATTR_SPEC(POSITION)] = translatedVerts;
  }

  const csRenderMeshType meshtype = mesh->meshtype;
  if ((meshtype >= CS_MESHTYPE_TRIANGLES) 
    && (meshtype <= CS_MESHTYPE_TRIANGLEFAN))
  {
    TriangleDrawer triDraw (*this, activebuffers, 
      rangeStart, rangeEnd, mesh, modes);

    uint indexstart = mesh->indexstart;
    uint indexend = mesh->indexend;

    switch (meshtype)
    {
    case CS_MESHTYPE_TRIANGLES:
      {
	uint32* tri = indices + indexstart;
	const uint32* triEnd = indices + indexend;
	while (tri < triEnd)
	{
	  triDraw.DrawTriangle (tri[0], tri[1], tri[2]);
	  tri += 3;
	}
      }
      break;
    case CS_MESHTYPE_TRIANGLESTRIP:
      {
	//triangulate
	uint32* tri = indices + indexstart;
	const uint32* triEnd = indices + indexend;
	uint32 old2 = *tri++;
	uint32 old1 = *tri++;
	while (tri < triEnd)
	{
	  const uint32 cur = *tri++;
	  triDraw.DrawTriangle (old2, old1, cur);
	  old2 = old1;
	  old1 = cur;
	}
	break;
      }
    case CS_MESHTYPE_TRIANGLEFAN:
      {
	//triangulate
	uint32* tri = indices + indexstart;
	const uint32* triEnd = indices + indexend;
	uint32 first = *tri++;
	uint32 old1 = *tri++;
	while (tri < triEnd)
	{
	  const uint32 cur = *tri++;
	  triDraw.DrawTriangle (first, old1, cur);
	  old1 = cur;
	}
	break;
      }
    case CS_MESHTYPE_QUADS:
      {
	//triangulate
	uint32* quad = indices + indexstart;
	const uint32* quadEnd = indices + indexend;
	while (quad < quadEnd)
	{
	  triDraw.DrawTriangle (quad[0], quad[1], quad[2]);
	  triDraw.DrawTriangle (quad[0], quad[2], quad[3]);
	  quad += 4;
	}
	break;
      }
    default:
      ;
    }
  }
}

} // namespace cspluginSoft3d
