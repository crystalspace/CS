/*
    OS/2 support for Crystal Space 3D library
    Copyright (C) 1998 by Jorrit Tyberghein
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

#ifndef __CSDIVE_H__
#define __CSDIVE_H__

#include "cscom/com.h"
#include "cs2d/common/graph2d.h"
#include "cssys/common/system.h"
#include "cssys/os2/icsos2.h"

// avoid including os2.h
class diveWindow;
typedef ULong HWND;
typedef ULong ULONG;

extern const CLSID CLSID_OS2DiveGraphics2D;

/// IGraphics2DFactory interface implementation
class csGraphics2DFactoryOS2DIVE : public IGraphics2DFactory
{
public:
  DECLARE_IUNKNOWN ()
  DECLARE_INTERFACE_TABLE (csGraphics2DFactoryOS2DIVE)

  STDMETHOD (CreateInstance) (REFIID riid, ISystem* piSystem, void** ppv);
  STDMETHOD (LockServer) (COMBOOL bLock);
};

/**
 * This is the SysGraphics2D class for OS/2. It implements drawing
 * on a DIVE context - either off-screen or memory, depending
 * on system parameters/available resources. Most system-dependent
 * code is in libDIVE*, the SysGraphics2D class contains only bindings.
 * Because of DIVE flexibility, we get also all the associated goodies -
 * hardware acceleration if there is one, hardware rescaling if there is one,
 * support for lots of pixel formats (although CS currently uses little).
 * Full-screen currently is implemented by just rescaling the window so that
 * client portion of window occupies entire screen. This works pretty fast
 * on my Matrox Mystique (in 1152x864x64K), but is very slow on sluggish
 * cards (such as Cirrus Logic 543X, 546X (last one becuz of lame drivers)).
 */
class csGraphics2DOS2DIVE : public csGraphics2D
{
  /// Pixel format (one of FOURCC_XXX)
  UInt PixelFormat;
  /// The width for which LineAddress has been computed last time
  UInt LineAddressFrameW;
  /// The DIVE window object
  diveWindow *dW;
  /// The handle of window where DIVE context is located
  HWND WinHandle;
  /// The palette
  ULONG DivePalette[256];
  /// true if palette has to be updated
  bool UpdatePalette;
  /// Do double buffering?
  bool dblbuff;
  /// Use native mouse cursor, if possible?
  bool HardwareCursor;

  /// Window position in percents
  int WindowX, WindowY;
  /// Window width and height
  int WindowWidth, WindowHeight;
  /// Pointer to system driver interface
  ISystem* System;
  /// Pointer to the OS/2 system driver
  IOS2SystemDriver* OS2System;

public:
  csGraphics2DOS2DIVE (ISystem* piSystem);
  virtual ~csGraphics2DOS2DIVE ();

  virtual void Initialize ();
  virtual bool Open (const char *Title);
  virtual void Close ();

  virtual void Print (csRect *area = NULL);
  virtual int GetPage ();
  virtual bool DoubleBuffer (bool Enable);
  virtual bool DoubleBuffer ();

  virtual void Clear (int color);
  virtual void SetRGB (int i, int r, int g, int b);

  /* These procedures locks/unlocks DIVE buffer */
  virtual bool BeginDraw ();
  virtual void FinishDraw ();

  virtual bool SetMousePosition (int x, int y);
  virtual bool SetMouseCursor (int iShape, ITextureHandle *hBitmap);

protected:
  DECLARE_IUNKNOWN ()
  DECLARE_INTERFACE_TABLE (csGraphics2DOS2DIVE)

private:
  static void KeyboardHandlerStub (void *Self, unsigned char ScanCode, int Down,
    unsigned char RepeatCount, int ShiftFlags);
  static void MouseHandlerStub (void *Self, int Button, int Down, int x, int y,
    int ShiftFlags);
  static void FocusHandlerStub (void *Self, bool Enable);
  static void TerminateHandlerStub (void *Self);
};

#endif // __CSDIVE_H__
