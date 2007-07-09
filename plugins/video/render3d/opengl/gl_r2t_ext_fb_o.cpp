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

#include "csplugincommon/opengl/glenum_identstrs.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{

  void csGLRender2TextureEXTfbo::FBOWrapper::Setup (int w, int h, 
                                                    GLenum depthStorage, 
                                                    GLenum stencilStorage)
  {
    FreeBuffers();

    fb_w = w; fb_h = h;

    ext->glGenFramebuffersEXT (1, &framebuffer);
    ext->glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, framebuffer);

    if (depthStorage != 0)
    {
      ext->glGenRenderbuffersEXT (1, &depthRB);
      ext->glBindRenderbufferEXT (GL_RENDERBUFFER_EXT, depthRB);
      ext->glRenderbufferStorageEXT (GL_RENDERBUFFER_EXT, 
        depthStorage, w, h);

      // initialize depth renderbuffer
      ext->glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, 
        GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthRB);
    }


    if (stencilStorage != 0)
    {
      if (stencilStorage != depthStorage)
      {
        ext->glGenRenderbuffersEXT (1, &stencilRB);
        ext->glBindRenderbufferEXT (GL_RENDERBUFFER_EXT, stencilRB);
        ext->glRenderbufferStorageEXT (GL_RENDERBUFFER_EXT, 
          stencilStorage , w, h);
      }
      else
        stencilRB = depthRB;

      // initialize stencil renderbuffer
      ext->glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, 
        GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, stencilRB);
    }
  }

  void csGLRender2TextureEXTfbo::FBOWrapper::FreeBuffers()
  {
    GLint currentFB;
    glGetIntegerv (GL_FRAMEBUFFER_BINDING_EXT, &currentFB);
    if (currentFB == framebuffer)
      ext->glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);

    if (depthRB != 0)
    {
      if (depthRB != stencilRB)
        ext->glDeleteRenderbuffersEXT (1, &depthRB);
      depthRB = 0;
    }
    if (stencilRB != 0)
    {
      ext->glDeleteRenderbuffersEXT (1, &stencilRB);
      stencilRB = 0;
    }
    if (framebuffer != 0)
    {
      ext->glDeleteFramebuffersEXT (1, &framebuffer);
      framebuffer = 0;
    }
  }
    
  GLuint csGLRender2TextureEXTfbo::FBOWrapper::AttachDrawBuffer ()
  {
    GLuint buf;

    ext->glGenRenderbuffersEXT (1, &buf);
    ext->glBindRenderbufferEXT (GL_RENDERBUFFER_EXT, buf);
    ext->glRenderbufferStorageEXT (GL_RENDERBUFFER_EXT, 
      GL_RGBA8, fb_w, fb_h);

    ext->glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, 
      GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, buf);

    return buf;
  }

  void csGLRender2TextureEXTfbo::FBOWrapper::UnattachDrawBuffer ()
  {
    ext->glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, 
      GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, 0);
  }

  //-------------------------------------------------------------------------

  csGLRender2TextureEXTfbo::csGLRender2TextureEXTfbo (csGLGraphics3D* G3D) :
    csGLRender2TextureFramebuf (G3D), enableFBO (true), allocatedFBOs (FBOListAlloc (8)), 
    frameNum (0), lastFBOPurge (0) 
  {
    FBOWrapper testFBO (G3D->ext);
    GLenum fbStatus = GL_FRAMEBUFFER_UNSUPPORTED_EXT;

    /* Try to determine a working depth, and if available, stencil buffer 
     * format.*/
     
    /* NV supposedly does not support separate stencil attachment, but packed
     * depth/stencil attachments (EXT_packed_depth_stencil or so) - tried 
     * first if available. */
    if (G3D->ext->CS_GL_EXT_packed_depth_stencil)
    {
      depthStorage = GL_DEPTH_STENCIL_EXT;
      stencilStorage = GL_DEPTH_STENCIL_EXT;

      testFBO.Setup (256, 256, depthStorage, stencilStorage);
      GLuint drawBuffer = testFBO.AttachDrawBuffer ();
      fbStatus = G3D->ext->glCheckFramebufferStatusEXT (
        GL_FRAMEBUFFER_EXT);
      testFBO.UnattachDrawBuffer ();
      G3D->ext->glDeleteRenderbuffersEXT (1, &drawBuffer);
    }

    /* Try separate depth and stencil attachments */
    if (fbStatus != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
      depthStorage = GL_DEPTH_COMPONENT;
      stencilStorage = GL_STENCIL_INDEX;

      testFBO.Setup (256, 256, depthStorage, stencilStorage);
      GLuint drawBuffer = testFBO.AttachDrawBuffer ();
      fbStatus = G3D->ext->glCheckFramebufferStatusEXT (
        GL_FRAMEBUFFER_EXT);
      testFBO.UnattachDrawBuffer ();
      G3D->ext->glDeleteRenderbuffersEXT (1, &drawBuffer);
    }

    /* At least ATI Catalyst 6.8 (2006-09-08) does not support separate depth
     * and stencil attachments. So fall back to pure depth. */
    if (fbStatus != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
      depthStorage = GL_DEPTH_COMPONENT;
      stencilStorage = 0;

      testFBO.Setup (256, 256, depthStorage, stencilStorage);
      GLuint drawBuffer = testFBO.AttachDrawBuffer ();
      fbStatus = G3D->ext->glCheckFramebufferStatusEXT (
        GL_FRAMEBUFFER_EXT);
      testFBO.UnattachDrawBuffer ();
      G3D->ext->glDeleteRenderbuffersEXT (1, &drawBuffer);
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

csGLRender2TextureEXTfbo::FBOWrapper& csGLRender2TextureEXTfbo::GetFBO (
  int w, int h)
{
  FBOListType::Iterator it (allocatedFBOs);
  while (it.HasNext())
  {
    FBOWrapper& wrap = it.Next ();
    if ((wrap.GetWidth() == w) && (wrap.GetHeight() == h))
    {
      allocatedFBOs.MoveToFront (it);
      return wrap;
    }
  }

  it = allocatedFBOs.PushFront (FBOWrapper (G3D->ext));
  FBOWrapper& wrap = it.FetchCurrent ();
  /* FIXME: Generic format sufficient? Or better pick some sized 
   *  format based on the current depth buffer depth or so? 
   *  Might also need a loop or so to test different formats until
   *  the framebuffer validates. */
  wrap.Setup (w, h, depthStorage, stencilStorage);
  return wrap;
}

void csGLRender2TextureEXTfbo::PurgeFBOs ()
{
  bool doPurge = false;
  FBOListType::Iterator it (allocatedFBOs);
  while (it.HasNext())
  {
    FBOWrapper& wrap = it.Next ();
    if (frameNum - wrap.lastUsedFrame >= fboMinPurgeAge)
    {
      doPurge = true;
      break;
    }
  }

  if (doPurge)
  {
    while (it.HasCurrent())
    {
      allocatedFBOs.Delete (it);
    }
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

void csGLRender2TextureEXTfbo::SetRenderTarget (iTextureHandle* handle, 
                                                bool persistent,
                                                int subtexture)
{
  if (enableFBO)
  {
    if (handle == 0)
    {
      G3D->ext->glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);
      //glReadBuffer (GL_BACK);
    }
    else
    {
      handle->GetRendererDimensions (txt_w, txt_h);
      csGLBasicTextureHandle* tex_mm = 
        static_cast<csGLBasicTextureHandle*> (handle->GetPrivateObject ());
      if (!tex_mm->IsWasRenderTarget())
      {
        tex_mm->SetupAutoMipping();
        tex_mm->SetWasRenderTarget (true);
        G3D->statecache->SetTexture (GL_TEXTURE_2D, tex_mm->GetHandle());
	// FIXME: Take persistence into account?
        tex_mm->EnsureUncompressed (false);
        G3D->statecache->SetTexture (GL_TEXTURE_2D, 0);
      }

      FBOWrapper& fbo = GetFBO (txt_w, txt_h);

      if (fbo.txthandle != handle)
      {
        fbo.txthandle = handle;

        //glReadBuffer (GL_NONE);
        G3D->ext->glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, fbo.framebuffer);

	// FIXME: Support cube map faces, rect textures etc. at some point
        G3D->ext->glFramebufferTexture2DEXT (GL_FRAMEBUFFER_EXT, 
          GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, tex_mm->GetHandle(), 0);

        GLenum fbStatus = G3D->ext->glCheckFramebufferStatusEXT (
          GL_FRAMEBUFFER_EXT);
        if (fbStatus != GL_FRAMEBUFFER_COMPLETE_EXT)
        {
          G3D->ext->glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);
          enableFBO = false;
          //glReadBuffer (GL_BACK);
          if (G3D->verbose)
            G3D->Report (CS_REPORTER_SEVERITY_WARNING, 
              "framebuffer status: %s - falling back to backbuffer", 
              FBStatusStr (fbStatus));
          allocatedFBOs.DeleteAll ();
        }
      }
      else
      {
	// The framebuffer should still be set up for txthandle
        G3D->ext->glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, fbo.framebuffer);
      }
      fbo.lastUsedFrame = frameNum;
    }
  }
  if (enableFBO)
    csGLRender2TextureFramebuf::SetRenderTarget (handle, false, subtexture);
  else
    csGLRender2TextureFramebuf::SetRenderTarget (handle, persistent, subtexture);
}

void csGLRender2TextureEXTfbo::FinishDraw ()
{
  if (enableFBO)
    rt_onscreen = false;

  csGLRender2TextureFramebuf::FinishDraw();
  //G3D->ext->glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

  //csGLTextureHandle* tex_mm = (csGLTextureHandle *)
  //  render_target->GetPrivateObject ();
  //tex_mm->SetNeedMips (true);

  frameNum++;
  if (frameNum >= lastFBOPurge+fboPurgeAfter)
  {
    PurgeFBOs ();
    lastFBOPurge = frameNum;
  }
}

}
CS_PLUGIN_NAMESPACE_END(gl3d)
