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
#include "glproc.h"
#include "gltex.h"
#include "g3dgl.h"

IMPLEMENT_IBASE (csGlideProcedural)
  IMPLEMENTS_INTERFACE (iGraphics3D)
IMPLEMENT_IBASE_END;


csGlideProcedural::csGlideProcedural (iBase * pParent)
{ 
  CONSTRUCT_IBASE (pParent);
  tex=NULL; 
  g3d=NULL; 
}

void csGlideProcedural::SetTarget (csGraphics3DGlide *g3d, csTextureHandleGlide *tex)
{ 
  this->g3d = g3d;
  this->tex = tex;
  nPixelBytes = g3d->GetDriver2D()->GetPixelBytes ();
  tex->GetMipMapDimensions (0, Width, Height);
  tex_0 = (csTextureGlide*) tex->get_texture (0);
}

bool csGlideProcedural::BeginDraw (int DrawFlags)
{
  bool succ = g3d->BeginDraw (DrawFlags);
  if (succ)
  {
    if (DrawFlags & CSDRAW_CLEARSCREEN)
      g3d->ClearBuffer ();
    else
      // copy the tex into upper left corner
      g3d->DrawPixmap (tex, 0, 0, Width, Height, 0, 0, Width, Height, 0);
  }
  return succ;
}

void csGlideProcedural::FinishDraw ()
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

void csGlideProcedural::Print (csRect *area)
{
  (void)area;
}

void csGlideProcedural::DrawPolygon (G3DPolygonDP& poly)
{ 
  g3d->DrawPolygon (poly); 
}

void csGlideProcedural::DrawPolygonDebug (G3DPolygonDP& poly)
{ 
  g3d->DrawPolygonDebug (poly); 
}

void csGlideProcedural::DrawLine (const csVector3& v1, const csVector3& v2, float fov, int color)
{ 
  g3d->DrawLine (v1, v2, fov, color); 
}

void csGlideProcedural::StartPolygonFX (iMaterialHandle* handle, UInt mode)
{ 
  g3d->StartPolygonFX (handle, mode); 
}

void csGlideProcedural::FinishPolygonFX ()
{ 
  g3d->FinishPolygonFX (); 
}

void csGlideProcedural::DrawPolygonFX (G3DPolygonDPFX& poly)
{ 
  g3d->DrawPolygonFX (poly); 
}

void csGlideProcedural::DrawTriangleMesh (G3DTriangleMesh& mesh)
{ 
  g3d->DrawTriangleMesh (mesh); 
}

void csGlideProcedural::DrawPolygonMesh (G3DPolygonMesh& mesh)
{ 
  g3d->DrawPolygonMesh (mesh); 
}

void csGlideProcedural::OpenFogObject (CS_ID id, csFog* fog)
{ 
  g3d->OpenFogObject (id, fog); 
}

void csGlideProcedural::DrawFogPolygon (CS_ID id, G3DPolygonDFP& poly, int fogtype)
{ 
  g3d->DrawFogPolygon (id, poly, fogtype);
}

void csGlideProcedural::CloseFogObject (CS_ID id)
{ 
  g3d->CloseFogObject (id); 
}

bool csGlideProcedural::SetRenderState (G3D_RENDERSTATEOPTION op, long val)
{ 
  return g3d->SetRenderState (op, val); 
}

long csGlideProcedural::GetRenderState (G3D_RENDERSTATEOPTION op)
{ 
  return g3d->GetRenderState (op); 
}

csGraphics3DCaps *csGlideProcedural::GetCaps ()
{ 
  return g3d->GetCaps (); 
}

uint32 *csGlideProcedural::GetZBuffAt (int x, int y)
{ 
  return g3d->GetZBuffAt (x, y); 
}

float csGlideProcedural::GetZBuffValue (int x, int y)
{ 
  return g3d->GetZBuffValue (x, y); 
}

void csGlideProcedural::DumpCache ()
{ 
  g3d->DumpCache (); 
}

void csGlideProcedural::ClearCache ()
{ 
  g3d->ClearCache (); 
}

void csGlideProcedural::RemoveFromCache (iPolygonTexture* poly_texture)
{ 
  g3d->RemoveFromCache (poly_texture); 
}

void csGlideProcedural::SetPerspectiveCenter (int x, int y)
{ 
  g3d->SetPerspectiveCenter (x, y); 
}

void csGlideProcedural::SetPerspectiveAspect (float aspect)
{ 
  g3d->SetPerspectiveAspect (aspect); 
}

void csGlideProcedural::SetObjectToCamera (csReversibleTransform* o2c)
{ 
  g3d->SetObjectToCamera (o2c); 
}

void csGlideProcedural::SetClipper (csVector2* vertices, int num_vertices)
{ 
  g3d->SetClipper (vertices, num_vertices);
}

void csGlideProcedural::GetPerspectiveCenter (int& x, int& y)
{ 
  g3d->GetPerspectiveCenter (x, y); 
}

float csGlideProcedural::GetPerspectiveAspect ()
{ 
  return g3d->GetPerspectiveAspect (); 
}

const csReversibleTransform& csGlideProcedural::GetObjectToCamera ()
{ 
  return g3d->GetObjectToCamera (); 
}

void csGlideProcedural::GetClipper (csVector2* vertices, int& num_vertices)
{ 
  g3d->GetClipper (vertices, num_vertices);
}

iGraphics2D *csGlideProcedural::GetDriver2D ()
{ 
  return g3d->GetDriver2D (); 
}

iTextureManager *csGlideProcedural::GetTextureManager ()
{ 
  return g3d->GetTextureManager (); 
}

iHalo *csGlideProcedural::CreateHalo (float iR, float iG, float iB, unsigned char *iAlpha, int iWidth, 
				   int iHeight)
{ 
  return g3d->CreateHalo (iR, iG, iB, iAlpha, iWidth, iHeight); 
}

void csGlideProcedural::DrawPixmap (iTextureHandle *hTex, int sx, int sy, int sw, int sh,
				 int tx, int ty, int tw, int th, uint8 Alpha)
{ 
  g3d->DrawPixmap (hTex, sx, sy, sw, sh, tx, ty, tw, th, Alpha); 
}
