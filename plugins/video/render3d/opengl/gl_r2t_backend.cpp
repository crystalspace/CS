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

#include "cssysdef.h"

#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"

#include "gl_r2t_backend.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{

csGLRender2TextureBackend::csGLRender2TextureBackend (csGLGraphics3D* G3D) 
  : G3D (G3D)
{
}
  
csGLRender2TextureBackend::~csGLRender2TextureBackend()
{
}

  //-------------------------------------------------------------------------

  bool csGLRender2TextureBackend::RTAttachment::operator!= (
    const RTAttachment& other) const
  {
    if (texture != other.texture) return true;
    if (subtexture != other.subtexture) return true;
    if (persistent != other.persistent) return true;

    return false;
  }

  //-------------------------------------------------------------------------

void R2TViewportHelper::Set2DViewport (iGraphics3D* G3D, int txt_w, int txt_h)
{
  iGraphics2D* g2d = G3D->GetDriver2D();
    
  g2d->GetFramebufferDimensions (framebufW, framebufH);
  g2d->GetViewport (vp_old_l, vp_old_t, vp_old_w, vp_old_h);

  g2d->SetViewport (0, framebufH - txt_h, txt_w, txt_h);
  g2d->GetClipRect (rt_old_minx, rt_old_miny, 
    rt_old_maxx, rt_old_maxy);
  if ((rt_old_minx != 0) || (rt_old_miny != 0)
    || (rt_old_maxx != txt_w) || (rt_old_maxy != txt_h))
  {
    g2d->SetClipRect (0, 0, txt_w, txt_h);
  }
}

void R2TViewportHelper::Reset2DViewport (iGraphics3D* G3D)
{
  iGraphics2D* g2d = G3D->GetDriver2D();
    
  g2d->SetViewport (vp_old_l, vp_old_t, vp_old_w, vp_old_h);
  g2d->SetClipRect (rt_old_minx, rt_old_miny, 
    rt_old_maxx, rt_old_maxy);
}

}
CS_PLUGIN_NAMESPACE_END(gl3d)
