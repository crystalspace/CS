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

#ifndef __GLX2D_H__
#define __GLX2D_H__

#include "cscom/com.h"
#include "cs2d/common/graph2d.h"
#include "cssys/unix/iunix.h"

#include <GL/glx.h>

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

interface ITextureHandle;

// The CLSID to create csGraphics2DGLX instances
extern const CLSID CLSID_GLXGraphics2D;

///
class csGraphics2DGLXFactory : public IGraphics2DFactory
{
public:
  DECLARE_IUNKNOWN ()
  DECLARE_INTERFACE_TABLE (csGraphics2DGLXFactory)

  STDMETHOD (CreateInstance) (REFIID riid, ISystem* piSystem, void** ppv);
  STDMETHOD (LockServer) (BOOL bLock);
};

///

/// XLIB version.
class csGraphics2DGLX : public csGraphics2D
{
  // The display context
  Display* dpy;
  int screen_num;
  int display_width, display_height;
  Window window;
  XImage* xim;
  GC gc;
  XVisualInfo *active_GLVisual;
  GLXContext active_GLContext;

  // Window colormap
  Colormap cmap;

#ifdef DO_SHM
  // Use SHM or not?
  bool do_shm;
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

  /// Pointer to system driver interface
  ISystem* System;
  /// Pointer to DOS-specific interface
  IUnixSystemDriver* UnixSystem;

public:
  csGraphics2DGLX (ISystem* piSystem);
  virtual ~csGraphics2DGLX ();

  virtual void Initialize ();
  virtual bool Open (char *Title);
  virtual void Close ();

  virtual bool BeginDraw () { return (Memory != NULL); }

  virtual void Print (csRect *area = NULL);
  virtual void SetRGB (int i, int r, int g, int b);

  /// Draw a line
  virtual void DrawLine (int x1, int y1, int x2, int y2, int color);
  /// Draw a horizontal line
  virtual void DrawHorizLine (int x1, int x2, int y, int color);
  /// Draw a pixel
  static void DrawPixelGL (int x, int y, int color);
  /// Write a single character
  static void WriteCharGL (int x, int y, int fg, int bg, char c);
  /// Draw a 2D sprite
  static void DrawSpriteGL (ITextureHandle *hTex, int sx, int sy,
    int sw, int sh, int tx, int ty, int tw, int th);
  /**
   * Get address of video RAM at given x,y coordinates.
   * The OpenGL version of this function just returns NULL.
   */
  static unsigned char* GetPixelAtGL (int x, int y);

  /// Set mouse position.
  virtual bool SetMousePosition (int x, int y);

  /// Set mouse cursor shape
  virtual bool SetMouseCursor (int iShape, ITextureHandle *iBitmap);

protected:
  /// This function is functionally equivalent to csSystemDriver::CsPrintf
  void CsPrintf (int msgtype, char *format, ...);

  /// This routine is called once per event loop
  static void ProcessEvents (void *Param);

  DECLARE_IUNKNOWN ()
  DECLARE_INTERFACE_TABLE (csGraphics2DGLX)
};

#endif // __XLIB2D_H__
