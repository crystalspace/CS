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

#include "csgfx/bakekeycolor.h"
#include "csgfx/imagememory.h"
#include "csgfx/packrgb.h"

#include "gl_render3d.h"
#include "gl_txtmgr.h"
#include "gl_r2t_framebuf.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{

void csGLRender2TextureFramebuf::Set2DViewport ()
{
  iGraphics2D* g2d = G3D->GetDriver2D();
    
  framebufW = g2d->GetWidth();
  framebufH = g2d->GetHeight();

  g2d->PerformExtension ("vp_set", txt_w, txt_h);

  g2d->GetClipRect (rt_old_minx, rt_old_miny, 
    rt_old_maxx, rt_old_maxy);
  if ((rt_old_minx != 0) || (rt_old_miny != 0)
    || (rt_old_maxx != txt_w) || (rt_old_maxy != txt_h))
  {
    g2d->SetClipRect (0, 0, txt_w, txt_h);
  }
}

bool csGLRender2TextureFramebuf::SetRenderTarget (iTextureHandle* handle, 
						  bool persistent,
						  int subtexture,
						  csRenderTargetAttachment attachment)
{
  RTAttachment* target;
  switch (attachment)
  {
  case rtaColor0:
    target = &colorTarget;
    break;
  case rtaDepth:
    // @@@ FIXME: Perhaps investigate how to draw to the depth buffer
    if (persistent) return false;
    target = &depthTarget;
    break;
  default:
    return false;
  }
  
  int targetW, targetH;
  handle->GetRendererDimensions (targetW, targetH);
  if (!targetsSet)
  {
    txt_w = targetW; txt_h = targetH;
    Set2DViewport ();
    targetsSet = true;
  }
  else
  {
    if ((txt_w != targetW) || (txt_h != targetH))
      return false;
  }
    
  target->Set (handle, persistent, subtexture);

  return true;
/*  if (handle)
  {
  }
  else
  {
    G3D->GetDriver2D()->PerformExtension ("vp_reset");
    G3D->GetDriver2D()->SetClipRect (rt_old_minx, rt_old_miny, 
      rt_old_maxx, rt_old_maxy);
  }*/
}

void csGLRender2TextureFramebuf::UnsetRenderTargets()
{
  if (targetsSet)
  {
    G3D->GetDriver2D()->PerformExtension ("vp_reset");
    G3D->GetDriver2D()->SetClipRect (rt_old_minx, rt_old_miny, 
      rt_old_maxx, rt_old_maxy);
    targetsSet = false;
  }
  colorTarget.Clear();
  depthTarget.Clear();
}
  
bool csGLRender2TextureFramebuf::CanSetRenderTarget (const char* format, 
                                                     csRenderTargetAttachment attachment)
{
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
  }
  return false;
}

iTextureHandle* csGLRender2TextureFramebuf::GetRenderTarget (csRenderTargetAttachment attachment,
                                                             int* subtexture) const
{
  const RTAttachment* target = 0;
  switch (attachment)
  {
  case rtaDepth:
    target = &depthTarget;
    break;
  case rtaColor0:
    target = &colorTarget;
    break;
  }
  if (target == 0) return 0;
  if (subtexture) *subtexture = target->subtexture;
  return target->texture;
}

void csGLRender2TextureFramebuf::BeginDraw (int drawflags)
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
  if (colorTarget.IsValid() && !rt_onscreen)
  {
    G3D->statecache->SetShadeModel (GL_FLAT);
    glColor4f (1.0f, 1.0f, 1.0f, 1.0f);
    G3D->ActivateTexture (colorTarget.texture);
    G3D->statecache->Disable_GL_BLEND ();
    G3D->SetZMode (CS_ZBUF_NONE);

    csGLBasicTextureHandle* tex_mm = 
      static_cast<csGLBasicTextureHandle*> (colorTarget.texture->GetPrivateObject ());
    GLenum textarget = tex_mm->GetGLTextureTarget();

    GLint oldMagFilt, oldMinFilt;
    glGetTexParameteriv (textarget, GL_TEXTURE_MAG_FILTER, &oldMagFilt);
    glGetTexParameteriv (textarget, GL_TEXTURE_MIN_FILTER, &oldMinFilt);
    glTexParameteri (textarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (textarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glBegin (GL_QUADS);
    glTexCoord2f (0, 0); glVertex2i (0, txt_h);
    glTexCoord2f (0, 1); glVertex2i (0, 0);
    glTexCoord2f (1, 1); glVertex2i (txt_w, 0);
    glTexCoord2f (1, 0); glVertex2i (txt_w, txt_h);
    glEnd ();
    glTexParameteri (textarget, GL_TEXTURE_MAG_FILTER, oldMagFilt);
    glTexParameteri (textarget, GL_TEXTURE_MIN_FILTER, oldMinFilt);
  }
  rt_onscreen = true;
  G3D->statecache->SetCullFace (GL_BACK);
}

void csGLRender2TextureFramebuf::SetupProjection ()
{
  GLRENDER3D_OUTPUT_LOCATION_MARKER;
  G3D->SetGlOrtho (true);
}

void csGLRender2TextureFramebuf::GrabFramebuffer (const RTAttachment& target, 
                                                  GLenum internalFormat)
{
  csGLBasicTextureHandle* tex_mm = 
    static_cast<csGLBasicTextureHandle*> (target.texture->GetPrivateObject ());
  tex_mm->Precache ();
  // Texture is in tha cache, update texture directly.
  G3D->ActivateTexture (tex_mm);
  /* Texture has a keycolor - so we need to deal specially with it
    * to make sure the keycolor gets transparent. */
  if (tex_mm->GetKeyColor ())
  {
    // @@@ Processing sucks, but how else to handle keycolor?
    const size_t numPix = txt_w * txt_h;
    pixelScratch.SetSize (numPix * 4);
    glReadPixels (0, 0, txt_w, txt_h, GL_RGBA, 
      GL_UNSIGNED_BYTE, pixelScratch.GetArray());
    csRGBpixel key;
    tex_mm->GetKeyColor (key.red, key.green, key.blue);
    csBakeKeyColor::RGBA2D (pixelScratch.GetArray(), 
      pixelScratch.GetArray(), txt_w, txt_h, key);
    tex_mm->Blit (0, 0, txt_w, txt_h, pixelScratch.GetArray());
  }
  else
  {
    GLenum textarget = tex_mm->GetGLTextureTarget();
    if ((textarget != GL_TEXTURE_2D) && (textarget != GL_TEXTURE_RECTANGLE_ARB) 
	&& (textarget != GL_TEXTURE_CUBE_MAP))
      return;

    bool handle_subtexture = (textarget == GL_TEXTURE_CUBE_MAP);
    /* Reportedly, some drivers crash if using CopyTexImage on a texture
      * size larger than the framebuffer. Use CopyTexSubImage then. */
    bool needSubImage = (txt_w > framebufW) || (txt_h > framebufH);
    // Texture was not used as a render target before.
    // Make some necessary adjustments.
    if (!tex_mm->IsWasRenderTarget())
    {
      tex_mm->SetupAutoMipping();
      tex_mm->SetWasRenderTarget (true);
      tex_mm->texFormat = iTextureHandle::RGBA8888;
      if (needSubImage)
      {
	// Gah. Turn target texture to required storage.
	if (handle_subtexture)
	{
	  for (int i = 0; i < 6; i++)
	    glTexImage2D (
	      GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + i, 0, internalFormat, 
	      txt_w, txt_h,0, internalFormat, GL_UNSIGNED_BYTE, 0);
	}
	else
	  glTexImage2D (textarget, 0, internalFormat, txt_w, txt_h, 
	    0, internalFormat, GL_UNSIGNED_BYTE, 0);
      }
    }
    if (needSubImage)
    {
      if (handle_subtexture)
	glCopyTexSubImage2D (
	  GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + target.subtexture,
	  0, 0, 0, 0, 0, G3D->GetWidth(), G3D->GetHeight());
      else
	glCopyTexSubImage2D (textarget, 0, 0, 0, 0, 0, 
	  csMin (txt_w, framebufW), csMin (txt_h, framebufH) );
    }
    else
    {
      if (handle_subtexture)
	glCopyTexSubImage2D (
	  GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + target.subtexture,
	  0, 0, 0, 0, 0, txt_w, txt_h);
      else
	glCopyTexImage2D (textarget, 0, internalFormat, 0, 0, txt_w, txt_h, 0);
    }
    tex_mm->RegenerateMipmaps();
  }
}

void csGLRender2TextureFramebuf::FinishDraw ()
{
  GLRENDER3D_OUTPUT_LOCATION_MARKER;

  G3D->statecache->SetCullFace (GL_FRONT);

  if (rt_onscreen)
  {
    if (colorTarget.IsValid()) GrabFramebuffer (colorTarget, GL_RGBA);
    if (depthTarget.IsValid()) GrabFramebuffer (depthTarget, GL_DEPTH_COMPONENT);
    rt_onscreen = false;
  }
}

void csGLRender2TextureFramebuf::SetClipRect (const csRect& clipRect)
{
  GLRENDER3D_OUTPUT_LOCATION_MARKER;
  glScissor (clipRect.xmin, txt_h - clipRect.ymax, clipRect.Width(),
    clipRect.Height());
}

void csGLRender2TextureFramebuf::SetupClipPortalDrawing ()
{
  G3D->statecache->SetMatrixMode (GL_MODELVIEW);
  glScalef (1, -1, 1);
  //G3D->statecache->SetCullFace (GL_BACK);
}

bool csGLRender2TextureFramebuf::HasStencil()
{
  GLint sbits;
  glGetIntegerv (GL_STENCIL_BITS, &sbits);
  return sbits > 0;
}

}
CS_PLUGIN_NAMESPACE_END(gl3d)
