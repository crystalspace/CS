/*
    Copyright (C) 2002 by Mårten Svanfeldt
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
#include "csutil/scf.h"
#include "csutil/cscolor.h"
#include "csgeom/vector2.h"
#include "csgeom/vector3.h"
#include "ivideo/rndbuf.h"

#include "gl_render3d.h"
#include "gl_vaobufmgr.h"

SCF_IMPLEMENT_IBASE (csVaoRenderBuffer)
  SCF_IMPLEMENTS_INTERFACE (iRenderBuffer)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csVaoRenderBufferManager)
  SCF_IMPLEMENTS_INTERFACE (iRenderBufferManager)
SCF_IMPLEMENT_IBASE_END

csVaoRenderBuffer::csVaoRenderBuffer(int size, CS_RENDERBUFFER_TYPE type, csVaoRenderBufferManager* vaomgr)
{
    SCF_CONSTRUCT_IBASE (NULL)

    tempbuffer = NULL;
    indexbuffer = NULL;
    csVaoRenderBuffer::size = size;
    csVaoRenderBuffer::type = type;
    csVaoRenderBuffer::vaomgr = vaomgr;
    locked = false;

    discarded = true;

    // Get a buffer-index to use
    VAObufferID = vaomgr->render3d->ext.glNewObjectBufferATI(size, NULL, GL_STATIC_ATI);
}

csVaoRenderBuffer::~csVaoRenderBuffer()
{
  vaomgr->render3d->ext.glFreeObjectBufferATI(VAObufferID);
  //release other buffers
  delete[] tempbuffer;
  delete[] indexbuffer;
}

void csVaoRenderBuffer::Release()
{ 
  locked = false; 
  if(lastlock != iRenderBuffer::CS_BUF_LOCK_RENDER && type != CS_BUF_INDEX)
  {
    //copy to our VAO buffer
    vaomgr->render3d->ext.glUpdateObjectBufferATI(VAObufferID, 0, size, tempbuffer, GL_DISCARD_ATI);
  }
  discarded = false;
}

csPtr<iRenderBuffer> csVaoRenderBufferManager::CreateBuffer(int buffersize, CS_RENDERBUFFER_TYPE location)
{
  csVaoRenderBuffer *buffer = new csVaoRenderBuffer (buffersize, location, this);
  return csPtr<iRenderBuffer> (buffer);
}

bool csVaoRenderBufferManager::Initialize(csGLRender3D* render3d)
{
    csVaoRenderBufferManager::render3d = render3d;
    return true;
}