/*
    DOS Allegro support for Crystal Space 3D library
    Copyright (C) 1999 by Dan Bogdanov <dan@pshg.edu.ee>

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

#ifndef __ALLEGRO_H__
#define __ALLEGRO_H__

#include "csutil/scf.h"
#include "cs2d/common/graph2d.h"
#include "cssys/djgpp/idjgpp.h"

/// DOS Allegro 2D graphics driver
class csGraphics2DDOSAlleg : public csGraphics2D
{
  /// Palette has been changed?
  bool PaletteChanged;
  /// Pointer to DOS-specific interface
  iDosSystemDriver* DosSystem;

public:
  DECLARE_IBASE;

  csGraphics2DDOSAlleg (iBase *iParent);
  virtual ~csGraphics2DDOSAlleg ();

  virtual bool Initialize (iSystem *pSystem);
  virtual bool Open(char* Title);
  virtual void Close(void);

  virtual void Print (csRect *area = NULL);

  virtual bool SetMousePosition (int x, int y);
  virtual bool SetMouseCursor (csMouseCursorID iShape, iTextureHandle *hBitmap);

  virtual void SetRGB(int i, int r, int g, int b);

  virtual bool BeginDraw ();

private:
  void FillEvents ();
  void printf_Enable (bool Enable);
};

#endif // __ALLEGRO_H__
