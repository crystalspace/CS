/*
    Copyright (C) 1998,1999 by Jorrit Tyberghein
    Overhauled and re-engineered by Eric Sunshine <sunshine@sunshineco.com>

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

#include <GL/gl.h>
#include "GraphicsDefs.h"
#include "Rect.h"
#include "csutil/scf.h"
#include "cs2d/common/graph2d.h"
#include "cssys/be/beitf.h"
#include "cs2d/openglcommon/glcommon2d.h"

class CrystGLView;
class CrystGLWindow;

/// Be 2D OpenGL Driver
class csGraphics2DGLBe : public csGraphics2DGLCommon
{
  friend CrystGLWindow;	// FIXME: Currently needs access to 'dpy'.
protected:
  iSystem* cs_system;
  iBeLibSystemDriver* be_system;
  CrystGLView* dpy;
  CrystGLWindow* window;
  color_space curr_color_space;
  BRect screen_frame;
  
public:
  csGraphics2DGLBe (iSystem*);
  virtual ~csGraphics2DGLBe();
  
  virtual bool Initialize (iSystem *pSystem);
  virtual bool Open (const char* title);
  virtual void Close ();
  
  virtual bool BeginDraw ();
  virtual void Print (csRect* area = NULL);
  virtual void FinishDraw ();

  virtual void ApplyDepthInfo(color_space this_color_space);
};

#endif // __BELIB2D_H__
