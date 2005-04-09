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

#ifndef __CS_GL_R2T_BACKEND_H__
#define __CS_GL_R2T_BACKEND_H__

#include "csgeom/csrect.h"

class csGLGraphics3D;

struct iTextureHandle;

/// Superclass for all render2texture backends
class csGLRender2TextureBackend
{
protected:
  csGLGraphics3D* G3D;
public:
  csGLRender2TextureBackend (csGLGraphics3D* G3D);
  virtual ~csGLRender2TextureBackend();

  virtual void SetRenderTarget (iTextureHandle* handle, bool persistent) = 0;
  virtual void BeginDraw (int drawflags) = 0;
  virtual void SetupProjection () = 0;
  virtual void FinishDraw () = 0;
  virtual void SetClipRect (const csRect& clipRect) = 0;
};

#endif // __CS_GL_R2T_BACKEND_H__
