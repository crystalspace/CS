/*
    OS/2 support for Crystal Space 3D library
    Copyright (C) 1998 by Andrew Zabolotny <bit@eltech.ru>

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

#include "video/canvas/common/graph2d.h"
#include "isys/event.h"

// avoid including os2.h
class diveWindow;
typedef ULong HWND;
typedef ULong ULONG;

/**
 * This is the SysGraphics2D class for OS/2. It implements drawing
 * on a DIVE context - either off-screen or video memory, depending
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
class csGraphics2DOS2DIVE : public csGraphics2D, public iEventPlug
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
  /// Active video page
  int ActivePage;

  /// Window position in percents
  int WindowX, WindowY;
  /// Window width and height
  int WindowWidth, WindowHeight;
  // The event outlet
  iEventOutlet *EventOutlet;

public:
  SCF_DECLARE_IBASE_EXT(csGraphics2D);

  csGraphics2DOS2DIVE (iBase *iParent);
  virtual ~csGraphics2DOS2DIVE ();

  virtual bool Initialize (iSystem* pSystem);
  virtual bool Open (const char *Title);
  virtual void Close ();

  virtual void Print (csRect *area = NULL);
  virtual int GetPage ();
  virtual bool DoubleBuffer (bool Enable);
  virtual bool GetDoubleBufferState ();

  virtual void Clear (int color);
  virtual void SetRGB (int i, int r, int g, int b);

  /* These procedures locks/unlocks DIVE buffer */
  virtual bool BeginDraw ();
  virtual void FinishDraw ();

  virtual bool SetMousePosition (int x, int y);
  virtual bool SetMouseCursor (csMouseCursorID iShape);

  virtual bool HandleEvent (iEvent &Event);

  //------------------------ iEventPlug interface ---------------------------//

  virtual unsigned GetPotentiallyConflictingEvents ()
  { return CSEVTYPE_Keyboard | CSEVTYPE_Mouse; }
  virtual unsigned QueryEventPriority (unsigned /*iType*/)
  { return 150; }

private:
  static void KeyboardHandlerStub (void *Self, unsigned char ScanCode,
    unsigned char CharCode, bool Down, unsigned char RepeatCount,
    int ShiftFlags);
  static void MouseHandlerStub (void *Self, int Button, bool Down,
    int x, int y, int ShiftFlags);
  static void FocusHandlerStub (void *Self, bool Enable);
  static void TerminateHandlerStub (void *Self);
};

#endif // __CSDIVE_H__
