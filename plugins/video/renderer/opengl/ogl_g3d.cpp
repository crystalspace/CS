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
#ifdef OS_WIN32
#include <windows.h>
#endif
#include "qint.h"
#include "csutil/scf.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "cs3d/opengl/ogl_g3d.h"
#include "cs3d/opengl/ogl_txtcache.h"
#include "csutil/inifile.h"
#include "isystem.h"
#include "igraph3d.h"
#include "itxtmgr.h"
#include "itexture.h"
#include "ipolygon.h"
#include "icamera.h"
#include "ilghtmap.h"
#include "igraph2d.h"

#include <GL/gl.h>
#include <GL/glu.h>

// uncomment the 'USE_MULTITEXTURE 1' define to enable compile-time tests for
// multitexture support
#define USE_MULTITEXTURE 0
#ifdef GL_ARB_multitexture
//#define USE_MULTITEXTURE 1
#endif

#if USE_MULTITEXTURE
// function to check for a certain extension - stolen from
// an example by Mark Kilgard -GJH
static int isExtensionSupported(const char * extension);
#endif
//---------------------------------------------------------------------------

// fog variables
// size of the fog texture
static const unsigned int FOGTABLE_SIZE=64;

// each texel in the fog table holds the fog alpha value at a certain
// (distance*density).  The median distance parameter determines the
// (distance*density) value represented by the texel at the center of
// the fog table.  The fog calculation is:
// alpha = 1.0 - exp( -(density*distance) / FOGTABLE_MEDIANDISTANCE)
//
static const double FOGTABLE_MEDIANDISTANCE=10.0;
static const double FOGTABLE_MAXDISTANCE = FOGTABLE_MEDIANDISTANCE * 2.0;

// fog (distance*density) is mapped to a texture coordinate and then
// clamped.  This determines the clamp value.  Some OpenGL drivers don't
// like clamping textures so we must do it ourself
static const double FOGTABLE_CLAMPVALUE = 0.95;

IMPLEMENT_FACTORY (csGraphics3DOpenGL)

EXPORT_CLASS_TABLE (gl3d)
  EXPORT_CLASS (csGraphics3DOpenGL, "crystalspace.graphics3d.opengl",
    "OpenGL 3D graphics driver for Crystal Space")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE (csGraphics3DOpenGL)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics3D)
IMPLEMENT_IBASE_END

csGraphics3DOpenGL::csGraphics3DOpenGL (iBase *iParent) : G2D (NULL), System (NULL)
{
  CONSTRUCT_IBASE (iParent);

  ShortcutDrawPolygon = 0;
  ShortcutStartPolygonFX = 0;
  ShortcutDrawPolygonFX = 0;
  ShortcutFinishPolygonFX = 0;

  dbg_max_polygons_to_draw = 2000000000;        // After 2 billion polygons we give up :-)

  texture_cache = NULL;
  lightmap_cache = NULL;
  txtmgr = NULL;
  m_fogtexturehandle = 0;
  dbg_max_polygons_to_draw = 2000000000;        // After 2 billion polygons we give up :-)
  config = new csIniFile ("opengl.cfg");
}

csGraphics3DOpenGL::~csGraphics3DOpenGL ()
{
  CHK (delete config);

  Close ();
  if (G2D)
    G2D->DecRef ();

  if (System)
    System->DecRef ();
}

bool csGraphics3DOpenGL::Initialize (iSystem *iSys)
{
  (System = iSys)->IncRef ();

  if (!System->RegisterDriver ("iGraphics3D", this))
    return false;

  G2D = LOAD_PLUGIN (System, OPENGL_2D_DRIVER, iGraphics2D);
  if (!G2D)
    return 0;

  CHK (txtmgr = new csTextureManagerOpenGL (System, G2D));
  txtmgr->SetConfig (config);
  txtmgr->Initialize ();

  m_renderstate.dither = config->GetYesNo ("OpenGL", "ENABLE_DITHER", false);
  z_buf_mode = CS_ZBUF_NONE;
  width = height = -1;

  m_renderstate.alphablend = true;
  m_renderstate.mipmap = 0;
  m_renderstate.gouraud = true;
  m_renderstate.lighting = true;
  m_renderstate.textured = true;
  m_renderstate.texel_filt = false;
  m_renderstate.perfect = false;

  m_config_options.do_multitexture_level = 0;
  m_config_options.do_extra_bright = false;

//  CHK (txtmgr = new csTextureManagerOpenGL (System, G2D));

  return true;
}

bool csGraphics3DOpenGL::Open (const char *Title)
{
  DrawMode = 0;

  if (!G2D->Open (Title))
  {
    SysPrintf (MSG_FATAL_ERROR, "Error opening Graphics2D context.");
    // set "not opened" flag
    width = height = -1;
    return false;
  }

  bool bFullScreen = G2D->GetFullScreen ();
  width = G2D->GetWidth ();
  height = G2D->GetHeight ();
  width2 = width / 2;
  height2 = height / 2;
  SetDimensions (width, height);

  SysPrintf (MSG_INITIALIZATION, "Using %s mode at resolution %dx%d.\n",
             bFullScreen ? "full screen" : "windowed", width, height);

  //csPixelFormat pfmt = *G2D->GetPixelFormat ();

  if (m_renderstate.dither)
    glEnable (GL_DITHER);
  else
    glDisable (GL_DITHER);

  if (config->GetYesNo ("OpenGL", "HINT_PERSPECTIVE_FAST", false))
    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
  else
    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  m_config_options.do_extra_bright = config->GetYesNo ("OpenGL", "EXTRA_BRIGHT", false);

  // determine what blend mode to use when combining lightmaps with their
  // underlying textures.  This mode is set in the Opengl configuration file
  struct {
     char *blendstylename;
     GLenum srcblend, dstblend;
  } blendstyles[] = {{ "multiplydouble", GL_DST_COLOR, GL_SRC_COLOR},
                     { "multiply"      , GL_DST_COLOR, GL_ZERO     },
                     { "add"           , GL_ONE      , GL_ONE      },
                     { "coloradd"      , GL_ONE      , GL_SRC_COLOR},
                     { NULL            , GL_DST_COLOR, GL_ZERO     }
		    };
  // default lightmap blend style
  m_config_options.m_lightmap_src_blend = GL_DST_COLOR;
  m_config_options.m_lightmap_dst_blend = GL_ZERO;

  // try to match user's blend name with a name in the blendstyles table
  char *lightmapstyle = config->GetStr("OpenGL", "LIGHTMAP_MODE","multiplydouble");
  int blendstyleindex = 0;
  while (blendstyles[blendstyleindex].blendstylename != NULL)
  {
    if (strcmp(lightmapstyle, blendstyles[blendstyleindex].blendstylename) == 0)
    {
      m_config_options.m_lightmap_src_blend = blendstyles[blendstyleindex].srcblend;
      m_config_options.m_lightmap_dst_blend = blendstyles[blendstyleindex].dstblend;
      break;
    }
    blendstyleindex++;
  }

#if USE_MULTITEXTURE
  // check with the GL driver and see if it supports the multitexure
  // extension
  if (isExtensionSupported("GL_ARB_multitexture"))
  {
    // if you support multitexture, you should allow more than one
    // texture, right?  Let's see how many we can get...
    GLint maxtextures;
    glGetIntegerv (GL_MAX_TEXTURE_UNITS_ARB, &maxtextures);

    if (maxtextures > 1)
    {
      m_config_options.do_multitexture_level = maxtextures;
      ShortcutDrawPolygon = &MultitextureDrawPolygon;
      SysPrintf (MSG_INITIALIZATION, "Using multitexture extension; using %d texture units",maxtextures);
    }
    else
    {
      SysPrintf (MSG_INITIALIZATION, "WARNING: driver supports multitexture extension but only allows one texture!\n");
    }
  }
#endif

  // need some good way of determining a good texture map size and
  // bit depth for the texture 'cache', since GL hides most of the details
  // from us.  Maybe we should just not worry about it... GJH

  CHK (texture_cache = new OpenGLTextureCache(1<<24,24));
  CHK (lightmap_cache = new OpenGLLightmapCache(1<<24,24));
  texture_cache->SetBilinearMapping (config->GetYesNo ("OpenGL", "ENABLE_BILINEARMAP", true));

  // tells OpenGL driver we align texture data on byte boundaries, instead
  // of perhaps word or dword boundaries
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

  // generate the exponential 1D texture for use in vertex fogging
  // this texture holds a 'table' of alpha values forming an exponential
  // curve, to use for generating exponential fog.
  const unsigned int FOGTABLE_SIZE = 64;
  unsigned char *transientfogdata = new unsigned char[FOGTABLE_SIZE*4];
  for (unsigned int fogindex = 0; fogindex < FOGTABLE_SIZE; fogindex++)
  {
    transientfogdata[fogindex*4+0] = (unsigned char) 255;
    transientfogdata[fogindex*4+1] = (unsigned char) 255;
    transientfogdata[fogindex*4+2] = (unsigned char) 255;
    double fogalpha =
    	(256 * ( 1.0 - exp ( - float(fogindex) *FOGTABLE_MAXDISTANCE / FOGTABLE_SIZE ) ) );
    transientfogdata[fogindex*4+3] = (unsigned char) fogalpha;
  }
  // prevent weird effects when 0 distance fog wraps around to the
  // 'max fog' texel
  transientfogdata[(FOGTABLE_SIZE-1)*4+3] = 0;

  // dump the fog table into an OpenGL texture for later user.
  // The texture is FOGTABLE_SIZE texels wide and one texel high;
  // we could use a 1D texture but some OpenGL drivers don't
  // handle 1D textures very well
  glGenTextures (1,&m_fogtexturehandle);
  glBindTexture (GL_TEXTURE_2D, m_fogtexturehandle);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FOGTABLE_SIZE, 1, 0,
  		GL_RGBA, GL_UNSIGNED_BYTE, transientfogdata);
  CHK (delete [] transientfogdata);

  GLenum errtest;
  errtest = glGetError();
  if (errtest != GL_NO_ERROR)
  {
    SysPrintf (MSG_DEBUG_0,"openGL error string: %s\n",gluErrorString(errtest) );
  }

  glClearColor (0.,0.,0.,0.);
  glClearDepth (-1.0);

  // if the user tries to draw lines, text, etc. before calling
  // BeginDraw() they will not see anything since the transforms are
  // not set up correctly until you call BeginDraw().  However,
  // the engine initializer likes to print out console messages
  // on the screen before any 'normal' drawing has been done, so
  // here we set up the transforms so that initialization text will appear on
  // the screen.
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  glOrtho(0.,(GLdouble)width,0.,(GLdouble)height,-1.0,1.0);
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity();

  return true;
}

void csGraphics3DOpenGL::Close ()
{
  if ((width == height) && height == -1)
    return;

  // we should remove all texture handles before we kill
  // the graphics context
  CHK (delete texture_cache);
  texture_cache = NULL;
  CHK (delete lightmap_cache);
  lightmap_cache = NULL;
  CHK (delete txtmgr);
  txtmgr = NULL;

  if (m_fogtexturehandle)
  {
    glDeleteTextures (1,&m_fogtexturehandle);
    m_fogtexturehandle = 0;
  }

  // kill the graphics context
  G2D->Close();

  width = height = -1;
}

void csGraphics3DOpenGL::SetDimensions (int width, int height)
{
  csGraphics3DOpenGL::width = width;
  csGraphics3DOpenGL::height = height;
  csGraphics3DOpenGL::width2 = width/2;
  csGraphics3DOpenGL::height2 = height/2;
}

void csGraphics3DOpenGL::SetPerspectiveCenter (int x, int y)
{
  width2 = x;
  height2 = y;
}

bool csGraphics3DOpenGL::BeginDraw (int DrawFlags)
{
  // if 2D graphics is not locked, lock it
  if ((DrawFlags & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
   && (!(DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))))
  {
    if (!G2D->BeginDraw())
      return false;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.,(GLdouble)width,0.,(GLdouble)height,-1.0,10.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor3f(1.,0.,0.);
    glClearColor(0.,0.,0.,0.);
    dbg_current_polygon = 0;
  }

  if (DrawFlags & CSDRAW_CLEARZBUFFER)
  {
    if (DrawFlags & CSDRAW_CLEARSCREEN)
      glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    else
      glClear(GL_DEPTH_BUFFER_BIT);
  }
  else if (DrawFlags & CSDRAW_CLEARSCREEN)
    G2D->Clear(0);

  DrawMode = DrawFlags;

  return true;
}

void csGraphics3DOpenGL::FinishDraw ()
{
  //ASSERT( G2D );

  if (DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
  {
    G2D->FinishDraw ();
  }
  DrawMode = 0;
}

void csGraphics3DOpenGL::Print (csRect *area)
{
  G2D->Print (area);
  //glClear(GL_COLOR_BUFFER_BIT);
}

void csGraphics3DOpenGL::SetZBufMode (G3DZBufMode mode)
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

void csGraphics3DOpenGL::DrawPolygon (G3DPolygonDP& poly)
{
  int i;
  float P1, P2, P3, P4;
  float Q1, Q2, Q3, Q4;
  float min_z, max_z;

  if (poly.num < 3)
    return;

  // take shortcut drawing if possible
  if (ShortcutDrawPolygon && (this->*ShortcutDrawPolygon)(poly))
    return;

  // For debugging: is we reach the maximum number of polygons to draw we simply stop.
  dbg_current_polygon++;
  if (dbg_current_polygon == dbg_max_polygons_to_draw-1)
    return;
  if (dbg_current_polygon >= dbg_max_polygons_to_draw-1)
    return;

  // Get the plane normal of the polygon. Using this we can calculate
  // '1/z' at every screen space point.
  float Ac, Bc, Cc, Dc, inv_Dc;
  Ac = poly.normal.A;
  Bc = poly.normal.B;
  Cc = poly.normal.C;
  Dc = poly.normal.D;

  float M, N, O;
  float inv_aspect = poly.inv_aspect;
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
  // Also compute the min_z/max_z in camera space coordinates. This is going to be
  // used for mipmapping.
  max_z = min_z = M * (poly.vertices[0].sx - width2)
                + N * (poly.vertices[0].sy - height2) + O;
  // count 'real' number of vertices
  int num_vertices = 1;
  for (i = 1 ; i < poly.num ; i++)
  {
    float inv_z = M * (poly.vertices[i].sx - width2)
                + N * (poly.vertices[i].sy - height2) + O;
    if (inv_z > min_z) min_z = inv_z;
    if (inv_z < max_z) max_z = inv_z;
    // theoretically we should do here sqrt(dx^2+dy^2), but
    // we can approximate it just by abs(dx)+abs(dy)
    if ((fabs (poly.vertices [i].sx - poly.vertices [i - 1].sx)
       + fabs (poly.vertices [i].sy - poly.vertices [i - 1].sy)) > VERTEX_NEAR_THRESHOLD)
      num_vertices++;
  }

  // if this is a 'degenerate' polygon, skip it
  if (num_vertices < 3)
    return;
/*
  // Mipmapping.
  int mipmap;
  if (!poly.uses_mipmaps ||  m_renderstate.mipmap == 1)
    mipmap = 0;
  else if (m_renderstate.mipmap == 0)
  {
    //@@@ The ZDIST_... config values should move to the 3D rasterizer
    if (min_z < 8) mipmap =  0;
    else if (min_z < 16) mipmap = 1;
    else if (min_z < 28) mipmap = 2;
    else mipmap = 3;
  }
  else
    mipmap = m_renderstate.mipmap - 1;
*/

  iPolygonTexture*   tex     = poly.poly_texture[0];
  csTextureMMOpenGL* txt_mm  = (csTextureMMOpenGL*)poly.txt_handle->GetPrivateObject ();

  // find lightmap information, if any
  iLightMap *thelightmap = tex->GetLightMap ();

  // Initialize our static drawing information and cache
  // the texture in the texture cache (if this is not already the case).
  CacheTexture (tex);

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

  bool tex_transp;
  int poly_alpha = poly.alpha;

  csHighColorCacheData *texturecache_data;
  texturecache_data = txt_mm->GetHighColorCacheData ();
  tex_transp = txt_mm->GetTransparent ();
  GLuint texturehandle = *( (GLuint *) (texturecache_data->pData) );

  float flat_r = 1., flat_g = 1., flat_b = 1.;

  glShadeModel (GL_FLAT);
  if (m_renderstate.textured)
    glEnable (GL_TEXTURE_2D);
  else
  {
    glDisable (GL_TEXTURE_2D);
    poly.txt_handle->GetMeanColor (flat_r, flat_g, flat_b);
  }

  SetGLZBufferFlags ();

  if ((poly_alpha > 0) || tex_transp)
  {
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvf  (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable (GL_BLEND);
    if (poly_alpha > 0)
      glColor4f (flat_r, flat_g, flat_b, 1.0 - (float)poly_alpha / 100.0);
    else
      glColor4f (flat_r, flat_g, flat_b, 1.0);
  }
  else
  {
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glDisable (GL_BLEND);
    glColor4f (flat_r, flat_g, flat_b, 0.);
  }

  float sx,sy,sz, one_over_sz, u_over_sz, v_over_sz;

  glBindTexture (GL_TEXTURE_2D, texturehandle);
  glBegin(GL_TRIANGLE_FAN);
  for (i=0; i<poly.num; i++)
  {
    sx = poly.vertices[i].sx - width2;
    sy = poly.vertices[i].sy - height2;
    one_over_sz = M*sx + N*sy + O;
    sz = 1.0/one_over_sz;
    u_over_sz = (J1 * sx + J2 * sy + J3);
    v_over_sz = (K1 * sx + K2 * sy + K3);
    // we must communicate the perspective correction (1/z) for textures
    // by using homogenous coordinates in either texture space
    // or in object (vertex) space.  We do it in texture space.
    //glTexCoord4f(u_over_sz,v_over_sz,one_over_sz,one_over_sz);
    //glVertex3f(poly.vertices[i].sx, poly.vertices[i].sy, -one_over_sz);

    // modified to use homogenous object space coordinates instead
    // of homogenous texture space coordinates
    glTexCoord2f (u_over_sz*sz, v_over_sz*sz);
    glVertex4f (poly.vertices[i].sx*sz, poly.vertices[i].sy*sz, -1.0, sz);
  }
  glEnd();

  // next draw the lightmap over the texture.  The two are blended
  // together.
  // if a lightmap exists, extract the proper data (GL handle, plus
  // texture coordinate bounds)
  if (thelightmap && m_renderstate.lighting)
  {
    csHighColorCacheData *lightmapcache_data = thelightmap->GetHighColorCache();
    GLuint lightmaphandle = *( (GLuint *)(lightmapcache_data->pData) );

    // Jorrit: this code was added to scale the lightmap.
    // @@@ Note that many calculations in this routine are not very optimal
    // to do here and should in fact be precalculated.
    int lmwidth = thelightmap->GetWidth ();
    int lmrealwidth = thelightmap->GetRealWidth ();
    int lmheight = thelightmap->GetHeight ();
    int lmrealheight = thelightmap->GetRealHeight ();
    float scale_u = (float)(lmrealwidth) / (float)(lmwidth);
    float scale_v = (float)(lmrealheight) / (float)(lmheight);
    //float scale_u = (float)(lmrealwidth) / (float)lmwidth;
    //float scale_v = (float)(lmrealheight) / (float)lmheight;

    float lightmap_low_u, lightmap_low_v, lightmap_high_u, lightmap_high_v;
    tex->GetTextureBox(lightmap_low_u,lightmap_low_v,
                       lightmap_high_u,lightmap_high_v);

    lightmap_low_u-=0.125;
    lightmap_low_v-=0.125;
    lightmap_high_u+=0.125;
    lightmap_high_v+=0.125;
    float lightmap_scale_u, lightmap_scale_v;

    if (lightmap_high_u <= lightmap_low_u)
    {
      lightmap_scale_u = scale_u;	// @@@ Is this right?
      lightmap_high_u = lightmap_low_u;
    }
    else
      lightmap_scale_u = scale_u / (lightmap_high_u - lightmap_low_u);

    if (lightmap_high_v <= lightmap_low_v)
    {
      lightmap_scale_v = scale_v;	// @@@ Is this right?
      lightmap_high_v = lightmap_low_v;
    }
    else
      lightmap_scale_v = scale_v / (lightmap_high_v - lightmap_low_v);

    float light_u, light_v;

    glBindTexture (GL_TEXTURE_2D, lightmaphandle);
    glEnable (GL_TEXTURE_2D);

    // Here we set the Z buffer depth function to GL_EQUAL to make sure
    // that the lightmap only overwrites those areas where the Z buffer
    // was updated in the previous pass. This makes sure that intersecting
    // polygons are properly lighted.
    if (z_buf_mode == CS_ZBUF_FILL)
      glDisable(GL_DEPTH_TEST);
    else
      glDepthFunc (GL_EQUAL);

    glEnable(GL_BLEND);
    // The following blend function is configurable.
    glBlendFunc (m_config_options.m_lightmap_src_blend,
    		 m_config_options.m_lightmap_dst_blend);

    glBegin(GL_TRIANGLE_FAN);
    for (i=0; i<poly.num; i++)
    {
      sx = poly.vertices[i].sx - width2;
      sy = poly.vertices[i].sy - height2;
      one_over_sz = M*sx + N*sy + O;
      sz = 1.0/one_over_sz;
      u_over_sz = (J1 * sx + J2 * sy + J3);
      v_over_sz = (K1 * sx + K2 * sy + K3);
      light_u = (u_over_sz*sz - lightmap_low_u) * lightmap_scale_u;
      light_v = (v_over_sz*sz - lightmap_low_v) * lightmap_scale_v;
      // we must communicate the perspective correction (1/z) for textures
      // by using homogenous coordinates in either texture space
      // or in object (vertex) space.  We do it in texture space.
      //glTexCoord4f(light_u/sz,light_v/sz,one_over_sz,one_over_sz);
      //glVertex3f(poly.vertices[i].sx, poly.vertices[i].sy, -one_over_sz);

      // modified to use homogenous object space coordinates instead
      // of homogenous texture space coordinates
      glTexCoord2f (light_u, light_v);
      glVertex4f (poly.vertices[i].sx*sz, poly.vertices[i].sy*sz, -1.0, sz);
    }
    glEnd();
  }

  // An extra pass which improves the lighting on SRC*DST so that
  // it looks more like 2*SRC*DST.
  if (m_config_options.do_extra_bright)
  {
    glDisable (GL_TEXTURE_2D);
    if (z_buf_mode == CS_ZBUF_FILL)
      glDisable(GL_DEPTH_TEST);
    else
      glDepthFunc (GL_EQUAL);
    glShadeModel (GL_SMOOTH);
    glEnable (GL_BLEND);
    glBlendFunc (GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
    //glBlendFunc (GL_ZERO, GL_SRC_COLOR);

    glBegin (GL_TRIANGLE_FAN);
    for (i = 0; i < poly.num; i++)
    {
      sx = poly.vertices [i].sx - width2;
      sy = poly.vertices [i].sy - height2;
      one_over_sz = M * sx + N * sy + O;
      sz = 1.0 / one_over_sz;
      glColor4f (2, 2, 2, 0);
      glVertex4f (poly.vertices [i].sx * sz, poly.vertices [i].sy * sz, -1.0, sz);
    }
    glEnd ();
  }

  // If there is vertex fog then we apply that last.
  if (poly.use_fog)
  {
    glBindTexture (GL_TEXTURE_2D, m_fogtexturehandle);
    glTexEnvf  (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    if (z_buf_mode == CS_ZBUF_FILL)
      glDisable(GL_DEPTH_TEST);
    else
      glDepthFunc (GL_EQUAL);
    glShadeModel (GL_SMOOTH);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin (GL_TRIANGLE_FAN);
    for (i = 0; i < poly.num; i++)
    {
      sx = poly.vertices [i].sx - width2;
      sy = poly.vertices [i].sy - height2;
      one_over_sz = M * sx + N * sy + O;
      sz = 1.0 / one_over_sz;

      // Formula for fog is:
      //    C = I * F + (1-I) * P
      //	I = intensity = density * thickness
      //	F = fog color
      //	P = texture color
      //	C = destination color

      // specify fog vertex color
      glColor3f (poly.fog_info [i].r, poly.fog_info [i].g, poly.fog_info [i].b);

      // specify fog vertex alpha value using the fog table and fog distance
      if (poly.fog_info[i].intensity > FOGTABLE_MAXDISTANCE * FOGTABLE_CLAMPVALUE)
        glTexCoord1f (FOGTABLE_CLAMPVALUE);
      else
        glTexCoord1f (poly.fog_info[i].intensity/FOGTABLE_MAXDISTANCE);

      // specify fog vertex location
      glVertex4f (poly.vertices [i].sx * sz, poly.vertices [i].sy * sz, -1.0, sz);
    }
    glEnd ();

  }
}

void csGraphics3DOpenGL::DrawPolygonDebug (G3DPolygonDP& /*poly*/)
{
}

// Calculate round (f) of a 16:16 fixed pointer number
// and return a long integer.
inline long round16 (long f)
{
  return (f + 0x8000) >> 16;
}

void csGraphics3DOpenGL::StartPolygonFX (iTextureHandle* handle, UInt mode)
{
  // try shortcut method if available
  if (ShortcutStartPolygonFX && (this->*ShortcutStartPolygonFX)(handle, mode) )
    return;

  m_gouraud = m_renderstate.lighting && m_renderstate.gouraud && ((mode & CS_FX_GOURAUD) != 0);
  m_mixmode = mode;
  m_alpha = 1.0f - float (mode & CS_FX_MASK_ALPHA) / 255.;

  GLuint texturehandle = 0;
  if (handle && m_renderstate.textured)
  {
    csTextureMMOpenGL* txt_mm = (csTextureMMOpenGL*)handle->GetPrivateObject ();
    texture_cache->Add (handle);

    csHighColorCacheData *cachedata;
    cachedata = txt_mm->GetHighColorCacheData ();
    texturehandle = *( (GLuint *) (cachedata->pData) );
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
      glBlendFunc(GL_DST_COLOR,GL_SRC_COLOR);
      glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      break;
    case CS_FX_ADD:
      // Color = 1 * DEST + 1 * SRC = DEST + SRC
      m_alpha = 1.0f;
      glBlendFunc(GL_ONE,GL_ONE);
      glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      break;
    case CS_FX_ALPHA:
      // Color = Alpha * DEST + (1-Alpha) * SRC
      glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
      glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      break;
    case CS_FX_TRANSPARENT:
      // Color = 1 * DEST + 0 * SRC
      m_alpha = 1.0f;
      glBlendFunc (GL_ZERO,GL_ONE);
      glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      break;
    case CS_FX_COPY:
    default:
      enable_blending = false;
      // Color = 0 * DEST + 1 * SRC = SRC
      m_alpha = 1.0f;
      glBlendFunc (GL_ONE, GL_ZERO);
      glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      break;
  }

  m_textured = (texturehandle != 0);
  if (m_textured)
  {
    glBindTexture (GL_TEXTURE_2D, texturehandle);
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

void csGraphics3DOpenGL::FinishPolygonFX ()
{
  // attempt shortcut method
  if (ShortcutFinishPolygonFX && (this->*ShortcutFinishPolygonFX)())
    return;
}

void csGraphics3DOpenGL::DrawPolygonFX (G3DPolygonDPFX& poly)
{
  // take shortcut if possible
  if (ShortcutDrawPolygonFX && (this->*ShortcutDrawPolygonFX)(poly))
    return;

  float flat_r = 1., flat_g = 1., flat_b = 1.;
  if (!m_textured)
  {
    flat_r = poly.flat_color_r;
    flat_g = poly.flat_color_g;
    flat_b = poly.flat_color_b;
  }

  glBegin (GL_TRIANGLE_FAN);
  float sx,sy;
  int i;
  for (i = 0; i < poly.num; i++)
  {
    sx = poly.vertices[i].sx - width2;
    sy = poly.vertices[i].sy - height2;
    glTexCoord2f (poly.vertices[i].u,poly.vertices[i].v);
    if (m_gouraud)
      glColor4f (flat_r * poly.vertices[i].r, flat_g * poly.vertices[i].g,
        flat_b * poly.vertices[i].b, m_alpha);
    else
      glColor4f (flat_r, flat_g, flat_b, m_alpha);
    glVertex3f (poly.vertices[i].sx,poly.vertices[i].sy,-poly.vertices[i].z);
  }
  glEnd ();

  // If there is vertex fog then we apply that last.
  if (poly.use_fog)
  {
    // let OpenGL save old values for us
    // -GL_COLOR_BUFFER_BIT saves blend types among other things,
    // -GL_DEPTH_BUFFER_BIT saves depth test function
    // -GL_TEXTURE_BIT saves current texture handle
    glPushAttrib (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_TEXTURE_BIT);

    // we need to texture and blend, without vertex color interpolation
    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, m_fogtexturehandle);
    glTexEnvf  (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
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
      //    C = I * F + (1-I) * P
      //	I = intensity = 1-exp(density * thickness)
      //	F = fog color
      //	P = texture color
      //	C = destination color
      // the '1-exp' part is embodied in the fog texture; view the
      // fog texture as a function producing the alpha value, with
      // input (texture coordinate) being equal to I.

      // specify fog vertex color
      glColor3f (poly.fog_info[i].r, poly.fog_info[i].g, poly.fog_info[i].b);

      // specify fog vertex transparency
      float I = poly.fog_info[i].intensity;
      if (I > FOGTABLE_MAXDISTANCE * FOGTABLE_CLAMPVALUE)
        glTexCoord1f (FOGTABLE_CLAMPVALUE);
      else
        glTexCoord1f (I/FOGTABLE_MAXDISTANCE);

      // specify fog vertex location
      glVertex3f (poly.vertices[i].sx,poly.vertices[i].sy,-poly.vertices[i].z);
    }
    glEnd ();

    // Restore state (blend mode, texture handle, etc.) for next triangle
    glPopAttrib();
    if (!m_textured)
      glDisable (GL_TEXTURE_2D);
    if (!m_gouraud)
      glShadeModel (GL_FLAT);
  }
}

/**
 * Initiate a volumetric fog object. This function will be called
 * before front-facing and back-facing fog polygons are added to
 * the object. The fog object will be convex but not necesarily
 *         closed.
 * The given CS_ID can be used to identify multiple fog
 * objects when
 * multiple objects are started.
 */
void csGraphics3DOpenGL::OpenFogObject(CS_ID /*id*/, csFog* /*fog*/)
{
}

/**
 * Add a front or back-facing fog polygon in the current fog object.
 * Note that it is guaranteed that all back-facing fog polygons
 * will have been added before the first front-facing polygon.
 * fogtype can be:
 *    CS_FOG_FRONT:   a front-facing polygon
 *    CS_FOG_BACK:    a back-facing polygon
 *    CS_FOG_VIEW:    the view-plane
 */
void csGraphics3DOpenGL::AddFogPolygon(CS_ID /*id*/, G3DPolygonAFP &/*poly*/, int /*fogtype*/)
{
}

/**
 * Close a volumetric fog object. After the volumetric object is
 * closed it should be rendered on screen (whether you do it here
 * or in DrawFrontFog/DrawBackFog is not important).
 */
void csGraphics3DOpenGL::CloseFogObject(CS_ID /*id*/)
{
}

void csGraphics3DOpenGL::CacheTexture (iPolygonTexture* texture)
{
  iTextureHandle* txt_handle = texture->GetTextureHandle ();
  texture_cache->Add (txt_handle);
  lightmap_cache->Add (texture);
}

void csGraphics3DOpenGL::CacheLightedTexture(iPolygonTexture* /*texture*/)
{
}

void csGraphics3DOpenGL::CacheInitTexture (iPolygonTexture* /*texture*/)
{
#if 0
  iTextureContainer* piTC = world->GetTextures ();
  tcache->init_texture (texture, piTC);
#endif
}

void csGraphics3DOpenGL::CacheSubTexture (iPolygonTexture* /*texture*/, int /*u*/, int /*v*/)
{
#if 0
  iTextureContainer* piTC = world->GetTextures ();
  tcache->use_sub_texture (texture, piTC, u, v);
#endif
}

void csGraphics3DOpenGL::CacheRectTexture (iPolygonTexture* /*tex*/,
  int /*minu*/, int /*minv*/, int /*maxu*/, int /*maxv*/)
{
#if 0
  iTextureContainer* piTC = world->GetTextures ();
  int subtex_size;
  tex->GetSubtexSize (subtex_size);

  int iu, iv;
  for (iu = minu ; iu < maxu ; iu += subtex_size)
  {
    for (iv = minv ; iv < maxv ; iv += subtex_size)
        tcache->use_sub_texture (tex, piTC, iu, iv);
    tcache->use_sub_texture (tex, piTC, iu, maxv);
  }
  for (iv = minv ; iv < maxv ; iv += subtex_size)
      tcache->use_sub_texture (tex, piTC, maxu, iv);
  tcache->use_sub_texture (tex, piTC, maxu, maxv);
#endif
}

void csGraphics3DOpenGL::UncacheTexture (iPolygonTexture* texture)
{
  (void)texture;
}

bool csGraphics3DOpenGL::SetRenderState(G3D_RENDERSTATEOPTION op, long value)
{
  switch(op)
  {
    case G3DRENDERSTATE_NOTHING:
      break;

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
      m_renderstate.dither = value;
      break;
    case G3DRENDERSTATE_SPECULARENABLE:
      m_renderstate.specular = value;
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
	// XAVIER: unhandled cases in enumerated switch (just to keep gcc happy)
    case G3DRENDERSTATE_MMXENABLE:
      return false;
    case G3DRENDERSTATE_INTERLACINGENABLE:
      return false;
    case G3DRENDERSTATE_FILTERINGENABLE:
      m_renderstate.texel_filt = value;
      break;
    case G3DRENDERSTATE_PERFECTMAPPINGENABLE:
      m_renderstate.perfect = value;
      break;
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

long csGraphics3DOpenGL::GetRenderState(G3D_RENDERSTATEOPTION op)
{
  switch(op)
  {
    case G3DRENDERSTATE_NOTHING:
      return 0;
    case G3DRENDERSTATE_ZBUFFERTESTENABLE:
      return (bool)(z_buf_mode & CS_ZBUF_TEST);
    case G3DRENDERSTATE_ZBUFFERFILLENABLE:
      return (bool)(z_buf_mode & CS_ZBUF_FILL);
    case G3DRENDERSTATE_DITHERENABLE:
      return m_renderstate.dither;
    case G3DRENDERSTATE_SPECULARENABLE:
      return m_renderstate.specular;
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
    case G3DRENDERSTATE_FILTERINGENABLE:
      return m_renderstate.texel_filt;
    case G3DRENDERSTATE_PERFECTMAPPINGENABLE:
      return m_renderstate.perfect;
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

void csGraphics3DOpenGL::GetCaps(G3D_CAPS *caps)
{
  if (!caps)
    return;

  caps->ColorModel = G3DCOLORMODEL_RGB;
  caps->CanClip = false;
  caps->SupportsArbitraryMipMapping = true;
  caps->BitDepth = 24;
  caps->ZBufBitDepth = 16;
  caps->minTexHeight = 2;
  caps->minTexWidth = 2;
  caps->maxTexHeight = 1024;
  caps->maxTexWidth = 1024;
  caps->PrimaryCaps.RasterCaps = G3DRASTERCAPS_SUBPIXEL;
  caps->PrimaryCaps.canBlend = true;
  caps->PrimaryCaps.ShadeCaps = G3DRASTERCAPS_LIGHTMAP;
  caps->PrimaryCaps.PerspectiveCorrects = true;
  caps->PrimaryCaps.FilterCaps = G3D_FILTERCAPS((int)G3DFILTERCAPS_NEAREST | (int)G3DFILTERCAPS_MIPNEAREST);
  caps->fog = G3DFOGMETHOD_VERTEX;
}

void csGraphics3DOpenGL::ClearCache ()
{
}

void csGraphics3DOpenGL::DumpCache ()
{
}

void csGraphics3DOpenGL::DrawLine (csVector3& v1, csVector3& v2, float fov, int color)
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

void csGraphics3DOpenGL::SetGLZBufferFlags()
{
  switch (z_buf_mode)
  {
    case CS_ZBUF_NONE:
      glDisable (GL_DEPTH_TEST);
      break;
    case CS_ZBUF_FILL:
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
  }
}

void csGraphics3DOpenGL::SysPrintf (int mode, char* szMsg, ...)
{
  char buf[1024];
  va_list arg;

  va_start (arg, szMsg);
  vsprintf (buf, szMsg, arg);
  va_end (arg);

  System->Print(mode, buf);
}

#if USE_MULTITEXTURE
// extension detection code blatantly lifted from Mark J. Kilgard's
// "All about OpenGL extensions" paper.
static int isExtensionSupported(const char *extension)
{
  const GLubyte *extensions = NULL;
  const GLubyte *start;
  GLubyte *where, *terminator;

  /* Extension names should not have spaces. */
  where = (GLubyte *) strchr(extension, ' ');
  if (where || *extension == '\0')
    return 0;

  extensions = glGetString(GL_EXTENSIONS);
  /* It takes a bit of care to be fool-proof about parsing the
     OpenGL extensions string. Don't be fooled by sub-strings, etc. */
  start = extensions;
  for (;;) {
    where = (GLubyte *) strstr((const char *) start, extension);
    if (!where)
      break;
    terminator = where + strlen(extension);
    if (where == start || *(where - 1) == ' ')
      if (*terminator == ' ' || *terminator == '\0')
        return 1;
    start = terminator;
  }
  return 0;
}
#endif

/// Shortcut to override standard polygon drawing when we have multitexture
bool csGraphics3DOpenGL::MultitextureDrawPolygon(G3DPolygonDP &poly)
{
// work in progress - GJH
#if USE_MULTITEXTURE
  iPolygonTexture* tex = poly.poly_texture[0];

  // find lightmap information, if any
  iLightMap *thelightmap = tex->GetLightMap ();

  // the shortcut works only if there is a lightmap and no fog
  if (!thelightmap || poly.use_fog)
    return false;

  // OK, we're gonna draw a polygon with a dual texture
  // Get the plane normal of the polygon. Using this we can calculate
  // '1/z' at every screen space point.
  float Ac, Bc, Cc, Dc, inv_Dc;
  Ac = poly.normal.A;
  Bc = poly.normal.B;
  Cc = poly.normal.C;
  Dc = poly.normal.D;

  float inv_aspect = poly.inv_aspect;
  float M, N, O;
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
  // Also compute the min_z/max_z in camera space coordinates. This is going to be
  // used for mipmapping.
  float max_z, min_z;
  max_z = min_z = M * (poly.vertices[0].sx - width2)
                + N * (poly.vertices[0].sy - height2) + O;
  // count 'real' number of vertices
  int num_vertices = 1;
  int i;
  for (i = 1 ; i < poly.num ; i++)
  {
    float inv_z = M * (poly.vertices[i].sx - width2)
                + N * (poly.vertices[i].sy - height2) + O;
    if (inv_z > min_z) min_z = inv_z;
    if (inv_z < max_z) max_z = inv_z;
    // theoretically we should do here sqrt(dx^2+dy^2), but
    // we can approximate it just by abs(dx)+abs(dy)
    if ((fabs (poly.vertices [i].sx - poly.vertices [i - 1].sx)
       + fabs (poly.vertices [i].sy - poly.vertices [i - 1].sy)) > VERTEX_NEAR_THRESHOLD)
      num_vertices++;
  }

  // if this is a 'degenerate' polygon, skip it
  if (num_vertices < 3)
    return false;

  // Mipmapping.
  int mipmap;
  if (!poly.uses_mipmaps ||  m_renderstate.mipmap == 1)
    mipmap = 0;
  else if (m_renderstate.mipmap == 0)
  {
    //@@@ The ZDIST_... config values should move to the 3D rasterizer
    if (min_z < 8) mipmap =  0;
    else if (min_z < 16) mipmap = 1;
    else if (min_z < 28) mipmap = 2;
    else mipmap = 3;
  }
  else
    mipmap = m_renderstate.mipmap - 1;

  tex = poly.poly_texture[mipmap];
  csTextureMMOpenGL* txt_mm  = (csTextureMMOpenGL*)poly.txt_handle->GetPrivateObject ();

  // find lightmap information, if any
  thelightmap = tex->GetLightMap ();

  // Initialize our static drawing information and cache
  // the texture in the texture cache (if this is not already the case).
  CacheTexture (tex);

  // @@@ The texture transform matrix is currently written as T = M*(C-V)
  // (with V being the transform vector, M the transform matrix, and C
  // the position in camera space coordinates. It would be better (more
  // suitable for the following calculations) if it would be written
  // as T = M*C - V.
  float P1, P2, P3, P4, Q1, Q2, Q3, Q4;
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

  bool tex_transp;
  int poly_alpha = poly.alpha;

  csHighColorCacheData *texturecache_data;
  texturecache_data = txt_mm->GetHighColorCache ();
  tex_transp = txt_mm->get_transparent ();
  GLuint texturehandle = *( (GLuint *) (texturecache_data->pData) );

  csHighColorCacheData *lightmapcache_data = thelightmap->GetHighColorCache();
  GLuint lightmaphandle = *( (GLuint *)(lightmapcache_data->pData) );

  // configure base texture for texure unit 0
  float flat_r = 1.0, flat_g = 1.0, flat_b = 1.0;
  glActiveTextureARB(GL_TEXTURE0_ARB);
  glBindTexture (GL_TEXTURE_2D, texturehandle);
  if ((poly_alpha > 0) || tex_transp)
  {
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvf  (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable (GL_BLEND);
    if (poly_alpha > 0)
      glColor4f (flat_r, flat_g, flat_b, 1.0 - (float)poly_alpha / 100.0);
    else
      glColor4f (flat_r, flat_g, flat_b, 1.0);
  }
  else
  {
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glDisable (GL_BLEND);
    glColor4f (flat_r, flat_g, flat_b, 0.);
  }

  // configure lightmap for texture unit 1
  glActiveTextureARB(GL_TEXTURE1_ARB);
  glEnable (GL_TEXTURE_2D);
  glBindTexture (GL_TEXTURE_2D, lightmaphandle);
  glTexEnvf  (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  SetGLZBufferFlags();

  // Jorrit: this code was added to scale the lightmap.
  // @@@ Note that many calculations in this routine are not very optimal
  // to do here and should in fact be precalculated.
  int lmwidth = thelightmap->GetWidth ();
  int lmrealwidth = thelightmap->GetRealWidth ();
  int lmheight = thelightmap->GetHeight ();
  int lmrealheight = thelightmap->GetRealHeight ();
  //float scale_u = (float)(lmrealwidth-1) / (float)lmwidth;
  //float scale_v = (float)(lmrealheight-1) / (float)lmheight;
  float scale_u = (float)(lmrealwidth) / (float)lmwidth;
  float scale_v = (float)(lmrealheight) / (float)lmheight;

  float lightmap_low_u, lightmap_low_v, lightmap_high_u, lightmap_high_v;
  tex->GetTextureBox(lightmap_low_u,lightmap_low_v, lightmap_high_u,lightmap_high_v);
  lightmap_low_u-=0.125;
  lightmap_low_v-=0.125;
  lightmap_high_u+=0.125;
  lightmap_high_v+=0.125;

  float lightmap_scale_u, lightmap_scale_v;

  if (lightmap_high_u == lightmap_low_u)
    lightmap_scale_u = scale_u;	// @@@ Is this right?
  else
    lightmap_scale_u = scale_u / (lightmap_high_u - lightmap_low_u);

  if (lightmap_high_v == lightmap_low_v)
    lightmap_scale_v = scale_v;	// @@@ Is this right?
  else
    lightmap_scale_v = scale_v / (lightmap_high_v - lightmap_low_v);

  float light_u, light_v;
  float sx, sy, sz, one_over_sz;
  float u_over_sz, v_over_sz;

  glBegin(GL_TRIANGLE_FAN);
  for (i=0; i<poly.num; i++)
  {
    sx = poly.vertices[i].sx - width2;
    sy = poly.vertices[i].sy - height2;
    one_over_sz = M*sx + N*sy + O;
    sz = 1.0/one_over_sz;
    u_over_sz = (J1 * sx + J2 * sy + J3);
    v_over_sz = (K1 * sx + K2 * sy + K3);
    light_u = (u_over_sz*sz - lightmap_low_u) * lightmap_scale_u;
    light_v = (v_over_sz*sz - lightmap_low_v) * lightmap_scale_v;

    // modified to use homogenous object space coordinates instead
    // of homogenous texture space coordinates
    glMultiTexCoord2fARB (GL_TEXTURE0_ARB,u_over_sz*sz, v_over_sz*sz);
    glMultiTexCoord2fARB (GL_TEXTURE1_ARB,light_u, light_v);
    glVertex4f (poly.vertices[i].sx*sz, poly.vertices[i].sy*sz, -1.0, sz);
  }
  glEnd();

  // we must disable the 2nd texture unit, so that other parts of the
  // code won't accidently have a second texture applied if they
  // don't want it.
  // at this point our active texture is still TEXTURE1_ARB
  glActiveTextureARB(GL_TEXTURE1_ARB);
  glDisable (GL_TEXTURE_2D);
  glActiveTextureARB(GL_TEXTURE0_ARB);

  return true;
#else
  (void)poly;
  // multitexture not enabled -- how did we get into this shortcut?
  return false;
#endif
}

/// Shortcuts to replace the standard Start/Draw/Finish set of Draw...FX functions;
/// this set collects up polygons and then draws them in batches, instead
/// of drawing each individual poly with gl calls
bool csGraphics3DOpenGL::BatchStartPolygonFX(iTextureHandle*, UInt /*mode*/)
{
  // not done yet
  return false;
}

bool csGraphics3DOpenGL::BatchAccumulatePolygonFX(G3DPolygonDPFX&)
{
  // not done yet
  return false;
}

bool csGraphics3DOpenGL::BatchFlushPolygonFX()
{
  // not done yet
  return false;
}
