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

#ifndef __SVGA_H__
#define __SVGA_H__

#include <vga.h>
#include <vgagl.h>
#include <vgakeyboard.h>
#include <vgamouse.h>

#include "cscom/com.h"
#include "cs2d/common/graph2d.h"
#include "cssys/unix/iunix.h"

// The CLSID to create csGraphics2DSVGA instances
extern const CLSID CLSID_SVGALibGraphics2D;

///
class csGraphics2DSVGALibFactory : public IGraphics2DFactory
{
public:
  DECLARE_IUNKNOWN ()
  DECLARE_INTERFACE_TABLE (csGraphics2DSVGALibFactory)

  STDMETHOD (CreateInstance) (REFIID riid, ISystem* piSystem, void** ppv);
  STDMETHOD (LockServer) (COMBOOL bLock);
};

/// SVGALIB version.
class csGraphics2DSVGALib : public csGraphics2D
{
  /// Physical graphics context
  GraphicsContext physicalscreen;
  /// Pointer to system driver interface
  ISystem* System;
  /// Pointer to DOS-specific interface
  IUnixSystemDriver* UnixSystem;

  /// This routine is called once per event loop
  static void ProcessEvents (void *Param);

  /// Keep track of pressed keys
  bool keydown [128];
  /// Keep track of pressed mouse buttons
  bool mouse_button [3];
  /// Keep track of mouse position
  int mouse_x, mouse_y;
  
public:
  csGraphics2DSVGALib (ISystem* piSystem);
  virtual ~csGraphics2DSVGALib ();

  virtual bool Open (char *Title);
  virtual void Close ();
  virtual void Initialize ();

  virtual bool BeginDraw () { return (Memory != NULL); }
  virtual bool DoubleBuffer (bool Enable) { return true; }
  virtual bool DoubleBuffer () { return true; }

  virtual void Print (csRect *area = NULL);
  virtual void SetRGB (int i, int r, int g, int b);

protected:
  /// This function is functionally equivalent to csSystemDriver::CsPrintf
  void CsPrintf (int msgtype, char *format, ...);

  DECLARE_IUNKNOWN ()
  DECLARE_INTERFACE_TABLE (csGraphics2DSVGALib)
};

#endif // __SVGA_H__
