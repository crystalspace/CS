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

#ifndef BELIBG2D_H
#define BELIBG2D_H

#include "cscom/com.h"
#include "cs2d/common/graph2d.h"
#include "cs2d/be/besysg2d.h"
#include "cs2d/be/CrystWindow.h"

// The CLSID to create csGraphics2DBELib instances
extern const CLSID CLSID_BeLibGraphics2D;

///
class csGraphics2DBeLibFactory : public IGraphics2DFactory
{
public:
  DECLARE_IUNKNOWN()
  DECLARE_INTERFACE_TABLE(csGraphics2DBeLibFactory)

  STDMETHOD(CreateInstance)(REFIID riid, ISystem* piSystem, void** ppv);
  STDMETHOD(LockServer)(COMBOOL bLock);
};

///

/// BELib version.
class csGraphics2DBeLib : public csGraphics2D
{
  friend class csGraphics3DSoftware;
  friend class CrystWindow;
private:
  CrystView*	dpy;
  int 			display_width, display_height;//actual screen dimensions, as opposed to window dimensions
  CrystWindow	*window;
//  BBitmap		*cryst_bitmap;	// this points to the BBitmap used by CrystWindow in non-direct framebuffer mode.
  								// this is the single buffer version.
  color_space	curr_color_space;
  
  // double buffer implementation
#define			NO_OF_BUFFERS 2
  BBitmap		*cryst_bitmap[NO_OF_BUFFERS];
  int			curr_page;
  
  // stuff to implement BDirectWindow  
protected:
  BLocker		*locker;
  bool			fDirty;
  bool			fConnected;
  bool			fConnectionDisabled;
  bool			fDrawingThreadSuspended;
		

// Everything for simulated depth
  int depth;
  csPixelFormat real_pfmt;	// Contains the real pfmt is simulating stuff
  unsigned char* real_Memory;	// Real memory to the display

public:
  csGraphics2DBeLib(ISystem* piSystem, bool bUses3D);
  virtual ~csGraphics2DBeLib(void);

  virtual bool Open (char *Title);
  virtual void Initialize ();
  virtual void Close ();

  virtual bool BeginDraw () /*{ return (Memory != NULL); }*/;
  virtual void FinishDraw ();

  virtual void Print (csRect *area = NULL);
  virtual int  GetPage ();
  virtual void SetRGB (int i, int r, int g, int b);
  virtual void ApplyDepthInfo(color_space this_color_space);

protected:
  DECLARE_IUNKNOWN()
  DECLARE_INTERFACE_TABLE(csGraphics2DBeLib)

  DECLARE_COMPOSITE_INTERFACE(XBeLibGraphicsInfo)
};

#endif //BELibG2D_H

