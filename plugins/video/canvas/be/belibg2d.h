/*
    Copyright (C) 1998,1999 by Jorrit Tyberghein
    Written by Xavier Planet.
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

#ifndef __CS_BELIBG2D_H__
#define __CS_BELIBG2D_H__

#include "GraphicsDefs.h"
#include "Rect.h"
#include "csutil/scf.h"
#include "cs2d/common/graph2d.h"
#include "cssys/be/icsbe.h"

class CrystView;
class CrystWindow;

/// Be 2D Graphics Driver
class csGraphics2DBeLib : public csGraphics2D
{
  typedef csGraphics2D superclass;
protected:
  iBeLibSystemDriver* be_system;
  CrystView* view;
  CrystWindow* window;
  BBitmap* bitmap;
  color_space curr_color_space;
  BRect screen_frame;
  
public:
  DECLARE_IBASE;

  csGraphics2DBeLib (iBase*);
  virtual ~csGraphics2DBeLib ();

  virtual bool Initialize (iSystem*);
  virtual bool Open (const char* title);
  virtual void Close ();
  virtual void Print (csRect* area = NULL);
  virtual bool SetMouseCursor (csMouseCursorID shape);
  virtual void ApplyDepthInfo (color_space);
};

#endif // __CS_BELIBG2D_H__
