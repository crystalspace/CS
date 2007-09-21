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

#ifndef __CS_GL_R2T_EXT_FB_O_H__
#define __CS_GL_R2T_EXT_FB_O_H__

#include "csutil/fixedsizeallocator.h"
#include "csutil/list.h"

#include "gl_r2t_framebuf.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{

class csGLRender2TextureEXTfbo : public csGLRender2TextureFramebuf
{
  class FBOWrapper
  {
    csGLExtensionManager* ext;
    int fb_w, fb_h;
  public:
    GLuint framebuffer, depthRB, stencilRB;
    iTextureHandle* txthandle;
    uint lastUsedFrame;

    FBOWrapper (csGLExtensionManager* ext) : ext (ext), framebuffer (0), depthRB (0), 
      stencilRB (0), txthandle (0) { }
    FBOWrapper (const FBOWrapper& other) : ext (other.ext), framebuffer (0), 
      depthRB (0), stencilRB (0), txthandle (0)
    {
      // Only allow copying of un-setup wrappers
      CS_ASSERT (other.framebuffer == 0);
      CS_ASSERT (other.depthRB == 0);
      CS_ASSERT (other.stencilRB == 0);
    }
    ~FBOWrapper() { FreeBuffers(); }

    void Setup (int w, int h, GLenum depthStorage, GLenum stencilStorage);
    void FreeBuffers();

    int GetWidth() const { return fb_w; }
    int GetHeight() const { return fb_h; }

    /**
     * Attach a draw buffer (not stored in the wrapper since it's only needed
     * when detecting the depth/stencil formats in the csGLRender2TextureEXTfbo
     * ctor).
     */
    GLuint AttachDrawBuffer ();
    void UnattachDrawBuffer ();
  };

  bool enableFBO;
  GLenum depthStorage, stencilStorage;

  typedef csFixedSizeAllocator<csList<FBOWrapper>::allocSize> FBOListAlloc;
  typedef csList<FBOWrapper, FBOListAlloc> FBOListType;
  FBOListType allocatedFBOs;

  /// Number of frames between checks whether FBOs can be purged
  static const uint fboPurgeAfter = 16;
  /// If an FBO wasn't used in these last frames, purge
  static const uint fboMinPurgeAge = 10;
  uint frameNum;
  uint lastFBOPurge;

  csString fboMsg;

  FBOWrapper& GetFBO (int w, int h);
  void PurgeFBOs ();
  const char* FBStatusStr (GLenum status);
public:
  csGLRender2TextureEXTfbo (csGLGraphics3D* G3D);
  virtual ~csGLRender2TextureEXTfbo();

  virtual void SetRenderTarget (iTextureHandle* handle, bool persistent,
  	int subtexture);
  virtual void FinishDraw ();

  virtual bool HasStencil() { return stencilStorage != 0; }
};

}
CS_PLUGIN_NAMESPACE_END(gl3d)

#endif // __CS_GL_R2T_EXT_FB_O_H__

