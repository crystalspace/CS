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

#ifndef _OGL_DYNTEXSOFT_H_
#define _OGL_DYNTEXSOFT_H_

#include "igraph3d.h"
class csGraphics3DOpenGL;
class csTextureMMOpenGL;
class csTextureOpenGLDynamic;
struct iGraphics2D;

class csOpenGLDynamicSoftware : public iGraphics3D
{
 protected:
  iSystem *system;
  iGraphics3D *g3d;
  char *buffer;

  csTextureMMOpenGL *tex;
  csTextureOpenGLDynamic *tex_0;
  csGraphics3DOpenGL *parent_g3d;
  iGraphics2D *g2d;
  int Width, Height, frame_height, nPixelBytes;
  csPixelFormat *pfmt;
  bool rstate_bilinearmap;
  
 public:
  DECLARE_IBASE;

  csOpenGLDynamicSoftware (iBase * pParent);
  virtual ~csOpenGLDynamicSoftware ();

  void SetTarget (csGraphics3DOpenGL *g3d, csTextureMMOpenGL *tex);

  virtual bool Initialize (iSystem */*pSystem*/){ return true; }
  virtual bool Open (const char */*Title*/){ return true; }
  virtual void Close () {}
  virtual void SetDimensions (int /*width*/, int /*height*/){}

  virtual bool BeginDraw (int DrawFlags);
  virtual void FinishDraw ();
  virtual void Print (csRect *area);

  virtual int GetWidth (){ return Width; }
  virtual int GetHeight (){ return Height; }

  virtual void DrawPolygon (G3DPolygonDP& poly);
  virtual void DrawPolygonDebug (G3DPolygonDP& poly);
  virtual void DrawLine (const csVector3& v1, const csVector3& v2, float fov, int color);
  virtual void StartPolygonFX (iTextureHandle* handle, UInt mode);
  virtual void FinishPolygonFX ();
  virtual void DrawPolygonFX (G3DPolygonDPFX& poly);
  virtual void DrawTriangleMesh (G3DTriangleMesh& mesh);
  virtual void DrawPolygonMesh (G3DPolygonMesh& mesh);
  virtual void OpenFogObject (CS_ID id, csFog* fog);
  virtual void DrawFogPolygon (CS_ID id, G3DPolygonDFP& poly, int fogtype);
  virtual void CloseFogObject (CS_ID id);
  virtual bool SetRenderState (G3D_RENDERSTATEOPTION op, long val);
  virtual long GetRenderState (G3D_RENDERSTATEOPTION op);
  virtual csGraphics3DCaps *GetCaps ();
  virtual unsigned long *GetZBuffAt (int x, int y);
  virtual float GetZBuffValue (int x, int y);
  virtual void DumpCache ();
  virtual void ClearCache ();
  virtual void RemoveFromCache (iPolygonTexture* poly_texture);
  virtual void SetPerspectiveCenter (int x, int y);
  virtual void SetPerspectiveAspect (float aspect);
  virtual void SetObjectToCamera (csReversibleTransform* o2c);
  virtual void SetClipper (csVector2* vertices, int num_vertices);
  virtual iGraphics2D *GetDriver2D ();
  virtual iTextureManager *GetTextureManager ();
  virtual iHalo *CreateHalo (float iR, float iG, float iB, unsigned char *iAlpha, int iWidth, int iHeight);
  virtual void DrawPixmap (iTextureHandle *hTex, int sx, int sy, int sw, int sh,
			     int tx, int ty, int tw, int th);
  virtual iGraphics3D *CreateOffScreenRenderer (iGraphics2D *parent_g2d, 
    int width, int height, csPixelFormat *pfmt, void *buffer, 
    RGBPixel *palette, int pal_size);

};

#endif // _OGL_DYNTEXSOFT_H_
