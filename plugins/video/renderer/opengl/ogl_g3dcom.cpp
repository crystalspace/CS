/*
 * Copyright (C) 1998 by Jorrit Tyberghein
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

#include "cssysdef.h"
#include "cssys/sysdriv.h"
#include "qint.h"
#include "csutil/scf.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/polyclip.h"
#include "csgeom/plane3.h"
#include "ogl_g3dcom.h"
#include "ogl_txtcache.h"
#include "ogl_txtmgr.h"
#include "icfgnew.h"
#include "isystem.h"
#include "igraph3d.h"
#include "itxtmgr.h"
#include "itexture.h"
#include "ipolygon.h"
#include "icamera.h"
#include "ilghtmap.h"
#include "igraph2d.h"
#include "csutil/garray.h"
#include "csutil/cscolor.h"
#include "csgfxldr/rgbpixel.h"

#include <GL/gl.h>
#include <GL/glu.h>

#define SysPrintf System->Printf

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

//@@@ Another experimental optimization:
// This optimization alone doesn't appear to be enough.
// We need better state caching. I keep it here to remind
// us of this.
//static GLuint prev_handle = 0;
void csglBindTexture (GLenum target, GLuint handle)
{
  //if (prev_handle != handle)
  //{
    //prev_handle = handle;
    glBindTexture (target, handle);
  //}
}

//@@@ For opt DrawPolygon:

void csGraphics3DOGLCommon::start_draw_poly ()
{
  if (in_draw_poly) return;
  in_draw_poly = true;
}

void csGraphics3DOGLCommon::end_draw_poly ()
{
  if (!in_draw_poly) return;
  in_draw_poly = false;
}

/*===========================================================================
 Fog Variables Section
 ===========================================================================*/

// size of the fog texture
static const unsigned int FOGTABLE_SIZE = 64;

// each texel in the fog table holds the fog alpha value at a certain
// (distance*density).  The median distance parameter determines the
// (distance*density) value represented by the texel at the center of
// the fog table.  The fog calculation is:
// alpha = 1.0 - exp( -(density*distance) / FOGTABLE_MEDIANDISTANCE)
//
static const double FOGTABLE_MEDIANDISTANCE = 10.0;
static const double FOGTABLE_MAXDISTANCE = FOGTABLE_MEDIANDISTANCE * 2.0;
static const double FOGTABLE_DISTANCESCALE = 1.0 / FOGTABLE_MAXDISTANCE;

// fog (distance*density) is mapped to a texture coordinate and then
// clamped.  This determines the clamp value.  Some OpenGL drivers don't
// like clamping textures so we must do it ourself
static const double FOGTABLE_CLAMPVALUE = 0.85;

static const double FOG_MAXVALUE = FOGTABLE_MAXDISTANCE * FOGTABLE_CLAMPVALUE;
/*=========================================================================
 Static growing array declaration for DrawTriangleMesh
=========================================================================*/
// smgh moved it here, no longer segfaults on exit as a consequence..
// Also IncRefing and DecRefing in the ctor/dtor, as the auxiliary buffer
// dynamic textures will utilise multiple instances of csGraphics3DOGLCommon

/// Static vertex array.
static DECLARE_GROWING_ARRAY_REF (tr_verts, csVector3);
/// Static uv array.
static DECLARE_GROWING_ARRAY_REF (uv_verts, csVector2);
/// The perspective corrected vertices.
//static DECLARE_GROWING_ARRAY_REF (persp, csVector3);
/// Array which indicates which vertices are visible and which are not.
//static DECLARE_GROWING_ARRAY_REF (visible, bool);
/// Array with colors.
static DECLARE_GROWING_ARRAY_REF (color_verts, csColor);
/// Array with RGBA colors.
static DECLARE_GROWING_ARRAY_REF (rgba_verts, GLfloat);
/// Array with fog values.
static DECLARE_GROWING_ARRAY_REF (fog_intensities, float);
/// Array with fog colors
static DECLARE_GROWING_ARRAY_REF (fog_color_verts, csColor);

/*=========================================================================
 Method implementations
=========================================================================*/

csGraphics3DOGLCommon::csGraphics3DOGLCommon ():
  G2D (NULL), config (NULL), System (NULL)
{
  ShortcutDrawPolygon = 0;
  ShortcutStartPolygonFX = 0;
  ShortcutDrawPolygonFX = 0;
  ShortcutFinishPolygonFX = 0;

  texture_cache = NULL;
  lightmap_cache = NULL;
  txtmgr = NULL;
  m_fogtexturehandle = 0;
  dbg_max_polygons_to_draw = 2000000000;	// After 2 billion
  // polygons we give up :-)

  Caps.CanClip = false;
  Caps.minTexHeight = 2;
  Caps.minTexWidth = 2;
  Caps.maxTexHeight = 1024;
  Caps.maxTexWidth = 1024;
  Caps.fog = G3DFOGMETHOD_VERTEX;
  Caps.NeedsPO2Maps = true;
  Caps.MaxAspectRatio = 32768;

  // default extension state is for all extensions to be OFF
  ARB_multitexture = false;
  clipper = NULL;

  // see note above
  tr_verts.IncRef ();
  uv_verts.IncRef ();
  color_verts.IncRef ();
  fog_intensities.IncRef ();
  fog_color_verts.IncRef ();

  in_draw_poly = false;

  // Are we going to use the inverted orthographic projection matrix?
  inverted = false;

}

csGraphics3DOGLCommon::~csGraphics3DOGLCommon ()
{
  if (config)
    config->DecRef();

  Close ();
  if (G2D) G2D->DecRef ();
  if (System) System->DecRef ();

  // see note above
  tr_verts.DecRef ();
  uv_verts.DecRef ();
  color_verts.DecRef ();
  fog_intensities.DecRef ();
  fog_color_verts.DecRef ();
}

bool csGraphics3DOGLCommon::NewInitialize (iSystem * iSys)
{
  (System = iSys)->IncRef ();

  config = System->CreateConfigNew ("/config/opengl.cfg");
  if (!config)
    return false;

  const char *driver = iSys->GetOptionCL ("canvas");
  if (!driver)
    driver = config->GetStr ("Video.OpenGL.Canvas", OPENGL_2D_DRIVER);

  G2D = LOAD_PLUGIN (System, driver, NULL, iGraphics2D);
  if (!G2D)
    return false;

  txtmgr = new csTextureManagerOpenGL (System, G2D, config, this);

  m_renderstate.dither = config->GetBool ("Video.OpenGL.EnableDither", false);
  z_buf_mode = CS_ZBUF_NONE;
  width = height = -1;

  m_renderstate.alphablend = true;
  m_renderstate.mipmap = 0;
  m_renderstate.gouraud = true;
  m_renderstate.lighting = true;
  m_renderstate.textured = true;

  m_config_options.do_multitexture_level = 0;
  m_config_options.do_extra_bright = false;

  return true;
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

bool csGraphics3DOGLCommon::NewOpen (const char *Title)
{
  CommonOpen ();

  if (!G2D->Open (Title))
  {
    SysPrintf (MSG_FATAL_ERROR, "Error opening Graphics2D context.\n");
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
    { NULL, GL_DST_COLOR, GL_ZERO }
  };

  // try to match user's blend name with a name in the blendstyles table
  const char *lightmapstyle = config->GetStr
        ("Video.OpenGL.LightmapMode","multiplydouble");
  int blendstyleindex = 0;
  while (blendstyles[blendstyleindex].blendstylename != NULL)
  {
    if (strcmp (lightmapstyle, blendstyles[blendstyleindex].blendstylename) == 0)
    {
      m_config_options.m_lightmap_src_blend = blendstyles[blendstyleindex].srcblend;
      m_config_options.m_lightmap_dst_blend = blendstyles[blendstyleindex].dstblend;
      break;
    }
    blendstyleindex++;
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
      SysPrintf (MSG_INITIALIZATION, "Using multitexture extension with %d texture units\n", maxtextures);
    }
    else
    {
      SysPrintf (MSG_INITIALIZATION, "WARNING: driver supports multitexture extension but only allows one texture unit!\n");
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
  const unsigned int FOGTABLE_SIZE = 64;
  unsigned char *transientfogdata = new unsigned char[FOGTABLE_SIZE * 4];
  for (unsigned int fogindex = 0; fogindex < FOGTABLE_SIZE; fogindex++)
  {
    transientfogdata[fogindex * 4 + 0] = (unsigned char) 255;
    transientfogdata[fogindex * 4 + 1] = (unsigned char) 255;
    transientfogdata[fogindex * 4 + 2] = (unsigned char) 255;
    double fogalpha =
    (256 * (1.0 - exp (-float (fogindex) * FOGTABLE_MAXDISTANCE / FOGTABLE_SIZE)));
    transientfogdata[fogindex * 4 + 3] = (unsigned char) fogalpha;
  }
  // prevent weird effects when 0 distance fog wraps around to the
  // 'max fog' texel
  transientfogdata[(FOGTABLE_SIZE - 1) * 4 + 3] = 0;

  // dump the fog table into an OpenGL texture for later user.
  // The texture is FOGTABLE_SIZE texels wide and one texel high;
  // we could use a 1D texture but some OpenGL drivers don't
  // handle 1D textures very well
  glGenTextures (1, &m_fogtexturehandle);
  csglBindTexture (GL_TEXTURE_2D, m_fogtexturehandle);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D (GL_TEXTURE_2D, 0, 4, FOGTABLE_SIZE, 1, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, transientfogdata);

  delete [] transientfogdata;

  glGetIntegerv (GL_MAX_TEXTURE_SIZE, &max_texture_size);

  int max_cache_size = 1024*1024*16; // 32mb combined cache
  texture_cache = new OpenGLTextureCache (max_cache_size, 24);
  lightmap_cache = new OpenGLLightmapCache (max_cache_size, 24);
  texture_cache->SetBilinearMapping (config->GetBool
        ("Video.OpenGL.EnableBilinearMap", true));

  GLenum errtest;
  errtest = glGetError ();
  if (errtest != GL_NO_ERROR)
  {
    //SysPrintf (MSG_DEBUG_0, "openGL error string: %s\n", gluErrorString (errtest));
    SysPrintf (MSG_DEBUG_0, "openGL error: %d\n", errtest);
  }

  // if blend style is 'auto' try to determine which mode to use by drawing on the
  // frame buffer.  We check the results to see if the OpenGL driver provides good
  // support for multipledouble (2*SRC*DST) blend mode; if not, fallback to the
  // normal multiply blend mode
  if (strcmp (lightmapstyle, "auto") == 0)
  {
    GLenum srcblend, dstblend;

    Guess_BlendMode (&srcblend, &dstblend);

    m_config_options.m_lightmap_src_blend = srcblend;
    m_config_options.m_lightmap_dst_blend = dstblend;
  }

  end_draw_poly ();

  glCullFace (GL_FRONT);
  glEnable (GL_CULL_FACE);

  // now that we know what pixelformat we use, clue the texture manager in
  txtmgr->SetPixelFormat (*G2D->GetPixelFormat ());

  return true;
}

void csGraphics3DOGLCommon::CommonOpen ()
{
  DrawMode = 0;
  width = G2D->GetWidth ();
  height = G2D->GetHeight ();
  width2 = width / 2;
  height2 = height / 2;
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
  m_config_options.m_lightmap_src_blend = d->m_config_options.m_lightmap_src_blend;
  m_config_options.m_lightmap_dst_blend = d->m_config_options.m_lightmap_dst_blend;
  m_fogtexturehandle = d->m_fogtexturehandle;
  texture_cache = d->texture_cache;
  lightmap_cache = d->lightmap_cache;
}

void csGraphics3DOGLCommon::Close ()
{
  if ((width == height) && height == -1)
    return;

  // we should remove all texture handles before we kill the graphics context
  delete txtmgr; txtmgr = NULL;
  delete texture_cache; texture_cache = NULL;
  delete lightmap_cache; lightmap_cache = NULL;

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
  csGraphics3DOGLCommon::width2 = width / 2;
  csGraphics3DOGLCommon::height2 = height / 2;
}

void csGraphics3DOGLCommon::SetClipper (csVector2* vertices, int num_vertices)
{
  delete clipper;
  clipper = NULL;
  if (!vertices) return;
  // @@@ This could be better! We are using a general polygon clipper
  // even in cases where a box clipper would be better. We should
  // have a special SetBoxClipper call in iGraphics3D.
  clipper = new csPolygonClipper (vertices, num_vertices, false, true);
}

void csGraphics3DOGLCommon::GetClipper (csVector2* vertices, int& num_vertices)
{
  if (!clipper) { num_vertices = 0; return; }
  num_vertices = clipper->GetNumVertices ();
  csVector2* clip_verts = clipper->GetClipPoly ();
  int i;
  for (i = 0 ; i < num_vertices ; i++)
    vertices[i] = clip_verts[i];
}

bool csGraphics3DOGLCommon::BeginDraw (int DrawFlags)
{
  // if 2D graphics is not locked, lock it
  if ((DrawFlags & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
   && (!(DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))))
  {
    if (!G2D->BeginDraw ())
      return false;

    dbg_current_polygon = 0;
  }

  if (DrawFlags & CSDRAW_CLEARZBUFFER)
  {
    if (DrawFlags & CSDRAW_CLEARSCREEN)
      glClear (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    else
      glClear (GL_DEPTH_BUFFER_BIT);
  }
  else if (DrawFlags & CSDRAW_CLEARSCREEN)
    G2D->Clear (0);

  DrawMode = DrawFlags;

  end_draw_poly ();

  return true;
}

void csGraphics3DOGLCommon::FinishDraw ()
{
  // ASSERT( G2D );


  if (DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
  {
    G2D->FinishDraw ();
  }
  DrawMode = 0;
}

void csGraphics3DOGLCommon::Print (csRect * area)
{
  G2D->Print (area);
  // glClear(GL_COLOR_BUFFER_BIT);
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

void csGraphics3DOGLCommon::DrawPolygonSingleTexture (G3DPolygonDP & poly)
{
  if (poly.num < 3)
    return;

  int i;
  float P1, P2, P3, P4;
  float Q1, Q2, Q3, Q4;

  // take shortcut drawing if possible
  if (ShortcutDrawPolygon && (this->*ShortcutDrawPolygon) (poly))
    return;

  // For debugging: is we reach the maximum number of polygons to draw
  // we simply stop.
  dbg_current_polygon++;
  if (dbg_current_polygon == dbg_max_polygons_to_draw - 1)
    return;
  if (dbg_current_polygon >= dbg_max_polygons_to_draw - 1)
    return;

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

  // Get the plane normal of the polygon. Using this we can calculate
  // '1/z' at every screen space point.
  float Ac, Bc, Cc, Dc, inv_Dc;
  Ac = poly.normal.A ();
  Bc = poly.normal.B ();
  Cc = poly.normal.C ();
  Dc = poly.normal.D ();

  float M, N, O;
  float inv_aspect = poly.inv_aspect;
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

  iPolygonTexture *tex = poly.poly_texture;
  iTextureHandle* txt_handle = poly.mat_handle->GetTexture ();
  csTextureHandleOpenGL *txt_mm = (csTextureHandleOpenGL *) txt_handle->GetPrivateObject ();

  // find lightmap information, if any
  iLightMap *thelightmap = tex->GetLightMap ();

  // Initialize our static drawing information and cache
  // the texture in the texture cache (if this is not already the case).
  CacheTexture (tex);

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

  bool tex_transp;
  int poly_alpha = poly.alpha;

  csGLCacheData *texturecache_data;
  texturecache_data = (csGLCacheData *)txt_mm->GetCacheData ();
  tex_transp = txt_mm->GetKeyColor () || txt_mm->GetAlphaMap ();
  GLuint texturehandle = texturecache_data->Handle;

  float flat_r = 1., flat_g = 1., flat_b = 1.;

#if 0
  // @@@ This section uses display lists to do the setup for DrawPolygon.
  // I expected this to be faster but it is not. It is slightly (but not
  // much) slower than the normal stuff. Why?
  static GLuint list = 0, listtxt = 0;
  static GLuint prev_txt = 0;
  static int prev_z_buf_mode = -1;
  if (list == 0)
  {
    // Precompute five display lists for the five possible Z buffer
    // modes.
    list = glGenLists (5);
    glNewList (list, GL_COMPILE);
    glShadeModel (GL_FLAT);
    glEnable (GL_TEXTURE_2D);
    glDisable (GL_DEPTH_TEST);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glDisable (GL_BLEND);
    glColor4f (flat_r, flat_g, flat_b, 0.);
    glEndList ();

    glNewList (list+1, GL_COMPILE);
    glShadeModel (GL_FLAT);
    glEnable (GL_TEXTURE_2D);
    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_ALWAYS);
    glDepthMask (GL_TRUE);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glDisable (GL_BLEND);
    glColor4f (flat_r, flat_g, flat_b, 0.);
    glEndList ();

    glNewList (list+2, GL_COMPILE);
    glShadeModel (GL_FLAT);
    glEnable (GL_TEXTURE_2D);
    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_GREATER);
    glDepthMask (GL_FALSE);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glDisable (GL_BLEND);
    glColor4f (flat_r, flat_g, flat_b, 0.);
    glEndList ();

    glNewList (list+3, GL_COMPILE);
    glShadeModel (GL_FLAT);
    glEnable (GL_TEXTURE_2D);
    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_GREATER);
    glDepthMask (GL_TRUE);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glDisable (GL_BLEND);
    glColor4f (flat_r, flat_g, flat_b, 0.);
    glEndList ();

    listtxt = glGenLists (1);
  }

  if (m_renderstate.textured && (poly_alpha <= 0) && !tex_transp)
  {
    // If we have the most common case (normal lightmapped polygon)
    // we can use a premade display list to execute our setup stuff.
    if (prev_txt != texturehandle || prev_z_buf_mode != z_buf_mode)
    {
      glNewList (listtxt, GL_COMPILE);
      glCallList (list+z_buf_mode);
      glBindTexture (GL_TEXTURE_2D, texturehandle);
      glEndList ();
      prev_txt = texturehandle;
      prev_z_buf_mode = z_buf_mode;
    }
    glCallList (listtxt);
  }
  else
  {
    glShadeModel (GL_FLAT);
    if (m_renderstate.textured)
      glEnable (GL_TEXTURE_2D);
    else
    {
      glDisable (GL_TEXTURE_2D);
      UByte r, g, b;
      poly.mat_handle->GetTexture ()->GetMeanColor (r, g, b);
      flat_r = BYTE_TO_FLOAT (r);
      flat_g = BYTE_TO_FLOAT (g);
      flat_b = BYTE_TO_FLOAT (b);
    }
    SetGLZBufferFlags ();

    if ((poly_alpha > 0) || tex_transp)
    {
      // Transparency.
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glEnable (GL_BLEND);
      if (poly_alpha > 0)
        glColor4f (flat_r, flat_g, flat_b, 1.0 - BYTE_TO_FLOAT (poly_alpha));
      else
        glColor4f (flat_r, flat_g, flat_b, 1.0);
    }
    else
    {
      glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      glDisable (GL_BLEND);
      glColor4f (flat_r, flat_g, flat_b, 0.);
    }

    glBindTexture (GL_TEXTURE_2D, texturehandle);
  }

#else
  // The old code (still faster).
  glShadeModel (GL_FLAT);
  if (m_renderstate.textured)
    glEnable (GL_TEXTURE_2D);
  else
  {
    glDisable (GL_TEXTURE_2D);
    UByte r, g, b;
    poly.mat_handle->GetTexture ()->GetMeanColor (r, g, b);
    flat_r = BYTE_TO_FLOAT (r);
    flat_g = BYTE_TO_FLOAT (g);
    flat_b = BYTE_TO_FLOAT (b);
  }

  SetGLZBufferFlags ();

  if ((poly_alpha > 0) || tex_transp)
  {
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable (GL_BLEND);
    if (poly_alpha > 0)
      glColor4f (flat_r, flat_g, flat_b, 1.0 - BYTE_TO_FLOAT (poly_alpha));
    else
    {
      glColor4f (flat_r, flat_g, flat_b, 1.0);
    }
  }
  else
  {
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glDisable (GL_BLEND);
    glColor4f (flat_r, flat_g, flat_b, 0.);
  }

  csglBindTexture (GL_TEXTURE_2D, texturehandle);
#endif

  float sx, sy, sz, one_over_sz, u_over_sz, v_over_sz;

  // Check if there was a DrawPolygon the previous time.
  start_draw_poly ();

  // First copy all data in an array so that we can minimize
  // the amount of code that goes between glBegin/glEnd. This
  // is from an OpenGL high-performance FAQ.
  // @@@ HARDCODED LIMIT OF 64 VERTICES!
  static GLfloat glverts[4*64];
  static GLfloat gltxt[2*64];
  int vtidx = 0;
  int txtidx = 0;
  for (i = 0; i < poly.num; i++)
  {
    sx = poly.vertices[i].sx - width2;
    sy = poly.vertices[i].sy - height2;
    one_over_sz = M * sx + N * sy + O;
    sz = 1.0 / one_over_sz;
    u_over_sz = (J1 * sx + J2 * sy + J3);
    v_over_sz = (K1 * sx + K2 * sy + K3);
    // we must communicate the perspective correction (1/z) for
    // textures by using homogenous coordinates in either texture
    // space or in object (vertex) space.  We do it in texture
    // space.
    // glTexCoord4f(u_over_sz,v_over_sz,one_over_sz,one_over_sz);
    // glVertex3f(poly.vertices[i].sx, poly.vertices[i].sy,
    // -one_over_sz);

    // modified to use homogenous object space coordinates instead
    // of homogenous texture space coordinates
    gltxt[txtidx++] = u_over_sz * sz;
    gltxt[txtidx++] = v_over_sz * sz;
    glverts[vtidx++] = poly.vertices[i].sx * sz;
    glverts[vtidx++] = poly.vertices[i].sy * sz;
    glverts[vtidx++] = -1.0;
    glverts[vtidx++] = sz;
  }

  GLfloat* p_gltxt, * p_glverts;
  p_gltxt = gltxt;
  p_glverts = glverts;

  glBegin (GL_TRIANGLE_FAN);
  for (i = 0 ; i < poly.num ; i++)
  {
    glTexCoord2fv (p_gltxt); p_gltxt += 2;
    glVertex4fv (p_glverts); p_glverts += 4;
  }
  glEnd ();


  // next draw the lightmap over the texture.  The two are blended
  // together. If a lightmap exists, extract the proper
  // data (GL handle, plus texture coordinate bounds)
  if (thelightmap && m_renderstate.lighting)
  {
    csGLCacheData *lightmapcache_data = (csGLCacheData *)thelightmap->GetCacheData ();
    GLuint lightmaphandle = lightmapcache_data->Handle;

    csglBindTexture (GL_TEXTURE_2D, lightmaphandle);
    glEnable (GL_TEXTURE_2D);

    // Here we set the Z buffer depth function to GL_EQUAL to make
    // sure that the lightmap only overwrites those areas where the
    // Z buffer was updated in the previous pass. This makes sure
    // that  intersecting polygons are properly lighted.
    if (z_buf_mode == CS_ZBUF_FILL)
      glDisable (GL_DEPTH_TEST);
    else
      glDepthFunc (GL_EQUAL);

    glEnable (GL_BLEND);
    // The following blend function is configurable.
    glBlendFunc (m_config_options.m_lightmap_src_blend,
		 m_config_options.m_lightmap_dst_blend);

    txtidx = 0;
    float lm_scale_u = lightmapcache_data->lm_scale_u;
    float lm_scale_v = lightmapcache_data->lm_scale_v;
    float lm_offset_u = lightmapcache_data->lm_offset_u;
    float lm_offset_v = lightmapcache_data->lm_offset_v;
    for (i = 0; i < poly.num; i++)
    {
      gltxt[txtidx] = (gltxt[txtidx] - lm_offset_u) * lm_scale_u;
      txtidx++;
      gltxt[txtidx] = (gltxt[txtidx] - lm_offset_v) * lm_scale_v;
      txtidx++;
    }

    p_gltxt = gltxt;
    p_glverts = glverts;
    glBegin (GL_TRIANGLE_FAN);
    for (i = 0 ; i < poly.num ; i++)
    {
      glTexCoord2fv (p_gltxt); p_gltxt += 2;
      glVertex4fv (p_glverts); p_glverts += 4;
    }
    glEnd ();
  }

  // An extra pass which improves the lighting on SRC*DST so that
  // it looks more like 2*SRC*DST.
  if (m_config_options.do_extra_bright)
  {
    glDisable (GL_TEXTURE_2D);
    if (z_buf_mode == CS_ZBUF_FILL)
      glDisable (GL_DEPTH_TEST);
    else
      glDepthFunc (GL_EQUAL);
    glShadeModel (GL_SMOOTH);
    glEnable (GL_BLEND);
    glBlendFunc (GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
    // glBlendFunc (GL_ZERO, GL_SRC_COLOR);

    p_glverts = glverts;
    glBegin (GL_TRIANGLE_FAN);
    glColor4f (2, 2, 2, 0);
    for (i = 0; i < poly.num; i++)
    {
      glVertex4fv (p_glverts);
      p_glverts += 4;
    }
    glEnd ();
  }
  // If there is vertex fog then we apply that last.
  if (poly.use_fog)
  {
    csglBindTexture (GL_TEXTURE_2D, m_fogtexturehandle);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    if (z_buf_mode == CS_ZBUF_FILL)
      glDisable (GL_DEPTH_TEST);
    else
      glDepthFunc (GL_EQUAL);
    glShadeModel (GL_SMOOTH);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    p_glverts = glverts;
    glBegin (GL_TRIANGLE_FAN);
    for (i = 0; i < poly.num; i++)
    {
      // Formula for fog is:
      // C = I * F + (1-I) * P
      // I = intensity = density * thickness
      // F = fog color
      // P = texture color
      // C = destination color

      // specify fog vertex color
      glColor3f (poly.fog_info[i].r, poly.fog_info[i].g, poly.fog_info[i].b);

      // specify fog vertex alpha value using the fog table and fog
      // distance
      if (poly.fog_info[i].intensity > FOG_MAXVALUE)
	glTexCoord2f (FOGTABLE_CLAMPVALUE,0.0);
      else
	glTexCoord2f (poly.fog_info[i].intensity / FOGTABLE_MAXDISTANCE, 0.0);

      // specify fog vertex location
      glVertex4fv (p_glverts); p_glverts += 4;
    }
    glEnd ();

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

  // Get the plane normal of the polygon. Using this we can calculate
  // '1/z' at every screen space point.
  float Ac, Bc, Cc, Dc, inv_Dc;
  Ac = poly.normal.A ();
  Bc = poly.normal.B ();
  Cc = poly.normal.C ();
  Dc = poly.normal.D ();

  float M, N, O;
  float inv_aspect = poly.inv_aspect;
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
  SetGLZBufferFlags ();
  glBlendFunc (GL_ZERO, GL_ONE);
  glEnable (GL_BLEND);

  // First copy all data in an array so that we can minimize
  // the amount of code that goes between glBegin/glEnd. This
  // is from an OpenGL high-performance FAQ.
  // @@@ HARDCODED LIMIT OF 64 VERTICES!
  static GLfloat glverts[4*64];
  int vtidx = 0;
  float sx, sy, sz, one_over_sz;
  for (i = 0; i < poly.num; i++)
  {
    sx = poly.vertices[i].sx - width2;
    sy = poly.vertices[i].sy - height2;
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

// Calculate round (f) of a 16:16 fixed pointer number
// and return a long integer.
inline long
round16 (long f)
{
  return (f + 0x8000) >> 16;
}

//static iTextureHandle* prev_handle = NULL;
//static UInt prev_mode = ~0;

void csGraphics3DOGLCommon::StartPolygonFX (iMaterialHandle * handle, UInt mode)
{
  end_draw_poly ();

//@@@ Experimental!!!
// If the following code is enabled then speed goes up considerably in some
// cases. However this doesn't work very well. We need a global state mechanism
// because other functions may mess up the state as well.
//  if (handle == prev_handle && mode == prev_mode) return;
//  prev_handle = handle;
//  prev_mode = mode;


  // try shortcut method if available
  if (ShortcutStartPolygonFX && (this ->* ShortcutStartPolygonFX) (handle, mode))
    return;

  m_gouraud = m_renderstate.lighting && m_renderstate.gouraud && ((mode & CS_FX_GOURAUD) != 0);
  m_mixmode = mode;
  m_alpha = 1.0f - BYTE_TO_FLOAT (mode & CS_FX_MASK_ALPHA);

  GLuint texturehandle = 0;
  bool has_alpha = false;
  if (handle && m_renderstate.textured)
  {
    iTextureHandle* txt_handle = handle->GetTexture ();
    csTextureHandleOpenGL *txt_mm = (csTextureHandleOpenGL *) txt_handle->GetPrivateObject ();
    texture_cache->cache_texture (txt_handle);
    csGLCacheData *cachedata = (csGLCacheData *)txt_mm->GetCacheData ();
    texturehandle = cachedata->Handle;
    if (txt_handle)
      has_alpha = txt_handle->GetKeyColor () || txt_handle->GetAlphaMap ();
  }
  if ((mode & CS_FX_MASK_MIXMODE) == CS_FX_ALPHA)
    m_gouraud = true;

  // set proper shading: flat is typically faster, but we need smoothing
  // enabled when doing gouraud shading -GJH
  if (m_gouraud)
    glShadeModel (GL_SMOOTH);
  else
    glShadeModel (GL_FLAT);

  // Note: In all explanations of Mixing:
  // Color: resulting color
  // SRC:   Color of the texel (content of the texture to be drawn)
  // DEST:  Color of the pixel on screen
  // Alpha: Alpha value of the polygon
  bool enable_blending = true;
  switch (mode & CS_FX_MASK_MIXMODE)
  {
    case CS_FX_MULTIPLY:
      // Color = SRC * DEST +   0 * SRC = DEST * SRC
      m_alpha = 1.0f;
      glBlendFunc (GL_ZERO, GL_SRC_COLOR);
      glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
      break;
    case CS_FX_MULTIPLY2:
      // Color = SRC * DEST + DEST * SRC = 2 * DEST * SRC
      m_alpha = 1.0f;
      glBlendFunc (GL_DST_COLOR, GL_SRC_COLOR);
      glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      break;
    case CS_FX_ADD:
      // Color = 1 * DEST + 1 * SRC = DEST + SRC
      m_alpha = 1.0f;
      glBlendFunc (GL_ONE, GL_ONE);
      glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      break;
    case CS_FX_ALPHA:
      // Color = Alpha * DEST + (1-Alpha) * SRC
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      break;
    case CS_FX_TRANSPARENT:
      // Color = 1 * DEST + 0 * SRC
      m_alpha = 0.0f;
      glBlendFunc (GL_ZERO, GL_ONE);
      glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      break;
    case CS_FX_COPY:
    default:
      enable_blending = has_alpha;
      // Color = 0 * DEST + 1 * SRC = SRC
      m_alpha = 1.0f;
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      break;
  }

  m_textured = (texturehandle != 0);
  if (m_textured)
  {
    csglBindTexture (GL_TEXTURE_2D, texturehandle);
    glEnable (GL_TEXTURE_2D);
  }
  else
    glDisable (GL_TEXTURE_2D);

  if (enable_blending)
    glEnable (GL_BLEND);
  else
    glDisable (GL_BLEND);

  SetGLZBufferFlags ();
}

void csGraphics3DOGLCommon::FinishPolygonFX ()
{
  // attempt shortcut method
  if (ShortcutFinishPolygonFX && (this ->* ShortcutFinishPolygonFX) ())
    return;
}

void csGraphics3DOGLCommon::DrawPolygonFX (G3DPolygonDPFX & poly)
{
  // take shortcut if possible
  if (ShortcutDrawPolygonFX && (this ->* ShortcutDrawPolygonFX) (poly))
    return;

  float flat_r = 1., flat_g = 1., flat_b = 1.;
  if (!m_textured)
  {
    flat_r = BYTE_TO_FLOAT (poly.flat_color_r);
    flat_g = BYTE_TO_FLOAT (poly.flat_color_g);
    flat_b = BYTE_TO_FLOAT (poly.flat_color_b);
  }
  glBegin (GL_TRIANGLE_FAN);
  float sx, sy;
  int i;
  for (i = 0; i < poly.num; i++)
  {
    sx = poly.vertices[i].sx - width2;
    sy = poly.vertices[i].sy - height2;
    glTexCoord2f (poly.vertices[i].u, poly.vertices[i].v);
    if (m_gouraud)
      glColor4f (flat_r * poly.vertices[i].r, flat_g * poly.vertices[i].g,
		 flat_b * poly.vertices[i].b, m_alpha);
    else
      glColor4f (flat_r, flat_g, flat_b, m_alpha);
#   if 1
    // Patch from Thomas Krug to enable perspective correct texture
    // mapping for DrawPolygonFX. I don't enable this by default yet
    // because it is a slow-down. I need to think about how to avoid
    // the divide if at all possible.
    // Maybe this should be optional.
    float sz = poly.vertices[i].z;
    if (ABS (sz) < SMALL_EPSILON) sz = 1.0/ SMALL_EPSILON;
    else sz = 1./sz;
    glVertex4f (poly.vertices[i].sx * sz, poly.vertices[i].sy * sz, -1, sz);
#   else
    glVertex3f (poly.vertices[i].sx, poly.vertices[i].sy, -poly.vertices[i].z);
#   endif
  }
  glEnd ();

  // If there is vertex fog then we apply that last.
  if (poly.use_fog)
  {
    // let OpenGL save old values for us
    // -GL_COLOR_BUFFER_BIT saves blend types among other things,
    // -GL_DEPTH_BUFFER_BIT saves depth test function
    // -GL_TEXTURE_BIT saves current texture handle
    glPushAttrib ( GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_TEXTURE_BIT );

    // we need to texture and blend, without vertex color
    // interpolation
    glEnable (GL_TEXTURE_2D);
    csglBindTexture (GL_TEXTURE_2D, m_fogtexturehandle);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable (GL_BLEND);
    glDepthFunc (GL_EQUAL);
    glShadeModel (GL_SMOOTH);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin (GL_TRIANGLE_FAN);
    for (i = 0; i < poly.num; i++)
    {
      sx = poly.vertices[i].sx - width2;
      sy = poly.vertices[i].sy - height2;

      // Formula for fog is:
      // C = I * F + (1-I) * P
      // I = intensity = 1-exp(density * thickness)
      // F = fog color
      // P = texture color
      // C = destination color
      // the '1-exp' part is embodied in the fog texture; view the
      // fog texture as a function producing the alpha value, with
      // input (texture coordinate) being equal to I.

      // specify fog vertex color
      glColor3f (poly.fog_info[i].r, poly.fog_info[i].g, poly.fog_info[i].b);

      // specify fog vertex transparency
      float I = poly.fog_info[i].intensity;
      if (I > FOG_MAXVALUE)
	glTexCoord1f (FOGTABLE_CLAMPVALUE);
      else
	glTexCoord1f (I / FOGTABLE_MAXDISTANCE);

      // specify fog vertex location
      glVertex3f (poly.vertices[i].sx, poly.vertices[i].sy, -poly.vertices[i].z);
    }
    glEnd ();

    // Restore state (blend mode, texture handle, etc.) for next
    // triangle
    glPopAttrib ();
    if (!m_textured)
      glDisable (GL_TEXTURE_2D);
    if (!m_gouraud)
      glShadeModel (GL_FLAT);
  }
}

void csGraphics3DOGLCommon::DrawTriangleMesh (G3DTriangleMesh& mesh)
{
  end_draw_poly ();

  // yet another work in progress - GJH

  // fallback to "default" implementation if something comes up that we can't
  // handle.  This includes software clipping.
  if (mesh.do_clip && clipper)
  {
  ///  SysPrintf(MSG_DEBUG_0, "Falling back to software clipping in DrawTriangleMesh\n");
    DefaultDrawTriangleMesh (mesh, this, o2c, clipper, aspect, width2, height2);
    return;
  }

  int i,k;

  // Update work tables.
  if (mesh.num_vertices > tr_verts.Limit ())
  {
    tr_verts.SetLimit (mesh.num_vertices);
    uv_verts.SetLimit (mesh.num_vertices);
    rgba_verts.SetLimit (mesh.num_vertices*4);
    color_verts.SetLimit (mesh.num_vertices);
    fog_intensities.SetLimit (mesh.num_vertices);
    fog_color_verts.SetLimit (mesh.num_vertices);
  }

  // Do vertex tweening and/or transformation to camera space
  // if any of those are needed. When this is done 'verts' will
  // point to an array of camera vertices.
  csVector3* f1 = mesh.vertices[0];
  csVector2* uv1 = mesh.texels[0][0];
  csColor* col1 = mesh.vertex_colors[0];
  csVector3* work_verts;
  csVector2* work_uv_verts;
  csColor* work_colors;

  if (mesh.num_vertices_pool > 1)
  {
    // Vertex morphing.
    float tween_ratio = mesh.morph_factor;
    float remainder = 1 - tween_ratio;
    csVector3* f2 = mesh.vertices[1];
    csVector2* uv2 = mesh.texels[1][0];
    csColor* col2 = mesh.vertex_colors[1];
    for (i = 0 ; i < mesh.num_vertices ; i++)
    {
      tr_verts[i] = tween_ratio * f2[i] + remainder * f1[i];
      if (mesh.do_morph_texels)
        uv_verts[i] = tween_ratio * uv2[i] + remainder * uv1[i];
      if (mesh.do_morph_colors)
      {
        color_verts[i].red = tween_ratio * col2[i].red + remainder * col1[i].red;
	color_verts[i].green = tween_ratio * col2[i].green + remainder * col1[i].green;
	color_verts[i].blue = tween_ratio * col2[i].blue + remainder * col1[i].blue;
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

  // set up coordinate transform
  GLfloat matrixholder[16];

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // set up world->camera transform, if needed
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

    glMultMatrixf(matrixholder);

    glTranslatef(-translation.x, -translation.y, -translation.z);
  }

  // Set up perspective transform.
  // we have to change the standard projection matrix used for
  // drawing in other parts of CS.  Normally an orthogonal projection
  // is used since CS does the perspective projection for us.
  // Here, we need to reproduce CS's perspective projection using
  // OpenGL matrices.

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();

  // With the back buffer procedural textures the orthographic projection
  // matrix are inverted.
  if (inverted)
    glOrtho (0., (GLdouble) width, (GLdouble) height, 0., -1.0, 10.0);
  else
    glOrtho (0., (GLdouble) width, 0., (GLdouble) height, -1.0, 10.0);


  glTranslatef(width2,height2,0);

  for (i = 0 ; i < 16 ; i++)
    matrixholder[i] = 0.0;

  matrixholder[0] = matrixholder[5] = 1.0;

  matrixholder[11] = +1.0/aspect;
  matrixholder[14] = -matrixholder[11];

  glMultMatrixf(matrixholder);

  if (mesh.do_fog)
  {
    for (i=0; i < mesh.num_vertices; i++)
    {
      // specify fog vertex transparency
      float I = mesh.vertex_fog[i].intensity;
      if (I > FOG_MAXVALUE)
      	I = FOGTABLE_CLAMPVALUE;
      else
	I = I * FOGTABLE_DISTANCESCALE;

      fog_intensities[i] = I;
      // @@@ To avoid this copy we better structure this differently
      // inside G3DTriangleMesh.
      fog_color_verts[i].red = mesh.vertex_fog[i].r;
      fog_color_verts[i].green = mesh.vertex_fog[i].g;
      fog_color_verts[i].blue = mesh.vertex_fog[i].b;
    }
  }

  csTriangle *triangles = mesh.triangles;

  // Draw all triangles.
  StartPolygonFX (mesh.mat_handle[0], mesh.fxmode);

  if (m_gouraud && work_colors)
  {
    glEnableClientState (GL_COLOR_ARRAY);

    // special hack for transparent meshes
    if (mesh.fxmode & CS_FX_ALPHA)
    {
      for (k=0, i=0; i<mesh.num_vertices; i++)
      {
        rgba_verts[k++] = work_colors[i].red;
        rgba_verts[k++] = work_colors[i].green;
        rgba_verts[k++] = work_colors[i].blue;
	rgba_verts[k++] = m_alpha;
      }
      glColorPointer (4, GL_FLOAT, 0, &rgba_verts[0]);
    }
    else
    {
      glColorPointer (3, GL_FLOAT, 0, & work_colors[0]);
    }
  }
  else
  {
    float flat_r = 1., flat_g = 1., flat_b = 1.;
    if (!m_textured)
    {
      // Fill flat color if renderer decide to paint it flat-shaded
      UByte r,g,b;
      mesh.mat_handle[0]->GetTexture ()->GetMeanColor (r, g, b);
      flat_r = BYTE_TO_FLOAT (r);
      flat_g = BYTE_TO_FLOAT (g);
      flat_b = BYTE_TO_FLOAT (b);
    }
    glColor4f (flat_r, flat_g, flat_b, m_alpha);
  }

  glEnableClientState (GL_VERTEX_ARRAY);
  glEnableClientState (GL_TEXTURE_COORD_ARRAY);
  glVertexPointer (3, GL_FLOAT, 0, & work_verts[0]);
  glTexCoordPointer (2, GL_FLOAT, 0, & work_uv_verts[0]);
  glDrawElements (GL_TRIANGLES, mesh.num_triangles*3 , GL_UNSIGNED_INT, & triangles[0]);

  // If there is vertex fog then we apply that last.
  if (mesh.do_fog)
  {
    // let OpenGL save old values for us
    // -GL_COLOR_BUFFER_BIT saves blend types among other things,
    // -GL_DEPTH_BUFFER_BIT saves depth test function
    // -GL_TEXTURE_BIT saves current texture handle
    glPushAttrib (GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_TEXTURE_BIT);

    // we need to texture and blend, with vertex color
    // interpolation
    glEnable (GL_TEXTURE_2D);
    csglBindTexture (GL_TEXTURE_2D, m_fogtexturehandle);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDepthFunc (GL_EQUAL);
    glShadeModel (GL_SMOOTH);

    glTexCoordPointer (1, GL_FLOAT, 0, & fog_intensities[0]);
    glColorPointer (3, GL_FLOAT, 0, & fog_color_verts[0]);

    glDrawElements (GL_TRIANGLES, mesh.num_triangles*3 , GL_UNSIGNED_INT, & triangles[0]);

    // Restore state (blend mode, texture handle, etc.) for next
    // triangle
    glPopAttrib ();
    if (!m_textured)
      glDisable (GL_TEXTURE_2D);
    if (!m_gouraud)
      glShadeModel (GL_FLAT);
  }

  if (m_gouraud && work_colors)
  {
    glDisableClientState (GL_COLOR_ARRAY);
  }

  glDisableClientState (GL_VERTEX_ARRAY);
  glDisableClientState (GL_TEXTURE_COORD_ARRAY);

  FinishPolygonFX ();

  glMatrixMode (GL_MODELVIEW);
  glPopMatrix ();
  glMatrixMode (GL_PROJECTION);
  glPopMatrix ();
}



void csGraphics3DOGLCommon::OpenFogObject (CS_ID /*id*/, csFog* /*fog*/)
{
  // OpenGL driver implements vertex-based fog ...
}

void csGraphics3DOGLCommon::DrawFogPolygon (CS_ID /*id*/, G3DPolygonDFP &/*poly*/, int /*fogtype*/)
{
  // OpenGL driver implements vertex-based fog ...
}

void csGraphics3DOGLCommon::CloseFogObject (CS_ID /*id*/)
{
  // OpenGL driver implements vertex-based fog ...
}

void csGraphics3DOGLCommon::CacheTexture (iPolygonTexture *texture)
{
  iTextureHandle* txt_handle = texture->GetMaterialHandle ()->GetTexture ();
  texture_cache->cache_texture (txt_handle);
  if (m_renderstate.lighting)
    lightmap_cache->cache_lightmap (texture);
}

bool csGraphics3DOGLCommon::SetRenderState (G3D_RENDERSTATEOPTION op, long value)
{
  //@@@end_draw_poly ();

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
      dbg_max_polygons_to_draw = value;
      if (dbg_max_polygons_to_draw < 0)
        dbg_max_polygons_to_draw = 0;
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
      return dbg_max_polygons_to_draw;
    default:
      return 0;
  }
}

void csGraphics3DOGLCommon::ClearCache ()
{
  end_draw_poly ();

  // We will clear lightmap cache since when unloading a world lightmaps
  // become invalid. We won't clear texture cache since texture items are
  // cleaned up individually when an iTextureHandle's RefCount reaches zero.
  if (lightmap_cache) lightmap_cache->Clear ();
}

void csGraphics3DOGLCommon::DumpCache ()
{
}

void csGraphics3DOGLCommon::DrawLine (const csVector3 & v1, const csVector3 & v2,
	float fov, int color)
{
  end_draw_poly ();

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

void csGraphics3DOGLCommon::SetGLZBufferFlags ()
{
  switch (z_buf_mode)
  {
    case CS_ZBUF_NONE:
      glDisable (GL_DEPTH_TEST);
      break;
    case CS_ZBUF_FILL:
    case CS_ZBUF_FILLONLY:
      glEnable (GL_DEPTH_TEST);
      glDepthFunc (GL_ALWAYS);
      glDepthMask (GL_TRUE);
      break;
    case CS_ZBUF_TEST:
      glEnable (GL_DEPTH_TEST);
      glDepthFunc (GL_GREATER);
      glDepthMask (GL_FALSE);
      break;
    case CS_ZBUF_USE:
      glEnable (GL_DEPTH_TEST);
      glDepthFunc (GL_GREATER);
      glDepthMask (GL_TRUE);
      break;
    default:
      break;
  }
}

// Shortcut to override standard polygon drawing when we have
// multitexture
bool csGraphics3DOGLCommon::DrawPolygonMultiTexture (G3DPolygonDP & poly)
{
  end_draw_poly ();//@@@

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

  // the shortcut works only if there is a lightmap and no fog
  if (!thelightmap || poly.use_fog)
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

  float inv_aspect = poly.inv_aspect;
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
  int poly_alpha = poly.alpha;

  csGLCacheData *texturecache_data;
  texturecache_data = (csGLCacheData *)txt_mm->GetCacheData ();
  tex_transp = txt_mm->GetKeyColor ();
  GLuint texturehandle = texturecache_data->Handle;

  // configure base texture for texure unit 0
  float flat_r = 1.0, flat_g = 1.0, flat_b = 1.0;
  glActiveTextureARB (GL_TEXTURE0_ARB);
  csglBindTexture (GL_TEXTURE_2D, texturehandle);
  if ((poly_alpha > 0) || tex_transp)
  {
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable (GL_BLEND);
    if (poly_alpha > 0)
      glColor4f (flat_r, flat_g, flat_b, 1.0 - BYTE_TO_FLOAT (poly_alpha));
    else
      glColor4f (flat_r, flat_g, flat_b, 1.0);
  }
  else
  {
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glDisable (GL_BLEND);
    glColor4f (flat_r, flat_g, flat_b, 0.);
  }

  csGLCacheData *lightmapcache_data = (csGLCacheData *)thelightmap->GetCacheData ();
  GLuint lightmaphandle = lightmapcache_data->Handle;

  // configure lightmap for texture unit 1
  glActiveTextureARB (GL_TEXTURE1_ARB);
  glEnable (GL_TEXTURE_2D);
  csglBindTexture (GL_TEXTURE_2D, lightmaphandle);
  glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  SetGLZBufferFlags ();

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
    sx = poly.vertices[i].sx - width2;
    sy = poly.vertices[i].sy - height2;
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
  // 0.090909=1/11, that is 1 divided by total depth delta set by
  // glOrtho. Where 0.090834 comes from, I don't know
  return (0.090834 / (zvalue - (0.090909)));
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
    DrawPolygonMultiTexture (poly);
    return;
  }
#endif
  DrawPolygonSingleTexture (poly);
}

void csGraphics3DOGLCommon::DrawPixmap (iTextureHandle *hTex,
  int sx, int sy, int sw, int sh, int tx, int ty, int tw, int th, uint8 Alpha)
{
  end_draw_poly ();

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
  texture_cache->cache_texture (hTex);

  // Get texture handle
  csTextureHandleOpenGL *txt_mm = (csTextureHandleOpenGL *)hTex->GetPrivateObject ();
  GLuint texturehandle = ((csGLCacheData *)txt_mm->GetCacheData ())->Handle;

  // as we are drawing in 2D, we disable some of the commonly used features
  // for fancy 3D drawing
  glShadeModel (GL_FLAT);
  glDisable (GL_DEPTH_TEST);
  glDepthMask (GL_FALSE);

  // if the texture has transparent bits, we have to tweak the
  // OpenGL blend mode so that it handles the transparent pixels correctly
  if (hTex->GetKeyColor () || hTex->GetAlphaMap () || Alpha)
  {
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
  else
    glDisable (GL_BLEND);

  glEnable (GL_TEXTURE_2D);
  glColor4f (1.0, 1.0, 1.0, Alpha ? (1.0 - BYTE_TO_FLOAT (Alpha)) : 1.0);
  csglBindTexture (GL_TEXTURE_2D, texturehandle);

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
 * the 2*SRC*DST blend mode--some beta/debug drivers support it badly, some hardware
 * does not support it at all.
 *
 * We check the driver by drawing a polygon with both blend modes:
 *   - we draw using SRC*DST blending and read back the color result, called A
 *   - we draw using 2*SRC*DST blending and read back the color result, called B
 *
 * Ideally B=2*A.  Here we guess that if B > 1.5*A then the 2*SRC*DST mode is
 * reasonably well supported and suggest using 2*SRC*DST mode.  Otherwise we
 * suggest using SRC*DST mode which is pretty well supported.
 */
void csGraphics3DOGLCommon::Guess_BlendMode(GLenum *src, GLenum*dst)
{
  // colors of the 2 polys to blend
  float testcolor1[3] = {0.5,0.5,0.5};
  float testcolor2[3] = {0.5,0.5,0.5};

  // these will hold the resultant color intensities
  float blendresult1[3], blendresult2[3];

  SysPrintf(MSG_INITIALIZATION, "Attempting to determine best blending mode to use.\n");

  // draw the polys
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  glShadeModel(GL_FLAT);

  // blend mode one

  glDisable(GL_BLEND);
  glColor3fv(testcolor1);
  glBegin(GL_QUADS);
  glVertex2i(0,0); glVertex2i(5,0); glVertex2i(5,5); glVertex2i(0,5);
  glEnd();

  glEnable(GL_BLEND);
  glBlendFunc(GL_DST_COLOR, GL_ZERO);
  glColor3fv(testcolor2);
  glBegin(GL_QUADS);
  glVertex2i(0,0); glVertex2i(5,0); glVertex2i(5,5); glVertex2i(0,5);
  glEnd();

  glReadPixels(0,0,1,1,GL_RGB,GL_FLOAT, &blendresult1);

  // blend mode two

  glDisable(GL_BLEND);
  glColor3fv(testcolor1);
  glBegin(GL_QUADS);
  glVertex2i(0,0); glVertex2i(5,0); glVertex2i(5,5); glVertex2i(0,5);
  glEnd();

  glEnable(GL_BLEND);
  glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
  glColor3fv(testcolor2);
  glBegin(GL_QUADS);
  glVertex2i(0,0); glVertex2i(5,0); glVertex2i(5,5); glVertex2i(0,5);
  glEnd();

  glReadPixels(0,0,1,1,GL_RGB,GL_FLOAT, &blendresult2);

  SysPrintf(MSG_INITIALIZATION, "Blend mode values are %f and %f...", blendresult1[1], blendresult2[1]);

  // compare the green component between the two results, A and B.  In the
  // ideal case B = 2*A.  If SRC*DST blend mode is supported but 2*SRC*DST is
  // not, then B = A.  So we guess that if B > 1.5*A that the 2*SRC*DST is
  // 'pretty well' supported and go with that.  Otherwise, fall back on the
  // normal SRC*DST mode.

  float resultA = blendresult1[1];
  float resultB = blendresult2[1];

  if (resultB > 1.5 * resultA)
  {
    SysPrintf(MSG_INITIALIZATION, "using 'multiplydouble' blend mode.\n");

    *src = GL_DST_COLOR;
    *dst = GL_SRC_COLOR;
  }
  else
  {
    SysPrintf(MSG_INITIALIZATION, "using 'multiply' blend mode.\n");

    *src = GL_DST_COLOR;
    *dst = GL_ZERO;
  }
}
