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

#ifndef __GLBE2D_H__
#define __GLBE2D_H__

#include "cscom/com.h"
#include "cs2d/common/graph2d.h"
#include "cssys/be/beitf.h"

#ifndef CRYST_WINDOW_H
#include "CrystGLWindow.h"
#endif

#include <GL/gl.h>

// The CLSID to create csGraphics2DGLX instances
extern const CLSID CLSID_GLBeGraphics2D;

///
class csGraphics2DGLBeFactory : public IGraphics2DFactory
{
public:
  DECLARE_IUNKNOWN ()
  DECLARE_INTERFACE_TABLE (csGraphics2DGLBeFactory)

  STDMETHOD (CreateInstance) (REFIID riid, ISystem* piSystem, void** ppv);
  STDMETHOD (LockServer) (BOOL bLock);
};

///

/// BeLIB version.
class csGraphics2DGLBe : public csGraphics2D
{
private:
  // The display context
  CrystGLView*	dpy;
  int 			display_width, display_height;
  CrystGLWindow	*window;
  BBitmap		*cryst_bitmap;

// Everything for simulated depth
  int depth;
  csPixelFormat real_pfmt;	// Contains the real pfmt is simulating stuff
  unsigned char* real_Memory;	// Real memory to the display

//  XImage* xim;
//  GC gc;
//  XVisualInfo *active_GLVisual;
//  GLContext active_GLContext;

  // Window colormap
//  Colormap cmap;

  // Hardware mouse cursor or software emulation?
  bool do_hwmouse;

  /// Pointer to system driver interface
  static ISystem* System;
  /// Pointer to DOS-specific interface
  static IBeLibSystemDriver* BeSystem;
  /// This function has the rights to access the static members
  friend void __printfGLBe (int msgtype, char *format, ...);

  /// The "real" mouse handler
  BeMouseHandler MouseHandler;
  /// The first parameter for "real" mouse handler
  void *MouseHandlerParm;
  /// The keyboard handler
  BeKeyboardHandler KeyboardHandler;
  /// The first parameter for keyboard handler
  void *KeyboardHandlerParm;
  /// The focus handler
  BeFocusHandler FocusHandler;
  /// The first parameter for focus handler
  void *FocusHandlerParm;

  /// This routine is called once per event loop
  static void ProcessEvents (void *Param);

public:
  csGraphics2DGLBe (ISystem* piSystem, bool bUses3D);
  virtual ~csGraphics2DGLBe ();

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
  static void DrawSpriteGL (ITextureHandle *hTex, int sx, int sy, int sw, int sh,
    int tx, int ty, int tw, int th);
  /**
   * Get address of video RAM at given x,y coordinates.
   * The OpenGL version of this function just returns NULL.
   */
  static unsigned char* GetPixelAtGL (int x, int y);

protected:
  DECLARE_IUNKNOWN ()
  DECLARE_INTERFACE_TABLE (csGraphics2DGLBe)
};

#endif // __BELIB2D_H__
