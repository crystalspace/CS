/*
    DOS support for Crystal Space 3D library
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by David N. Arnold <derek_arnold@fuse.net>
    Written by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __RAW_H__
#define __RAW_H__

#include "csutil/scf.h"
#include "video/canvas/common/graph2d.h"

/// Raw DOS SVGA 2D graphics driver
class csGraphics2DDOSRAW : public csGraphics2D
{
  /// Palette has been changed?
  bool PaletteChanged;

public:
  csGraphics2DDOSRAW (iBase *iParent);
  virtual ~csGraphics2DDOSRAW ();

  virtual bool Initialize (iSystem *pSystem);
  virtual bool Open (const char* Title);
  virtual void Close (void);

  virtual void Print (csRect *area = NULL);

  virtual bool SetMousePosition (int x, int y);
  virtual bool SetMouseCursor (csMouseCursorID iShape);

  virtual void SetRGB(int i, int r, int g, int b);

  virtual bool BeginDraw ();
#if !USE_ALLEGRO
  virtual void FinishDraw ();
  virtual void Clear (int color);
  virtual int GetPage ();
  virtual bool GetDoubleBufferState ();
  virtual bool DoubleBuffer (bool Enable);
#endif // USE_ALLEGRO

private:
  void FillEvents ();
  void printf_Enable (bool Enable);
};

#endif // __RAW_H__
