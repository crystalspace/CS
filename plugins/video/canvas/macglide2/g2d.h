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
 
#include "cscom/com.h"
#include "cs2d/common/graph2d.h"
#include "cs2d/macglide2/xg2d.h"


#if defined(DISP_X11)
  #define XK_MISCELLANY 1
  #include <X11/Xlib.h>
  #include <X11/keysymdef.h>
#endif           

extern const CLSID CLSID_Glide2xGraphics2D;

///
class csGraphics2DGlide2xFactory : public IGraphics2DFactory 
{
public:
    DECLARE_IUNKNOWN()
    DECLARE_INTERFACE_TABLE(csGraphics2DGlide2xFactory)

    STDMETHOD(CreateInstance)(REFIID riid, ISystem* piSystem, void** ppv);
    STDMETHOD(LockServer)(COMBOOL bLock);
};

class csGraphics2DGlide2x : public csGraphics2D
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
  csGraphics2DGlide2x(ISystem* piSystem);
  virtual ~csGraphics2DGlide2x(void);
  
  virtual bool Open (const char *Title);
  virtual void Close ();
  
  virtual void Print (csRect *area = NULL);
  
  virtual void SetRGB(int i, int r, int g, int b);
 
  virtual bool BeginDraw(/*int Flag = (CSDRAW_2DGRAPHICS | CSDRAW_2DGRAPHICS_WRITE)*/);
  virtual void FinishDraw();

  virtual void SetTMUPalette(int tmu);
  
  int GraphicsReady;
  static int Depth;
  
protected:
  // print to the system's device
  void SysPrintf(int mode, const char* str, ...);

#if defined(OS_WIN32)
  HWND m_hWnd;
#endif
  bool bPalettized;
  bool bPaletteChanged;
  int glDrawMode;
  GrLfbInfo_t lfbInfo;
  bool locked;

  DECLARE_IUNKNOWN()
  DECLARE_INTERFACE_TABLE(csGraphics2DGlide2x)
  DECLARE_COMPOSITE_INTERFACE(XGlide2xGraphicsInfo)
};

#endif // G3D_GLIDE_H

