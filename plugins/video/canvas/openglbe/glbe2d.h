/*
    Copyright (C) 1999-2001 by Eric Sunshine <sunshine@sunshineco.com>

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

#ifndef __CS_GLBE2D_H__
#define __CS_GLBE2D_H__

#include <GL/gl.h>
#include <GraphicsDefs.h>
#include <Rect.h>
#include "csutil/scf.h"
#include "video/canvas/common/graph2d.h"
#include "video/canvas/openglcommon/glcommon2d.h"
class CrystGLView;
class CrystGLWindow;

class csGraphics2DGLBe : public csGraphics2DGLCommon
{
  typedef csGraphics2DGLCommon superclass;
protected:
  CrystGLView* view;
  CrystGLWindow* window;
  color_space curr_color_space;
  BRect screen_frame;
  
public:
  csGraphics2DGLBe(iBase*);
  virtual ~csGraphics2DGLBe();
  
  virtual bool Initialize(iSystem*);
  virtual bool Open();
  virtual void Close();
  virtual bool BeginDraw();
  virtual void Print(csRect* area = NULL);
  virtual void FinishDraw();
  virtual bool SetMouseCursor(csMouseCursorID);
  virtual void ApplyDepthInfo(color_space);
};

#endif // __CS_GLBE2D_H__
