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

#include <mgraph.h>

#include "csutil/scf.h"
#include "cs2d/common/graph2d.h"

class csGraphics2DMGL : public csGraphics2D
{
  /// MGL context
  MGLDC *dc;

  /// Hardware mouse cursor or software emulation?
  bool do_hwmouse;

  /// Use back buffer in system memory or in video memory
  bool do_hwbackbuf;

  /// This routine is called once per event loop
  static void ProcessEvents (void *Param);

public:
  DECLARE_IBASE;

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
  { return (bbtype == bbSecondVRAMPage); }

  /// Clear backbuffer
  virtual void Clear (int color);

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

private:
  /// The video mode ID
  int video_mode;
  /// Number of available video pages
  int numPages;
  /// The handle to back buffer context (if any)
  MGLDC *backdc;
  /// Back buffer type
  enum { bbSecondVRAMPage, bbOffscreenVRAM, bbSystemMemory } bbtype;
  /// Current active video page (if supported)
  int videoPage;
  /// Do we allow page flipping + double buffering?
  bool allowDB;
  /// Allocate the back buffer: either in hardware or software
  void AllocateBackBuffer ();
  /// Deallocate the back buffer
  void DeallocateBackBuffer ();
};

#endif // __MGL2D_H__
