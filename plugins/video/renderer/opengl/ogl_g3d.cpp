/*
 * Copyright (C) 1998 by Jorrit Tyberghein
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "cssysdef.h"

#include <stdarg.h>
#include <stdio.h>

#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif

#ifndef GL_VERSION_1_1
#error OpenGL version 1.1 required! Stopping compilation.
#endif

#include "ogl_g3d.h"

/*=========================================================================
 SCF macro section
=========================================================================*/

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csGraphics3DOpenGL)


/*=========================================================================
 Method implementations
=========================================================================*/

csGraphics3DOpenGL::csGraphics3DOpenGL (iBase *p) :
  csGraphics3DOGLCommon (p)
{
}

csGraphics3DOpenGL::~csGraphics3DOpenGL ()
{
}

bool csGraphics3DOpenGL::Initialize (iObjectRegistry* p)
{
  csGraphics3DOGLCommon::Initialize(p);
  return NewInitialize ();
}

bool csGraphics3DOpenGL::Open ()
{
  return NewOpen ();
}
