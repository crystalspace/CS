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
#include "cs2d/openglcommon/glcommon2d.h"

#include <GL/gl.h>

#ifndef CRYST_WINDOW_H
#include "CrystGLWindow.h"
#endif

interface ITextureHandle;
class OpenGLTextureCache;

// The CLSID to create csGraphics2DGLX instances
extern const CLSID CLSID_GLBeGraphics2D;

///
class csGraphics2DGLBeFactory : public IGraphics2DFactory
{
public:
  DECLARE_IUNKNOWN ()
  DECLARE_INTERFACE_TABLE (csGraphics2DGLBeFactory)

  STDMETHOD (CreateInstance) (REFIID riid, ISystem* piSystem, void** ppv);
  STDMETHOD (LockServer) (COMBOOL bLock);
};

///

/// BeLIB version.
class csGraphics2DGLBe : public csGraphics2DGLCommon
{
  friend class csGraphics3DOpenGL;	// dh: is this necessary?
  friend class CrystGLWindow;
private:
  // The display context
  CrystGLView*	dpy;
  int 			display_width, display_height;
  CrystGLWindow	*window;
  BBitmap		*cryst_bitmap;	// dh: is this necessary?
  color_space	curr_color_space;
  
  int			curr_page;

public:
  // Hardware mouse cursor or software emulation?
  bool do_hwmouse;

  /// Pointer to DOS-specific interface
  IBeLibSystemDriver* BeSystem;

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

//  static csGraphics2DOpenGLFontServer *LocalFontServer;// moved into glcommon2d.h
  
public:
  csGraphics2DGLBe (ISystem* piSystem/*, bool bUses3D*/);// dh: removed bool as glx version doesn't have it.
  virtual ~csGraphics2DGLBe ();
  
  virtual void Initialize();//dh: declared in glcommon2d.h

  virtual bool Open (char *Title);//dh: declared in glcommon2d.h
  virtual void Close ();//dh: declared in glcommon2d.h
  
  virtual int  GetPage ();
  virtual bool DoubleBuffer (bool Enable);

  virtual bool BeginDraw () /*{ return (Memory != NULL); }*/;
  virtual void FinishDraw ();

  virtual void Print (csRect *area = NULL);

  virtual void ApplyDepthInfo(color_space this_color_space);

protected:
  DECLARE_IUNKNOWN ()
  DECLARE_INTERFACE_TABLE (csGraphics2DGLBe)
};

#endif // __BELIB2D_H__
