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

#ifndef __CS_G2D_DD8_H__
#define __CS_G2D_DD8_H__

#include <ddraw.h>

#include "csutil/scf.h"
#include "video/canvas/common/graph2d.h"
#include "i_dd8.h"

class csGraphics2DDDraw8 : public csGraphics2D, public iGraphics2DDDraw8
{

public:
  SCF_DECLARE_IBASE_EXT(csGraphics2D);

protected:

  LPDIRECTDRAW7        m_lpDD7;
  LPDIRECTDRAWSURFACE7 m_lpddsPrimary;
  LPDIRECTDRAWSURFACE7 m_lpddsBack;
  LPDIRECTDRAWSURFACE7 m_lpddsBackLeft; // For stereo mode (Not implemented yet)
  LPDIRECTDRAWCLIPPER  m_lpddClipper;
  LPDIRECTDRAWPALETTE  m_lpddPal;

  RECT m_rcWindow;
  HWND m_hWnd;
  HINSTANCE m_hInstance;
  HPALETTE m_hWndPalette;

  WNDPROC m_OldWndProc;

  int  m_nCmdShow;
  bool m_bUses3D;
  bool m_bPalettized;
  bool m_bPaletteChanged;
  int  m_nActivePage;
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
  void Refresh (RECT &rect);

public:
  csGraphics2DDDraw8 (iBase *iParent);
  virtual ~csGraphics2DDDraw8 (void);

  virtual bool Initialize (iObjectRegistry *object_reg);

  virtual bool Open ();
  virtual void Close ();

  void Report (int severity, const char* msg, ...);

  virtual void Print (csRect *area = NULL);
  virtual void SetRGB(int i, int r, int g, int b);

  virtual bool BeginDraw();
  virtual void FinishDraw();

  virtual bool SetMouseCursor (csMouseCursorID iShape);
  virtual bool SetMousePosition (int x, int y);
  virtual int  GetPage ();
  virtual bool DoubleBuffer (bool Enable);
  virtual bool GetDoubleBufferState ();
  virtual bool PerformExtensionV (char const* command, va_list);

  // iGraphics2DDDraw8
  virtual void GetDirectDrawDriver (LPDIRECTDRAW7* lplpDirectDraw);
  virtual void GetDirectDrawPrimary (LPDIRECTDRAWSURFACE7* lplpDirectDrawPrimary);
  virtual void GetDirectDrawBackBuffer (LPDIRECTDRAWSURFACE7* lplpDirectDrawBackBuffer);
  virtual void GetDirectDetection (IDirectDetectionInternal** lplpDDetection);
  virtual void SetColorPalette ();
  virtual void SetFor3D(bool For3D);
  virtual void SetModeSwitchCallback (void (*Callback) (void *), void *Data);
};

#endif // __CS_G2D_DD8_H__
