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
  STDMETHOD(LockServer)(BOOL bLock);
};

///

/// BELib version.
class csGraphics2DBeLib : public csGraphics2D
{
  friend class csGraphics3DSoftware;
private:
  CrystView*	dpy;
  int 		display_width, display_height;
  CrystWindow	*window;
  BBitmap	*cryst_bitmap;

// Everything for simulated depth
  int depth;
  csPixelFormat real_pfmt;	// Contains the real pfmt is simulating stuff
  unsigned char* real_Memory;	// Real memory to the display

public:
  csGraphics2DBeLib(ISystem* piSystem, bool bUses3D);
  virtual ~csGraphics2DBeLib(void);

  virtual bool Open (char *Title);
  virtual bool Initialize ();
  virtual void Close ();

  virtual bool BeginDraw () { return (Memory != NULL); }

  virtual void Print (csRect *area = NULL);
  virtual void SetRGB (int i, int r, int g, int b);

protected:
  DECLARE_IUNKNOWN()
  DECLARE_INTERFACE_TABLE(csGraphics2DBeLib)

  DECLARE_COMPOSITE_INTERFACE(XBeLibGraphicsInfo)
};

#endif //BELibG2D_H

