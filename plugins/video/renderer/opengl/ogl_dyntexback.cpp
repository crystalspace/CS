/*
    Copyright (C) 2000 by Norman Krämer
    Adapted for Opengl by Samuel Humphreys
  
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
#include <GL/gl.h>
#include "ogl_txtcache.h"
#include "ogl_txtmgr.h"
#include "ogl_dyntexback.h"
#include "ogl_g3d.h"

IMPLEMENT_IBASE (csOpenGLDynamicBackBuffer)
  IMPLEMENTS_INTERFACE (iGraphics3D)
IMPLEMENT_IBASE_END;

//#define COPY_TO_TEXTURE

csOpenGLDynamicBackBuffer::csOpenGLDynamicBackBuffer (iBase * pParent)
{ 
  CONSTRUCT_IBASE (pParent);
  tex=NULL; 
  g3d=NULL; 
  rstate_bilinearmap = false;
}

void csOpenGLDynamicBackBuffer::SetTarget (csGraphics3DOpenGL *g3d, csTextureMMOpenGL *tex)
{ 
  this->g3d = g3d;
  this->tex = tex;
  g2d = g3d->GetDriver2D();
  nPixelBytes = g2d->GetPixelBytes ();
  frame_height = g2d->GetHeight ();
  tex->GetMipMapDimensions (0, Width, Height);
  tex_0 = (csTextureOpenGLDynamic*) tex->get_texture (0);
}

bool csOpenGLDynamicBackBuffer::BeginDraw (int DrawFlags)
{
  bool succ = g3d->BeginDraw (DrawFlags);
  if (succ)
  {
    if (DrawFlags & CSDRAW_CLEARSCREEN)
      g2d->Clear (0);
    else
      // copy the tex into upper left corner
      g3d->DrawPixmap (tex, 0, 0, Width, Height, 0, 0, Width, Height);

  }
  return succ;
}

void csOpenGLDynamicBackBuffer::FinishDraw ()
{
  g3d->FinishDraw ();
  
  glEnable (GL_TEXTURE_2D);
  glDisable (GL_BLEND);
  glDisable (GL_DEPTH_TEST);
  glDisable (GL_DITHER);
  glDisable (GL_ALPHA_TEST);
  glPixelZoom (-1,0);
  csGLCacheData *tex_data = (csGLCacheData*) tex->GetCacheData();
  if (tex_data)
  {
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    // Texture is in tha cache, update texture directly.
#ifdef COPY_TO_TEXTURE
    glDeleteTextures(1, &tex_data->Handle);
    tex_data->Handle = 0;
    glGenTextures (1, &tex_data->Handle);
#endif

    glBindTexture (GL_TEXTURE_2D, tex_data->Handle);

    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
		     rstate_bilinearmap ? GL_LINEAR : GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
		     rstate_bilinearmap ? GL_LINEAR : GL_NEAREST);

#ifdef COPY_TO_TEXTURE
    // We know currently dynamic textures are not mip-mapped
    glCopyTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 
		      0, frame_height - Height, Width, Height, 0);
#else
    glCopyTexSubImage2D (GL_TEXTURE_2D, 0,
			 0, 0, 0, frame_height - Height,
			 Width, Height);
#endif
  }
  else
  {
    // Not in cache.
    glReadPixels (0, frame_height - Height, Width, Height,
		  GL_RGBA, GL_UNSIGNED_BYTE, tex_0->get_image_data());
  }
}

void csOpenGLDynamicBackBuffer::Print (csRect *area)
{
  (void)area;
}

void csOpenGLDynamicBackBuffer::DrawPolygon (G3DPolygonDP& poly)
{ 
  g3d->DrawPolygon (poly); 
}

void csOpenGLDynamicBackBuffer::DrawPolygonDebug (G3DPolygonDP& poly)
{ 
  g3d->DrawPolygonDebug (poly); 
}

void csOpenGLDynamicBackBuffer::DrawLine (const csVector3& v1, const csVector3& v2, float fov, int color)
{ 
  g3d->DrawLine (v1, v2, fov, color); 
}

void csOpenGLDynamicBackBuffer::StartPolygonFX (iTextureHandle* handle, UInt mode)
{ 
  g3d->StartPolygonFX (handle, mode); 
}

void csOpenGLDynamicBackBuffer::FinishPolygonFX ()
{ 
  g3d->FinishPolygonFX (); 
}

void csOpenGLDynamicBackBuffer::DrawPolygonFX (G3DPolygonDPFX& poly)
{ 
  g3d->DrawPolygonFX (poly); 
}

void csOpenGLDynamicBackBuffer::DrawTriangleMesh (G3DTriangleMesh& mesh)
{ 
  g3d->DrawTriangleMesh (mesh); 
}

void csOpenGLDynamicBackBuffer::DrawPolygonMesh (G3DPolygonMesh& mesh)
{ 
  g3d->DrawPolygonMesh (mesh); 
}

void csOpenGLDynamicBackBuffer::OpenFogObject (CS_ID id, csFog* fog)
{ 
  g3d->OpenFogObject (id, fog); 
}

void csOpenGLDynamicBackBuffer::DrawFogPolygon (CS_ID id, G3DPolygonDFP& poly, int fogtype)
{ 
  g3d->DrawFogPolygon (id, poly, fogtype);
}

void csOpenGLDynamicBackBuffer::CloseFogObject (CS_ID id)
{ 
  g3d->CloseFogObject (id); 
}

bool csOpenGLDynamicBackBuffer::SetRenderState (G3D_RENDERSTATEOPTION op, long val)
{ 
  return g3d->SetRenderState (op, val); 
}

long csOpenGLDynamicBackBuffer::GetRenderState (G3D_RENDERSTATEOPTION op)
{ 
  return g3d->GetRenderState (op); 
}

csGraphics3DCaps *csOpenGLDynamicBackBuffer::GetCaps ()
{ 
  return g3d->GetCaps (); 
}

unsigned long *csOpenGLDynamicBackBuffer::GetZBuffAt (int x, int y)
{ 
  return g3d->GetZBuffAt (x, y); 
}

float csOpenGLDynamicBackBuffer::GetZBuffValue (int x, int y)
{ 
  return g3d->GetZBuffValue (x, y); 
}

void csOpenGLDynamicBackBuffer::DumpCache ()
{ 
  g3d->DumpCache (); 
}

void csOpenGLDynamicBackBuffer::ClearCache ()
{ 
  g3d->ClearCache (); 
}

void csOpenGLDynamicBackBuffer::RemoveFromCache (iPolygonTexture* poly_texture)
{ 
  g3d->RemoveFromCache (poly_texture); 
}

void csOpenGLDynamicBackBuffer::SetPerspectiveCenter (int x, int y)
{ 
  g3d->SetPerspectiveCenter (x, y); 
}

void csOpenGLDynamicBackBuffer::SetPerspectiveAspect (float aspect)
{ 
  g3d->SetPerspectiveAspect (aspect); 
}

void csOpenGLDynamicBackBuffer::SetObjectToCamera (csReversibleTransform* o2c)
{ 
  g3d->SetObjectToCamera (o2c); 
}

void csOpenGLDynamicBackBuffer::SetClipper (csVector2* vertices, int num_vertices)
{ 
  g3d->SetClipper (vertices, num_vertices);
}

iGraphics2D *csOpenGLDynamicBackBuffer::GetDriver2D ()
{ 
  return g3d->GetDriver2D (); 
}

iTextureManager *csOpenGLDynamicBackBuffer::GetTextureManager ()
{ 
  return g3d->GetTextureManager (); 
}

iHalo *csOpenGLDynamicBackBuffer::CreateHalo (float iR, float iG, float iB, unsigned char *iAlpha, int iWidth, 
				   int iHeight)
{ 
  return g3d->CreateHalo (iR, iG, iB, iAlpha, iWidth, iHeight); 
}

void csOpenGLDynamicBackBuffer::DrawPixmap (iTextureHandle *hTex, int sx, 
  int sy, int sw, int sh,int tx, int ty, int tw, int th)
{ 
  g3d->DrawPixmap (hTex, sx, sy, sw, sh, tx, ty, tw, th); 
}

iGraphics3D *csOpenGLDynamicBackBuffer::CreateOffScreenRenderer 
  (iGraphics2D *parent_g2d, int width, int height, csPixelFormat *pfmt, 
   void *buffer, RGBPixel *palette, int pal_size)
{ 
  return g3d->CreateOffScreenRenderer (parent_g2d, width, height, pfmt, 
				       buffer, palette, pal_size); 
}

