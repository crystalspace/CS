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

#include "cscom/com.h"
#include "cs2d/common/graph2d.h"
#include "cssys/djgpp/idjgpp.h"

extern const CLSID CLSID_DosRawGraphics2D;

/// IGraphics2DFactory interface implementation
class csGraphics2DFactoryDOSAlleg : public IGraphics2DFactory
{
public:
  DECLARE_IUNKNOWN ()
  DECLARE_INTERFACE_TABLE (csGraphics2DFactoryDOSAlleg)

  STDMETHOD (CreateInstance) (REFIID riid, ISystem* piSystem, void** ppv);
  STDMETHOD (LockServer) (COMBOOL bLock);
};

/// DOS Allegro 2D graphics driver COM class
class csGraphics2DDOSAlleg : public csGraphics2D
{
  /// Palette has been changed?
  bool PaletteChanged;
  /// Pointer to system driver interface
  static ISystem* System;
  /// Pointer to DOS-specific interface
  static IDosSystemDriver* DosSystem;

public:
  csGraphics2DDOSAlleg (ISystem* piSystem);
  virtual ~csGraphics2DDOSAlleg ();

  virtual void Initialize ();
  virtual bool Open(char* Title);
  virtual void Close(void);

  virtual void Print (csRect *area = NULL);

  virtual bool SetMousePosition (int x, int y);
  virtual bool SetMouseCursor (int iShape, ITextureHandle *hBitmap);

  virtual void SetRGB(int i, int r, int g, int b);

  virtual bool BeginDraw ();

private:
  void CsPrintf (int msgtype, char *format, ...);
  void FillEvents ();
  void printf_Enable (bool Enable);

protected:
  DECLARE_IUNKNOWN ()
  DECLARE_INTERFACE_TABLE (csGraphics2DDOSAlleg)
};

#endif // __ALLEGRO_H__
