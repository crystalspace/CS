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
#include "cs2d/common/graph2d.h"
#include "cssys/win32/win32itf.h"
#include "cs2d/ddraw61/ig2d.h"

/// Windows version.
class csGraphics2DDDraw6 : public csGraphics2D, public iGraphics2DDDraw6
{
public:
  DECLARE_IBASE;

  csGraphics2DDDraw6(iBase *iParent);
  virtual ~csGraphics2DDDraw6(void);
  
  virtual bool Initialize (iSystem *pSystem);

  virtual bool Open (char *Title);
  virtual void Close ();
  
  virtual void Print (csRect *area = NULL);
  
  virtual void SetRGB(int i, int r, int g, int b);
 
  virtual bool BeginDraw();
  virtual void FinishDraw();
  
  virtual bool SetMouseCursor (csMouseCursorID iShape);
  virtual bool SetMousePosition (int x, int y);
  virtual int GetPage ();
  virtual bool DoubleBuffer (bool Enable);
  virtual bool DoubleBuffer ();

  int m_nGraphicsReady;
  int m_nDepth;

  ///--------------- iGraphics2DDDraw6 interface implementation ---------------
  ///
  virtual void GetDirectDrawDriver (LPDIRECTDRAW4* lplpDirectDraw);
  ///
  virtual void GetDirectDrawPrimary (LPDIRECTDRAWSURFACE4* lplpDirectDrawPrimary);
  ///
  virtual void GetDirectDrawBackBuffer (LPDIRECTDRAWSURFACE4* lplpDirectDrawBackBuffer);
  ///
  virtual void GetDirectDetection (IDirectDetectionInternal** lplpDDetection);
  ///
  virtual void SetColorPalette ();
  
protected:
  LPDIRECTDRAW m_lpDD;
  LPDIRECTDRAW4 m_lpDD4;
  LPDIRECTDRAWSURFACE4 m_lpddsPrimary;
  LPDIRECTDRAWSURFACE4 m_lpddsBack;
  LPDIRECTDRAWCLIPPER m_lpddClipper;
  LPDIRECTDRAWPALETTE m_lpddPal;
  
  HWND m_hWnd;
  HINSTANCE  m_hInstance;
  int m_nCmdShow;
  bool m_bUses3D;
  bool m_bPalettized;
  bool m_bPaletteChanged;
  int m_nActivePage;
  bool m_bDisableDoubleBuffer;
  bool m_bLocked;
  
  HRESULT RestoreAll();
  unsigned char *LockBackBuf();
};

#endif
