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
#define SCF_DEBUG

#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "cssysdef.h"
#include "cssys/sysfunc.h"
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
#include "ogl_polybuf.h"
#include "iutil/cfgfile.h"
#include "iutil/cmdline.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "iutil/plugin.h"
#include "iutil/eventh.h"
#include "iutil/virtclk.h"
#include "iutil/comp.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/lightmap.h" //@@@
#include "ivideo/graph2d.h"
#include "csutil/garray.h"
#include "csutil/cscolor.h"
#include "csutil/csstring.h"
#include "csutil/csstrvec.h"
#include "csutil/util.h"
#include "csgfx/rgbpixel.h"
#include "qsqrt.h"

#include "ivideo/effects/efserver.h"
#include "ivideo/effects/efdef.h"
#include "ivideo/effects/eftech.h"
#include "ivideo/effects/efpass.h"
#include "ivideo/effects/eflayer.h"
#include "ivideo/effects/efstring.h"
#include "effectdata.h"

#define BYTE_TO_FLOAT(x) ((x) * (1.0 / 255.0))

/*=========================================================================
 Static growing array declaration for DrawTriangleMesh
=========================================================================*/
// smgh moved it here, no longer segfaults on exit as a consequence..
// Also IncRefing and DecRefing in the ctor/dtor, as the auxiliary buffer
// dynamic textures will utilise multiple instances of csGraphics3DOGLCommon

/// Static vertex array.
CS_TYPEDEF_GROWING_ARRAY_REF (ogl_g3dcom_tr_verts, csVector3);
CS_IMPLEMENT_STATIC_VAR (Get_tr_verts, ogl_g3dcom_tr_verts, ())
/// Static uv array.
CS_TYPEDEF_GROWING_ARRAY_REF (ogl_g3dcom_uv_verts, csVector2);
CS_IMPLEMENT_STATIC_VAR (Get_uv_verts, ogl_g3dcom_uv_verts, ())
/// Static uv array for multi-texture.
CS_TYPEDEF_GROWING_ARRAY_REF (ogl_g3dcom_uv_mul_verts, csVector2);
CS_IMPLEMENT_STATIC_VAR (Get_uv_mul_verts, ogl_g3dcom_uv_mul_verts, ())
/// Array with colors.
CS_TYPEDEF_GROWING_ARRAY_REF (ogl_g3dcom_color_verts, csColor);
CS_IMPLEMENT_STATIC_VAR (Get_color_verts, ogl_g3dcom_color_verts, ())
/// Array with RGBA colors.
CS_TYPEDEF_GROWING_ARRAY_REF (ogl_g3dcom_rgba_verts, GLfloat);
CS_IMPLEMENT_STATIC_VAR (Get_rgba_verts, ogl_g3dcom_rgba_verts, ())

/// Array for clipping.
CS_TYPEDEF_GROWING_ARRAY_REF (ogl_g3dcom_clipped_triangles, csTriangle);
CS_IMPLEMENT_STATIC_VAR (Get_clipped_triangles, ogl_g3dcom_clipped_triangles, ())
/// Array for clipping.
CS_TYPEDEF_GROWING_ARRAY_REF (ogl_g3dcom_clipped_translate, int);
CS_IMPLEMENT_STATIC_VAR (Get_clipped_translate, ogl_g3dcom_clipped_translate, ())
/// Array for clipping.
CS_TYPEDEF_GROWING_ARRAY_REF (ogl_g3dcom_clipped_vertices, csVector3);
CS_IMPLEMENT_STATIC_VAR (Get_clipped_vertices, ogl_g3dcom_clipped_vertices, ())
/// Array for clipping.
CS_TYPEDEF_GROWING_ARRAY_REF (ogl_g3dcom_clipped_texels, csVector2);
CS_IMPLEMENT_STATIC_VAR (Get_clipped_texels, ogl_g3dcom_clipped_texels, ())
/// Array for clipping.
CS_TYPEDEF_GROWING_ARRAY_REF (ogl_g3dcom_clipped_colors, csColor);
CS_IMPLEMENT_STATIC_VAR (Get_clipped_colors, ogl_g3dcom_clipped_colors, ())
/// Array for clipping.
CS_TYPEDEF_GROWING_ARRAY_REF (ogl_g3dcom_clipped_fog, G3DFogInfo);
CS_IMPLEMENT_STATIC_VAR (Get_clipped_fog, ogl_g3dcom_clipped_fog, ())

/// Array for clipping.
CS_TYPEDEF_GROWING_ARRAY_REF (ogl_g3dcom_clipped_user, float);
CS_IMPLEMENT_STATIC_VAR (Get_clipped_user0, ogl_g3dcom_clipped_user, ())
CS_IMPLEMENT_STATIC_VAR (Get_clipped_user1, ogl_g3dcom_clipped_user, ())
CS_IMPLEMENT_STATIC_VAR (Get_clipped_user2, ogl_g3dcom_clipped_user, ())
CS_IMPLEMENT_STATIC_VAR (Get_clipped_user3, ogl_g3dcom_clipped_user, ())
CS_IMPLEMENT_STATIC_VAR (Get_clipped_user4, ogl_g3dcom_clipped_user, ())
CS_IMPLEMENT_STATIC_VAR (Get_clipped_user5, ogl_g3dcom_clipped_user, ())
CS_IMPLEMENT_STATIC_VAR (Get_clipped_user6, ogl_g3dcom_clipped_user, ())
CS_IMPLEMENT_STATIC_VAR (Get_clipped_user7, ogl_g3dcom_clipped_user, ())
CS_IMPLEMENT_STATIC_VAR (Get_clipped_user8, ogl_g3dcom_clipped_user, ())
CS_IMPLEMENT_STATIC_VAR (Get_clipped_user9, ogl_g3dcom_clipped_user, ())
CS_IMPLEMENT_STATIC_VAR (Get_clipped_user10, ogl_g3dcom_clipped_user, ())
CS_IMPLEMENT_STATIC_VAR (Get_clipped_user11, ogl_g3dcom_clipped_user, ())
CS_IMPLEMENT_STATIC_VAR (Get_clipped_user12, ogl_g3dcom_clipped_user, ())
CS_IMPLEMENT_STATIC_VAR (Get_clipped_user13, ogl_g3dcom_clipped_user, ())
CS_IMPLEMENT_STATIC_VAR (Get_clipped_user14, ogl_g3dcom_clipped_user, ())
CS_IMPLEMENT_STATIC_VAR (Get_clipped_user15, ogl_g3dcom_clipped_user, ())

CS_IMPLEMENT_STATIC_VAR_ARRAY (GetStaticClipInfo1, csClipInfo, [100])
CS_IMPLEMENT_STATIC_VAR_ARRAY (GetStaticClipInfo2, csClipInfo, [100])
CS_IMPLEMENT_STATIC_VAR_ARRAY (GetStaticClipInfo3, csClipInfo, [100])
CS_IMPLEMENT_STATIC_VAR_ARRAY (GetStaticClipInfo4, csClipInfo, [100])

static ogl_g3dcom_tr_verts *tr_verts = NULL;
static ogl_g3dcom_uv_verts *uv_verts = NULL;
static ogl_g3dcom_uv_mul_verts *uv_mul_verts = NULL;
static ogl_g3dcom_color_verts *color_verts = NULL;
static ogl_g3dcom_rgba_verts *rgba_verts = NULL;
static ogl_g3dcom_clipped_triangles *clipped_triangles = NULL;
static ogl_g3dcom_clipped_translate *clipped_translate = NULL;
static ogl_g3dcom_clipped_vertices *clipped_vertices = NULL;
static ogl_g3dcom_clipped_texels *clipped_texels = NULL;
static ogl_g3dcom_clipped_colors *clipped_colors = NULL;
static ogl_g3dcom_clipped_fog *clipped_fog = NULL;
static ogl_g3dcom_clipped_user *clipped_user[CS_VBUF_TOTAL_USERA] = {
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

/*=========================================================================
 Method implementations
=========================================================================*/

SCF_IMPLEMENT_IBASE(csGraphics3DOGLCommon)
  SCF_IMPLEMENTS_INTERFACE(iGraphics3D)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iEffectClient)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGraphics3DOGLCommon::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGraphics3DOGLCommon::eiEffectClient)
  SCF_IMPLEMENTS_INTERFACE (iEffectClient)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csGraphics3DOGLCommon::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

csGraphics3DOGLCommon* csGraphics3DOGLCommon::ogl_g3d = NULL;
iGLStateCache* csGraphics3DOGLCommon::statecache = NULL;

#define USE_OGL_EXT(ext) \
  bool csGraphics3DOGLCommon::ext = false;
#include "ogl_suppext.h"
#undef USE_OGL_EXT

# define _CSGLEXT_
# define CSGL_FOR_ALL
# define CSGL_FUNCTION(fType,fName) \
fType  csGraphics3DOGLCommon::fName = (fType) NULL;
# include "csglext.h"
# undef CSGL_FUNCTION

float sAc, sBc, sCc, sDc;
//csMatrix3 sM;
//csVector3 sV;

csGraphics3DOGLCommon::csGraphics3DOGLCommon (iBase* parent):
  object_reg (NULL)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiEffectClient);

  scfiEventHandler = NULL;

  ogl_g3d = this;
  texture_cache = NULL;
  lightmap_cache = NULL;
  txtmgr = NULL;
  vbufmgr = NULL;
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

  ARB_texture_compression = false;
  clipper = NULL;
  cliptype = CS_CLIPPER_NONE;
  toplevel_init = false;
  stencil_init = false;
  planes_init = false;
  frustum_valid = false;

  if (!tr_verts)
  {
    tr_verts = Get_tr_verts ();
    uv_verts = Get_uv_verts ();
    uv_mul_verts = Get_uv_mul_verts ();
    color_verts = Get_color_verts ();
    rgba_verts = Get_rgba_verts ();
    clipped_triangles = Get_clipped_triangles ();
    clipped_translate = Get_clipped_translate ();
    clipped_vertices = Get_clipped_vertices ();
    clipped_texels = Get_clipped_texels ();
    clipped_colors = Get_clipped_colors ();
    clipped_fog = Get_clipped_fog ();
    clipped_user[0] = Get_clipped_user0();
    clipped_user[1] = Get_clipped_user1();
    clipped_user[2] = Get_clipped_user2();
    clipped_user[3] = Get_clipped_user3();
    clipped_user[4] = Get_clipped_user4();
    clipped_user[5] = Get_clipped_user5();
    clipped_user[6] = Get_clipped_user6();
    clipped_user[7] = Get_clipped_user7();
    clipped_user[8] = Get_clipped_user8();
    clipped_user[9] = Get_clipped_user9();
    clipped_user[10] = Get_clipped_user10();
    clipped_user[11] = Get_clipped_user11();
    clipped_user[12] = Get_clipped_user12();
    clipped_user[13] = Get_clipped_user13();
    clipped_user[14] = Get_clipped_user14();
    clipped_user[15] = Get_clipped_user15();
  }

  // See note above.
  tr_verts->IncRef ();
  uv_verts->IncRef ();
  uv_mul_verts->IncRef ();
  color_verts->IncRef ();
  rgba_verts->IncRef ();
  clipped_triangles->IncRef ();
  clipped_translate->IncRef ();
  clipped_vertices->IncRef ();
  clipped_texels->IncRef ();
  clipped_colors->IncRef ();
  clipped_fog->IncRef ();
  for( int i=0; i<CS_VBUF_TOTAL_USERA; i++ )
    clipped_user[i]->IncRef ();

  // Are we going to use the inverted orthographic projection matrix?
  inverted = false;
}

csGraphics3DOGLCommon::~csGraphics3DOGLCommon ()
{
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
    if (q != 0)
      q->RemoveListener (scfiEventHandler);
    scfiEventHandler->DecRef ();
  }

  Close ();

  // see note above
  tr_verts->DecRef ();
  uv_verts->DecRef ();
  uv_mul_verts->DecRef ();
  color_verts->DecRef ();
  rgba_verts->DecRef ();
  clipped_triangles->DecRef ();
  clipped_translate->DecRef ();
  clipped_vertices->DecRef ();
  clipped_texels->DecRef ();
  clipped_colors->DecRef ();
  clipped_fog->DecRef ();
  int i;
  for (i=0 ; i<CS_VBUF_TOTAL_USERA ; i++)
    clipped_user[i]->DecRef ();
}

void csGraphics3DOGLCommon::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (object_reg, iReporter));
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
  if (!scfiEventHandler)
    scfiEventHandler = new EventHandler (this);
  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (q != 0)
    q->RegisterListener (scfiEventHandler, CSMASK_Broadcast);
  return true;
}

void csGraphics3DOGLCommon::InitGLExtensions ()
{
#define EXT_CONFIG_KEY "Video.OpenGL.UseExtension"
  if (G2D)
  {
    csRef<iOpenGLInterface> G2DGL (SCF_QUERY_INTERFACE (G2D, iOpenGLInterface));
    if (G2DGL)
    {
      const unsigned char* extensions = glGetString(GL_EXTENSIONS);
      if (!extensions)
	return; //  no need to look any further

      // check the extension string for each extension in turn
      char* extbuf = new char[strlen((const char*)extensions)+1];
      strcpy (extbuf, (const char*)extensions);

      char* ext = extbuf;

      while (ext && *ext)
      {
        char* next = strchr(ext, ' ');
        if (next) *next++ = 0;

	csString cfgkey;
	cfgkey << EXT_CONFIG_KEY << "." << ext;

#       define USE_OGL_EXT(extname)     \
	if (!strcmp (ext, "GL_" #extname))    \
	{           \
	  if (!config->GetBool(EXT_CONFIG_KEY   \
		".GL_" #extname, false))      \
	  {           \
	    Report (CS_REPORTER_SEVERITY_NOTIFY,  \
		"... not using %s", "GL_" #extname);  \
	  }           \
	  else            \
	  {           \
	    Report (CS_REPORTER_SEVERITY_NOTIFY,  \
		"... using %s", "GL_" #extname);    \
	    Init_##extname (G2DGL);     \
	  }           \
	} else

#       include "ogl_suppext.h"
#       undef USE_OGL_EXT
	{ /* the last 'else' */ }

	ext = next;
      }
      delete[] extbuf;
    }

    // Some diagnostic info, to detect typos etc.
    csRef<iConfigIterator> it (config->Enumerate (EXT_CONFIG_KEY));
    while (it->Next())
    {
#     define USE_OGL_EXT(extname)           \
      if (!strcmp (it->GetKey(), EXT_CONFIG_KEY ".GL_" #extname)) \
      { continue; } else

#     include "ogl_suppext.h"
#     undef USE_OGL_EXT
      {
	Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "Extension %s was used in config but is "
	  "actually not supported", it->GetKey());
      }
    }
  }
#undef EXT_CONFIG_KEY
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

  csRef<iCommandLineParser> cmdline (CS_QUERY_REGISTRY (object_reg,
    iCommandLineParser));

  const char *driver = cmdline->GetOption ("canvas");
  if (!driver)
    driver = config->GetStr ("Video.OpenGL.Canvas", CS_OPENGL_2D_DRIVER);

  report_gl_errors = config->GetBool ("Video.OpenGL.ReportGLErrors"
#ifdef CS_DEBUG
    , true
#endif
    );

  csRef<iPluginManager> plugin_mgr (
    CS_QUERY_REGISTRY (object_reg, iPluginManager));
  G2D = CS_LOAD_PLUGIN (plugin_mgr, driver, iGraphics2D);
  if (!G2D)
    return false;
  if (!object_reg->Register (G2D, "iGraphics2D"))
  {
    Report (CS_REPORTER_SEVERITY_ERROR,
  "Could not register the canvas!");
    return false;
  }

  G2D->PerformExtension("getstatecache", &statecache);

  width = height = -1;

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
  SetPerspectiveCenter (width / 2, height / 2);

  G3DTriangleMesh mesh;
  int res = 64;
  int num_vertices = (res+1)*(res+1);
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
  mesh.mixmode = CS_FX_GOURAUD;
  mesh.morph_factor = 0;
  mesh.mat_handle = NULL;
  mesh.vertex_fog = NULL;
  mesh.triangles = new csTriangle [mesh.num_triangles];
  csVector3* vertices = new csVector3 [num_vertices];
  csVector2* texels = new csVector2 [num_vertices];
  csColor* colors = new csColor [num_vertices];

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
  for (y = 0; y <= res; y++)
  {
    fy = float (y) / float (res) - .5;
    for (x = 0; x <= res; x++)
    {
      fx = float (x) / float (res) - .5;
      vertices[i].Set (10.*fx, 10.*fy, z);
      texels[i].Set (0, 0);
      colors[i].Set (1, 0, 0);
      i++;
    }
  }
  i = 0;
  t = 0;
  for (y = 0; y < res; y++)
  {
    for (x = 0; x < res; x++)
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

  csRef<iVertexBuffer> vbuf (GetVertexBufferManager ()->CreateBuffer (0));
  GetVertexBufferManager ()->LockBuffer (vbuf, vertices, texels,
    colors, num_vertices, 0);
  mesh.buffers[0] = vbuf;

  if (compute_outer && !GLCaps.need_screen_clipping)
  {
    //========
    // First test clipping geometry against the outer portal (close
    // to screen boundaries).
    //========
    for (i = 0; i < test_mode_cnt; i++)
    {
      clip_outer[0] = test_modes[i].mode;
      int cnt = 0;
      csTicks end = csGetTicks () + 1000;
      while (csGetTicks () < end)
      {
        statecache->SetDepthMask (GL_TRUE);
        glClear (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_USE);
        csReversibleTransform o2c;  // Identity transform.
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
      for (i = 0; i < 3; i++)
        clip_outer[i] = '0';
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
  for (i = 0; i < num_vertices; i++)
    vertices[i].z = z;
  for (i = 0; i < test_mode_cnt; i++)
  {
    if (test_modes[i].mode == 'z') test_modes[i].mode = 'n';
    else if (test_modes[i].mode == 'Z') test_modes[i].mode = 'N';
    clip_optional[0] = test_modes[i].mode;
    int cnt = 0;
    csTicks end = csGetTicks () + 1000;
    while (csGetTicks () < end)
    {
      statecache->SetDepthMask (GL_TRUE);
      glClear (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
      SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_USE);
      csReversibleTransform o2c;  // Identity transform.
      SetObjectToCamera (&o2c);
      DrawTriangleMesh (mesh);
      cnt++;
    }
    test_modes[i].cnt = cnt;
    Report (CS_REPORTER_SEVERITY_NOTIFY,
      "    %d FPS for %c (small clipper)", cnt,
      test_modes[i].mode); fflush (stdout);
  }
  GetVertexBufferManager ()->UnlockBuffer (vbuf);

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

  char buf[4];
  buf[3] = 0;

  for (i = 0; i < 3; i++)
  {
    buf[i] = clip_required[i];
  }
  config->SetStr ("Video.OpenGL.ClipRequired", buf);

  for (i = 0; i < 3; i++)
  {
    buf[i] = clip_outer[i];
  }
  config->SetStr ("Video.OpenGL.ClipOuter", buf);

  for (i = 0; i < 3; i++)
  {
    buf[i] = clip_optional[i];
  }
  config->SetStr ("Video.OpenGL.ClipOptional", buf);
  config->Save ();

  SetClipper (NULL, CS_CLIPPER_NONE);

  FinishDraw ();
  Print (NULL);
}

void csGraphics3DOGLCommon::SharedInitialize (csGraphics3DOGLCommon *d)
{
  config.AddConfig(object_reg, "/config/opengl.cfg");

  txtmgr = d->txtmgr;
  vbufmgr = d->vbufmgr;
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
  if (!G2D->Open ())
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error opening Graphics2D context.");
    // set "not opened" flag
    width = height = -1;
    return false;
  }

#define OGLCONFIGS_PREFIX "Video.OpenGL.Config."
#define OGLCONFIGS_SUFFIX ".config"

  CS_ASSERT (object_reg != NULL);

  const char *sGL_RENDERER   = (const char *)glGetString (GL_RENDERER);
  const char *sGL_VENDOR     = (const char *)glGetString (GL_VENDOR);
  const char *sGL_VERSION    = (const char *)glGetString (GL_VERSION);
  const char *sGL_EXTENSIONS = (const char *)glGetString (GL_EXTENSIONS);

  csStrVector oglconfigs;

  csRef<iConfigIterator> it (config->Enumerate (OGLCONFIGS_PREFIX));
  while (it->Next ())
  {
    csString oglconfig;
    char const* key = it->GetKey(true);
    char const* dot = strchr(key, '.');
    if (dot == 0)
      oglconfig = key;
    else
      oglconfig.Append(key, dot - key); // Drop the '.' and all that follows.

    if (oglconfigs.FindKey (oglconfig.GetData()) == -1)
    {
      bool apply = true;
      int count = 0;

#define CHECK_STRING(str) \
      if (s##str) \
      { \
        csString s_##str; \
	s_##str << OGLCONFIGS_PREFIX << oglconfig << '.' << #str; \
	if (config->KeyExists(s_##str.GetData())) \
	{ \
	  count++; \
	  apply &= csGlobMatches(s##str, config->GetStr(s_##str.GetData())); \
	} \
      }

      CHECK_STRING (GL_VENDOR);
      CHECK_STRING (GL_VERSION);
      CHECK_STRING (GL_RENDERER);
      CHECK_STRING (GL_EXTENSIONS);
#undef CHECK_STRING

      if (apply && count != 0)
      {
	csString cfgfkey;
	cfgfkey << OGLCONFIGS_PREFIX << oglconfig << OGLCONFIGS_SUFFIX;
	csString cfgfile("/config/");
	cfgfile << config->GetStr(cfgfkey.GetData());
	config.AddConfig(object_reg, cfgfile.GetData(),
	  iConfigManager::ConfigPriorityPlugin + 1);
	Report (CS_REPORTER_SEVERITY_NOTIFY, "read config for '%s' from %s",
	  oglconfig.GetData(), cfgfile.GetData());
      }
      oglconfigs.Push(csStrNew (oglconfig.GetData()));
    }
  }

#undef OGLCONFIGS_PREFIX
#undef OGLCONFIGS_SUFFIX

  G2D->PerformExtension("configureopengl");

  vbufmgr = new csTriangleArrayVertexBufferManager (object_reg);

  m_renderstate.dither = config->GetBool ("Video.OpenGL.EnableDither", false);

  z_buf_mode = CS_ZBUF_NONE;
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

  // See if we find any OpenGL extensions, and set the corresponding
  // flags.
  CommonOpen ();

  if (m_renderstate.dither)
    statecache->EnableState (GL_DITHER);
  else
    statecache->DisableState (GL_DITHER);

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
  statecache->SetTexture (GL_TEXTURE_2D, m_fogtexturehandle);

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

  int max_cache_size = // 128mb combined cache per default
    config->GetInt("Video.OpenGL.MaxTextureCache", 128) * 1024*1024; 
  texture_cache = new OpenGLTextureCache (max_cache_size, this);
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
  statecache->EnableState (GL_CULL_FACE);
  glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  statecache->DisableState (GL_BLEND);

  // Now that we know what pixelformat we use, clue the texture manager in.
  txtmgr = new csTextureManagerOpenGL (object_reg, G2D, config, this);
  txtmgr->SetPixelFormat (*G2D->GetPixelFormat ());

  PerfTest ();

  glCullFace (GL_FRONT);
  statecache->EnableState (GL_CULL_FACE);
  glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  statecache->DisableState (GL_BLEND);

  effectserver = CS_QUERY_REGISTRY(object_reg, iEffectServer);
  if( !effectserver )
  {
    csRef<iPluginManager> plugin_mgr (
      CS_QUERY_REGISTRY (object_reg, iPluginManager));
    effectserver = CS_LOAD_PLUGIN (plugin_mgr,
      "crystalspace.video.effects.stdserver", iEffectServer);
    if (!effectserver)
    {
      Report (CS_REPORTER_SEVERITY_ERROR, "FATAL: Wasn't able to load effect"
	  " server plugin. Did you compile it (effects)?");
      // This is the only way to stop here. As the return value from Open() is
      // ignored :-/
      csRef<iEventQueue> queue = CS_QUERY_REGISTRY (object_reg, iEventQueue);
      if (queue)
	queue->GetEventOutlet()->Broadcast (cscmdQuit);
    }

    object_reg->Register (effectserver, "iEffectServer");
  }
  //csEffectStrings::InitStrings( effectserver );
  InitStockEffects();

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
  InitGLExtensions ();

  // Initialize the default method calls
  DrawPolygonCall = &csGraphics3DOGLCommon::DrawPolygonSingleTexture;

#if 0
  if (ARB_multitexture)
    DrawPolygonCall = &csGraphics3DOGLCommon::DrawPolygonMultiTexture;
#endif
}

void csGraphics3DOGLCommon::SharedOpen (csGraphics3DOGLCommon *d)
{
  CommonOpen ();

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
  if (txtmgr)
  {
    txtmgr->Clear ();
    txtmgr->DecRef (); txtmgr = NULL;
  }
  if (vbufmgr)
  {
    vbufmgr->DecRef (); vbufmgr = NULL;
  }

  Report (CS_REPORTER_SEVERITY_NOTIFY,
    "Peak GL texture cache size: %1.2f MB",
    ((float)texture_cache->GetPeakTotalTextureSize() / (1024.0f * 1024.0f)));

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

void csGraphics3DOGLCommon::SetGlOrtho (bool inverted)
{
  if (render_target)
  {
    if (inverted)
      glOrtho (0., (GLdouble) (width+1), (GLdouble) (height+1), 0., -1.0, 10.0);
    else
      glOrtho (0., (GLdouble) (width+1), 0., (GLdouble) (height+1), -1.0, 10.0);
  }
  else
  {
    if (inverted)
      glOrtho (0., (GLdouble) width, (GLdouble) height, 0., -1.0, 10.0);
    else
      glOrtho (0., (GLdouble) width, 0., (GLdouble) height, -1.0, 10.0);
  }
}

bool csGraphics3DOGLCommon::BeginDraw (int DrawFlags)
{
  if ((G2D->GetWidth() != width) ||
      (G2D->GetHeight() != height))
    SetDimensions (G2D->GetWidth(), G2D->GetHeight());

  if (DrawMode & CSDRAW_3DGRAPHICS)
  {
    FlushDrawPolygon ();
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

  if (render_target)
  {
    int txt_w, txt_h;
    render_target->GetMipMapDimensions (0, txt_w, txt_h);
    if (!rt_cliprectset)
    {
      G2D->GetClipRect (rt_old_minx, rt_old_miny, rt_old_maxx, rt_old_maxy);
      G2D->SetClipRect (-1, -1, txt_w+1, txt_h+1);
      rt_cliprectset = true;

      glMatrixMode (GL_PROJECTION);
      glLoadIdentity ();
      SetGlOrtho (false);
      glViewport (1, -1, width+1, height+1);
    }

    if (!rt_onscreen)
    {
      texture_cache->Cache (render_target);
      GLuint handle = ((csTxtCacheData *)render_target->GetCacheData ())
      	->Handle;
      statecache->SetShadeModel (GL_FLAT);
      statecache->EnableState (GL_TEXTURE_2D);
      glColor4f (1.0f, 1.0f, 1.0f, 1.0f);
      statecache->SetTexture (GL_TEXTURE_2D, handle);
      SetupBlend (CS_FX_COPY, 0, false);
      SetGLZBufferFlags (CS_ZBUF_NONE);

      glBegin (GL_QUADS);
      glTexCoord2f (0, 0); glVertex2i (0, height-txt_h+1);
      glTexCoord2f (0, 1); glVertex2i (0, height-0+1);
      glTexCoord2f (1, 1); glVertex2i (txt_w, height-0+1);
      glTexCoord2f (1, 0); glVertex2i (txt_w, height-txt_h+1);
      glEnd ();
      rt_onscreen = true;
    }
  }

  if (DrawFlags & CSDRAW_CLEARZBUFFER)
  {
    statecache->SetDepthMask (GL_TRUE);
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
  }

  if (DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
  {
    G2D->FinishDraw ();
  }
  DrawMode = 0;

  if (render_target)
  {
    if (rt_cliprectset)
    {
      rt_cliprectset = false;
      G2D->SetClipRect (rt_old_minx, rt_old_miny, rt_old_maxx, rt_old_maxy);
      glMatrixMode (GL_PROJECTION);
      glLoadIdentity ();
      glOrtho (0., width, 0., height, -1.0, 10.0);
      glViewport (0, 0, width, height);
    }

    if (rt_onscreen)
    {
      rt_onscreen = false;
      statecache->EnableState (GL_TEXTURE_2D);
      SetGLZBufferFlags (CS_ZBUF_NONE);
      SetupBlend (CS_FX_COPY, 0, false);
      statecache->DisableState (GL_ALPHA_TEST);
      int txt_w, txt_h;
      render_target->GetMipMapDimensions (0, txt_w, txt_h);
      csTextureHandleOpenGL* tex_mm = (csTextureHandleOpenGL *)
	    render_target->GetPrivateObject ();
      csTextureOpenGL *tex_0 = tex_mm->vTex[0];
      csTxtCacheData *tex_data = (csTxtCacheData*)render_target->GetCacheData();
      if (tex_data)
      {
        // Texture is in tha cache, update texture directly.
        statecache->SetTexture (GL_TEXTURE_2D, tex_data->Handle);
	// Texture was not used as a render target before.
	// Make some necessary adjustments.
	if (!tex_mm->was_render_target)
	{
	  if (!(tex_mm->GetFlags() & CS_TEXTURE_NOMIPMAPS))
	  {
	    if (SGIS_generate_mipmap)
	    {
	      glTexParameteri (GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	    }
	    else
	    {
	      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		texture_cache->GetBilinearMapping() ? GL_LINEAR : GL_NEAREST);
	    }
	  }
	  tex_mm->was_render_target = true;
	}
        glCopyTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 1, height-txt_h,
      	  txt_w, txt_h, 0);
      }
      else
      {
        // Not in cache.
       #ifdef GL_VERSION_1_2x
        if (pfmt.PixelBytes == 2)
        {
          char* buffer = new char[2*txt_w*txt_h]; // @@@ Alloc elsewhere!!!
          glReadPixels (0, height-txt_h, txt_w, txt_h,
		GL_RGB, GL_UNSIGNED_SHORT_5_6_5, buffer);

          csRGBpixel *dst = tex_0->get_image_data ();
          uint16 bb = 8 - pfmt.BlueBits;
          uint16 gb = 8 - pfmt.GreenBits;
          uint16 rb = 8 - pfmt.RedBits;
          uint16 *src = (uint16*) buffer;
          int i;
          for (i = 0 ; i < width*height ; i++, src++, dst++)
          {
	    dst->red = ((*src & pfmt.RedMask) >> pfmt.RedShift) << rb;
	    dst->green = ((*src & pfmt.GreenMask) >> pfmt.GreenShift) << gb;
	    dst->blue = ((*src & pfmt.BlueMask) >> pfmt.BlueShift) << bb;
          }
	  delete[] buffer;
        }
        else
          glReadPixels (1, height-txt_h, txt_w, txt_h,
            GL_RGBA, GL_UNSIGNED_BYTE, tex_0->get_image_data());
       #else
        glReadPixels (1, height-txt_h, txt_w, txt_h, tex_mm->SourceFormat (),
          tex_mm->SourceType (), tex_0->get_image_data ());
       #endif
      }
    }
  }
  render_target = NULL;
}

void csGraphics3DOGLCommon::Print (csRect * area)
{
  if (fps_limit)
  {
    csTicks elapsed_time, current_time;
    csRef<iVirtualClock> vc (CS_QUERY_REGISTRY (object_reg, iVirtualClock));
    elapsed_time = vc->GetElapsedTicks ();
    current_time = vc->GetCurrentTicks ();
    /// Smooth last n frames, to avoid jitter when objects appear/disappear.
    static int num = 10;
    static int times[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    static int cur = 0;
    static int totaltime = 0;
    totaltime -= times[cur];
    times[cur] = elapsed_time;
    totaltime += times[cur];
    cur = (cur+1)%num;
    if (totaltime/10 < fps_limit) csSleep (fps_limit - totaltime/10);
  }
  G2D->Print (area);
}

void csGraphics3DOGLCommon::DrawTriangleMeshEdges (G3DTriangleMesh& mesh)
{
  int i;
  int color = G2D->FindRGB (255, 255, 255);
  int num_vertices = mesh.buffers[0]->GetVertexCount ();

  //===========
  // Do vertex tweening and/or transformation to camera space
  // if any of those are needed. When this is done 'verts' will
  // point to an array of camera vertices.
  //===========
  csVector3* f1 = mesh.buffers[0]->GetVertices ();
  csVector3* work_verts;

  if (num_vertices > tr_verts->Limit ())
  {
    tr_verts->SetLimit (num_vertices);
    uv_verts->SetLimit (num_vertices);
    color_verts->SetLimit (num_vertices);
  }

  if (mesh.num_vertices_pool > 1)
  {
    // Vertex morphing.
    float tr = mesh.morph_factor;
    float remainder = 1 - tr;
    csVector3* f2 = mesh.buffers[1]->GetVertices ();
    for (i = 0 ; i < num_vertices ; i++)
      (*tr_verts)[i] = tr * f2[i] + remainder * f1[i];
    work_verts = tr_verts->GetArray ();
  }
  else
  {
    work_verts = f1;
  }
  csTriangle *triangles = mesh.triangles;

  glPushAttrib (GL_ENABLE_BIT|GL_COLOR_BUFFER_BIT|GL_CURRENT_BIT|
    GL_DEPTH_BUFFER_BIT);
  statecache->DisableState (GL_DEPTH_TEST);
  statecache->DisableState (GL_BLEND);
  for (i = 0 ; i < mesh.num_triangles ; i++)
  {
    int a = triangles[i].a;
    int b = triangles[i].b;
    int c = triangles[i].c;
    csVector3 va = work_verts[a];
    csVector3 vb = work_verts[b];
    csVector3 vc = work_verts[c];
    if (mesh.vertex_mode == G3DTriangleMesh::VM_VIEWSPACE)
    {
      // We have 3D coordinates that are already transformed to camera space
      // (in 3 floats).
    }
    else
    {
      // We have 3D coordinates that are not transformed to camera space
      // (in 3 floats).
      va = o2c.Other2This (va);
      vb = o2c.Other2This (vb);
      vc = o2c.Other2This (vc);
    }
    if (va.z < .01 || vb.z < .01 || vc.z < .01) continue;
    float iz;
    float x1, y1, x2, y2, x3, y3;
    iz = aspect / va.z;
    x1 = va.x * iz + asp_center_x;
    y1 = height - va.y * iz - asp_center_y;
    iz = aspect / vb.z;
    x2 = vb.x * iz + asp_center_x;
    y2 = height - vb.y * iz - asp_center_y;
    iz = aspect / vc.z;
    x3 = vc.x * iz + asp_center_x;
    y3 = height - vc.y * iz - asp_center_y;

    G2D->DrawLine (x1, y1, x2, y2, color);
    G2D->DrawLine (x2, y2, x3, y3, color);
    G2D->DrawLine (x3, y3, x1, y1, color);
  }
  glPopAttrib ();
}

void csGraphics3DOGLCommon::DebugDrawElements (iGraphics2D* g2d,
  int num_tri3, int* tris,
    GLfloat* verts, int color, bool coords3d, bool transformed)
{
  glPushAttrib (GL_ENABLE_BIT|GL_COLOR_BUFFER_BIT|GL_CURRENT_BIT|
    GL_DEPTH_BUFFER_BIT);
  statecache->DisableState (GL_DEPTH_TEST);
  statecache->DisableState (GL_BLEND);
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

static float GetAlpha (uint mode, float m_alpha, bool txt_alpha)
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


float csGraphics3DOGLCommon::SetupBlend (uint mode,
  float m_alpha, bool txt_alpha)
{

  // Note: In all explanations of Mixing:
  // Color: resulting color
  // SRC:   Color of the texel (content of the texture to be drawn)
  // DEST:  Color of the pixel on screen
  // Alpha: Alpha value of the polygon
  bool enable_blending = true;
  switch (mode & (CS_FX_MASK_MIXMODE | CS_FX_EXTRA_MODES))
  {
    case CS_FX_SRCDST:
      statecache->SetBlendFunc (ogl_g3d->m_config_options.m_lightmap_src_blend,
             ogl_g3d->m_config_options.m_lightmap_dst_blend);
      break;
    case CS_FX_HALOOVF:
      statecache->SetBlendFunc (GL_SRC_ALPHA, GL_ONE);
      break;
    case CS_FX_MULTIPLY:
      // Color = SRC * DEST +   0 * SRC = DEST * SRC
      m_alpha = 1.0f;
      statecache->SetBlendFunc (GL_ZERO, GL_SRC_COLOR);
      break;
    case CS_FX_MULTIPLY2:
      // Color = SRC * DEST + DEST * SRC = 2 * DEST * SRC
      m_alpha = 1.0f;
      statecache->SetBlendFunc (GL_DST_COLOR, GL_SRC_COLOR);
      break;
    case CS_FX_ADD:
      // Color = 1 * DEST + 1 * SRC = DEST + SRC
      m_alpha = 1.0f;
      statecache->SetBlendFunc (GL_ONE, GL_ONE);
      break;
    case CS_FX_ALPHA:
      // Color = Alpha * DEST + (1-Alpha) * SRC
      statecache->SetBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      break;
    case CS_FX_TRANSPARENT:
      // Color = 1 * DEST + 0 * SRC
      m_alpha = 0.0f;
      statecache->SetBlendFunc (GL_ZERO, GL_ONE);
      break;
    case CS_FX_COPY:
    default:
      enable_blending = txt_alpha;
      if (txt_alpha)
      {
        // Color = 0 * DEST + 1 * SRC = SRC
        m_alpha = 1.0f;
        statecache->SetBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      }
      else
      {
	m_alpha = 0;
      }
      break;
  }

  if (enable_blending)
    statecache->EnableState (GL_BLEND);
  else
    statecache->DisableState (GL_BLEND);

  return m_alpha;
}

uint prev_ct = 0; // @@@ Move to class (static).

void csGraphics3DOGLCommon::SetClientStates (uint ct)
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
    statecache->EnableState (GL_STENCIL_TEST);
      glClearStencil (0);
    glClear (GL_STENCIL_BUFFER_BIT);
    statecache->SetStencilFunc (GL_ALWAYS, 1, 1);
    statecache->SetStencilOp (GL_REPLACE, GL_REPLACE, GL_REPLACE);
    int nv = clipper->GetVertexCount ();
    csVector2* v = clipper->GetClipPoly ();
    glColor4f (0, 0, 0, 0);
    statecache->SetShadeModel (GL_FLAT);
    SetGLZBufferFlags (CS_ZBUF_NONE);
    statecache->DisableState (GL_TEXTURE_2D);
    SetupBlend (CS_FX_TRANSPARENT, 0, false);
    glBegin (GL_TRIANGLE_FAN);
    int i;
    for (i = 0 ; i < nv ; i++)
      glVertex2f (v[i].x, v[i].y);
    glEnd ();
    statecache->DisableState (GL_STENCIL_TEST);
  }
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
    if (txt_handle)
    {
      txt_mm = (csTextureHandleOpenGL *)txt_handle->GetPrivateObject ();
      tex_transp = txt_mm->GetKeyColor () || txt_mm->GetAlphaMap ();
      // Initialize our static drawing information and cache
      // the texture in the texture cache (if this is not already the case).
      CacheTexture (queue.mat_handle);
      texturecache_data = (csTxtCacheData *)txt_mm->GetCacheData ();
      texturehandle = texturecache_data->Handle;
    }
  }

  float flat_r = queue.flat_color_r;
  float flat_g = queue.flat_color_g;
  float flat_b = queue.flat_color_b;

  float alpha = BYTE_TO_FLOAT (queue.mixmode & CS_FX_MASK_ALPHA);
  alpha = SetupBlend (queue.mixmode, alpha, tex_transp);

  if (m_renderstate.textured && txt_handle)
  {
    statecache->EnableState (GL_TEXTURE_2D);
    if (txt_mm->GetKeyColor() && !(alpha < OPENGL_KEYCOLOR_MIN_ALPHA))
    {
      statecache->EnableState (GL_ALPHA_TEST);
      statecache->SetAlphaFunc (GL_GEQUAL, OPENGL_KEYCOLOR_MIN_ALPHA);
      SetupBlend (queue.mixmode, 1.0f, false);
    }
    else
    {
      statecache->DisableState (GL_ALPHA_TEST);
    }
  }
  else
  {
    statecache->DisableState (GL_TEXTURE_2D);
    csRGBpixel color;
    if (txt_handle)
    {
      txt_handle->GetMeanColor (color.red, color.green, color.blue);
      flat_r = BYTE_TO_FLOAT (color.red);
      flat_g = BYTE_TO_FLOAT (color.green);
      flat_b = BYTE_TO_FLOAT (color.blue);
    }
    else if (mat_handle)
    {
      mat_handle->GetFlatColor (color);
      flat_r = BYTE_TO_FLOAT (color.red);
      flat_g = BYTE_TO_FLOAT (color.green);
      flat_b = BYTE_TO_FLOAT (color.blue);
    }
  }

  SetGLZBufferFlags (queue.z_buf_mode);

  glColor4f (flat_r, flat_g, flat_b, alpha);

  if (txt_handle)
    statecache->SetTexture (GL_TEXTURE_2D, texturehandle);

  //=================
  // Pass 1: The unlit texture with optional gouraud shading.
  // Gouraud shading will be delayed to after the multiple texture layers
  // if we have them.
  //=================
  // Only do gouraud shading in the first pass if we don't have
  // extra material layers.
  if (gouraud && !multimat)
  {
    statecache->SetShadeModel (GL_SMOOTH);
    SetClientStates (CS_CLIENTSTATE_ALL);
    glColorPointer (4, GL_FLOAT, 0, queue.glcol);
  }
  else
  {
    SetClientStates (CS_CLIENTSTATE_VT);
    statecache->SetShadeModel (GL_FLAT);
  }
  glVertexPointer (4, GL_FLOAT, 0, queue.glverts);
  glTexCoordPointer (2, GL_FLOAT, 0, queue.gltxt);
  glDrawElements (GL_TRIANGLES, queue.num_triangles*3, GL_UNSIGNED_INT,
      queue.tris);
  if (gouraud && !multimat)
  {
    statecache->SetShadeModel (GL_FLAT);
  }

  //=================
  // If we have need other texture passes (for whatever reason)
  // we set the z-buffer to second pass state.
  //=================
  if (m_config_options.do_extra_bright || queue.use_fog || multimat)
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
      statecache->SetTexture (GL_TEXTURE_2D, texturehandle);

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
      statecache->DisableState (GL_TEXTURE_2D);
      statecache->SetShadeModel (GL_SMOOTH);
      SetupBlend (CS_FX_MULTIPLY2, 0, false);

      SetClientStates (CS_CLIENTSTATE_VC);
      glColorPointer (4, GL_FLOAT, 0, queue.glcol);
      glVertexPointer (4, GL_FLOAT, 0, queue.glverts);
      glDrawElements (GL_TRIANGLES, queue.num_triangles*3, GL_UNSIGNED_INT,
      queue.tris);
    }
  }

  if (queue.use_fog)
  {
    statecache->EnableState (GL_TEXTURE_2D);
    statecache->SetTexture (GL_TEXTURE_2D, m_fogtexturehandle);
    statecache->SetShadeModel (GL_SMOOTH);
    SetupBlend (CS_FX_ALPHA, 0, false);

    SetClientStates (CS_CLIENTSTATE_ALL);
    glColorPointer (3, GL_FLOAT, 0, queue.fog_color);
    glVertexPointer (4, GL_FLOAT, 0, queue.glverts);
    glTexCoordPointer (2, GL_FLOAT, 0, queue.fog_txt);
    glDrawElements (GL_TRIANGLES, queue.num_triangles*3, GL_UNSIGNED_INT,
        queue.tris);
  }

  if (debug_edges)
    DebugDrawElements (G2D,
      queue.num_triangles*3, queue.tris, queue.glverts,
      G2D->FindRGB (255, 255, 255), false, false);

  queue.Reset ();

  if (txt_mm && txt_mm->GetKeyColor())
  {
    statecache->DisableState (GL_ALPHA_TEST);
  }
}

int csPolyQueue::AddVertices (int num)
{
  CS_ASSERT (num_vertices >= 0 && num_vertices <= max_vertices);
  num_vertices += num;
  if (num_vertices > max_vertices)
  {
    GLfloat* new_ar;
    int old_num = num_vertices-num;
    max_vertices = num_vertices + 40;

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

    new_ar = new GLfloat [max_vertices*3];
    if (fog_color) memcpy (new_ar, fog_color, sizeof (GLfloat)*3*old_num);
    delete[] fog_color; fog_color = new_ar;

    new_ar = new GLfloat [max_vertices*2];
    if (fog_txt) memcpy (new_ar, fog_txt, sizeof (GLfloat)*2*old_num);
    delete[] fog_txt; fog_txt = new_ar;
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

  // count 'real' number of vertices
  int num_vertices = 1;
  for (i = 1; i < poly.num; i++)
  {
    if ((ABS (poly.vertices[i].x - poly.vertices[i - 1].x)
   + ABS (poly.vertices[i].y - poly.vertices[i - 1].y))
    > VERTEX_NEAR_THRESHOLD)
      num_vertices++;
  }
  // if this is a 'degenerate' polygon, skip it
  if (num_vertices < 3)
    return;

  float flat_r = 1., flat_g = 1., flat_b = 1.;

  iTextureHandle *txt = poly.mat_handle?poly.mat_handle->GetTexture():NULL;
  float alpha = ((poly.mixmode & CS_FX_MASK_MIXMODE) == CS_FX_ALPHA)?
    (poly.mixmode & CS_FX_MASK_ALPHA)/255.0f:1.0f;

  bool flatlighting = ((txt && txt->GetAlphaMap() &&
    !(txt->GetKeyColor() && (alpha >= OPENGL_KEYCOLOR_MIN_ALPHA) ))
    || (alpha != 1.0f ) );

  FlushDrawPolygon ();

  // The following attempt to speed up only hits for about 10% in flarge,
  // so maybe its prolly not woth it. However, for highly triangulized cases
  // it may be worth to rethink. - norman
  //  if (prevPolyPlane.norm != poly.normal.norm || prevPolyPlane.DD != poly.normal.DD
  //      || prevTexMatrix != *poly.plane.m_cam2tex || prevTexVector != *poly.plane.v_cam2tex)
  {
    //    prevPolyPlane = poly.normal;
    //    prevTexMatrix = *poly.plane.m_cam2tex;
    //    prevTexVector = *poly.plane.v_cam2tex;

    // Get the plane normal of the polygon. Using this we can calculate
    // '1/z' at every screen space point.
    float Ac, Bc, Cc, Dc;
    Ac = poly.normal.A ();
    Bc = poly.normal.B ();
    Cc = poly.normal.C ();
    Dc = poly.normal.D ();

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
      float inv_Dc = 1 / Dc;
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
    float P1, P2, P3, P4;
    float Q1, Q2, Q3, Q4;
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
  }

  float sx, sy, sz, one_over_sz, u_over_sz, v_over_sz;

  iPolygonTexture *tex = poly.poly_texture;
  iLightMap* lm = tex->GetLightMap ();

  bool tex_transp = false;
  bool multimat = false;
  csMaterialHandle* mat_handle = (csMaterialHandle*)poly.mat_handle;
  iTextureHandle* txt_handle = NULL;
  csTextureHandleOpenGL *txt_mm = NULL;
  csTxtCacheData *texturecache_data = NULL;
  GLuint texturehandle = 0;

  if (mat_handle)
  {
    multimat = mat_handle->GetTextureLayerCount () > 0;
    txt_handle = mat_handle->GetTexture ();
    if (txt_handle)
    {
      txt_mm = (csTextureHandleOpenGL *)txt_handle->GetPrivateObject ();
      tex_transp = txt_mm->GetKeyColor () || txt_mm->GetAlphaMap ();
      // Initialize our static drawing information and cache
      // the texture in the texture cache (if this is not already the case).
      CacheTexture (poly.mat_handle);
      texturecache_data = (csTxtCacheData *)txt_mm->GetCacheData ();
      texturehandle = texturecache_data->Handle;
    }
  }

  alpha = BYTE_TO_FLOAT (poly.mixmode & CS_FX_MASK_ALPHA);
  alpha = SetupBlend (poly.mixmode, alpha, tex_transp);

  if (m_renderstate.textured && txt_handle)
  {
    statecache->EnableState (GL_TEXTURE_2D);
    if (txt_mm->GetKeyColor() && !(alpha < OPENGL_KEYCOLOR_MIN_ALPHA))
    {
      statecache->EnableState (GL_ALPHA_TEST);
      statecache->SetAlphaFunc (GL_GEQUAL, OPENGL_KEYCOLOR_MIN_ALPHA);
      SetupBlend (poly.mixmode, 1.0f, false);
    }
    else
    {
      statecache->DisableState (GL_ALPHA_TEST);
    }
  }
  else
  {
    statecache->DisableState (GL_TEXTURE_2D);
    csRGBpixel color;
    if (txt_handle)
    {
      txt_handle->GetMeanColor (color.red, color.green, color.blue);
      flat_r = BYTE_TO_FLOAT (color.red);
      flat_g = BYTE_TO_FLOAT (color.green);
      flat_b = BYTE_TO_FLOAT (color.blue);
    }
    else if (mat_handle)
    {
      mat_handle->GetFlatColor (color);
      flat_r = BYTE_TO_FLOAT (color.red);
      flat_g = BYTE_TO_FLOAT (color.green);
      flat_b = BYTE_TO_FLOAT (color.blue);
    }
  }

  SetGLZBufferFlags (z_buf_mode);

  if (txt_handle)
    statecache->SetTexture (GL_TEXTURE_2D, texturehandle);

  if (flatlighting)
  {
    int ir = 255, ig = 255, ib = 255;
    if (lm)
    {
      tex->RecalculateDynamicLights ();
      lm->GetMeanLighting(ir, ig, ib);
    }
    flat_r = ir/128.0f;
    flat_g = ig/128.0f;
    flat_b = ib/128.0f;
  }

  glColor4f (flat_r, flat_g, flat_b, alpha);
  statecache->SetShadeModel (GL_FLAT);

  //=================
  // Setup: calculate the polygon and texture information.
  //=================

  static GLfloat glverts[100*4];
  static GLfloat gltxt[100*2];
  static GLfloat gltxttrans[100*2];

  GLfloat* glv = glverts, * glt = gltxt;
  for (i = 0; i < poly.num; i++)
  {
    sx = poly.vertices[i].x - asp_center_x;
    sy = poly.vertices[i].y - asp_center_y;
    one_over_sz = M * sx + N * sy + O;
    sz = 1.0 / one_over_sz;
    u_over_sz = (J1 * sx + J2 * sy + J3);
    v_over_sz = (K1 * sx + K2 * sy + K3);
    // Modified to use homogenous object space coordinates instead
    // of homogenous texture space coordinates.
    *glt++ = u_over_sz * sz;
    *glt++ = v_over_sz * sz;
    *glv++ = poly.vertices[i].x * sz;
    *glv++ = poly.vertices[i].y * sz;
    *glv++ = -1.0;
    *glv++ = sz;
  }

  //=================
  // Pass 1: The unlit texture
  //=================

  SetClientStates (CS_CLIENTSTATE_VT);
  glVertexPointer (4, GL_FLOAT, 0, glverts);
  glTexCoordPointer (2, GL_FLOAT, 0, gltxt);
  glDrawArrays (GL_TRIANGLE_FAN, 0, poly.num);

  //=================
  // If we have need other texture passes (for whatever reason)
  // we set the z-buffer to second pass state.
  //=================
  bool do_lm = !flatlighting && !poly.do_fullbright
    && m_renderstate.lighting && lm;
  if (multimat || do_lm || poly.use_fog)
  {
    SetGLZBufferFlagsPass2 (z_buf_mode, true);
    statecache->EnableState (GL_TEXTURE_2D);
  }
  else
  {
    // Nothing more to do.
    return;
  }

  //=================
  // Pass 2: Here we add all extra texture layers if there are some.
  //=================
  if (multimat)
  {
    int j;
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
      statecache->SetTexture (GL_TEXTURE_2D, texturehandle);

      float alpha = 1.0f - BYTE_TO_FLOAT (layer->mode & CS_FX_MASK_ALPHA);
      alpha = SetupBlend (layer->mode, alpha, tex_transp);

      float uscale = layer->uscale;
      float vscale = layer->vscale;
      float ushift = layer->ushift;
      float vshift = layer->vshift;
      bool trans = mat_handle->TextureLayerTranslated (j);

      glColor4f (1., 1., 1., alpha);
      if (trans)
      {
        glt = gltxt;
	GLfloat* gltt = gltxttrans;
        for (i = 0; i < poly.num; i++)
        {
	  *gltt++ = (*glt++) * uscale + ushift;
	  *gltt++ = (*glt++) * vscale + vshift;
        }
        glTexCoordPointer (2, GL_FLOAT, 0, gltxttrans);
      }
      else
      {
        glTexCoordPointer (2, GL_FLOAT, 0, gltxt);
      }
      //glVertexPointer (4, GL_FLOAT, 0, glverts);  // No need to set.
      glDrawArrays (GL_TRIANGLE_FAN, 0, poly.num);
    }
  }

  //=================
  // Pass 3: Lightmaps
  //=================
  if (do_lm)
  {
    glColor4f (1, 1, 1, 0);
    SetupBlend (CS_FX_SRCDST, 0, false);

    GLuint TempHandle = lightmap_cache->GetTempHandle ();
    tex->RecalculateDynamicLights ();
    statecache->SetTexture (GL_TEXTURE_2D, TempHandle);
    int lmwidth = lm->GetWidth ();
    int lmheight = lm->GetHeight ();
    csRGBpixel* lm_data = lm->GetMapData ();
    glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0,
	lmwidth, lmheight, GL_RGBA, GL_UNSIGNED_BYTE, lm_data);

    float lm_offset_u, lm_offset_v, lm_high_u, lm_high_v;
    float lm_scale_u, lm_scale_v;
    tex->GetTextureBox (lm_offset_u, lm_offset_v, lm_high_u, lm_high_v);

    // lightmap fudge factor
    if (lm_high_u <= lm_offset_u)
      lm_scale_u = 1.;       // @@@ Is this right?
    else
      lm_scale_u = 1. / (lm_high_u - lm_offset_u);

    if (lm_high_v <= lm_offset_v)
      lm_scale_v = 1.;       // @@@ Is this right?
    else
      lm_scale_v = 1. / (lm_high_v - lm_offset_v);

    lm_offset_u -= .75 / (float (lmwidth) * lm_scale_u);
    lm_high_u += .75 / (float (lmwidth) * lm_scale_u);

    lm_offset_v -= .75 / (float (lmheight) * lm_scale_v);
    lm_high_v += .75 / (float (lmheight) * lm_scale_v);

    lm_scale_u = float (lmwidth) / (256. * (lm_high_u - lm_offset_u));
    lm_scale_v = float (lmheight) / (256. * (lm_high_v - lm_offset_v));

    glt = gltxt;
    GLfloat* gltt = gltxttrans;
    for (i = 0; i < poly.num; i++)
    {
      *gltt++ = (*(glt++) - lm_offset_u) * lm_scale_u;
      *gltt++ = (*(glt++) - lm_offset_v) * lm_scale_v;
    }
    //glVertexPointer (4, GL_FLOAT, 0, glverts);  // No need to set.
    glTexCoordPointer (2, GL_FLOAT, 0, gltxttrans);
    glDrawArrays (GL_TRIANGLE_FAN, 0, poly.num);
  }

  //=================
  // Pass 3: Fog
  //=================
  if (poly.use_fog)
  {
    statecache->SetTexture (GL_TEXTURE_2D, m_fogtexturehandle);
    statecache->SetShadeModel (GL_SMOOTH);
    SetupBlend (CS_FX_ALPHA, 0, false);

    GLfloat* gltt = gltxttrans;
    for (i = 0; i < poly.num; i++)
    {
      *gltt++ = poly.fog_info[i].intensity;
      *gltt++ = 0.0;
    }
    SetClientStates (CS_CLIENTSTATE_ALL);
    glColorPointer (3, GL_FLOAT, sizeof (G3DFogInfo), &poly.fog_info[0].r);
    //glVertexPointer (4, GL_FLOAT, 0, glverts);  // No need to set.
    glTexCoordPointer (2, GL_FLOAT, 0, gltxttrans);
    glDrawArrays (GL_TRIANGLE_FAN, 0, poly.num);
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
    if ((ABS (poly.vertices[i].x - poly.vertices[i - 1].x)
	+ ABS (poly.vertices[i].y - poly.vertices[i - 1].y))
	> VERTEX_NEAR_THRESHOLD)
      num_vertices++;
  }
  // if this is a 'degenerate' polygon, skip it
  if (num_vertices < 3)
    return;

  FlushDrawPolygon ();

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

  statecache->DisableState (GL_TEXTURE_2D);
  statecache->SetShadeModel (GL_FLAT);
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
    sx = poly.vertices[i].x - asp_center_x;
    sy = poly.vertices[i].y - asp_center_y;
    one_over_sz = M * sx + N * sy + O;
    sz = 1.0 / one_over_sz;
    glverts[vtidx++] = poly.vertices[i].x * sz;
    glverts[vtidx++] = poly.vertices[i].y * sz;
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

  bool gouraud = (poly.mixmode & CS_FX_GOURAUD) != 0;
  float alpha = 1.0f - BYTE_TO_FLOAT (poly.mixmode & CS_FX_MASK_ALPHA);
  bool txt_alpha = false;
  if (poly.mat_handle)
  {
    iTextureHandle* txt_handle = poly.mat_handle->GetTexture ();
    if (txt_handle)
      txt_alpha = txt_handle->GetKeyColor () || txt_handle->GetAlphaMap ();
  }
  alpha = GetAlpha (poly.mixmode, alpha, txt_alpha);

  //========
  // First check if this polygon is different from the current polygons
  // in the queue. If so we need to flush the queue.
  //========
  if (poly.mat_handle != queue.mat_handle ||
      poly.mixmode != queue.mixmode ||
      z_buf_mode != queue.z_buf_mode ||
      flat_r != queue.flat_color_r ||
      flat_g != queue.flat_color_g ||
      flat_b != queue.flat_color_b ||
      poly.use_fog != queue.use_fog)
  {
    FlushDrawPolygon ();
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
  queue.use_fog = poly.use_fog;

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

    *glverts++ = poly.vertices[i].x * sz;
    *glverts++ = poly.vertices[i].y * sz;
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
    // Copy vertex info.
    GLfloat* fog_color = queue.GetFogColor (idx);
    GLfloat* fog_txt = queue.GetFogTxt (idx);
    for (i = 0 ; i < poly.num ; i++)
    {
      *fog_color++ = poly.fog_info[i].r;
      *fog_color++ = poly.fog_info[i].g;
      *fog_color++ = poly.fog_info[i].b;

      *fog_txt++ = poly.fog_info[i].intensity;
      *fog_txt++ = 0.0;
    }
  }
}

// Find out the location of a vertex by recursing through the
// clipinfo tree.
static void ResolveVertex (
  csClipInfo* ci,
  int* clipped_translate,
  csVector3* overts, csVector2* otexels,
  csColor* ocolors, float** ouserarrays, int* userarraycomponents,
  G3DFogInfo* ofog,
  int outi,
  csVector2* texel, csColor* color, float** userarrays,
  G3DFogInfo* fog)
{
  static float user1[CS_VBUF_TOTAL_USERA*4], user2[CS_VBUF_TOTAL_USERA*4];
  static float* userpointers1[CS_VBUF_TOTAL_USERA];
  static float* userpointers2[CS_VBUF_TOTAL_USERA];
  static bool init = false;
  if (!init)
  {
    for (int u=0; u<CS_VBUF_TOTAL_USERA; u++)
    {
      userpointers1[u] = &user1[u*4];
      userpointers2[u] = &user2[u*4];
    }
    init = true;
  }

  switch (ci->type)
  {
    case CS_CLIPINFO_ORIGINAL:
      texel[outi] = otexels[ci->original.idx];
      if (ocolors) color[outi] = ocolors[ci->original.idx];
      if (ofog) fog[outi] = ofog[ci->original.idx];
      if (ouserarrays)
      {
        for (int u=0; u<CS_VBUF_TOTAL_USERA; u++ )
        {
	  if( ouserarrays[u] )
	  {
	    for( int c=0; c<userarraycomponents[u]; c++ )
	    {
	      (userarrays[u])[outi*userarraycomponents[u]+c] =
		(ouserarrays[u])[ci->original.idx*userarraycomponents[u]+c];
	    }
	  }
        }
      }
      break;
    case CS_CLIPINFO_ONEDGE:
    {
      int i1 = ci->onedge.i1;
      int i2 = ci->onedge.i2;
      float r = ci->onedge.r;
      texel[outi] = otexels[i1] * (1-r) + otexels[i2] * r;
      if (ocolors)
      {
	color[outi].red = ocolors[i1].red * (1-r) + ocolors[i2].red * r;
	color[outi].green = ocolors[i1].green * (1-r) + ocolors[i2].green * r;
	color[outi].blue = ocolors[i1].blue * (1-r) + ocolors[i2].blue * r;
      }
      if (ofog)
      {
	fog[outi].intensity = ofog[i1].intensity*(1-r)+ofog[i2].intensity*r;
	fog[outi].intensity2 = 0;
	fog[outi].r = ofog[i1].r * (1-r) + ofog[i2].r * r;
	fog[outi].g = ofog[i1].g * (1-r) + ofog[i2].g * r;
	fog[outi].b = ofog[i1].b * (1-r) + ofog[i2].b * r;
      }
      if (ouserarrays)
      {
        for (int u=0; u<CS_VBUF_TOTAL_USERA; u++ )
        {
	  if( ouserarrays[u] )
	  {
	    for( int c=0; c<userarraycomponents[u]; c++ )
	    {
	      (userarrays[u])[outi*userarraycomponents[u]+c] =
		(ouserarrays[u])[i1*userarraycomponents[u]+c] * (1-r) +
		(ouserarrays[u])[i2*userarraycomponents[u]+c] * r;
	    }
	  }
        }
      }
      break;
    }
    case CS_CLIPINFO_INSIDE:
    {
      csVector2 texel1, texel2;
      csColor color1, color2;
      G3DFogInfo fog1, fog2;
      ResolveVertex (ci->inside.ci1, clipped_translate, overts, otexels,
	ocolors, ouserarrays, userarraycomponents,
	ofog, 0, &texel1, &color1, userpointers1, &fog1);
      ResolveVertex (ci->inside.ci2, clipped_translate, overts, otexels,
	ocolors, ouserarrays, userarraycomponents,
	ofog, 0, &texel2, &color2, userpointers2, &fog2);
      delete ci->inside.ci1;
      delete ci->inside.ci2;
      ci->type = CS_CLIPINFO_ORIGINAL;
      float r = ci->inside.r;
      texel[outi] = texel1 * (1-r) + texel2 * r;
      if (ocolors)
      {
	color[outi].red = color1.red * (1-r) + color2.red * r;
	color[outi].green = color1.green * (1-r) + color2.green * r;
	color[outi].blue = color1.blue * (1-r) + color2.blue * r;
      }
      if (ofog)
      {
	fog[outi].intensity =  fog1.intensity*(1-r)+fog2.intensity*r;
	fog[outi].intensity2 = 0;
	fog[outi].r = fog1.r * (1-r) + fog2.r * r;
	fog[outi].g = fog1.g * (1-r) + fog2.g * r;
	fog[outi].b = fog1.b * (1-r) + fog2.b * r;
      }
      if (ouserarrays)
      {
        for (int u=0; u<CS_VBUF_TOTAL_USERA; u++ )
        {
	  if( ouserarrays[u] )
	  {
	    for( int c=0; c<userarraycomponents[u]; c++ )
	    {
	      (userarrays[u])[outi*userarraycomponents[u]+c] =
		user1[u*4+c] * (1-r) + user2[u*4+c] * r;
	    }
	  }
        }
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
    float** userarrays,
    int* userarraycomponents,
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

  csPlane3 frustum_planes[100]; // @@@ Arbitrary limit
  csPlane3 diagonal_planes[50]; // @@@ Arbitrary number.
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
    texels, vertex_colors, userarrays, userarraycomponents, vertex_fog,
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
    float** userarrays,
    int* userarraycomponents,
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
  if (num_tri > clipped_triangles->Limit ())
  {
    // Use two times as many triangles. Hopefully this is enough.
    clipped_triangles->SetLimit (num_tri);
  }
  if (num_vertices > clipped_translate->Limit ())
    clipped_translate->SetLimit (num_vertices); // Used for original vertices.
  int num_vts = num_vertices*2+100;
  if (num_vts > clipped_vertices->Limit ())
  {
    clipped_vertices->SetLimit (num_vts);
    clipped_texels->SetLimit (num_vts);
    clipped_colors->SetLimit (num_vts);
    clipped_fog->SetLimit (num_vts);
    for( i=0; i<CS_VBUF_TOTAL_USERA; i++ )
      clipped_user[i]->SetLimit (num_vts);
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
	break;  // Not inside.
      }
    }
    if (inside)
    {
      if (exact_clipping)
      {
        (*clipped_translate)[i] = num_clipped_vertices;
        (*clipped_vertices)[num_clipped_vertices] = v;
        (*clipped_texels)[num_clipped_vertices] = texels[i];
        if (vertex_colors)
          (*clipped_colors)[num_clipped_vertices] = vertex_colors[i];
        if (vertex_fog)
          (*clipped_fog)[num_clipped_vertices] = vertex_fog[i];
        if (userarrays)
        {
          for (int u=0; u<CS_VBUF_TOTAL_USERA; u++)
            if (userarrays[u] != NULL)
            {
              for (int c=0; c<userarraycomponents[u]; c++)
                (*clipped_user[u])[num_clipped_vertices] =
                  (userarrays[u])[i*userarraycomponents[u]+c];
            }
        }
        num_clipped_vertices++;
      }
      else
        (*clipped_translate)[i] = i;
    }
    else
      (*clipped_translate)[i] = -1;
  }

  // If we have lazy clipping then the number of vertices remains the same.
  if (!exact_clipping)
    num_clipped_vertices = num_vertices;

  // Now clip all triangles.
  for (i = 0 ; i < num_triangles ; i++)
  {
    csTriangle& tri = triangles[i];
    int cnt = int ((*clipped_translate)[tri.a] != -1)
        + int ((*clipped_translate)[tri.b] != -1)
	+ int ((*clipped_translate)[tri.c] != -1);
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
      (*clipped_triangles)[num_clipped_triangles].a = (*clipped_translate)[tri.a];
      (*clipped_triangles)[num_clipped_triangles].b = (*clipped_translate)[tri.b];
      (*clipped_triangles)[num_clipped_triangles].c = (*clipped_translate)[tri.c];
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
        (*clipped_triangles)[num_clipped_triangles].a = tri.a;
        (*clipped_triangles)[num_clipped_triangles].b = tri.b;
        (*clipped_triangles)[num_clipped_triangles].c = tri.c;
        num_clipped_triangles++;
	continue;
      }

      csVector3 poly[100];  // @@@ Arbitrary limit
      static csClipInfo *clipinfo = GetStaticClipInfo1 ();
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
		(*clipped_translate)[clipinfo[j].original.idx];
	}
        else
	{
	  float* clipped_userpointers[CS_VBUF_TOTAL_USERA];
	  for (int u=0; u<CS_VBUF_TOTAL_USERA; u++)
	    clipped_userpointers[u] = clipped_user[u]->GetArray ();
	  ResolveVertex (&clipinfo[j], clipped_translate->GetArray (),
	    vertices, texels, vertex_colors,
	    userarrays, userarraycomponents, vertex_fog,
	    num_clipped_vertices,
	    clipped_texels->GetArray (),
	    clipped_colors->GetArray (),
	    clipped_userpointers,
	    clipped_fog->GetArray ());
	  (*clipped_vertices)[num_clipped_vertices] = poly[j]+frust_origin;
	  clipinfo[j].original.idx = num_clipped_vertices;
	  num_clipped_vertices++;
	}
      }

      //-----
      // Triangulate the resulting polygon.
      //-----
      for (j = 2 ; j < num_poly ; j++)
      {
        (*clipped_triangles)[num_clipped_triangles].a = clipinfo[0].original.idx;
        (*clipped_triangles)[num_clipped_triangles].b = clipinfo[j-1].original.idx;
        (*clipped_triangles)[num_clipped_triangles].c = clipinfo[j].original.idx;
        num_clipped_triangles++;
      }
    }
  }
}


////////////////////////////////////////////////////////////////////


void csGraphics3DOGLCommon::DrawPolygonMesh (G3DPolygonMesh& mesh)
{
  csRef<iVertexBufferManager> vbman = GetVertexBufferManager();
  csRef<iVertexBuffer> vb = vbman->CreateBuffer (0);

  csTriangleArrayPolygonBuffer* polbuf =
    (csTriangleArrayPolygonBuffer*)mesh.polybuf;

  SetGLZBufferFlags (z_buf_mode);

  G3DTriangleMesh trimesh;
  trimesh.buffers[0] = vb;
  trimesh.morph_factor = 0;
  trimesh.num_vertices_pool = 1;
  trimesh.use_vertex_color = false;
  trimesh.clip_plane = mesh.clip_plane;
  trimesh.clip_portal = mesh.clip_portal;
  trimesh.clip_z_plane = mesh.clip_z_plane;
  trimesh.do_fog = false;
  trimesh.vertex_fog = NULL;
  trimesh.do_mirror = mesh.do_mirror;
  trimesh.mixmode = mesh.mixmode;
  if (mesh.vertex_mode == G3DPolygonMesh::VM_VIEWSPACE)
    trimesh.vertex_mode = G3DTriangleMesh::VM_VIEWSPACE;
  else
    trimesh.vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;

  // Loop over all sub-meshes. Every sub-mesh represents a different material.
  TrianglesNode *t = polbuf->GetFirst ();
  while (t != NULL)
  {
    csTrianglesPerMaterial* tpm = t->info;

    // Clear the vertex arrays in the polygon buffer since they are only
    // needed while building the polygon buffer. Not later.
    tpm->ClearVertexArray ();

    trimesh.mat_handle = polbuf->GetMaterialPolygon (t);
    vbman->LockBuffer (vb,
      tpm->verticesPoints.GetArray (),
      tpm->texels.GetArray (),
      NULL,
      tpm->numVertices, 0);
    trimesh.triangles = tpm->triangles.GetArray ();
    trimesh.num_triangles = tpm->numTriangles;
    DrawTriangleMesh (trimesh);
    vbman->UnlockBuffer (vb);
    t = t->next;
  }

  switch (z_buf_mode)
  {
    case CS_ZBUF_FILL:
    case CS_ZBUF_FILLONLY:
    case CS_ZBUF_USE:
      z_buf_mode = CS_ZBUF_EQUAL;
      break;
    default:
      break;
  }

  trimesh.use_vertex_color = false;
  trimesh.mat_handle = NULL;
  TrianglesSuperLightmapNode *sln = polbuf->GetFirstTrianglesSLM ();
  bool dirty = polbuf->superLM.GetLightmapsDirtyState ();
  bool modified = false;
  while (sln != NULL)
  {
    csTrianglesPerSuperLightmap* tplm = sln->info;

    lightmap_cache->Cache (tplm, dirty, &modified);
    if (!tplm->cacheData->IsUnlit ())
    {
      vbman->LockBuffer (vb,
	tplm->vec_vertices.GetArray (),
	tplm->texels.GetArray (),
        NULL,
	tplm->numVertices, 0);

      trimesh.triangles = tplm->triangles.GetArray ();
      trimesh.num_triangles = tplm->numTriangles;

      EffectDrawTriangleMesh (trimesh, tplm->cacheData->Handle);
      vbman->UnlockBuffer (vb);
    }
    sln = sln->prev;
  }
  polbuf->superLM.ClearLightmapsDirty ();
}

csStringID csGraphics3DOGLCommon::GLBlendToString (GLenum blend)
{
  switch( blend )
  {
    case GL_ONE:
      return effectserver->GetStandardStrings()->one;
    case GL_ZERO:
      return effectserver->GetStandardStrings()->zero;
    case GL_SRC_COLOR:
      return effectserver->GetStandardStrings()->source_color;
    case GL_ONE_MINUS_SRC_COLOR:
      return effectserver->GetStandardStrings()->inverted_source_color;
    case GL_DST_COLOR:
      return effectserver->GetStandardStrings()->destination_color;
    case GL_ONE_MINUS_DST_COLOR:
      return effectserver->GetStandardStrings()->inverted_destination_color;
    case GL_SRC_ALPHA:
      return effectserver->GetStandardStrings()->source_alpha;
    case GL_ONE_MINUS_SRC_ALPHA:
      return effectserver->GetStandardStrings()->inverted_source_alpha;
    case GL_DST_ALPHA:
      return effectserver->GetStandardStrings()->destination_alpha;
    case GL_ONE_MINUS_DST_ALPHA:
      return effectserver->GetStandardStrings()->inverted_destination_alpha;
    case GL_SRC_ALPHA_SATURATE:
      return effectserver->GetStandardStrings()->saturated_source_alpha;
    default:
      return csInvalidStringID;
  }
}


#define START_FX_SRCDST \
  pass = tech->CreatePass(); \
  pass->SetStateString( efstrings->blending, efstrings->enabled ); \
  pass->SetStateString( efstrings->source_blend_mode, GLBlendToString( m_config_options.m_lightmap_src_blend ) ); \
  pass->SetStateString( efstrings->destination_blend_mode, GLBlendToString( m_config_options.m_lightmap_dst_blend ) ); \
  layer = pass->CreateLayer(); \
  layer->SetStateFloat( efstrings->texture_source, 1 ); \
  layer->SetStateString( efstrings->texture_coordinate_source, efstrings->mesh );

#define START_FX_HALOOVF \
  pass = tech->CreatePass(); \
  pass->SetStateString( efstrings->blending, efstrings->enabled ); \
  pass->SetStateString( efstrings->source_blend_mode, efstrings->source_alpha ); \
  pass->SetStateString( efstrings->destination_blend_mode, efstrings->one ); \
  layer = pass->CreateLayer(); \
  layer->SetStateFloat( efstrings->texture_source, 1 ); \
  layer->SetStateString( efstrings->texture_coordinate_source, efstrings->mesh );

#define START_FX_MULTIPLY \
  pass = tech->CreatePass(); \
  pass->SetStateString( efstrings->blending, efstrings->enabled ); \
  pass->SetStateString( efstrings->source_blend_mode, efstrings->zero ); \
  pass->SetStateString( efstrings->destination_blend_mode, efstrings->source_color ); \
  layer = pass->CreateLayer(); \
  layer->SetStateFloat( efstrings->texture_source, 1 ); \
  layer->SetStateString( efstrings->texture_coordinate_source, efstrings->mesh );

#define START_FX_MULTIPLY2 \
  pass = tech->CreatePass(); \
  pass->SetStateString( efstrings->blending, efstrings->enabled ); \
  pass->SetStateString( efstrings->source_blend_mode, efstrings->destination_color ); \
  pass->SetStateString( efstrings->destination_blend_mode, efstrings->source_color ); \
  layer = pass->CreateLayer(); \
  layer->SetStateFloat( efstrings->texture_source, 1 ); \
  layer->SetStateString( efstrings->texture_coordinate_source, efstrings->mesh );

#define START_FX_ADD \
  pass = tech->CreatePass(); \
  pass->SetStateString( efstrings->blending, efstrings->enabled ); \
  pass->SetStateString( efstrings->source_blend_mode, efstrings->one ); \
  pass->SetStateString( efstrings->destination_blend_mode, efstrings->one ); \
  layer = pass->CreateLayer(); \
  layer->SetStateFloat( efstrings->texture_source, 1 ); \
  layer->SetStateString( efstrings->texture_coordinate_source, efstrings->mesh );

#define START_FX_ALPHA \
  pass = tech->CreatePass(); \
  pass->SetStateString( efstrings->blending, efstrings->enabled ); \
  pass->SetStateString( efstrings->source_blend_mode, efstrings->source_alpha ); \
  pass->SetStateString( efstrings->destination_blend_mode, efstrings->inverted_source_alpha ); \
  /*pass->SetStateFloat( efstrings->scale_alpha, 0.5f);*/ \
  layer = pass->CreateLayer(); \
  layer->SetStateFloat( efstrings->texture_source, 1 ); \
  layer->SetStateString( efstrings->texture_coordinate_source, efstrings->mesh );


// @@@ Should use writemask instead of blendmode
//     since blendmode cuts fillrate in half
//     --Anders Stenberg
#define START_FX_TRANSPARENT \
  pass = tech->CreatePass(); \
  pass->SetStateString( efstrings->blending, efstrings->enabled ); \
  pass->SetStateString( efstrings->source_blend_mode, efstrings->zero ); \
  pass->SetStateString( efstrings->destination_blend_mode, efstrings->one ); \
  layer = pass->CreateLayer(); \
  layer->SetStateFloat( efstrings->texture_source, 1 ); \
  layer->SetStateString( efstrings->texture_coordinate_source, efstrings->mesh );

#define START_FX_COPY_ALPHA \
  pass = tech->CreatePass(); \
  pass->SetStateString( efstrings->blending, efstrings->enabled ); \
  pass->SetStateString( efstrings->source_blend_mode, efstrings->source_alpha ); \
  pass->SetStateString( efstrings->destination_blend_mode, efstrings->inverted_source_alpha ); \
  layer = pass->CreateLayer(); \
  layer->SetStateFloat( efstrings->texture_source, 1 ); \
  layer->SetStateString( efstrings->texture_coordinate_source, efstrings->mesh );

#define START_FX_COPY_NOALPHA \
  pass = tech->CreatePass(); \
  pass->SetStateString( efstrings->blending, efstrings->disabled ); \
  layer = pass->CreateLayer(); \
  layer->SetStateFloat( efstrings->texture_source, 1 ); \
  layer->SetStateString( efstrings->texture_coordinate_source, efstrings->mesh );

#define SINGLETEXTURE_FOG \
  pass = tech->CreatePass(); \
  pass->SetStateString( efstrings->blending, efstrings->enabled ); \
  pass->SetStateString( efstrings->source_blend_mode, efstrings->source_alpha ); \
  pass->SetStateString( efstrings->destination_blend_mode, efstrings->inverted_source_alpha ); \
  pass->SetStateString( efstrings->vertex_color_source, efstrings->fog ); \
  layer = pass->CreateLayer(); \
  layer->SetStateString( efstrings->texture_source, efstrings->fog ); \
  layer->SetStateString( efstrings->texture_coordinate_source, efstrings->fog );

#define MULTITEXTURE_FOG \
  layer = pass->CreateLayer(); \
  layer->SetStateString( efstrings->texture_source, efstrings->fog ); \
  layer->SetStateString( efstrings->texture_coordinate_source, efstrings->fog ); \
  layer->SetStateString( efstrings->constant_color_source, efstrings->fog ); \
  layer->SetStateString( efstrings->color_source_1, efstrings->constant_color ); \
  layer->SetStateString( efstrings->color_source_2, efstrings->previous_layer_color ); \
  layer->SetStateString( efstrings->color_source_3, efstrings->texture_color); \
  layer->SetStateString( efstrings->color_source_modifier_3, efstrings->source_alpha); \
  layer->SetStateString( efstrings->color_operation, efstrings->interpolate ); \
  layer->SetStateString( efstrings->alpha_source_1, efstrings->previous_layer_alpha ); \
  layer->SetStateString( efstrings->alpha_operation, efstrings->use_source_1 );

void csGraphics3DOGLCommon::InitStockEffects()
{
  iEffectTechnique* tech;
  iEffectPass* pass;
  iEffectLayer* layer;
  csEffectStrings* efstrings = effectserver->GetStandardStrings();

  SeparateLightmapStockEffect = effectserver->CreateEffect ();
  tech = SeparateLightmapStockEffect->CreateTechnique();
  pass = tech->CreatePass();
  pass->SetStateString( efstrings->blending, efstrings->enabled );
  pass->SetStateString( efstrings->source_blend_mode, GLBlendToString( m_config_options.m_lightmap_src_blend ) );
  pass->SetStateString( efstrings->destination_blend_mode, GLBlendToString( m_config_options.m_lightmap_dst_blend ) );
  layer = pass->CreateLayer();
  layer->SetStateString( efstrings->texture_source, efstrings->lightmap );
  layer->SetStateString( efstrings->texture_coordinate_source, efstrings->lightmap );
  effectserver->Validate (SeparateLightmapStockEffect);

  SeparateFogStockEffect = effectserver->CreateEffect ();
  tech = SeparateFogStockEffect->CreateTechnique();
  pass = tech->CreatePass();
  pass->SetStateString( efstrings->blending, efstrings->enabled );
  pass->SetStateString( efstrings->source_blend_mode, efstrings->source_alpha  );
  pass->SetStateString( efstrings->destination_blend_mode, efstrings->inverted_source_alpha  );
  layer = pass->CreateLayer();
  layer->SetStateString( efstrings->texture_source, efstrings->fog );
  layer->SetStateString( efstrings->texture_coordinate_source, efstrings->fog );
  effectserver->Validate (SeparateFogStockEffect);

  ////////////////////////////////////////////
  // NO LIGHTMAPS, NO FOG
  ////////////////////////////////////////////

  StockEffects[0][0][0] = effectserver->CreateEffect();
  tech = StockEffects[0][0][0]->CreateTechnique();
  START_FX_SRCDST
  effectserver->Validate( StockEffects[0][0][0] );

  StockEffects[0][0][1] = effectserver->CreateEffect();
  tech = StockEffects[0][0][1]->CreateTechnique();
  START_FX_HALOOVF
  effectserver->Validate( StockEffects[0][0][1] );

  StockEffects[0][0][2] = effectserver->CreateEffect();
  tech = StockEffects[0][0][2]->CreateTechnique();
  START_FX_MULTIPLY
  effectserver->Validate( StockEffects[0][0][2] );

  StockEffects[0][0][3] = effectserver->CreateEffect();
  tech = StockEffects[0][0][3]->CreateTechnique();
  START_FX_MULTIPLY2
  effectserver->Validate( StockEffects[0][0][3] );

  StockEffects[0][0][4] = effectserver->CreateEffect();
  tech = StockEffects[0][0][4]->CreateTechnique();
  START_FX_ADD
  effectserver->Validate( StockEffects[0][0][4] );

  StockEffects[0][0][5] = effectserver->CreateEffect();
  tech = StockEffects[0][0][5]->CreateTechnique();
  START_FX_ALPHA
  effectserver->Validate( StockEffects[0][0][5] );

  StockEffects[0][0][6] = effectserver->CreateEffect();
  tech = StockEffects[0][0][6]->CreateTechnique();
  START_FX_TRANSPARENT
  effectserver->Validate( StockEffects[0][0][6] );

  StockEffects[0][0][7] = effectserver->CreateEffect();
  tech = StockEffects[0][0][7]->CreateTechnique();
  START_FX_COPY_ALPHA
  effectserver->Validate( StockEffects[0][0][7] );

  StockEffects[0][0][8] = effectserver->CreateEffect();
  tech = StockEffects[0][0][8]->CreateTechnique();
  START_FX_COPY_NOALPHA
  effectserver->Validate( StockEffects[0][0][8] );


  ////////////////////////////////////////////
  // NO LIGHTMAPS, FOG
  ////////////////////////////////////////////

  StockEffects[0][1][0] = effectserver->CreateEffect();
  tech = StockEffects[0][1][0]->CreateTechnique();
  START_FX_SRCDST
  MULTITEXTURE_FOG
  tech->SetQuality( 1 );
  tech = StockEffects[0][1][0]->CreateTechnique();
  START_FX_SRCDST
  SINGLETEXTURE_FOG
  tech->SetQuality( 0 );
  effectserver->Validate( StockEffects[0][1][0] );

  StockEffects[0][1][1] = effectserver->CreateEffect();
  tech = StockEffects[0][1][1]->CreateTechnique();
  START_FX_HALOOVF
  MULTITEXTURE_FOG
  tech->SetQuality( 1 );
  tech = StockEffects[0][1][1]->CreateTechnique();
  START_FX_HALOOVF
  SINGLETEXTURE_FOG
  tech->SetQuality( 0 );
  effectserver->Validate( StockEffects[0][1][1] );

  StockEffects[0][1][2] = effectserver->CreateEffect();
  tech = StockEffects[0][1][2]->CreateTechnique();
  START_FX_MULTIPLY
  MULTITEXTURE_FOG
  tech->SetQuality( 1 );
  tech = StockEffects[0][1][2]->CreateTechnique();
  START_FX_MULTIPLY
  SINGLETEXTURE_FOG
  tech->SetQuality( 0 );
  effectserver->Validate( StockEffects[0][1][2] );

  StockEffects[0][1][3] = effectserver->CreateEffect();
  tech = StockEffects[0][1][3]->CreateTechnique();
  START_FX_MULTIPLY2
  MULTITEXTURE_FOG
  tech->SetQuality( 1 );
  tech = StockEffects[0][1][3]->CreateTechnique();
  START_FX_MULTIPLY2
  SINGLETEXTURE_FOG
  tech->SetQuality( 0 );
  effectserver->Validate( StockEffects[0][1][3] );

  StockEffects[0][1][4] = effectserver->CreateEffect();
  tech = StockEffects[0][1][4]->CreateTechnique();
  START_FX_ADD
  MULTITEXTURE_FOG
  tech->SetQuality( 1 );
  tech = StockEffects[0][1][4]->CreateTechnique();
  START_FX_ADD
  SINGLETEXTURE_FOG
  tech->SetQuality( 0 );
  effectserver->Validate( StockEffects[0][1][4] );

  StockEffects[0][1][5] = effectserver->CreateEffect();
  tech = StockEffects[0][1][5]->CreateTechnique();
  START_FX_ALPHA
  MULTITEXTURE_FOG
  tech->SetQuality( 1 );
  tech = StockEffects[0][1][5]->CreateTechnique();
  START_FX_ALPHA
  SINGLETEXTURE_FOG
  tech->SetQuality( 0 );
  effectserver->Validate( StockEffects[0][1][5] );

  StockEffects[0][1][6] = effectserver->CreateEffect();
  tech = StockEffects[0][1][6]->CreateTechnique();
  START_FX_TRANSPARENT
  MULTITEXTURE_FOG
  tech->SetQuality( 1 );
  tech = StockEffects[0][1][6]->CreateTechnique();
  START_FX_TRANSPARENT
  SINGLETEXTURE_FOG
  tech->SetQuality( 0 );
  effectserver->Validate( StockEffects[0][1][6] );

  StockEffects[0][1][7] = effectserver->CreateEffect();
  tech = StockEffects[0][1][7]->CreateTechnique();
  START_FX_COPY_ALPHA
  MULTITEXTURE_FOG
  tech->SetQuality( 1 );
  tech = StockEffects[0][1][7]->CreateTechnique();
  START_FX_COPY_ALPHA
  SINGLETEXTURE_FOG
  tech->SetQuality( 0 );
  effectserver->Validate( StockEffects[0][1][7] );

  StockEffects[0][1][8] = effectserver->CreateEffect();
  tech = StockEffects[0][1][8]->CreateTechnique();
  START_FX_COPY_NOALPHA
  MULTITEXTURE_FOG
  tech->SetQuality( 1 );
  tech = StockEffects[0][1][8]->CreateTechnique();
  START_FX_COPY_NOALPHA
  SINGLETEXTURE_FOG
  tech->SetQuality( 0 );
  effectserver->Validate( StockEffects[0][1][8] );
}

iEffectTechnique* csGraphics3DOGLCommon::GetStockTechnique (
  G3DTriangleMesh& mesh)
{
  if( (((csMaterialHandle*)mesh.mat_handle)->GetTextureLayerCount() > 0) ||
      !mesh.mat_handle->GetTexture() )
    return NULL;

  switch (mesh.mixmode & (CS_FX_MASK_MIXMODE | CS_FX_EXTRA_MODES))
  {
    case CS_FX_SRCDST:
      return effectserver->SelectAppropriateTechnique(
        StockEffects[0][mesh.do_fog?1:0][0] );
    case CS_FX_HALOOVF:
      return effectserver->SelectAppropriateTechnique(
        StockEffects[0][mesh.do_fog?1:0][1] );
    case CS_FX_MULTIPLY:
      return effectserver->SelectAppropriateTechnique(
        StockEffects[0][mesh.do_fog?1:0][2] );
    case CS_FX_MULTIPLY2:
      return effectserver->SelectAppropriateTechnique(
        StockEffects[0][mesh.do_fog?1:0][3] );
    case CS_FX_ADD:
      return effectserver->SelectAppropriateTechnique(
        StockEffects[0][mesh.do_fog?1:0][4] );
    case CS_FX_ALPHA:
      return NULL;/*effectserver->SelectAppropriateTechnique(
         StockEffects[0][mesh.do_fog?1:0][5] );*/
    case CS_FX_TRANSPARENT:
      return effectserver->SelectAppropriateTechnique(
        StockEffects[0][mesh.do_fog?1:0][6] );
    case CS_FX_COPY:
    default:
      if (mesh.mat_handle->GetTexture()->GetKeyColor()
          || mesh.mat_handle->GetTexture()->GetAlphaMap ())
        return effectserver->SelectAppropriateTechnique(
		StockEffects[0][mesh.do_fog?1:0][7] );
      else
        return effectserver->SelectAppropriateTechnique(
		StockEffects[0][mesh.do_fog?1:0][8] );
  }
}

void csGraphics3DOGLCommon::DrawTriangleMesh (G3DTriangleMesh& mesh)
{
#if 0
  // The new effects support is atm not perfect. it supports not everything
  // correctly yet (like, transparency in mixmode) and thus
  // is faster, but looks worse than OldDrawTriangleMesh().
  OldDrawTriangleMesh (mesh);
  return;
#else
  EffectDrawTriangleMesh (mesh);
#endif
}

void csGraphics3DOGLCommon::EffectDrawTriangleMesh (
  G3DTriangleMesh& mesh, GLuint lightmap, csVector2* lightmapcoords)
{
  int i, l;
  if (!lightmap)
  {
    if (!mesh.mat_handle)
    {
      // If there is no material (which is legal) we temporarily use
      // OldDrawTriangleMesh().
      OldDrawTriangleMesh (mesh);
      return;
    }
  }

  iMaterial* material = NULL;
  iEffectDefinition* effect = NULL;
  iEffectTechnique* technique = NULL;
  if (mesh.mat_handle)
  {
    material = ((csMaterialHandle*)(mesh.mat_handle))->GetMaterial();
    effect = material->GetEffect();
    technique = effectserver->SelectAppropriateTechnique (effect);
  }
  else
  {
    effect = SeparateLightmapStockEffect;
    technique = effectserver->SelectAppropriateTechnique (effect);
  }

  if (!technique)
  {
    technique = GetStockTechnique (mesh);
    if (!technique)
    {
      OldDrawTriangleMesh (mesh); // Should never get here.
      return;
    }
  }

  FlushDrawPolygon ();

  int num_vertices = mesh.buffers[0]->GetVertexCount ();
  int num_triangles = mesh.num_triangles;

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
    bool no_zbuf_clipping = (technique->GetClientFlags()
      & EFFECTFLAG_RUINSZCLIPPING)
      || (z_buf_mode == CS_ZBUF_NONE || z_buf_mode == CS_ZBUF_FILL ||
      z_buf_mode == CS_ZBUF_FILLONLY);

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
    for (i = 0 ; i < 3 ; i++)
    {
      char c = clip_modes[i];
      // We cannot use n,N,z, or Z if no_zbuf_clipping is true.
      if ((c == 'n' || c == 'N' || c == 'z' || c == 'Z') && no_zbuf_clipping)
        continue;
      // We cannot use s or S if effect uses stencil.
      if ((c == 's' || c == 'S')
          && (technique->GetClientFlags() & EFFECTFLAG_RUINSSCLIPPING))
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

  //===========
  // Update work tables.
  //===========
  if (num_vertices > tr_verts->Limit ())
  {
    tr_verts->SetLimit (num_vertices);
    uv_verts->SetLimit (num_vertices);
    color_verts->SetLimit (num_vertices);
  }

  //===========
  // Do vertex tweening and/or transformation to camera space
  // if any of those are needed. When this is done 'verts' will
  // point to an array of camera vertices.
  //===========
  csVector3* f1 = mesh.buffers[0]->GetVertices ();
  csVector2* uv1 = mesh.buffers[0]->GetTexels ();
  csColor* col1 = mesh.buffers[0]->GetColors ();
  if (!col1) mesh.do_morph_colors = false;
  csVector3* work_verts;
  csVector2* work_uv_verts;
  csColor* work_colors;
  float* work_userarrays[CS_VBUF_TOTAL_USERA];
  int userarraycomponents[CS_VBUF_TOTAL_USERA];

  for (i=0; i<CS_VBUF_TOTAL_USERA; i++)
  {
    work_userarrays[i] = mesh.buffers[0]->GetUserArray (i);
    userarraycomponents[i] = mesh.buffers[0]->GetUserArrayComponentCount (i);
  }

  if (mesh.num_vertices_pool > 1)
  {
    // Vertex morphing.
    float tr = mesh.morph_factor;
    float remainder = 1 - tr;
    csVector3* f2 = mesh.buffers[1]->GetVertices ();
    csVector2* uv2 = mesh.buffers[1]->GetTexels ();
    csColor* col2 = mesh.buffers[1]->GetColors ();
    for (i = 0 ; i < num_vertices ; i++)
    {
      (*tr_verts)[i] = tr * f2[i] + remainder * f1[i];
      if (mesh.do_morph_texels)
        (*uv_verts)[i] = tr * uv2[i] + remainder * uv1[i];
      if (mesh.do_morph_colors)
      {
        (*color_verts)[i].red = tr * col2[i].red + remainder * col1[i].red;
	(*color_verts)[i].green = tr * col2[i].green + remainder * col1[i].green;
	(*color_verts)[i].blue = tr * col2[i].blue + remainder * col1[i].blue;
      }
    }
    work_verts = tr_verts->GetArray ();
    if (mesh.do_morph_texels)
      work_uv_verts = uv_verts->GetArray ();
    else
      work_uv_verts = uv1;
    if (mesh.do_morph_colors)
      work_colors = color_verts->GetArray ();
    else
      work_colors = col1;
  }
  else
  {
    work_verts = f1;
    work_uv_verts = uv1;
    work_colors = col1;
  }
  csTriangle* triangles = mesh.triangles;
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
      work_userarrays,
      userarraycomponents,
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
      work_verts = clipped_vertices->GetArray ();
      work_uv_verts = clipped_texels->GetArray ();
      work_colors = clipped_colors->GetArray ();
      work_fog = clipped_fog->GetArray ();
      for (i=0; i<CS_VBUF_TOTAL_USERA; i++)
      {
        if (work_userarrays[i])
          work_userarrays[i] = clipped_user[i]->GetArray();
        else
          work_userarrays[i] = NULL;
      }
    }
    triangles = clipped_triangles->GetArray ();
    if (num_triangles <= 0) return; // Nothing to do!
  }

  //===========
  // First setup the clipper that we need.
  //===========
  if (how_clip == 's')
  {
    SetupStencil ();
    stencil_enabled = true;
    // Use the stencil area.
    statecache->EnableState (GL_STENCIL_TEST);
    statecache->SetStencilFunc (GL_EQUAL, 1, 1);
    statecache->SetStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
  }
  else if (how_clip == 'p')
  {
    SetupClipPlanes (do_near_plane && mesh.clip_plane != CS_CLIP_NOT,
      mesh.clip_z_plane != CS_CLIP_NOT);
    clip_planes_enabled = true;
    for (i = 0 ; i < frustum.GetVertexCount ()+reserved_planes ; i++)
      statecache->EnableState ((GLenum)(GL_CLIP_PLANE0+i));
  }

  // set up coordinate transform
  GLfloat matrixholder[16];

  //glPushAttrib( GL_ALL_ATTRIB_BITS );
  //glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

  glMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  glLoadIdentity ();

  //===========
  // set up world->camera transform, if needed
  //===========
  if (mesh.vertex_mode == G3DTriangleMesh::VM_WORLDSPACE)
  {
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

  glMatrixMode (GL_PROJECTION);
  glPushMatrix ();
  glLoadIdentity ();

  // With the back buffer procedural textures the orthographic projection
  // matrix is inverted.
  SetGlOrtho (inverted);

  glTranslatef (asp_center_x, asp_center_y, 0);
  for (i = 0 ; i < 16 ; i++) matrixholder[i] = 0.0;
  matrixholder[0] = matrixholder[5] = 1.0;
  matrixholder[11] = inv_aspect;
  matrixholder[14] = -inv_aspect;
  glMultMatrixf (matrixholder);


  //===========
  // Draw the base mesh.
  //===========
  glVertexPointer (3, GL_FLOAT, 0, &work_verts[0]);

  statecache->SetShadeModel (GL_SMOOTH);

  SetClientStates( CS_CLIENTSTATE_ALL );

  //@@@EXPERIMENTAL!!
  //CONTAINS EXPERIMENTAL VERSION OF RendererData-system  by Mårten Svanfeldt

  int maxlayers = 0;
  for (int p=0 ; p<technique->GetPassCount () ; p++)
  {
    iEffectPass* pass = technique->GetPass (p);

    //get rendererdata to use
    csRef<csOpenGlEffectPassData> pass_data =
      SCF_QUERY_INTERFACE (pass->GetRendererData(), csOpenGlEffectPassData);

    if (p == 0)
      SetGLZBufferFlags (z_buf_mode);
    else if (p == 1)
      SetGLZBufferFlagsPass2 (z_buf_mode, true);

    if (pass_data->vertex_program > 0)
    {
      ///@@@HACK.. THESE SHOULD BE CHANEGD
      //set all constants

      for(int i = 0; i<pass_data->vertex_constants.Length(); i++)
      {
        csOpenGlVPConstant* c = (csOpenGlVPConstant*)pass_data
		->vertex_constants[i];
        if (c->efvariableType == CS_EFVARIABLETYPE_FLOAT)
        {
          //set a float
          float var = effect->GetVariableFloat(c->variableID);
          glProgramLocalParameter4fARB( GL_VERTEX_PROGRAM_ARB,
            c->constantNumber, var, var, var, var);

        }
        else if  (c->efvariableType == CS_EFVARIABLETYPE_VECTOR4)
        {
          //set a vec4
          csEffectVector4 vec = effect->GetVariableVector4(c->variableID);
          glProgramLocalParameter4fARB( GL_VERTEX_PROGRAM_ARB,
              c->constantNumber, vec.x, vec.y, vec.z, vec.w);
        }
      }
      glBindProgramARB(GL_VERTEX_PROGRAM_ARB, pass_data->vertex_program);
      statecache->EnableState( GL_VERTEX_PROGRAM_ARB );
    }

    if (pass_data->doblending)
    {
      statecache->EnableState (GL_BLEND);
      statecache->SetBlendFunc (pass_data->sblend, pass_data->dblend);
    }
    else
    {
      statecache->DisableState (GL_BLEND);
    }

    statecache->SetShadeModel (pass_data->shade_state);

    if (pass_data->vcsource == ED_SOURCE_FOG)
    {
      glColorPointer (3, GL_FLOAT, sizeof(G3DFogInfo), & work_fog[0].r);
      glEnableClientState (GL_COLOR_ARRAY);
    }
    else
    {
      if (mesh.buffers[0]->GetColors())
      {
        glColorPointer (3, GL_FLOAT, 0, work_colors);
        glEnableClientState (GL_COLOR_ARRAY);
      }
      else
      {
        glDisableClientState (GL_COLOR_ARRAY);
        glColor4f (1,1,1,1);
      }
    }


    if (pass->GetLayerCount() > maxlayers) maxlayers = pass->GetLayerCount();
    for (l=0 ; l<pass->GetLayerCount() ; l++ )
    {
      iEffectLayer* layer = pass->GetLayer (l);

      csRef<csOpenGlEffectLayerData> layer_data = SCF_QUERY_INTERFACE (
        layer->GetRendererData(), csOpenGlEffectLayerData);

      if (ARB_multitexture && (ARB_texture_env_combine
          || EXT_texture_env_combine))
      {
        glActiveTextureARB( GL_TEXTURE0_ARB + l );
        glClientActiveTextureARB( GL_TEXTURE0_ARB + l );
      }

      if (layer_data->ccsource == ED_SOURCE_FOG)
      {
        //float tmp[4] = {1, 0, 0, 1};
        glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, & work_fog[0].r );
      }

      if (layer_data->vcord_source == ED_SOURCE_FOG)
      {
        glTexCoordPointer (2, GL_FLOAT, sizeof(G3DFogInfo),
		&work_fog[0].intensity);
        glEnableClientState (GL_TEXTURE_COORD_ARRAY);
      }
      else if (layer_data->vcord_source == ED_SOURCE_LIGHTMAP)
      {
        if (!lightmapcoords)
        {
          glTexCoordPointer (2, GL_FLOAT, 0, work_uv_verts);
          glEnableClientState (GL_TEXTURE_COORD_ARRAY);
        }
	else
	{
	  /*glTexCoordPointer (2, GL_FLOAT, sizeof(csVector2),
	          &((csVector2*)work_userarrays[CS_GL_LIGHTMAP_USERA])->x);
	    glEnableClientState(GL_TEXTURE_COORD_ARRAY);*/
	  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
      }
      else if (layer_data->vcord_source == ED_SOURCE_MESH)
      {
        glTexCoordPointer (2, GL_FLOAT, 0, work_uv_verts);
        glEnableClientState (GL_TEXTURE_COORD_ARRAY);
      }
      else if ((layer_data->vcord_source >= ED_SOURCE_USERARRAY(0)) &&
	       (layer_data->vcord_source
	       	< ED_SOURCE_USERARRAY(CS_VBUF_TOTAL_USERA)))
      {
        int idx = layer_data->vcord_source-ED_SOURCE_USERARRAY(0);
        if (work_userarrays[idx])
        {
          glTexCoordPointer (userarraycomponents[idx],
            GL_FLOAT, 0, work_userarrays[idx]);
          glEnableClientState (GL_TEXTURE_COORD_ARRAY);
        }
      }

      GLuint texturehandle = 0;
      iTextureHandle* txt_handle = NULL;

      if (layer_data->inputtex==-1)
      {
        statecache->SetTexture (GL_TEXTURE_2D, m_fogtexturehandle);
        statecache->EnableState (GL_TEXTURE_2D, l);
      }
      else if (layer_data->inputtex==-2)
      {
        statecache->SetTexture (GL_TEXTURE_2D, lightmap);
        statecache->EnableState (GL_TEXTURE_2D, l);
      }
      else if (layer_data->inputtex==0)
      {
        txt_handle = 0;
      }
      else if (layer_data->inputtex==1)
      {
        CacheTexture (mesh.mat_handle);
        txt_handle = mesh.mat_handle->GetTexture ();
      }
      else if( layer_data->inputtex-2<((csMaterialHandle*)mesh.mat_handle)
        ->GetTextureLayerCount())
      {
        csTextureLayer* lay = ((csMaterialHandle*)mesh.mat_handle)
	  ->GetTextureLayer (layer_data->inputtex-2);
        txt_handle = lay->txt_handle;
      }

      if (txt_handle)
      {
        csTextureHandleOpenGL *txt_mm = (csTextureHandleOpenGL *)
		txt_handle->GetPrivateObject ();
        csTxtCacheData *cachedata = (csTxtCacheData *)txt_mm->GetCacheData ();
        texturehandle = cachedata->Handle;

        statecache->SetTexture (GL_TEXTURE_2D, texturehandle, l);
        statecache->EnableState (GL_TEXTURE_2D, l);
      }

      if (ARB_texture_env_combine || EXT_texture_env_combine)
      {
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);

        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, layer_data->colorsource[0]);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, layer_data->colormod[0]);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, layer_data->colorsource[1]);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, layer_data->colormod[1]);
        if (layer_data->colorsource[2] != -1)
        {
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, layer_data->colorsource[2]);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, layer_data->colormod[2]);
        }
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, layer_data->colorp );
	glTexEnvfv(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, layer_data->scale_rgb);

        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, layer_data->alphasource[0]);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, layer_data->alphamod[0]);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, layer_data->alphasource[1]);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB, layer_data->alphamod[1]);
        if (layer_data->colorsource[2] != -1)
        {
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_ARB, layer_data->alphasource[2]);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_ARB, layer_data->alphamod[2]);
        }
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, layer_data->alphap);
	glTexEnvf(GL_TEXTURE_ENV, GL_ALPHA_SCALE, layer_data->scale_alpha);
      }
    }
    glDrawElements (GL_TRIANGLES, num_triangles*3, GL_UNSIGNED_INT, triangles);

    if (pass_data->vertex_program > 0)
      statecache->DisableState (GL_VERTEX_PROGRAM_ARB);
  }



  if (ARB_multitexture && (ARB_texture_env_combine || EXT_texture_env_combine))
  {
    for (l=maxlayers-1 ; l>=0 ; l--)
    {
      glActiveTextureARB (GL_TEXTURE0_ARB+l);
      glClientActiveTextureARB (GL_TEXTURE0_ARB+l);
      if (l>0)
        statecache->DisableState (GL_TEXTURE_2D, l);
      glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }
  }

  if (stencil_enabled)
    statecache->DisableState (GL_STENCIL_TEST);
  if (clip_planes_enabled)
    for (i = 0 ; i < frustum.GetVertexCount ()+reserved_planes ; i++)
      statecache->DisableState ((GLenum)(GL_CLIP_PLANE0+i));

  glPopMatrix ();
  glMatrixMode (GL_MODELVIEW);
  glPopMatrix ();

  //glPopClientAttrib();
  //glPopAttrib();
}

void csGraphics3DOGLCommon::OldDrawTriangleMesh (G3DTriangleMesh& mesh)
{
#if CS_DEBUG
  // Check if the vertex buffers are locked.
  CS_ASSERT (mesh.buffers[0]->IsLocked ());
  if (mesh.num_vertices_pool > 1)
  {
    CS_ASSERT (mesh.buffers[1]->IsLocked ());
  }
#endif

  int num_vertices = mesh.buffers[0]->GetVertexCount ();

  FlushDrawPolygon ();
  //iTextureHandle *tex = mesh.mat_handle?mesh.mat_handle->GetTexture():NULL;

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
  int num_triangles = mesh.num_triangles;
  if (num_vertices > tr_verts->Limit ())
  {
    tr_verts->SetLimit (num_vertices);
    uv_verts->SetLimit (num_vertices);
    color_verts->SetLimit (num_vertices);
  }

  //===========
  // Do vertex tweening and/or transformation to camera space
  // if any of those are needed. When this is done 'verts' will
  // point to an array of camera vertices.
  //===========
  csVector3* f1 = mesh.buffers[0]->GetVertices ();
  csVector2* uv1 = mesh.buffers[0]->GetTexels ();
  csColor* col1 = mesh.buffers[0]->GetColors ();
  if (!col1) mesh.do_morph_colors = false;
  csVector3* work_verts;
  csVector2* work_uv_verts;
  csColor* work_colors;

  if (mesh.num_vertices_pool > 1)
  {
    // Vertex morphing.
    float tr = mesh.morph_factor;
    float remainder = 1 - tr;
    csVector3* f2 = mesh.buffers[1]->GetVertices ();
    csVector2* uv2 = mesh.buffers[1]->GetTexels ();
    csColor* col2 = mesh.buffers[1]->GetColors ();
    for (i = 0 ; i < num_vertices ; i++)
    {
      (*tr_verts)[i] = tr * f2[i] + remainder * f1[i];
      if (mesh.do_morph_texels)
        (*uv_verts)[i] = tr * uv2[i] + remainder * uv1[i];
      if (mesh.do_morph_colors)
      {
        (*color_verts)[i].red = tr * col2[i].red + remainder * col1[i].red;
  (*color_verts)[i].green = tr * col2[i].green + remainder * col1[i].green;
  (*color_verts)[i].blue = tr * col2[i].blue + remainder * col1[i].blue;
      }
    }
    work_verts = tr_verts->GetArray ();
    if (mesh.do_morph_texels)
      work_uv_verts = uv_verts->GetArray ();
    else
      work_uv_verts = uv1;
    if (mesh.do_morph_colors)
      work_colors = color_verts->GetArray ();
    else
      work_colors = col1;
  }
  else
  {
    work_verts = f1;
    work_uv_verts = uv1;
    work_colors = col1;
  }
  csTriangle* triangles = mesh.triangles;
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
	NULL,
	NULL,
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
      work_verts = clipped_vertices->GetArray ();
      work_uv_verts = clipped_texels->GetArray ();
      work_colors = clipped_colors->GetArray ();
      work_fog = clipped_fog->GetArray ();
    }
    triangles = clipped_triangles->GetArray ();
    if (num_triangles <= 0) return; // Nothing to do!
  }

  //===========
  // First setup the clipper that we need.
  //===========
  if (how_clip == 's')
  {
    SetupStencil ();
    stencil_enabled = true;
    // Use the stencil area.
    statecache->EnableState (GL_STENCIL_TEST);
    statecache->SetStencilFunc (GL_EQUAL, 1, 1);
    statecache->SetStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
  }
  else if (how_clip == 'p')
  {
    SetupClipPlanes (do_near_plane && mesh.clip_plane != CS_CLIP_NOT,
      mesh.clip_z_plane != CS_CLIP_NOT);
    clip_planes_enabled = true;
    for (i = 0 ; i < frustum.GetVertexCount ()+reserved_planes ; i++)
      statecache->EnableState ((GLenum)(GL_CLIP_PLANE0+i));
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
  SetGlOrtho (inverted);

  glTranslatef (asp_center_x, asp_center_y, 0);
  for (i = 0 ; i < 16 ; i++) matrixholder[i] = 0.0;
  matrixholder[0] = matrixholder[5] = 1.0;
  matrixholder[11] = inv_aspect;
  matrixholder[14] = -inv_aspect;
  glMultMatrixf (matrixholder);

  //===========
  // Setup states
  //===========
  uint m_mixmode = mesh.mixmode;
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
    statecache->SetTexture (GL_TEXTURE_2D, texturehandle);
    statecache->EnableState (GL_TEXTURE_2D);
  }
  else
    statecache->DisableState (GL_TEXTURE_2D);

  SetGLZBufferFlags (z_buf_mode);

  csMaterialHandle* mat = NULL;
  iTextureHandle* txt_handle = NULL;
  if (mesh.mat_handle)
  {
    txt_handle = mesh.mat_handle->GetTexture ();
    mat = (csMaterialHandle*)mesh.mat_handle;
  }
  bool do_gouraud = m_gouraud && work_colors;

  //--------
  // First see if the material specified a flat color.
  // If so we need to modify the color array with this
  // flat color.
  //--------
  float flat_r = 1., flat_g = 1., flat_b = 1.;
  // If do_multiply_color == true this means that we will multiply
  // the base color (color array in mesh) with the flat color from
  // the material.
  bool do_multiply_color = !m_textured;
  if (mesh.mat_handle && !m_textured)
  {
    csRGBpixel color;
    if (txt_handle)
      txt_handle->GetMeanColor (color.red, color.green, color.blue);
    else
    {
      // We have a material without texture. In this case the flat
      // color will be used in all cases. We only do this if the
      // flat color from the material is different from white though.
      mesh.mat_handle->GetFlatColor (color);
      if (color.red < 255 || color.green < 255 || color.blue < 255)
        do_multiply_color = true;
    }
    flat_r = BYTE_TO_FLOAT (color.red);
    flat_g = BYTE_TO_FLOAT (color.green);
    flat_b = BYTE_TO_FLOAT (color.blue);
  }

  if (do_gouraud)
  {
    // special hack for transparent meshes
    if (mesh.mixmode & CS_FX_ALPHA)
    {
      if ((num_vertices*4) > rgba_verts->Limit ())
        rgba_verts->SetLimit (num_vertices*4);
      if (do_multiply_color)
      {
        for (k=0, i=0; i<num_vertices; i++)
        {
          (*rgba_verts)[k++] = work_colors[i].red * flat_r;
          (*rgba_verts)[k++] = work_colors[i].green * flat_g;
          (*rgba_verts)[k++] = work_colors[i].blue * flat_b;
	  (*rgba_verts)[k++] = m_alpha;
        }
      }
      else
      {
        for (k=0, i=0; i<num_vertices; i++)
        {
          (*rgba_verts)[k++] = work_colors[i].red;
          (*rgba_verts)[k++] = work_colors[i].green;
          (*rgba_verts)[k++] = work_colors[i].blue;
	  (*rgba_verts)[k++] = m_alpha;
        }
      }
    }
    else if (do_multiply_color)
    {
      // If we are already using the array in color_verts then
      // we do not have to copy.
      if (work_colors == color_verts->GetArray ())
      {
        for (i = 0 ; i < num_vertices ; i++)
	{
	  work_colors[i].red *= flat_r;
	  work_colors[i].green *= flat_g;
	  work_colors[i].blue *= flat_b;
	}
      }
      else
      {
        csColor* old = work_colors;
	if (num_vertices > color_verts->Limit())
	  color_verts->SetLimit(num_vertices);
	work_colors = color_verts->GetArray ();
	for (i = 0 ; i < num_vertices ; i++)
	{
	  work_colors[i].red = old[i].red * flat_r;
	  work_colors[i].green = old[i].green * flat_g;
	  work_colors[i].blue = old[i].blue * flat_b;
	}
      }
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
    statecache->SetShadeModel (GL_SMOOTH);
    SetClientStates (CS_CLIENTSTATE_ALL);
    if (mesh.mixmode & CS_FX_ALPHA)
      glColorPointer (4, GL_FLOAT, 0, rgba_verts->GetArray() );
    else
      glColorPointer (3, GL_FLOAT, 0, & work_colors[0]);
  }
  else
  {
    SetClientStates (CS_CLIENTSTATE_VT);
    statecache->SetShadeModel (GL_FLAT);
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
    statecache->SetShadeModel (GL_FLAT);
    SetClientStates (CS_CLIENTSTATE_VT);
    if (num_vertices > uv_mul_verts->Limit ())
      uv_mul_verts->SetLimit (num_vertices);

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
      statecache->SetTexture (GL_TEXTURE_2D, texturehandle);
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
	mul_uv = uv_mul_verts->GetArray ();
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
      statecache->DisableState (GL_TEXTURE_2D);
      statecache->SetShadeModel (GL_SMOOTH);
      SetupBlend (CS_FX_MULTIPLY2, 0, false);
      SetClientStates (CS_CLIENTSTATE_ALL);
      if (mesh.mixmode & CS_FX_ALPHA)
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
    statecache->EnableState (GL_TEXTURE_2D);
    statecache->SetTexture (GL_TEXTURE_2D, m_fogtexturehandle);
    SetupBlend (CS_FX_ALPHA, 0, false);

    statecache->SetShadeModel (GL_SMOOTH);
    SetClientStates (CS_CLIENTSTATE_ALL);
    glVertexPointer (3, GL_FLOAT, 0, & work_verts[0]);
    glTexCoordPointer (2, GL_FLOAT, sizeof(G3DFogInfo), &work_fog[0].intensity);
    glColorPointer (3, GL_FLOAT, sizeof (G3DFogInfo), &work_fog[0].r);
    glDrawElements (GL_TRIANGLES, num_triangles*3, GL_UNSIGNED_INT, triangles);

    if (!m_textured)
      statecache->DisableState (GL_TEXTURE_2D);
    if (!m_gouraud)
      statecache->SetShadeModel (GL_FLAT);
  }

  glMatrixMode (GL_MODELVIEW);
  glPopMatrix ();
  glMatrixMode (GL_PROJECTION);
  glPopMatrix ();

  //===========
  // Disable/cleanup all clipping stuff.
  //===========
  if (stencil_enabled)
    statecache->DisableState (GL_STENCIL_TEST);
  if (clip_planes_enabled)
    for (i = 0 ; i < frustum.GetVertexCount ()+reserved_planes ; i++)
      statecache->DisableState ((GLenum)(GL_CLIP_PLANE0+i));

  if (debug_edges)
    DrawTriangleMeshEdges (mesh);
    //DebugDrawElements (G2D,
  //num_triangles*3, (int*)triangles, (GLfloat*)& work_verts[0],
    //G2D->FindRGB (255, 0, 0), true,
    //mesh.vertex_mode == G3DTriangleMesh::VM_VIEWSPACE);

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

void csGraphics3DOGLCommon::RemoveFromCache (iPolygonTexture* /*poly_texture*/)
{
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
  if (!lightmap_cache) return;  // System is being destructed.
  FlushDrawPolygon ();
  lightmap_cache->Clear ();
}

void csGraphics3DOGLCommon::DumpCache ()
{
}

void csGraphics3DOGLCommon::DrawLine (const csVector3 & v1,
  const csVector3 & v2, float fov, int color)
{
  FlushDrawPolygon ();

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
  switch (flags)
  {
    case CS_ZBUF_NONE:
      statecache->DisableState (GL_DEPTH_TEST);
      //glDepthMask (GL_FALSE);@@@ Is this needed or not?
      break;
    case CS_ZBUF_FILL:
    case CS_ZBUF_FILLONLY:
      statecache->EnableState (GL_DEPTH_TEST);
      statecache->SetDepthFunc (GL_ALWAYS);
      statecache->SetDepthMask (GL_TRUE);
      break;
    case CS_ZBUF_EQUAL:
      statecache->EnableState (GL_DEPTH_TEST);
      statecache->SetDepthFunc (GL_EQUAL);
      statecache->SetDepthMask (GL_FALSE);
      break;
    case CS_ZBUF_TEST:
      statecache->EnableState (GL_DEPTH_TEST);
      statecache->SetDepthFunc (GL_GREATER);
      statecache->SetDepthMask (GL_FALSE);
      break;
    case CS_ZBUF_USE:
      statecache->EnableState (GL_DEPTH_TEST);
      statecache->SetDepthFunc (GL_GREATER);
      statecache->SetDepthMask (GL_TRUE);
      break;
    default:
      break;
  }
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
  (this->*DrawPolygonCall)(poly);
}

void csGraphics3DOGLCommon::DrawPixmap (iTextureHandle *hTex,
  int sx, int sy, int sw, int sh, int tx, int ty, int tw, int th, uint8 Alpha)
{
  FlushDrawPolygon ();

  // If original dimensions are different from current dimensions (because
  // image has been scaled to conform to OpenGL texture size restrictions)
  // we correct the input coordinates here.
  int bitmapwidth = 0, bitmapheight = 0;
  hTex->GetMipMapDimensions (0, bitmapwidth, bitmapheight);
  csTextureHandleOpenGL *txt_mm = (csTextureHandleOpenGL *)
    hTex->GetPrivateObject ();
  int owidth = txt_mm->orig_width;
  int oheight = txt_mm->orig_height;
  if (owidth != bitmapwidth || oheight != bitmapheight)
  {
    //sx = sx * owidth / bitmapwidth;
    //sy = sy * oheight / bitmapheight;
    sw = sw * owidth / bitmapwidth;
    sh = sh * oheight / bitmapheight;
  }

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
  }
  if (sx + sw > ClipX2)                 // Right margin crossed?
  {
    int nw = ClipX2 - sx;               // New width
    _tw = (_tw * nw) / sw;              // Adjust width on texture
    sw = nw;
  }
  if (sy < ClipY1)                      // Top margin crossed?
  {
    int nh = sh - (ClipY1 - sy);        // New height
    _ty += (ClipY1 - sy) * _th / sh;    // Adjust Y coord on texture
    _th = (_th * nh) / sh;              // Adjust height on texture
    sh = nh; sy = ClipY1;
  }
  if (sy + sh > ClipY2)                 // Bottom margin crossed?
  {
    int nh = ClipY2 - sy;               // New height
    _th = (_th * nh) / sh;              // Adjust height on texture
    sh = nh;
  }

  // cache the texture if we haven't already.
  texture_cache->Cache (hTex);

  // Get texture handle
  GLuint texturehandle = ((csTxtCacheData *)txt_mm->GetCacheData ())->Handle;

  // as we are drawing in 2D, we disable some of the commonly used features
  // for fancy 3D drawing
  statecache->SetShadeModel (GL_FLAT);
  SetGLZBufferFlags (CS_ZBUF_NONE);
  //@@@???statecache->SetDepthMask (GL_FALSE);

  // if the texture has transparent bits, we have to tweak the
  // OpenGL blend mode so that it handles the transparent pixels correctly
  if (hTex->GetKeyColor () || hTex->GetAlphaMap () || Alpha)
    SetupBlend (CS_FX_ALPHA, 0, false);
  else
    SetupBlend (CS_FX_COPY, 0, false);

  statecache->EnableState (GL_TEXTURE_2D);
  glColor4f (1.0, 1.0, 1.0, Alpha ? (1.0 - BYTE_TO_FLOAT (Alpha)) : 1.0);
  statecache->SetTexture (GL_TEXTURE_2D, texturehandle);

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
  statecache->DisableState (GL_TEXTURE_2D);
  statecache->DisableState (GL_DEPTH_TEST);
  statecache->SetShadeModel (GL_FLAT);

  // blend mode one

  statecache->DisableState (GL_BLEND);
  glColor3fv (testcolor1);
  glBegin (GL_QUADS);
  glVertex2i (0,0); glVertex2i(5,0); glVertex2i(5,5); glVertex2i(0,5);
  glEnd ();

  statecache->EnableState (GL_BLEND);
  statecache->SetBlendFunc (GL_DST_COLOR, GL_ZERO);
  glColor3fv (testcolor2);
  glBegin (GL_QUADS);
  glVertex2i (0,0); glVertex2i(5,0); glVertex2i(5,5); glVertex2i(0,5);
  glEnd ();

  glReadPixels (2,2,1,1,GL_RGB,GL_FLOAT, &blendresult1);

  // blend mode two

  statecache->DisableState (GL_BLEND);
  glColor3fv (testcolor1);
  glBegin (GL_QUADS);
  glVertex2i (0,0); glVertex2i (5,0); glVertex2i (5,5); glVertex2i (0,5);
  glEnd ();

  statecache->EnableState (GL_BLEND);
  statecache->SetBlendFunc (GL_DST_COLOR, GL_SRC_COLOR);
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

iTextureManager *csGraphics3DOGLCommon::GetTextureManager ()
{
  return txtmgr;
}

bool csGraphics3DOGLCommon::IsLightmapOK (iPolygonTexture* poly_texture)
{
  return lightmap_cache->IsLightmapOK (poly_texture);
}

void csGraphics3DOGLCommon::SetRenderTarget (iTextureHandle* handle,
	bool persistent)
{
  render_target = handle;
  rt_onscreen = !persistent;
  rt_cliprectset = false;
}


///@@@EXPERIMENTAL!!
///CONTAINS EXPERIMENTAL VERSION OF RendererData-system  by Mårten Svanfeldt
bool csGraphics3DOGLCommon::Validate( iEffectDefinition* effect, iEffectTechnique* technique )
{
  int p, l;
  csEffectStrings* efstrings = effectserver->GetStandardStrings();

  for( p=0; p<technique->GetPassCount(); p++ )
  {
    csRef<csOpenGlEffectPassData> pass_data = 
      csPtr <csOpenGlEffectPassData> (new csOpenGlEffectPassData());

    iEffectPass* pass = technique->GetPass(p);
    csStringID pass_state = pass->GetFirstState();
    csStringID pass_statestring;

    while(pass_state != csInvalidStringID )
    {
      if (pass_state == efstrings->blending )
      {
        pass_statestring = pass->GetStateString( pass_state );
        if( pass_statestring == efstrings->enabled)
          pass_data->doblending = true;
        else if( pass_statestring == efstrings->disabled)
          pass_data->doblending = false;
        else
          return false;

      }
      else if( pass_state == efstrings->shade_mode )
      {
        pass_statestring = pass->GetStateString( pass_state );
        if(pass_statestring == efstrings->flat)
          pass_data->shade_state = GL_FLAT;
        else if(pass_statestring == efstrings->smooth)
          pass_data->shade_state = GL_SMOOTH;
        else
          return false;
      }
      else if( pass_state == efstrings->source_blend_mode )
      {
        pass_statestring = pass->GetStateString( pass_state );
        if( pass_statestring == efstrings->destination_color )
          pass_data->sblend = GL_DST_COLOR;
        else if(pass_statestring == efstrings->inverted_destination_color)
          pass_data->sblend = GL_ONE_MINUS_DST_COLOR;
        else if( pass_statestring == efstrings->source_alpha )
          pass_data->sblend = GL_SRC_ALPHA;
        else if( pass_statestring == efstrings->inverted_source_alpha )
          pass_data->sblend = GL_ONE_MINUS_SRC_ALPHA;
        else if( pass_statestring == efstrings->destination_alpha )
          pass_data->sblend = GL_DST_ALPHA;
        else if(pass_statestring == efstrings->inverted_destination_alpha)
          pass_data->sblend = GL_ONE_MINUS_DST_ALPHA;
        else if( pass_statestring == efstrings->saturated_source_alpha )
          pass_data->sblend = GL_SRC_ALPHA_SATURATE;
        else if( pass_statestring == efstrings->one )
          pass_data->sblend = GL_ONE;
        else if( pass_statestring == efstrings->zero )
          pass_data->sblend = GL_ZERO;
        else return false;
      }
      else if( pass_state == efstrings->destination_blend_mode )
      {
        pass_statestring = pass->GetStateString( pass_state );
        if( pass_statestring == efstrings->source_color )
          pass_data->dblend = GL_SRC_COLOR;
        else if( pass_statestring == efstrings->inverted_source_color )
          pass_data->dblend = GL_ONE_MINUS_SRC_COLOR;
        else if( pass_statestring == efstrings->source_alpha )
          pass_data->dblend = GL_SRC_ALPHA;
        else if( pass_statestring == efstrings->inverted_source_alpha )
          pass_data->dblend = GL_ONE_MINUS_SRC_ALPHA;
        else if( pass_statestring == efstrings->destination_alpha )
          pass_data->dblend = GL_DST_ALPHA;
        else if(pass_statestring == efstrings->inverted_destination_alpha)
          pass_data->dblend = GL_ONE_MINUS_DST_ALPHA;
        else if( pass_statestring == efstrings->one )
          pass_data->dblend = GL_ONE;
        else if( pass_statestring == efstrings->zero )
          pass_data->dblend = GL_ZERO;
        else return false;
      }
      else if( (pass_state == efstrings->vertex_color_source) )
      {
        pass_statestring = pass->GetStateString( pass_state );
        if(pass_statestring == efstrings->fog)
          pass_data->vcsource = ED_SOURCE_FOG;
        else if( pass->GetStateString( pass_state )
          == efstrings->mesh )
          pass_data->vcsource = ED_SOURCE_MESH;
        else
        {
          pass_data->vcsource =
            ED_SOURCE_USERARRAY((int)pass->GetStateString( pass_state )-1);
        }
      }
      else if ( pass_state == efstrings->nvvertex_program_gl )
      {
        if( !ARB_vertex_program || !glGenProgramsARB ||
            !glBindProgramARB || ! glProgramStringARB)
          return false;

        csStringID vp_s = pass->GetStateString(pass_state);
        unsigned char* vp;
        if(vp_s != csInvalidStringID)
        {
          //get a program
          vp = (unsigned char*)effectserver->RequestString(vp_s);
        }
        else
        {
          vp = (unsigned char*)pass->GetStateOpaque(pass_state);
        }
        if (!vp) return false;
        //create and load vertex program

        glGenProgramsARB(1, &pass_data->vertex_program);
        glBindProgramARB(GL_VERTEX_PROGRAM_ARB, pass_data->vertex_program);

        //load it from string
        int i = strlen((const char*)vp);
        glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, i, vp);

        const unsigned char * programErrorString=glGetString(GL_PROGRAM_ERROR_STRING_ARB);

        GLint errorPos;
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);

        if(errorPos != -1)
        {
          Report (CS_REPORTER_SEVERITY_WARNING,
                  "Vertexprogram error at position %d in effect %s (pass %d)",
                  errorPos, effect->GetName(), p);
          Report (CS_REPORTER_SEVERITY_WARNING,
                  "Vertexprogram errorstring: %s", programErrorString);
          return false;
        }
      }
      else
      {
        if(strncasecmp(effectserver->RequestString(pass_state), "vertex program constant", 23) == 0)
        {
          //this is a vertexconstant
          const char* constname = (const char*) effectserver->RequestString(pass_state);
          if ( strlen(constname) < 24) return false; //must contain which constant
          int constnum;
          sscanf(constname, "vertex program constant %d", &constnum);

          if ( (constnum < 4) ||(constnum > 96) ) return false;

          int varnum = effect->GetVariableID(pass->GetStateString(pass_state), false);
          char vartype = effect->GetVariableType(varnum);

          if(vartype == 0) return false;

          pass_data->vertex_constants.Push(new csOpenGlVPConstant(constnum,  varnum, vartype));

        } else
        return false;
      }
      pass_state = pass->GetNextState();
    }
    if( ARB_multitexture &&
      (pass->GetLayerCount() > m_config_options.do_multitexture_level) )
      return false;
    for( l=0; l<pass->GetLayerCount(); l++ )
    {
      iEffectLayer* layer = pass->GetLayer(l);
      csRef<csOpenGlEffectLayerData> layer_data = 
	csPtr<csOpenGlEffectLayerData> (new csOpenGlEffectLayerData);

      csStringID layer_state = layer->GetFirstState();
      csStringID layer_statestring;
      while(layer_state != csInvalidStringID )
      {
        if( (layer_state == efstrings->color_source_1) ||
          (layer_state == efstrings->color_source_2) ||
          (layer_state == efstrings->color_source_3) )
        {
          if (!ARB_texture_env_combine && !EXT_texture_env_combine)
            return false;
          layer_statestring = layer->GetStateString( layer_state );

          //which texture-unit
          int tu = 0;
          if(layer_state == efstrings->color_source_1) tu = 0;
          else if(layer_state == efstrings->color_source_2) tu = 1;
          else if(layer_state == efstrings->color_source_3) tu = 2;
          //which source
          if( layer_statestring == efstrings->vertex_color )
            layer_data->colorsource[tu] = GL_PRIMARY_COLOR_ARB;
          else if( layer_statestring == efstrings->texture_color )
            layer_data->colorsource[tu] = GL_TEXTURE;
          else if( layer_statestring == efstrings->constant_color )
            layer_data->colorsource[tu] = GL_CONSTANT_ARB;
          else if( layer_statestring == efstrings->previous_layer_color )
            layer_data->colorsource[tu] = GL_PREVIOUS_ARB;
          else return false;
        }
        else if( (layer_state == efstrings->color_source_modifier_1) ||
          (layer_state == efstrings->color_source_modifier_2) ||
          (layer_state == efstrings->color_source_modifier_3) )
        {
          if (!ARB_texture_env_combine && !EXT_texture_env_combine)
            return false;
          layer_statestring = layer->GetStateString( layer_state );

          //tu to use..
          int tu = 0;
          if(layer_state == efstrings->color_source_modifier_1)
            tu = 0;
          else if(layer_state == efstrings->color_source_modifier_2)
            tu = 1;
          else if(layer_state == efstrings->color_source_modifier_3)
            tu = 2;

          if( layer_statestring == efstrings->source_color )
            layer_data->colormod[tu] = GL_SRC_COLOR;
          else if( layer_statestring == efstrings->inverted_source_color )
            layer_data->colormod[tu]  = GL_ONE_MINUS_SRC_COLOR;
          else if( layer_statestring == efstrings->source_alpha )
            layer_data->colormod[tu]  = GL_SRC_ALPHA;
          else if( layer_statestring == efstrings->inverted_source_alpha )
            layer_data->colormod[tu]  = GL_ONE_MINUS_SRC_ALPHA;
          else return false;
        }
        else if( (layer_state == efstrings->alpha_source_1) ||
          (layer_state == efstrings->alpha_source_2) ||
          (layer_state == efstrings->alpha_source_3) )
        {
          if (!ARB_texture_env_combine && !EXT_texture_env_combine)
            return false;
          layer_statestring = layer->GetStateString( layer_state );

          int tu = 0;
          if( layer_state == efstrings->alpha_source_1) tu = 0;
          else if( layer_state == efstrings->alpha_source_2) tu = 1;
          else if( layer_state == efstrings->alpha_source_3) tu = 2;

          if( layer_statestring == efstrings->vertex_alpha )
            layer_data->alphasource[tu] = GL_PRIMARY_COLOR_ARB;
          else if( layer_statestring == efstrings->texture_alpha )
            layer_data->alphasource[tu] = GL_TEXTURE;
          else if( layer_statestring == efstrings->constant_alpha )
            layer_data->alphasource[tu] = GL_CONSTANT_ARB;
          else if( layer_statestring == efstrings->previous_layer_alpha )
            layer_data->alphasource[tu] = GL_PREVIOUS_ARB;
          else return false;
        }
        else if( (layer_state == efstrings->alpha_source_modifier_1) ||
          (layer_state == efstrings->alpha_source_modifier_2) ||
          (layer_state == efstrings->alpha_source_modifier_3) )
        {
          if (!ARB_texture_env_combine && !EXT_texture_env_combine)
            return false;
          layer_statestring = layer->GetStateString( layer_state );

          int tu = 0;
          if( layer_state == efstrings->alpha_source_modifier_1)
            tu = 0;
          else if( layer_state == efstrings->alpha_source_modifier_2)
            tu = 1;
          else if( layer_state == efstrings->alpha_source_modifier_3)
            tu = 2;

          if( layer_statestring == efstrings->source_alpha )
            layer_data->alphamod[tu] = GL_SRC_ALPHA;
          else if( layer_statestring == efstrings->inverted_source_alpha )
            layer_data->alphamod[tu] = GL_ONE_MINUS_SRC_ALPHA;
          else return false;
        }
        else if (layer_state == efstrings->alpha_operation)
        {
          if (!ARB_texture_env_combine && !EXT_texture_env_combine)
            return false;
          layer_statestring = layer->GetStateString( layer_state );

          if( layer_statestring == efstrings->use_source_1 )
            layer_data->alphap = GL_REPLACE;
          else if( layer_statestring == efstrings->multiply )
            layer_data->alphap = GL_MODULATE;
          else if( layer_statestring == efstrings->add )
            layer_data->alphap = GL_ADD;
          else if( layer_statestring == efstrings->add_signed )
            layer_data->alphap = GL_ADD_SIGNED_ARB;
          else if( layer_statestring == efstrings->subtract )
            layer_data->alphap = GL_SUBTRACT_ARB;
          else if( layer_statestring == efstrings->interpolate )
            layer_data->alphap = GL_INTERPOLATE_ARB;
          else if( layer_statestring == efstrings->dot_product )
          {
            if(ARB_texture_env_dot3 || EXT_texture_env_dot3)
              layer_data->alphap = GL_DOT3_RGB_ARB;
            else
              return false;
          }
          else if( layer_statestring == efstrings->dot_product_to_alpha )
          {
            if(ARB_texture_env_dot3 || EXT_texture_env_dot3)
              layer_data->alphap = GL_DOT3_RGBA_ARB;
            else
              return false;
          }
          else return false;
        }
        else if( (layer_state == efstrings->color_operation) )
        {
          if (!ARB_texture_env_combine && !EXT_texture_env_combine)
            return false;
          layer_statestring = layer->GetStateString( layer_state );

          if( layer_statestring == efstrings->use_source_1 )
            layer_data->colorp = GL_REPLACE;
          else if( layer_statestring == efstrings->multiply )
            layer_data->colorp = GL_MODULATE;
          else if( layer_statestring == efstrings->add )
            layer_data->colorp = GL_ADD;
          else if( layer_statestring == efstrings->add_signed )
            layer_data->colorp = GL_ADD_SIGNED_ARB;
          else if( layer_statestring == efstrings->subtract )
            layer_data->colorp = GL_SUBTRACT_ARB;
          else if( layer_statestring == efstrings->interpolate )
            layer_data->colorp = GL_INTERPOLATE_ARB;
          else if( layer_statestring == efstrings->dot_product )
          {
            if(ARB_texture_env_dot3 || EXT_texture_env_dot3)
              layer_data->colorp = GL_DOT3_RGB_ARB;
            else
              return false;
          }
          else if( layer_statestring == efstrings->dot_product_to_alpha )
          {
            if(ARB_texture_env_dot3 || EXT_texture_env_dot3)
              layer_data->colorp = GL_DOT3_RGBA_ARB;
            else
              return false;
          }

        }
        else if( (layer_state == efstrings->texture_source) )
        {
          if( (layer->GetStateString( layer_state ) == csInvalidStringID) &&
            (layer->GetStateFloat( layer_state ) == 0) &&
            layer->GetStateOpaque( layer_state ) == NULL )
            return false;
          else if( layer->GetStateString( layer_state) == efstrings->fog)
            layer_data->inputtex = -1;
          else if( layer->GetStateString( layer_state) == efstrings->lightmap)
            layer_data->inputtex = -2;
          else
            // @@@SUSPICIOUS. IS INT, BUT FUNCTION RETURNS FLAOT
            layer_data->inputtex = (int)layer->GetStateFloat(layer_state);
        }
        else if(  (layer_state == efstrings->texture_coordinate_source) )
        {
          if( layer->GetStateString( layer_state )
            == efstrings->fog)
            layer_data->vcord_source = ED_SOURCE_FOG;
          else if( layer->GetStateString( layer_state )
            == efstrings->mesh)
            layer_data->vcord_source = ED_SOURCE_MESH;
          else if( layer->GetStateString( layer_state )
            == efstrings->lightmap)
            layer_data->vcord_source = ED_SOURCE_LIGHTMAP;
          else
          {
            layer_data->vcord_source =
              ED_SOURCE_USERARRAY((int)layer->GetStateFloat( layer_state )-1);
          }
        }
        else if( (layer_state == efstrings->constant_color_source) )
        {
          layer_statestring = layer->GetStateString( layer_state );

          if( layer_statestring == efstrings->fog)
            layer_data->ccsource = ED_SOURCE_FOG;
          else
            layer_data->ccsource = ED_SOURCE_NONE;
        }
	else if ((layer_state == efstrings->scale_rgb))
	{
	  csEffectVector4 layer_scale_rgb = layer->GetStateVector4 (layer_state);

	  layer_data->scale_rgb[0] = layer_scale_rgb.x;
	  layer_data->scale_rgb[1] = layer_scale_rgb.y;
	  layer_data->scale_rgb[2] = layer_scale_rgb.z;
	}
	else if ((layer_state == efstrings->scale_alpha))
	{
	  float layer_scale_alpha = layer->GetStateFloat (layer_state);

	  layer_data->scale_alpha = layer_scale_alpha;
	}
        else
          return false;
        layer_state = layer->GetNextState();
      }
      layer->SetRendererData( layer_data );
    }
    //csRef<iBase> pass_data_b = ;
    pass->SetRendererData( pass_data );
  }
  return true;
}

bool csGraphics3DOGLCommon::CheckGLError (char* call)
{
  GLenum res = glGetError();

  if (res == GL_NO_ERROR)
  {
    return true;
  }
  else
  {
    char msg[64];
    switch(res)
    {
      case GL_INVALID_ENUM:
        strcpy (msg, "enum argument out of range");
	break;
      case GL_INVALID_VALUE:
        strcpy (msg, "Numeric argument out of range");
	break;
      case GL_INVALID_OPERATION:
	strcpy (msg, "Operation illegal in current state");
	break;
      case GL_STACK_OVERFLOW: 
	strcpy (msg, "Command would cause a stack overflow");
	break;
      case GL_STACK_UNDERFLOW:
	strcpy (msg, "Command would cause a stack undeflow");
	break;
      case GL_OUT_OF_MEMORY:
	strcpy (msg, "Not enough memory left to execute command");
	break;
      case GL_TABLE_TOO_LARGE:
	strcpy (msg, "The specified table is too large");
	break;
      default:
	sprintf (msg, "unknown GL error %x", (unsigned int)res);
	break;
    }
    Report (CS_REPORTER_SEVERITY_WARNING,
      "GL reported error for %s: %s", call, msg);
    
    return false;
  }
}
