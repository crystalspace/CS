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

#include <gl/gl.h>
#include "cscom/com.h"
#include "cs2d/common/graph2d.h"
#include "cssys/win32/win32itf.h"
#include "cs2d/openglwin/xsysg2d.h"
#include "cs2d/openglcommon/gl2d_font.h"
#include "cs2d/openglcommon/glcommon2d.h"


class csTextureHandle;
class csGraphics2DOpenGLFontServer;
class OpenGLTextureCache;

// the CLSID to create csGraphics2DWin32 instances.
extern const CLSID CLSID_OpenGLGraphics2D;

extern const IID IID_IGraphics2DOpenGLFactory;
/// dummy interface
interface IGraphics2DOpenGLFactory : public IGraphics2DFactory
{
};

///
class csGraphics2DOpenGLFactory : public IGraphics2DOpenGLFactory 
{
public:
    DECLARE_IUNKNOWN()
    DECLARE_INTERFACE_TABLE(csGraphics2DOpenGLFactory)

    STDMETHOD(CreateInstance)(REFIID riid, ISystem* piSystem, void** ppv);
    STDMETHOD(LockServer)(BOOL bLock);
};

/// Windows version.
class csGraphics2DOpenGL : public csGraphics2DGLCommon
{
  friend class csGraphics3DOpenGL;

private:
  // Calculate the OpenGL pixel format.
  void CalcPixelFormat ();

public:
  csGraphics2DOpenGL(ISystem* piSystem, bool bUses3D);
  virtual ~csGraphics2DOpenGL(void);
  
  virtual bool Open (const char *Title);
  virtual void Close ();
  
  virtual void Initialize();

  virtual void Print (csRect *area = NULL);
  
  virtual void SetRGB(int i, int r, int g, int b);
 
  virtual bool BeginDraw();
  virtual void FinishDraw();
  virtual HRESULT SetColorPalette();
  
  virtual bool SetMouseCursor (int iShape, csTextureHandle* iBitmap);
  virtual bool SetMousePosition (int x, int y);
  virtual int GetPage ();
  virtual bool DoubleBuffer (bool Enable);
  virtual bool DoubleBuffer ();

  int m_nGraphicsReady;

  /**
   * Get address of video RAM at given x,y coordinates.
   * The OpenGL version of this function just returns NULL.
   */
  static unsigned char* GetPixelAtGL (int x, int y);

protected:
  
  HDC hDC;
  HGLRC hGLRC;
  HWND m_hWnd;
  HINSTANCE  m_hInstance;
  int m_nCmdShow;
  
  IWin32SystemDriver* m_piWin32System;

  bool m_bPalettized;
  bool m_bPaletteChanged;
  int m_nActivePage;
  bool m_bDisableDoubleBuffer;
  
  HRESULT RestoreAll();
  unsigned char *LockBackBuf();

  DECLARE_IUNKNOWN()
  DECLARE_INTERFACE_TABLE(csGraphics2DOpenGL)
  DECLARE_COMPOSITE_INTERFACE(XOpenGLGraphicsInfo)
};

#endif
