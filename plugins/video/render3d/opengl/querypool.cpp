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

#include "gl_render3d.h" // includes querypool.h

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{
  GLuint QueryPool::AllocQuery()
  {
    if (queries.GetSize() == 0)
    {
      queries.SetSize (queryGenerateNum);
      csGLGraphics3D::ext->glGenQueriesARB (queryGenerateNum, queries.GetArray());
    }
    return queries.Pop ();
  }
  
  void QueryPool::RecycleQuery (GLuint query)
  {
    queries.Push (query);
    if (queries.GetSize() >= queryDiscardThreshold)
    {
      size_t newSize = queries.GetSize() - queryDiscardNum;
      csGLGraphics3D::ext->glDeleteQueriesARB (queryDiscardNum, queries.GetArray()+newSize);
      queries.Truncate (newSize);
    }
  }
  
  void QueryPool::DiscardAllQueries ()
  {
    if (queries.GetSize() > 0)
    {
      csGLGraphics3D::ext->glDeleteQueriesARB ((GLsizei)queries.GetSize(), queries.GetArray());
      queries.DeleteAll ();
    }
  }
}
CS_PLUGIN_NAMESPACE_END(gl3d)
