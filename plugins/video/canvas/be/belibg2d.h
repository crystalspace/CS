/*
    Copyright (C) 1998,1999 by Jorrit Tyberghein
    Written by Xavier Planet.
    Overhauled and re-engineered by Eric Sunshine <sunshine@sunshineco.com>

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

#ifndef BELIBG2D_H
#define BELIBG2D_H

#include "GraphicsDefs.h"
#include "Rect.h"
#include "cscom/com.h"
#include "cs2d/common/graph2d.h"
#include "cs2d/be/besysg2d.h"
#include "cssys/be/beitf.h"
class CrystView;
class CrystWindow;

extern const CLSID CLSID_BeLibGraphics2D;

class csGraphics2DBeLibFactory : public IGraphics2DFactory
{
public:
  DECLARE_IUNKNOWN()
  DECLARE_INTERFACE_TABLE(csGraphics2DBeLibFactory)
  STDMETHOD(CreateInstance)(REFIID riid, ISystem*, void** ppv);
  STDMETHOD(LockServer)(COMBOOL bLock);
};

/// Be 2D Graphics Driver
class csGraphics2DBeLib : public csGraphics2D
{
protected:
  ISystem* cs_system;
  IBeLibSystemDriver* be_system;
  CrystView* dpy;
  CrystWindow* window;
  color_space curr_color_space;
  BRect screen_frame;
  
  // double buffer implementation
#define BUFFER_COUNT 2
  BBitmap* cryst_bitmap[BUFFER_COUNT];
  int curr_page;
  bool double_buffered;
  
  // stuff to implement BDirectWindow  
#if 0
protected:
  BLocker* locker;
  bool fDirty;
  bool fConnected;
  bool fConnectionDisabled;
  bool fDrawingThreadSuspended;
#endif		

public:
  csGraphics2DBeLib(ISystem*);
  virtual ~csGraphics2DBeLib();

  virtual void Initialize ();
  virtual bool Open (const char* title);
  virtual void Close ();

  virtual bool BeginDraw ();
  virtual void Print (csRect* area = NULL);
  virtual void FinishDraw ();

  virtual bool DoubleBuffer (bool Enable);
  virtual bool DoubleBuffer ();
  virtual int  GetPage ();
  virtual bool SetMouseCursor (int shape, ITextureHandle*);
  virtual void ApplyDepthInfo(color_space);

protected:
  DECLARE_IUNKNOWN()
  DECLARE_INTERFACE_TABLE(csGraphics2DBeLib)
  DECLARE_COMPOSITE_INTERFACE(XBeLibGraphicsInfo)
};

#endif //BELibG2D_H

