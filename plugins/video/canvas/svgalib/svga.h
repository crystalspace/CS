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

#include "csutil/scf.h"
#include "cs2d/common/graph2d.h"
#include "cssys/unix/iunix.h"

/// SVGALIB version.
class csGraphics2DSVGALib : public csGraphics2D
{
  /// Physical graphics context
  GraphicsContext physicalscreen;
  /// Pointer to DOS-specific interface
  iUnixSystemDriver* UnixSystem;

  /// This routine is called once per event loop
  static void ProcessEvents (void *Param);

  /// Keep track of pressed keys
  bool keydown [128];
  /// Keep track of pressed mouse buttons
  bool mouse_button [3];
  /// Keep track of mouse position
  int mouse_x, mouse_y;
  
public:
  DECLARE_IBASE;

  csGraphics2DSVGALib (iBase *iParent);
  virtual ~csGraphics2DSVGALib ();

  virtual bool Open (const char *Title);
  virtual void Close ();
  virtual bool Initialize (iSystem *pSystem);

  virtual bool BeginDraw () { return (Memory != NULL); }
  virtual bool DoubleBuffer (bool Enable) { return true; }
  virtual bool DoubleBuffer () { return true; }

  virtual void Print (csRect *area = NULL);
  virtual void SetRGB (int i, int r, int g, int b);
};

#endif // __SVGA_H__
