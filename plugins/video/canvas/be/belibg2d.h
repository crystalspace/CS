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
#include "csutil/scf.h"
#include "cs2d/common/graph2d.h"
#include "cs2d/be/besysg2d.h"
#include "cssys/be/beitf.h"

class CrystView;
class CrystWindow;

/// Be 2D Graphics Driver
class csGraphics2DBeLib : public csGraphics2D
{
protected:
  iSystem* cs_system;
  iBeLibSystemDriver* be_system;
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
  DECLARE_IBASE;

  csGraphics2DBeLib(iSystem*);
  virtual ~csGraphics2DBeLib();

  virtual bool Initialize (iSystem *pSystem);
  virtual bool Open (const char* title);
  virtual void Close ();

  virtual bool BeginDraw ();
  virtual void Print (csRect* area = NULL);
  virtual void FinishDraw ();

  virtual bool DoubleBuffer (bool Enable);
  virtual bool DoubleBuffer ();
  virtual int  GetPage ();
  virtual bool SetMouseCursor (csMouseCursorID shape, iTextureHandle*);
  virtual void ApplyDepthInfo(color_space);

  bool DirectConnect (direct_buffer_info *info);
};

#endif //BELibG2D_H
