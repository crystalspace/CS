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

#ifndef __GLIDEX2D_H__
#define __GLIDEX2D_H__

#include "csutil/scf.h"
#include "cs2d/common/graph2d.h"
#include "cssys/unix/iunix.h"
#include "cs2d/glide2common2d/iglide2d.h"

#include <glide.h>

#define XK_MISCELLANY 1
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/cursorfont.h>

#ifdef DO_SHM
#  include <X11/extensions/XShm.h>
#  include <sys/ipc.h>
#  include <sys/shm.h>
#endif /* DO_SHM */

/// XLIB version.
class csGraphics2DGlideX : public csGraphics2DGlideCommon
{
private:
  // The display context
  Display* dpy;
  int screen_num;
  int display_width, display_height;
  Window window;
  XImage* xim;
  GC gc;
  XVisualInfo *active_GLVisual;
  short GraphicsReady;
  bool bPalettized;
  bool bPaletteChanged;
  int glDrawMode;
  GrLfbInfo_t lfbInfo;
  bool m_DoGlideInWindow;

  // Window colormap
  Colormap cmap;

  // Use SHM or not?
  bool do_shm;
#ifdef DO_SHM
  XShmSegmentInfo shmi;
  XImage shm_image;
#endif

  // Hardware mouse cursor or software emulation?
  bool do_hwmouse;
  /// Mouse cursors (if hardware mouse cursors are used)  
  Cursor MouseCursor [int(csmcWait) + 1];
  /// Empty mouse cursor (consist of EmptyPixmap)
  Cursor EmptyMouseCursor;
  /// A empty pixmap
  Pixmap EmptyPixmap;

  /// Pointer to Unix-specific interface
  iUnixSystemDriver* UnixSystem;

public:
  csGraphics2DGlideX (iBase *iParent);
  virtual ~csGraphics2DGlideX ();

  virtual bool Initialize (iSystem *pSystem);
  virtual bool Open (const char *Title);
  virtual void Close ();
  
  virtual bool BeginDraw ();
  virtual void FinishDraw ();
  virtual void SetTMUPalette(int tmu);
  virtual void Print (csRect *area = NULL);
  virtual void SetRGB (int i, int r, int g, int b);

  /// Set mouse cursor shape
  virtual bool SetMouseCursor (csMouseCursorID iShape);

  virtual void DrawLine (int x1, int y1, int x2, int y2, int color);
  
  static void DrawPixelGlide (int x, int y, int color);
  static void WriteCharGlide (int x, int y, int fg, int bg, char c);
  static void DrawSpriteGlide (iTextureHandle *hTex, int sx, int sy, 
                        int sw, int sh, int tx, int ty, int tw, int th);
  static unsigned char* GetPixelAtGlide (int x, int y);          

protected:
  /// This routine is called once per event loop
  static void ProcessEvents (void *Param);
  
  /// This method is used for GlideInWindow...
  void FXgetImage();

  /// Is framebuffer locked?
  bool locked;
};

#endif // __GLIDEX2D_H__
