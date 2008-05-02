/*
    Copyright (C) 2005 by Jorrit Tyberghein
              (C) 2005-2008 by Frank Richter

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

#ifndef __CS_GL_R2T_BACKEND_H__
#define __CS_GL_R2T_BACKEND_H__

#include "csgeom/csrect.h"
#include "csutil/ref.h"
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"

struct iTextureHandle;

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{

class csGLGraphics3D;

/// Superclass for all render2texture backends
class csGLRender2TextureBackend
{
protected:
  csGLGraphics3D* G3D;
public:  
  struct RTAttachment
  {
    csRef<iTextureHandle> texture;
    int subtexture;
    int persistent; /* 'int' to avoid uninit bytes that occur with bool */

    RTAttachment() : subtexture (0), persistent (false) {}
    
    void Clear()
    {
      texture.Invalidate ();
      subtexture = 0;
      persistent = false;
    }
    bool IsValid() const
    {
      return texture.IsValid();
    }
    void Set (iTextureHandle* handle, bool persistent,
      int subtexture)
    {
      this->texture = handle;
      this->subtexture = subtexture;
      this->persistent = persistent;
    }

    bool operator== (const RTAttachment& other) const;
    bool operator!= (const RTAttachment& other) const;
  };

  csGLRender2TextureBackend (csGLGraphics3D* G3D);
  virtual ~csGLRender2TextureBackend();
  virtual bool Status() = 0;

  virtual bool SetRenderTarget (iTextureHandle* handle, bool persistent,
  	int subtexture, csRenderTargetAttachment attachment) = 0;
  virtual bool ValidateRenderTargets () = 0;
  virtual void UnsetRenderTargets() = 0;
  virtual bool CanSetRenderTarget (const char* format,
    csRenderTargetAttachment attachment) = 0;
  virtual iTextureHandle* GetRenderTarget (csRenderTargetAttachment attachment,
    int* subtexture) const = 0;
  
  virtual void BeginDraw (int drawflags) = 0;
  virtual void SetupProjection () = 0;
  virtual void FinishDraw () = 0;
  virtual void SetClipRect (const csRect& clipRect) = 0;
  virtual void SetupClipPortalDrawing () = 0;
  virtual bool HasStencil() = 0;

  virtual void NextFrame() {}
};

// Helper class for viewport changes made by R2T backends
class R2TViewportHelper
{
  /// Old clip rect to restore after rendering on a proc texture.
  int rt_old_minx, rt_old_miny, rt_old_maxx, rt_old_maxy;
  /// Framebuffer dimensions
  int framebufW, framebufH;
  /// Old viewport
  int vp_old_l, vp_old_t, vp_old_w, vp_old_h;
public:
  void Set2DViewport (iGraphics3D* G3D, int texW, int texH);
  void Reset2DViewport (iGraphics3D* G3D);

  int GetOriginalFramebufferWidth() const { return framebufW; }
  int GetOriginalFramebufferHeight() const { return framebufH; }
};

}
CS_PLUGIN_NAMESPACE_END(gl3d)

#endif // __CS_GL_R2T_BACKEND_H__
