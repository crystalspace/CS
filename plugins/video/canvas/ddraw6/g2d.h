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

#ifndef __DD6G2D_H__
#define __DD6G2D_H__

#include "ddraw.h"
#include "cscom/com.h"
#include "cs2d/common/graph2d.h"
#include "cs2d/ddraw6/xg2d.h"

// the CLSID to create csGraphics2DDDraw6 instances.
extern const CLSID CLSID_DirectDrawDX6With3DGraphics2D;

extern const IID IID_IGraphics2DDirect3DFactory;
/// dummy interface
interface IGraphics2DDirect3DFactory : public IGraphics2DFactory
{
};

class csGraphics2DWithDirect3DFactory : public IGraphics2DDirect3DFactory
{
public:
    DECLARE_IUNKNOWN()
    DECLARE_INTERFACE_TABLE(csGraphics2DWithDirect3DFactory)

    STDMETHOD(CreateInstance)(REFIID riid, ISystem* piSystem, void** ppv);
    STDMETHOD(LockServer)(BOOL bLock);
};

/// Windows version.
class csGraphics2DDDraw6 : public csGraphics2D
{
  friend class csGraphics3DSoftware;
  friend class csGraphics3DDirect3D;
  
public:
  csGraphics2DDDraw6(ISystem* piSystem, bool bUses3D);
  virtual ~csGraphics2DDDraw6(void);
  
  virtual bool Open (const char *Title);
  virtual void Close ();
  
  virtual void Print (csRect *area = NULL);
  
  virtual void SetRGB(int i, int r, int g, int b);
 
  virtual bool BeginDraw();
  virtual void FinishDraw();
  virtual HRESULT SetColorPalette();
  
  virtual bool SetMouseCursor (int iShape, TextureMM* iBitmap);
  virtual int GetPage ();
  virtual bool DoubleBuffer (bool Enable);
  virtual bool DoubleBuffer ();

  int m_nGraphicsReady;
  int m_nDepth;
  
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
  
  bool m_bPalettized;
  bool m_bPaletteChanged;
  int m_nActivePage;
  bool m_bDisableDoubleBuffer;
  bool m_bLocked;
  
  HRESULT RestoreAll();
  unsigned char *LockBackBuf();

  DECLARE_IUNKNOWN()
  DECLARE_INTERFACE_TABLE(csGraphics2DDDraw6)
  DECLARE_COMPOSITE_INTERFACE(XDDraw6GraphicsInfo)
};

#endif
