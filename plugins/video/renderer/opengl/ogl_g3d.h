/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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

#ifndef __CS_GRAPH3D_OPENGL_H__
#define __CS_GRAPH3D_OPENGL_H__

// GRAPH3D.H
// csGraphics3DOpenGL OpenGL rasterizer class.

// Concieved by Jorrit Tyberghein and Gary Clark
// Expanded by Dan Ogles
// Further expanded by Gary Haussmann

#include "ogl_g3dcom.h"
struct iGraphics3D;

class csGraphics3DOpenGL : public csGraphics3DOGLCommon
{
public:
  csGraphics3DOpenGL (iBase*);
  virtual ~csGraphics3DOpenGL ();
  virtual bool Initialize (iObjectRegistry*);
  virtual bool Open();

};

#endif // __CS_GRAPH3D_OPENGL_H__
