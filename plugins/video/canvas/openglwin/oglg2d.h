/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#ifndef __CS_OGLG2D_H__
#define __CS_OGLG2D_H__

#include <GL/gl.h>
#include "csutil/scf.h"
#include "csplugincommon/canvas/graph2d.h"
#include "csplugincommon/opengl/glcommon2d.h"
#include "csplugincommon/iopengl/openglinterface.h"
#include "csplugincommon/win32/customcursor.h"

#include "detectdriver.h"

struct iWin32Assistant;

/// Windows version.
class csGraphics2DOpenGL : public scfImplementationExt1<csGraphics2DOpenGL, 
							csGraphics2DGLCommon, 
							iOpenGLInterface>
{
private:
  struct DummyWndInfo
  {
    int pixelFormat;
    csGraphics2DOpenGL* this_;
    GLPixelFormat* chosenFormat;
    csGLPixelFormatPicker* picker;
  };
  // Calculate the OpenGL pixel format.
  int FindPixelFormatGDI (HDC hDC, csGLPixelFormatPicker& picker);
  int FindPixelFormatWGL (csGLPixelFormatPicker& picker);
  static LRESULT CALLBACK DummyWindow (HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam);

  bool RestoreDisplayMode ();
public:
  virtual const char* GetRendererString (const char* str);
  virtual const char* GetVersionString (const char* ver);

  csGraphics2DOpenGL(iBase *iParent);
  virtual ~csGraphics2DOpenGL(void);

  virtual bool Open ();
  virtual void Close ();

  void Report (int severity, const char* msg, ...);

  virtual bool Initialize(iObjectRegistry *object_reg);

  virtual void Print (csRect const* area = 0);

  virtual void SetRGB(int i, int r, int g, int b);

  virtual HRESULT SetColorPalette();

  virtual bool SetMouseCursor (csMouseCursorID iShape);
  virtual bool SetMouseCursor (iImage *image, const csRGBcolor* keycolor, 
                               int hotspot_x, int hotspot_y,
                               csRGBcolor fg, csRGBcolor bg);
  virtual bool SetMousePosition (int x, int y);

  virtual bool PerformExtensionV (char const* command, va_list);

  /// Set the window title
  virtual void SetTitle (const char* title);
  /// Display a nice message box.
  virtual void AlertV (int type, const char* title, const char* okMsg,
  	const char* msg, va_list args);

  virtual void AllowResize (bool iAllow);

  virtual bool Resize (int width, int height);

  virtual void SetFullScreen (bool b);

  int m_nGraphicsReady;

  /**
   * Get address of video RAM at given x,y coordinates.
   * The OpenGL version of this function just returns 0.
   */
  static unsigned char* GetPixelAtGL (int x, int y);

  virtual void *GetProcAddress (const char *funcname)
  { return (void*)(wglGetProcAddress ((const char *)funcname)); }
protected:

  HDC hDC;
  HGLRC hGLRC;
  HWND m_hWnd;
  HINSTANCE  m_hInstance;
  int m_nCmdShow;

  csDetectDriver detector;

  csRef<iWin32Assistant> m_piWin32Assistant;

  bool m_bPalettized;
  bool m_bPaletteChanged;

  bool m_bHardwareCursor;

  uint m_nDisplayFrequency;

  // Old window procedure (the one in win32.cpp)
  WNDPROC m_OldWndProc;

  static LRESULT CALLBACK WindowProc (HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam);

  bool m_bActivated;

  /// Window is de-/activated. Perform stuff like modeswitches.
  void Activate (bool activate);

  /// true if the screen mode was actually changed
  bool modeSwitched;
  /// Setup/leave fullscreen display mode
  void SwitchDisplayMode (bool userMode);
  /// hardware accelerated?
  bool hardwareAccelerated;

  csWin32CustomCursors cursors;
};

#endif // __CS_OGLG2D_H__
