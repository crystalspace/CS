/*
    Copyright (C) 2000 by Andrew Zabolotny

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

#ifndef __MGL2D_H__
#define __MGL2D_H__

/* Convert our system definition flags to MGL compatible */
#if defined (OS_DOS)
#  define __MSDOS32__
#elif defined (OS_OS2)
#  define __OS2_32__
#elif defined (OS_WIN32)
#  define __WINDOWS32__
#elif defined (OS_BEOS)
#  define __BEOS__
#elif defined (OS_LINUX)
#  define __LINUX__
#elif defined (OS_FREEBSD)
#  define __FREEBSD__
#elif defined (OS_MAC)
#  define __MACOS__
#endif

#if defined (OS_UNIX)
#  define __UNIX__
#endif

#if defined (PROC_X86)
#  define __INTEL__
#elif defined (PROC_ALPHA)
#  define __ALPHA__
#elif defined (PROC_MIPS)
#  define __MIPS__
#elif defined (PROC_PPC)
#  define __PPC__
#elif defined (PROC_M68K)
#  define __MC68K__
#endif

#if defined (CS_BIG_ENDIAN)
#  define __BIG_ENDIAN__
#endif

#include <mgraph.h>

#include "csutil/scf.h"
#include "video/canvas/common/graph2d.h"
#include "isys/event.h"

class csGraphics2DMGL : public csGraphics2D, public iEventPlug
{
  /// MGL context
  MGLDC *dc;
  /// Hardware mouse cursor or software emulation?
  bool do_hwmouse;
  /// Do double (triple) buffering?
  bool do_doublebuff;
  /// The video mode ID
  int video_mode;
  /// Number of available video pages
  int numPages;
  /// The handle to back buffer context (if any)
  MGLDC *backdc;
  /// Current active video page (if supported)
  int videoPage;
  /// Last known joystick button mask
  size_t joybutt;
  /// Last known joystick positions
  int joyposx [2], joyposy [2];
  /// true if palette was changed
  bool paletteChanged;
  // The event outlet
  iEventOutlet *EventOutlet;

public:
  SCF_DECLARE_IBASE_EXT(csGraphics2D);

  /// Initialize the 2D graphics context
  csGraphics2DMGL (iBase *iParent);
  /// Destroy the 2D graphics context
  virtual ~csGraphics2DMGL ();

  /// Initialize the 2D plugin
  virtual bool Initialize (iSystem *pSystem);
  /// Open the graphics context
  virtual bool Open (const char *Title);
  /// Close the graphics context
  virtual void Close ();

  /// Begin drawing (lock context)
  virtual bool BeginDraw ();
  /// Finish drawing
  virtual void FinishDraw ();

  /// Swap graphics buffers
  virtual void Print (csRect *area = NULL);
  /// Set palette entry
  virtual void SetRGB (int i, int r, int g, int b);

  /// Get active videopage number (starting from zero)
  virtual int GetPage ()
  { return videoPage; }
  /// Enable or disable double buffering; return TRUE if supported
  virtual bool DoubleBuffer (bool Enable);
  /// Return current double buffering state
  virtual bool GetDoubleBufferState ()
  { return do_doublebuff; }

  /// Returns 'true' if the program is being run full-screen.
  virtual bool GetFullScreen ()
  { return true; }

  /// Set clipping rectangle
  virtual void SetClipRect (int nMinX, int nMinY, int nMaxX, int nMaxY);

  /// Clear backbuffer
  virtual void Clear (int color);
  /// Draw a line
//virtual void DrawLine (float x1, float y1, float x2, float y2, int color);
  /// Draw a box
  virtual void DrawBox (int x, int y, int w, int h, int color);
  /// Draw a pixel.
  virtual void DrawPixel (int x, int y, int color);

  /// Set mouse cursor position; return success status
  virtual bool SetMousePosition (int x, int y);

  /**
   * Set mouse cursor to one of predefined shape classes
   * (see csmcXXX enum above). If a specific mouse cursor shape
   * is not supported, return 'false'; otherwise return 'true'.
   * If system supports it and iBitmap != NULL, shape should be
   * set to the bitmap passed as second argument; otherwise cursor
   * should be set to its nearest system equivalent depending on
   * iShape argument.
   */
  virtual bool SetMouseCursor (csMouseCursorID iShape);

  /// Called on every frame by system driver
  virtual bool HandleEvent (iEvent &Event);

  //------------------------- iEventPlug interface ---------------------------//

  virtual unsigned GetPotentiallyConflictingEvents ()
  { return CSEVTYPE_Keyboard | CSEVTYPE_Mouse; }
  virtual unsigned QueryEventPriority (unsigned /*iType*/)
  { return 150; }

private:
  /// Allocate the back buffer: either in hardware or software
  void AllocateBackBuffer ();
  /// Deallocate the back buffer
  void DeallocateBackBuffer ();
  /// Translate an MGL key to Crystal Space
  int TranslateKey (int mglKey);
};

#endif // __MGL2D_H__
