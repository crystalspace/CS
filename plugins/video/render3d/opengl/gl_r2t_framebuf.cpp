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

#include "csgeom/projections.h"
#include "csgfx/bakekeycolor.h"
#include "csgfx/imagememory.h"
#include "csgfx/packrgb.h"
#include "csplugincommon/opengl/glhelper.h"

#include "gl_render3d.h"
#include "gl_txtmgr.h"
#include "gl_r2t_framebuf.h"
#include "profilescope.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{

bool csGLRender2TextureFramebuf::SetRenderTarget (iTextureHandle* handle, 
						  bool persistent,
						  int subtexture,
						  csRenderTargetAttachment attachment)
{
  RTAttachment<>* target;
  switch (attachment)
  {
  case rtaColor0:
    target = &colorTarget;
    rt_onscreen = !persistent;
    break;
  case rtaDepth:
    // @@@ FIXME: Perhaps investigate how to draw to the depth buffer
    if (persistent) return false;
    target = &depthTarget;
    break;
  default:
    return false;
  }
  
  int targetW, targetH, targetD;
  handle->GetRendererDimensions (targetW, targetH, targetD);
  if (!targetsSet)
  {
    txt_w = targetW; txt_h = targetH; txt_d = targetD;
    viewportHelper.Set2DViewport (G3D, txt_w, txt_h, true);
    targetsSet = true;
  }
  else
  {
    if ((txt_w != targetW) || (txt_h != targetH))
      return false;
  }
    
  target->Set (handle, persistent, subtexture);

  return true;
}

void csGLRender2TextureFramebuf::UnsetRenderTargets()
{
  if (targetsSet)
  {
    viewportHelper.Reset2DViewport (G3D);
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
  default:
    break;
  }
  return false;
}

iTextureHandle* csGLRender2TextureFramebuf::GetRenderTarget (csRenderTargetAttachment attachment,
                                                             int* subtexture) const
{
  const RTAttachment<>* target = 0;
  switch (attachment)
  {
  case rtaDepth:
    target = &depthTarget;
    break;
  case rtaColor0:
    target = &colorTarget;
    break;
  default:
    return 0;
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

    csGLBasicTextureHandle* tex_mm = static_cast<csGLBasicTextureHandle*> (
	(iTextureHandle*)colorTarget.texture);
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

CS::Math::Matrix4 csGLRender2TextureFramebuf::FixupProjection (
    const CS::Math::Matrix4& projectionMatrix)
{
  CS::Math::Matrix4 actual;
  bool needSubImage = (txt_w > viewportHelper.GetVPWidth()) 
    || (txt_h > viewportHelper.GetVPHeight());
    
  if (needSubImage)
  {
    float scaleX = (float)viewportHelper.GetVPWidth() / (float)txt_w;
    float scaleY = (float)viewportHelper.GetVPHeight() / (float)txt_h;
    CS::Math::Matrix4 flipYAndScale (
      CS::Math::Projections::Ortho (-1, (scaleX-0.5f)*2.0f, (scaleY-0.5f)*2.0f, -1, 1, -1));
    actual = flipYAndScale * projectionMatrix;
  }
  else
  {
    CS::Math::Matrix4 flipY (
	1, 0, 0, 0,
	0, -1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1);
    actual = flipY * projectionMatrix;
  }
  
  return actual;
}

GLenum csGLRender2TextureFramebuf::GetInternalFormat (
  InternalFormatClass fmtClass, csGLBasicTextureHandle* tex)
{
  GLenum textarget = tex->GetGLTextureTarget();
  GLenum glInternalFormat;
  glGetTexLevelParameteriv ((textarget == GL_TEXTURE_CUBE_MAP) 
      ? GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB : textarget, 
    0, GL_TEXTURE_INTERNAL_FORMAT, (GLint*)&glInternalFormat);

  switch (fmtClass)
  {
  default:
  case ifColor:
    return GetInternalFormatColor (glInternalFormat);
  case ifDepth:
    return GetInternalFormatDepth (glInternalFormat);
  }
}

struct InternalFormatForDepth
{
  int components[4];
  GLenum format;
};

static GLenum MatchFormatDepth (const InternalFormatForDepth formats[],
				size_t formatNum,
				GLenum comp0, GLenum comp1, GLenum comp2, 
				GLenum comp3, GLenum fallback)
{
  GLint depths[4] = {0, 0, 0, 0};
  int compNum = 0;
  if (comp0 != 0)
  {
    compNum = 1;
    glGetIntegerv (comp0, &depths[0]);
    if (comp1 != 0)
    {
      compNum = 2;
      glGetIntegerv (comp1, &depths[1]);
      if (comp2 != 0)
      {
	compNum = 3;
	glGetIntegerv (comp2, &depths[2]);
	if (comp3 != 0)
	{
	  compNum = 4;
	  glGetIntegerv (comp3, &depths[3]);
	}
      }
    }
  }

  for (size_t f = 0; f < formatNum; f++)
  {
    int c;
    for (c = 0; c < compNum; c++)
    {
      if (formats[f].components[c] < depths[c]) break;
    }
    if (c >= compNum)
      return formats[f].format;
  }

  return fallback;
}

GLenum csGLRender2TextureFramebuf::GetInternalFormatColor (
  GLenum texInternalFormat)
{
  switch (texInternalFormat)
  {
  case GL_RGB4:
  case GL_RGB5:
  case GL_RGB8:
  case GL_RGB10:
  case GL_RGB12:
  case GL_RGB16:
  case GL_RGB16F_ARB:
  case GL_RGB32F_ARB:
  default:
    {
      static const InternalFormatForDepth rgbFormats[] = {
	{{ 4,  4,  4, 0}, GL_RGB4},
	{{ 5,  5,  5, 0}, GL_RGB5},
	{{ 8,  8,  8, 0}, GL_RGB8},
	{{10, 10, 10, 0}, GL_RGB10},
	{{12, 12, 12, 0}, GL_RGB12},
	{{16, 16, 16, 0}, GL_RGB16}
      };
      return MatchFormatDepth (rgbFormats, 
	sizeof (rgbFormats) / sizeof (rgbFormats[0]),
	GL_RED_BITS, GL_GREEN_BITS, GL_BLUE_BITS, 0,
	GL_RGB8);
    }
    break;
  case GL_RGBA2:
  case GL_RGBA4:
  case GL_RGB5_A1:
  case GL_RGBA8:
  case GL_RGB10_A2:
  case GL_RGBA12:
  case GL_RGBA16:
  case GL_RGBA16F_ARB:
  case GL_RGBA32F_ARB:
    {
      static const InternalFormatForDepth rgbaFormats[] = {
	{{ 2,  2,  4,  2}, GL_RGBA2},
	{{ 4,  4,  4,  4}, GL_RGBA4},
	{{ 5,  5,  5,  1}, GL_RGB5_A1},
	{{ 8,  8,  8,  8}, GL_RGBA8},
	{{10, 10, 10,  2}, GL_RGB10_A2},
	{{12, 12, 12, 12}, GL_RGBA12},
	{{16, 16, 16, 16}, GL_RGBA16}
      };
      return MatchFormatDepth (rgbaFormats, 
	sizeof (rgbaFormats) / sizeof (rgbaFormats[0]),
	GL_RED_BITS, GL_GREEN_BITS, GL_BLUE_BITS, GL_ALPHA_BITS,
	GL_RGBA8);
    }
    break;
  }
}

GLenum csGLRender2TextureFramebuf::GetInternalFormatDepth (
  GLenum texInternalFormat)
{
  switch (texInternalFormat)
  {
  case GL_DEPTH_COMPONENT16:
  case GL_DEPTH_COMPONENT24:
  case GL_DEPTH_COMPONENT32:
  default:
    {
      static const InternalFormatForDepth depthFormats[] = {
	{{16, 0, 0, 0}, GL_DEPTH_COMPONENT16},
	{{24, 0, 0, 0}, GL_DEPTH_COMPONENT24},
	{{32, 0, 0, 0}, GL_DEPTH_COMPONENT32},
      };
      return MatchFormatDepth (depthFormats, 
	sizeof (depthFormats) / sizeof (depthFormats[0]),
	GL_DEPTH_BITS, 0, 0, 0,	GL_DEPTH_COMPONENT24);
    }
    break;
  case GL_DEPTH24_STENCIL8_EXT:
    {
      static const InternalFormatForDepth depthStencilFormats[] = {
	{{24, 8, 0, 0}, GL_DEPTH24_STENCIL8_EXT},
      };
      return MatchFormatDepth (depthStencilFormats, 
	sizeof (depthStencilFormats) / sizeof (depthStencilFormats[0]),
	GL_DEPTH_BITS, GL_STENCIL_BITS, 0, 0, GL_DEPTH24_STENCIL8_EXT);
    }
    break;
  }
}

GLenum csGLRender2TextureFramebuf::GetBaseFormat (
  InternalFormatClass fmtClass, csGLBasicTextureHandle* tex)
{
  GLenum textarget = tex->GetGLTextureTarget();
  GLenum glInternalFormat;
  glGetTexLevelParameteriv ((textarget == GL_TEXTURE_CUBE_MAP) 
      ? GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB : textarget, 
    0, GL_TEXTURE_INTERNAL_FORMAT, (GLint*)&glInternalFormat);

  switch (fmtClass)
  {
  default:
  case ifColor:
    return GetBaseFormatColor (glInternalFormat);
  case ifDepth:
    return GetBaseFormatDepth (glInternalFormat);
  }
}

GLenum csGLRender2TextureFramebuf::GetBaseFormatColor (
  GLenum texInternalFormat)
{
  switch (texInternalFormat)
  {
  case GL_RGB4:
  case GL_RGB5:
  case GL_RGB8:
  case GL_RGB10:
  case GL_RGB12:
  case GL_RGB16:
  case GL_RGB16F_ARB:
  case GL_RGB32F_ARB:
  default:
    return GL_RGB;
    break;
  case GL_RGBA2:
  case GL_RGBA4:
  case GL_RGB5_A1:
  case GL_RGBA8:
  case GL_RGB10_A2:
  case GL_RGBA12:
  case GL_RGBA16:
  case GL_RGBA16F_ARB:
  case GL_RGBA32F_ARB:
    return GL_RGBA;
    break;
  }
}

GLenum csGLRender2TextureFramebuf::GetBaseFormatDepth (
  GLenum texInternalFormat)
{
  switch (texInternalFormat)
  {
  case GL_DEPTH_COMPONENT16:
  case GL_DEPTH_COMPONENT24:
  case GL_DEPTH_COMPONENT32:
  default:
    return GL_DEPTH_COMPONENT;
    break;
  case GL_DEPTH24_STENCIL8_EXT:
    return GL_DEPTH_STENCIL_NV;
    break;
  }
}

void csGLRender2TextureFramebuf::GrabFramebuffer (const RTAttachment<>& target,
						  InternalFormatClass fmtClass)
{
  csGLBasicTextureHandle* tex_mm = 
    static_cast<csGLBasicTextureHandle*> ((iTextureHandle*)target.texture);
  tex_mm->Precache ();
  // Texture is in tha cache, update texture directly.
  G3D->ActivateTexture (tex_mm);

  GLenum internalFormat = 0;

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
    if ((textarget != GL_TEXTURE_2D)
        && (textarget != GL_TEXTURE_3D)  
        && (textarget != GL_TEXTURE_RECTANGLE_ARB) 
	&& (textarget != GL_TEXTURE_CUBE_MAP))
      return;

    bool handle_subtexture = (textarget == GL_TEXTURE_CUBE_MAP);
    bool handle_3d = (textarget == GL_TEXTURE_3D);
    /* Reportedly, some drivers crash if using CopyTexImage on a texture
      * size larger than the framebuffer. Use CopyTexSubImage then. */
    bool needSubImage = (txt_w > viewportHelper.GetVPWidth()) 
      || (txt_h > viewportHelper.GetVPHeight());
    // Texture was not used as a render target before.
    // Make some necessary adjustments.
    if (needSubImage)
    {
      if (!tex_mm->IsWasRenderTarget())
      {
	internalFormat = GetInternalFormat (fmtClass, tex_mm);
	GLenum baseFormat = GetBaseFormat (fmtClass, tex_mm);
  
	tex_mm->SetupAutoMipping();
	tex_mm->SetWasRenderTarget (true);
	tex_mm->texFormat = iTextureHandle::RGBA8888;
	// Gah. Turn target texture to required storage.
	if (handle_subtexture)
	{
	  for (int i = 0; i < 6; i++)
	    glTexImage2D (
	      GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + i, 0, internalFormat, 
	      txt_w, txt_h,0, baseFormat, GL_UNSIGNED_BYTE, 0);
	}
	else if (handle_3d)
	{
	  G3D->ext->glTexImage3D (textarget, 0, internalFormat, txt_w, txt_h, 
	    txt_d, 0, baseFormat, GL_UNSIGNED_BYTE, 0);
	}
	else
	  glTexImage2D (textarget, 0, internalFormat, txt_w, txt_h, 
	    0, baseFormat, GL_UNSIGNED_BYTE, 0);
      }
      int orgX = viewportHelper.GetVPOfsX();
      int orgY = viewportHelper.GetOriginalFramebufferHeight()
	- (viewportHelper.GetVPOfsY() 
	  + csMin (txt_h, viewportHelper.GetVPHeight()));
    
      if (handle_subtexture)
	glCopyTexSubImage2D (
	  GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + target.subtexture,
	  0, 0, 0, orgX, orgY, 
	  csMin (txt_w, viewportHelper.GetVPWidth()), 
	  csMin (txt_h, viewportHelper.GetVPHeight()));
      else if (handle_3d)
	G3D->ext->glCopyTexSubImage3D (textarget, 0, 0, 0, orgX, orgY,
	  target.subtexture,
	  csMin (txt_w, viewportHelper.GetVPWidth()),
	  csMin (txt_h, viewportHelper.GetVPHeight()));
      else
	glCopyTexSubImage2D (textarget, 0, 0, 0, orgX, orgY, 
	  csMin (txt_w, viewportHelper.GetVPWidth()),
	  csMin (txt_h, viewportHelper.GetVPHeight()));
    }
    else
    {
      int orgX = viewportHelper.GetVPOfsX();
      int orgY = viewportHelper.GetOriginalFramebufferHeight()
	- (viewportHelper.GetVPOfsY() + txt_h);
    
      if (!tex_mm->IsWasRenderTarget())
      {
	internalFormat = GetInternalFormat (fmtClass, tex_mm);
  
	tex_mm->SetupAutoMipping();
	tex_mm->SetWasRenderTarget (true);
	tex_mm->texFormat = iTextureHandle::RGBA8888;
	if (handle_subtexture)
	  glCopyTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + target.subtexture, 
	    0, internalFormat, orgX, orgY, txt_w, txt_h, 0);
	else if (handle_3d)
	{
	  // Gah. Turn target texture to required storage.
	  GLenum baseFormat = GetBaseFormat (fmtClass, tex_mm);
	  G3D->ext->glTexImage3D (textarget, 0, internalFormat, txt_w, txt_h, 
	    txt_d, 0, baseFormat, GL_UNSIGNED_BYTE, 0);
	  G3D->ext->glCopyTexSubImage3D (textarget, 0, 0, 0, orgX, orgY,
	    target.subtexture, txt_w, txt_h);
	}
	else
	  glCopyTexImage2D (textarget, 0, internalFormat,
	    orgX, orgY, txt_w, txt_h, 0);
      }
      else
      {
	glGetTexLevelParameteriv ((textarget == GL_TEXTURE_CUBE_MAP) 
	    ? GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB : textarget, 
	  0, GL_TEXTURE_INTERNAL_FORMAT, (GLint*)&internalFormat);
	
	if (handle_subtexture)
	  glCopyTexSubImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + target.subtexture, 
	    0, 0, 0, orgX, orgY, txt_w, txt_h);
	else if (handle_3d)
	  G3D->ext->glCopyTexSubImage3D (textarget, 0, 0, 0, orgX, orgY,
	    target.subtexture, txt_w, txt_h);
	else
	  glCopyTexSubImage2D (textarget, 0,
	    0, 0, orgX, orgY, txt_w, txt_h);
      }
    }
    tex_mm->RegenerateMipmaps();
  }
}

void csGLRender2TextureFramebuf::FinishDraw (bool readbackTargets)
{
  GLRENDER3D_OUTPUT_LOCATION_MARKER;

  G3D->statecache->SetCullFace (GL_BACK);

  if (rt_onscreen)
  {
    if (colorTarget.IsValid()) GrabFramebuffer (colorTarget, ifColor);
    if (depthTarget.IsValid()) GrabFramebuffer (depthTarget, ifDepth);
    rt_onscreen = false;
    
    if (readbackTargets)
    {
      ProfileScope _profile (G3D, "render target readback");
      
      if (colorTarget.IsValid())
      {
	csGLBasicTextureHandle* tex_mm = 
	  static_cast<csGLBasicTextureHandle*> ((iTextureHandle*)colorTarget.texture);
	tex_mm->ReadbackFramebuffer();
      }
      if (depthTarget.IsValid())
      {
	csGLBasicTextureHandle* tex_mm = 
	  static_cast<csGLBasicTextureHandle*> ((iTextureHandle*)depthTarget.texture);
	tex_mm->ReadbackFramebuffer();
      }
    }
  }
}

void csGLRender2TextureFramebuf::SetClipRect (const csRect& clipRect)
{
  GLRENDER3D_OUTPUT_LOCATION_MARKER;
  GLint vp[4];
  glGetIntegerv (GL_VIEWPORT, vp);
  glScissor (vp[0] + clipRect.xmin, vp[1] + txt_h - clipRect.ymax, clipRect.Width(),
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

bool csGLRender2TextureFramebuf::HasMultisample()
{
  return G3D->GetMultisampleEnabled();
}

}
CS_PLUGIN_NAMESPACE_END(gl3d)
