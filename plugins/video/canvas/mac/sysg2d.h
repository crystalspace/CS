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

#include "cscom/com.h"
#include "cs2d/common/graph2d.h"
#include "xsysg2d.h"

///
class csGraphics2DMacFactory : public IGraphics2DFactory 
{
public:
    DECLARE_IUNKNOWN()
    DECLARE_INTERFACE_TABLE(csGraphics2DMacFactory)

    STDMETHOD(CreateInstance)(REFIID riid, ISystem* piSystem, void** ppv);
    STDMETHOD(LockServer)(BOOL bLock);
};

/// Macintosh version.
class csGraphics2DMac : public csGraphics2D
{
  friend class csGraphics3DSoftware;
  
public:
  csGraphics2DMac(ISystem* piSystem);
  virtual ~csGraphics2DMac(void);
  
  virtual bool Open (char *Title);
  virtual void Close ();
  
  virtual void Print (csRect *area = NULL);
  
  virtual void SetRGB(int i, int r, int g, int b);
 
  virtual bool BeginDraw();
  virtual void FinishDraw();
  
  virtual bool SetMouseCursor (int iShape, TextureMM* iBitmap);
  virtual int GetPage ();
  virtual bool DoubleBuffer (bool Enable);
  virtual bool DoubleBuffer ();

  int m_nGraphicsReady;
  int m_nDepth;
  
protected:
//  HWND m_hWnd;
  HINSTANCE  m_hInstance;
  int m_nCmdShow;

  DECLARE_IUNKNOWN()
  DECLARE_INTERFACE_TABLE(csGraphics2DMac)

  DECLARE_COMPOSITE_INTERFACE(XMacGraphicsInfo)
};

#endif
