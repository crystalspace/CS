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
class csGraphics2DOpenGL : public scfImplementationExt2<csGraphics2DOpenGL, 
							csGraphics2DGLCommon, 
							iOpenGLInterface,
							iWin32Canvas>
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

  virtual bool SetMouseCursor (csMouseCursorID iShape);
  virtual bool SetMouseCursor (iImage *image, const csRGBcolor* keycolor, 
                               int hotspot_x, int hotspot_y,
                               csRGBcolor fg, csRGBcolor bg);
  virtual bool SetMousePosition (int x, int y);

  virtual bool PerformExtensionV (char const* command, va_list);

  /// Set the window title
  virtual void SetTitle (const char* title);
  
  /** Sets the icon of this window with the provided one.
   *
   *  @param image the iImage to set as the icon of this window.
   */  
  virtual void SetIcon (iImage *image);
  /// Display a nice message box.
  virtual void AlertV (int type, const char* title, const char* okMsg,
  	const char* msg, va_list args);

  virtual void AllowResize (bool iAllow);

  virtual bool Resize (int width, int height);

  virtual void SetFullScreen (bool b);

  int m_nGraphicsReady;

  virtual void *GetProcAddress (const char *funcname)
  { return (void*)(wglGetProcAddress ((const char *)funcname)); }

  virtual HWND GetWindowHandle() { return m_hWnd; }

  // Vista+ window transparency
  virtual bool IsWindowTransparencyAvailable();
  virtual bool SetWindowTransparent (bool transparent);
  virtual bool GetWindowTransparent ();

  // Window decorations
  virtual bool SetWindowDecoration (WindowDecoration decoration, bool flag);
  virtual bool GetWindowDecoration (WindowDecoration decoration);
protected:

  HDC hDC;
  HGLRC hGLRC;
  HWND m_hWnd;
  HINSTANCE  m_hInstance;
  int m_nCmdShow;
  HMONITOR primaryMonitor;

  csDetectDriver detector;

  csRef<iWin32Assistant> m_piWin32Assistant;

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
  /// Window style in windowed mode
  LONG windowModeStyle;

  csWin32CustomCursors cursors;
  HICON customIcon;

  bool transparencyRequested;
  bool transparencyState;

  bool hideDecoClientFrame;

  // Get the current working area rect
  csRect GetWorkspaceRect ();
  // Compute the default window rect (centered on screen)
  void ComputeDefaultRect (RECT& windowRect, LONG style, LONG exStyle = 0);

  bool GetWorkspaceDimensions (int& width, int& height);
  bool AddWindowFrameDimensions (int& width, int& height);
};

#endif // __CS_OGLG2D_H__
