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

#ifndef __G3XG2D_H__
#define __G3XG2D_H__

#include <glide.h>
 
#include "csutil/scf.h"
#include "cs2d/common/graph2d.h"
// whats that ?
//#include "cs2d/winglide3/xg2d.h"
#include "cs2d/glide2common2d/iglide2d.h"
#include "cs2d/glide2common2d/glide2common2d.h"

class csGraphics2DGlide3x : public csGraphics2DGlideCommon
{
public:
  csGraphics2DGlide3x(iBase *iParent);
  virtual ~csGraphics2DGlide3x(void);
  
  virtual bool Open (const char *Title);
  virtual void Close ();
  
  bool Initialize (iSystem *pSystem);

  virtual long GethWnd(unsigned long *hwnd);
  
protected:
  HWND m_hWnd;

};

#endif // G3D_GLIDE_H
