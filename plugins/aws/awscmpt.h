 #ifndef __AWSCMPT_H__
 #define __AWSCMPT_H__
/**************************************************************************
    Copyright (C) 2000-2001 by Christopher Nelson
    	      (c) 2001 F.Richter

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
*****************************************************************************/

#include "cstool/proctex.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "csutil/typedvec.h"
#include "csgeom/transfrm.h"
#include "av3dtxtm.h"
#include "awscspt.h"
#include "awscmpt.h"
#include "csutil/cfgacc.h"

class awsMultiProctexCanvas : public iAwsCanvas
{
private:
  class awscG2D : public iGraphics2D {
  private:
    iFontServer *FontServer;
    iObjectRegistry* object_reg;
    int Width, Height;
    int ClipX1, ClipX2, ClipY1, ClipY2;
    int FrameBufferLocked;

    awsMultiProctexCanvas *awsc;

    iGraphics2D *rG2D;
  public:
    SCF_DECLARE_IBASE;

    awscG2D (awsMultiProctexCanvas *parent, iGraphics2D *aG2D);

    /// Initialize the plugin
    virtual bool Initialize (iObjectRegistry*);

    virtual bool Open ();
    virtual void Close ();

    /// Set clipping rectangle
    virtual void SetClipRect (int xmin, int ymin, int xmax, int ymax);
    /// Query clipping rectangle
    virtual void GetClipRect (int &xmin, int &ymin, int &xmax, int &ymax)
      { xmin = ClipX1; xmax = ClipX2; ymin = ClipY1; ymax = ClipY2; }

    /// Draw a pixel
    virtual void DrawPixel (int x, int y, int color);
    /// Draw a line
    virtual void DrawLine (float x1, float y1, float x2, float y2, int color);
    /// Draw a box
    virtual void DrawBox (int x, int y, int w, int h, int color);
    virtual void SetRGB (int i, int r, int g, int b)
	{(void) i; (void) r; (void) g; (void) b; };
    /// Write a text string
    virtual void Write (iFont*, int x, int y, int fg, int bg, const char *text);

    /**
     * Get address of video RAM at given x,y coordinates.
     * returns NULL at the moment
     */
    virtual unsigned char *GetPixelAt (int x, int y);

    virtual int GetPalEntryCount ();
    virtual int GetPixelBytes ();
    virtual csPixelFormat* GetPixelFormat ();

    virtual csImageArea *SaveArea (int x, int y, int w, int h)
	{ (void) x; (void) y; (void) w; (void) h; return NULL; }
    virtual void RestoreArea (csImageArea *Area, bool Free = true)
	{ (void) Area; (void) Free; };
    virtual void FreeArea (csImageArea *Area)
	{ (void) Area; };

    virtual bool ClipLine (float &x1, float &y1, float &x2, float &y2,
      int xmin, int ymin, int xmax, int ymax)
      { return rG2D->ClipLine(x1, y1, x2, y2, xmin, ymin, xmax, ymax); }

    virtual iFontServer *GetFontServer ()
	{ return FontServer; }

    virtual int GetWidth () { return Width; }
    virtual int GetHeight () { return Height; }
    virtual csRGBpixel *GetPalette () { return NULL; }
    virtual void GetPixel (int x, int y, uint8 &oR, uint8 &oG, uint8 &oB);

    virtual bool PerformExtension (char const* command, ...)
	{ (void) command; return false; };
    virtual bool PerformExtensionV (char const* command, va_list)
	{ (void) command; return false; };
    virtual iImage *ScreenShot () { return NULL; };
    virtual iGraphics2D *CreateOffScreenCanvas
    (int width, int height, void *buffer, bool alone_hint,
     csPixelFormat *pfmt = NULL, csRGBpixel *palette = NULL, int pal_size = 0)
    { (void) width; (void) height; (void) buffer; (void) alone_hint;
      (void) pfmt; (void) palette; (void) pal_size; return NULL; };

    virtual void AllowResize (bool /*iAllow*/) { };
    virtual bool Resize (int w, int h)
	{ (void) w; (void) h; return false; };
    virtual iNativeWindow* GetNativeWindow () { return NULL; };
    virtual bool GetFullScreen () { return false; }
    virtual void SetFullScreen (bool b)
	{ (void) b; };
    virtual bool SetMousePosition (int x, int y)
	{ (void) x; (void) y; return false; };
    virtual bool SetMouseCursor (csMouseCursorID iShape)
	{ (void) iShape; return false; };

    /**
     * This routine should be called before any draw operations.
     * It should return true if graphics context is ready.
     */
    virtual bool BeginDraw ();
    /// This routine should be called when you finished drawing
    virtual void FinishDraw ();

    virtual int GetPage ();
    virtual bool DoubleBuffer (bool Enable);
    virtual bool GetDoubleBufferState ();

    virtual void Clear (int color);
    virtual void ClearAll (int color);

    virtual void Print (csRect *area = NULL);
  };

  class awscG3D : public iGraphics3D
  {
    csZBufMode z_buf_mode;
    csGraphics3DCaps Caps;
    int width;
    int height;
    int width2;
    int height2;
    csPlane3 near_plane;

    float aspect;
    float inv_aspect;

    int DrawMode;

    csPixelFormat pfmt;
    csReversibleTransform o2c;

    iGraphics3D *rG3D;
    awsMultiProctexCanvas *awsc;

  public:
    SCF_DECLARE_IBASE;

    awscG2D* G2D;

    csConfigAccess config;

    csTextureManagerNull* texman;

    iObjectRegistry* object_reg;

    awscG3D (awsMultiProctexCanvas *parent, iGraphics3D *aG3D);
    virtual ~awscG3D ();

    virtual bool Initialize (iObjectRegistry *object_reg);
    bool HandleEvent (iEvent&);
    virtual bool Open ();
    virtual void Close ();

    virtual void SetDimensions (int width, int height);

    virtual bool BeginDraw (int DrawFlags);

    virtual void FinishDraw ();

    virtual void Print (csRect *area);

    virtual void DrawPolygon (G3DPolygonDP& poly);

    virtual void DrawPolygonDebug (G3DPolygonDP&) { }

    virtual void OpenFogObject (CS_ID, csFog* ) { }

    virtual void DrawFogPolygon (CS_ID, G3DPolygonDFP&, int ) { }

    virtual void CloseFogObject (CS_ID) { }

    virtual void DrawLine (const csVector3& v1, const csVector3& v2,
  	  float fov, int color);

    virtual void DrawPolygonFX (G3DPolygonDPFX& poly);

    void CacheTexture (iPolygonTexture*) { }

    void CacheInitTexture (iPolygonTexture*) { }

    void CacheSubTexture (iPolygonTexture*, int, int) { }

    void CacheRectTexture (iPolygonTexture*, int, int, int, int) { }

    virtual bool SetRenderState (G3D_RENDERSTATEOPTION op, long val);

    virtual long GetRenderState (G3D_RENDERSTATEOPTION op);

    virtual csGraphics3DCaps *GetCaps ()
    { return &Caps; }

    virtual uint32 *GetZBuffAt (int, int) { return NULL; }

    virtual void DumpCache () { }

    virtual void ClearCache () { }

    virtual void RemoveFromCache (iPolygonTexture* /*poly_texture*/) { }

    virtual int GetWidth () { return width; }
    virtual int GetHeight () { return height; }
    virtual void SetPerspectiveCenter (int x, int y);
    virtual void GetPerspectiveCenter (int& x, int& y)
    {
      x = width2;
      y = height2;
    }
    virtual void SetPerspectiveAspect (float aspect)
    {
      this->aspect = aspect;
      inv_aspect = 1./aspect;
    }
    virtual float GetPerspectiveAspect ()
    {
      return aspect;
    }
    virtual void SetObjectToCamera (csReversibleTransform* /*o2c*/)
    { }
    virtual const csReversibleTransform& GetObjectToCamera ()
    { return o2c; }

    virtual void SetClipper (iClipper2D*, int) { }
    virtual iClipper2D* GetClipper () { return NULL; }
    virtual int GetClipType () { return CS_CLIPPER_NONE; }

    virtual void SetNearPlane (const csPlane3&) { }

    virtual void ResetNearPlane () { }

    virtual const csPlane3& GetNearPlane () { return near_plane; }

    virtual bool HasNearPlane () { return false; }


    virtual void DrawTriangleMesh (G3DTriangleMesh& /*mesh*/)
    { }

    virtual void DrawPolygonMesh (G3DPolygonMesh& /*mesh*/)
    { }

    virtual iGraphics2D *GetDriver2D ()
    { return G2D; }

    virtual iTextureManager *GetTextureManager ()
    { return texman; }

    virtual iVertexBufferManager* GetVertexBufferManager ()
    { return NULL; }

    virtual float GetZBuffValue (int, int) { return 0; }

    virtual iHalo *CreateHalo (float, float, float,
      unsigned char *, int, int) { return NULL; }

    virtual void DrawPixmap (iTextureHandle*, int, int, int, int, int, int,
      int, int, uint8);
  };

  awscG2D *vG2D;
  awscG3D *vG3D;
public:
  SCF_DECLARE_IBASE;

  iGraphics2D *rG2D;
  iGraphics3D *rG3D;

  int CanvasWidth;
  int CanvasHeight;

  int HorzCanvases;
  int VertCanvases;


  awsSimpleCanvas *canvas_list;
  csRect          *rect_list;

//CS_DECLARE_TYPED_VECTOR (awsSimpleCanvasVector, awsSimpleCanvas);
//CS_DECLARE_TYPED_VECTOR (awsSimpleCanvasVectorVector, awsSimpleCanvasVector);
//  awsSimpleCanvasVectorVector canvases;

//CS_DECLARE_TYPED_VECTOR (csRectVector, csRect);
//CS_DECLARE_TYPED_VECTOR (csRectVectorVector, csRectVector);
//  csRectVectorVector rects;

  awsMultiProctexCanvas (int w, int h, iObjectRegistry* object_reg, iEngine* engine,
      	iTextureManager* txtmgr);

  virtual ~awsMultiProctexCanvas ();

  virtual void Animate (csTicks current_time);

  iGraphics2D *G2D() { return vG2D; }
  iGraphics3D *G3D() { return vG3D; }

  virtual void Show (csRect *area = NULL, iGraphics3D *g3d=NULL, uint8 Alpha=0);

  /// Returns the index to which the vertical and horizontal indices specify.
  int GetIndex(int &v, int &h)
  { return (v*VertCanvases)+h; }

  /// Gets the number of vertical and horizontal canvases
  void GetCanvasCount(int &v, int &h)
  { v = VertCanvases; h = HorzCanvases; };

  /// Gets the number of canvases total
  int GetFlatCanvasCount()
  { return VertCanvases * HorzCanvases; }

  // Gets the canvas with v,h syntax
  awsSimpleCanvas *GetCanvas(int v, int h)
  { return &canvas_list[GetIndex(v,h)]; };

  // Gets the canvas with flat syntax
  awsSimpleCanvas *GetFlatCanvas(int index)
  { return &canvas_list[index]; }

  // Gets the rect with v,h syntax
  csRect *GetCanvasRect(int v, int h)
  { return &rect_list[GetIndex(v,h)]; };

  // Gets the rect with flat syntax
  csRect *GetFlatCanvasRect(int index)
  { return &rect_list[index]; };

  // Gets the real 3d context (not one of the proctex contexts)
  iGraphics3D *realG3D()
  { return rG3D; }
};

#endif // __AWSCMPT_H__

