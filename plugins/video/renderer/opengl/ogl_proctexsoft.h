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

#ifndef _OGL_PROCTEXSOFT_H_
#define _OGL_PROCTEXSOFT_H_

#include "igraph3d.h"
#include "igraph2d.h" // for csPixelFormat

class csGraphics3DOGLCommon;
class csTextureMMOpenGL;
class csTextureProcOpenGL;
class TxtHandleVector;
struct iSoftProcTexture;

/**
 * This class implements the functionality required to utilise a software 
 * renderer for its procedural textures. The reason for having this option is
 * that under some circumstances this will be faster than the back buffer
 * and auxiliary buffer implementations. glCopyTexSubImage2D and 
 * glCopyTexImage2D are notoriously underoptimised/disfunctional among some 
 * opengl implementations/drivers/gfx-cards.
 *
 * How it works: There are the stand alone and sharing modes. It is strongly 
 * recommended that this implementation is not used in 'sharing' mode. Sharing
 *  mode is implemented mostly for consistency among procedural texture 
 * implementations so the engine can render to it if necessary transparently.
 *  Sharing mode also adds a further layer of complexity to the stand alone 
 * mode, but without performance penalty.
 */

class csOpenGLProcSoftware : public iGraphics3D
{
 protected:
  iSystem *system;
  char *buffer;
  int width, height;
  csPixelFormat pfmt;
  /// The interface to the software procedural texture
  iSoftProcTexture *isoft_proc;
  csTextureMMOpenGL *tex;
  /// The main renderer
  csGraphics3DOGLCommon *parent_g3d;

  /**
   * This function is called when we are in alone_mode but the most recent
   * software procedural texture isn't. This requires that all of the procedural
   * textures take up the new mode.
   * In practice however it will be rarely that procedural textures are not in 
   * alone mode.
   */
  void ConvertAloneMode ();

 public:
  DECLARE_IBASE;

  /// The instance of csSoftProcTexture3D this instance wraps
  iGraphics3D *g3d;

  /// We keep a singly linked list of software textures
  csOpenGLProcSoftware *head_soft_tex;
  csOpenGLProcSoftware *next_soft_tex;

  /**
   * The mode the software textures are in. If true then the software texture
   * manager is not updated with procedural textures. If one of the software
   * procedural textures registers with this as false then all procedural 
   * textures are changed to false.
   */
  bool alone_mode;

  /// A vector of opengl texture handles and their software texture counterparts
  TxtHandleVector *txts_vector;

  csOpenGLProcSoftware (iBase * pParent);
  virtual ~csOpenGLProcSoftware ();

  /// Prepare.
  bool Prepare 
    (csGraphics3DOGLCommon *parent_g3d, csOpenGLProcSoftware *partner_tex, 
    csTextureMMOpenGL *tex, csPixelFormat *pfmt, void *buffer, bool alone_hint);

  virtual bool Initialize (iSystem * /*pSystem*/){ return false; }
  virtual bool Open (const char * /*Title*/){ return false; }
  virtual void Close () {}
  virtual void SetDimensions (int /*width*/, int /*height*/){}

  virtual bool BeginDraw (int DrawFlags);
  virtual void FinishDraw ();
  virtual void Print (csRect *area);

  virtual int GetWidth (){ return width; }
  virtual int GetHeight (){ return height; }

  virtual void DrawPolygon (G3DPolygonDP& poly);
  virtual void DrawPolygonDebug (G3DPolygonDP& poly);
  virtual void DrawLine (const csVector3& v1, const csVector3& v2, 
			 float fov, int color);
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
  virtual iHalo *CreateHalo (float iR, float iG, float iB, 
			     unsigned char *iAlpha, int iWidth, int iHeight);
  virtual void DrawPixmap (iTextureHandle *hTex, int sx, int sy, int sw, int sh,
			     int tx, int ty, int tw, int th);
};

#endif // _OGL_PROCEXSOFT_H_
