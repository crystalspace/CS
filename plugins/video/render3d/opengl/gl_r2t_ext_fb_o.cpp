/*
    Copyright (C) 2005 by Jorrit Tyberghein
              (C) 2005 by Frank Richter

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

#include "cssysdef.h"

#include "gl_render3d.h"
#include "gl_txtmgr.h"
#include "gl_r2t_ext_fb_o.h"
#include "profilescope.h"

#include "csplugincommon/opengl/glenum_identstrs.h"
#include "csplugincommon/opengl/glhelper.h"

//#define FBO_DEBUG

#ifdef FBO_DEBUG
  #define FBO_PRINTF csPrintf
#else
  #define FBO_PRINTF while(0) csPrintf
#endif

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{

  void RenderBufferWrapper::Setup (int w, int h, GLenum storage)
  {
    ext->glGenRenderbuffersEXT (1, &buffer);
    ext->glBindRenderbufferEXT (GL_RENDERBUFFER_EXT, buffer);
    ext->glRenderbufferStorageEXT (GL_RENDERBUFFER_EXT, 
      storage, w, h);
    bufSize.width = w;
    bufSize.height = h;
  }

  void RenderBufferWrapper::Free()
  {
    ext->glDeleteRenderbuffersEXT (1, &buffer);
  }

  //-------------------------------------------------------------------------

  void FBOWrapper::FreeBuffers()
  {
    depthRB = 0;
    stencilRB = 0;

    if (framebuffer != 0)
    {
  #ifdef CS_DEBUG
      GLint currentFB;
      glGetIntegerv (GL_FRAMEBUFFER_BINDING_EXT, &currentFB);
      CS_ASSERT_MSG("Unbind frame buffer before freeing it",
	currentFB != (GLint)framebuffer);
  #endif

      FBO_PRINTF ("Freeing FBO %u\n", framebuffer);
      ext->glDeleteFramebuffersEXT (1, &framebuffer);
      framebuffer = 0;
    }
  }

#ifdef CS_DEBUG
  static GLuint boundFBO;
#endif

  void FBOWrapper::Complete2 (bool& needsDepth, bool& needsStencil)
  {
    CS_ASSERT(boundFBO == framebuffer);

    const R2TAttachmentGroup<>& attachments = this->attachments;
    // Bind textures
    static const GLenum fbAttachments[rtaNumAttachments] = 
    {
      GL_DEPTH_ATTACHMENT_EXT, 
      GL_COLOR_ATTACHMENT0_EXT,
      GL_COLOR_ATTACHMENT1_EXT,
      GL_COLOR_ATTACHMENT2_EXT,
      GL_COLOR_ATTACHMENT3_EXT,
      GL_COLOR_ATTACHMENT4_EXT,
      GL_COLOR_ATTACHMENT5_EXT,
      GL_COLOR_ATTACHMENT6_EXT,
      GL_COLOR_ATTACHMENT7_EXT,
      GL_COLOR_ATTACHMENT8_EXT,
      GL_COLOR_ATTACHMENT9_EXT,
      GL_COLOR_ATTACHMENT10_EXT,
      GL_COLOR_ATTACHMENT11_EXT,
      GL_COLOR_ATTACHMENT12_EXT,
      GL_COLOR_ATTACHMENT13_EXT,
      GL_COLOR_ATTACHMENT14_EXT,
      GL_COLOR_ATTACHMENT15_EXT,
    };

    initialAttachments = 0;
    for (int a = 0; a < rtaNumAttachments; a++)
    {
      const WRTAG::RTA& attachment =
	attachments.GetAttachment (csRenderTargetAttachment(a));
      if (!attachment.IsValid()) continue;
      
      initialAttachments |= 1 << a;

      csGLBasicTextureHandle* tex_mm = static_cast<csGLBasicTextureHandle*> (
	(iTextureHandle*)attachment.texture);
      const GLenum texTarget = tex_mm->GetGLTextureTarget();
      const GLuint texHandle = tex_mm->GetHandle();
      switch (texTarget)
      {
	case GL_TEXTURE_1D:
	  ext->glFramebufferTexture1DEXT (GL_FRAMEBUFFER_EXT,
	    fbAttachments[a], GL_TEXTURE_1D, texHandle, 0);
	  break;
	case GL_TEXTURE_2D:
	case GL_TEXTURE_RECTANGLE_ARB:
	  ext->glFramebufferTexture2DEXT (GL_FRAMEBUFFER_EXT, 
	    fbAttachments[a], texTarget, texHandle, 0);
	  break;
	case GL_TEXTURE_CUBE_MAP:
	  ext->glFramebufferTexture2DEXT (GL_FRAMEBUFFER_EXT, 
	    fbAttachments[a], 
	    GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + attachment.subtexture, 
	    texHandle, 0);
	  break;
	case GL_TEXTURE_3D:
	  ext->glFramebufferTexture3DEXT (GL_FRAMEBUFFER_EXT, 
	    fbAttachments[a], texTarget, texHandle, 0,
	    attachment.subtexture);
	  break;
      }
    }

    bool hasColor = attachments.GetAttachment (rtaColor0).IsValid();
    bool hasDepth = attachments.GetAttachment (rtaDepth).IsValid();
    // Check if we need to attach depth+stencil buffers
    needsDepth = needsStencil = !hasDepth;

    if (!hasColor)
    {
      // Disable color drawing
      glDrawBuffer (GL_NONE);
      glReadBuffer (GL_NONE);
    }
  }

  void FBOWrapper::SetRBAttachment (GLenum attachment, RenderBufferWrapper* rb)
  {
    if (rb == 0) return;
    CS_ASSERT(boundFBO == framebuffer);

    ext->glFramebufferRenderbufferEXT (GL_FRAMEBUFFER_EXT,
      attachment, GL_RENDERBUFFER_EXT, rb->GetBuffer());
  }

  void FBOWrapper::Bind ()
  {
    if (framebuffer == 0)
    {
      ext->glGenFramebuffersEXT (1, &framebuffer);
      FBO_PRINTF ("Created FBO %u\n", framebuffer);
    }
    ext->glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, framebuffer);
    SetDrawBuffers();
  #ifdef CS_DEBUG
    boundFBO = framebuffer;
  #endif
  }

  void FBOWrapper::Unbind ()
  {
    ext->glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);
  #ifdef CS_DEBUG
    boundFBO = 0;
  #endif
  }

  void FBOWrapper::SetDrawBuffers() const
  {
    static GLenum openGLColorAttachmentEnums[] = 
    {
      GL_COLOR_ATTACHMENT0_EXT,
      GL_COLOR_ATTACHMENT1_EXT,
      GL_COLOR_ATTACHMENT2_EXT,
      GL_COLOR_ATTACHMENT3_EXT,
      GL_COLOR_ATTACHMENT4_EXT,
      GL_COLOR_ATTACHMENT5_EXT,
      GL_COLOR_ATTACHMENT6_EXT,
      GL_COLOR_ATTACHMENT7_EXT,
      GL_COLOR_ATTACHMENT8_EXT,
      GL_COLOR_ATTACHMENT9_EXT,
      GL_COLOR_ATTACHMENT10_EXT,
      GL_COLOR_ATTACHMENT11_EXT,
      GL_COLOR_ATTACHMENT12_EXT,
      GL_COLOR_ATTACHMENT13_EXT,
      GL_COLOR_ATTACHMENT14_EXT,
      GL_COLOR_ATTACHMENT15_EXT,
    };

    CS_ASSERT ((sizeof(openGLColorAttachmentEnums) / sizeof(GLenum)) == rtaNumColorAttachments);

    // Builds the attachment array passed into glDrawBuffers.
    GLint count = 0;
    GLenum buffers[rtaNumColorAttachments] = { 0 };
    
    // NOTE: We assume that the color attachment enumerates increases as:
    //  rtaColor0 = N where N is some constant >= 0
    //  rtaColor1 = rtaColor0 + 1
    //  rtaColor2 = rtaColor1 + 1
    //   ... etc.

    for (int i = 0; i < rtaNumColorAttachments; i++)
    {
      csRenderTargetAttachment attachment = (csRenderTargetAttachment)(rtaColor0 + i);

      const WRTAG::RTA &rta = attachments.GetAttachment (attachment);
      if (rta.IsValid ())
      {
        buffers[count] = openGLColorAttachmentEnums[i];
        count++;
      }
    }

    if (count > 0 && ext->glDrawBuffersARB)
    {
      ext->glDrawBuffersARB (count, buffers);
    }
  }

  //-------------------------------------------------------------------------

  csGLRender2TextureEXTfbo::csGLRender2TextureEXTfbo (csGLGraphics3D* G3D) :
    csGLRender2TextureBackend (G3D), enableFBO (true), currentFBO (0), 
    viewportSet (false)
  {
    GLenum fbStatus = GL_FRAMEBUFFER_UNSUPPORTED_EXT;

    /* Make sure we have access to glDrawBuffersARB for rendering to multiple 
     * render targets. */
    G3D->ext->InitGL_ARB_draw_buffers ();

    /* Try to determine a working depth, and if available, stencil buffer 
     * format.*/
     
    /* NV supposedly does not support separate stencil attachment, but packed
     * depth/stencil attachments (EXT_packed_depth_stencil or so) - tried 
     * first if available. */
    G3D->ext->InitGL_EXT_packed_depth_stencil ();
    if (G3D->ext->CS_GL_EXT_packed_depth_stencil)
    {
      depthStorage = GL_DEPTH_STENCIL_EXT;
      stencilStorage = GL_DEPTH_STENCIL_EXT;

      FBOWrapper testFBO (G3D->ext, 256, 256);
      testFBO.Bind();
      testFBO.Complete (*this);
      fbStatus = testFBO.GetStatus();
      testFBO.Unbind();
      depthRBCache.Clear (true); stencilRBCache.Clear (true);
    }

    /* Try separate depth and stencil attachments */
    if (fbStatus != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
      depthStorage = GL_DEPTH_COMPONENT;
      stencilStorage = GL_STENCIL_INDEX;

      FBOWrapper testFBO (G3D->ext, 256, 256);
      testFBO.Bind();
      testFBO.Complete (*this);
      fbStatus = G3D->ext->glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
      testFBO.Unbind();
      depthRBCache.Clear (true); stencilRBCache.Clear (true);
    }

    /* At least ATI Catalyst 6.8 (2006-09-08) does not support separate depth
     * and stencil attachments. So fall back to pure depth. */
    if (fbStatus != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
      depthStorage = GL_DEPTH_COMPONENT;
      stencilStorage = 0;

      FBOWrapper testFBO (G3D->ext, 256, 256);
      testFBO.Bind();
      testFBO.Complete (*this);
      fbStatus = G3D->ext->glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
      testFBO.Unbind();
      depthRBCache.Clear (true); stencilRBCache.Clear (true);
    }

    /* Still no dice? Poor. */
    if (fbStatus != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
      enableFBO = false;
      if (G3D->verbose)
        G3D->Report (CS_REPORTER_SEVERITY_NOTIFY,
          "FBO: No suitable depth/stencil formats found");
    }
    else
    {
      if (G3D->verbose)
      {
        csString depthStr (csOpenGLEnums.StringForIdent (depthStorage));
        csString stencilStr (csOpenGLEnums.StringForIdent (stencilStorage));
        G3D->Report (CS_REPORTER_SEVERITY_NOTIFY, 
          "FBO: depth format %s, stencil format %s", depthStr.GetData(), 
          stencilStr.GetData());
      }
    }
  }

csGLRender2TextureEXTfbo::~csGLRender2TextureEXTfbo()
{
}

bool csGLRender2TextureEXTfbo::SetRenderTarget (iTextureHandle* handle, 
                                                bool persistent,
                                                int subtexture,
                                                csRenderTargetAttachment attachment)
{
  csGLBasicTextureHandle* tex_mm = 
    static_cast<csGLBasicTextureHandle*> ((iTextureHandle*)handle);
  if (!tex_mm->IsWasRenderTarget())
  {
    tex_mm->SetupAutoMipping();
    tex_mm->SetWasRenderTarget (true);
  }

  if (!viewportSet)
  {
    int txt_w, txt_h;
    tex_mm->GetRendererDimensions (txt_w, txt_h);
    viewportHelper.Set2DViewport (G3D, txt_w, txt_h);
    viewportSet = true;
  }

  // @@@ Here, some initial validity checks could be made (e.g. dimension)
  currentAttachments.GetAttachment (attachment).Set (handle, persistent, 
    subtexture);
  tex_mm->SetInFBO (true);
  return true;
}

void csGLRender2TextureEXTfbo::UnsetRenderTargets ()
{
  for (int a = 0; a < rtaNumAttachments; a++)
  {
    const WRTAG::RTA& attachment =
	currentAttachments.GetAttachment (csRenderTargetAttachment(a));
    if (!attachment.IsValid()) continue;

    RegenerateTargetMipmaps (attachment);
  }

  currentAttachments.Clear();
  currentFBO = 0;
  if (viewportSet) viewportHelper.Reset2DViewport (G3D);
  viewportSet = false;
}

bool csGLRender2TextureEXTfbo::ValidateRenderTargets ()
{
  SelectCurrentFBO ();
  if (!currentFBO) return false;

  return (currentFBO->GetStatus() == GL_FRAMEBUFFER_COMPLETE_EXT);
}

bool csGLRender2TextureEXTfbo::CanSetRenderTarget (const char* format, 
                                                   csRenderTargetAttachment attachment)
{
  /* @@@ TODO: Implement using actual framebuffer object */
  CS::StructuredTextureFormat texfmt (CS::TextureFormatStrings::ConvertStructured (format));
  uint fmtcomp = texfmt.GetComponentMask();
  
  switch (attachment)
  {
  case rtaDepth:
    {
      // Support only depth-stencil formats
      if ((fmtcomp & ~CS::StructuredTextureFormat::compDepthStencil) != 0)
        return false;
      // Require D
      if ((fmtcomp & CS::StructuredTextureFormat::compD) == 0)
        return false;
      // Make sure stencil is available, if requested
      if (((fmtcomp & CS::StructuredTextureFormat::compS) != 0)
          && !HasStencil())
        return false;
      return true;
    }
    break;
  case rtaColor0:
    {
      if (((fmtcomp & CS::StructuredTextureFormat::compRGB) != 0)
          && ((fmtcomp & ~CS::StructuredTextureFormat::compRGBA) == 0))
        return true;
    }
    break;
  default:
    break;
  }
  return false;
}

iTextureHandle* csGLRender2TextureEXTfbo::GetRenderTarget (csRenderTargetAttachment attachment,
                                                           int* subtexture) const
{
  const WRTAG::RTA& rtAttachment =
    currentAttachments.GetAttachment (attachment);
  if (subtexture) *subtexture = rtAttachment.subtexture;
  return rtAttachment.texture;
}

void csGLRender2TextureEXTfbo::BeginDraw (int drawflags)
{
  GLRENDER3D_OUTPUT_STRING_MARKER((" "));

  /* Note: the renderer relies on this function to setup
   * matrices etc. So be careful when changing stuff. */

  G3D->GetDriver2D()->PerformExtension ("glflushtext");
  if (drawflags & CSDRAW_3DGRAPHICS)
  {
  }
  else if (drawflags & CSDRAW_2DGRAPHICS)
  {
    /*
      Render target: draw everything flipped.
    */
    G3D->statecache->SetMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    G3D->SetGlOrtho (true);
  }
  G3D->statecache->SetCullFace (GL_BACK);

  SelectCurrentFBO ();
}

CS::Math::Matrix4 csGLRender2TextureEXTfbo::FixupProjection (
    const CS::Math::Matrix4& projectionMatrix)
{
  CS::Math::Matrix4 flipY (
      1, 0, 0, 0,
      0, -1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1);
  CS::Math::Matrix4 actual = flipY * projectionMatrix;
  return actual;
}

void csGLRender2TextureEXTfbo::FinishDraw (bool readbackTargets)
{
  GLRENDER3D_OUTPUT_LOCATION_MARKER;
  
  if (readbackTargets)
  {
    ProfileScope _profile (G3D, "render target readback");
      
    for (int a = 0; a < rtaNumAttachments; a++)
    {
      const WRTAG::RTA& attachment =
	currentAttachments.GetAttachment (csRenderTargetAttachment(a));
      if (attachment.IsValid())
      {
	csGLBasicTextureHandle* tex_mm = 
	  static_cast<csGLBasicTextureHandle*> ((iTextureHandle*)attachment.texture);
	tex_mm->ReadbackFramebuffer();
      }
    }
  }

  currentFBO->Unbind();
  G3D->statecache->SetCullFace (GL_FRONT);
}

void csGLRender2TextureEXTfbo::SetClipRect (const csRect& clipRect)
{
  SelectCurrentFBO ();

  GLRENDER3D_OUTPUT_LOCATION_MARKER;
  glScissor (clipRect.xmin, currentFBO->GetHeight() - clipRect.ymax, 
    clipRect.Width(), clipRect.Height());
}

void csGLRender2TextureEXTfbo::SetupClipPortalDrawing ()
{
  GLRENDER3D_OUTPUT_LOCATION_MARKER;
  G3D->statecache->SetMatrixMode (GL_MODELVIEW);
  glScalef (1, -1, 1);
}

void csGLRender2TextureEXTfbo::NextFrame (uint frameNum)
{
  fboCache.AdvanceTime (frameNum);
  depthRBCache.AdvanceTime (frameNum);
  stencilRBCache.AdvanceTime (frameNum);
  
  fboCache.agedPurgeInterval = 60;
}

void csGLRender2TextureEXTfbo::CleanupFBOs()
{
  fboCache.agedPurgeInterval = 0;
}

void csGLRender2TextureEXTfbo::GetDepthStencilRBs (const Dimensions& fbSize, 
						   bool needsDepth, 
						   csRef<RenderBufferWrapper>& depthRB, 
						   bool needsStencil,
						   csRef<RenderBufferWrapper>& stencilRB)
{
  csRef<RenderBufferWrapper> newRB;
  csRef<RenderBufferWrapper>* cachedBuffer;

  if (needsDepth)
  {
    cachedBuffer = depthRBCache.Query (fbSize, true);
    if (cachedBuffer == 0)
    {
      newRB.AttachNew (new RenderBufferWrapper (G3D->ext));
      newRB->Setup (fbSize.width, fbSize.height, depthStorage);
      cachedBuffer = depthRBCache.AddActive (newRB);
    }
    depthRB = *cachedBuffer;
    // Put buffer back right away so it can be used by the next FBO
    depthRBCache.SetAvailable (cachedBuffer);
  }
  else
  {
    depthRB = 0;
  }

  if (needsStencil)
  {
    if ((stencilStorage == depthStorage) && depthRB.IsValid())
      stencilRB = depthRB;
    else
    {
      cachedBuffer = stencilRBCache.Query (fbSize, true);
      if (cachedBuffer == 0)
      {
	newRB.AttachNew (new RenderBufferWrapper (G3D->ext));
	newRB->Setup (fbSize.width, fbSize.height, stencilStorage);
	cachedBuffer = stencilRBCache.AddActive (newRB);
      }
      stencilRB = *cachedBuffer;
      // Put buffer back right away so it can be used by the next FBO
      stencilRBCache.SetAvailable (cachedBuffer);
    }
  }
  else
  {
    stencilRB = 0;
  }
}

void csGLRender2TextureEXTfbo::RegenerateTargetMipmaps (const WRTAG::RTA& target)
{
  if (!target.texture) return;

  csGLBasicTextureHandle* tex_mm = static_cast<csGLBasicTextureHandle*> (
	(iTextureHandle*)target.texture);
  if (!(tex_mm->GetFlags() & CS_TEXTURE_NOMIPMAPS))
  {
    tex_mm->RegenerateMipmaps();
  }
}
  
void csGLRender2TextureEXTfbo::SelectCurrentFBO ()
{
  if (currentFBO != 0) return;

  currentAttachments.ComputeHash();
  currentFBO = fboCache.Query (currentAttachments, true);
  
  // Weak refs may got zeroed
  if (currentFBO != 0)
  {
    const R2TAttachmentGroup<>& fboRTAG = currentFBO->attachments;
    for (int a = 0; a < rtaNumAttachments; a++)
    {
      const RRTAG::RTA& attachment =
	currentAttachments.GetAttachment (csRenderTargetAttachment(a));
      const WRTAG::RTA& fboAttachment =
	fboRTAG.GetAttachment (csRenderTargetAttachment(a));
      if (attachment.IsValid() && !fboAttachment.IsValid())
      {
        fboCache.RemoveActive (currentFBO);
        currentFBO = 0;
        break;
      }
    }
  }
  
  if (currentFBO == 0)
  {
    // @@@ We can prolly get away with FB dimensions entirely
    Dimensions dim;

    for (int a = 0; a < rtaNumAttachments; a++)
    {
      const WRTAG::RTA& attachment =
	currentAttachments.GetAttachment (csRenderTargetAttachment(a));
      if (!attachment.IsValid()) continue;

      attachment.texture->GetRendererDimensions (dim.width, dim.height);
      break;
    }

    currentFBO = fboCache.AddActive (FBOWrapper (G3D->ext, dim.width, dim.height));
    currentFBO->attachments = currentAttachments;
    currentFBO->attachments.ComputeHash ();
    currentFBO->Bind ();
    currentFBO->Complete (*this);
    if (currentFBO->GetStatus() != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
      if (G3D->verbose)
	G3D->Report (CS_REPORTER_SEVERITY_WARNING, 
	  "framebuffer object status: %s", FBStatusStr (currentFBO->GetStatus()));
    }
  }
  else
  {
    currentFBO->Bind ();
  }
}

const char* csGLRender2TextureEXTfbo::FBStatusStr (GLenum status)
{
  switch (status)
  {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
      return "complete";
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
      return "incomplete - attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
      return "incomplete - missing attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
      return "incomplete - duplicate attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
      return "incomplete - dimensions";
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
      return "incomplete - formats";
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
      return "incomplete - draw buffer";
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
      return "incomplete - read buffer";
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
      return "incomplete - unsupported";
    case GL_FRAMEBUFFER_STATUS_ERROR_EXT:
      return "error";
    default:
      {
	fboMsg.Format ("unknown %lx", (unsigned long)status);
	return fboMsg;
      }
  }
}

}
CS_PLUGIN_NAMESPACE_END(gl3d)
