/*
    Copyright (C) 2000 by Samuel Humphreys
  
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
#include "ogl_dyntexsoft.h"
#include "ogl_g3d.h"
#include "isystem.h"

IMPLEMENT_IBASE (csOpenGLDynamicSoftware)
  IMPLEMENTS_INTERFACE (iGraphics3D)
IMPLEMENT_IBASE_END;

#define SysPrintf system->Printf


csOpenGLDynamicSoftware::csOpenGLDynamicSoftware (iBase * pParent)
{ 
  CONSTRUCT_IBASE (pParent);
  tex=NULL; 
  g3d=NULL; 
  rstate_bilinearmap = false;
}

csOpenGLDynamicSoftware::~csOpenGLDynamicSoftware ()
{
  if (g3d) g3d->DecRef ();
}

void csOpenGLDynamicSoftware::SetTarget (csGraphics3DOpenGL *parent_g3d, 
  csTextureMMOpenGL *tex)
{ 
  system = parent_g3d->System;
  this->g3d = parent_g3d;
  this->tex = tex;
  g2d = g3d->GetDriver2D();
  nPixelBytes = g2d->GetPixelBytes ();
  frame_height = g2d->GetHeight ();
  tex->GetMipMapDimensions (0, Width, Height);
  tex_0 = (csTextureOpenGLDynamic*) tex->get_texture (0);
}

void csOpenGLDynamicSoftware::Print (csRect */*area*/)
{
  glEnable (GL_TEXTURE_2D);
  glDisable (GL_BLEND);
  glDisable (GL_DEPTH_TEST);
  glDisable (GL_ALPHA_TEST);

  csGLCacheData *tex_data = (csGLCacheData*) tex->GetCacheData();
  if (tex_data)
  {
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    // Texture is in tha cache, update texture directly.

    glBindTexture (GL_TEXTURE_2D, tex_data->Handle);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
		    Width, Height,
		    GL_RGBA, GL_UNSIGNED_BYTE, buffer);
  }
  else
  {
    // Not in cache. Sharing buffer with texture so do nothing

  }
}

iGraphics3D *csOpenGLDynamicSoftware::CreateOffScreenRenderer 
  (iGraphics2D */*parent_g2d*/, int width, int height, csPixelFormat *pfmt, 
   void *buffer, RGBPixel *palette, int pal_size)
{
  g3d = LOAD_PLUGIN (system, 
    "crystalspace.graphics3d.software.offscreen", NULL, iGraphics3D);
  if (!g3d)
  {
    SysPrintf (MSG_FATAL_ERROR, "Error creating offscreen software renderer\n");
    return NULL;
  }

  this->buffer = (char*) buffer;
  return g3d->CreateOffScreenRenderer (g2d, width, height, pfmt, 
				       buffer, palette, pal_size); 
}

bool csOpenGLDynamicSoftware::BeginDraw (int DrawFlags)
{
  return g3d->BeginDraw (DrawFlags);
}

void csOpenGLDynamicSoftware::FinishDraw ()
{
  g3d->FinishDraw ();
}

void csOpenGLDynamicSoftware::DrawPolygon (G3DPolygonDP& poly)
{ 
  g3d->DrawPolygon (poly); 
}

void csOpenGLDynamicSoftware::DrawPolygonDebug (G3DPolygonDP& poly)
{ 
  g3d->DrawPolygonDebug (poly); 
}

void csOpenGLDynamicSoftware::DrawLine (const csVector3& v1, const csVector3& v2, float fov, int color)
{ 
  g3d->DrawLine (v1, v2, fov, color); 
}

void csOpenGLDynamicSoftware::StartPolygonFX (iTextureHandle* handle, UInt mode)
{ 
  g3d->StartPolygonFX (handle, mode); 
}

void csOpenGLDynamicSoftware::FinishPolygonFX ()
{ 
  g3d->FinishPolygonFX (); 
}

void csOpenGLDynamicSoftware::DrawPolygonFX (G3DPolygonDPFX& poly)
{ 
  g3d->DrawPolygonFX (poly); 
}

void csOpenGLDynamicSoftware::DrawTriangleMesh (G3DTriangleMesh& mesh)
{ 
  g3d->DrawTriangleMesh (mesh); 
}

void csOpenGLDynamicSoftware::DrawPolygonMesh (G3DPolygonMesh& mesh)
{ 
  g3d->DrawPolygonMesh (mesh); 
}

void csOpenGLDynamicSoftware::OpenFogObject (CS_ID id, csFog* fog)
{ 
  g3d->OpenFogObject (id, fog); 
}

void csOpenGLDynamicSoftware::DrawFogPolygon (CS_ID id, G3DPolygonDFP& poly, int fogtype)
{ 
  g3d->DrawFogPolygon (id, poly, fogtype);
}

void csOpenGLDynamicSoftware::CloseFogObject (CS_ID id)
{ 
  g3d->CloseFogObject (id); 
}

bool csOpenGLDynamicSoftware::SetRenderState (G3D_RENDERSTATEOPTION op, long val)
{ 
  return g3d->SetRenderState (op, val); 
}

long csOpenGLDynamicSoftware::GetRenderState (G3D_RENDERSTATEOPTION op)
{ 
  return g3d->GetRenderState (op); 
}

csGraphics3DCaps *csOpenGLDynamicSoftware::GetCaps ()
{ 
  return g3d->GetCaps (); 
}

unsigned long *csOpenGLDynamicSoftware::GetZBuffAt (int x, int y)
{ 
  return g3d->GetZBuffAt (x, y); 
}

float csOpenGLDynamicSoftware::GetZBuffValue (int x, int y)
{ 
  return g3d->GetZBuffValue (x, y); 
}

void csOpenGLDynamicSoftware::DumpCache ()
{ 
  g3d->DumpCache (); 
}

void csOpenGLDynamicSoftware::ClearCache ()
{ 
  g3d->ClearCache (); 
}

void csOpenGLDynamicSoftware::RemoveFromCache (iPolygonTexture* poly_texture)
{ 
  g3d->RemoveFromCache (poly_texture); 
}

void csOpenGLDynamicSoftware::SetPerspectiveCenter (int x, int y)
{ 
  g3d->SetPerspectiveCenter (x, y); 
}

void csOpenGLDynamicSoftware::SetPerspectiveAspect (float aspect)
{ 
  g3d->SetPerspectiveAspect (aspect); 
}

void csOpenGLDynamicSoftware::SetObjectToCamera (csReversibleTransform* o2c)
{ 
  g3d->SetObjectToCamera (o2c); 
}

void csOpenGLDynamicSoftware::SetClipper (csVector2* vertices, int num_vertices)
{ 
  g3d->SetClipper (vertices, num_vertices);
}

iGraphics2D *csOpenGLDynamicSoftware::GetDriver2D ()
{ 
  return g3d->GetDriver2D (); 
}

iTextureManager *csOpenGLDynamicSoftware::GetTextureManager ()
{ 
  return g3d->GetTextureManager (); 
}

iHalo *csOpenGLDynamicSoftware::CreateHalo (float iR, float iG, float iB, unsigned char *iAlpha, int iWidth, 
				   int iHeight)
{ 
  return g3d->CreateHalo (iR, iG, iB, iAlpha, iWidth, iHeight); 
}

void csOpenGLDynamicSoftware::DrawPixmap (iTextureHandle *hTex, int sx, 
  int sy, int sw, int sh,int tx, int ty, int tw, int th)
{ 
  g3d->DrawPixmap (hTex, sx, sy, sw, sh, tx, ty, tw, th); 
}



