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

/**
 * This is the procedural texture renderer for glide.
 *
 * And here is the basic idea:
 * On call of a BeginDraw we copy the texture in question into the backbuffer.
 * Then normal rendering occurs.
 * On FinishDraw we copy it back into the csGlideTexture.
 * If the texture is currently in the cache we remove it to ensure, that its up to date when referenced.
 * To keep the backbuffer intact we save and restore the area.
 */

#ifndef _GLPROCTEX_H_
#define _GLPROCTEX_H_

#include "ivideo/graph3d.h"

class csGraphics3DGlide;
class csTextureHandleGlide;
class csTextureGlide;

class csGlideProcedural : public iGraphics3D
{
 protected:
  csTextureHandleGlide *tex;
  csTextureGlide *tex_0;
  csGraphics3DGlide *g3d;
  int Width, Height, nPixelBytes;
  csPixelFormat *pfmt;
  
 public:
  SCF_DECLARE_IBASE;

  csGlideProcedural (iBase * pParent);
  virtual ~csGlideProcedural (){}

  void SetTarget (csGraphics3DGlide *g3d, csTextureHandleGlide *tex);

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
  void StartPolygonFX (iMaterialHandle* handle, UInt mode); //@@@!!!
  void FinishPolygonFX (); //@@@!!!
  virtual void DrawPolygonFX (G3DPolygonDPFX& poly);
  virtual void DrawTriangleMesh (G3DTriangleMesh& mesh);
  virtual void DrawPolygonMesh (G3DPolygonMesh& mesh);
  virtual void OpenFogObject (CS_ID id, csFog* fog);
  virtual void DrawFogPolygon (CS_ID id, G3DPolygonDFP& poly, int fogtype);
  virtual void CloseFogObject (CS_ID id);
  virtual bool SetRenderState (G3D_RENDERSTATEOPTION op, long val);
  virtual long GetRenderState (G3D_RENDERSTATEOPTION op);
  virtual csGraphics3DCaps *GetCaps ();
  virtual uint32 *GetZBuffAt (int x, int y);
  virtual float GetZBuffValue (int x, int y);
  virtual void DumpCache ();
  virtual void ClearCache ();
  virtual void RemoveFromCache (iPolygonTexture* poly_texture);
  virtual void SetPerspectiveCenter (int x, int y);
  virtual void SetPerspectiveAspect (float aspect);
  virtual void SetObjectToCamera (csReversibleTransform* o2c);
  virtual void SetClipper (csVector2* vertices, int num_vertices);
  virtual void GetPerspectiveCenter (int& x, int& y);
  virtual float GetPerspectiveAspect ();
  virtual const csReversibleTransform& GetObjectToCamera ();
  virtual void GetClipper (csVector2* vertices, int& num_vertices);
  virtual iGraphics2D *GetDriver2D ();
  virtual iTextureManager *GetTextureManager ();
  virtual iHalo *CreateHalo (float iR, float iG, float iB, unsigned char *iAlpha, int iWidth, int iHeight);
  virtual void DrawPixmap (iTextureHandle *hTex, int sx, int sy, int sw, int sh,
			     int tx, int ty, int tw, int th, uint8 Alpha);
};

#endif
