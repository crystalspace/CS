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

#ifndef __GLIDEBE2D_H__
#define __GLIDEBE2D_H__

#include "csutil/scf.h"
#include "cs2d/common/graph2d.h"
#include "cssys/be/csbe.h"
#include "cs2d/glide2common/glide2common2d.h"

#include <glide.h>

#ifndef CRYST_GLIDE_WINDOW_H
#include "cs2d/beglide2/CrystGlideWindow.h"
#endif
#include "cs2d/beglide2/xg2d.h"

/// BeOS version.
class csGraphics2DBeGlide : public csGraphics2DGlideCommon
{
friend class CrystGlideWindow;

private:
  // The display context
  CrystGlideView *dpy;
  int screen_num;
  int display_width, display_height;
  CrystGlideWindow		*window;
  
  color_space			curr_color_space;

  // buffer implementation (just temporary)
  BBitmap				*cryst_bitmap;
  unsigned char			  *BeMemory;
  
  // double buffer implementation
  int			curr_page;// just temporary
//  XImage* xim;
//  GC gc;
//  XVisualInfo *active_GLVisual;
  short GraphicsReady;
  bool bPalettized;
//  bool bPaletteChanged;// moved to common class
  int glDrawMode;
  GrLfbInfo_t lfbInfo;
//  bool locked;	 //moved to common class
//  bool m_DoGlideInWindow; //moved to common class

  // Window colormap
//  Colormap cmap;//dh: removed 230699
/*
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
*/

  /// Pointer to DOS-specific interface
  iBeLibSystemDriver* BeSystem;

public:
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
  
public:
  DECLARE_IBASE;

  csGraphics2DBeGlide (iBase *iParent);
  virtual ~csGraphics2DBeGlide ();

  virtual bool Initialize (iSystem *pSystem);
  virtual bool Open (const char *Title);
  virtual void Close ();
  
  virtual bool BeginDraw ();
  virtual void FinishDraw ();
//  virtual void SetTMUPalette(int tmu);// dh: moved to glide2common2d.h
  virtual void Print (csRect *area = NULL);
//  virtual void SetRGB (int i, int r, int g, int b);// dh: moved to glide2common2d.h
/*
  /// Set mouse cursor shape
  virtual bool SetMouseCursor (csMouseCursorID iShape);

  virtual void DrawLine (int x1, int y1, int x2, int y2, int color);
  
  void DrawPixelGlide (int x, int y, int color);
  static void WriteCharGlide (int x, int y, int fg, int bg, char c);
  static void DrawSpriteGlide (iTextureHandle *hTex, int sx, int sy, 
                        int sw, int sh, int tx, int ty, int tw, int th);
  static unsigned char* GetPixelAtGlide (int x, int y);          

protected:
*/
  /// This routine is called once per event loop
  static void ProcessEvents (void *Param);
  
  /// This method is used for GlideInWindow...
  void FXgetImage();

  void ApplyDepthInfo(color_space this_color_space);
};

#endif // __GLIDEBE2D_H__
