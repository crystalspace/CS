/*
    2d canvas for embedding on existing gl context for Crystal Space (header)
    Copyright (C) 2006 by George Yohng <yohng@drivex.dosware.8m.com>
    Copyright (C) 2006 by Anders Stenberg <dentoid@users.sourceforge.net>
    Copyright (C) 2006 by Pablo Martin <caedesv@users.sourceforge.net>

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

#ifndef __CS_EMBEDGL2D_H__
#define __CS_EMBEDGL2D_H__

#include "csutil/scf.h"
#include "csplugincommon/opengl/glcommon2d.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "csutil/win32/win32.h"
#include "csutil/win32/wintools.h"
#elif !defined(CS_PLATFORM_MACOSX)
#define USE_GLX
#endif

#ifdef USE_GLX
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#endif

#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif

class csGraphics2DGLEmbed : public scfImplementationExt1<csGraphics2DGLEmbed,
  csGraphics2DGLCommon, iOpenGLInterface>
{
public:
  csGraphics2DGLEmbed (iBase *iParent);
  virtual ~csGraphics2DGLEmbed () {};
  
  bool Open();

  bool PerformExtensionV (char const* command, va_list args);
  virtual void *GetProcAddress (const char *funcname)
  {
    #ifdef WIN32
      return (void*)wglGetProcAddress ((const char*)funcname);
    #elif defined(USE_GLX)
      return (void*)glXGetProcAddressARB ((const GLubyte*) funcname);
    #else
      return 0;
    #endif
  }
};

#endif // __CS_EMBEDGL2D_H__
