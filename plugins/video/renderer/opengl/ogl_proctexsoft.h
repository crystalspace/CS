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

#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"

class csGraphics3DOGLCommon;
class csTextureHandleOpenGL;
class csTextureProcOpenGL;
class TxtHandleVector;
struct iSoftProcTexture;
struct iNativeWindow;

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
 * mode is implemented mostly for consistency among procedural texture 
 * implementations so the engine can render to it if necessary transparently.
 * Sharing mode also adds a further layer of complexity to the stand alone 
 * mode, but without performance penalty.
 */

class csOpenGLProcSoftware : public iGraphics3D
{
 protected:
  iObjectRegistry *object_reg;
  char *buffer;
  int width, height;
  csPixelFormat pfmt;
  /// The interface to the software procedural texture
  iSoftProcTexture *isoft_proc;
  csTextureHandleOpenGL *tex;
  /// The main renderer
  csGraphics3DOGLCommon *parent_g3d;

  /**
   * This function is called when we are in alone_mode but the most recent
   * software procedural texture isn't.  This requires that all of the
   * procedural textures take up the new mode.  In practice however it will be
   * rarely that procedural textures are not in alone mode.
   */
  void ConvertAloneMode ();

 public:
  SCF_DECLARE_IBASE;

  /// The instance of csSoftProcTexture3D this instance wraps
  iGraphics3D *g3d;

  /// The redirector to the real G2D
  iGraphics2D *dummy_g2d;

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

  /**
   * A vector of opengl texture handles and their software texture
   * counterparts.
   */
  TxtHandleVector *txts_vector;

  csOpenGLProcSoftware (iBase * pParent);
  virtual ~csOpenGLProcSoftware ();

  /// Prepare.
  bool Prepare (
    csGraphics3DOGLCommon *parent_g3d,
    csOpenGLProcSoftware *partner_tex, 
    csTextureHandleOpenGL *tex,
    csPixelFormat *pfmt,
    void *buffer,
    bool alone_hint);

  virtual bool Open (){ return false; }
  virtual void Close () {}
  virtual void SetDimensions (int, int) {}

  virtual bool BeginDraw (int DrawFlags);
  virtual void FinishDraw ();
  virtual void Print (csRect *area);

  virtual int GetWidth (){ return width; }
  virtual int GetHeight (){ return height; }

  virtual void DrawPolygon (G3DPolygonDP& poly);
  virtual void DrawPolygonDebug (G3DPolygonDP& poly);
  virtual void DrawLine (const csVector3& v1, const csVector3& v2, 
			 float fov, int color);
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
  virtual void GetPerspectiveCenter (int& x, int& y);
  virtual void SetPerspectiveAspect (float aspect);
  virtual float GetPerspectiveAspect ();
  virtual void SetObjectToCamera (csReversibleTransform* o2c);
  virtual const csReversibleTransform& GetObjectToCamera ();
  virtual void SetClipper (iClipper2D* clipper, int cliptype)
  {
    g3d->SetClipper (clipper, cliptype);
  }
  virtual iClipper2D* GetClipper () { return g3d->GetClipper (); }
  virtual int GetClipType () { return g3d->GetClipType (); }
  virtual void SetNearPlane (const csPlane3& pl)
  {
    g3d->SetNearPlane (pl);
  }
  virtual void ResetNearPlane ()
  {
    g3d->ResetNearPlane ();
  }
  virtual const csPlane3& GetNearPlane ()
  {
    return g3d->GetNearPlane ();
  }
  virtual bool HasNearPlane () { return g3d->HasNearPlane (); }
  virtual iGraphics2D *GetDriver2D ();
  virtual iTextureManager *GetTextureManager ();
  virtual iHalo *CreateHalo (float iR, float iG, float iB, 
			     unsigned char *iAlpha, int iWidth, int iHeight);
  virtual void DrawPixmap (iTextureHandle*, int sx, int sy, int sw, int sh,
    int tx, int ty, int tw, int th, uint8 Alpha);
};


class csOpenGLProcSoftware2D : public iGraphics2D
{
  iGraphics2D *g2d;

  int ConvertColour (int col)
  {
    return soft_texman->FindRGB 
      (((col&gl_pfmt->RedMask  )>>gl_pfmt->RedShift  )<<(8-gl_pfmt->RedBits  ),
       ((col&gl_pfmt->GreenMask)>>gl_pfmt->GreenShift)<<(8-gl_pfmt->GreenBits),
       ((col&gl_pfmt->BlueMask )>>gl_pfmt->BlueShift )<<(8-gl_pfmt->BlueBits));
  }

  iTextureManager *soft_texman;
  csPixelFormat *gl_pfmt;

 public:
  SCF_DECLARE_IBASE;

  csOpenGLProcSoftware2D (iGraphics3D *g3d, csPixelFormat *pfmt)
  {
    SCF_CONSTRUCT_IBASE (NULL);
    g2d = g3d->GetDriver2D ();
    soft_texman = g3d->GetTextureManager ();
    gl_pfmt = pfmt;
  }

  virtual ~csOpenGLProcSoftware2D () {};

  virtual bool Open () { return false; }
  virtual void Close () {};

  virtual void SetClipRect (int nMinX, int nMinY, int nMaxX, int nMaxY)
  { g2d->SetClipRect (nMinX, nMinY, nMaxX, nMaxY); }

  virtual void GetClipRect (int& nMinX, int& nMinY, int& nMaxX, int& nMaxY)
  { g2d->GetClipRect (nMinX, nMinY, nMaxX, nMaxY); }

  virtual bool BeginDraw ()
  { return g2d->BeginDraw (); }

  virtual void FinishDraw ()
  { g2d->FinishDraw (); }

  virtual void Print (csRect* pArea) 
  { g2d->Print (pArea); }

  virtual int GetPage ()
  { return g2d->GetPage (); }

  virtual bool DoubleBuffer (bool /*Enable*/)
  { return false; }

  virtual bool GetDoubleBufferState ()
  { return false; }

  virtual void Clear (int color)
  { g2d->Clear (ConvertColour (color)); }

  virtual void ClearAll (int color)
  { g2d->ClearAll (ConvertColour (color)); }

  virtual void DrawLine (float x1, float y1, float x2, float y2, int color)
  { g2d->DrawLine (x1, y1, x2, y2, ConvertColour (color)); }

  virtual void DrawBox (int x, int y, int w, int h, int color)
  { g2d->DrawBox (x, y, w, h, ConvertColour (color)); }

  virtual bool ClipLine (float& x1, float& y1, float& x2, float& y2,
    int xmin, int ymin, int xmax, int ymax)
  { return g2d->ClipLine (x1, y1, x2, y2, xmin, ymin, xmax, ymax); }

  virtual void DrawPixel (int x, int y, int color)
  { g2d->DrawPixel (x, y, ConvertColour (color)); }

  virtual unsigned char *GetPixelAt (int x, int y)
  { return g2d->GetPixelAt (x, y); }

  virtual csImageArea *SaveArea (int x, int y, int w, int h)
  { return g2d->SaveArea (x, y, w, h); }

  virtual void RestoreArea (csImageArea *Area, bool Free)
  { g2d->RestoreArea (Area, Free); }

  virtual void FreeArea (csImageArea *Area)
  { g2d->FreeArea (Area); }

  virtual void SetRGB (int i, int r, int g, int b)
  { g2d->SetRGB (i, r, g, b); }
  ///
  virtual void Write (iFont *font, int x, int y, int fg, int bg, const char *s)
  { 
    int cbg = (bg == -1) ? ConvertColour (bg) : bg;
    g2d->Write (font, x, y, ConvertColour (fg), cbg, s);
  }

  virtual bool PerformExtensionV (char const* command, va_list args)
  { return g2d->PerformExtensionV (command, args); }

  virtual bool PerformExtension (char const* command, ...)
  {
    va_list args;
    va_start (args, command);
    bool rc = PerformExtensionV(command, args);
    va_end (args);
    return rc;
  }

  virtual int GetPixelBytes ()
  { return g2d->GetPixelBytes (); }

  virtual csPixelFormat *GetPixelFormat ()
  { return g2d->GetPixelFormat (); }

  virtual int GetWidth ()
  { return g2d->GetWidth (); }

  virtual int GetHeight ()
  { return g2d->GetHeight (); }

  virtual int GetPalEntryCount ()
  { return g2d->GetPalEntryCount (); }

  virtual csRGBpixel *GetPalette ()
  { return g2d->GetPalette (); }

  virtual void GetPixel (int x, int y, UByte &oR, UByte &oG, UByte &oB)
  { g2d->GetPixel (x, y, oR, oG, oB); }

  virtual iImage *ScreenShot () 
  { return g2d->ScreenShot(); }

  virtual iGraphics2D *CreateOffScreenCanvas 
  (int /*width*/, int /*height*/, void* /*buffer*/, bool /*hint*/, 
   csPixelFormat* /*ipfmt = NULL*/, csRGBpixel* /*palette = NULL*/, 
   int /*pal_size = 0*/)
  { return NULL; }

  virtual void AllowResize (bool /*iAllow*/)
  {}

  virtual bool Resize (int, int)
  { return false; }

  /// Get the active font server (does not do IncRef())
  virtual iFontServer *GetFontServer ()
  { return g2d->GetFontServer (); }
  /// Get the native window.
  virtual iNativeWindow* GetNativeWindow () { return NULL; }

  virtual bool GetFullScreen ()
  { return false; }
  virtual void SetFullScreen (bool)
  {  }

  virtual bool SetMousePosition (int /*x*/, int /*y*/)
  { return false; }

  virtual bool SetMouseCursor (csMouseCursorID /*iShape*/)
  { return false; }

};

#endif // _OGL_PROCEXSOFT_H_
