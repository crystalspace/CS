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

#ifndef __DD3G2D_H__
#define __DD3G2D_H__

#include "ddraw.h"
#include "csutil/scf.h"
#include "video/canvas/common/graph2d.h"
#include "cssys/win32/win32itf.h"

/// Windows version.
class csGraphics2DDDraw3 : public csGraphics2D
{
public:
  SCF_DECLARE_IBASE;

  csGraphics2DDDraw3 (iBase *iParent);
  // Uses3D is currently not specified
  virtual ~csGraphics2DDDraw3 ();
  
  virtual bool Initialize (iSystem *pSystem);

  virtual bool Open (const char *Title);
  virtual void Close ();
  
  virtual void Print (csRect *area = NULL);
  
  virtual void SetRGB (int i, int r, int g, int b);

  virtual bool PerformExtension (const char *iCommand, ...);
	
  virtual bool BeginDraw ();
  virtual void FinishDraw ();
  
  virtual bool SetMouseCursor (csMouseCursorID iShape);

  /// Set mouse cursor position; return success status
  virtual bool SetMousePosition (int x, int y);

  virtual int GetPage ();
  virtual bool DoubleBuffer (bool Enable);
  virtual bool GetDoubleBufferState ();

protected:
  static long FAR PASCAL WindowProc (HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam);

  LPDIRECTDRAW m_lpDD;
  LPDIRECTDRAWSURFACE m_lpddsPrimary;
  LPDIRECTDRAWSURFACE m_lpddsBack;
  LPDIRECTDRAWPALETTE m_lpddPal;
  
  HWND m_hWnd;
  HINSTANCE m_hInstance;
  HPALETTE m_hWndPalette;

  // Old window procedure (the one in win32.cpp)
  WNDPROC m_OldWndProc;

  bool m_bPalettized;
  bool m_bPaletteChanged;
  int m_nActivePage;
  bool m_bDisableDoubleBuffer;
  bool m_bLocked;
  bool m_bHardwareCursor;
  
  // Saves the window size & pos.
  RECT m_rcWindow;
  // true if double buffer is enabled
  bool m_bDoubleBuffer;
  bool m_bVisible;
  bool m_bAllowWindowed;

  HRESULT InitSurfaces ();
  HRESULT ReleaseAllObjects ();
  HRESULT ChangeCoopLevel ();
  HRESULT InitFail (HRESULT hRet, LPCTSTR szError);

  void ClearSystemPalette ();
  bool CreateIdentityPalette (csRGBpixel *p);
  // Check if palette has changed and if so, realize it
  void SetColorPalette ();
  // Refresh a rectangle on client area from back buffer
  void Refresh (RECT &rect);
};

#endif
