/*
    Copyright (C) 1998 by Jorrit Tyberghein and Dan Ogles
  
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

// g2d_glide.h
// Graphics2DGlide Class Declaration
// Written by xtrochu and Nathaniel

#ifndef G2D_GLIDE2_H
#define G2D_GLIDE2_H

#include <glide.h>
 
#include "csutil/scf.h"
#include "cs2d/common/graph2d.h"
#include "cs2d/winglide2/xg2d.h"
#include "cs2d/glide2common2d/iglide2d.h"

#if defined(DISP_X11)
  #define XK_MISCELLANY 1
  #include <X11/Xlib.h>
  #include <X11/keysymdef.h>
#endif           

class csGraphics2DGlide2x : public csGraphics2DGlideCommon
{
  friend class csGraphics3DGlide;
  friend class csGraphics2DGlide2x;
  
#if defined(DISP_X11)
private:
  Display* dpy;
  int screen_num;
  int display_width, display_height;
  Window window;
  GC gc;                 
#endif
public:
  csGraphics2DGlide2x(iBase *iParent);
  virtual ~csGraphics2DGlide2x(void);
  
  virtual bool Open (const char *Title);
  virtual void Close ();

  bool Initialize (iSystem *pSystem);
  
  virtual void Print (csRect *area = NULL);
  
  virtual void SetRGB(int i, int r, int g, int b);
 
  virtual bool BeginDraw(/*int Flag = (CSDRAW_2DGRAPHICS | CSDRAW_2DGRAPHICS_WRITE)*/);
  virtual void FinishDraw();

  virtual void SetTMUPalette(int tmu);
  
  int GraphicsReady;
  static int Depth;

#if defined(OS_WIN32)
  ///
  virtual HWND GethWnd ();
#endif
#if defined (OS_LINUX)
  virtual Display *GetDisplay ();
#endif    
  
protected:
#if defined(OS_WIN32)
  HWND m_hWnd;
#endif
  bool bPalettized;
  bool bPaletteChanged;
  int glDrawMode;
  GrLfbInfo_t lfbInfo;
  bool locked;
};

#endif // G3D_GLIDE_H

