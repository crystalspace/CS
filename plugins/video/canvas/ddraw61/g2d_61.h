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

#ifndef __DD61G2D_H__
#define __DD61G2D_H__

#include "ddraw.h"
#include "csutil/scf.h"
#include "video/canvas/common/graph2d.h"
#include "cssys/win32/win32itf.h"
#include "video/canvas/ddraw61/ig2d.h"

/// Windows version.
class csGraphics2DDDraw6 : public csGraphics2D, public iGraphics2DDDraw6
{
public:
  DECLARE_IBASE;

  csGraphics2DDDraw6(iBase *iParent);
  virtual ~csGraphics2DDDraw6(void);
  
  virtual bool Initialize (iSystem *pSystem);

  virtual bool Open (const char *Title);
  virtual void Close ();
  
  virtual void Print (csRect *area = NULL);
  
  virtual void SetRGB(int i, int r, int g, int b);
 
  virtual bool BeginDraw();
  virtual void FinishDraw();
  
  virtual bool SetMouseCursor (csMouseCursorID iShape);
  virtual bool SetMousePosition (int x, int y);
  virtual int GetPage ();
  virtual bool DoubleBuffer (bool Enable);
  virtual bool GetDoubleBufferState ();
  virtual bool PerformExtension (const char *iCommand, ...);

  ///--------------- iGraphics2DDDraw6 interface implementation ---------------
  ///
  virtual void GetDirectDrawDriver (LPDIRECTDRAW4* lplpDirectDraw);
  ///
  virtual void GetDirectDrawPrimary (LPDIRECTDRAWSURFACE4* lplpDirectDrawPrimary);
  ///
  virtual void GetDirectDrawBackBuffer (LPDIRECTDRAWSURFACE4* lplpDirectDrawBackBuffer);
  ///
  virtual void GetDirectDetection (IDirectDetectionInternal** lplpDDetection);
  /// Check if palette has changed and if so, realize it
  virtual void SetColorPalette ();
  ///
  virtual void SetFor3D(bool For3D);
  ///
  virtual void SetModeSwitchCallback (void (*Callback) (void *), void *Data);

protected:
  LPDIRECTDRAW4 m_lpDD4;
  LPDIRECTDRAWSURFACE4 m_lpddsPrimary;
  LPDIRECTDRAWSURFACE4 m_lpddsBack;
  LPDIRECTDRAWCLIPPER m_lpddClipper;
  LPDIRECTDRAWPALETTE m_lpddPal;
  
  // Saves the window size & pos.
  RECT m_rcWindow;
  HWND m_hWnd;
  HINSTANCE m_hInstance;
  HPALETTE m_hWndPalette;

  // Old window procedure (the one in win32.cpp)
  WNDPROC m_OldWndProc;

  int m_nCmdShow;
  bool m_bUses3D;
  bool m_bPalettized;
  bool m_bPaletteChanged;
  int m_nActivePage;
  bool m_bLocked;
  bool m_bDoubleBuffer;
  bool m_bAllowWindowed;

  void (*D3DCallback) (void *);
  void *D3DCallbackData;
  
  static long FAR PASCAL WindowProc (HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam);
  HRESULT InitSurfaces ();
  HRESULT ReleaseAllObjects ();
  HRESULT InitFail (HRESULT hRet, LPCTSTR szError);
  HRESULT ChangeCoopLevel ();
  void ClearSystemPalette ();
  bool CreateIdentityPalette (csRGBpixel *p);
  // Refresh a rectangle on client area from back buffer
  void Refresh (RECT &rect);
};

#endif
