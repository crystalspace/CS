/*
  Copyright (C) 2003 by Mårten Svanfeldt
                        Anders Stenberg

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

#include "gl_render3d.h"
#include "gl_renderbuffer.h"

SCF_IMPLEMENT_IBASE (csSysRenderBuffer)
SCF_IMPLEMENTS_INTERFACE (iRenderBuffer)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csVBORenderBuffer)
SCF_IMPLEMENTS_INTERFACE (iRenderBuffer)
SCF_IMPLEMENT_IBASE_END

csPtr<iRenderBuffer> csGLRender3D::CreateRenderBuffer (int size, 
  csRenderBufferType type, csRenderBufferComponentType componentType,
  int componentCount)
{
  if (use_hw_render_buffers && type != CS_BUF_INDEX)
  {
    csVBORenderBuffer *buffer = new csVBORenderBuffer (
      size, type, componentType, componentCount, ext);
    return csPtr<iRenderBuffer> (buffer);
  }
  else
  {
    csSysRenderBuffer *buffer = new csSysRenderBuffer (
      new char[size], size, type, componentType, componentCount);
    return csPtr<iRenderBuffer> (buffer);
  }
  return csPtr<iRenderBuffer> (0);
}