/*
    Copyright (C) 2000  by Samuel Humphreys 
    Based on the glide implementation by Norman Krämer
  
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
 * This is an hardware accelerated procedural texture renderer for opengl based
 * on Norman's Glide implementation:
 *
 * And here is the basic idea:
 * On call of a BeginDraw we copy the texture in question into the backbuffer.
 * Then normal rendering occurs.
 * On Print we copy the area rendered to back to the texture. TODO..
 * To keep the backbuffer intact we save and restore the area. TODO..
 */

#ifndef _OGL_PROCTEXBACK_H_
#define _OGL_PROCTEXBACK_H_

#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "isys/system.h"
#include "video/renderer/common/dtmesh.h"
#include "video/renderer/common/dpmesh.h"
#include "ogl_g3dcom.h"
#include "csgeom/transfrm.h"

class csTextureHandleOpenGL;
class csTextureProcOpenGL;

class csOpenGLProcBackBuffer : public csGraphics3DOGLCommon
{
 protected:
  csTextureHandleOpenGL *tex_mm;
  csTextureProcOpenGL *tex_0;

  int frame_width, frame_height, pixel_bytes;
  csPixelFormat pfmt;
  bool rstate_bilinearmap;

  /// The pair of intefaces to the frame buffer
  csGraphics3DOGLCommon *g3d;
  iGraphics2D *g2d;

  /// Does the user expect the buffer to seem to remain intact between frames?
  bool persistent;

  /// Temporarty buffer to convert glReadPixels to csRGBpixel
  char *buffer;

 public:
  csOpenGLProcBackBuffer (iBase*);
  virtual ~csOpenGLProcBackBuffer ();

  void Prepare (csGraphics3DOGLCommon *g3d, csTextureHandleOpenGL *tex, 
		csPixelFormat *ipfmt, bool bpersistent);

  virtual bool Open (const char* /*Title*/)
  { return false; }

  virtual void Close ();

  virtual bool BeginDraw (int DrawFlags);

  virtual void FinishDraw ();

  virtual void Print (csRect *area);

  float GetZBuffValue (int x, int y);
};


// We do all this just to intercept a few calls. Mostly to transform 
// y co-ordinates to a native opengl screen co-ordinate system, but also
// to be able to report the correct texture width and height of this context.
class csOpenGLProcBackBuffer2D : public iGraphics2D
{
  iGraphics2D *g2d;
  int frame_height, width, height;
  csPixelFormat *pfmt;

 public:
  SCF_DECLARE_IBASE;

  csOpenGLProcBackBuffer2D (iGraphics2D *ig2d, int iwidth, int iheight, 
			    csPixelFormat *ipfmt);  
  virtual ~csOpenGLProcBackBuffer2D ();

  virtual bool Open (const char*) { return false; }
  virtual void Close () {}

  virtual void SetClipRect (int nMinX, int nMinY, int nMaxX, int nMaxY)
  { g2d->SetClipRect (nMinX, nMinY, nMaxX, nMaxY); }
  virtual void GetClipRect (int& nMinX, int& nMinY, int& nMaxX, int& nMaxY)
  { g2d->GetClipRect (nMinX, nMinY, nMaxX, nMaxY); }

  virtual bool BeginDraw ()
  { return g2d->BeginDraw (); }

  virtual void FinishDraw ()
  { g2d->FinishDraw (); }

  virtual void Print (csRect* /*pArea*/) {};
  ///?
  virtual int GetPage ()
  { return g2d->GetPage (); }

  virtual bool DoubleBuffer (bool /*Enable*/)
  { return false; }

  virtual bool GetDoubleBufferState ()
  { return false; }

  // Use DrawBox?
  virtual void Clear (int color);

  // UseDrawBox?
  virtual void ClearAll (int color)
  { g2d->ClearAll (color); }

  virtual void DrawLine (float x1, float y1, float x2, float y2, int color);
  virtual void DrawBox (int x, int y, int w, int h, int color);

  virtual bool ClipLine (float& x1, float& y1, float& x2, float& y2,
    int xmin, int ymin, int xmax, int ymax)
  { return g2d->ClipLine (x1, y1, x2, y2, xmin, ymin, xmax, ymax); }
  virtual void DrawPixel (int x, int y, int color);
  virtual unsigned char *GetPixelAt (int x, int y);
  virtual csImageArea *SaveArea (int x, int y, int w, int h);

  virtual void RestoreArea (csImageArea *Area, bool Free)
  { g2d->RestoreArea (Area, Free); }

  virtual void FreeArea (csImageArea *Area)
  { g2d->FreeArea (Area); }

  virtual bool SetMousePosition (int /*x*/, int /*y*/)
  { return false; }

  virtual bool SetMouseCursor (csMouseCursorID /*iShape*/)
  { return false; }

  virtual void SetRGB (int i, int r, int g, int b)
  { g2d->SetRGB (i, r, g, b); }

  virtual void Write (iFont*, int x, int y, int fg, int bg, const char *str);

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

  virtual int GetWidth ();
  virtual int GetHeight ();

  virtual bool GetFullScreen ()
  { return false; }

  virtual int GetPalEntryCount ()
  { return g2d->GetPalEntryCount (); }

  virtual csRGBpixel *GetPalette ()
  { return g2d->GetPalette (); }

  virtual void GetPixel (int x, int y, UByte &oR, UByte &oG, UByte &oB);

  virtual iImage *ScreenShot () 
  { return g2d->ScreenShot(); }

  virtual iGraphics2D *CreateOffScreenCanvas 
  (int /*width*/, int /*height*/, void* /*buffer*/, bool /*hint*/, 
   csPixelFormat* /*ipfmt = NULL*/, csRGBpixel* /*palette = NULL*/, 
   int /*pal_size = 0*/)
  { return NULL; }

  virtual void AllowCanvasResize (bool /*iAllow*/)
  { }

  /// Get the active font server (does not do IncRef())
  virtual iFontServer *GetFontServer ()
  { return g2d->GetFontServer (); }
};

#endif // _OGL_PROCTEXBACK_H_
