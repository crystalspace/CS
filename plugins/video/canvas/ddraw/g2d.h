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

#ifndef __CS_DD3G2D_H__
#define __CS_DD3G2D_H__

#include <ddraw.h>
#include "csutil/scf.h"
#include "csplugincommon/canvas/graph2d.h"
#include "csplugincommon/win32/customcursor.h"

/// Windows version.
class csGraphics2DDDraw3 : public csGraphics2D
{
public:
  csGraphics2DDDraw3 (iBase *iParent);
  virtual ~csGraphics2DDDraw3 ();

  virtual bool Initialize (iObjectRegistry*);

  virtual bool Open ();
  virtual void Close ();

  void Report (int severity, const char* msg, ...);

  virtual void Print (csRect const* area = 0);

  virtual void SetRGB (int i, int r, int g, int b);

  virtual bool PerformExtensionV (char const* command, va_list);

  virtual bool BeginDraw ();
  virtual void FinishDraw ();

  virtual bool SetMouseCursor (csMouseCursorID iShape);
  virtual bool SetMouseCursor (iImage *image, const csRGBcolor* keycolor, 
                               int hotspot_x, int hotspot_y,
                               csRGBcolor fg, csRGBcolor bg);

  /// Set mouse cursor position; return success status
  virtual bool SetMousePosition (int x, int y);

  virtual int GetPage ();
  virtual bool DoubleBuffer (bool Enable);
  virtual bool GetDoubleBufferState ();

  /// Set the window title
  virtual void SetTitle (const char* title);
  /// Display a message box
  virtual void AlertV (int type, const char* title, 
    const char* okMsg, const char* msg, va_list args);
protected:
  static LRESULT CALLBACK WindowProc (HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam);

  LPDIRECTDRAW m_lpDD;
  LPDIRECTDRAWSURFACE m_lpddsPrimary;
  LPDIRECTDRAWSURFACE m_lpddsBack;
  LPDIRECTDRAWPALETTE m_lpddPal;

  HWND m_hWnd;
  HINSTANCE m_hInstance;
  HPALETTE m_hWndPalette;

  HDC hdc;

  csRef<iWin32Assistant> m_piWin32Assistant;

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

  DirectDetection DDetection;
  const DirectDetectionDevice *DirectDevice;

  csWin32CustomCursors cursors;

  void ClearSystemPalette ();
  bool CreateIdentityPalette (csRGBpixel *p);
  // Check if palette has changed and if so, realize it
  void SetColorPalette ();
  // Refresh a rectangle on client area from back buffer
  void Refresh (RECT &rect);
};

#endif // __CS_DD3G2D_H__
