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

#ifndef __SYSG2D_H__
#define __SYSG2D_H__

#include <GL/gl.h>
#include "csutil/scf.h"
#include "video/canvas/common/graph2d.h"
#include "video/canvas/openglcommon/gl2d_font.h"
#include "video/canvas/openglcommon/glcommon2d.h"
#include "video/canvas/openglcommon/iogl.h"

struct iWin32Assistant;

/// Windows version.
class csGraphics2DOpenGL : public csGraphics2DGLCommon
{
private:
  // Calculate the OpenGL pixel format.
  void CalcPixelFormat ();

  bool RestoreDisplayMode ();

public:
  SCF_DECLARE_IBASE_EXT (csGraphics2DGLCommon);

  csGraphics2DOpenGL(iBase *iParent);
  virtual ~csGraphics2DOpenGL(void);

  virtual bool Open ();
  virtual void Close ();

  void Report (int severity, const char* msg, ...);

  virtual bool Initialize(iObjectRegistry *object_reg);

  virtual void Print (csRect *area = NULL);

  virtual void SetRGB(int i, int r, int g, int b);

  virtual HRESULT SetColorPalette();

  virtual bool SetMouseCursor (csMouseCursorID iShape);
  virtual bool SetMousePosition (int x, int y);

  virtual bool PerformExtensionV (char const* command, va_list);

  /// Set the window title
  virtual void SetTitle (const char* title);
  /// Display a nice message box.
  virtual void AlertV (int type, const char* title, const char* okMsg,
  	const char* msg, va_list args);

  int m_nGraphicsReady;

  /**
   * Get address of video RAM at given x,y coordinates.
   * The OpenGL version of this function just returns NULL.
   */
  static unsigned char* GetPixelAtGL (int x, int y);

  struct eiOpenGLInterface : public iOpenGLInterface
  {
    SCF_DECLARE_EMBEDDED_IBASE (csGraphics2DOpenGL);
    virtual void *GetProcAddress (const char *funcname)
    { return (void*)(wglGetProcAddress ((const char *)funcname)); }
  } scfiOpenGLInterface;

protected:

  HDC hDC;
  HGLRC hGLRC;
  HWND m_hWnd;
  HINSTANCE  m_hInstance;
  int m_nCmdShow;

  iWin32Assistant* m_piWin32Assistant;

  bool m_bPalettized;
  bool m_bPaletteChanged;

  bool m_bHardwareCursor;

  int DepthBits;

  // Old window procedure (the one in win32.cpp)
  WNDPROC m_OldWndProc;

  static LRESULT CALLBACK WindowProc (HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam);

  bool m_bActivated;

  void Activate (bool activate);
  // Setup fullscreen display mode
  void SwitchDisplayMode ();
};

#endif
