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

#include "cssysdef.h"

#include "profilescope.h"

#include "gl_render3d.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{
  ProfileScope::ProfileScope (csGLGraphics3D* renderer, const char* descr)
  : renderer (renderer), descr (descr), startStamp (0),
    startQuery (0), endQuery (0)   
  {
    if (renderer->glProfiling)
    {
      startQuery = renderer->queryPool.AllocQuery ();
      endQuery = renderer->queryPool.AllocQuery ();
      startStamp = csGetMicroTicks();
      csGLGraphics3D::ext->glQueryCounter (startQuery, GL_TIMESTAMP);
    }
  }

  ProfileScope::~ProfileScope ()
  {
    if (startQuery != 0)
    {
      csGLGraphics3D::ext->glQueryCounter (endQuery, GL_TIMESTAMP);
      renderer->profileHelper.RecordTimeSpan (renderer->frameNum,
					      startStamp,
					      startQuery, endQuery,
					      descr);
    }
  }

}
CS_PLUGIN_NAMESPACE_END(gl3d)
