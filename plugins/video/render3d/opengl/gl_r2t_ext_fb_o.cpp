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

#include "cstool/debugimagewriter.h"

#include "gl_render3d.h"
#include "gl_txtmgr.h"
#include "gl_r2t_ext_fb_o.h"

csGLRender2TextureEXTfbo::~csGLRender2TextureEXTfbo()
{
  FreeBuffers();
}

void csGLRender2TextureEXTfbo::FreeBuffers()
{
  if (depthRB != 0)
    G3D->ext->glDeleteRenderbuffersEXT (1, &depthRB);
  if (stencilRB != 0)
    G3D->ext->glDeleteRenderbuffersEXT (1, &stencilRB);
  if (framebuffer != 0)
    G3D->ext->glDeleteFramebuffersEXT (1, &framebuffer);
}

static const char* FBStatusStr (GLenum status)
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
	static csString msg;
	msg.Format ("unknown %x", status);
	return msg;
      }
  }
}

void csGLRender2TextureEXTfbo::SetRenderTarget (iTextureHandle* handle, 
						bool persistent)
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
      csGLTextureHandle* tex_mm = (csGLTextureHandle *)
	handle->GetPrivateObject ();
      if (!tex_mm->IsWasRenderTarget())
      {
	tex_mm->SetupAutoMipping();
	tex_mm->SetWasRenderTarget (true);
	G3D->statecache->SetTexture (GL_TEXTURE_2D, tex_mm->GetHandle());
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, txt_w, txt_h, 
	  0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	G3D->statecache->SetTexture (GL_TEXTURE_2D, 0);
      }

      if ((framebuffer == 0) || (txt_w > fb_w) || (txt_h > fb_h))
      {
	FreeBuffers();

	G3D->ext->glGenFramebuffersEXT (1, &framebuffer);
	G3D->ext->glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, framebuffer);
	G3D->ext->glGenRenderbuffersEXT (1, &depthRB);
	G3D->ext->glGenRenderbuffersEXT (1, &stencilRB);

	G3D->ext->glBindRenderbufferEXT (GL_RENDERBUFFER_EXT, depthRB);
	G3D->ext->glRenderbufferStorageEXT (GL_RENDERBUFFER_EXT, 
	  GL_DEPTH_COMPONENT24_ARB, txt_w, txt_h);

	/*G3D->ext->glBindRenderbufferEXT (GL_RENDERBUFFER_EXT, stencilRB);
	G3D->ext->glRenderbufferStorageEXT (GL_RENDERBUFFER_EXT, 
	  GL_STENCIL_INDEX8_EXT, txt_w, txt_h);*/

	//G3D->ext->glBindRenderbufferEXT (GL_RENDERBUFFER_EXT, 0);
	fb_w = txt_w; fb_h = txt_h;
      }
      //glReadBuffer (GL_NONE);
      G3D->ext->glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, framebuffer);
      //csPrintf ("framebuffer status(%d): %s\n", __LINE__,
	//FBStatusStr (G3D->ext->glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT)));
      G3D->ext->glFramebufferTexture2DEXT (GL_FRAMEBUFFER_EXT, 
	GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, tex_mm->GetHandle(), 0);
      G3D->ext->glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, 
	GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthRB);
      //G3D->ext->glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, 
	//GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, stencilRB);

      GLenum fbStatus = G3D->ext->glCheckFramebufferStatusEXT (
	GL_FRAMEBUFFER_EXT);
      if (fbStatus != GL_FRAMEBUFFER_COMPLETE_EXT)
      {
	G3D->ext->glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);
	enableFBO = false;
	//glReadBuffer (GL_BACK);
	csPrintf ("framebuffer status: %s\n", FBStatusStr (fbStatus));
      }
    }
  }
  if (enableFBO)
    csGLRender2TextureFramebuf::SetRenderTarget (handle, false);
  else
  {
    csGLRender2TextureFramebuf::SetRenderTarget (handle, persistent);
  }
}

void csGLRender2TextureEXTfbo::FinishDraw ()
{
  if (enableFBO)
    rt_onscreen = false;
  csGLRender2TextureFramebuf::FinishDraw();

  //csGLTextureHandle* tex_mm = (csGLTextureHandle *)
  //  render_target->GetPrivateObject ();
  //tex_mm->SetNeedMips (true);
}
