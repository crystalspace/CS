/*
    SDL 2d canvas for Crystal Space (header)
    Copyright (C) 2000 by George Yohng <yohng@drivex.dosware.8m.com>

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

#ifndef __CS_NULL2D_H__
#define __CS_NULL2D_H__

#include "csutil/scf.h"
#include "video/canvas/common/graph2d.h"

/// Null version.
class csGraphics2DNull : public csGraphics2D
{
public:
  csGraphics2DNull (iBase *iParent);
  virtual ~csGraphics2DNull ();

  virtual bool Initialize (iObjectRegistry *object_reg);
  virtual bool Open ();
  virtual void Close ();

  /// Necessary to access framebuffer
  virtual bool BeginDraw();
  virtual void FinishDraw();

  virtual void Print (csRect *area = NULL);
  virtual void SetRGB (int i, int r, int g, int b);

  virtual bool SetMousePosition (int , int );
  virtual bool SetMouseCursor (csMouseCursorID iShape);
};

#endif // __CS_SDL2D_H__
