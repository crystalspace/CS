/*
  Copyright (C) 2010 by Frank Richter

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

#ifndef __PROFILESCOPE_H__
#define __PROFILESCOPE_H__

#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{
  class csGLGraphics3D;

  class ProfileScope
  {
    csGLGraphics3D* renderer;
    const char* descr;
    int64 startStamp;
    GLuint startQuery, endQuery;
  public:
    ProfileScope (csGLGraphics3D* renderer, const char* descr);
    ~ProfileScope ();
  };

}
CS_PLUGIN_NAMESPACE_END(gl3d)

#endif // __PROFILESCOPE_H__
