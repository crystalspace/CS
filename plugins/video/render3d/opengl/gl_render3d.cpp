/*
  Copyright (C) 2002 by Marten Svanfeldt
                        Anders Stenberg

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"

#include <ctype.h>

#include "csqint.h"

#include "csgfx/csimgvec.h"
#include "csgfx/memimage.h"
#include "csgfx/renderbuffer.h"

#include "csgeom/polyclip.h"
#include "csgeom/transfrm.h"
#include "csgeom/vector4.h"

#include "cstool/fogmath.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/strset.h"

#include "igeom/clip2d.h"

#include "iutil/cmdline.h"
#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "iutil/eventq.h"

#include "ivaria/reporter.h"

#include "ivideo/lighting.h"
#include "ivideo/txtmgr.h"
#include "ivideo/graph3d.h"
#include "ivideo/rendermesh.h"
#include "ivideo/halo.h"

#include "ivideo/shader/shader.h"

#include "gl_render3d.h"
#include "gl_renderbuffer.h"
#include "gl_txtmgr.h"
#include "gl_polyrender.h"
#include "gl_r2t_framebuf.h"

#include "csplugincommon/opengl/glextmanager.h"

#include "csplugincommon/render3d/txtmgr.h"
#include "csplugincommon/render3d/normalizationcube.h"

#define BYTE_TO_FLOAT(x) ((x) * (1.0 / 255.0))

csGLStateCache* csGLGraphics3D::statecache = 0;
csGLExtensionManager* csGLGraphics3D::ext = 0;

const int CS_CLIPPER_EMPTY = 0xf008412;

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csGLGraphics3D)

SCF_IMPLEMENT_IBASE(csGLGraphics3D)
  SCF_IMPLEMENTS_INTERFACE(iGraphics3D)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iShaderRenderInterface)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iDebugHelper)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGLGraphics3D::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGLGraphics3D::eiShaderRenderInterface)
  SCF_IMPLEMENTS_INTERFACE (iShaderRenderInterface)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csGLGraphics3D::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END


csGLGraphics3D::csGLGraphics3D (iBase *parent) : isOpen (false), 
  wantToSwap (false), delayClearFlags (0)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiShaderRenderInterface);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiDebugHelper);

  verbose = false;
  frustum_valid = false;

  do_near_plane = false;
  viewwidth = 100;
  viewheight = 100;

  stencilclipnum = 0;
  clip_planes_enabled = false;
  hasOld2dClip = false;

  render_target = 0;

  current_drawflags = 0;
  current_shadow_state = 0;
  current_zmode = CS_ZBUF_NONE;
  zmesh = false;
  forceWireframe = false;

  use_hw_render_buffers = false;
  vbo_thresshold = 0;
  stencil_threshold = 500;
  broken_stencil = false;

  unsigned int i;
  for (i=0; i<16; i++)
  {
    texunittarget[i] = 0;
    texunitenabled[i] = false;
  }
  for (i = 0; i < CS_VATTRIB_SPECIFIC_LAST+1; i++)
  {
    scrapMapping[i] = CS_BUFFER_NONE;
  }
  scrapMapping[CS_VATTRIB_POSITION] = CS_BUFFER_POSITION;
  scrapMapping[CS_VATTRIB_TEXCOORD0] = CS_BUFFER_TEXCOORD0;
  scrapMapping[CS_VATTRIB_COLOR] = CS_BUFFER_COLOR;
//  lastUsedShaderpass = 0;

  scrapIndicesSize = 0;
  scrapVerticesSize = 0;
  scrapBufferHolder.AttachNew (new csRenderBufferHolder);

  shadow_stencil_enabled = false;
  clipping_stencil_enabled = false;
  clipportal_dirty = true;
  clipportal_floating = 0;
  cliptype = CS_CLIPPER_NONE;

  r2tbackend = 0;
}

csGLGraphics3D::~csGLGraphics3D()
{
  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (q)
    q->RemoveListener (scfiEventHandler);

  SCF_DESTRUCT_EMBEDDED_IBASE(scfiDebugHelper);
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiShaderRenderInterface);
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

////////////////////////////////////////////////////////////////////
// Private helpers
////////////////////////////////////////////////////////////////////


void csGLGraphics3D::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csReportV (object_reg, severity, "crystalspace.graphics3d.opengl", msg, arg);
  va_end (arg);
}

void csGLGraphics3D::SetCorrectStencilState ()
{
  if (shadow_stencil_enabled || clipping_stencil_enabled ||
  	clipportal_floating)
  {
    statecache->Enable_GL_STENCIL_TEST ();
  }
  else
  {
    statecache->Disable_GL_STENCIL_TEST ();
  }
}

void csGLGraphics3D::EnableStencilShadow ()
{
  shadow_stencil_enabled = true;
  statecache->Enable_GL_STENCIL_TEST ();
}

void csGLGraphics3D::DisableStencilShadow ()
{
  shadow_stencil_enabled = false;
  SetCorrectStencilState ();
}

void csGLGraphics3D::EnableStencilClipping ()
{
  clipping_stencil_enabled = true;
  statecache->Enable_GL_STENCIL_TEST ();
}

void csGLGraphics3D::DisableStencilClipping ()
{
  clipping_stencil_enabled = false;
  SetCorrectStencilState ();
}

void csGLGraphics3D::SetGlOrtho (bool inverted)
{
  if (inverted)
    glOrtho (0., (GLdouble) viewwidth, (GLdouble) viewheight, 0., -1.0, 10.0);
  else
    glOrtho (0., (GLdouble) viewwidth, 0., (GLdouble) viewheight, -1.0, 10.0);
}

csZBufMode csGLGraphics3D::GetZModePass2 (csZBufMode mode)
{
  switch (mode)
  {
    case CS_ZBUF_NONE:
    case CS_ZBUF_TEST:
    case CS_ZBUF_EQUAL:
      return mode;
    case CS_ZBUF_FILL:
    case CS_ZBUF_FILLONLY:
    case CS_ZBUF_USE:
      return CS_ZBUF_EQUAL;
    default:
      return CS_ZBUF_NONE;
  }
}

void csGLGraphics3D::SetZModeInternal (csZBufMode mode)
{
  switch (mode)
  {
    case CS_ZBUF_NONE:
      statecache->Disable_GL_DEPTH_TEST ();
      break;
    case CS_ZBUF_FILL:
    case CS_ZBUF_FILLONLY:
      statecache->Enable_GL_DEPTH_TEST ();
      statecache->SetDepthFunc (GL_ALWAYS);
      statecache->SetDepthMask (GL_TRUE);
      break;
    case CS_ZBUF_EQUAL:
      statecache->Enable_GL_DEPTH_TEST ();
      statecache->SetDepthFunc (GL_EQUAL);
      statecache->SetDepthMask (GL_FALSE);
      break;
    case CS_ZBUF_INVERT:
      statecache->Enable_GL_DEPTH_TEST ();
      statecache->SetDepthFunc (GL_LESS);
      statecache->SetDepthMask (GL_FALSE);
      break;
    case CS_ZBUF_TEST:
    case CS_ZBUF_USE:
      statecache->Enable_GL_DEPTH_TEST ();
      statecache->SetDepthFunc (GL_GEQUAL);
      statecache->SetDepthMask ((mode == CS_ZBUF_USE) ? GL_TRUE : GL_FALSE);
      break;
    default:
      break;
  }
}

void csGLGraphics3D::SetMixMode (uint mode)
{

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
      statecache->SetBlendFunc (GL_ZERO, GL_SRC_COLOR);
      break;
    case CS_FX_MULTIPLY2:
      // Color = SRC * DEST + DEST * SRC = 2 * DEST * SRC
      statecache->SetBlendFunc (GL_DST_COLOR, GL_SRC_COLOR);
      break;
    case CS_FX_ADD:
      // Color = 1 * DEST + 1 * SRC = DEST + SRC
      statecache->SetBlendFunc (GL_ONE, GL_ONE);
      break;
    case CS_FX_DESTALPHAADD:
      // Color = DEST + DestAlpha * SRC
      statecache->SetBlendFunc (GL_DST_ALPHA, GL_ONE);
      break;
    case CS_FX_SRCALPHAADD:
      // Color = DEST + SrcAlpha * SRC
      statecache->SetBlendFunc (GL_SRC_ALPHA, GL_ONE);
      break;
    case CS_FX_PREMULTALPHA:
      // Color = (1-SrcAlpha) * DEST + SRC
      statecache->SetBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
      break;
    case CS_FX_ALPHA:
      // Color = Alpha * SRC + (1-Alpha) * DEST
      statecache->SetBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      break;
    case CS_FX_TRANSPARENT:
      // Color = 1 * DEST + 0 * SRC
      statecache->SetBlendFunc (GL_ZERO, GL_ONE);
      break;
    case CS_FX_COPY:
    default:
      enable_blending = false;
      break;
  }

  if (enable_blending)
    statecache->Enable_GL_BLEND ();
  else
    statecache->Disable_GL_BLEND ();
}

void csGLGraphics3D::SetAlphaType (csAlphaMode::AlphaType alphaType)
{
  switch (alphaType)
  {
    default:
    case csAlphaMode::alphaNone:
      statecache->Disable_GL_BLEND ();
      statecache->Disable_GL_ALPHA_TEST ();
      break;
    case csAlphaMode::alphaBinary:
      statecache->Disable_GL_BLEND ();
      statecache->Enable_GL_ALPHA_TEST ();
      statecache->SetAlphaFunc (GL_GEQUAL, 0.5f);
      break;
    case csAlphaMode::alphaSmooth:
      statecache->Enable_GL_BLEND ();
      statecache->Disable_GL_ALPHA_TEST ();			      
      statecache->SetBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      break;
  }
}

void csGLGraphics3D::SetMirrorMode (bool mirror)
{
  if (mirror)
    statecache->SetCullFace (GL_BACK);
  else
    statecache->SetCullFace (GL_FRONT);
}

void csGLGraphics3D::CalculateFrustum ()
{
  if (frustum_valid) return;
  frustum_valid = true;
  if (clipper)
  {
    frustum.MakeEmpty ();
    size_t nv = clipper->GetVertexCount ();
    csVector3 v3;
    v3.z = 1;
    csVector2* v = clipper->GetClipPoly ();
    size_t i;
    for (i = 0 ; i < nv ; i++)
    {
      v3.x = (v[i].x - asp_center_x) * (1.0/aspect);
      v3.y = (v[i].y - asp_center_y) * (1.0/aspect);
      frustum.AddVertex (v3);
    }
  }
}

void csGLGraphics3D::SetupStencil ()
{
  if (stencil_initialized)
    return;

  stencil_initialized = true;

  if (clipper)
  {
    statecache->SetMatrixMode (GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity ();
    statecache->SetMatrixMode (GL_MODELVIEW);
    glPushMatrix ();
    glLoadIdentity ();
    // First set up the stencil area.
    EnableStencilClipping ();

    //stencilclipnum++;
    //if (stencilclipnum>255)
    {
      /*glStencilMask (128);
      glClearStencil (128);
      glClear (GL_STENCIL_BUFFER_BIT);*/
      stencilclipnum = 1;
    }
    size_t nv = clipper->GetVertexCount ();
    csVector2* v = clipper->GetClipPoly ();

    statecache->SetShadeModel (GL_FLAT);

    bool oldz = statecache->IsEnabled_GL_DEPTH_TEST ();
    statecache->Disable_GL_DEPTH_TEST ();
    bool tex2d = statecache->IsEnabled_GL_TEXTURE_2D ();
    statecache->Disable_GL_TEXTURE_2D ();

    GLboolean wmRed, wmGreen, wmBlue, wmAlpha;
    statecache->GetColorMask (wmRed, wmGreen, wmBlue, wmAlpha);
    statecache->SetColorMask (false, false, false, false);

    statecache->SetStencilMask (stencil_clip_mask);
    statecache->SetStencilFunc (GL_ALWAYS, stencil_clip_value, stencil_clip_mask);
    statecache->SetStencilOp (GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glBegin (GL_TRIANGLE_FAN);
      glVertex2f ( 1, -1);
      glVertex2f (-1, -1);
      glVertex2f (-1,  1);
      glVertex2f ( 1,  1);
    glEnd ();

    statecache->SetStencilFunc (GL_ALWAYS, 0, stencil_clip_mask);

    glBegin (GL_TRIANGLE_FAN);
    size_t i;
    const float clipVertScaleX = 2.0f / (float)viewwidth;
    const float clipVertScaleY = 2.0f / (float)viewheight;
    for (i = 0 ; i < nv ; i++)
      glVertex2f (v[i].x*clipVertScaleX - 1.0f,
                  v[i].y*clipVertScaleY - 1.0f);
    glEnd ();

    statecache->SetColorMask (wmRed, wmGreen, wmBlue, wmAlpha);

    glPopMatrix ();
    statecache->SetMatrixMode (GL_PROJECTION);
    glPopMatrix ();
    if (oldz) statecache->Enable_GL_DEPTH_TEST ();
    if (tex2d) statecache->Enable_GL_TEXTURE_2D ();
  }
}

int csGLGraphics3D::SetupClipPlanes (bool add_clipper,
                                   bool add_near_clip,
                                   bool add_z_clip)
{
  if (!(add_clipper || add_near_clip || add_z_clip)) return 0;

  statecache->SetMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  glLoadIdentity ();

  int planes = 0;
  GLdouble plane_eq[4];

  // This routine assumes the hardware planes can handle the
  // required number of planes from the clipper.
  if (clipper && add_clipper)
  {
    CalculateFrustum ();
    csPlane3 pl;
    int i1;
    i1 = (int)frustum.GetVertexCount ()-1;

    int maxfrustplanes = 6;
    if (add_near_clip) maxfrustplanes--;
    if (add_z_clip) maxfrustplanes--;
    int numfrustplanes = (int)frustum.GetVertexCount ();
    // Correct for broken stencil implementation.
    if (numfrustplanes > maxfrustplanes)
      numfrustplanes = maxfrustplanes;

    int i;
    for (i = 0 ; i < numfrustplanes ; i++)
    {
      pl.Set (csVector3 (0), frustum[i], frustum[i1]);
      plane_eq[0] = pl.A ();
      plane_eq[1] = pl.B ();
      plane_eq[2] = pl.C ();
      plane_eq[3] = pl.D ();
      glClipPlane ((GLenum)(GL_CLIP_PLANE0+planes), plane_eq);
      planes++;
      i1 = i;
    }
  }

  if (add_near_clip)
  {
    plane_eq[0] = -near_plane.A ();
    plane_eq[1] = -near_plane.B ();
    plane_eq[2] = -near_plane.C ();
    plane_eq[3] = -near_plane.D ();
    glClipPlane ((GLenum)(GL_CLIP_PLANE0+planes), plane_eq);
    planes++;
  }
  if (add_z_clip)
  {
    plane_eq[0] = 0;
    plane_eq[1] = 0;
    plane_eq[2] = 1;
    plane_eq[3] = -.001;
    glClipPlane ((GLenum)(GL_CLIP_PLANE0+planes), plane_eq);
    planes++;
  }

  glPopMatrix ();
  return planes;
}

void csGLGraphics3D::SetupClipper (int clip_portal,
                                 int clip_plane,
                                 int clip_z_plane,
				 int tri_count)
{
  // @@@@ RETHINK!!! THIS IS A HUGE PERFORMANCE BOOST. BUT???
  clip_z_plane = CS_CLIP_NOT;

  // There are two cases to consider.
  // 1. We have not encountered any floating portals yet. In that case
  //    we use scissor, stencil,  or plane clipping as directed by the
  //    clipper, clipping needs, and object (number of triangles).
  // 2. We have encountered a floating portal. In that case we always
  //    consider every subsequent portal as being floating.
  if (clipportal_floating)
  {
    if (clipportal_dirty)
    {
      clipportal_dirty = false;
      SetupClipPortals ();
    }
  }

  // If we have a box clipper then we can simply use glScissor (which
  // is already set up) to do the clipping. In case we have a floating
  // portal we also can use the following part.
  if ((clipper && clipper->GetClipperType() == iClipper2D::clipperBox) ||
  	clipportal_floating)
  {
    SetCorrectStencilState ();
    // If we still need plane clipping then we must set that up too.
    if (!clip_plane && !clip_z_plane)
      return;
    // We force clip_portal to CS_CLIP_NOT here so that we only do
    // the other clipping.
    clip_portal = CS_CLIP_NOT;
  }

  // Normal clipping.
  if (cache_clip_portal == clip_portal &&
      cache_clip_plane == clip_plane &&
      cache_clip_z_plane == clip_z_plane)
  {
    SetCorrectStencilState ();
    return;
  }
  cache_clip_portal = clip_portal;
  cache_clip_plane = clip_plane;
  cache_clip_z_plane = clip_z_plane;

  clip_planes_enabled = false;

  //===========
  // First we are going to find out what kind of clipping (if any)
  // we need. This depends on various factors including what the engine
  // says about the mesh (the clip_portal and clip_plane flags in the
  // mesh), what the current clipper is (the current cliptype),
  // and what the prefered clipper (stencil or glClipPlane).
  //===========

  // If the following flag becomes true in this routine then this means
  // that for portal clipping we will use stencil.
  bool clip_with_stencil = false;
  // If the following flag becomes true in this routine then this means
  // that for portal clipping we will use glClipPlane. This flag does
  // not say anything about z-plane and near plane clipping.
  bool clip_with_planes = false;
  // If one of the following flags is true then this means
  // that we will have to do plane clipping using glClipPlane for the near
  // or z=0 plane.
  bool do_plane_clipping = (do_near_plane && clip_plane != CS_CLIP_NOT);
  bool do_z_plane_clipping = (clip_z_plane != CS_CLIP_NOT);

  bool m_prefer_stencil = (stencil_threshold >= 0) && 
    (tri_count > stencil_threshold);

  // First we see how many additional planes we might need because of
  // z-plane clipping and/or near-plane clipping. These additional planes
  // will not be usable for portal clipping (if we're using OpenGL plane
  // clipping).
  size_t reserved_planes = int (do_plane_clipping) + int (do_z_plane_clipping);

  if (clip_portal != CS_CLIP_NOT)//@@@??? && cliptype != CS_CLIPPER_OPTIONAL)
  {
    // Some clipping may be required.
    if (m_prefer_stencil)
      clip_with_stencil = true;
    else if (clipper && (clipper->GetVertexCount () > 6-reserved_planes))
    {
      if (broken_stencil || !stencil_clipping_available)
      {
        // If the stencil is broken we will clip with planes
	// even if we don't have enough planes. We will just
	// ignore the other planes then.
        clip_with_stencil = false;
        clip_with_planes = true;
      }
      else
      {
        clip_with_stencil = true;
      }
    }
    else
      clip_with_planes = true;
  }

  //===========
  // First setup the clipper that we need.
  //===========
  if (clip_with_stencil)
  {
    SetupStencil ();
    // Use the stencil area.
    EnableStencilClipping ();
  }
  else
  {
    DisableStencilClipping ();
  }

  int planes = SetupClipPlanes (clip_with_planes, do_plane_clipping,
  	do_z_plane_clipping);
  if (planes > 0)
  {
    clip_planes_enabled = true;
    for (int i = 0 ; i < planes ; i++)
      glEnable ((GLenum)(GL_CLIP_PLANE0+i));
  }
  // @@@ Hard coded max number of planes (6). Maybe not so good.
  for (int i = planes ; i < 6 ; i++)
    glDisable ((GLenum)(GL_CLIP_PLANE0+i));
}

/*void csGLGraphics3D::ApplyObjectToCamera ()
{
  GLfloat matrixholder[16];
  const csMatrix3 &orientation = object2camera.GetO2T();
  const csVector3 &translation = object2camera.GetO2TTranslation();

  matrixholder[0] = orientation.m11;
  matrixholder[1] = orientation.m21;
  matrixholder[2] = orientation.m31;
  matrixholder[3] = 0.0f;

  matrixholder[4] = orientation.m12;
  matrixholder[5] = orientation.m22;
  matrixholder[6] = orientation.m32;
  matrixholder[7] = 0.0f;

  matrixholder[8] = orientation.m13;
  matrixholder[9] = orientation.m23;
  matrixholder[10] = orientation.m33;
  matrixholder[11] = 0.0f;

  matrixholder[12] = 0.0f;
  matrixholder[13] = 0.0f;
  matrixholder[14] = 0.0f;
  matrixholder[15] = 1.0f;

  statecache->SetMatrixMode (GL_MODELVIEW);
  glLoadMatrixf (matrixholder);
  glTranslatef (-translation.x, -translation.y, -translation.z);
}
*/

void csGLGraphics3D::SetupProjection ()
{
  if (!needProjectionUpdate) return;

  statecache->SetMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  if (render_target)
    r2tbackend->SetupProjection();
  else
  {
    SetGlOrtho (false);
    //glTranslatef (asp_center_x, asp_center_y, 0);
  }
  glTranslatef (asp_center_x, asp_center_y, 0);

  GLfloat matrixholder[16];
  for (int i = 0 ; i < 16 ; i++) matrixholder[i] = 0.0;
  matrixholder[0] = matrixholder[5] = 1.0;
  matrixholder[11] = 1.0/aspect;
  matrixholder[14] = -matrixholder[11];
  glMultMatrixf (matrixholder);

  statecache->SetMatrixMode (GL_MODELVIEW);
  needProjectionUpdate = false;
}

////////////////////////////////////////////////////////////////////
// iGraphics3D
////////////////////////////////////////////////////////////////////

bool csGLGraphics3D::Open ()
{
  if (isOpen) return true;
  isOpen = true;
  csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY (
  	object_reg, iPluginManager);

  csRef<iVerbosityManager> verbosemgr (
    CS_QUERY_REGISTRY (object_reg, iVerbosityManager));
  if (verbosemgr) verbose = verbosemgr->CheckFlag ("renderer");
  if (!verbose) bugplug = 0;

  textureLodBias = config->GetFloat ("Video.OpenGL.TextureLODBias",
    -0.3f);
  if (verbose)
    Report (CS_REPORTER_SEVERITY_NOTIFY,
      "Texture LOD bias %g", textureLodBias);
 
  if (!G2D->Open ())
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error opening Graphics2D context.");
    return false;
  }

  SetPerspectiveAspect (G2D->GetHeight ());
  SetPerspectiveCenter (G2D->GetWidth ()/2, G2D->GetHeight ()/2);
  
  object_reg->Register( G2D, "iGraphics2D");

  G2D->PerformExtension ("getstatecache", &statecache);
  G2D->PerformExtension	("getextmanager", &ext);

  int w = G2D->GetWidth ();
  int h = G2D->GetHeight ();
  SetDimensions (w, h);
  asp_center_x = w/2;
  asp_center_y = h/2;

  // The extension manager requires to initialize all used extensions with
  // a call to Init<ext> first.
  ext->InitGL_version_1_2 ();
  if (ext->CS_GL_version_1_2)
    glDrawRangeElements = ext->glDrawRangeElements;
  else
    glDrawRangeElements = myDrawRangeElements;
  ext->InitGL_ARB_multitexture ();
  ext->InitGL_ARB_texture_cube_map();
  ext->InitGL_EXT_texture3D ();
  ext->InitGL_ARB_vertex_buffer_object ();
  ext->InitGL_SGIS_generate_mipmap ();
  ext->InitGL_EXT_texture_filter_anisotropic ();
  ext->InitGL_EXT_texture_lod_bias ();
  ext->InitGL_EXT_stencil_wrap ();
  ext->InitGL_EXT_stencil_two_side ();
  ext->InitGL_ARB_point_parameters ();
  ext->InitGL_ARB_point_sprite ();
  /*
    Check whether to init NVidia-only exts.
    Note: NV extensions supported by multiple vendors
     should always be inited.
   */
  if (config->GetBool ("Video.OpenGL.UseNVidiaExt", true))
  {
  }
  /*
    Check whether to init ATI-only exts.
    Note: ATI extensions supported by multiple vendors
     should always be inited.
   */
  if (config->GetBool ("Video.OpenGL.UseATIExt", true))
  {
    ext->InitGL_ATI_separate_stencil ();
  }

  rendercaps.minTexHeight = 2;
  rendercaps.minTexWidth = 2;
  GLint mts = config->GetInt ("Video.OpenGL.Caps.MaxTextureSize", -1);
  if (mts == -1)
  {
    glGetIntegerv (GL_MAX_TEXTURE_SIZE, &mts);
    if (mts <= 0)
    {
      // There appears to be a bug in some OpenGL drivers where
      // getting the maximum texture size simply doesn't work. In that
      // case we will issue a warning about this and assume 256x256.
      mts = 256;
      Report (CS_REPORTER_SEVERITY_WARNING, 
	"Detecting maximum texture size fails! 256x256 is assumed.\n"
	"Edit Video.OpenGL.Caps.MaxTextureSize if you want to specify a "
	"value.");
    }
  }
  if (verbose)
    Report (CS_REPORTER_SEVERITY_NOTIFY,
      "Maximum texture size is %dx%d", mts, mts);
  rendercaps.maxTexHeight = mts;
  rendercaps.maxTexWidth = mts;

  rendercaps.SupportsPointSprites = ext->CS_GL_ARB_point_parameters &&
    ext->CS_GL_ARB_point_sprite;
  if (verbose)
    if (rendercaps.SupportsPointSprites)
      Report (CS_REPORTER_SEVERITY_NOTIFY, "Point sprites are supported.");
    else
      Report (CS_REPORTER_SEVERITY_NOTIFY, "Point sprites are NOT supported.");

  {
    GLint abits;
    glGetIntegerv (GL_ALPHA_BITS, &abits);
    rendercaps.DestinationAlpha = abits > 0;
  }

  // check for support of VBO
  vbo_thresshold = config->GetInt ("Video.OpenGL.VBOThresshold", 0);
  use_hw_render_buffers = ext->CS_GL_ARB_vertex_buffer_object;
  if (verbose)
  {
    if (use_hw_render_buffers)
    {
      if (vbo_thresshold == 0)
        Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "VBO is supported and always used.");
      else
        Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "VBO is supported and only used for buffers > %d bytes.",
	  vbo_thresshold);
    }
    else
    {
      Report (CS_REPORTER_SEVERITY_NOTIFY, "VBO is NOT supported.");
    }
  }
  if (use_hw_render_buffers) 
    vboManager.AttachNew (new csGLVBOBufferManager (ext, statecache, object_reg));

  stencil_shadow_mask = 127;
  {
    GLint sbits;
    glGetIntegerv (GL_STENCIL_BITS, &sbits);

    stencil_clipping_available = sbits > 0;
    if (stencil_clipping_available)
      stencil_clip_value = stencil_clip_mask = 1 << (sbits - 1);
    else
      stencil_clip_value = stencil_clip_mask = 0;
    if ((rendercaps.StencilShadows = (sbits > 1)))
    {
      stencil_shadow_mask = (1 << (sbits - 1)) - 1;
    }
  }

  stencil_threshold = config->GetInt ("Video.OpenGL.StencilThreshold", 500);
  broken_stencil = false;
  if (config->GetBool ("Video.OpenGL.BrokenStencil", false))
  {
    broken_stencil = true;
    stencil_threshold = -1;
  }
  if (verbose)
    if (broken_stencil)
      Report (CS_REPORTER_SEVERITY_NOTIFY, "Stencil clipping is broken!");
    else if (!stencil_clipping_available)
      Report (CS_REPORTER_SEVERITY_NOTIFY, "Stencil clipping is not available");
    else
    {
      if (stencil_threshold >= 0)
      {
	Report (CS_REPORTER_SEVERITY_NOTIFY, 
	  "Stencil clipping is used for objects >= %d triangles.", 
	  stencil_threshold);
      }
      else
      {
	Report (CS_REPORTER_SEVERITY_NOTIFY, 
	  "Plane clipping is preferred.");
      }
    }

  stencilClearWithZ = config->GetBool ("Video.OpenGL.StencilClearWithZ", true);
  if (verbose)
    Report (CS_REPORTER_SEVERITY_NOTIFY, 
    "Clearing Z buffer when stencil clear is needed %s", 
    stencilClearWithZ ? "enabled" : "disabled");

  CS_QUERY_REGISTRY_PLUGIN(shadermgr, object_reg,
    "crystalspace.graphics3d.shadermanager", iShaderManager);

  txtmgr.AttachNew (new csGLTextureManager (
    object_reg, GetDriver2D (), config, this));

  glClearDepth (0.0);
  statecache->Enable_GL_CULL_FACE ();
  statecache->SetCullFace (GL_FRONT);

  statecache->SetStencilMask (stencil_shadow_mask);

  // Set up texture LOD bias.
  if (ext->CS_GL_EXT_texture_lod_bias)
  {
    if (ext->CS_GL_ARB_multitexture)
    {
      GLint texUnits;
      glGetIntegerv (GL_MAX_TEXTURE_UNITS_ARB, &texUnits);
      for (int u = texUnits - 1; u >= 0; u--)
      {
	statecache->SetActiveTU (u);
        statecache->ActivateTU ();
        glTexEnvf (GL_TEXTURE_FILTER_CONTROL_EXT, 
	  GL_TEXTURE_LOD_BIAS_EXT, textureLodBias); 
      }
    }
    else
    {
      glTexEnvf (GL_TEXTURE_FILTER_CONTROL_EXT, 
	GL_TEXTURE_LOD_BIAS_EXT, textureLodBias); 
    }
  }

  string_vertices = strings->Request ("vertices");
  string_texture_coordinates = strings->Request ("texture coordinates");
  string_normals = strings->Request ("normals");
  string_colors = strings->Request ("colors");
  string_indices = strings->Request ("indices");
  string_point_radius = strings->Request ("point radius");
  string_point_scale = strings->Request ("point scale");
  string_texture_diffuse = strings->Request (CS_MATERIAL_TEXTURE_DIFFUSE);
  string_world2camera = strings->Request ("world2camera transform");
  string_object2world = strings->Request ("object2world transform");

  /* @@@ All those default textures, better put them into the engine? */

  // @@@ These shouldn't be here, I guess.
  #ifdef CS_FOGTABLE_SIZE
  #undef CS_FOGTABLE_SIZE
  #endif
  #define CS_FOGTABLE_SIZE 256
  // Each texel in the fog table holds the fog alpha value at a certain
  // (distance*density).  The median distance parameter determines the
  // (distance*density) value represented by the texel at the center of
  // the fog table.  The fog calculation is:
  // alpha = 1.0 - exp( -(density*distance) / CS_FOGTABLE_MEDIANDISTANCE)
  #define CS_FOGTABLE_MEDIANDISTANCE 10.0f
  #define CS_FOGTABLE_MAXDISTANCE (CS_FOGTABLE_MEDIANDISTANCE * 2.0f)
  #define CS_FOGTABLE_DISTANCESCALE (1.0f / CS_FOGTABLE_MAXDISTANCE)

  unsigned char *transientfogdata = new unsigned char[CS_FOGTABLE_SIZE * CS_FOGTABLE_SIZE * 4];
  memset(transientfogdata, 255, CS_FOGTABLE_SIZE * CS_FOGTABLE_SIZE * 4);
  for (unsigned int fogindex1 = 0; fogindex1 < CS_FOGTABLE_SIZE; fogindex1++)
  {
    for (unsigned int fogindex2 = 0; fogindex2 < CS_FOGTABLE_SIZE; fogindex2++)
    {
      unsigned char fogalpha1 = (unsigned char)(255 * csFogRamp((float)fogindex1 / CS_FOGTABLE_SIZE));
      if (fogindex1 == (CS_FOGTABLE_SIZE - 1))
        fogalpha1 = 255;
      unsigned char fogalpha2 = (unsigned char)(255 * csFogRamp((float)fogindex2 / CS_FOGTABLE_SIZE));
      if (fogindex2 == (CS_FOGTABLE_SIZE - 1))
        fogalpha2 = 255;
      transientfogdata[(fogindex1+fogindex2*CS_FOGTABLE_SIZE) * 4 + 3] = MIN(fogalpha1, fogalpha2);
    }
  }

  csRef<iImage> img = csPtr<iImage> (new csImageMemory (
    CS_FOGTABLE_SIZE, CS_FOGTABLE_SIZE, transientfogdata, true, 
    CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA));
  csRef<iTextureHandle> fogtex = txtmgr->RegisterTexture (
    img, CS_TEXTURE_3D | CS_TEXTURE_CLAMP | CS_TEXTURE_NOMIPMAPS);

  csRef<csShaderVariable> fogvar = csPtr<csShaderVariable> (
  	new csShaderVariable (strings->Request ("standardtex fog")));
  fogvar->SetValue (fogtex);
  shadermgr->AddVariable(fogvar);

  {
    const int normalizeCubeSize = config->GetInt (
      "Video.OpenGL.NormalizeCubeSize", 256);

    csRef<csShaderVariable> normvar = 
      csPtr<csShaderVariable> (new csShaderVariable (
      strings->Request ("standardtex normalization map")));
    csRef<iShaderVariableAccessor> normCube;
    normCube.AttachNew (new csNormalizationCubeAccessor (txtmgr, 
      normalizeCubeSize));
    normvar->SetAccessor (normCube);
    shadermgr->AddVariable(normvar);
  }

  {
    csRGBpixel* white = new csRGBpixel[1];
    white->Set (255, 255, 255);
    img = csPtr<iImage> (new csImageMemory (1, 1, white, true, 
      CS_IMGFMT_TRUECOLOR));

    csRef<iTextureHandle> whitetex = txtmgr->RegisterTexture (
      img, CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS);

    csRef<csShaderVariable> whitevar = csPtr<csShaderVariable> (
      new csShaderVariable (
      strings->Request ("standardtex white")));
    whitevar->SetValue (whitetex);
    shadermgr->AddVariable (whitevar);
  }

  cache_clip_portal = -1;
  cache_clip_plane = -1;
  cache_clip_z_plane = -1;

  r2tbackend = new csGLRender2TextureFramebuf (this);
  
  return true;
}

void csGLGraphics3D::Close ()
{
  if (!isOpen) return;

  glFinish ();

  if (txtmgr)
  {
    txtmgr->Clear ();
    //delete txtmgr; txtmgr = 0;
  }
  txtmgr = 0;
  shadermgr = 0;
  delete r2tbackend; r2tbackend = 0;
  for (size_t h = 0; h < halos.Length(); h++)
  {
    if (halos[h]) halos[h]->DeleteTexture();
  }

  if (G2D)
    G2D->Close ();
}

bool csGLGraphics3D::BeginDraw (int drawflags)
{
  SetWriteMask (true, true, true, true);

  clipportal_dirty = true;
  clipportal_floating = 0;
  CS_ASSERT (clipportal_stack.Length () == 0);

  debug_inhibit_draw = false;

  int i = 0;
  for (i = 15; i >= 0; i--)
    DeactivateTexture (i);

  // if 2D graphics is not locked, lock it
  if ((drawflags & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
   && (!(current_drawflags & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))))
  {
    if (!G2D->BeginDraw ())
      return false;
  }
  const int old_drawflags = current_drawflags;
  current_drawflags = drawflags;

  if (render_target)
    r2tbackend->BeginDrawCommon (); 

  int clearMask;
  const bool doStencilClear = 
    (drawflags & CSDRAW_3DGRAPHICS) && stencil_clipping_available;
  const bool doZbufferClear = (drawflags & CSDRAW_CLEARZBUFFER)
    || (doStencilClear && stencilClearWithZ);
  if (doZbufferClear)
  {
    const GLbitfield stencilFlag = 
      stencil_clipping_available ? GL_STENCIL_BUFFER_BIT : 0;
    statecache->SetDepthMask (GL_TRUE);
    if (drawflags & CSDRAW_CLEARSCREEN)
      clearMask = GL_DEPTH_BUFFER_BIT | stencilFlag
      	| GL_COLOR_BUFFER_BIT;
    else
      clearMask = GL_DEPTH_BUFFER_BIT | stencilFlag;
  }
  else if (drawflags & CSDRAW_CLEARSCREEN)
    clearMask = GL_COLOR_BUFFER_BIT;
  else if (doStencilClear)
    clearMask = GL_STENCIL_BUFFER_BIT;
#ifndef DELAYED_SWAP
  glClear (clearMask);
#else
  delayClearFlags = clearMask;
#endif

  if (drawflags & CSDRAW_3DGRAPHICS)
  {
    glViewport (0, 0, viewwidth, viewheight);
    needProjectionUpdate = true;

    if (render_target)
      r2tbackend->BeginDraw3D (); 
    //statecache->SetCullFace (render_target ? GL_BACK : GL_FRONT);

    statecache->SetMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
//    object2camera.Identity ();
    //@@@ TODO FIX
    return true;
  }
  else if (drawflags & CSDRAW_2DGRAPHICS)
  {
    SwapIfNeeded();
    // Don't set up the 2D stuff if we already are in 2D mode
    if (!(old_drawflags & CSDRAW_2DGRAPHICS))
    {
      /*
	Turn off some stuff that isn't needed for 2d (or even can
	cause visual glitches.)
      */
      if (use_hw_render_buffers)
      {
	ext->glBindBufferARB (GL_ARRAY_BUFFER_ARB, 0);
	ext->glBindBufferARB (GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
      }
      statecache->Disable_GL_ALPHA_TEST ();
      if (ext->CS_GL_ARB_multitexture)
      {
	statecache->SetActiveTU (0);
        statecache->ActivateTU ();
      }

      statecache->SetMatrixMode (GL_PROJECTION);
      glLoadIdentity ();
      if (render_target)
	r2tbackend->BeginDraw2D (); 
      else
	SetGlOrtho (false);
      //glViewport (0, 0, viewwidth, viewheight);
      glViewport (0, 0, viewwidth, viewheight);
      needProjectionUpdate = false; 
      /* Explicitly avoid update. The 3D mode projection will not work in
       * 2D mode. */

      statecache->SetMatrixMode (GL_MODELVIEW);
      glLoadIdentity ();
//      object2camera.Identity ();
      //@@@ TODO FIX

      SetZMode (CS_ZBUF_NONE);
      
      SetMixMode (CS_FX_ALPHA); 
      // So alpha blending works w/ 2D drawing
      glColor4f (1.0f, 1.0f, 1.0f, 1.0f);
    }
    return true;//G2D->BeginDraw ();
  }

  current_drawflags = 0;
  return false;
}

void csGLGraphics3D::PrepareAsRenderTarget (csGLTextureHandle* tex_mm)
{
  // Texture was not used as a render target before.
  // Make some necessary adjustments.
  if (!tex_mm->IsWasRenderTarget())
  {
    // Pull texture data and set as RGBA again, to prevent compression (slooow)
    // on subsequent glTexSubImage() calls.
    uint8* pixels = new uint8[tex_mm->actual_width * tex_mm->actual_height * 4];
    glGetTexImage (GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    if (!(tex_mm->GetFlags() & CS_TEXTURE_NOMIPMAPS))
    {
      if (ext->CS_GL_SGIS_generate_mipmap)
      {
        glTexParameteri (GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
      }
      else
      {
	glTexParameteri  (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
	  txtmgr->rstate_bilinearmap ? GL_LINEAR : GL_NEAREST);
      }
    }
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, tex_mm->actual_width, 
      tex_mm->actual_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    delete[] pixels;
    tex_mm->SetWasRenderTarget (true);
  }
}

void csGLGraphics3D::FinishDraw ()
{
  if (current_drawflags & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
    G2D->FinishDraw ();

  if (render_target)
  {
    r2tbackend->FinishDraw();
    SetRenderTarget (0);
  }
  
  SetMirrorMode (false);

  //render_target = 0;
  current_drawflags = 0;
}

void csGLGraphics3D::Print (csRect const* area)
{
  //glFinish ();
  if (bugplug)
    bugplug->ResetCounter ("Triangle Count");

  if (vboManager.IsValid ())
  {
    vboManager->ResetFrameStats ();
  }

#ifdef DELAYED_SWAP
  if (area == 0)
  {
    wantToSwap = true;
    return;
  }
  SwapIfNeeded();
#endif
  G2D->Print (area);
}

void csGLGraphics3D::DrawLine (const csVector3 & v1, const csVector3 & v2,
	float fov, int color)
{
  SwapIfNeeded();

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
  int px1 = csQint (x1 * iz1 + (viewwidth / 2));
  int py1 = viewheight - 1 - csQint (y1 * iz1 + (viewheight / 2));
  float iz2 = fov / z2;
  int px2 = csQint (x2 * iz2 + (viewwidth / 2));
  int py2 = viewheight - 1 - csQint (y2 * iz2 + (viewheight / 2));

  G2D->DrawLine (px1, py1, px2, py2, color);
}


bool csGLGraphics3D::ActivateBuffers (csRenderBufferHolder *holder, 
                                      csRenderBufferName mapping[CS_VATTRIB_SPECIFIC_LAST+1])
{
  if (!holder) return false;

  iRenderBuffer *buffer = 0;
  void *data = 0;

  buffer = holder->GetRenderBuffer (mapping[CS_VATTRIB_POSITION]);
  if (buffer) 
  {
    //csGLRenderBuffer* glbuffer = (csGLRenderBuffer*)buffer;
    AssignSpecBuffer (CS_VATTRIB_POSITION, buffer);
    GLenum compType;
    data = //glbuffer->RenderLock (CS_GLBUF_RENDERLOCK_ARRAY);
      RenderLock (buffer, CS_GLBUF_RENDERLOCK_ARRAY, compType);
    if (data != (void*)-1) 
    {
      statecache->SetVertexPointer (buffer->GetComponentCount (),
        compType, (GLsizei)buffer->GetStride (), data);
      statecache->Enable_GL_VERTEX_ARRAY ();
    }
  }
  else
  {
    statecache->Disable_GL_VERTEX_ARRAY ();
  }

  buffer = holder->GetRenderBuffer (mapping[CS_VATTRIB_NORMAL]);
  if (buffer) 
  {
    //csGLRenderBuffer* glbuffer = (csGLRenderBuffer*)buffer;
    AssignSpecBuffer (CS_VATTRIB_NORMAL, buffer);
    GLenum compType;
    data = //glbuffer->RenderLock (CS_GLBUF_RENDERLOCK_ARRAY);
      RenderLock (buffer, CS_GLBUF_RENDERLOCK_ARRAY, compType);
    if (data != (void*)-1) 
    {
      statecache->SetNormalPointer (compType, 
        (GLsizei)buffer->GetStride(), data);
      statecache->Enable_GL_NORMAL_ARRAY ();
    }
  }
  else
  {
    statecache->Disable_GL_NORMAL_ARRAY ();
  }
  
  buffer = holder->GetRenderBuffer (mapping[CS_VATTRIB_COLOR]);
  if (buffer)
  {
    //csGLRenderBuffer* glbuffer = (csGLRenderBuffer*)buffer;
    AssignSpecBuffer (CS_VATTRIB_COLOR, buffer);
    GLenum compType;
    data = //glbuffer->RenderLock (CS_GLBUF_RENDERLOCK_ARRAY);
      RenderLock (buffer, CS_GLBUF_RENDERLOCK_ARRAY, compType);
    if (data != (void*)-1) 
    {
      statecache->SetColorPointer (buffer->GetComponentCount(),
        compType, (GLsizei)buffer->GetStride (), data);
      statecache->Enable_GL_COLOR_ARRAY ();
    }
  }
  else
  {
    statecache->Disable_GL_COLOR_ARRAY ();
  }

  buffer = holder->GetRenderBuffer (mapping[CS_VATTRIB_TEXCOORD0]);
  if (buffer)
  {
    AssignSpecBuffer (CS_VATTRIB_TEXCOORD0, buffer);
    GLenum compType;
    data = 
      RenderLock (buffer, CS_GLBUF_RENDERLOCK_ARRAY, compType);
    if (data != (void*)-1)
    {
      if (ext->CS_GL_ARB_multitexture)
      {
        statecache->SetActiveTU (0);
      }
      statecache->SetTexCoordPointer (buffer->GetComponentCount (),
        compType, (GLsizei)buffer->GetStride (), data);
      statecache->Enable_GL_TEXTURE_COORD_ARRAY ();
    }
  }
  else
  {
    if (ext->CS_GL_ARB_multitexture)
    {
      statecache->SetActiveTU (0);
    }
    statecache->Disable_GL_TEXTURE_COORD_ARRAY ();
  }

  if (ext->CS_GL_ARB_multitexture)
  {
    unsigned int i;
    for (i = 1; i < 8; i++)
    {
      buffer = holder->GetRenderBuffer (mapping[CS_VATTRIB_TEXCOORD0+i]);
      if (buffer)
      {
        AssignSpecBuffer (CS_VATTRIB_TEXCOORD0+i, buffer);
	GLenum compType;
	data = 
	  RenderLock (buffer, CS_GLBUF_RENDERLOCK_ARRAY, compType);
        if (data != (void*)-1)
        {
          statecache->SetActiveTU (i);
          statecache->SetTexCoordPointer (buffer->GetComponentCount (),
            compType, (GLsizei)buffer->GetStride (), data);
          statecache->Enable_GL_TEXTURE_COORD_ARRAY ();
        }
      }
      else
      {
        statecache->SetActiveTU (i);
        statecache->Disable_GL_TEXTURE_COORD_ARRAY ();
      }
    }
  }
  return true;
}

bool csGLGraphics3D::ActivateBuffers (csVertexAttrib *attribs, 
                                      iRenderBuffer** buffers, unsigned int count)
{
  for (unsigned int i = 0; i < count; i++)
  {
    csVertexAttrib att = attribs[i];
    iRenderBuffer *buffer = buffers[i];
    if (!buffer) continue;

    //csGLRenderBuffer* glbuffer = (csGLRenderBuffer*)buffer;
    
    if (CS_VATTRIB_IS_GENERIC (att)) 
      AssignGenericBuffer (att-CS_VATTRIB_GENERIC_FIRST, buffer);
    else 
      AssignSpecBuffer (att-CS_VATTRIB_SPECIFIC_FIRST, buffer);

    GLenum compType;
    void *data = //glbuffer->RenderLock (CS_GLBUF_RENDERLOCK_ARRAY);
      RenderLock (buffer, CS_GLBUF_RENDERLOCK_ARRAY, compType);

    if (data == (void*)-1) return false;

    switch (att)
    {
    case CS_VATTRIB_POSITION:
      statecache->Enable_GL_VERTEX_ARRAY ();
      statecache->SetVertexPointer (buffer->GetComponentCount (),
        compType, (GLsizei)buffer->GetStride (), data);
      break;
    case CS_VATTRIB_NORMAL:
      statecache->Enable_GL_NORMAL_ARRAY ();
      statecache->SetNormalPointer (compType, (GLsizei)buffer->GetStride (), 
	data);
      break;
    case CS_VATTRIB_COLOR:
      statecache->Enable_GL_COLOR_ARRAY ();
      statecache->SetColorPointer (buffer->GetComponentCount (),
        compType, (GLsizei)buffer->GetStride (), data);
      break;
    default:
      if (att >= CS_VATTRIB_TEXCOORD0 && att <= CS_VATTRIB_TEXCOORD7)
      {
        //texcoord
        unsigned int unit = att- CS_VATTRIB_TEXCOORD0;
        if (ext->CS_GL_ARB_multitexture)
        {
          statecache->SetActiveTU (unit);
        } 
        statecache->Enable_GL_TEXTURE_COORD_ARRAY ();
        statecache->SetTexCoordPointer (buffer->GetComponentCount (),
          compType, (GLsizei)buffer->GetStride (), data);
      }
      else if (CS_VATTRIB_IS_GENERIC(att) && ext->glEnableVertexAttribArrayARB)
      {
        ext->glEnableVertexAttribArrayARB (att);
        ext->glVertexAttribPointerARB(att, buffer->GetComponentCount (),
          compType, false, (GLsizei)buffer->GetStride (), data);
      }
      else
      {
        //none, assert...
        CS_ASSERT_MSG("Unknown vertex attribute", 0);
      }
    }
  }
  return true;
}

void csGLGraphics3D::DeactivateBuffers (csVertexAttrib *attribs, unsigned int count)
{
  if (vboManager) vboManager->DeactivateVBO ();
  unsigned int i;
  if (!attribs)
  {
    //disable all
    statecache->Disable_GL_VERTEX_ARRAY ();
    statecache->Disable_GL_NORMAL_ARRAY ();
    statecache->Disable_GL_COLOR_ARRAY ();
    statecache->Disable_GL_TEXTURE_COORD_ARRAY ();
    if (ext->CS_GL_ARB_multitexture)
    {
      for (i = 0; i < CS_GL_MAX_LAYER; i++)
      {
        statecache->SetActiveTU (i);
        statecache->Disable_GL_TEXTURE_COORD_ARRAY ();
      }
    }

    for (i = 0; i < CS_VATTRIB_SPECIFIC_LAST-CS_VATTRIB_SPECIFIC_FIRST+1; i++)
    {
      iRenderBuffer *b = spec_renderBuffers[i];
      if (b) RenderRelease (b);// b->RenderRelease ();
      spec_renderBuffers[i] = 0;
    }
    for (i = 0; i < CS_VATTRIB_GENERIC_LAST-CS_VATTRIB_GENERIC_FIRST+1; i++)
    {
      iRenderBuffer *b = gen_renderBuffers[i];
      if (b) RenderRelease (b);// b->RenderRelease ();
      gen_renderBuffers[i] = 0;
    }
  }
  else
  {
    for (i = 0; i < count; i++)
    {
      csVertexAttrib att = attribs[i];
      if (CS_VATTRIB_IS_GENERIC (att))
      {
        if (gen_renderBuffers[att]) 
        {
          RenderRelease (gen_renderBuffers[att]);
          gen_renderBuffers[att] = 0;
        }
      }
      else
      {
        if (spec_renderBuffers[att]) 
        {
          RenderRelease (spec_renderBuffers[att]);
          spec_renderBuffers[att] = 0;
        }
      }
      switch (att)
      {
      case CS_VATTRIB_POSITION:
        statecache->Disable_GL_VERTEX_ARRAY ();
        break;
      case CS_VATTRIB_NORMAL:
        statecache->Disable_GL_NORMAL_ARRAY ();
        break;
      case CS_VATTRIB_COLOR:
        statecache->Disable_GL_COLOR_ARRAY ();
        break;
      default:
        if (att >= CS_VATTRIB_TEXCOORD0 && att <= CS_VATTRIB_TEXCOORD7)
        {
          //texcoord
          unsigned int unit = att- CS_VATTRIB_TEXCOORD0;
          if (ext->CS_GL_ARB_multitexture)
          {
            statecache->SetActiveTU (unit);
          }
          statecache->Disable_GL_TEXTURE_COORD_ARRAY ();
        }
        else if (CS_VATTRIB_IS_GENERIC(att) && ext->glDisableVertexAttribArrayARB)
        {
          ext->glDisableVertexAttribArrayARB (att);
        }
        else
        {
          //none, assert...
          CS_ASSERT_MSG("Unknown vertex attribute", 0);
        }
      }
    }
  }
}

bool csGLGraphics3D::ActivateTexture (iTextureHandle *txthandle, int unit)
{
  if (ext->CS_GL_ARB_multitexture)
  {
    statecache->SetActiveTU (unit);
    statecache->ActivateTU ();
  }
  else if (unit != 0) return false;

  csGLTextureHandle* gltxthandle = (csGLTextureHandle*)txthandle;
  GLuint texHandle = gltxthandle->GetHandle ();

  switch (gltxthandle->target)
  {
    case iTextureHandle::CS_TEX_IMG_1D:
      statecache->Enable_GL_TEXTURE_1D ();
      statecache->SetTexture (GL_TEXTURE_1D, texHandle);
      break;
    case iTextureHandle::CS_TEX_IMG_2D:
      statecache->Enable_GL_TEXTURE_2D ();
      statecache->SetTexture (GL_TEXTURE_2D, texHandle);
      break;
    case iTextureHandle::CS_TEX_IMG_3D:
      statecache->Enable_GL_TEXTURE_3D ();
      statecache->SetTexture (GL_TEXTURE_3D, texHandle);
      break;
    case iTextureHandle::CS_TEX_IMG_CUBEMAP:
      statecache->Enable_GL_TEXTURE_CUBE_MAP ();
      statecache->SetTexture (GL_TEXTURE_CUBE_MAP, texHandle);
      break;
    default:
      DeactivateTexture (unit);
      return false;
  }
  /*texunitenabled[unit] = true;
  texunittarget[unit] = gltxthandle->target;*/
  return true;
}

void csGLGraphics3D::DeactivateTexture (int unit)
{
  /*if (!texunitenabled[unit])
    return;*/

  if (ext->CS_GL_ARB_multitexture)
  {
    statecache->SetActiveTU (unit);
  }
  else if (unit != 0) return;

  /*switch (texunittarget[unit])
  {
    case iTextureHandle::CS_TEX_IMG_1D:
      statecache->Disable_GL_TEXTURE_1D ();
      break;
    case iTextureHandle::CS_TEX_IMG_2D:
      statecache->Disable_GL_TEXTURE_2D ();
      break;
    case iTextureHandle::CS_TEX_IMG_3D:
      statecache->Disable_GL_TEXTURE_3D ();
      break;
    case iTextureHandle::CS_TEX_IMG_CUBEMAP:
      statecache->Disable_GL_TEXTURE_CUBE_MAP ();
      break;
  }*/

  statecache->Disable_GL_TEXTURE_1D ();
  statecache->Disable_GL_TEXTURE_2D ();
  statecache->Disable_GL_TEXTURE_3D ();
  statecache->Disable_GL_TEXTURE_CUBE_MAP ();

  texunitenabled[unit] = false;
}

void csGLGraphics3D::SetTextureState (int* units, iTextureHandle** textures,
	int count)
{
  int i;
  int unit = 0;
  for (i = 0 ; i < count ; i++)
  {
    unit = units[i];
    iTextureHandle* txt = textures[i];
    if (txt)
      ActivateTexture (txt, unit);
    else
      DeactivateTexture (unit);
  }
}

GLvoid csGLGraphics3D::myDrawRangeElements (GLenum mode, GLuint start, 
    GLuint end, GLsizei count, GLenum type, const GLvoid* indices)
{
  glDrawElements (mode, count, type, indices);
}

void makeGLMatrix (const csReversibleTransform& t, float matrix[16])
{
  const csMatrix3 &orientation = t.GetO2T();
  const csVector3 &translation = t.GetO2TTranslation();

  matrix[0] = orientation.m11;
  matrix[1] = orientation.m12;
  matrix[2] = orientation.m13;
  matrix[3] = 0.0f;

  matrix[4] = orientation.m21;
  matrix[5] = orientation.m22;
  matrix[6] = orientation.m23;
  matrix[7] = 0.0f;

  matrix[8] = orientation.m31;
  matrix[9] = orientation.m32;
  matrix[10] = orientation.m33;
  matrix[11] = 0.0f;

  matrix[12] = translation.x;
  matrix[13] = translation.y;
  matrix[14] = translation.z;
  matrix[15] = 1.0f;
}

void csGLGraphics3D::SetWorldToCamera (const csReversibleTransform& w2c)
{
  world2camera = w2c;
  float m[16];

  shadermgr->GetVariableAdd (string_world2camera)->SetValue (w2c);

  makeGLMatrix (world2camera.GetInverse (), m);
  statecache->SetMatrixMode (GL_MODELVIEW);
  glLoadMatrixf (m);
}

void csGLGraphics3D::DrawMesh (const csCoreRenderMesh* mymesh,
    const csRenderMeshModes& modes,
    const csArray< csArray<csShaderVariable*> > &stacks)
{
  if (cliptype == CS_CLIPPER_EMPTY) 
    return;

  SwapIfNeeded();

  SetupProjection ();

  int num_tri = 0;

  SetupClipper (mymesh->clip_portal, 
                mymesh->clip_plane, 
                mymesh->clip_z_plane,
		num_tri);
  if (debug_inhibit_draw) 
    return;

  csReversibleTransform o2w;

  CS_ASSERT (string_object2world<(csStringID)stacks.Length () 
      && stacks[string_object2world].Length () > 0);
    
  stacks[string_object2world].Top ()->GetValue (o2w);
  

  float matrix[16];
  makeGLMatrix (o2w, matrix);
  statecache->SetMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  glMultMatrixf (matrix);

    
  iRenderBuffer* iIndexbuf = (modes.buffers ? modes.buffers->GetRenderBuffer(CS_BUFFER_INDEX) : 0);
  
  if (!iIndexbuf)
  {
    CS_ASSERT (string_indices<(csStringID)stacks.Length ()
      && stacks[string_indices].Length () > 0);
    csShaderVariable* indexBufSV = stacks[string_indices].Top ();
    indexBufSV->GetValue (iIndexbuf);
    CS_ASSERT(iIndexbuf);
  }
  
  const size_t indexCompsBytes = 
    csRenderBufferComponentSizes[iIndexbuf->GetComponentType()];
  CS_ASSERT_MSG("Expecting index buffers to have only 1 component",
    (iIndexbuf->GetComponentCount() == 1));
  CS_ASSERT((indexCompsBytes * mymesh->indexstart) <= iIndexbuf->GetSize());
  CS_ASSERT((indexCompsBytes * mymesh->indexend) <= iIndexbuf->GetSize());

  GLenum primitivetype = GL_TRIANGLES;
  switch (mymesh->meshtype)
  {
    case CS_MESHTYPE_QUADS:
      num_tri = (mymesh->indexend-mymesh->indexstart)/2;
      primitivetype = GL_QUADS;
      break;
    case CS_MESHTYPE_TRIANGLESTRIP:
      num_tri = (mymesh->indexend-mymesh->indexstart)-2;
      primitivetype = GL_TRIANGLE_STRIP;
      break;
    case CS_MESHTYPE_TRIANGLEFAN:
      num_tri = (mymesh->indexend-mymesh->indexstart)-2;
      primitivetype = GL_TRIANGLE_FAN;
      break;
    case CS_MESHTYPE_POINTS:
      primitivetype = GL_POINTS;
      num_tri = (mymesh->indexend-mymesh->indexstart);
      break;
    case CS_MESHTYPE_POINT_SPRITES:
    {
      num_tri = (mymesh->indexend-mymesh->indexstart);
      if(!(ext->CS_GL_ARB_point_sprite && ext->CS_GL_ARB_point_parameters))
      {
        break;
      }
      float radius, scale;
      CS_ASSERT (string_point_radius<(csStringID)stacks.Length ()
          && stacks[string_point_radius].Length () > 0);
      csShaderVariable* radiusSV = stacks[string_point_radius].Top ();
      radiusSV->GetValue (radius);

      CS_ASSERT (string_point_scale < (csStringID)stacks.Length ()
          && stacks[string_point_scale].Length () > 0);
      csShaderVariable* scaleSV = stacks[string_point_scale].Top ();
      scaleSV->GetValue (scale);

      glPointSize (1.0f);
      GLfloat atten[3] = {0.0f, 0.0f, scale * scale};
      ext->glPointParameterfvARB (GL_POINT_DISTANCE_ATTENUATION_ARB, atten);
      ext->glPointParameterfARB (GL_POINT_SIZE_MAX_ARB, 9999.0f);
      ext->glPointParameterfARB (GL_POINT_SIZE_MIN_ARB, 0.0f);
      ext->glPointParameterfARB (GL_POINT_FADE_THRESHOLD_SIZE_ARB, 1.0f);

      glEnable (GL_POINT_SPRITE_ARB);
      primitivetype = GL_POINTS;
      statecache->SetActiveTU (0);
      statecache->ActivateTU ();
      glTexEnvi (GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);

      break;
    }
    case CS_MESHTYPE_LINES:
      num_tri = (mymesh->indexend-mymesh->indexstart)/2;
      primitivetype = GL_LINES;
      break;
    case CS_MESHTYPE_LINESTRIP:
      num_tri = (mymesh->indexend-mymesh->indexstart)-1;
      primitivetype = GL_LINE_STRIP;
      break;
    case CS_MESHTYPE_POLYGON:
    case CS_MESHTYPE_TRIANGLES:
    default:
      num_tri = (mymesh->indexend-mymesh->indexstart)/3;
      primitivetype = GL_TRIANGLES;
      break;
  }

  // Based on the kind of clipping we need we set or clip mask.
  int clip_mask, clip_value;
  if (clipportal_floating)
  {
    clip_mask = stencil_clip_mask;
    clip_value = stencil_clip_value;
  }
  else if (clipping_stencil_enabled)
  {
    clip_mask = stencil_clip_mask;
    clip_value = 0;
  }
  else
  {
    clip_mask = 0;
    clip_value = 0;
  }

  switch (current_shadow_state)
  {
    case CS_SHADOW_VOLUME_PASS1:
      statecache->SetStencilOp (GL_KEEP, GL_KEEP, GL_INCR);
      statecache->SetStencilFunc (GL_ALWAYS, clip_value, clip_mask);
      break;
    case CS_SHADOW_VOLUME_FAIL1:
      statecache->SetStencilOp (GL_KEEP, GL_INCR, GL_KEEP);
      statecache->SetStencilFunc (GL_ALWAYS, clip_value, clip_mask);
      break;
    case CS_SHADOW_VOLUME_PASS2:
      statecache->SetStencilOp (GL_KEEP, GL_KEEP, GL_DECR);
      statecache->SetStencilFunc (GL_ALWAYS, clip_value, clip_mask);
      break;
    case CS_SHADOW_VOLUME_FAIL2:
      statecache->SetStencilOp (GL_KEEP, GL_DECR, GL_KEEP);
      statecache->SetStencilFunc (GL_ALWAYS, clip_value, clip_mask);
      break;
    case CS_SHADOW_VOLUME_USE:
      statecache->SetStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
      statecache->SetStencilFunc (GL_EQUAL, clip_value, stencil_shadow_mask | clip_mask);
      break;
    default:
      if (clip_mask)
      {
        statecache->SetStencilFunc (GL_EQUAL, clip_value, clip_mask);
        statecache->SetStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
      }
  }

  /*if (current_shadow_state == CS_SHADOW_VOLUME_PASS2 ||
      current_shadow_state == CS_SHADOW_VOLUME_FAIL1)
    SetMirrorMode (!mymesh->do_mirror);
  else
    SetMirrorMode (mymesh->do_mirror);*/


  const uint mixmode = modes.mixmode;
  statecache->SetShadeModel ((mixmode & CS_FX_FLAT) ? GL_FLAT : GL_SMOOTH);


  GLenum compType;
  void* bufData = //indexbuf->RenderLock (CS_GLBUF_RENDERLOCK_ELEMENTS);
    RenderLock (iIndexbuf, CS_GLBUF_RENDERLOCK_ELEMENTS, compType);
  if (bufData != (void*)-1)
  {
    if ((mixmode & CS_FX_MASK_MIXMODE) != CS_FX_COPY)
      SetMixMode (mixmode);
    else
      SetAlphaType (modes.alphaType);

    if (bugplug)
    {
      bugplug->AddCounter ("Triangle Count", num_tri);
      bugplug->AddCounter ("Mesh Count", 1);
    }

    if ((current_zmode == CS_ZBUF_MESH) || (current_zmode == CS_ZBUF_MESH2))
    {
      CS_ASSERT_MSG ("Meshes can't have zmesh zmode. You deserve some spanking", 
	(modes.z_buf_mode != CS_ZBUF_MESH) && 
	(modes.z_buf_mode != CS_ZBUF_MESH2));
        SetZModeInternal ((current_zmode == CS_ZBUF_MESH2) ? 
	  GetZModePass2 (modes.z_buf_mode) : modes.z_buf_mode);
/*      if (mymesh->z_buf_mode != CS_ZBUF_MESH)
      {
        SetZMode (mymesh->z_buf_mode, true);
      } else {
        // Should do some real reporting here. :)
        csPrintf ("Bwaaaah! Meshes can't have zmesh zmode. You deserve some spanking!\n");
        return;
      }*/
    }

    float alpha = 1.0f;
    if ((mixmode & CS_FX_MASK_MIXMODE) == CS_FX_ALPHA)
      alpha = (float)(mixmode & CS_FX_MASK_ALPHA) / 255.0f;
    glColor4f (1.0f, 1.0f, 1.0f, alpha);
    glDrawRangeElements (primitivetype, (GLuint)iIndexbuf->GetRangeStart(), 
      (GLuint)iIndexbuf->GetRangeEnd(), mymesh->indexend - mymesh->indexstart,
      compType, 
      ((uint8*)bufData) + (indexCompsBytes * mymesh->indexstart));
    //indexbuf->Release();
  }

  if (mymesh->meshtype == CS_MESHTYPE_POINT_SPRITES) 
  {
    glTexEnvi (GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_FALSE);
    glDisable (GL_POINT_SPRITE_ARB);
  }
  glPopMatrix ();
  //indexbuf->RenderRelease ();
  RenderRelease (iIndexbuf);
  //SetMirrorMode (false);
}

void csGLGraphics3D::DrawPixmap (iTextureHandle *hTex,
  int sx, int sy, int sw, int sh, 
  int tx, int ty, int tw, int th, uint8 Alpha)
{
  SwapIfNeeded();

  /*
    @@@ DrawPixmap is called in 2D mode quite often.
    To reduce state changes, the text drawing states are reset as late
    as possible. The 2D canvas methods call a routine to flush all text to
    the screen, do the same here.
   */
  G2D->PerformExtension ("glflushtext");

  // If original dimensions are different from current dimensions (because
  // image has been scaled to conform to OpenGL texture size restrictions)
  // we correct the input coordinates here.
  int bitmapwidth = 0, bitmapheight = 0;
  hTex->GetRendererDimensions (bitmapwidth, bitmapheight);
  csGLTextureHandle *txt_mm = (csGLTextureHandle *)
    hTex->GetPrivateObject ();
  int owidth = txt_mm->orig_width;
  int oheight = txt_mm->orig_height;
  if (owidth != bitmapwidth || oheight != bitmapheight)
  {
    tx = (int)(tx * (float)bitmapwidth  / (float)owidth );
    ty = (int)(ty * (float)bitmapheight / (float)oheight);
    tw = (int)(tw * (float)bitmapwidth  / (float)owidth );
    th = (int)(th * (float)bitmapheight / (float)oheight);
  }

  // cache the texture if we haven't already.
  hTex->Precache ();

  // as we are drawing in 2D, we disable some of the commonly used features
  // for fancy 3D drawing
  statecache->SetShadeModel (GL_FLAT);
  SetZModeInternal (CS_ZBUF_NONE);
  //@@@???statecache->SetDepthMask (GL_FALSE);

  // if the texture has transparent bits, we have to tweak the
  // OpenGL blend mode so that it handles the transparent pixels correctly
  if ((hTex->GetKeyColor () || hTex->GetAlphaMap () || Alpha) ||
    (current_drawflags & CSDRAW_2DGRAPHICS)) // In 2D mode we always want to blend
    SetMixMode (CS_FX_ALPHA);
  else
    SetMixMode (CS_FX_COPY);

  statecache->Enable_GL_TEXTURE_2D ();
  glColor4f (1.0, 1.0, 1.0, Alpha ? (1.0 - BYTE_TO_FLOAT (Alpha)) : 1.0);
  ActivateTexture (hTex);

  // convert texture coords given above to normalized (0-1.0) texture
  // coordinates
  float ntx1,nty1,ntx2,nty2;
  ntx1 = ((float)tx            ) / bitmapwidth;
  ntx2 = ((float)tx + (float)tw) / bitmapwidth;
  nty1 = ((float)ty            ) / bitmapheight;
  nty2 = ((float)ty + (float)th) / bitmapheight;

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

  // smgh: This works in software opengl.
  // wouter: removed height-sy-1 to be height-sy.
  //    this is because on opengl y=0.0 is off screen, as is y=height.
  //    using height-sy gives output on screen which is identical to 
  //    using the software canvas.
  glTexCoord2f (ntx1, nty1);
  glVertex2i (sx, viewheight - sy);
  glTexCoord2f (ntx2, nty1);
  glVertex2i (sx + sw, viewheight - sy);
  glTexCoord2f (ntx2, nty2);
  glVertex2i (sx + sw, viewheight - (sy + sh));
  glTexCoord2f (ntx1, nty2);
  glVertex2i (sx, viewheight - (sy + sh));
  glEnd ();

  // Restore.
  SetZModeInternal (current_zmode);
}

void csGLGraphics3D::SetShadowState (int state)
{
  switch (state)
  {
    case CS_SHADOW_VOLUME_BEGIN:
      current_shadow_state = CS_SHADOW_VOLUME_BEGIN;
      stencil_initialized = false;
      glClearStencil (0);
      glClear (GL_STENCIL_BUFFER_BIT);
      EnableStencilShadow ();
      //statecache->SetStencilFunc (GL_ALWAYS, 0, 127);
      //statecache->SetStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
      // @@@ Jorrit: to avoid flickering I had to increase the
      // values below and multiply them with 3.
      //glPolygonOffset (-0.1f, -4.0f); 
      glPolygonOffset (-0.3f, -12.0f); 
      statecache->Enable_GL_POLYGON_OFFSET_FILL ();
      break;
    case CS_SHADOW_VOLUME_PASS1:
      current_shadow_state = CS_SHADOW_VOLUME_PASS1;
      break;
    case CS_SHADOW_VOLUME_FAIL1:
      current_shadow_state = CS_SHADOW_VOLUME_FAIL1;
      break;
    case CS_SHADOW_VOLUME_PASS2:
      current_shadow_state = CS_SHADOW_VOLUME_PASS2;
      break;
    case CS_SHADOW_VOLUME_FAIL2:
      current_shadow_state = CS_SHADOW_VOLUME_FAIL2;
      break;
    case CS_SHADOW_VOLUME_USE:
      current_shadow_state = CS_SHADOW_VOLUME_USE;
      statecache->Disable_GL_POLYGON_OFFSET_FILL ();
      break;
    case CS_SHADOW_VOLUME_FINISH:
      current_shadow_state = 0;
      DisableStencilShadow ();
      break;
  }
}

void csGLGraphics3D::DebugVisualizeStencil (uint32 mask)
{
  statecache->Enable_GL_STENCIL_TEST ();

  statecache->SetStencilMask (mask);
  statecache->SetStencilFunc (GL_EQUAL, 0xff, mask);
  statecache->SetStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
  glScissor (0, 0, 640, 480);
  statecache->Disable_GL_TEXTURE_2D ();
  statecache->SetShadeModel (GL_FLAT);

  SetZModeInternal (CS_ZBUF_FILL);
  glColor4f (1, 1, 1, 0);

  statecache->SetMatrixMode (GL_PROJECTION);
  glPushMatrix ();
  glLoadIdentity ();
  statecache->SetMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  glLoadIdentity ();

  glBegin (GL_QUADS);
  glVertex3f (-1.0f, 1.0f, 1.0f);
  glVertex3f (1.0f, 1.0f, 1.0f);
  glVertex3f (1.0f, -1.0f, 1.0f);
  glVertex3f (-1.0f, -1.0f, 1.0f);
  glEnd ();

  glPopMatrix ();
  statecache->SetMatrixMode (GL_PROJECTION);
  glPopMatrix ();

  SetZModeInternal (current_zmode);
  SetCorrectStencilState ();
}

void csGLGraphics3D::OpenPortal (size_t numVertices, 
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

void csGLGraphics3D::ClosePortal (bool use_zfill_portal)
{
  if (clipportal_stack.Length () <= 0) return;
  csClipPortal* cp = clipportal_stack.Pop ();

  if (use_zfill_portal)
  {
    GLboolean wmRed, wmGreen, wmBlue, wmAlpha;
    statecache->GetColorMask (wmRed, wmGreen, wmBlue, wmAlpha);
    statecache->SetColorMask (false, false, false, false);

    GLenum oldcullface;
    statecache->GetCullFace (oldcullface);
    statecache->SetCullFace (GL_FRONT);
    bool tex2d = statecache->IsEnabled_GL_TEXTURE_2D ();
    statecache->Disable_GL_TEXTURE_2D ();
    statecache->SetShadeModel (GL_FLAT);

    SetZModeInternal (CS_ZBUF_FILLONLY);
    Draw2DPolygon (cp->poly, cp->num_poly, cp->normal);
    SetZModeInternal (current_zmode);

    statecache->SetColorMask (wmRed, wmGreen, wmBlue, wmAlpha);
    statecache->SetCullFace (oldcullface);
    if (tex2d)
      statecache->Enable_GL_TEXTURE_2D ();
  }
  
  delete cp;
  clipportal_dirty = true;
  if (clipportal_floating > 0)
    clipportal_floating--;
}

void* csGLGraphics3D::RenderLock (iRenderBuffer* buffer, 
				  csGLRenderBufferLockType type, 
				  GLenum& compGLType)
{
  compGLType = compGLtypes[buffer->GetComponentType()];
  if (vboManager.IsValid())
    return vboManager->RenderLock (buffer, type);
  else
  {
    void* data;
    iRenderBuffer* master;
    if ((master = buffer->GetMasterBuffer()) != 0)
      data = master->Lock (CS_BUF_LOCK_READ);
    else
      data = buffer->Lock (CS_BUF_LOCK_READ);
    if (data == (void*)-1) return (void*)-1;
    return ((uint8*)data + buffer->GetOffset());
  }
}

void csGLGraphics3D::RenderRelease (iRenderBuffer* buffer)
{
  if (vboManager.IsValid())
    vboManager->RenderRelease (buffer);
  else
  {
    iRenderBuffer* master;
    if ((master = buffer->GetMasterBuffer()) != 0)
      master->Release();
    else
      buffer->Release();
  }
}

void csGLGraphics3D::Draw2DPolygon (csVector2* poly, int num_poly,
	const csPlane3& normal)
{
  SwapIfNeeded();

  // Get the plane normal of the polygon. Using this we can calculate
  // '1/z' at every screen space point.
  float M, N, O;
  float Dc = normal.D ();
  if (ABS (Dc) < 0.01f)
  {
    M = N = 0;
    O = 1;
  }
  else
  {
    float inv_Dc = 1.0f / Dc;
    M = -normal.A () * inv_Dc * inv_aspect;
    N = -normal.B () * inv_Dc * inv_aspect;
    O = -normal.C () * inv_Dc;
  }

  int v;
  glBegin (GL_TRIANGLE_FAN);
  csVector2* vt = poly;
  for (v = 0 ; v < num_poly ; v++)
  {
    float sx = vt->x - asp_center_x;
    float sy = vt->y - asp_center_y;
    float one_over_sz = M * sx + N * sy + O;
    float sz = 1.0f / one_over_sz;
    glVertex4f (vt->x * sz, vt->y * sz, -1.0f, sz);
    vt++;
  }
  glEnd ();
}

void csGLGraphics3D::SwapIfNeeded()
{
#ifdef DELAYED_SWAP
  if (wantToSwap)
  {
    G2D->Print (0);
    wantToSwap = false;
    if (delayClearFlags != 0)
    {
      glClear (delayClearFlags);
      delayClearFlags = 0;
    }
  }
#endif
}

void csGLGraphics3D::SetupClipPortals ()
{
  if (broken_stencil || !stencil_clipping_available)
    return;

  csClipPortal* cp = clipportal_stack.Top ();

  // First setup projection matrix for 2D drawing.
  statecache->SetMatrixMode (GL_PROJECTION);
  glPushMatrix ();
  glLoadIdentity ();
  SetGlOrtho (false);
  statecache->SetMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  glLoadIdentity ();

  // First set up the stencil area.
  statecache->Enable_GL_STENCIL_TEST ();
  statecache->SetStencilMask (stencil_clip_mask);
  statecache->SetStencilFunc (GL_ALWAYS, stencil_clip_value, stencil_clip_mask);
  statecache->SetStencilOp (GL_ZERO, GL_ZERO, GL_REPLACE);

  GLboolean wmRed, wmGreen, wmBlue, wmAlpha;
  statecache->GetColorMask (wmRed, wmGreen, wmBlue, wmAlpha);
  statecache->SetColorMask (false, false, false, false);
  GLenum oldcullface;
  statecache->GetCullFace (oldcullface);
  statecache->SetCullFace (GL_FRONT);
  bool tex2d = statecache->IsEnabled_GL_TEXTURE_2D ();
  statecache->Disable_GL_TEXTURE_2D ();
  statecache->SetShadeModel (GL_FLAT);

  SetZModeInternal (CS_ZBUF_TEST);

  Draw2DPolygon (cp->poly, cp->num_poly, cp->normal);

  // Use the stencil area.
  statecache->SetStencilFunc (GL_EQUAL, stencil_clip_value, stencil_clip_mask);
  statecache->SetStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);

  // First clear the z-buffer here.
  SetZModeInternal (CS_ZBUF_FILLONLY);

  statecache->SetMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  glBegin (GL_QUADS);
  glVertex3f (-1.0f, 1.0f, -1.0f);
  glVertex3f (1.0f, 1.0f, -1.0f);
  glVertex3f (1.0f, -1.0f, -1.0f);
  glVertex3f (-1.0f, -1.0f, -1.0f);
  glEnd ();

  statecache->SetMatrixMode (GL_MODELVIEW);
  glPopMatrix ();
  statecache->SetMatrixMode (GL_PROJECTION);
  glPopMatrix ();

  statecache->SetCullFace (oldcullface);
  statecache->SetColorMask (wmRed, wmGreen, wmBlue, wmAlpha);
  if (tex2d)
    statecache->Enable_GL_TEXTURE_2D ();

  SetZModeInternal (current_zmode);

  //DebugVisualizeStencil (128);
  //debug_inhibit_draw = true;
}

void csGLGraphics3D::SetClipper (iClipper2D* clipper, int cliptype)
{
  //clipper = new csBoxClipper (10, 10, 200, 200);
  csGLGraphics3D::clipper = clipper;
  if (!clipper) cliptype = CS_CLIPPER_NONE;
  csGLGraphics3D::cliptype = cliptype;
  stencil_initialized = false;
  frustum_valid = false;
  size_t i;
  for (i = 0; i<6; i++)
    glDisable ((GLenum)(GL_CLIP_PLANE0+i));
  DisableStencilClipping ();
  cache_clip_portal = -1;
  cache_clip_plane = -1;
  cache_clip_z_plane = -1;

  if (cliptype != CS_CLIPPER_NONE)
  {
    if (!hasOld2dClip)
      G2D->GetClipRect (old2dClip.xmin, old2dClip.ymin, 
	old2dClip.xmax, old2dClip.ymax);
    hasOld2dClip = true;

    csVector2* clippoly = clipper->GetClipPoly ();
    csBox2 scissorbox;
    scissorbox.AddBoundingVertex (clippoly[0]);
    for (i=1; i<clipper->GetVertexCount (); i++)
      scissorbox.AddBoundingVertexSmart (csVector2 (clippoly[i].x,
	/*(float)viewheight - */clippoly[i].y));
    csBox2 scissorClip;
    // Correct for differences in 2D and 3D coord systems.
    scissorClip.Set (old2dClip.xmin, /*viewheight - */old2dClip.ymin,
      old2dClip.xmax, /*viewheight - */old2dClip.ymax);
    scissorbox *= csBox2 (scissorClip);
    if (scissorbox.Empty())
    {
      csGLGraphics3D::cliptype = CS_CLIPPER_EMPTY;
      return;
    }

    const csRect scissorRect ((int)floorf (scissorbox.MinX ()), 
      // @@@ UGLY UGLY UGLY
      render_target ? viewheight - (int)ceilf (scissorbox.MaxY ()) : (int)floorf (scissorbox.MinY ()), 
      (int)ceilf (scissorbox.MaxX ()), 
      // @@@ UGLY UGLY UGLY
      render_target ? viewheight - (int)floorf (scissorbox.MinY ()) : (int)ceilf (scissorbox.MaxY ()));
    glScissor (scissorRect.xmin, scissorRect.ymin, scissorRect.Width(),
      scissorRect.Height());
  }
  else if (hasOld2dClip)
  {
    G2D->SetClipRect (old2dClip.xmin, old2dClip.ymin, 
      old2dClip.xmax, old2dClip.ymax);
    hasOld2dClip = false;
  }
}

// @@@ doesn't serve any purpose for now, but might in the future.
// left in for now.
bool csGLGraphics3D::SetRenderState (G3D_RENDERSTATEOPTION op, long val)
{
  switch (op)
  {
    case G3DRENDERSTATE_EDGES:
      forceWireframe = (val != 0);
      if (forceWireframe)
        glPolygonMode (GL_BACK, GL_LINE);
      else
        glPolygonMode (GL_BACK, GL_FILL);
      return true;
    default:
      return false;
  }
}

long csGLGraphics3D::GetRenderState (G3D_RENDERSTATEOPTION op) const
{
  switch (op)
  {
    case G3DRENDERSTATE_EDGES:
      return (forceWireframe ? 1 : 0);
    default:
      return 0;
  }
}

bool csGLGraphics3D::SetOption (const char* name, const char* value)
{
  if (!strcmp (name, "StencilThreshold"))
  {
    sscanf (value, "%d", &stencil_threshold);
    return true;
  }
  return false;
}

csPtr<iPolygonRenderer> csGLGraphics3D::CreatePolygonRenderer ()
{
  return csPtr<iPolygonRenderer> (new csGLPolygonRenderer (this));
}

void csGLGraphics3D::DrawSimpleMesh (const csSimpleRenderMesh& mesh, 
				     uint flags)
{  
  if (current_drawflags & CSDRAW_2DGRAPHICS)
  {
    // Try to be compatible with 2D drawing mode
    G2D->PerformExtension ("glflushtext");
  }

  if (scrapIndicesSize < mesh.indexCount)
  {
    scrapIndices = csRenderBuffer::CreateIndexRenderBuffer (mesh.indexCount,
      CS_BUF_STREAM, CS_BUFCOMP_UNSIGNED_INT,
      0, mesh.vertexCount - 1);
    scrapIndicesSize = mesh.indexCount;
  }
  if (scrapVerticesSize < mesh.vertexCount)
  {
    scrapVertices = csRenderBuffer::CreateRenderBuffer (
      mesh.vertexCount, CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 3);
    scrapTexcoords = csRenderBuffer::CreateRenderBuffer (
      mesh.vertexCount, CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 2);
    scrapColors = csRenderBuffer::CreateRenderBuffer (
      mesh.vertexCount, CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 4);

    scrapVerticesSize = mesh.vertexCount;
  }

  bool useShader = (mesh.shader != 0);

  csShaderVariable* sv;
  sv = scrapContext.GetVariableAdd (string_indices);
  if (mesh.indices)
  {
    scrapIndices->CopyInto (mesh.indices, mesh.indexCount);
    sv->SetValue (scrapIndices);
    scrapBufferHolder->SetRenderBuffer (CS_BUFFER_INDEX, scrapIndices);
  }
  else
  {
    scrapBufferHolder->SetRenderBuffer (CS_BUFFER_INDEX, 0);
    sv->SetValue (0);
  }
  sv = scrapContext.GetVariableAdd (string_vertices);
  if (mesh.vertices)
  {
    scrapVertices->CopyInto (mesh.vertices, mesh.vertexCount);
    scrapBufferHolder->SetRenderBuffer (CS_BUFFER_POSITION, scrapVertices);
    if (useShader)
      sv->SetValue (scrapVertices);
  }
  else
  {
    scrapBufferHolder->SetRenderBuffer (CS_BUFFER_POSITION, 0);
    if (useShader)
      sv->SetValue (0);
  }
  sv = scrapContext.GetVariableAdd (string_texture_coordinates);
  if (mesh.texcoords)
  {
    scrapTexcoords->CopyInto (mesh.texcoords, mesh.vertexCount);
    scrapBufferHolder->SetRenderBuffer (CS_BUFFER_TEXCOORD0, scrapTexcoords);
    if (useShader)
      sv->SetValue (scrapTexcoords);
  }
  else
  {
    scrapBufferHolder->SetRenderBuffer (CS_BUFFER_TEXCOORD0, 0);
    if (useShader)
      sv->SetValue (0);
  }
  sv = scrapContext.GetVariableAdd (string_colors);
  if (mesh.colors)
  {
    scrapColors->CopyInto (mesh.colors, mesh.vertexCount);
    scrapBufferHolder->SetRenderBuffer (CS_BUFFER_COLOR, scrapColors);
    if (useShader)
      sv->SetValue (scrapColors);
  }
  else
  {
    scrapBufferHolder->SetRenderBuffer (CS_BUFFER_COLOR, 0);
    if (useShader)
      sv->SetValue (0);
  }
  if (useShader)
  {
    sv = scrapContext.GetVariableAdd (string_texture_diffuse);
    sv->SetValue (mesh.texture);
  }
  else
  {
    if (ext->CS_GL_ARB_multitexture)
    {
      statecache->SetActiveTU (0);
      statecache->ActivateTU ();
    }
    if (mesh.texture)
      ActivateTexture (mesh.texture);
    else
      DeactivateTexture ();
  }

  csRenderMesh rmesh;
  //rmesh.z_buf_mode = mesh.z_buf_mode;
  rmesh.mixmode = mesh.mixmode;
  rmesh.clip_portal = 0;
  rmesh.clip_plane = 0;
  rmesh.clip_z_plane = 0;
  rmesh.do_mirror = false;
  rmesh.meshtype = mesh.meshtype;
  rmesh.indexstart = 0;
  rmesh.indexend = mesh.indexCount;
  rmesh.variablecontext = &scrapContext;
  rmesh.buffers = scrapBufferHolder;

  if (flags & csSimpleMeshScreenspace)
  {
    csReversibleTransform camtrans;
    if (current_drawflags & CSDRAW_2DGRAPHICS)
    {
      camtrans.SetO2T (
        csMatrix3 (1.0f, 0.0f, 0.0f,
                   0.0f, -1.0f, 0.0f,
                   0.0f, 0.0f, 1.0f));
      camtrans.SetO2TTranslation (csVector3 (0, viewheight, 0));
    } 
    else 
    {
      const float vwf = (float)(viewwidth);
      const float vhf = (float)(viewheight);

      camtrans.SetO2T (
      csMatrix3 (1.0f, 0.0f, 0.0f,
                 0.0f, -1.0f, 0.0f,
                 0.0f, 0.0f, 1.0f));
      camtrans.SetO2TTranslation (csVector3 (
      vwf / 2.0f, vhf / 2.0f, -aspect));
    }
    SetWorldToCamera (camtrans);
  }
  
  scrapContext.GetVariableAdd (string_object2world)->SetValue (mesh.object2world);

  csShaderVarStack stacks;
  shadermgr->PushVariables (stacks);
  scrapContext.PushVariables (stacks);
  if (mesh.dynDomain != 0) mesh.dynDomain->PushVariables (stacks);

  if (mesh.alphaType.autoAlphaMode)
  {
    csAlphaMode::AlphaType autoMode = csAlphaMode::alphaNone;

    iTextureHandle* tex = 0;
    if (mesh.alphaType.autoModeTexture != csInvalidStringID
        && mesh.alphaType.autoModeTexture < (csStringID)stacks.Length ()
        && stacks[mesh.alphaType.autoModeTexture].Length () > 0)
    {
      csShaderVariable* texVar = 
        stacks[mesh.alphaType.autoModeTexture].Top ();
      if (texVar)
	texVar->GetValue (tex);
    }
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
  
  csZBufMode old_zbufmode = current_zmode;
  SetZMode (mesh.z_buf_mode);
  csRenderMeshModes modes (rmesh);

  size_t shaderTicket = 0;
  size_t passCount = 1;
  if (mesh.shader != 0)
  {
    shaderTicket = mesh.shader->GetTicket (modes, stacks);
    passCount = mesh.shader->GetNumberOfPasses (shaderTicket);
  }

  for (size_t p = 0; p < passCount; p++)
  {
    if (mesh.shader != 0)
    {
      mesh.shader->ActivatePass (shaderTicket, p);
      mesh.shader->SetupPass (shaderTicket, &rmesh, modes, stacks);
    }
    else
    {
      ActivateBuffers (scrapBufferHolder, scrapMapping);
    }
    DrawMesh (&rmesh, modes, stacks);
    if (mesh.shader != 0)
    {
      mesh.shader->TeardownPass (shaderTicket);
      mesh.shader->DeactivatePass (shaderTicket);
    }
    else
    {
      DeactivateBuffers (0,0);
    }
  }

  if (flags & csSimpleMeshScreenspace)
  {
    if (current_drawflags & CSDRAW_2DGRAPHICS)
    {
      // Bring it back, that old new york rap! 
      // Or well, at least that old identity transform
      //SetObjectToCameraInternal (csReversibleTransform ());
    }
  }

  if (!useShader)
  {
    if (mesh.texture)
      DeactivateTexture ();
  }

  SetZMode (old_zbufmode);
}

SCF_IMPLEMENT_IBASE (csOpenGLHalo)
  SCF_IMPLEMENTS_INTERFACE (iHalo)
SCF_IMPLEMENT_IBASE_END

csOpenGLHalo::csOpenGLHalo (float iR, float iG, float iB, unsigned char *iAlpha,
  int iWidth, int iHeight, csGLGraphics3D* iG3D)
  : R(iR), G(iG), B(iB)
{
  SCF_CONSTRUCT_IBASE (0);

  // Initialization  
  R = iR; G = iG; B = iB;
  // OpenGL can only use 2^n sized textures
  Width = csFindNearestPowerOf2 (iWidth);
  Height = csFindNearestPowerOf2 (iHeight);

  uint8* rgba = new uint8 [Width * Height * 4];
  memset (rgba, 0, Width * Height * 4);
  uint8* rgbaPtr = rgba;
  for (int y = 0; y < iHeight; y++)
  {
    for (int x = 0; x < iWidth; x++)
    {
      *rgbaPtr++ = 0xff;
      *rgbaPtr++ = 0xff;
      *rgbaPtr++ = 0xff;
      *rgbaPtr++ = *iAlpha++;
    }
    rgbaPtr += (Width - iWidth) * 4;
  }

  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  // Create handle
  glGenTextures (1, &halohandle);
  // Activate handle
  csGLGraphics3D::statecache->SetActiveTU (0);
  csGLGraphics3D::statecache->ActivateTU ();
  csGLGraphics3D::statecache->SetTexture (GL_TEXTURE_2D, halohandle);

  // Jaddajaddajadda
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA,
    GL_UNSIGNED_BYTE, rgba);

  delete[] rgba;
  (G3D = iG3D)->IncRef ();

  Wfact = float (iWidth) / Width;
  Hfact = float (iHeight) / Height;

  Width = iWidth;
  Height = iHeight;

  if (R > 1.0 || G > 1.0 || B > 1.0)
  {
    dstblend = CS_FX_SRCALPHAADD;
    R /= 2; G /= 2; B /= 2;
  }
  else
    dstblend = CS_FX_ALPHA;
}

csOpenGLHalo::~csOpenGLHalo ()
{
  DeleteTexture();
  G3D->DecRef ();
  SCF_DESTRUCT_IBASE();
}

void csOpenGLHalo::DeleteTexture ()
{
  // Kill, crush and destroy
  // Delete generated OpenGL handle
  if (halohandle != 0)
  {
    glDeleteTextures (1, &halohandle);
    halohandle = 0;
  }
}

// Draw the halo. Wasn't that a suprise
void csOpenGLHalo::Draw (float x, float y, float w, float h, float iIntensity,
  csVector2 *iVertices, int iVertCount)
{
  //G3D->SwapIfNeeded();
  int swidth = G3D->GetWidth ();
  int sheight = G3D->GetHeight ();
  int i;

  if (w < 0) w = Width;
  if (h < 0) h = Height;

  csVector2 HaloPoly [4];
  if (!iVertices)
  {
    iVertCount = 4;
    iVertices = HaloPoly;

    float x1 = x, y1 = y, x2 = x + w, y2 = y + h;
    if (x1 < 0) x1 = 0; if (x2 > swidth ) x2 = swidth ;
    if (y1 < 0) y1 = 0; if (y2 > sheight) y2 = sheight;
    if ((x1 >= x2) || (y1 >= y2))
      return;

    HaloPoly [0].Set (x1, y1);
    HaloPoly [1].Set (x1, y2);
    HaloPoly [2].Set (x2, y2);
    HaloPoly [3].Set (x2, y1);
  };

  /// The inverse width and height of the halo
  float inv_W = Wfact / w, inv_H = Hfact / h;
  float aspect = (float)G3D->GetWidth() / G3D->GetAspect();
  float hw = (float)G3D->GetWidth() * 0.5f;

  int oldTU = G3D->statecache->GetActiveTU ();
  if (G3D->ext->CS_GL_ARB_multitexture)
    G3D->statecache->SetActiveTU (0);
  G3D->statecache->ActivateTU ();

  
  //csGLGraphics3D::SetGLZBufferFlags (CS_ZBUF_NONE);
  // @@@ Is this correct to override current_zmode?
  G3D->SetZMode (CS_ZBUF_NONE);
  bool texEnabled = 
    csGLGraphics3D::statecache->IsEnabled_GL_TEXTURE_2D ();
  csGLGraphics3D::statecache->Enable_GL_TEXTURE_2D ();

  csGLGraphics3D::statecache->SetShadeModel (GL_FLAT);
  csGLGraphics3D::statecache->SetTexture (GL_TEXTURE_2D, halohandle);
  G3D->SetAlphaType (csAlphaMode::alphaSmooth);

  //???@@@statecache->SetMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  glLoadIdentity();
  G3D->SetGlOrtho (false);
  //glTranslatef (0, 0, 0);

  G3D->SetMixMode (dstblend);

  glColor4f (R, G, B, iIntensity);

  glBegin (GL_POLYGON);
  for (i = iVertCount - 1; i >= 0; i--)
  {
    float vx = iVertices [i].x, vy = iVertices [i].y;
    glTexCoord2f ((vx - x) * inv_W, (vy - y) * inv_H);
    glVertex3f ((vx - hw) * aspect + hw, sheight - vy, -14.0f);
  }
  glEnd ();

  glPopMatrix ();

  /*
    @@@ Urgh. B/C halos are drawn outside the normal 
    shader/texture/buffer activation/mesh drawing realms, the states
    changed have to be backed up and restored when done.
   */
  csGLGraphics3D::statecache->SetTexture (GL_TEXTURE_2D, 0);
  if (!texEnabled)
    csGLGraphics3D::statecache->Disable_GL_TEXTURE_2D ();
  if (G3D->ext->CS_GL_ARB_multitexture)
    G3D->statecache->SetActiveTU (oldTU);
  G3D->statecache->ActivateTU ();
}

iHalo *csGLGraphics3D::CreateHalo (float iR, float iG, float iB,
  unsigned char *iAlpha, int iWidth, int iHeight)
{
  csOpenGLHalo* halo = new csOpenGLHalo (iR, iG, iB, iAlpha, iWidth, iHeight, 
    this);
  halos.Push (halo);
  return halo;
}

void csGLGraphics3D::RemoveHalo (csOpenGLHalo* halo)
{
  halos.DeleteFast (halo);
}

float csGLGraphics3D::GetZBuffValue (int x, int y)
{
  GLfloat zvalue;
  glReadPixels (x, viewheight - y - 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, 
    &zvalue);
  if (zvalue < .000001) return 1000000000.;
  // 0.090909=1/11, that is 1 divided by total depth delta set by
  // glOrtho. Where 0.090834 comes from, I don't know
  //return (0.090834 / (zvalue - (0.090909)));
  // @@@ Jorrit: I have absolutely no idea what they are trying to do
  // but changing the above formula to the one below at least appears
  // to give more accurate results.
  return (0.090728 / (zvalue - (0.090909)));
}

////////////////////////////////////////////////////////////////////
// iComponent
////////////////////////////////////////////////////////////////////

bool csGLGraphics3D::Initialize (iObjectRegistry* p)
{
  bool ok = true;
  object_reg = p;

  if (!scfiEventHandler)
    scfiEventHandler = csPtr<EventHandler> (new EventHandler (this));

  csRef<iEventQueue> q = CS_QUERY_REGISTRY(object_reg, iEventQueue);
  if (q)
    q->RegisterListener (scfiEventHandler, CSMASK_Broadcast);

  scfiShaderRenderInterface.Initialize(p);

  bugplug = CS_QUERY_REGISTRY (object_reg, iBugPlug);

  strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.shared.stringset", iStringSet);

  csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY (
  	object_reg, iPluginManager);
  csRef<iCommandLineParser> cmdline = CS_QUERY_REGISTRY (
  	object_reg, iCommandLineParser);

  config.AddConfig(object_reg, "/config/r3dopengl.cfg");

  const char *driver = cmdline->GetOption ("canvas");
  if (!driver)
    driver = config->GetStr ("Video.OpenGL.Canvas", CS_OPENGL_2D_DRIVER);

  G2D = CS_LOAD_PLUGIN (plugin_mgr, driver, iGraphics2D);
  if (G2D != 0)
    object_reg->Register(G2D, "iGraphics2D");
  else
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error loading Graphics2D plugin.");
    ok = false;
  }

  return ok;
}




////////////////////////////////////////////////////////////////////
// iEventHandler
////////////////////////////////////////////////////////////////////




bool csGLGraphics3D::HandleEvent (iEvent& Event)
{
  if (Event.Type == csevBroadcast)
    switch (Event.Command.Code)
    {
      case cscmdSystemOpen:
        Open ();
        return true;
      case cscmdSystemClose:
        Close ();
        return true;
      case cscmdContextResize:
	{
	  int w = G2D->GetWidth ();
	  int h = G2D->GetHeight ();
	  SetDimensions (w, h);
	  asp_center_x = w/2;
	  asp_center_y = h/2;
	}
	return true;
    }
  return false;
}


////////////////////////////////////////////////////////////////////
//                    iShaderRenderInterface
////////////////////////////////////////////////////////////////////

csGLGraphics3D::eiShaderRenderInterface::eiShaderRenderInterface()
{
}

csGLGraphics3D::eiShaderRenderInterface::~eiShaderRenderInterface()
{

}

void* csGLGraphics3D::eiShaderRenderInterface::GetPrivateObject (
	const char* name)
{
  if (strcasecmp(name, "ext") == 0)
    return (void*) (scfParent->ext);
  return 0;
}

void csGLGraphics3D::eiShaderRenderInterface::Initialize (iObjectRegistry *reg)
{
  object_reg = reg;
}

////////////////////////////////////////////////////////////////////
//                          iDebugHelper
////////////////////////////////////////////////////////////////////

SCF_IMPLEMENT_EMBEDDED_IBASE (csGLGraphics3D::eiDebugHelper)
  SCF_IMPLEMENTS_INTERFACE (iDebugHelper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

bool csGLGraphics3D::DebugCommand (const char* cmdstr)
{
  CS_ALLOC_STACK_ARRAY(char, cmd, strlen (cmdstr) + 1);
  strcpy (cmd, cmdstr);
  char* param = 0;
  char* space = strchr (cmd, ' ');
  if (space)
  {
    param = space + 1;
    *space = 0;
  }

  if (strcasecmp (cmd, "dump_slms") == 0)
  {
    csRef<iImageIO> imgsaver = CS_QUERY_REGISTRY (object_reg, iImageIO);
    if (!imgsaver)
    {
      Report (CS_REPORTER_SEVERITY_WARNING,
        "Could not get image saver.");
      return false;
    }

    csRef<iVFS> vfs = CS_QUERY_REGISTRY (object_reg, iVFS);
    if (!vfs)
    {
      Report (CS_REPORTER_SEVERITY_WARNING, 
	"Could not get VFS.");
      return false;
    }

    if (txtmgr)
    {
      const char* dir = 
	((param != 0) && (*param != 0)) ? param : "/tmp/slmdump/";
      txtmgr->DumpSuperLightmaps (vfs, imgsaver, dir);
    }

    return true;
  }
  else if (strcasecmp (cmd, "dump_zbuf") == 0)
  {
    const char* dir = 
      ((param != 0) && (*param != 0)) ? param : "/tmp/zbufdump/";
    DumpZBuffer (dir);

    return true;
  }
  else if (strcasecmp (cmd, "dump_vbostat") == 0)
  {
    if (vboManager) vboManager->DumpStats ();
    return true;
  }
  return false;
}

void csGLGraphics3D::DumpZBuffer (const char* path)
{
  csRef<iImageIO> imgsaver = CS_QUERY_REGISTRY (object_reg, iImageIO);
  if (!imgsaver)
  {
    Report (CS_REPORTER_SEVERITY_WARNING,
      "Could not get image saver.");
    return;
  }

  csRef<iVFS> vfs = CS_QUERY_REGISTRY (object_reg, iVFS);
  if (!vfs)
  {
    Report (CS_REPORTER_SEVERITY_WARNING, 
      "Could not get VFS.");
    return;
  }

  static int zBufDumpNr = 0;
  csString filenameZ;
  csString filenameScr;
  do
  {
    int nr = zBufDumpNr++;
    filenameZ.Format ("%s%d_z.png", path, nr);
    filenameScr.Format ("%s%d_scr.png", path, nr);
  }
  while (vfs->Exists (filenameZ) && vfs->Exists (filenameScr));

  {
    csRef<iImage> screenshot = G2D->ScreenShot ();
    csRef<iDataBuffer> buf = imgsaver->Save (screenshot, "image/png");
    if (!buf)
    {
      Report (CS_REPORTER_SEVERITY_WARNING,
	"Could not save screen.");
    }
    else
    {
      if (!vfs->WriteFile (filenameScr, (char*)buf->GetInt8 (), 
	buf->GetSize ()))
      {
	Report (CS_REPORTER_SEVERITY_WARNING,
	  "Could not write to %s.", filenameScr.GetData ());
      }
      else
      {
	Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "Dumped screen to %s", filenameScr.GetData ());
      }
    }
  }
  {
    csRef<csImageMemory> zImage;
    zImage.AttachNew (new csImageMemory (viewwidth, viewheight));

    static const uint8 zBufColors[][3] = {
      {  0,   0,   0},
      {  0, 255,   0},
      {255, 255,   0},
      {255,   0,   0},
      {255,   0, 255},
      {  0,   0, 255},
      {255, 255, 255},
    };
    const int colorMax = (sizeof (zBufColors) / sizeof (zBufColors[0])) - 1;

    int num = viewwidth * viewheight;
    GLfloat* zvalues = new GLfloat[num];
    glReadPixels (0, 0, viewwidth, viewheight, GL_DEPTH_COMPONENT, GL_FLOAT, 
      zvalues);
    GLfloat minValue = 1.0f;
    GLfloat maxValue = 0.0f;
    for (int i = 0; i < num; i++)
    {
      if (zvalues[i] == 0.0f) continue;	 // possibly leftovers from a Z buffer clean
      if (zvalues[i] < minValue)
	minValue = zvalues[i];
      else if (zvalues[i] > maxValue)
	maxValue = zvalues[i];
    }
    float zMul = 1.0f;
    if (maxValue - minValue > 0)
      zMul /= (maxValue - minValue);
    csRGBpixel* imgPtr = (csRGBpixel*)zImage->GetImageData ();
    for (int y = 0; y < viewheight; y++)
    {
      GLfloat* zPtr = zvalues + (viewheight - y - 1) * viewwidth;
      for (int x = 0; x < viewwidth; x++)
      {
	GLfloat zv = *zPtr++; 
	zv -= minValue;
	zv *= zMul;
	float cif = zv * (float)colorMax;
	int ci = csQint (cif);
	ci = MAX (0, MIN (ci, colorMax));
	if (ci == colorMax)
	{
	  (imgPtr++)->Set (zBufColors[ci][0], zBufColors[ci][1],
	    zBufColors[ci][2]);
	}
	else
	{
	  float ratio = cif - (float)ci;
	  float invRatio = 1.0f - ratio;
	  (imgPtr++)->Set (
	    csQint (zBufColors[ci+1][0] * ratio + zBufColors[ci][0] * invRatio), 
	    csQint (zBufColors[ci+1][1] * ratio + zBufColors[ci][1] * invRatio), 
	    csQint (zBufColors[ci+1][2] * ratio + zBufColors[ci][2] * invRatio));

	}
      }
    }
    delete[] zvalues;

    csRef<iDataBuffer> buf = imgsaver->Save (zImage, "image/png");
    if (!buf)
    {
      Report (CS_REPORTER_SEVERITY_WARNING,
	"Could not save Z buffer.");
    }
    else
    {
      if (!vfs->WriteFile (filenameZ, (char*)buf->GetInt8 (), 
	buf->GetSize ()))
      {
	Report (CS_REPORTER_SEVERITY_WARNING,
	  "Could not write to %s.", filenameZ.GetData ());
      }
      else
      {
	Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "Dumped Z buffer to %s", filenameZ.GetData ());
      }
    }
  }
}
