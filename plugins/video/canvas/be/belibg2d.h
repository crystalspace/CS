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

#ifndef __CS_BELIBG2D_H__
#define __CS_BELIBG2D_H__

#include <GraphicsDefs.h>
#include <Rect.h>
#include "csutil/scf.h"
#include "video/canvas/common/graph2d.h"

class CrystView;
class CrystWindow;

/// Be 2D Graphics Driver
class csGraphics2DBeLib : public csGraphics2D
{
  typedef csGraphics2D superclass;
protected:
  CrystView* view;
  CrystWindow* window;
  BBitmap* bitmap;
  color_space curr_color_space;
  BRect screen_frame;
  
public:
  csGraphics2DBeLib(iBase*);
  virtual ~csGraphics2DBeLib();

  virtual bool Initialize(iObjectRegistry*);
  virtual bool Open();
  virtual void Close();
  virtual void Print(csRect* area = NULL);
  virtual bool SetMouseCursor(csMouseCursorID);
  virtual void ApplyDepthInfo(color_space);
};

#endif // __CS_BELIBG2D_H__
