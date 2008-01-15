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

#ifndef __CS_GL_R2T_FRAMEBUF_H__
#define __CS_GL_R2T_FRAMEBUF_H__

#include "csutil/dirtyaccessarray.h"

#include "gl_r2t_backend.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{

/// Render2texture backend using the framebuffer
class csGLRender2TextureFramebuf : public csGLRender2TextureBackend
{
protected:
  //@{
  /// Current render targets.
  RTAttachment colorTarget;
  RTAttachment depthTarget;
  bool targetsSet;
  //@}
  /// If true then the current render target has been put on screen.
  bool rt_onscreen;
  /// Old clip rect to restore after rendering on a proc texture.
  int rt_old_minx, rt_old_miny, rt_old_maxx, rt_old_maxy;
  /// Render target dimensions
  int txt_w, txt_h;
  /// Framebuffer dimensions
  int framebufW, framebufH;

  csDirtyAccessArray<uint8> pixelScratch;
  void Set2DViewport ();
  void GrabFramebuffer (const RTAttachment& target, GLenum internalFormat);
public:
  csGLRender2TextureFramebuf (csGLGraphics3D* G3D) 
    : csGLRender2TextureBackend (G3D), targetsSet (false), 
      rt_onscreen (false) { }

  bool SetRenderTarget (iTextureHandle* handle, bool persistent,
    int subtexture, csRenderTargetAttachment attachment);
  void UnsetRenderTargets();
  bool CanSetRenderTarget (const char* format, csRenderTargetAttachment attachment);
  iTextureHandle* GetRenderTarget (csRenderTargetAttachment attachment, int* subtexture) const;

  virtual void BeginDraw (int drawflags);
  virtual void SetupProjection ();
  virtual void FinishDraw ();
  virtual void SetClipRect (const csRect& clipRect);
  virtual void SetupClipPortalDrawing ();
  virtual bool HasStencil();
};

}
CS_PLUGIN_NAMESPACE_END(gl3d)

#endif // __CS_GL_R2T_FRAMEBUF_H__
