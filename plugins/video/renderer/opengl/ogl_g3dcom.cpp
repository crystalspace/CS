/*
 * Copyright (C) 1998-2001 by Jorrit Tyberghein
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>

#include "cssysdef.h"
#include "cssys/sysdriv.h"
#include "qint.h"
#include "csutil/scf.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/polyclip.h"
#include "csgeom/plane3.h"
#include "csgeom/frustum.h"
#include "ogl_g3dcom.h"
#include "ogl_txtcache.h"
#include "ogl_txtmgr.h"
#include "iutil/cfgfile.h"
#include "iutil/cmdline.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "isys/system.h"
#include "isys/plugin.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/lightmap.h"	//@@@
#include "ivideo/graph2d.h"
#include "csutil/garray.h"
#include "csutil/cscolor.h"
#include "csgfx/rgbpixel.h"
#include "qsqrt.h"

// uncomment the 'USE_MULTITEXTURE 1' define to enable code for
// multitexture support - this is independent of the extension detection,
// but  it may rely on the extension module to supply proper function
// prototypes for the ARB_MULTITEXTURE functions
//#define USE_MULTITEXTURE 1

// Whether or not we should try  and use OpenGL extensions. This should be
// removed eventually, when all platforms have been updated.
//#define USE_EXTENSIONS 1

// ---------------------------------------------------------------------------

// if you figure out how to support OpenGL extensions on your
// machine add an appropriate file in the 'ext/'
// directory and mangle the file 'ext/ext_auto.inc'
// to access your extension code
#if USE_EXTENSIONS
#include "ext/ext_auto.inc"
#else
void csGraphics3DOGLCommon::DetectExtensions() {}
#endif

#define BYTE_TO_FLOAT(x) ((x) * (1.0 / 255.0))

/*=========================================================================
 Static growing array declaration for DrawTriangleMesh
=========================================================================*/
// smgh moved it here, no longer segfaults on exit as a consequence..
// Also IncRefing and DecRefing in the ctor/dtor, as the auxiliary buffer
// dynamic textures will utilise multiple instances of csGraphics3DOGLCommon

/// Static vertex array.
static CS_DECLARE_GROWING_ARRAY_REF (tr_verts, csVector3);
/// Static uv array.
static CS_DECLARE_GROWING_ARRAY_REF (uv_verts, csVector2);
/// Static uv array for multi-texture.
static CS_DECLARE_GROWING_ARRAY_REF (uv_mul_verts, csVector2);
/// Array with colors.
static CS_DECLARE_GROWING_ARRAY_REF (color_verts, csColor);
/// Array with RGBA colors.
static CS_DECLARE_GROWING_ARRAY_REF (rgba_verts, GLfloat);

/// Array for clipping.
static CS_DECLARE_GROWING_ARRAY_REF (clipped_triangles, csTriangle);
/// Array for clipping.
static CS_DECLARE_GROWING_ARRAY_REF (clipped_translate, int);
/// Array for clipping.
static CS_DECLARE_GROWING_ARRAY_REF (clipped_vertices, csVector3);
/// Array for clipping.
static CS_DECLARE_GROWING_ARRAY_REF (clipped_texels, csVector2);
/// Array for clipping.
static CS_DECLARE_GROWING_ARRAY_REF (clipped_colors, csColor);
/// Array for clipping.
static CS_DECLARE_GROWING_ARRAY_REF (clipped_fog, G3DFogInfo);

/*=========================================================================
 Method implementations
=========================================================================*/

SCF_IMPLEMENT_IBASE(csGraphics3DOGLCommon)
  SCF_IMPLEMENTS_INTERFACE(iGraphics3D)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGraphics3DOGLCommon::eiPlugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csGraphics3DOGLCommon* csGraphics3DOGLCommon::ogl_g3d = NULL;

csGraphics3DOGLCommon::csGraphics3DOGLCommon (iBase* parent):
  G2D (NULL), object_reg (NULL)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);

  ogl_g3d = this;
  texture_cache = NULL;
  lightmap_cache = NULL;
  txtmgr = NULL;
  m_fogtexturehandle = 0;
  fps_limit = 0;
  debug_edges = false;

  /// caps will be read from config or reset to defaults during Initialize.
  Caps.CanClip = false;
  Caps.minTexHeight = 2;
  Caps.minTexWidth = 2;
  Caps.maxTexHeight = 1024;
  Caps.maxTexWidth = 1024;
  Caps.fog = G3DFOGMETHOD_VERTEX;
  Caps.NeedsPO2Maps = false;
  Caps.MaxAspectRatio = 32768;
  GLCaps.need_screen_clipping = false;
  GLCaps.use_stencil = false;
  clip_optional[0] = OPENGL_CLIP_AUTO;
  clip_optional[1] = OPENGL_CLIP_SOFTWARE;
  clip_optional[2] = OPENGL_CLIP_SOFTWARE;
  clip_required[0] = OPENGL_CLIP_AUTO;
  clip_required[1] = OPENGL_CLIP_SOFTWARE;
  clip_required[2] = OPENGL_CLIP_SOFTWARE;
  clip_outer[0] = OPENGL_CLIP_AUTO;
  clip_outer[1] = OPENGL_CLIP_SOFTWARE;
  clip_outer[2] = OPENGL_CLIP_SOFTWARE;

  // Default extension state is for all extensions to be OFF
  ARB_multitexture = false;
  clipper = NULL;
  cliptype = CS_CLIPPER_NONE;
  toplevel_init = false;
  stencil_init = false;
  planes_init = false;
  frustum_valid = false;

  // See note above.
  tr_verts.IncRef ();
  uv_verts.IncRef ();
  uv_mul_verts.IncRef ();
  color_verts.IncRef ();
  rgba_verts.IncRef ();
  clipped_triangles.IncRef ();
  clipped_translate.IncRef ();
  clipped_vertices.IncRef ();
  clipped_texels.IncRef ();
  clipped_colors.IncRef ();
  clipped_fog.IncRef ();

  // Are we going to use the inverted orthographic projection matrix?
  inverted = false;
}

csGraphics3DOGLCommon::~csGraphics3DOGLCommon ()
{
  Close ();
  if (G2D) G2D->DecRef ();

  // see note above
  tr_verts.DecRef ();
  uv_verts.DecRef ();
  uv_mul_verts.DecRef ();
  color_verts.DecRef ();
  rgba_verts.DecRef ();
  clipped_triangles.DecRef ();
  clipped_translate.DecRef ();
  clipped_vertices.DecRef ();
  clipped_texels.DecRef ();
  clipped_colors.DecRef ();
  clipped_fog.DecRef ();
}

void csGraphics3DOGLCommon::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  iReporter* rep = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (rep)
    rep->ReportV (severity, "crystalspace.graphics3d.opengl", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

bool csGraphics3DOGLCommon::Initialize (iObjectRegistry* p)
{
  object_reg = p;
  CS_ASSERT (object_reg != NULL);
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  iSystem* sys = CS_GET_SYSTEM (object_reg);	//@@@
  sys->CallOnEvents (&scfiPlugin, CSMASK_Broadcast);
  return true;
}

bool csGraphics3DOGLCommon::HandleEvent (iEvent& Event)
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

bool csGraphics3DOGLCommon::NewInitialize ()
{
  CS_ASSERT (object_reg != NULL);
  config.AddConfig(object_reg, "/config/opengl.cfg");
  iCommandLineParser* cmdline = CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser);

  const char *driver = cmdline->GetOption ("canvas");
  if (!driver)
    driver = config->GetStr ("Video.OpenGL.Canvas", CS_OPENGL_2D_DRIVER);

  G2D = CS_LOAD_PLUGIN (plugin_mgr, driver, NULL, iGraphics2D);
  if (!G2D)
    return false;

  txtmgr = new csTextureManagerOpenGL (object_reg, G2D, config, this);

  m_renderstate.dither = config->GetBool ("Video.OpenGL.EnableDither", false);
  z_buf_mode = CS_ZBUF_NONE;
  width = height = -1;
  Caps.CanClip = config->GetBool("Video.OpenGL.Caps.CanClip", false);
  Caps.minTexHeight = config->GetInt("Video.OpenGL.Caps.MinTexHeight", 2);
  Caps.minTexWidth = config->GetInt("Video.OpenGL.Caps.MinTexWidth", 2);
  Caps.maxTexHeight = config->GetInt("Video.OpenGL.Caps.MaxTexHeight", 1024);
  Caps.maxTexWidth = config->GetInt("Video.OpenGL.Caps.MaxTexWidth", 1024);
  Caps.fog = G3DFOGMETHOD_VERTEX;
  Caps.NeedsPO2Maps = config->GetBool("Video.OpenGL.Caps.NeedsPO2Maps", false);
  Caps.MaxAspectRatio = config->GetInt("Video.OpenGL.Caps.MaxAspectRatio", 
    32768);
  GLCaps.use_stencil = config->GetBool ("Video.OpenGL.Caps.Stencil", false);
  GLCaps.need_screen_clipping =
  	config->GetBool ("Video.OpenGL.Caps.NeedScreenClipping", false);
  GLCaps.nr_hardware_planes = config->GetInt ("Video.OpenGL.Caps.HWPlanes", 6);
  fps_limit = config->GetInt ("Video.OpenGL.FpsLimit", 0);
  OpenGLLightmapCache::super_lm_num = config->GetInt (
  	"Video.OpenGL.SuperLightMapNum", 10);
  OpenGLLightmapCache::super_lm_size = config->GetInt (
  	"Video.OpenGL.SuperLightMapSize", 256);
  if (OpenGLLightmapCache::super_lm_size > Caps.maxTexWidth)
    OpenGLLightmapCache::super_lm_size = Caps.maxTexWidth;
  Report (CS_REPORTER_SEVERITY_NOTIFY,
  	"  Super lightmaps: num=%d size=%dx%d",
  	OpenGLLightmapCache::super_lm_num,
	OpenGLLightmapCache::super_lm_size,
	OpenGLLightmapCache::super_lm_size);

  unsigned int i, j;
  const char* clip_opt = config->GetStr ("Video.OpenGL.ClipOptional", "auto");
  if (!strcmp (clip_opt, "auto"))
    clip_optional[0] = OPENGL_CLIP_AUTO;
  else
  {
    for (j = i = 0 ; i < strlen (clip_opt) ; i++)
    {
      char c = clip_opt[i];
      if ((c == 's' || c == 'S') && !GLCaps.use_stencil) continue;
      if ((c == 'p' || c == 'P') && GLCaps.nr_hardware_planes <= 0) continue;
      if (c == 'z' || c == 'Z') continue;
      clip_optional[j++] = c;
      if (j >= 3) break;
    }
    while (j < 3) clip_optional[j++] = '0';
    Report (CS_REPORTER_SEVERITY_NOTIFY, "  Optional Clipping: %c%c%c",
      clip_optional[0], clip_optional[1], clip_optional[2]);
  }

  const char* clip_req = config->GetStr ("Video.OpenGL.ClipRequired", "auto");
  if (!strcmp (clip_req, "auto"))
    clip_required[0] = OPENGL_CLIP_AUTO;
  else
  {
    for (j = i = 0 ; i < strlen (clip_req) ; i++)
    {
      char c = clip_req[i];
      if ((c == 's' || c == 'S') && !GLCaps.use_stencil) continue;
      if ((c == 'p' || c == 'P') && GLCaps.nr_hardware_planes <= 0) continue;
      if (c == 'z' || c == 'Z') continue;
      if (c == 'n' || c == 'N') continue;
      clip_required[j++] = c;
      if (j >= 3) break;
    }
    while (j < 3) clip_required[j++] = '0';
    Report (CS_REPORTER_SEVERITY_NOTIFY, "  Required Clipping: %c%c%c",
        clip_required[0], clip_required[1], clip_required[2]);
  }

  const char* clip_out = config->GetStr ("Video.OpenGL.ClipOuter", "zsp0");
  if (!strcmp (clip_out, "auto"))
    clip_outer[0] = OPENGL_CLIP_AUTO;
  else
  {
    for (j = i = 0 ; i < strlen (clip_out) ; i++)
    {
      char c = clip_out[i];
      if ((c == 's' || c == 'S') && !GLCaps.use_stencil) continue;
      if ((c == 'p' || c == 'P') && GLCaps.nr_hardware_planes <= 0) continue;
      if ((c == 'z' || c == 'Z' || c == 's' || c == 'S' || c == 'p' || c == 'P')
    	  && GLCaps.need_screen_clipping) continue;
      if (c == 'n' || c == 'N') continue;
      clip_outer[j++] = c;
      if (j >= 3) break;
    }
    while (j < 3) clip_outer[j++] = '0';
    Report (CS_REPORTER_SEVERITY_NOTIFY, "  Outer Clipping: %c%c%c",
        clip_outer[0], clip_outer[1], clip_outer[2]);
  }

  m_renderstate.alphablend = true;
  m_renderstate.mipmap = 0;
  m_renderstate.gouraud = true;
  m_renderstate.lighting = true;
  m_renderstate.textured = true;

  m_config_options.do_multitexture_level = 0;
  m_config_options.do_extra_bright = false;

  return true;
}

struct ModRes
{
  char mode;
  int pref_order;
  int cnt;
};

static int compare_mode (const void* el1, const void* el2)
{
  ModRes* m1 = (ModRes*)el1;
  ModRes* m2 = (ModRes*)el2;
  if (m1->cnt < m2->cnt) return 1;
  else if (m1->cnt > m2->cnt) return -1;
  else if (m1->pref_order < m2->pref_order) return 1;
  else if (m1->pref_order > m2->pref_order) return -1;
  else return 0;
}

void csGraphics3DOGLCommon::PerfTest ()
{
  bool compute_optional = clip_optional[0] == OPENGL_CLIP_AUTO;
  bool compute_outer = clip_outer[0] == OPENGL_CLIP_AUTO;
  bool compute_required = clip_required[0] == OPENGL_CLIP_AUTO;
  if (!compute_optional && !compute_outer && !compute_required)
    return;

  SetPerspectiveAspect (height);
  SetPerspectiveCenter (width/2, height/2);

  G3DTriangleMesh mesh;
  int res = 64;
  mesh.num_vertices = (res+1)*(res+1);
  mesh.num_triangles = res*res*2;
  mesh.num_vertices_pool = 1;
  mesh.clip_portal = CS_CLIP_NEEDED;
  mesh.clip_plane = CS_CLIP_NOT;
  mesh.clip_z_plane = CS_CLIP_NOT;
  mesh.use_vertex_color = true;
  mesh.do_fog = false;
  mesh.do_morph_texels = false;
  mesh.do_morph_colors = false;
  mesh.vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
  mesh.fxmode = CS_FX_GOURAUD;
  mesh.morph_factor = 0;
  mesh.mat_handle = NULL;
  mesh.vertex_fog = NULL;
  mesh.triangles = new csTriangle [mesh.num_triangles];
  mesh.vertices[0] = new csVector3 [mesh.num_vertices];
  mesh.texels[0] = new csVector2 [mesh.num_vertices];
  mesh.vertex_colors[0] = new csColor [mesh.num_vertices];

  float zx, zy, z;
  // First we calculate the z which will bring the top-left vertex
  // further than (-asp_center_x,-asp_center_y).
  zx = -5. / ((-asp_center_x-asp_center_x) * inv_aspect);
  zy = -5. / ((-asp_center_y-asp_center_y) * inv_aspect);
  if (zy < zx) z = zy;
  else z = zx;

  int x, y, i, t;
  float fx, fy;
  i = 0;
  for (y = 0 ; y <= res ; y++)
  {
    fy = float (y) / float (res) - .5;
    for (x = 0 ; x <= res ; x++)
    {
      fx = float (x) / float (res) - .5;
      mesh.vertices[0][i].Set (10.*fx, 10.*fy, z);
      mesh.texels[0][i].Set (0, 0);
      mesh.vertex_colors[0][i].Set (1, 0, 0);
      i++;
    }
  }
  i = 0;
  t = 0;
  for (y = 0 ; y < res ; y++)
  {
    for (x = 0 ; x < res ; x++)
    {
      mesh.triangles[t].c = i;
      mesh.triangles[t].b = i+1;
      mesh.triangles[t].a = i+res+1;
      t++;
      mesh.triangles[t].c = i+1;
      mesh.triangles[t].b = i+res+1+1;
      mesh.triangles[t].a = i+res+1;
      t++;
      i++;
    }
    i++;
  }

  BeginDraw (CSDRAW_3DGRAPHICS);

  int dw = width / 20;
  int dh = height / 20;
  csBoxClipper clipper (dw, dh, width-dw, height-dh);
  SetClipper ((iClipper2D*)&clipper, CS_CLIPPER_TOPLEVEL);
  ResetNearPlane ();

  ModRes test_modes[9];
  int test_mode_cnt = 0;
  test_modes[test_mode_cnt].mode = '0';
  test_modes[test_mode_cnt++].pref_order = 3;
  test_modes[test_mode_cnt].mode = 'z';
  test_modes[test_mode_cnt++].pref_order = 7;
  test_modes[test_mode_cnt].mode = 'Z';
  test_modes[test_mode_cnt++].pref_order = 6;
  if (GLCaps.use_stencil)
  {
    test_modes[test_mode_cnt].mode = 's';
    test_modes[test_mode_cnt++].pref_order = 2;
    test_modes[test_mode_cnt].mode = 'S';
    test_modes[test_mode_cnt++].pref_order = 1;
  }
  if (GLCaps.nr_hardware_planes > 0)
  {
    test_modes[test_mode_cnt].mode = 'p';
    test_modes[test_mode_cnt++].pref_order = 5;
    test_modes[test_mode_cnt].mode = 'P';
    test_modes[test_mode_cnt++].pref_order = 4;
  }

  if (compute_outer && !GLCaps.need_screen_clipping)
  {
    //========
    // First test clipping geometry against the outer portal (close
    // to screen boundaries).
    //========
    for (i = 0 ; i < test_mode_cnt ; i++)
    {
      clip_outer[0] = test_modes[i].mode;
      int cnt = 0;
      csTicks end = csGetTicks () + 1000;
      while (csGetTicks () < end)
      {
        glDepthMask (GL_TRUE);
        glClear (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_USE);
        csReversibleTransform o2c;	// Identity transform.
        SetObjectToCamera (&o2c);
        DrawTriangleMesh (mesh);
        cnt++;
      }
      test_modes[i].cnt = cnt;
      Report (CS_REPORTER_SEVERITY_NOTIFY, "    %d FPS for %c", cnt,
    	  test_modes[i].mode); fflush (stdout);
    }
    // Sort the results.
    qsort (test_modes, test_mode_cnt, sizeof (ModRes), compare_mode);
    clip_outer[0] = test_modes[0].mode;
    if (test_mode_cnt > 1) clip_outer[1] = test_modes[1].mode;
    else clip_outer[1] = '0';
    if (test_mode_cnt > 2) clip_outer[2] = test_modes[2].mode;
    else clip_outer[2] = '0';
  }
  else
  {
    if (compute_outer)
    {
      clip_outer[0] = '0';
      clip_outer[1] = '0';
      clip_outer[2] = '0';
    }
  }

  //========
  // Now test again for a very small clipper. This test is to see if
  // it is beneficial to even enable clipping on optional portals.
  //========
  csBoxClipper clipper2 (width/2-dw, height/2-dh, width/2+dw, height/2+dh);
  SetClipper ((iClipper2D*)&clipper2, CS_CLIPPER_OPTIONAL);
  // First relocate all vertices so that the mesh fits on screen
  // but not in the small clipper.
  // First we calculate the z which will bring the top-left vertex
  // beyond (dw,dh).
  zx = -5. / ((dw-asp_center_x) * inv_aspect);
  zy = -5. / ((dh-asp_center_y) * inv_aspect);
  if (zy > zx) z = zy;
  else z = zx;
  for (i = 0 ; i < mesh.num_vertices ; i++)
    mesh.vertices[0][i].z = z;
  for (i = 0 ; i < test_mode_cnt ; i++)
  {
    if (test_modes[i].mode == 'z') test_modes[i].mode = 'n';
    else if (test_modes[i].mode == 'Z') test_modes[i].mode = 'N';
    clip_optional[0] = test_modes[i].mode;
    int cnt = 0;
    csTicks end = csGetTicks () + 1000;
    while (csGetTicks () < end)
    {
      glDepthMask (GL_TRUE);
      glClear (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
      SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_USE);
      csReversibleTransform o2c;	// Identity transform.
      SetObjectToCamera (&o2c);
      DrawTriangleMesh (mesh);
      cnt++;
    }
    test_modes[i].cnt = cnt;
    Report (CS_REPORTER_SEVERITY_NOTIFY, "    %d FPS for %c (small clipper)", cnt,
    	test_modes[i].mode); fflush (stdout);
  }

  // Sort the results.
  qsort (test_modes, test_mode_cnt, sizeof (ModRes), compare_mode);
  int j;
  if (compute_required)
  {
    j = 0;
    for (i = 0 ; i < 3 ; i++)
    {
      while (j < test_mode_cnt &&
    	  (test_modes[j].mode == 'n' || test_modes[j].mode == 'N')) j++;
      if (j >= test_mode_cnt) clip_required[i] = '0';
      else clip_required[i] = test_modes[j].mode;
      j++;
    }
  }
  if (compute_optional)
  {
    j = 0;
    for (i = 0 ; i < 3 ; i++)
    {
      if (j >= test_mode_cnt) clip_optional[i] = '0';
      else clip_optional[i] = test_modes[j].mode;
      j++;
    }
  }
  Report (CS_REPORTER_SEVERITY_NOTIFY, "    Video.OpenGL.ClipOuter = %c%c%c",
  	clip_outer[0], clip_outer[1], clip_outer[2]);
  Report (CS_REPORTER_SEVERITY_NOTIFY, "    Video.OpenGL.ClipRequired = %c%c%c",
  	clip_required[0], clip_required[1], clip_required[2]);
  Report (CS_REPORTER_SEVERITY_NOTIFY, "    Video.OpenGL.ClipOptional = %c%c%c",
  	clip_optional[0], clip_optional[1], clip_optional[2]);
  char buf[4]; buf[3] = 0;
  buf[0] = clip_required[0];
  buf[1] = clip_required[1];
  buf[2] = clip_required[2];
  config->SetStr ("Video.OpenGL.ClipRequired", buf);
  buf[0] = clip_outer[0];
  buf[1] = clip_outer[1];
  buf[2] = clip_outer[2];
  config->SetStr ("Video.OpenGL.ClipOuter", buf);
  buf[0] = clip_optional[0];
  buf[1] = clip_optional[1];
  buf[2] = clip_optional[2];
  config->SetStr ("Video.OpenGL.ClipOptional", buf);
  config->Save ();

  SetClipper (NULL, CS_CLIPPER_NONE);

  FinishDraw ();
  Print (NULL);
}

void csGraphics3DOGLCommon::SharedInitialize (csGraphics3DOGLCommon *d)
{
  txtmgr = d->txtmgr;
  z_buf_mode = CS_ZBUF_NONE;
  width = height = -1;

  m_renderstate.dither = d->m_renderstate.dither;

  m_renderstate.alphablend = true;
  m_renderstate.mipmap = 0;
  m_renderstate.gouraud = true;
  m_renderstate.lighting = true;
  m_renderstate.textured = true;

  m_config_options.do_multitexture_level = 0;
  m_config_options.do_extra_bright = false;
}

bool csGraphics3DOGLCommon::NewOpen ()
{
  CommonOpen ();

  if (!G2D->Open ())
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error opening Graphics2D context.");
    // set "not opened" flag
    width = height = -1;
    return false;
  }
  // See if we find any OpenGL extensions, and set the corresponding
  // flags. Look at the bottom
  // of ogl_g3d.h for known extensions (currently only multitexture)
#if USE_EXTENSIONS
  DetectExtensions ();
#endif

  if (m_renderstate.dither)
    glEnable (GL_DITHER);
  else
    glDisable (GL_DITHER);

  if (config->GetBool ("Video.OpenGL.HintPerspectiveFast", false))
    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
  else
    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  m_config_options.do_extra_bright = config->GetBool
        ("Video.OpenGL.ExtraBright", false);
  // determine what blend mode to use when combining lightmaps with
  // their  underlying textures.  This mode is set in the Opengl
  // configuration  file
  struct
  {
    char *blendstylename;
    GLenum srcblend, dstblend;
  }
  blendstyles[] =
  {
    { "multiplydouble", GL_DST_COLOR, GL_SRC_COLOR } ,
    { "multiply", GL_DST_COLOR, GL_ZERO } ,
    { "add", GL_ONE, GL_ONE } ,
    { "coloradd", GL_ONE, GL_SRC_COLOR } ,
    { "ps2tristage", (GLenum)696969, (GLenum)696969 } ,
    { NULL, GL_DST_COLOR, GL_ZERO }
  };

  // try to match user's blend name with a name in the blendstyles table
  const char *lightmapstyle = config->GetStr
        ("Video.OpenGL.LightmapMode","multiplydouble");
  int bl_idx = 0;
  while (blendstyles[bl_idx].blendstylename != NULL)
  {
    if (strcmp (lightmapstyle, blendstyles[bl_idx].blendstylename) == 0)
    {
      m_config_options.m_lightmap_src_blend = blendstyles[bl_idx].srcblend;
      m_config_options.m_lightmap_dst_blend = blendstyles[bl_idx].dstblend;
      break;
    }
    bl_idx++;
  }

#if USE_EXTENSIONS
  // check with the GL driver and see if it supports the multitexure
  // extension
  if (ARB_multitexture)
  {
    // if you support multitexture, you should allow more than one
    // texture, right?  Let's see how many we can get...
    GLint maxtextures;
    glGetIntegerv (GL_MAX_TEXTURE_UNITS_ARB, &maxtextures);

    if (maxtextures > 1)
    {
      m_config_options.do_multitexture_level = maxtextures;
      Report (CS_REPORTER_SEVERITY_NOTIFY,
      	"Using multitexture extension with %d texture units", maxtextures);
    }
    else
    {
      Report (CS_REPORTER_SEVERITY_NOTIFY,
      	"WARNING: driver supports multitexture extension but only allows one texture unit!");
    }
  }
#endif

  // tells OpenGL driver we align texture data on byte boundaries,
  // instead of perhaps word or dword boundaries
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

  // generate the exponential 1D texture for use in vertex fogging
  // this texture holds a 'table' of alpha values forming an exponential
  // curve, used for generating exponential fog by mapping it onto
  // fogged polygons as we draw them.
  unsigned char *transientfogdata = new unsigned char[CS_FOGTABLE_SIZE * 4];
  for (unsigned int fogindex = 0; fogindex < CS_FOGTABLE_SIZE; fogindex++)
  {
    transientfogdata[fogindex * 4 + 0] = (unsigned char) 255;
    transientfogdata[fogindex * 4 + 1] = (unsigned char) 255;
    transientfogdata[fogindex * 4 + 2] = (unsigned char) 255;
    double fogalpha = (256 * (1.0 - exp (-float (fogindex)
    	* CS_FOGTABLE_MAXDISTANCE / CS_FOGTABLE_SIZE)));
    transientfogdata[fogindex * 4 + 3] = (unsigned char) fogalpha;
  }
  // prevent weird effects when 0 distance fog wraps around to the
  // 'max fog' texel
  transientfogdata[(CS_FOGTABLE_SIZE - 1) * 4 + 3] = 0;

  // dump the fog table into an OpenGL texture for later user.
  // The texture is CS_FOGTABLE_SIZE texels wide and one texel high;
  // we could use a 1D texture but some OpenGL drivers don't
  // handle 1D textures very well
  glGenTextures (1, &m_fogtexturehandle);
  glBindTexture (GL_TEXTURE_2D, m_fogtexturehandle);

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D (GL_TEXTURE_2D, 0, 4, CS_FOGTABLE_SIZE, 1, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, transientfogdata);

  delete [] transientfogdata;

  glGetIntegerv (GL_MAX_TEXTURE_SIZE, &max_texture_size);
  // adjust max texture size if bigger than maxwidth/height from config
  if(Caps.maxTexWidth < max_texture_size)
    max_texture_size = Caps.maxTexWidth;
  if(Caps.maxTexHeight < max_texture_size)
    max_texture_size = Caps.maxTexHeight;

  int max_cache_size = 1024*1024*16; // 32mb combined cache
  texture_cache = new OpenGLTextureCache (max_cache_size, 24);
  lightmap_cache = new OpenGLLightmapCache (this);
  texture_cache->SetBilinearMapping (config->GetBool
        ("Video.OpenGL.EnableBilinearMap", true));

  GLenum errtest;
  errtest = glGetError ();
  if (errtest != GL_NO_ERROR)
  {
    //Report (CS_REPORTER_SEVERITY_DEBUG, "openGL error string: %s",
    	//gluErrorString (errtest));
    Report (CS_REPORTER_SEVERITY_DEBUG, "openGL error: %d", errtest);
  }

  // If blend style is 'auto' try to determine which mode to use by drawing
  // on the frame buffer. We check the results to see if the OpenGL driver
  // provides good support for multipledouble (2*SRC*DST) blend mode; if
  // not, fallback to the normal multiply blend mode.
  if (strcmp (lightmapstyle, "auto") == 0)
  {
    GLenum srcblend, dstblend;
    Guess_BlendMode (&srcblend, &dstblend);
    m_config_options.m_lightmap_src_blend = srcblend;
    m_config_options.m_lightmap_dst_blend = dstblend;
  }

  glCullFace (GL_FRONT);
  glEnable (GL_CULL_FACE);
  glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glDisable (GL_BLEND);

  // Now that we know what pixelformat we use, clue the texture manager in.
  txtmgr->SetPixelFormat (*G2D->GetPixelFormat ());

  PerfTest ();

  glCullFace (GL_FRONT);
  glEnable (GL_CULL_FACE);
  glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glDisable (GL_BLEND);

  return true;
}

void csGraphics3DOGLCommon::CommonOpen ()
{
  DrawMode = 0;
  width = G2D->GetWidth ();
  height = G2D->GetHeight ();
  asp_center_x = width / 2;
  asp_center_y = height / 2;
  SetDimensions (width, height);
  // default lightmap blend style
  m_config_options.m_lightmap_src_blend = GL_DST_COLOR;
  m_config_options.m_lightmap_dst_blend = GL_ZERO;
}

void csGraphics3DOGLCommon::SharedOpen (csGraphics3DOGLCommon *d)
{
  CommonOpen ();
  ARB_multitexture = d->ARB_multitexture;
  m_config_options.do_multitexture_level =
	d->m_config_options.do_multitexture_level;
  m_config_options.do_extra_bright = d->m_config_options.do_extra_bright;
  m_config_options.m_lightmap_src_blend =
  	d->m_config_options.m_lightmap_src_blend;
  m_config_options.m_lightmap_dst_blend =
  	d->m_config_options.m_lightmap_dst_blend;
  m_fogtexturehandle = d->m_fogtexturehandle;
  texture_cache = d->texture_cache;
  lightmap_cache = d->lightmap_cache;
}

void csGraphics3DOGLCommon::Close ()
{
  if ((width == height) && height == -1)
    return;

  // we should remove all texture handles before we kill the graphics context
  txtmgr->Clear();
  txtmgr->DecRef(); txtmgr = NULL;
  delete texture_cache; texture_cache = NULL;
  delete lightmap_cache; lightmap_cache = NULL;
  if (clipper)
  {
    clipper->DecRef ();
    clipper = NULL;
    cliptype = CS_CLIPPER_NONE;
  }

  if (m_fogtexturehandle)
  {
    glDeleteTextures (1, &m_fogtexturehandle);
    m_fogtexturehandle = 0;
  }
  // kill the graphics context
  if (G2D)
    G2D->Close ();

  width = height = -1;
}

void csGraphics3DOGLCommon::SetDimensions (int width, int height)
{
  csGraphics3DOGLCommon::width = width;
  csGraphics3DOGLCommon::height = height;
  csGraphics3DOGLCommon::asp_center_x = width / 2;
  csGraphics3DOGLCommon::asp_center_y = height / 2;
  frustum_valid = false;
}

void csGraphics3DOGLCommon::SetupClipPlanes (bool add_near_clip,
	bool add_z_clip)
{
  if (planes_init) return;
  planes_init = true;
  // This routine assumes the hardware planes can handle the
  // required number of planes from the clipper.
  if (clipper && GLCaps.nr_hardware_planes >= 0)
  {
    CalculateFrustum ();
    csPlane3 pl;
    GLdouble plane_eq[4];
    int i, i1;
    i1 = frustum.GetVertexCount ()-1;
    for (i = 0 ; i < frustum.GetVertexCount () ; i++)
    {
      pl.Set (csVector3 (0), frustum[i], frustum[i1]);
      plane_eq[0] = pl.A ();
      plane_eq[1] = pl.B ();
      plane_eq[2] = pl.C ();
      plane_eq[3] = pl.D ();
      glClipPlane ((GLenum)(GL_CLIP_PLANE0+i), plane_eq);
      i1 = i;
    }
    if (add_near_clip)
    {
      plane_eq[0] = -near_plane.A ();
      plane_eq[1] = -near_plane.B ();
      plane_eq[2] = -near_plane.C ();
      plane_eq[3] = -near_plane.D ();
      glClipPlane ((GLenum)(GL_CLIP_PLANE0+i), plane_eq);
      i++;
    }
    if (add_z_clip)
    {
      plane_eq[0] = 0;
      plane_eq[1] = 0;
      plane_eq[2] = 1;
      plane_eq[3] = -.001;
      glClipPlane ((GLenum)(GL_CLIP_PLANE0+i), plane_eq);
    }
  }
}

void csGraphics3DOGLCommon::CalculateFrustum ()
{
  if (frustum_valid) return;
  frustum_valid = true;
  if (clipper)
  {
    frustum.MakeEmpty ();
    int nv = clipper->GetVertexCount ();
    csVector3 v3;
    v3.z = 1;
    csVector2* v = clipper->GetClipPoly ();
    int i;
    for (i = 0 ; i < nv ; i++)
    {
      v3.x = (v[i].x - asp_center_x) * inv_aspect;
      v3.y = (v[i].y - asp_center_y) * inv_aspect;
      frustum.AddVertex (v3);
    }
  }
}

void csGraphics3DOGLCommon::SetClipper (iClipper2D* clip, int cliptype)
{
  if (clip) clip->IncRef ();
  if (clipper) clipper->DecRef ();
  clipper = clip;
  if (!clipper) cliptype = CS_CLIPPER_NONE;
  csGraphics3DOGLCommon::cliptype = cliptype;
  frustum_valid = false;
  stencil_init = false;
  planes_init = false;

#if 0
// @@@ TODO: init z-buffer
  if (!toplevel_init && cliptype == CS_CLIPPER_TOPLEVEL)
  {
    // We have a toplevel clipper and we didn't initialize it yet.
    // In this case we will update the Z-buffer around the top-level
    // portal so that we don't need clipping for DrawTriangleMesh.
    // @@@ SideNote! This toplevel_init could cause problems for multiple
    // views. Have to think about this.
    toplevel_init = true;
    int nv = clipper->GetVertexCount ();
    csVector2* v = clipper->GetClipPoly ();
    int i, i1;
    i1 = nv-1;
    for (i = 0 ; i < nv ; i++)
    {
      i1 = i;
    }
  }
#endif
}

bool csGraphics3DOGLCommon::BeginDraw (int DrawFlags)
{
  if ((G2D->GetWidth() != width) ||

      (G2D->GetHeight() != height))

    SetDimensions (G2D->GetWidth(), G2D->GetHeight());



  if (DrawMode & CSDRAW_3DGRAPHICS)
  {
    FlushDrawPolygon ();
    lightmap_cache->Flush ();
    FlushDrawFog ();
  }

  // If we go to 2D mode then we do as if several modes are disabled.
  // 2D operations don't like blending nor the z-buffer.
  if (DrawFlags & CSDRAW_2DGRAPHICS)
  {
    SetupBlend (CS_FX_COPY, 0, false);
    SetGLZBufferFlags (CS_ZBUF_NONE);
  }

  // if 2D graphics is not locked, lock it
  if ((DrawFlags & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
   && (!(DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))))
  {
    if (!G2D->BeginDraw ())
      return false;
  }

  if (DrawFlags & CSDRAW_CLEARZBUFFER)
  {
    glDepthMask (GL_TRUE);
    if (DrawFlags & CSDRAW_CLEARSCREEN)
      glClear (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    else
      glClear (GL_DEPTH_BUFFER_BIT);
  }
  else if (DrawFlags & CSDRAW_CLEARSCREEN)
    G2D->Clear (0);

  DrawMode = DrawFlags;

  toplevel_init = false;

  return true;
}

void csGraphics3DOGLCommon::FinishDraw ()
{
  //if (DrawMode & CSDRAW_3DGRAPHICS)
  {
    FlushDrawPolygon ();
    lightmap_cache->Flush ();
    FlushDrawFog ();
  }

  if (DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
  {
    G2D->FinishDraw ();
  }
  DrawMode = 0;
}

void csGraphics3DOGLCommon::Print (csRect * area)
{
  if (fps_limit)
  {
    csTicks elapsed_time, current_time;
    iSystem* sys = CS_GET_SYSTEM (object_reg);	//@@@
    sys->GetElapsedTime (elapsed_time, current_time);
    /// Smooth last n frames, to avoid jitter when objects appear/disappear.
    static int num = 10;
    static int times[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    static int cur = 0;
    static int totaltime = 0;
    totaltime -= times[cur];
    times[cur] = elapsed_time;
    totaltime += times[cur];
    cur = (cur+1)%num;
    if (totaltime/10 < fps_limit) sys->Sleep (fps_limit - totaltime/10);
  }
  G2D->Print (area);
}

void csGraphics3DOGLCommon::DebugDrawElements (iGraphics2D* g2d,
	int num_tri3, int* tris,
  	GLfloat* verts, int color, bool coords3d, bool transformed)
{
  glPushAttrib (GL_ENABLE_BIT|GL_COLOR_BUFFER_BIT|GL_CURRENT_BIT|
  	GL_DEPTH_BUFFER_BIT);
  glDisable (GL_DEPTH_TEST);
  glDisable (GL_BLEND);
  num_tri3 /= 3;
  int i;
  float x1, y1, x2, y2, x3, y3;
  for (i = 0 ; i < num_tri3 ; i++)
  {
    int a = *tris++;
    int b = *tris++;
    int c = *tris++;
    if (!coords3d)
    {
      // We have 2D projected coordinates (in 4 floats).
      a <<= 2;
      b <<= 2;
      c <<= 2;
      x1 = verts[a] / verts[a+3];
      y1 = ogl_g3d->height - verts[a+1] / verts[a+3];
      x2 = verts[b] / verts[b+3];
      y2 = ogl_g3d->height - verts[b+1] / verts[b+3];
      x3 = verts[c] / verts[c+3];
      y3 = ogl_g3d->height - verts[c+1] / verts[c+3];
    }
    else
    {
      a *= 3;
      b *= 3;
      c *= 3;
      csVector3 va, vb, vc;
      va.Set (verts[a], verts[a+1], verts[a+2]);
      vb.Set (verts[b], verts[b+1], verts[b+2]);
      vc.Set (verts[c], verts[c+1], verts[c+2]);
      if (transformed)
      {
        // We have 3D coordinates that are already transformed to camera space
        // (in 3 floats).
      }
      else
      {
        // We have 3D coordinates that are not transformed to camera space
        // (in 3 floats).
        va = ogl_g3d->o2c.Other2This (va);
        vb = ogl_g3d->o2c.Other2This (vb);
        vc = ogl_g3d->o2c.Other2This (vc);
      }
      float iz;
      iz = ogl_g3d->aspect / va.z;
      x1 = va.x * iz + ogl_g3d->asp_center_x;
      y1 = ogl_g3d->height - va.y * iz - ogl_g3d->asp_center_y;
      iz = ogl_g3d->aspect / vb.z;
      x2 = vb.x * iz + ogl_g3d->asp_center_x;
      y2 = ogl_g3d->height - vb.y * iz - ogl_g3d->asp_center_y;
      iz = ogl_g3d->aspect / vc.z;
      x3 = vc.x * iz + ogl_g3d->asp_center_x;
      y3 = ogl_g3d->height - vc.y * iz - ogl_g3d->asp_center_y;
    }
    g2d->DrawLine (x1, y1, x2, y2, color);
    g2d->DrawLine (x2, y2, x3, y3, color);
    g2d->DrawLine (x3, y3, x1, y1, color);
  }
  glPopAttrib ();
}

static float GetAlpha (UInt mode, float m_alpha, bool txt_alpha)
{
  switch (mode & CS_FX_MASK_MIXMODE)
  {
    case CS_FX_MULTIPLY:
      // Color = SRC * DEST +   0 * SRC = DEST * SRC
      m_alpha = 1.0f;
      break;
    case CS_FX_MULTIPLY2:
      // Color = SRC * DEST + DEST * SRC = 2 * DEST * SRC
      m_alpha = 1.0f;
      break;
    case CS_FX_ADD:
      // Color = 1 * DEST + 1 * SRC = DEST + SRC
      m_alpha = 1.0f;
      break;
    case CS_FX_ALPHA:
      // Color = Alpha * DEST + (1-Alpha) * SRC
      break;
    case CS_FX_TRANSPARENT:
      // Color = 1 * DEST + 0 * SRC
      m_alpha = 0.0f;
      break;
    case CS_FX_COPY:
    default:
      // Color = 0 * DEST + 1 * SRC = SRC
      if (txt_alpha)
        m_alpha = 1.0f;
      else
        m_alpha = 0;
      break;
  }
  return m_alpha;
}


static UInt prev_mode = 0xffffffff; //@@@ Move to class (static).
static bool prev_txt_alpha = false;

float csGraphics3DOGLCommon::SetupBlend (UInt mode,
	float m_alpha, bool txt_alpha)
{
  if (prev_mode == mode && prev_txt_alpha == txt_alpha)
    return GetAlpha (mode, m_alpha, txt_alpha);
  prev_mode = mode;
  prev_txt_alpha = txt_alpha;
  static bool blend_enabled = false;

  // Note: In all explanations of Mixing:
  // Color: resulting color
  // SRC:   Color of the texel (content of the texture to be drawn)
  // DEST:  Color of the pixel on screen
  // Alpha: Alpha value of the polygon
  bool enable_blending = true;
  switch (mode & (CS_FX_MASK_MIXMODE | CS_FX_EXTRA_MODES))
  {
    case CS_FX_SRCDST:
      glBlendFunc (ogl_g3d->m_config_options.m_lightmap_src_blend,
	           ogl_g3d->m_config_options.m_lightmap_dst_blend);
      break;
    case CS_FX_HALOOVF:
      glBlendFunc (GL_SRC_ALPHA, GL_ONE);
      break;
    case CS_FX_MULTIPLY:
      // Color = SRC * DEST +   0 * SRC = DEST * SRC
      m_alpha = 1.0f;
      glBlendFunc (GL_ZERO, GL_SRC_COLOR);
      break;
    case CS_FX_MULTIPLY2:
      // Color = SRC * DEST + DEST * SRC = 2 * DEST * SRC
      m_alpha = 1.0f;
      glBlendFunc (GL_DST_COLOR, GL_SRC_COLOR);
      break;
    case CS_FX_ADD:
      // Color = 1 * DEST + 1 * SRC = DEST + SRC
      m_alpha = 1.0f;
      glBlendFunc (GL_ONE, GL_ONE);
      break;
    case CS_FX_ALPHA:
      // Color = Alpha * DEST + (1-Alpha) * SRC
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      break;
    case CS_FX_TRANSPARENT:
      // Color = 1 * DEST + 0 * SRC
      m_alpha = 0.0f;
      glBlendFunc (GL_ZERO, GL_ONE);
      break;
    case CS_FX_COPY:
    default:
      enable_blending = txt_alpha;
      if (txt_alpha)
      {
        // Color = 0 * DEST + 1 * SRC = SRC
        m_alpha = 1.0f;
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      }
      else
      {
	m_alpha = 0;
      }
      break;
  }

  if (enable_blending)
  {
    if (!blend_enabled) { glEnable (GL_BLEND); blend_enabled = true; }
  }
  else
  {
    if (blend_enabled) { glDisable (GL_BLEND); blend_enabled = false; }
  }
  return m_alpha;
}

UInt prev_ct = 0;	// @@@ Move to class (static).

void csGraphics3DOGLCommon::SetClientStates (UInt ct)
{
  if (prev_ct == ct) return;

  if (!(prev_ct & CS_CLIENTSTATE_COLOR_ARRAY) &&
  	(ct & CS_CLIENTSTATE_COLOR_ARRAY))
    glEnableClientState (GL_COLOR_ARRAY);
  else if (!(ct & CS_CLIENTSTATE_COLOR_ARRAY) &&
  	(prev_ct & CS_CLIENTSTATE_COLOR_ARRAY))
    glDisableClientState (GL_COLOR_ARRAY);

  if (!(prev_ct & CS_CLIENTSTATE_VERTEX_ARRAY) &&
  	(ct & CS_CLIENTSTATE_VERTEX_ARRAY))
    glEnableClientState (GL_VERTEX_ARRAY);
  else if (!(ct & CS_CLIENTSTATE_VERTEX_ARRAY) &&
  	(prev_ct & CS_CLIENTSTATE_VERTEX_ARRAY))
    glDisableClientState (GL_VERTEX_ARRAY);

  if (!(prev_ct & CS_CLIENTSTATE_TEXTURE_COORD_ARRAY) &&
  	(ct & CS_CLIENTSTATE_TEXTURE_COORD_ARRAY))
    glEnableClientState (GL_TEXTURE_COORD_ARRAY);
  else if (!(ct & CS_CLIENTSTATE_TEXTURE_COORD_ARRAY) &&
  	(prev_ct & CS_CLIENTSTATE_TEXTURE_COORD_ARRAY))
    glDisableClientState (GL_TEXTURE_COORD_ARRAY);

  prev_ct = ct;
}

static bool mirror_mode = false;

void csGraphics3DOGLCommon::SetMirrorMode (bool mirror)
{
  if (mirror == mirror_mode)
    return;
  mirror_mode = mirror;
  if (mirror)
    glCullFace (GL_BACK);
  else
    glCullFace (GL_FRONT);
}

void csGraphics3DOGLCommon::SetupStencil ()
{
  if (stencil_init) return;
  stencil_init = true;
  if (clipper && GLCaps.use_stencil)
  {
    // First set up the stencil area.
    glEnable (GL_STENCIL_TEST);
    glClearStencil (0);
    glClear (GL_STENCIL_BUFFER_BIT);
    glStencilFunc (GL_ALWAYS, 1, 1);
    glStencilOp (GL_REPLACE, GL_REPLACE, GL_REPLACE);
    int nv = clipper->GetVertexCount ();
    csVector2* v = clipper->GetClipPoly ();
    glColor4f (0, 0, 0, 0);
    glShadeModel (GL_FLAT);
    SetGLZBufferFlags (CS_ZBUF_NONE);
    glDisable (GL_TEXTURE_2D);
    SetupBlend (CS_FX_TRANSPARENT, 0, false);
    glBegin (GL_TRIANGLE_FAN);
    for (int i = 0 ; i < nv ; i++)
      glVertex2f (v[i].x, v[i].y);
    glEnd ();
    glDisable (GL_STENCIL_TEST);
  }
}

void csGraphics3DOGLCommon::FlushDrawFog ()
{
  if (fog_queue.num_triangles <= 0) return;

  SetGLZBufferFlagsPass2 (fog_queue.z_buf_mode, true);

  glEnable (GL_TEXTURE_2D);
  glBindTexture (GL_TEXTURE_2D, m_fogtexturehandle);
  glShadeModel (GL_SMOOTH);
  SetupBlend (CS_FX_ALPHA, 0, false);

  SetClientStates (CS_CLIENTSTATE_ALL);
  glColorPointer (3, GL_FLOAT, 0, fog_queue.fog_color);
  glVertexPointer (4, GL_FLOAT, 0, fog_queue.glverts);
  glTexCoordPointer (2, GL_FLOAT, 0, fog_queue.fog_txt);
  glDrawElements (GL_TRIANGLES, fog_queue.num_triangles*3, GL_UNSIGNED_INT,
  	  fog_queue.tris);

  fog_queue.Reset ();
}

void csGraphics3DOGLCommon::FlushDrawPolygon ()
{
  if (queue.num_triangles <= 0) return;

  int i, j;
  csMaterialHandle* mat_handle = (csMaterialHandle*)queue.mat_handle;
  iTextureHandle* txt_handle = NULL;
  csTextureHandleOpenGL *txt_mm = NULL;
  csTxtCacheData *texturecache_data = NULL;
  GLuint texturehandle = 0;
  bool multimat = false;
  bool tex_transp = false;
  bool gouraud = (queue.mixmode & CS_FX_GOURAUD) != 0;

  if (mat_handle)
  {
    multimat = mat_handle->GetTextureLayerCount () > 0;
    txt_handle = mat_handle->GetTexture ();
    txt_mm = (csTextureHandleOpenGL *)txt_handle->GetPrivateObject ();
    tex_transp = txt_mm->GetKeyColor () || txt_mm->GetAlphaMap ();
    // Initialize our static drawing information and cache
    // the texture in the texture cache (if this is not already the case).
    CacheTexture (queue.mat_handle);
    texturecache_data = (csTxtCacheData *)txt_mm->GetCacheData ();
    texturehandle = texturecache_data->Handle;
  }

  float flat_r = queue.flat_color_r;
  float flat_g = queue.flat_color_g;
  float flat_b = queue.flat_color_b;

  if (m_renderstate.textured && mat_handle)
    glEnable (GL_TEXTURE_2D);
  else
  {
    glDisable (GL_TEXTURE_2D);
    if (mat_handle)
    {
      UByte r, g, b;
      mat_handle->GetTexture ()->GetMeanColor (r, g, b);
      flat_r = BYTE_TO_FLOAT (r);
      flat_g = BYTE_TO_FLOAT (g);
      flat_b = BYTE_TO_FLOAT (b);
    }
  }

  SetGLZBufferFlags (queue.z_buf_mode);

  float alpha = 1.0f - BYTE_TO_FLOAT (queue.mixmode & CS_FX_MASK_ALPHA);
  alpha = SetupBlend (queue.mixmode, alpha, tex_transp);
  glColor4f (flat_r, flat_g, flat_b, alpha);

  if (mat_handle)
    glBindTexture (GL_TEXTURE_2D, texturehandle);

  //=================
  // Pass 1: The unlit texture with optional gouraud shading.
  // Gouraud shading will be delayed to after the multiple texture layers
  // if we have them.
  //=================
  // Only do gouraud shading in the first pass if we don't have
  // extra material layers.
  if (gouraud && !multimat)
  {
    glShadeModel (GL_SMOOTH);
    SetClientStates (CS_CLIENTSTATE_ALL);
    glColorPointer (4, GL_FLOAT, 0, queue.glcol);
  }
  else
  {
    SetClientStates (CS_CLIENTSTATE_VT);
    glShadeModel (GL_FLAT);
  }
  glVertexPointer (4, GL_FLOAT, 0, queue.glverts);
  glTexCoordPointer (2, GL_FLOAT, 0, queue.gltxt);
  glDrawElements (GL_TRIANGLES, queue.num_triangles*3, GL_UNSIGNED_INT,
  	  queue.tris);
  if (gouraud && !multimat)
  {
    glShadeModel (GL_FLAT);
  }

  //=================
  // If we have need other texture passes (for whatever reason)
  // we set the z-buffer to second pass state.
  //=================
  if (m_config_options.do_extra_bright ||
      /*@@@queue.use_fog ||*/ multimat)
  {
    SetGLZBufferFlagsPass2 (queue.z_buf_mode, true);
  }

  //=================
  // Pass 2: Here we add all extra texture layers if there are some.
  // If gouraud shading is needed we do that at the end here.
  //=================
  if (multimat)
  {
    for (j = 0 ; j < mat_handle->GetTextureLayerCount () ; j++)
    {
      csTextureLayer* layer = mat_handle->GetTextureLayer (j);
      iTextureHandle* txt_handle = layer->txt_handle;
      csTextureHandleOpenGL *txt_mm = (csTextureHandleOpenGL *)
    	  txt_handle->GetPrivateObject ();
      csTxtCacheData *texturecache_data;
      texturecache_data = (csTxtCacheData *)txt_mm->GetCacheData ();
      tex_transp = txt_mm->GetKeyColor () || txt_mm->GetAlphaMap ();
      GLuint texturehandle = texturecache_data->Handle;
      glBindTexture (GL_TEXTURE_2D, texturehandle);

      float alpha = 1.0f - BYTE_TO_FLOAT (layer->mode & CS_FX_MASK_ALPHA);
      alpha = SetupBlend (layer->mode, alpha, tex_transp);
      glColor4f (1., 1., 1., alpha);
      GLfloat* p_gltxt;
      if (mat_handle->TextureLayerTranslated (j))
      {
        GLfloat* src = queue.gltxt;
        GLfloat* dst = queue.layer_gltxt;
        float uscale = layer->uscale;
        float vscale = layer->vscale;
        float ushift = layer->ushift;
        float vshift = layer->vshift;
        for (i = 0 ; i < queue.num_vertices ; i++)
        {
	  *dst++ = (*src++) * uscale + ushift;
	  *dst++ = (*src++) * vscale + vshift;
        }

        p_gltxt = queue.layer_gltxt;
      }
      else
      {
        p_gltxt = queue.gltxt;
      }
      glTexCoordPointer (2, GL_FLOAT, 0, p_gltxt);
      glVertexPointer (4, GL_FLOAT, 0, queue.glverts);
      glDrawElements (GL_TRIANGLES, queue.num_triangles*3, GL_UNSIGNED_INT,
  	  queue.tris);
    }

    // If we have to do gouraud shading and we have multiple texture layers
    // then this is the right time to do this.
    if (gouraud)
    {
      glDisable (GL_TEXTURE_2D);
      glShadeModel (GL_SMOOTH);
      SetupBlend (CS_FX_MULTIPLY2, 0, false);

      SetClientStates (CS_CLIENTSTATE_VC);
      glColorPointer (4, GL_FLOAT, 0, queue.glcol);
      glVertexPointer (4, GL_FLOAT, 0, queue.glverts);
      glDrawElements (GL_TRIANGLES, queue.num_triangles*3, GL_UNSIGNED_INT,
  	  queue.tris);
    }
  }

  if (debug_edges)
    DebugDrawElements (G2D,
	queue.num_triangles*3, queue.tris, queue.glverts,
		txtmgr->FindRGB (255, 255, 255), false, false);

#if 0
  //@@@ TEMPORARILY DISABLED
  //=================
  // Pass 3: an extra optional pass which improves the lighting on SRC*DST
  // so that it looks more like 2*SRC*DST.
  //=================
  if (m_config_options.do_extra_bright)
  {
    glDisable (GL_TEXTURE_2D);
    glShadeModel (GL_SMOOTH);
    glEnable (GL_BLEND);
    glBlendFunc (GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
    //@@@ INVALIDATE BLEND MODE!
    // glBlendFunc (GL_ZERO, GL_SRC_COLOR);

    glColor4f (2, 2, 2, 0);
    glVertexPointer (4, GL_FLOAT, 0, queue.glverts);
    glDrawElements (GL_TRIANGLES, queue.num_triangles*3, GL_UNSIGNED_INT,
  	  queue.tris);
  }
#endif

  queue.Reset ();
}

int csFogQueue::AddVertices (int num)
{
  num_vertices += num;
  if (num_vertices > max_vertices)
  {
    GLfloat* new_ar;
    int old_num = num_vertices-num;
    max_vertices += 40;

    new_ar = new GLfloat [max_vertices*4];
    if (glverts) memcpy (new_ar, glverts, sizeof (GLfloat)*4*old_num);
    delete[] glverts; glverts = new_ar;

    new_ar = new GLfloat [max_vertices*3];
    if (fog_color) memcpy (new_ar, fog_color, sizeof (GLfloat)*3*old_num);
    delete[] fog_color; fog_color = new_ar;

    new_ar = new GLfloat [max_vertices*2];
    if (fog_txt) memcpy (new_ar, fog_txt, sizeof (GLfloat)*2*old_num);
    delete[] fog_txt; fog_txt = new_ar;
  }
  return num_vertices-num;
}

void csFogQueue::AddTriangle (int i1, int i2, int i3)
{
  int old_num = num_triangles;
  num_triangles++;
  if (num_triangles > max_triangles)
  {
    max_triangles += 20;
    int* new_ar;
    new_ar = new int [max_triangles*3];
    if (tris) memcpy (new_ar, tris, sizeof (int) * 3 * old_num);
    delete[] tris; tris = new_ar;
  }
  tris[old_num*3+0] = i1;
  tris[old_num*3+1] = i2;
  tris[old_num*3+2] = i3;
}

int csPolyQueue::AddVertices (int num)
{
  CS_ASSERT (num_vertices >= 0 && num_vertices <= max_vertices);
  num_vertices += num;
  if (num_vertices > max_vertices)
  {
    GLfloat* new_ar;
    int old_num = num_vertices-num;
    max_vertices += 40;

    new_ar = new GLfloat [max_vertices*4];
    if (glverts) memcpy (new_ar, glverts, sizeof (GLfloat)*4*old_num);
    delete[] glverts; glverts = new_ar;

    new_ar = new GLfloat [max_vertices*2];
    if (gltxt) memcpy (new_ar, gltxt, sizeof (GLfloat)*2*old_num);
    delete[] gltxt; gltxt = new_ar;

    new_ar = new GLfloat [max_vertices*4];
    if (glcol) memcpy (new_ar, glcol, sizeof (GLfloat)*4*old_num);
    delete[] glcol; glcol = new_ar;

    new_ar = new GLfloat [max_vertices*2];
    if (layer_gltxt) memcpy (new_ar, layer_gltxt, sizeof (GLfloat)*2*old_num);
    delete[] layer_gltxt; layer_gltxt = new_ar;
  }
  return num_vertices-num;
}

void csPolyQueue::AddTriangle (int i1, int i2, int i3)
{
  CS_ASSERT (num_triangles >= 0 && num_triangles <= max_triangles);
  int old_num = num_triangles;
  num_triangles++;
  if (num_triangles > max_triangles)
  {
    max_triangles += 20;
    int* new_ar;
    new_ar = new int [max_triangles*3];
    if (tris)
    {
      CS_ASSERT (old_num > 0);
      memcpy (new_ar, tris, sizeof (int) * 3 * old_num);
    }
    delete[] tris; tris = new_ar;
  }
  tris[old_num*3+0] = i1;
  tris[old_num*3+1] = i2;
  tris[old_num*3+2] = i3;
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

void csGraphics3DOGLCommon::DrawPolygonSingleTexture (G3DPolygonDP& poly)
{
  if (poly.num < 3) return;

  int i;
  float P1, P2, P3, P4;
  float Q1, Q2, Q3, Q4;

  // count 'real' number of vertices
  int num_vertices = 1;
  for (i = 1; i < poly.num; i++)
  {
    if ((ABS (poly.vertices[i].sx - poly.vertices[i - 1].sx)
	 + ABS (poly.vertices[i].sy - poly.vertices[i - 1].sy))
	 	> VERTEX_NEAR_THRESHOLD)
      num_vertices++;
  }
  // if this is a 'degenerate' polygon, skip it
  if (num_vertices < 3)
    return;

  float flat_r = 1., flat_g = 1., flat_b = 1.;
  
  //========
  // First check if this polygon is different from the current polygons
  // in the queue. If so we need to flush the queue.
  //========
  if (poly.mat_handle != queue.mat_handle ||
      poly.mixmode != queue.mixmode ||
      z_buf_mode != queue.z_buf_mode ||
      flat_r != queue.flat_color_r ||
      flat_g != queue.flat_color_g ||
      flat_b != queue.flat_color_b)
  {
    FlushDrawPolygon ();
    lightmap_cache->FlushIfNeeded ();
    if (!CompatibleZBufModes (fog_queue.z_buf_mode, z_buf_mode))
      FlushDrawFog ();
  }

  //========
  // Store information in the queue.
  //========
  queue.mat_handle = poly.mat_handle;
  queue.mixmode = poly.mixmode;
  queue.z_buf_mode = z_buf_mode;
  queue.flat_color_r = flat_r;
  queue.flat_color_g = flat_g;
  queue.flat_color_b = flat_b;

  // Get the plane normal of the polygon. Using this we can calculate
  // '1/z' at every screen space point.
  float Ac, Bc, Cc, Dc, inv_Dc;
  Ac = poly.normal.A ();
  Bc = poly.normal.B ();
  Cc = poly.normal.C ();
  Dc = poly.normal.D ();

  float M, N, O;
  if (ABS (Dc) < SMALL_D)
  {
    // The Dc component of the plane normal is too small. This means
    // that  the plane of the polygon is almost perpendicular to the
    // eye of the viewer. In this case, nothing much can be seen of
    // the plane anyway so we just take one value for the entire
    // polygon.
    M = 0;
    N = 0;
    // For O choose the transformed z value of one vertex.
    // That way Z buffering should at least work.
    O = 1 / poly.z_value;
  }
  else
  {
    inv_Dc = 1 / Dc;
    M = -Ac * inv_Dc * inv_aspect;
    N = -Bc * inv_Dc * inv_aspect;
    O = -Cc * inv_Dc;
  }

  // @@@ The texture transform matrix is currently written as
  // T = M*(C-V)
  // (with V being the transform vector, M the transform matrix, and C
  // the position in camera space coordinates. It would be better (more
  // suitable for the following calculations) if it would be written
  // as T = M*C - V.
  P1 = poly.plane.m_cam2tex->m11;
  P2 = poly.plane.m_cam2tex->m12;
  P3 = poly.plane.m_cam2tex->m13;
  P4 = -(P1 * poly.plane.v_cam2tex->x
	 + P2 * poly.plane.v_cam2tex->y
	 + P3 * poly.plane.v_cam2tex->z);
  Q1 = poly.plane.m_cam2tex->m21;
  Q2 = poly.plane.m_cam2tex->m22;
  Q3 = poly.plane.m_cam2tex->m23;
  Q4 = -(Q1 * poly.plane.v_cam2tex->x
	 + Q2 * poly.plane.v_cam2tex->y
	 + Q3 * poly.plane.v_cam2tex->z);

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
    J3 = P3 + P4 * O;
    K1 = Q1 * inv_aspect + Q4 * M;
    K2 = Q2 * inv_aspect + Q4 * N;
    K3 = Q3 + Q4 * O;
  }

  float sx, sy, sz, one_over_sz, u_over_sz, v_over_sz;

  int idx = queue.AddVertices (poly.num);
  GLfloat* glverts = queue.GetGLVerts (idx);
  GLfloat* gltxt = queue.GetGLTxt (idx);
  for (i = 0; i < poly.num; i++)
  {
    sx = poly.vertices[i].sx - asp_center_x;
    sy = poly.vertices[i].sy - asp_center_y;
    one_over_sz = M * sx + N * sy + O;
    sz = 1.0 / one_over_sz;
    u_over_sz = (J1 * sx + J2 * sy + J3);
    v_over_sz = (K1 * sx + K2 * sy + K3);

    // Modified to use homogenous object space coordinates instead
    // of homogenous texture space coordinates.
    *gltxt++ = u_over_sz * sz;
    *gltxt++ = v_over_sz * sz;
    *glverts++ = poly.vertices[i].sx * sz;
    *glverts++ = poly.vertices[i].sy * sz;
    *glverts++ = -1.0;
    *glverts++ = sz;
  }

  // Triangulate.
  for (i = 2 ; i < poly.num ; i++)
  {
    queue.AddTriangle (idx+0, idx+i-1, idx+i);
  }

  iPolygonTexture *tex = poly.poly_texture;
  iLightMap* lm = tex->GetLightMap ();
  if (m_renderstate.lighting && lm)
  {
    lightmap_cache->Cache (tex);
    csLMCacheData* clm = (csLMCacheData *)lm->GetCacheData ();
    if (clm)
    {
      csLightMapQueue* lm_queue = lightmap_cache->GetQueue (clm);
      int lm_idx = lm_queue->AddVertices (poly.num);

      // Copy vertex info.
      GLfloat* glverts = queue.GetGLVerts (idx);
      GLfloat* lm_glverts = lm_queue->GetGLVerts (lm_idx);
      memcpy (lm_glverts, glverts, poly.num*sizeof (GLfloat)*4);

      // Copy lightmap texture info.
      float lm_scale_u = clm->lm_scale_u;
      float lm_scale_v = clm->lm_scale_v;
      float lm_offset_u = clm->lm_offset_u;
      float lm_offset_v = clm->lm_offset_v;
      GLfloat* gltxt = queue.GetGLTxt (idx);
      GLfloat* lm_gltxt = lm_queue->GetGLTxt (lm_idx);
      for (i = 0; i < poly.num; i++)
      {
        *lm_gltxt++ = (*gltxt++ - lm_offset_u) * lm_scale_u;
        *lm_gltxt++ = (*gltxt++ - lm_offset_v) * lm_scale_v;
      }

      // Triangulate.
      for (i = 2 ; i < poly.num ; i++)
      {
        lm_queue->AddTriangle (lm_idx+0, lm_idx+i-1, lm_idx+i);
      }
    }
  }

  if (poly.use_fog)
  {
    fog_queue.z_buf_mode = z_buf_mode;
    int fog_idx = fog_queue.AddVertices (poly.num);

    // Copy vertex info.
    GLfloat* glverts = queue.GetGLVerts (idx);
    GLfloat* fog_glverts = fog_queue.GetGLVerts (fog_idx);
    memcpy (fog_glverts, glverts, poly.num*sizeof (GLfloat)*4);
    GLfloat* fog_color = fog_queue.GetFogColor (fog_idx);
    GLfloat* fog_txt = fog_queue.GetFogTxt (fog_idx);
    for (i = 0 ; i < poly.num ; i++)
    {
      *fog_color++ = poly.fog_info[i].r;
      *fog_color++ = poly.fog_info[i].g;
      *fog_color++ = poly.fog_info[i].b;

      *fog_txt++ = poly.fog_info[i].intensity;
      *fog_txt++ = 0.0;
    }

    // Triangulate.
    for (i = 2 ; i < poly.num ; i++)
    {
      fog_queue.AddTriangle (fog_idx+0, fog_idx+i-1, fog_idx+i);
    }
  }
}

void csGraphics3DOGLCommon::DrawPolygonZFill (G3DPolygonDP & poly)
{
  if (poly.num < 3)
    return;

  int i;

  // count 'real' number of vertices
  int num_vertices = 1;
  for (i = 1; i < poly.num; i++)
  {
    if ((ABS (poly.vertices[i].sx - poly.vertices[i - 1].sx)
	 + ABS (poly.vertices[i].sy - poly.vertices[i - 1].sy))
	 	> VERTEX_NEAR_THRESHOLD)
      num_vertices++;
  }
  // if this is a 'degenerate' polygon, skip it
  if (num_vertices < 3)
    return;

  FlushDrawPolygon ();
  lightmap_cache->Flush ();
  FlushDrawFog ();

  // Get the plane normal of the polygon. Using this we can calculate
  // '1/z' at every screen space point.
  float Ac, Bc, Cc, Dc, inv_Dc;
  Ac = poly.normal.A ();
  Bc = poly.normal.B ();
  Cc = poly.normal.C ();
  Dc = poly.normal.D ();

  float M, N, O;
  if (ABS (Dc) < SMALL_D)
  {
    // The Dc component of the plane normal is too small. This means
    // that  the plane of the polygon is almost perpendicular to the
    // eye of the viewer. In this case, nothing much can be seen of
    // the plane anyway so we just take one value for the entire
    // polygon.
    M = 0;
    N = 0;
    // For O choose the transformed z value of one vertex.
    // That way Z buffering should at least work.
    O = 1 / poly.z_value;
  }
  else
  {
    inv_Dc = 1 / Dc;
    M = -Ac * inv_Dc * inv_aspect;
    N = -Bc * inv_Dc * inv_aspect;
    O = -Cc * inv_Dc;
  }

  glDisable (GL_TEXTURE_2D);
  glShadeModel (GL_FLAT);
  SetGLZBufferFlags (z_buf_mode);
  SetupBlend (CS_FX_TRANSPARENT, 0, false);

  // First copy all data in an array so that we can minimize
  // the amount of code that goes between glBegin/glEnd. This
  // is from an OpenGL high-performance FAQ.
  // @@@ HARDCODED LIMIT OF 64 VERTICES!
  static GLfloat glverts[4*64];
  int vtidx = 0;
  float sx, sy, sz, one_over_sz;
  for (i = 0; i < poly.num; i++)
  {
    sx = poly.vertices[i].sx - asp_center_x;
    sy = poly.vertices[i].sy - asp_center_y;
    one_over_sz = M * sx + N * sy + O;
    sz = 1.0 / one_over_sz;
    glverts[vtidx++] = poly.vertices[i].sx * sz;
    glverts[vtidx++] = poly.vertices[i].sy * sz;
    glverts[vtidx++] = -1.0;
    glverts[vtidx++] = sz;
  }

  GLfloat* p_glverts;
  p_glverts = glverts;

  glBegin (GL_TRIANGLE_FAN);
  for (i = 0 ; i < poly.num ; i++)
  {
    glVertex4fv (p_glverts); p_glverts += 4;
  }
  glEnd ();
}

void csGraphics3DOGLCommon::DrawPolygonDebug (G3DPolygonDP &/* poly */ )
{
}

void csGraphics3DOGLCommon::DrawPolygonFX (G3DPolygonDPFX & poly)
{
  if (poly.num < 3) return;

  float flat_r = 1., flat_g = 1., flat_b = 1.;
  if (poly.mat_handle == NULL)
  {
    flat_r = BYTE_TO_FLOAT (poly.flat_color_r);
    flat_g = BYTE_TO_FLOAT (poly.flat_color_g);
    flat_b = BYTE_TO_FLOAT (poly.flat_color_b);
  }

  //========
  // First check if this polygon is different from the current polygons
  // in the queue. If so we need to flush the queue.
  //========
  if (poly.mat_handle != queue.mat_handle ||
      poly.mixmode != queue.mixmode ||
      z_buf_mode != queue.z_buf_mode ||
      flat_r != queue.flat_color_r ||
      flat_g != queue.flat_color_g ||
      flat_b != queue.flat_color_b)
  {
    FlushDrawPolygon ();
    lightmap_cache->FlushIfNeeded ();
    if (!CompatibleZBufModes (fog_queue.z_buf_mode, z_buf_mode))
      FlushDrawFog ();
  }

  //========
  // Store information in the queue.
  //========
  queue.mat_handle = poly.mat_handle;
  queue.mixmode = poly.mixmode;
  queue.z_buf_mode = z_buf_mode;
  queue.flat_color_r = flat_r;
  queue.flat_color_g = flat_g;
  queue.flat_color_b = flat_b;

  bool gouraud = (queue.mixmode & CS_FX_GOURAUD) != 0;
  float alpha = 1.0f - BYTE_TO_FLOAT (queue.mixmode & CS_FX_MASK_ALPHA);
  bool txt_alpha = false;
  if (poly.mat_handle)
  {
    iTextureHandle* txt_handle = poly.mat_handle->GetTexture ();
    if (txt_handle)
      txt_alpha = txt_handle->GetKeyColor () || txt_handle->GetAlphaMap ();
  }
  alpha = GetAlpha (queue.mixmode, alpha, txt_alpha);

  //========
  // Update polygon info in queue.
  //========
  int idx = queue.AddVertices (poly.num);
  GLfloat* glverts = queue.GetGLVerts (idx);
  GLfloat* gltxt = queue.GetGLTxt (idx);
  GLfloat* glcol = queue.GetGLCol (idx);
  int i;
  for (i = 0 ; i < poly.num ; i++)
  {
    if (gouraud)
    {
      *glcol++ = flat_r * poly.vertices[i].r;
      *glcol++ = flat_g * poly.vertices[i].g;
      *glcol++ = flat_b * poly.vertices[i].b;
      *glcol++ = alpha;
    }
    float sz = poly.vertices[i].z;
    if (ABS (sz) < SMALL_EPSILON) sz = 1. / SMALL_EPSILON;
    else sz = 1./sz;

    *glverts++ = poly.vertices[i].sx * sz;
    *glverts++ = poly.vertices[i].sy * sz;
    *glverts++ = -1;
    *glverts++ = sz;

    *gltxt++ = poly.vertices[i].u;
    *gltxt++ = poly.vertices[i].v;
  }

  //========
  // Triangulate.
  //========
  for (i = 2 ; i < poly.num ; i++)
  {
    queue.AddTriangle (idx+0, idx+i-1, idx+i);
  }

  //========
  // Add fog info to the fog queue.
  //========
  if (poly.use_fog)
  {
    fog_queue.z_buf_mode = z_buf_mode;
    int fog_idx = fog_queue.AddVertices (poly.num);

    // Copy vertex info.
    GLfloat* glverts = queue.GetGLVerts (idx);
    GLfloat* fog_glverts = fog_queue.GetGLVerts (fog_idx);
    memcpy (fog_glverts, glverts, poly.num*sizeof (GLfloat)*4);
    GLfloat* fog_color = fog_queue.GetFogColor (idx);
    GLfloat* fog_txt = fog_queue.GetFogTxt (idx);
    for (i = 0 ; i < poly.num ; i++)
    {
      *fog_color++ = poly.fog_info[i].r;
      *fog_color++ = poly.fog_info[i].g;
      *fog_color++ = poly.fog_info[i].b;

      *fog_txt++ = poly.fog_info[i].intensity;
      *fog_txt++ = 0.0;
    }

    // Triangulate.
    for (i = 2 ; i < poly.num ; i++)
    {
      fog_queue.AddTriangle (fog_idx+0, fog_idx+i-1, fog_idx+i);
    }
  }
}

// Find out the location of a vertex by recursing through the
// clipinfo tree.
static void ResolveVertex (
	csClipInfo* ci,
	int* clipped_translate,
	csVector3* overts, csVector2* otexels,
	csColor* ocolors, G3DFogInfo* ofog,
	csVector2& texel, csColor& color, G3DFogInfo& fog)
{
  switch (ci->type)
  {
    case CS_CLIPINFO_ORIGINAL:
      texel = otexels[ci->original.idx];
      if (ocolors) color = ocolors[ci->original.idx];
      if (ofog) fog = ofog[ci->original.idx];
      break;
    case CS_CLIPINFO_ONEDGE:
    {
      int i1 = ci->onedge.i1;
      int i2 = ci->onedge.i2;
      float r = ci->onedge.r;
      texel = otexels[i1] * (1-r) + otexels[i2] * r;
      if (ocolors)
      {
	color.red = ocolors[i1].red * (1-r) + ocolors[i2].red * r;
	color.green = ocolors[i1].green * (1-r) + ocolors[i2].green * r;
	color.blue = ocolors[i1].blue * (1-r) + ocolors[i2].blue * r;
      }
      if (ofog)
      {
	fog.intensity = ofog[i1].intensity*(1-r)+ofog[i2].intensity*r;
	fog.intensity2 = 0;
	fog.r = ofog[i1].r * (1-r) + ofog[i2].r * r;
	fog.g = ofog[i1].g * (1-r) + ofog[i2].g * r;
	fog.b = ofog[i1].b * (1-r) + ofog[i2].b * r;
      }
      break;
    }
    case CS_CLIPINFO_INSIDE:
    {
      csVector2 texel1, texel2;
      csColor color1, color2;
      G3DFogInfo fog1, fog2;
      ResolveVertex (ci->inside.ci1, clipped_translate, overts, otexels,
		ocolors, ofog, texel1, color1, fog1);
      ResolveVertex (ci->inside.ci2, clipped_translate, overts, otexels,
		ocolors, ofog, texel2, color2, fog2);
      delete ci->inside.ci1;
      delete ci->inside.ci2;
      ci->type = CS_CLIPINFO_ORIGINAL;
      float r = ci->inside.r;
      texel = texel1 * (1-r) + texel2 * r;
      if (ocolors)
      {
	color.red = color1.red * (1-r) + color2.red * r;
	color.green = color1.green * (1-r) + color2.green * r;
	color.blue = color1.blue * (1-r) + color2.blue * r;
      }
      if (ofog)
      {
	fog.intensity =  fog1.intensity*(1-r)+fog2.intensity*r;
	fog.intensity2 = 0;
	fog.r = fog1.r * (1-r) + fog2.r * r;
	fog.g = fog1.g * (1-r) + fog2.g * r;
	fog.b = fog1.b * (1-r) + fog2.b * r;
      }
      break;
    }
  }
}

void csGraphics3DOGLCommon::ClipTriangleMesh (
    int num_triangles,
    int num_vertices,
    csTriangle* triangles,
    csVector3* vertices,
    csVector2* texels,
    csColor* vertex_colors,
    G3DFogInfo* vertex_fog,
    int& num_clipped_triangles,
    int& num_clipped_vertices,
    bool transform,
    bool mirror,
    bool exact_clipping,
    bool plane_clipping,
    bool z_plane_clipping,
    bool frustum_clipping)
{
  // Make sure the frustum is ok.
  if (frustum_clipping)
    CalculateFrustum ();

  csPlane3 frustum_planes[100];	// @@@ Arbitrary limit
  csPlane3 diagonal_planes[50];	// @@@ Arbitrary number.
  int num_frust = 0;
  int num_diagonal_planes = 0;

  int i, j, j1;
  if (frustum_clipping)
  {
    // Now calculate the frustum as seen in object space for the given
    // mesh.
    csPoly3D obj_frustum;
    int mir_i;
    num_frust = frustum.GetVertexCount ();
    for (i = 0 ; i < num_frust ; i++)
    {
      if (mirror) mir_i = num_frust-i-1;
      else mir_i = i;
      if (transform)
        obj_frustum.AddVertex (o2c.This2OtherRelative (frustum[mir_i]));
      else
        obj_frustum.AddVertex (frustum[mir_i]);
    }
    j1 = num_frust-1;
    for (j = 0 ; j < num_frust ; j++)
    {
      frustum_planes[j].Set (csVector3 (0), obj_frustum[j1], obj_frustum[j]);
      j1 = j;
    }

    // In addition to the frustum planes itself we also calculate all
    // diagonal planes which go from one side of the frustum to the other.
    // These are going to be used to detect the special case of a triangle
    // that has none of its vertices in the frustum. But this triangle can
    // still be visible. To detect this we test if there is one of these
    // extra planes that cuts the triangle in two. Since the number of diagonal
    // planes is half the number of frustum planes we can save some
    // calculation here.
    if (num_frust > 3)
      // Use (num_frust+1)/2 to make sure that odd frustums get one extra plane.
      for (j = 0 ; j < (num_frust+1) / 2 ; j++)
      {
        j1 = j + (num_frust+1) / 2;
        j1 = j1 % num_frust;
        diagonal_planes[num_diagonal_planes++].Set
      	  (csVector3 (0), obj_frustum[j], obj_frustum[j1]);
      }
  }

  // num_planes is the number of planes to test with. If there is no
  // near clipping plane then this will be equal to num_frust.
  int num_planes = num_frust;
  if (plane_clipping)
  {
  //@@@ If mirror???
    if (transform)
      frustum_planes[num_planes] = o2c.This2OtherRelative (near_plane);
    else
      frustum_planes[num_planes] = near_plane;
    num_planes++;
  }
  if (z_plane_clipping)
  {
    // @@@ In principle z-plane clipping can be done more efficiently.
    // Currently we just do it the general way. Have to think about an
    // easy way to optimize this.
    if (transform)
      frustum_planes[num_planes] = o2c.This2OtherRelative (
      	csPlane3 (0, 0, -1, .001));
    else
      frustum_planes[num_planes] = csPlane3 (0, 0, -1, .001);
    num_planes++;
  }

  csVector3 frust_origin;
  if (transform)
    frust_origin = o2c.This2Other (csVector3 (0));
  else
    frust_origin.Set (0, 0, 0);

  ClipTriangleMesh (num_triangles, num_vertices, triangles, vertices,
    texels, vertex_colors, vertex_fog,
    num_clipped_triangles, num_clipped_vertices, exact_clipping,
    frust_origin, frustum_planes, num_planes,
    diagonal_planes, num_diagonal_planes);
}

void csGraphics3DOGLCommon::ClipTriangleMesh (
    int num_triangles,
    int num_vertices,
    csTriangle* triangles,
    csVector3* vertices,
    csVector2* texels,
    csColor* vertex_colors,
    G3DFogInfo* vertex_fog,
    int& num_clipped_triangles,
    int& num_clipped_vertices,
    bool exact_clipping,
    const csVector3& frust_origin,
    csPlane3* planes, int num_planes,
    csPlane3* diag_planes, int num_diag_planes)
{
  int i, j;

  // Make sure our worktables are big enough for the clipped mesh.
  int num_tri = num_triangles*2+50;
  if (num_tri > clipped_triangles.Limit ())
  {
    // Use two times as many triangles. Hopefully this is enough.
    clipped_triangles.SetLimit (num_tri);
  }
  if (num_vertices > clipped_translate.Limit ())
    clipped_translate.SetLimit (num_vertices);	// Used for original vertices.
  int num_vts = num_vertices*2+100;
  if (num_vts > clipped_vertices.Limit ())
  {
    clipped_vertices.SetLimit (num_vts);
    clipped_texels.SetLimit (num_vts);
    clipped_colors.SetLimit (num_vts);
    clipped_fog.SetLimit (num_vts);
  }

  num_clipped_triangles = 0;
  num_clipped_vertices = 0;

  // Check all original vertices and see if they are in frustum.
  // If yes we set clipped_translate to the new position in the transformed
  // vertex array. Otherwise we set clipped_translate to -1.
  for (i = 0 ; i < num_vertices ; i++)
  {
    const csVector3& v = vertices[i];
    bool inside = true;
    for (j = 0 ; j < num_planes ; j++)
    {
      if (planes[j].Classify (v-frust_origin) >= 0)
      {
	inside = false;
	break;	// Not inside.
      }
    }
    if (inside)
    {
      if (exact_clipping)
      {
        clipped_translate[i] = num_clipped_vertices;
        clipped_vertices[num_clipped_vertices] = v;
        clipped_texels[num_clipped_vertices] = texels[i];
        if (vertex_colors)
          clipped_colors[num_clipped_vertices] = vertex_colors[i];
        if (vertex_fog)
          clipped_fog[num_clipped_vertices] = vertex_fog[i];
        num_clipped_vertices++;
      }
      else
        clipped_translate[i] = i;
    }
    else
      clipped_translate[i] = -1;
  }

  // If we have lazy clipping then the number of vertices remains the same.
  if (!exact_clipping)
    num_clipped_vertices = num_vertices;

  // Now clip all triangles.
  for (i = 0 ; i < num_triangles ; i++)
  {
    csTriangle& tri = triangles[i];
    int cnt = int (clipped_translate[tri.a] != -1)
      	+ int (clipped_translate[tri.b] != -1)
	+ int (clipped_translate[tri.c] != -1);
    if (cnt == 0)
    {
      //=====
      // Here we have a special case where we need to test if the
      // triangle is cut by the diagonal planes. If yes then we have
      // to clip anyway.
      //=====
      // @@@ WARNING: This test is not 100% correct and it is possible
      // to reproduce this problem fairly easily. Especially if the
      // clipper is a triangle in which case this test will not even
      // function.
      for (j = 0 ; j < num_diag_planes ; j++)
      {
        csPlane3& pl = diag_planes[j];
        csVector3 v0 = vertices[tri.a] - frust_origin;
        csVector3 v1 = vertices[tri.b] - frust_origin;
        csVector3 v2 = vertices[tri.c] - frust_origin;
	float c0 = pl.Classify (v0);
	float c1 = pl.Classify (v1);
	// Set cnt to 1 so that we will clip in the next part.
	if ((c0 < 0 && c1 > 0) || (c0 > 0 && c1 < 0)) { cnt = 1; break; }
	float c2 = pl.Classify (v2);
	if ((c0 < 0 && c2 > 0) || (c0 > 0 && c2 < 0)) { cnt = 1; break; }
	if ((c1 < 0 && c2 > 0) || (c1 > 0 && c2 < 0)) { cnt = 1; break; }
      }
    }

    if (cnt == 0)
    {
      //=====
      // Easiest case: triangle is not visible.
      //=====
    }
    else if (cnt == 3)
    {
      //=====
      // Easy case: the triangle is fully in view.
      //=====
      clipped_triangles[num_clipped_triangles].a = clipped_translate[tri.a];
      clipped_triangles[num_clipped_triangles].b = clipped_translate[tri.b];
      clipped_triangles[num_clipped_triangles].c = clipped_translate[tri.c];
      num_clipped_triangles++;
    }
    else
    {
      //=====
      // Difficult case: clipping will result in several triangles.
      //=====
      if (!exact_clipping)
      {
        // If we have lazy clipping then we just add the triangle.
        clipped_triangles[num_clipped_triangles].a = tri.a;
        clipped_triangles[num_clipped_triangles].b = tri.b;
        clipped_triangles[num_clipped_triangles].c = tri.c;
        num_clipped_triangles++;
	continue;
      }

      csVector3 poly[100];	// @@@ Arbitrary limit
      static csClipInfo clipinfo[100];
      poly[0] = vertices[tri.a] - frust_origin;
      poly[1] = vertices[tri.b] - frust_origin;
      poly[2] = vertices[tri.c] - frust_origin;
      clipinfo[0].Clear ();
      clipinfo[1].Clear ();
      clipinfo[2].Clear ();
      clipinfo[0].type = CS_CLIPINFO_ORIGINAL; clipinfo[0].original.idx = tri.a;
      clipinfo[1].type = CS_CLIPINFO_ORIGINAL; clipinfo[1].original.idx = tri.b;
      clipinfo[2].type = CS_CLIPINFO_ORIGINAL; clipinfo[2].original.idx = tri.c;
      int num_poly = 3;

      //-----
      // First we clip the triangle to the given planes.
      // This will result in a polygon. The clipper keeps information
      // (in clipinfo) about what happens to all the vertices.
      //-----
      for (j = 0 ; j < num_planes ; j++)
      {
	csFrustum::ClipToPlane (poly, num_poly, clipinfo, planes[j]);
	if (num_poly <= 0) break;
      }

      //-----
      // First add all new vertices and resolve coordinates of texture
      // mapping and so on using the clipinfo.
      //-----
      for (j = 0 ; j < num_poly ; j++)
      {
	if (clipinfo[j].type == CS_CLIPINFO_ORIGINAL)
	{
	  clipinfo[j].original.idx =
	  	clipped_translate[clipinfo[j].original.idx];
	}
        else
	{
	  ResolveVertex (&clipinfo[j], clipped_translate.GetArray (),
	  	vertices, texels, vertex_colors, vertex_fog,
		clipped_texels.GetArray ()[num_clipped_vertices],
		clipped_colors.GetArray ()[num_clipped_vertices],
		clipped_fog.GetArray ()[num_clipped_vertices]);
	  clipped_vertices[num_clipped_vertices] = poly[j]+frust_origin;
	  clipinfo[j].original.idx = num_clipped_vertices;
	  num_clipped_vertices++;
	}
      }

      //-----
      // Triangulate the resulting polygon.
      //-----
      for (j = 2 ; j < num_poly ; j++)
      {
        clipped_triangles[num_clipped_triangles].a = clipinfo[0].original.idx;
        clipped_triangles[num_clipped_triangles].b = clipinfo[j-1].original.idx;
        clipped_triangles[num_clipped_triangles].c = clipinfo[j].original.idx;
        num_clipped_triangles++;
      }
    }
  }
}

void csGraphics3DOGLCommon::DrawTriangleMesh (G3DTriangleMesh& mesh)
{
  FlushDrawPolygon ();
  lightmap_cache->FlushIfNeeded ();
  if (!CompatibleZBufModes (fog_queue.z_buf_mode, z_buf_mode))
    FlushDrawFog ();

  bool stencil_enabled = false;
  bool clip_planes_enabled = false;

  //===========
  // First we are going to find out what kind of clipping (if any)
  // we need. This depends on various factors including what the engine
  // says about the mesh (the clip_portal and clip_plane flags in the
  // mesh), what the current clipper is (the current cliptype), what
  // the current z-buf render mode is, and what the settings are to use
  // for the clipper on the current type of hardware (the clip_... arrays).
  //===========
  char how_clip = OPENGL_CLIP_NONE;
  bool use_lazy_clipping = false;
  bool do_plane_clipping = false;
  bool do_z_plane_clipping = false;

  // First we see how many additional planes we might need because of
  // z-plane clipping and/or near-plane clipping. These additional planes
  // will not be usable for portal clipping (if we're using OpenGL plane
  // clipping).
  int reserved_planes =
  	int (do_near_plane && mesh.clip_plane != CS_CLIP_NOT) +
	int (mesh.clip_z_plane != CS_CLIP_NOT);

  if (mesh.clip_portal != CS_CLIP_NOT)
  {
    // Some clipping may be required.

    // In some z-buf modes we cannot use clipping modes that depend on
    // zbuffer ('n','N', 'z', or 'Z').
    bool no_zbuf_clipping = (z_buf_mode == CS_ZBUF_NONE
    	|| z_buf_mode == CS_ZBUF_FILL || z_buf_mode == CS_ZBUF_FILLONLY);

    // Select the right clipping mode variable depending on the
    // type of clipper.
    int ct = cliptype;
    // If clip_portal in the mesh indicates that we might need toplevel
    // clipping then we do as if the current clipper type is toplevel.
    if (mesh.clip_portal == CS_CLIP_TOPLEVEL) ct = CS_CLIPPER_TOPLEVEL;
    char* clip_modes;
    switch (ct)
    {
      case CS_CLIPPER_OPTIONAL: clip_modes = clip_optional; break;
      case CS_CLIPPER_REQUIRED: clip_modes = clip_required; break;
      case CS_CLIPPER_TOPLEVEL: clip_modes = clip_outer; break;
      default: clip_modes = clip_optional;
    }

    // Go through all the modes and select the first one that is appropriate.
    int i;
    for (i = 0 ; i < 3 ; i++)
    {
      char c = clip_modes[i];
      // We cannot use n,N,z, or Z if no_zbuf_clipping is true.
      if ((c == 'n' || c == 'N' || c == 'z' || c == 'Z') && no_zbuf_clipping)
        continue;
      // We cannot use p or P if the clipper has more vertices than the
      // number of hardware planes minus one (for the view plane).
      if ((c == 'p' || c == 'P') &&
      		clipper->GetVertexCount ()
		>= GLCaps.nr_hardware_planes-reserved_planes)
        continue;
      how_clip = c;
      break;
    }
    if (how_clip != '0' && toupper (how_clip) == how_clip)
    {
      use_lazy_clipping = true;
      how_clip = tolower (how_clip);
    }
  }

  // Check for the near-plane.
  if (do_near_plane && mesh.clip_plane != CS_CLIP_NOT)
  {
    do_plane_clipping = true;
    // If we must do clipping to the near plane then we cannot use
    // lazy clipping.
    use_lazy_clipping = false;
    // If we are doing plane clipping already then we don't have
    // to do additional software plane clipping as the OpenGL plane
    // clipper can do this too.
    if (how_clip == 'p')
    {
      do_plane_clipping = false;
    }
  }

  // Check for the z-plane.
  if (mesh.clip_z_plane != CS_CLIP_NOT)
  {
    do_z_plane_clipping = true;
    // If hardware requires clipping to the z-plane (because it
    // crashes otherwise) we have to disable lazy clipping.
    // @@@
    if (true)
    {
      use_lazy_clipping = false;
    }
    else
    {
      // If we are doing plane clipping already then we don't have
      // to do additional software plane clipping as the OpenGL plane
      // clipper can do this too.
      if (how_clip == 'p')
      {
        do_z_plane_clipping = false;
      }
    }
  }

  int i, k;

  //===========
  // Update work tables.
  //===========
  int num_vertices = mesh.num_vertices;
  int num_triangles = mesh.num_triangles;
  if (num_vertices > tr_verts.Limit ())
  {
    tr_verts.SetLimit (num_vertices);
    uv_verts.SetLimit (num_vertices);
    color_verts.SetLimit (num_vertices);
  }

  //===========
  // Do vertex tweening and/or transformation to camera space
  // if any of those are needed. When this is done 'verts' will
  // point to an array of camera vertices.
  //===========
  csVector3* f1 = mesh.vertices[0];
  csVector2* uv1 = mesh.texels[0];
  csColor* col1 = mesh.vertex_colors[0];
  if (!col1) mesh.do_morph_colors = false;
  csVector3* work_verts;
  csVector2* work_uv_verts;
  csColor* work_colors;

  if (mesh.num_vertices_pool > 1)
  {
    // Vertex morphing.
    float tr = mesh.morph_factor;
    float remainder = 1 - tr;
    csVector3* f2 = mesh.vertices[1];
    csVector2* uv2 = mesh.texels[1];
    csColor* col2 = mesh.vertex_colors[1];
    for (i = 0 ; i < num_vertices ; i++)
    {
      tr_verts[i] = tr * f2[i] + remainder * f1[i];
      if (mesh.do_morph_texels)
        uv_verts[i] = tr * uv2[i] + remainder * uv1[i];
      if (mesh.do_morph_colors)
      {
        color_verts[i].red = tr * col2[i].red + remainder * col1[i].red;
	color_verts[i].green = tr * col2[i].green + remainder * col1[i].green;
	color_verts[i].blue = tr * col2[i].blue + remainder * col1[i].blue;
      }
    }
    work_verts = tr_verts.GetArray ();
    if (mesh.do_morph_texels)
      work_uv_verts = uv_verts.GetArray ();
    else
      work_uv_verts = uv1;
    if (mesh.do_morph_colors)
      work_colors = color_verts.GetArray ();
    else
      work_colors = col1;
  }
  else
  {
    work_verts = f1;
    work_uv_verts = uv1;
    work_colors = col1;
  }
  csTriangle *triangles = mesh.triangles;
  G3DFogInfo* work_fog = mesh.vertex_fog;

  //===========
  // Here we perform lazy or software clipping if needed.
  //===========
  if (how_clip == '0' || use_lazy_clipping
  	|| do_plane_clipping || do_z_plane_clipping)
  {
    ClipTriangleMesh (
	num_triangles,
	num_vertices,
	triangles,
	work_verts,
	work_uv_verts,
	work_colors,
	work_fog,
	num_triangles,
	num_vertices,
	mesh.vertex_mode == G3DTriangleMesh::VM_WORLDSPACE,
	mesh.do_mirror,
	!use_lazy_clipping,
	do_plane_clipping,
	do_z_plane_clipping,
	how_clip == '0' || use_lazy_clipping);
    if (!use_lazy_clipping)
    {
      work_verts = clipped_vertices.GetArray ();
      work_uv_verts = clipped_texels.GetArray ();
      work_colors = clipped_colors.GetArray ();
      work_fog = clipped_fog.GetArray ();
    }
    triangles = clipped_triangles.GetArray ();
    if (num_triangles <= 0) return;	// Nothing to do!
  }

  //===========
  // First setup the clipper that we need.
  //===========
  if (how_clip == 's')
  {
    SetupStencil ();
    stencil_enabled = true;
    // Use the stencil area.
    glEnable (GL_STENCIL_TEST);
    glStencilFunc (GL_EQUAL, 1, 1);
    glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
  }
  else if (how_clip == 'p')
  {
    SetupClipPlanes (do_near_plane && mesh.clip_plane != CS_CLIP_NOT,
    	mesh.clip_z_plane != CS_CLIP_NOT);
    clip_planes_enabled = true;
    for (i = 0 ; i < frustum.GetVertexCount ()+reserved_planes ; i++)
      glEnable ((GLenum)(GL_CLIP_PLANE0+i));
  }

  // set up coordinate transform
  GLfloat matrixholder[16];

  glMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  glLoadIdentity ();

  //===========
  // set up world->camera transform, if needed
  //===========
  if (mesh.vertex_mode == G3DTriangleMesh::VM_WORLDSPACE)
  {
    // we basically have to duplicate the
    // original transformation code:
    //   tr_verts[i] = o2c.GetO2T() *  (f1[i] - o2c.GetO2TTranslation() );
    //
    // we do this by applying both a translation and rotation matrix
    // using values pulled from the o2c quantity, which represents
    // the current world->camera transform
    //
    // Wonder why we do the orientation before the translation?
    // Many 3D graphics and OpenGL books discuss how the order
    // of 4x4 transform matrices represent certain transformations,
    // and they do a much better job than I ever could.  Please refer
    // to an OpenGL reference for good insight into proper manipulation
    // of the modelview matrix.

    const csMatrix3 &orientation = o2c.GetO2T();

    matrixholder[0] = orientation.m11;
    matrixholder[1] = orientation.m21;
    matrixholder[2] = orientation.m31;

    matrixholder[4] = orientation.m12;
    matrixholder[5] = orientation.m22;
    matrixholder[6] = orientation.m32;

    matrixholder[8] = orientation.m13;
    matrixholder[9] = orientation.m23;
    matrixholder[10] = orientation.m33;

    matrixholder[3] = matrixholder[7] = matrixholder[11] =
    matrixholder[12] = matrixholder[13] = matrixholder[14] = 0.0;
    matrixholder[15] = 1.0;

    const csVector3 &translation = o2c.GetO2TTranslation();

    glMultMatrixf (matrixholder);
    glTranslatef (-translation.x, -translation.y, -translation.z);
  }

  //===========
  // Set up perspective transform.
  // we have to change the standard projection matrix used for
  // drawing in other parts of CS.  Normally an orthogonal projection
  // is used since CS does the perspective projection for us.
  // Here, we need to reproduce CS's perspective projection using
  // OpenGL matrices.
  //===========

  // @@@ CACHE matrix mode too!!!???
  // Probably very worthwhile!
  glMatrixMode (GL_PROJECTION);
  glPushMatrix ();
  glLoadIdentity ();

  // With the back buffer procedural textures the orthographic projection
  // matrix is inverted.
  if (inverted)
    glOrtho (0., (GLdouble) width, (GLdouble) height, 0., -1.0, 10.0);
  else
    glOrtho (0., (GLdouble) width, 0., (GLdouble) height, -1.0, 10.0);


  glTranslatef (asp_center_x, asp_center_y, 0);
  for (i = 0 ; i < 16 ; i++) matrixholder[i] = 0.0;
  matrixholder[0] = matrixholder[5] = 1.0;
  matrixholder[11] = inv_aspect;
  matrixholder[14] = -inv_aspect;
  glMultMatrixf (matrixholder);

  //===========
  // Setup states
  //===========
  UInt m_mixmode = mesh.fxmode;
  float m_alpha = 1.0f - BYTE_TO_FLOAT (m_mixmode & CS_FX_MASK_ALPHA);
  bool m_gouraud = m_renderstate.lighting && m_renderstate.gouraud &&
  	((m_mixmode & CS_FX_GOURAUD) != 0);

  GLuint texturehandle = 0;
  bool txt_alpha = false;
  csMaterialHandle* m_multimat = NULL;
  if (mesh.mat_handle && m_renderstate.textured)
  {
    CacheTexture (mesh.mat_handle);
    iTextureHandle* txt_handle = mesh.mat_handle->GetTexture ();
    if (txt_handle)
    {
      csTextureHandleOpenGL *txt_mm = (csTextureHandleOpenGL *)
      txt_handle->GetPrivateObject ();
      csTxtCacheData *cachedata = (csTxtCacheData *)txt_mm->GetCacheData ();
      texturehandle = cachedata->Handle;

      txt_alpha = txt_handle->GetKeyColor () || txt_handle->GetAlphaMap ();
    }
    if (((csMaterialHandle*)mesh.mat_handle)->GetTextureLayerCount () > 0)
      m_multimat = (csMaterialHandle*)mesh.mat_handle;
  }
  m_alpha = SetupBlend (m_mixmode, m_alpha, txt_alpha);
  SetMirrorMode (mesh.do_mirror);

  bool m_textured = (texturehandle != 0);
  if (m_textured)
  {
    glBindTexture (GL_TEXTURE_2D, texturehandle);
    glEnable (GL_TEXTURE_2D);
  }
  else
    glDisable (GL_TEXTURE_2D);

  SetGLZBufferFlags (z_buf_mode);

  csMaterialHandle* mat = NULL;
  if (mesh.mat_handle)
  {
    mat = (csMaterialHandle*)mesh.mat_handle;
  }
  bool do_gouraud = m_gouraud && work_colors;

  float flat_r = 1., flat_g = 1., flat_b = 1.;
  if (do_gouraud)
  {
    // special hack for transparent meshes
    if (mesh.fxmode & CS_FX_ALPHA)
    {
      if ((num_vertices*4) > rgba_verts.Limit ())
        rgba_verts.SetLimit (num_vertices*4);
      for (k=0, i=0; i<num_vertices; i++)
      {
        rgba_verts[k++] = work_colors[i].red;
        rgba_verts[k++] = work_colors[i].green;
        rgba_verts[k++] = work_colors[i].blue;
	rgba_verts[k++] = m_alpha;
      }
    }
  }
  else
  {
    if (!m_textured)
    {
      // Fill flat color if renderer decide to paint it flat-shaded
      UByte r,g,b;
      if (mesh.mat_handle)
        mesh.mat_handle->GetTexture ()->GetMeanColor (r, g, b);
      else
        r = g = b = 1;
      flat_r = BYTE_TO_FLOAT (r);
      flat_g = BYTE_TO_FLOAT (g);
      flat_b = BYTE_TO_FLOAT (b);
    }
  }

  //===========
  // Draw the base mesh.
  //===========
  glVertexPointer (3, GL_FLOAT, 0, & work_verts[0]);
  glTexCoordPointer (2, GL_FLOAT, 0, & work_uv_verts[0]);
  // If multi-texturing is enabled we delay apply of gouraud shading
  // until later.
  if (do_gouraud && !m_multimat)
  {
    glShadeModel (GL_SMOOTH);
    SetClientStates (CS_CLIENTSTATE_ALL);
    if (mesh.fxmode & CS_FX_ALPHA)
      glColorPointer (4, GL_FLOAT, 0, & rgba_verts[0]);
    else
      glColorPointer (3, GL_FLOAT, 0, & work_colors[0]);
  }
  else
  {
    SetClientStates (CS_CLIENTSTATE_VT);
    glShadeModel (GL_FLAT);
    glColor4f (flat_r, flat_g, flat_b, m_alpha);
  }

  glDrawElements (GL_TRIANGLES, num_triangles*3, GL_UNSIGNED_INT, triangles);

  // If we have multi-texturing or fog we set the second pass Z-buffer
  // mode here.
  if (m_multimat || mesh.do_fog)
    SetGLZBufferFlagsPass2 (z_buf_mode, true);

  //===========
  // Here we perform multi-texturing.
  //===========
  if (m_multimat)
  {
    glShadeModel (GL_FLAT);
    SetClientStates (CS_CLIENTSTATE_VT);
    if (num_vertices > uv_mul_verts.Limit ())
      uv_mul_verts.SetLimit (num_vertices);

    int j;
    for (j = 0 ; j < mat->GetTextureLayerCount () ; j++)
    {
      csTextureLayer* layer = mat->GetTextureLayer (j);
      iTextureHandle* txt_handle = layer->txt_handle;
      csTextureHandleOpenGL *txt_mm = (csTextureHandleOpenGL *)
      	txt_handle->GetPrivateObject ();
      csTxtCacheData *texturecache_data;
      texturecache_data = (csTxtCacheData *)txt_mm->GetCacheData ();
      bool tex_transp = txt_mm->GetKeyColor () || txt_mm->GetAlphaMap ();
      GLuint texturehandle = texturecache_data->Handle;
      glBindTexture (GL_TEXTURE_2D, texturehandle);
      float alpha = 1.0f - BYTE_TO_FLOAT (layer->mode & CS_FX_MASK_ALPHA);
      alpha = SetupBlend (layer->mode, alpha, tex_transp);
      glColor4f (1., 1., 1., alpha);
      csVector2* mul_uv = work_uv_verts;
      if (mat->TextureLayerTranslated (j))
      {
        float uscale = layer->uscale;
        float vscale = layer->vscale;
        float ushift = layer->ushift;
        float vshift = layer->vshift;
// @@@ Experimental define to see if using a TEXTURE matrix
// instead of scaling manually is more efficient. Have to try
// this out on various cards to see the effect.
// Conclusion: it seems to be slower for some reason (but not much).
#define EXP_SCALE_MATRIX 0
#if EXP_SCALE_MATRIX
	glMatrixMode (GL_TEXTURE);
	glPushMatrix ();
	glLoadIdentity ();
	GLfloat scalematrix[16];
	for (i = 0 ; i < 16 ; i++) scalematrix[i] = 0.0;
	scalematrix[0] = uscale;
	scalematrix[5] = vscale;
	scalematrix[10] = 1;
	scalematrix[15] = 1;
	// @@@ Shift is ignored for now.
	glMultMatrixf (scalematrix);
#else
        mul_uv = uv_mul_verts.GetArray ();
	for (i = 0 ; i < num_vertices ; i++)
	{
	  mul_uv[i].x = work_uv_verts[i].x * uscale + ushift;
	  mul_uv[i].y = work_uv_verts[i].y * vscale + vshift;
	}
#endif
      }

      glVertexPointer (3, GL_FLOAT, 0, & work_verts[0]);
      glTexCoordPointer (2, GL_FLOAT, 0, mul_uv);
      glDrawElements (GL_TRIANGLES, num_triangles*3,
      	GL_UNSIGNED_INT, triangles);
#if EXP_SCALE_MATRIX
      if (mat->TextureLayerTranslated (j))
      {
	glPopMatrix ();
	glMatrixMode (GL_MODELVIEW);
      }
#endif
    }

    // If we have to do gouraud shading we do it here.
    if (do_gouraud)
    {
      glDisable (GL_TEXTURE_2D);
      glShadeModel (GL_SMOOTH);
      SetupBlend (CS_FX_MULTIPLY2, 0, false);
      SetClientStates (CS_CLIENTSTATE_ALL);
      if (mesh.fxmode & CS_FX_ALPHA)
        glColorPointer (4, GL_FLOAT, 0, & rgba_verts[0]);
      else
        glColorPointer (3, GL_FLOAT, 0, & work_colors[0]);
      glVertexPointer (3, GL_FLOAT, 0, & work_verts[0]);
      glDrawElements (GL_TRIANGLES, num_triangles*3,
      	GL_UNSIGNED_INT, triangles);
    }
  }

  //===========
  // If there is vertex fog then we apply that last.
  //===========
  if (mesh.do_fog)
  {
    // we need to texture and blend, with vertex color
    // interpolation
    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, m_fogtexturehandle);
    SetupBlend (CS_FX_ALPHA, 0, false);

    glShadeModel (GL_SMOOTH);
    SetClientStates (CS_CLIENTSTATE_ALL);
    glVertexPointer (3, GL_FLOAT, 0, & work_verts[0]);
    glTexCoordPointer (2, GL_FLOAT, sizeof(G3DFogInfo), &work_fog[0].intensity);
    glColorPointer (3, GL_FLOAT, sizeof (G3DFogInfo), &work_fog[0].r);
    glDrawElements (GL_TRIANGLES, num_triangles*3, GL_UNSIGNED_INT, triangles);

    if (!m_textured)
      glDisable (GL_TEXTURE_2D);
    if (!m_gouraud)
      glShadeModel (GL_FLAT);
  }

  glMatrixMode (GL_MODELVIEW);
  glPopMatrix ();
  glMatrixMode (GL_PROJECTION);
  glPopMatrix ();

  if (debug_edges)
    DebugDrawElements (G2D,
	num_triangles*3, (int*)triangles, (GLfloat*)& work_verts[0],
		txtmgr->FindRGB (255, 0, 0), true,
		mesh.vertex_mode == G3DTriangleMesh::VM_VIEWSPACE);

  //===========
  // Disable/cleanup all clipping stuff.
  //===========
  if (stencil_enabled)
    glDisable (GL_STENCIL_TEST);
  if (clip_planes_enabled)
    for (i = 0 ; i < frustum.GetVertexCount ()+reserved_planes ; i++)
      glDisable ((GLenum)(GL_CLIP_PLANE0+i));

  SetMirrorMode (false);
}



void csGraphics3DOGLCommon::OpenFogObject (CS_ID, csFog*)
{
  // OpenGL driver implements vertex-based fog ...
}

void csGraphics3DOGLCommon::DrawFogPolygon (CS_ID, G3DPolygonDFP&, int)
{
  // OpenGL driver implements vertex-based fog ...
}

void csGraphics3DOGLCommon::CloseFogObject (CS_ID)
{
  // OpenGL driver implements vertex-based fog ...
}

void csGraphics3DOGLCommon::CacheTexture (iMaterialHandle *imat_handle)
{
  csMaterialHandle* mat_handle = (csMaterialHandle*)imat_handle;
  iTextureHandle* txt_handle = mat_handle->GetTexture ();
  if (txt_handle)
    texture_cache->Cache (txt_handle);
  // Also cache all textures used in the texture layers.
  int i;
  for (i = 0 ; i < mat_handle->GetTextureLayerCount () ; i++)
  {
    iTextureHandle* txt_layer_handle = mat_handle->GetTextureLayer (i)->
    	txt_handle;
    if (txt_layer_handle)
      texture_cache->Cache (txt_layer_handle);
  }
}

void csGraphics3DOGLCommon::RemoveFromCache (iPolygonTexture* poly_texture)
{
  lightmap_cache->Uncache (poly_texture);
}

void csGraphics3DOGLCommon::CacheTexture (iPolygonTexture *texture)
{
  CacheTexture (texture->GetMaterialHandle ());
  if (m_renderstate.lighting)
    lightmap_cache->Cache (texture);
}

bool csGraphics3DOGLCommon::SetRenderState (G3D_RENDERSTATEOPTION op,
	long value)
{
  switch (op)
  {
    case G3DRENDERSTATE_ZBUFFERMODE:
      z_buf_mode = (csZBufMode) value;
      break;
    case G3DRENDERSTATE_DITHERENABLE:
      m_renderstate.dither = value;
      break;
    case G3DRENDERSTATE_BILINEARMAPPINGENABLE:
      texture_cache->SetBilinearMapping (value);
      break;
    case G3DRENDERSTATE_TRILINEARMAPPINGENABLE:
      m_renderstate.trilinearmap = value;
      break;
    case G3DRENDERSTATE_TRANSPARENCYENABLE:
      m_renderstate.alphablend = value;
      break;
    case G3DRENDERSTATE_MIPMAPENABLE:
      m_renderstate.mipmap = value;
      break;
    case G3DRENDERSTATE_TEXTUREMAPPINGENABLE:
      m_renderstate.textured = value;
      break;
    case G3DRENDERSTATE_MMXENABLE:
      return false;
    case G3DRENDERSTATE_INTERLACINGENABLE:
      return false;
    case G3DRENDERSTATE_LIGHTINGENABLE:
      m_renderstate.lighting = value;
      break;
    case G3DRENDERSTATE_GOURAUDENABLE:
      m_renderstate.gouraud = value;
      break;
    case G3DRENDERSTATE_MAXPOLYGONSTODRAW:
      break;
    case G3DRENDERSTATE_EDGES:
      debug_edges = value;
      break;
    default:
      return false;
  }

  return true;
}

long csGraphics3DOGLCommon::GetRenderState (G3D_RENDERSTATEOPTION op)
{
  switch (op)
  {
    case G3DRENDERSTATE_ZBUFFERMODE:
      return z_buf_mode;
    case G3DRENDERSTATE_DITHERENABLE:
      return m_renderstate.dither;
    case G3DRENDERSTATE_BILINEARMAPPINGENABLE:
      return texture_cache->GetBilinearMapping ();
    case G3DRENDERSTATE_TRILINEARMAPPINGENABLE:
      return m_renderstate.trilinearmap;
    case G3DRENDERSTATE_TRANSPARENCYENABLE:
      return m_renderstate.alphablend;
    case G3DRENDERSTATE_MIPMAPENABLE:
      return m_renderstate.mipmap;
    case G3DRENDERSTATE_TEXTUREMAPPINGENABLE:
      return m_renderstate.textured;
    case G3DRENDERSTATE_MMXENABLE:
      return 0;
    case G3DRENDERSTATE_INTERLACINGENABLE:
      return false;
    case G3DRENDERSTATE_LIGHTINGENABLE:
      return m_renderstate.lighting;
    case G3DRENDERSTATE_GOURAUDENABLE:
      return m_renderstate.gouraud;
    case G3DRENDERSTATE_MAXPOLYGONSTODRAW:
      return 0;
    case G3DRENDERSTATE_EDGES:
      return debug_edges;
    default:
      return 0;
  }
}

void csGraphics3DOGLCommon::ClearCache ()
{
  // We will clear lightmap cache since when unloading a world lightmaps
  // become invalid. We won't clear texture cache since texture items are
  // cleaned up individually when an iTextureHandle's RefCount reaches zero.
  if (!lightmap_cache) return;	// System is being destructed.
  FlushDrawPolygon ();
  lightmap_cache->Flush ();
  FlushDrawFog ();
  lightmap_cache->Clear ();
}

void csGraphics3DOGLCommon::DumpCache ()
{
}

void csGraphics3DOGLCommon::DrawLine (const csVector3 & v1,
	const csVector3 & v2, float fov, int color)
{
  FlushDrawPolygon ();
  lightmap_cache->Flush ();
  FlushDrawFog ();

  if (v1.z < SMALL_Z && v2.z < SMALL_Z)
    return;

  float x1 = v1.x, y1 = v1.y, z1 = v1.z;
  float x2 = v2.x, y2 = v2.y, z2 = v2.z;

  if (z1 < SMALL_Z)
  {
    // x = t*(x2-x1)+x1;
    // y = t*(y2-y1)+y1;
    // z = t*(z2-z1)+z1;
    float t = (SMALL_Z - z1) / (z2 - z1);
    x1 = t * (x2 - x1) + x1;
    y1 = t * (y2 - y1) + y1;
    z1 = SMALL_Z;
  }
  else if (z2 < SMALL_Z)
  {
    // x = t*(x2-x1)+x1;
    // y = t*(y2-y1)+y1;
    // z = t*(z2-z1)+z1;
    float t = (SMALL_Z - z1) / (z2 - z1);
    x2 = t * (x2 - x1) + x1;
    y2 = t * (y2 - y1) + y1;
    z2 = SMALL_Z;
  }
  float iz1 = fov / z1;
  int px1 = QInt (x1 * iz1 + (width / 2));
  int py1 = height - 1 - QInt (y1 * iz1 + (height / 2));
  float iz2 = fov / z2;
  int px2 = QInt (x2 * iz2 + (width / 2));
  int py2 = height - 1 - QInt (y2 * iz2 + (height / 2));

  G2D->DrawLine (px1, py1, px2, py2, color);
}

bool csGraphics3DOGLCommon::CompatibleZBufModes (csZBufMode m1, csZBufMode m2)
{
  if (m1 == m2) return true;
  if (m1 == CS_ZBUF_FILL || m1 == CS_ZBUF_FILLONLY || m1 == CS_ZBUF_USE)
    return m2 == CS_ZBUF_FILL || m2 == CS_ZBUF_FILLONLY || m2 == CS_ZBUF_USE;
  return false;
}

void csGraphics3DOGLCommon::SetGLZBufferFlags (csZBufMode flags)
{
  static csZBufMode old_flags = CS_ZBUF_NONE;
  if (old_flags == flags) return;
  switch (flags)
  {
    case CS_ZBUF_NONE:
      glDisable (GL_DEPTH_TEST);
      //glDepthMask (GL_FALSE);@@@ Is this needed or not?
      break;
    case CS_ZBUF_FILL:
    case CS_ZBUF_FILLONLY:
      if (old_flags == CS_ZBUF_NONE)
	glEnable (GL_DEPTH_TEST);
      glDepthFunc (GL_ALWAYS);
      glDepthMask (GL_TRUE);
      break;
    case CS_ZBUF_EQUAL:
      if (old_flags == CS_ZBUF_NONE)
	glEnable (GL_DEPTH_TEST);
      glDepthFunc (GL_EQUAL);
      glDepthMask (GL_FALSE);
      break;
    case CS_ZBUF_TEST:
      if (old_flags == CS_ZBUF_NONE)
	glEnable (GL_DEPTH_TEST);
      glDepthFunc (GL_GREATER);
      glDepthMask (GL_FALSE);
      break;
    case CS_ZBUF_USE:
      if (old_flags == CS_ZBUF_NONE)
	glEnable (GL_DEPTH_TEST);
      glDepthFunc (GL_GREATER);
      glDepthMask (GL_TRUE);
      break;
    default:
      break;
  }
  old_flags = flags;
}

void csGraphics3DOGLCommon::SetGLZBufferFlagsPass2 (csZBufMode flags,
	bool multiPol)
{
  switch (flags)
  {
    case CS_ZBUF_NONE:
    case CS_ZBUF_TEST:
    case CS_ZBUF_EQUAL:
      SetGLZBufferFlags (flags);
      break;
    case CS_ZBUF_FILL:
    case CS_ZBUF_FILLONLY:
      if (multiPol)
	SetGLZBufferFlags (CS_ZBUF_EQUAL);
      else
	SetGLZBufferFlags (CS_ZBUF_NONE);
      break;
    case CS_ZBUF_USE:
      SetGLZBufferFlags (CS_ZBUF_EQUAL);
      break;
    default:
      break;
  }
}

// Shortcut to override standard polygon drawing when we have
// multitexture
bool csGraphics3DOGLCommon::DrawPolygonMultiTexture (G3DPolygonDP & poly)
{
// work in progress - GJH
#if USE_MULTITEXTURE

  // count 'real' number of vertices
  int num_vertices = 1;
  int i;
  for (i = 1; i < poly.num; i++)
  {
    // theoretically we should do here sqrt(dx^2+dy^2), but
    // we can approximate it just by abs(dx)+abs(dy)
    if ((ABS (poly.vertices[i].sx - poly.vertices[i - 1].sx)
	 + ABS (poly.vertices[i].sy - poly.vertices[i - 1].sy))
	 	> VERTEX_NEAR_THRESHOLD)
      num_vertices++;
  }

  // if this is a 'degenerate' polygon, skip it
  if (num_vertices < 3)
    return false;

  iPolygonTexture *tex = poly.poly_texture;

  // find lightmap information, if any
  iLightMap *thelightmap = tex->GetLightMap ();

  // the shortcut works only if there is a lightmap and no fog and
  // no multitexturing
  csMaterialHandle* mat_handle = (csMaterialHandle*)poly.mat_handle;
  if (!thelightmap || poly.use_fog || mat_handle->GetTextureLayerCount () > 0)
  {
    DrawPolygonSingleTexture (poly);
    return true;
  }
  // OK, we're gonna draw a polygon with a dual texture
  // Get the plane normal of the polygon. Using this we can calculate
  // '1/z' at every screen space point.
  float Ac, Bc, Cc, Dc, inv_Dc;
  Ac = poly.normal.A ();
  Bc = poly.normal.B ();
  Cc = poly.normal.C ();
  Dc = poly.normal.D ();

  float M, N, O;
  if (ABS (Dc) < SMALL_D)
  {
    // The Dc component of the plane normal is too small. This means
    // that the plane of the polygon is almost perpendicular to the
    // eye of the viewer. In this case, nothing much can be seen of
    // the plane anyway so we just take one value for the entire
    // polygon.
    M = 0;
    N = 0;
    // For O choose the transformed z value of one vertex.
    // That way Z buffering should at least work.
    O = 1 / poly.z_value;
  }
  else
  {
    inv_Dc = 1 / Dc;
    M = -Ac * inv_Dc * inv_aspect;
    N = -Bc * inv_Dc * inv_aspect;
    O = -Cc * inv_Dc;
  }

  tex = poly.poly_texture;
  iTextureHandle* txt_handle = poly.mat_handle->GetTexture ();
  csTextureHandleOpenGL *txt_mm = (csTextureHandleOpenGL *) txt_handle->GetPrivateObject ();

  // find lightmap information, if any
  thelightmap = tex->GetLightMap ();

  // Initialize our static drawing information and cache
  // the texture in the texture cache (if this is not already the case).
  CacheTexture (tex);

  // @@@ The texture transform matrix is currently written as
  // T = M*(C-V)
  // (with V being the transform vector, M the transform matrix, and C
  // the position in camera space coordinates. It would be better (more
  // suitable for the following calculations) if it would be written
  // as T = M*C - V.
  float P1, P2, P3, P4, Q1, Q2, Q3, Q4;
  P1 = poly.plane.m_cam2tex->m11;
  P2 = poly.plane.m_cam2tex->m12;
  P3 = poly.plane.m_cam2tex->m13;
  P4 = -(P1 * poly.plane.v_cam2tex->x
	 + P2 * poly.plane.v_cam2tex->y
	 + P3 * poly.plane.v_cam2tex->z);
  Q1 = poly.plane.m_cam2tex->m21;
  Q2 = poly.plane.m_cam2tex->m22;
  Q3 = poly.plane.m_cam2tex->m23;
  Q4 = -(Q1 * poly.plane.v_cam2tex->x
	 + Q2 * poly.plane.v_cam2tex->y
	 + Q3 * poly.plane.v_cam2tex->z);

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
    J3 = P3 + P4 * O;
    K1 = Q1 * inv_aspect + Q4 * M;
    K2 = Q2 * inv_aspect + Q4 * N;
    K3 = Q3 + Q4 * O;
  }

  bool tex_transp;

  csTxtCacheData *texturecache_data;
  texturecache_data = (csTxtCacheData *)txt_mm->GetCacheData ();
  tex_transp = txt_mm->GetKeyColor () || txt_mm->GetAlphaMap ();
  GLuint texturehandle = texturecache_data->Handle;

  // configure base texture for texure unit 0
  float flat_r = 1.0, flat_g = 1.0, flat_b = 1.0;
  glActiveTextureARB (GL_TEXTURE0_ARB);
  glBindTexture (GL_TEXTURE_2D, texturehandle);
  float alpha = 1.0f - BYTE_TO_FLOAT (poly.mixmode & CS_FX_MASK_ALPHA);
  alpha = SetupBlend (poly.mixmode, alpha, tex_transp);
  glColor4f (flat_r, flat_g, flat_b, alpha);

  csLMCacheData *lightmapcache_data = (csLMCacheData *)thelightmap->GetCacheData ();
  GLuint lightmaphandle = lightmapcache_data->Handle;

  // configure lightmap for texture unit 1
  glActiveTextureARB (GL_TEXTURE1_ARB);
  glEnable (GL_TEXTURE_2D);
  glBindTexture (GL_TEXTURE_2D, lightmaphandle);

  SetGLZBufferFlags (z_buf_mode);

  float lm_scale_u = lightmapcache_data->lm_scale_u;
  float lm_scale_v = lightmapcache_data->lm_scale_v;
  float lm_offset_u = lightmapcache_data->lm_offset_u;
  float lm_offset_v = lightmapcache_data->lm_offset_v;

  float light_u, light_v;
  float sx, sy, sz, one_over_sz;
  float u_over_sz, v_over_sz;

  glBegin (GL_TRIANGLE_FAN);
  for (i = 0; i < poly.num; i++)
  {
    sx = poly.vertices[i].sx - asp_center_x;
    sy = poly.vertices[i].sy - asp_center_y;
    one_over_sz = M * sx + N * sy + O;
    sz = 1.0 / one_over_sz;
    u_over_sz = (J1 * sx + J2 * sy + J3);
    v_over_sz = (K1 * sx + K2 * sy + K3);
    light_u = (u_over_sz * sz - lm_offset_u) * lm_scale_u;
    light_v = (v_over_sz * sz - lm_offset_v) * lm_scale_v;

    // modified to use homogenous object space coordinates instead
    // of homogenous texture space coordinates
    glMultiTexCoord2fARB (GL_TEXTURE0_ARB, u_over_sz * sz, v_over_sz * sz);
    glMultiTexCoord2fARB (GL_TEXTURE1_ARB, light_u, light_v);
    glVertex4f (poly.vertices[i].sx * sz, poly.vertices[i].sy * sz, -1.0, sz);
  }
  glEnd ();

  // we must disable the 2nd texture unit, so that other parts of the
  // code won't accidently have a second texture applied if they
  // don't want it. At this point our active texture is still TEXTURE1_ARB
  glActiveTextureARB (GL_TEXTURE1_ARB);
  glDisable (GL_TEXTURE_2D);
  glActiveTextureARB (GL_TEXTURE0_ARB);

  return true;
#else
  (void) poly;
  // multitexture not enabled -- how did we get into this shortcut?
  return false;
#endif
}

float csGraphics3DOGLCommon::GetZBuffValue (int x, int y)
{
  GLfloat zvalue;
  glReadPixels (x, height - y - 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &zvalue);
  if (zvalue < .000001) return 1000000000.;
  // 0.090909=1/11, that is 1 divided by total depth delta set by
  // glOrtho. Where 0.090834 comes from, I don't know
  //return (0.090834 / (zvalue - (0.090909)));
  // @@@ Jorrit: I have absolutely no idea what they are trying to do
  // but changing the above formula to the one below at least appears
  // to give more accurate results.
  return (0.090728 / (zvalue - (0.090909)));
}

void csGraphics3DOGLCommon::DrawPolygon (G3DPolygonDP & poly)
{
  if (z_buf_mode == CS_ZBUF_FILLONLY)
  {
    DrawPolygonZFill (poly);
    return;
  }

#if USE_EXTENSIONS
  if (ARB_multitexture)
  {
    //@@@ This needs to be done differently. Currently disabled.
    //DrawPolygonMultiTexture (poly);
    //return;
  }
#endif
  DrawPolygonSingleTexture (poly);
}

void csGraphics3DOGLCommon::DrawPixmap (iTextureHandle *hTex,
  int sx, int sy, int sw, int sh, int tx, int ty, int tw, int th, uint8 Alpha)
{
  FlushDrawPolygon ();
  lightmap_cache->Flush ();
  FlushDrawFog ();

  int ClipX1, ClipY1, ClipX2, ClipY2;
  G2D->GetClipRect (ClipX1, ClipY1, ClipX2, ClipY2);

  // Texture coordinates (floats)
  float _tx = tx, _ty = ty, _tw = tw, _th = th;

  // Clipping
  if ((sx >= ClipX2) || (sy >= ClipY2) ||
      (sx + sw <= ClipX1) || (sy + sh <= ClipY1))
    return;                             // Sprite is totally invisible
  if (sx < ClipX1)                      // Left margin crossed?
  {
    int nw = sw - (ClipX1 - sx);        // New width
    _tx += (ClipX1 - sx) * _tw / sw;    // Adjust X coord on texture
    _tw = (_tw * nw) / sw;              // Adjust width on texture
    sw = nw; sx = ClipX1;
  } /* endif */
  if (sx + sw > ClipX2)                 // Right margin crossed?
  {
    int nw = ClipX2 - sx;               // New width
    _tw = (_tw * nw) / sw;              // Adjust width on texture
    sw = nw;
  } /* endif */
  if (sy < ClipY1)                      // Top margin crossed?
  {
    int nh = sh - (ClipY1 - sy);        // New height
    _ty += (ClipY1 - sy) * _th / sh;    // Adjust Y coord on texture
    _th = (_th * nh) / sh;              // Adjust height on texture
    sh = nh; sy = ClipY1;
  } /* endif */
  if (sy + sh > ClipY2)                 // Bottom margin crossed?
  {
    int nh = ClipY2 - sy;               // New height
    _th = (_th * nh) / sh;              // Adjust height on texture
    sh = nh;
  } /* endif */

  // cache the texture if we haven't already.
  texture_cache->Cache (hTex);

  // Get texture handle
  csTextureHandleOpenGL *txt_mm = (csTextureHandleOpenGL *)hTex->GetPrivateObject ();
  GLuint texturehandle = ((csTxtCacheData *)txt_mm->GetCacheData ())->Handle;

  // as we are drawing in 2D, we disable some of the commonly used features
  // for fancy 3D drawing
  glShadeModel (GL_FLAT);
  SetGLZBufferFlags (CS_ZBUF_NONE);
  //@@@???glDepthMask (GL_FALSE);

  // if the texture has transparent bits, we have to tweak the
  // OpenGL blend mode so that it handles the transparent pixels correctly
  if (hTex->GetKeyColor () || hTex->GetAlphaMap () || Alpha)
    SetupBlend (CS_FX_ALPHA, 0, false);
  else
    SetupBlend (CS_FX_COPY, 0, false);

  glEnable (GL_TEXTURE_2D);
  glColor4f (1.0, 1.0, 1.0, Alpha ? (1.0 - BYTE_TO_FLOAT (Alpha)) : 1.0);
  glBindTexture (GL_TEXTURE_2D, texturehandle);

  int bitmapwidth = 0, bitmapheight = 0;
  hTex->GetMipMapDimensions (0, bitmapwidth, bitmapheight);

  // convert texture coords given above to normalized (0-1.0) texture
  // coordinates
  float ntx1,nty1,ntx2,nty2;
  ntx1 = (_tx      ) / bitmapwidth;
  ntx2 = (_tx + _tw) / bitmapwidth;
  nty1 = (_ty      ) / bitmapheight;
  nty2 = (_ty + _th) / bitmapheight;

  // draw the bitmap
  glBegin (GL_QUADS);
//    glTexCoord2f (ntx1, nty1);
//    glVertex2i (sx, height - sy - 1);
//    glTexCoord2f (ntx2, nty1);
//    glVertex2i (sx + sw, height - sy - 1);
//    glTexCoord2f (ntx2, nty2);
//    glVertex2i (sx + sw, height - sy - sh - 1);
//    glTexCoord2f (ntx1, nty2);
//    glVertex2i (sx, height - sy - sh - 1);

  // smgh: This works in software opengl and with cswstest
  glTexCoord2f (ntx1, nty1);
  glVertex2i (sx, height - sy - 1);
  glTexCoord2f (ntx2, nty1);
  glVertex2i (sx + sw, height - sy - 1);
  glTexCoord2f (ntx2, nty2);
  glVertex2i (sx + sw, height - (sy+1 + sh));
  glTexCoord2f (ntx1, nty2);
  glVertex2i (sx, height - (sy+1 + sh));
  glEnd ();
}

/* this function is called when the user configures the OpenGL renderer to use
 * 'auto' blend mode.  It tries to figure out how well the driver supports
 * the 2*SRC*DST blend mode--some beta/debug drivers support it badly, some
 * hardware does not support it at all.
 *
 * We check the driver by drawing a polygon with both blend modes:
 *   - we draw using SRC*DST blending and read back the color result, called A
 *   - we draw using 2*SRC*DST blending and read back the color result, called B
 *
 * Ideally B=2*A.  Here we guess that if B > 1.5*A then the 2*SRC*DST mode is
 * reasonably well supported and suggest using 2*SRC*DST mode.  Otherwise we
 * suggest using SRC*DST mode which is pretty well supported.
 */
void csGraphics3DOGLCommon::Guess_BlendMode (GLenum *src, GLenum*dst)
{
  // colors of the 2 polys to blend
  float testcolor1[3] = {0.5,0.5,0.5};
  float testcolor2[3] = {0.5,0.5,0.5};

  // these will hold the resultant color intensities
  float blendresult1[3], blendresult2[3];

  Report (CS_REPORTER_SEVERITY_NOTIFY,
  	"Attempting to determine best blending mode to use.");

  // draw the polys
  glDisable (GL_TEXTURE_2D);
  glDisable (GL_DEPTH_TEST);
  glShadeModel (GL_FLAT);

  // blend mode one

  glDisable (GL_BLEND);
  glColor3fv (testcolor1);
  glBegin (GL_QUADS);
  glVertex2i (0,0); glVertex2i(5,0); glVertex2i(5,5); glVertex2i(0,5);
  glEnd ();

  glEnable (GL_BLEND);
  glBlendFunc (GL_DST_COLOR, GL_ZERO);
  glColor3fv (testcolor2);
  glBegin (GL_QUADS);
  glVertex2i (0,0); glVertex2i(5,0); glVertex2i(5,5); glVertex2i(0,5);
  glEnd ();

  glReadPixels (2,2,1,1,GL_RGB,GL_FLOAT, &blendresult1);

  // blend mode two

  glDisable (GL_BLEND);
  glColor3fv (testcolor1);
  glBegin (GL_QUADS);
  glVertex2i (0,0); glVertex2i (5,0); glVertex2i (5,5); glVertex2i (0,5);
  glEnd ();

  glEnable (GL_BLEND);
  glBlendFunc (GL_DST_COLOR, GL_SRC_COLOR);
  glColor3fv (testcolor2);
  glBegin (GL_QUADS);
  glVertex2i (0,0); glVertex2i (5,0); glVertex2i (5,5); glVertex2i (0,5);
  glEnd ();

  glReadPixels (2,2,1,1,GL_RGB,GL_FLOAT, &blendresult2);

  Report (CS_REPORTER_SEVERITY_NOTIFY,
  	"Blend mode values are %f and %f...",
	blendresult1[1],
	blendresult2[1]);

  // compare the green component between the two results, A and B.  In the
  // ideal case B = 2*A.  If SRC*DST blend mode is supported but 2*SRC*DST is
  // not, then B = A.  So we guess that if B > 1.5*A that the 2*SRC*DST is
  // 'pretty well' supported and go with that.  Otherwise, fall back on the
  // normal SRC*DST mode.

  float resultA = blendresult1[1];
  float resultB = blendresult2[1];

  if (resultB > 1.5 * resultA)
  {
    Report(CS_REPORTER_SEVERITY_NOTIFY, "using 'multiplydouble' blend mode.");

    *src = GL_DST_COLOR;
    *dst = GL_SRC_COLOR;
  }
  else
  {
    Report(CS_REPORTER_SEVERITY_NOTIFY, "using 'multiply' blend mode.");

    *src = GL_DST_COLOR;
    *dst = GL_ZERO;
  }
}

