/*
    Copyright (C) 2000 by Norman Krämer
  
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
#include "gllib.h"
#include "gldyn.h"
#include "gltex.h"
#include "g3dgl.h"

IMPLEMENT_IBASE (csGlideDynamic)
  IMPLEMENTS_INTERFACE (iGraphics3D)
IMPLEMENT_IBASE_END;


csGlideDynamic::csGlideDynamic (iBase * pParent)
{ 
  CONSTRUCT_IBASE (pParent);
  tex=NULL; 
  g3d=NULL; 
}

void csGlideDynamic::SetTarget (csGraphics3DGlide *g3d, csTextureMMGlide *tex)
{ 
  this->g3d = g3d;
  this->tex = tex;
  nPixelBytes = g3d->GetDriver2D()->GetPixelBytes ();
  tex->GetMipMapDimensions (0, Width, Height);
  tex_0 = (csTextureGlide*) tex->get_texture (0);
}

bool csGlideDynamic::BeginDraw (int DrawFlags)
{
  bool succ = g3d->BeginDraw (DrawFlags);
  if (succ){
    if (DrawFlags & CSDRAW_CLEARSCREEN){
      g3d->ClearBuffer ();
    }else
    // copy the tex into upper left corner
    g3d->DrawPixmap (tex, 0, 0, Width, Height, 0, 0, Width, Height);
  }
  return succ;
}

void csGlideDynamic::FinishDraw ()
{
  g3d->FinishDraw ();
  
  if ( GlideLib_grLfbReadRegion( GR_BUFFER_BACKBUFFER, 0, 0, Width, Height, 
				 Width*nPixelBytes, tex_0->get_bitmap () ) )
  {
    // reorder to prepare for restoring
    long n = Width*Height;
    UShort hlp;
    UShort *vram = (UShort*)tex_0->get_bitmap ();
    for (long x=0; x < n ; x+=2)
    {
      hlp = *vram;
      *vram++ = *(vram+1);
      *vram++ = hlp;
    }
    // remove it from cache, so next time its drawn the new data is guaranteed to be send to the card
    g3d->m_pTextureCache->Remove (tex);
  }
  
}

void csGlideDynamic::Print (csRect *area)
{
  (void)area;
}

void csGlideDynamic::DrawPolygon (G3DPolygonDP& poly)
{ 
  g3d->DrawPolygon (poly); 
}

void csGlideDynamic::DrawPolygonDebug (G3DPolygonDP& poly)
{ 
  g3d->DrawPolygonDebug (poly); 
}

void csGlideDynamic::DrawLine (const csVector3& v1, const csVector3& v2, float fov, int color)
{ 
  g3d->DrawLine (v1, v2, fov, color); 
}

void csGlideDynamic::StartPolygonFX (iTextureHandle* handle, UInt mode)
{ 
  g3d->StartPolygonFX (handle, mode); 
}

void csGlideDynamic::FinishPolygonFX ()
{ 
  g3d->FinishPolygonFX (); 
}

void csGlideDynamic::DrawPolygonFX (G3DPolygonDPFX& poly)
{ 
  g3d->DrawPolygonFX (poly); 
}

void csGlideDynamic::DrawTriangleMesh (G3DTriangleMesh& mesh)
{ 
  g3d->DrawTriangleMesh (mesh); 
}

void csGlideDynamic::DrawPolygonMesh (G3DPolygonMesh& mesh)
{ 
  g3d->DrawPolygonMesh (mesh); 
}

void csGlideDynamic::OpenFogObject (CS_ID id, csFog* fog)
{ 
  g3d->OpenFogObject (id, fog); 
}

void csGlideDynamic::DrawFogPolygon (CS_ID id, G3DPolygonDFP& poly, int fogtype)
{ 
  g3d->DrawFogPolygon (id, poly, fogtype);
}

void csGlideDynamic::CloseFogObject (CS_ID id)
{ 
  g3d->CloseFogObject (id); 
}

bool csGlideDynamic::SetRenderState (G3D_RENDERSTATEOPTION op, long val)
{ 
  return g3d->SetRenderState (op, val); 
}

long csGlideDynamic::GetRenderState (G3D_RENDERSTATEOPTION op)
{ 
  return g3d->GetRenderState (op); 
}

csGraphics3DCaps *csGlideDynamic::GetCaps ()
{ 
  return g3d->GetCaps (); 
}

unsigned long *csGlideDynamic::GetZBuffAt (int x, int y)
{ 
  return g3d->GetZBuffAt (x, y); 
}

float csGlideDynamic::GetZBuffValue (int x, int y)
{ 
  return g3d->GetZBuffValue (x, y); 
}

void csGlideDynamic::DumpCache ()
{ 
  g3d->DumpCache (); 
}

void csGlideDynamic::ClearCache ()
{ 
  g3d->ClearCache (); 
}

void csGlideDynamic::RemoveFromCache (iPolygonTexture* poly_texture)
{ 
  g3d->RemoveFromCache (poly_texture); 
}

void csGlideDynamic::SetPerspectiveCenter (int x, int y)
{ 
  g3d->SetPerspectiveCenter (x, y); 
}

void csGlideDynamic::SetPerspectiveAspect (float aspect)
{ 
  g3d->SetPerspectiveAspect (aspect); 
}

void csGlideDynamic::SetObjectToCamera (csReversibleTransform* o2c)
{ 
  g3d->SetObjectToCamera (o2c); 
}

void csGlideDynamic::SetClipper (csVector2* vertices, int num_vertices)
{ 
  g3d->SetClipper (vertices, num_vertices);
}

iGraphics2D *csGlideDynamic::GetDriver2D ()
{ 
  return g3d->GetDriver2D (); 
}

iTextureManager *csGlideDynamic::GetTextureManager ()
{ 
  return g3d->GetTextureManager (); 
}

iHalo *csGlideDynamic::CreateHalo (float iR, float iG, float iB, unsigned char *iAlpha, int iWidth, 
				   int iHeight)
{ 
  return g3d->CreateHalo (iR, iG, iB, iAlpha, iWidth, iHeight); 
}

void csGlideDynamic::DrawPixmap (iTextureHandle *hTex, int sx, int sy, int sw, int sh,
				 int tx, int ty, int tw, int th)
{ 
  g3d->DrawPixmap (hTex, sx, sy, sw, sh, tx, ty, tw, th); 
}

iGraphics3D *csGlideDynamic::CreateOffScreenRenderer (iGraphics2D *parent_g2d, 
  int width, int height, csPixelFormat *pfmt, void *buffer, RGBPixel *palette, 
  int pal_size)
{ 
  return g3d->CreateOffScreenRenderer (parent_g2d, width, height, pfmt, buffer,
				       palette, pal_size); 
}

