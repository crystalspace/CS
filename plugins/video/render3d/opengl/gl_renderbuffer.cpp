/*
  Copyright (C) 2003 by Marten Svanfeldt
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

CS_LEAKGUARD_IMPLEMENT (csGLRenderBuffer)

SCF_IMPLEMENT_IBASE (csGLRenderBuffer)
  SCF_IMPLEMENTS_INTERFACE (iRenderBuffer)
SCF_IMPLEMENT_IBASE_END

static const int compSizes[] = 
{
  sizeof (char), sizeof (unsigned char), 
  sizeof (short), sizeof (unsigned short),
  sizeof (int), sizeof (unsigned int),
  sizeof (float),
  sizeof (double)
};
static const GLenum compGLtypes[] =
{
  GL_BYTE, GL_UNSIGNED_BYTE,
  GL_SHORT, GL_UNSIGNED_SHORT,
  GL_INT, GL_UNSIGNED_INT,
  GL_FLOAT,
  GL_DOUBLE
};

csGLRenderBuffer::csGLRenderBuffer (size_t size, csRenderBufferType type,
	csRenderBufferComponentType comptype, int compcount)
{
  SCF_CONSTRUCT_IBASE (0)
  csGLRenderBuffer::size = size;
  csGLRenderBuffer::type = type;
  csGLRenderBuffer::comptype = comptype;
  csGLRenderBuffer::compcount = compcount;
  stride = offset = 0;

  compSize = compSizes [comptype];
  compGLType = compGLtypes [comptype];

  nodelete = false;
}

csGLRenderBuffer::~csGLRenderBuffer()
{
  SCF_DESTRUCT_IBASE()
}

void csGLRenderBuffer::SetComponentType (
  csRenderBufferComponentType type)
{
  comptype = type;
  compSize = compSizes [type];
  compGLType = compGLtypes [type];
}

//-----------------------------------------------------------------

void* csSysRenderBuffer::RenderLock (csGLRenderBufferLockType type)
{
  return (unsigned char*)Lock (CS_BUF_LOCK_NORMAL) + offset;
}

//-----------------------------------------------------------------

void* csVBORenderBuffer::RenderLock (csGLRenderBufferLockType type)
{
  if (locked && (lastRLock != type)) return (void*)-1;
  
  locked = true; 
  ext->glBindBufferARB (bufferTarget, bufferId);
  lastRLock = type;

  return (void*)offset; // Offset for an interleaved buffer
}

//-----------------------------------------------------------------

csPtr<iRenderBuffer> csGLGraphics3D::CreateRenderBuffer (size_t size, 
  csRenderBufferType type, csRenderBufferComponentType componentType,
  int componentCount)
{
  if (use_hw_render_buffers && size >= vbo_thresshold)
  {
    csVBORenderBuffer *buffer = new csVBORenderBuffer 
      (size, type, componentType, componentCount, false, ext);
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

csPtr<iRenderBuffer> csGLGraphics3D::CreateIndexRenderBuffer (size_t size, 
    csRenderBufferType type, csRenderBufferComponentType componentType,
    size_t rangeStart, size_t rangeEnd)
{
  if (use_hw_render_buffers && size >= vbo_thresshold)
  {
    csVBORenderBuffer *buffer = new csVBORenderBuffer 
      (size, type, componentType, 1, true, ext);
    buffer->SetRange (rangeStart, rangeEnd);
    return csPtr<iRenderBuffer> (buffer);
  }
  else
  {
    csSysRenderBuffer *buffer = new csSysRenderBuffer (
      new char[size], size, type, componentType, 1);
    buffer->SetRange (rangeStart, rangeEnd);
    return csPtr<iRenderBuffer> (buffer);
  }
  return csPtr<iRenderBuffer> (0);
}

void csGLGraphics3D::CreateInterleavedRenderBuffers (size_t size,
  csRenderBufferType type, int count, csRefArray<iRenderBuffer>& buffers)
{
  csRef<iRenderBuffer> buf;
  if (use_hw_render_buffers && size >= vbo_thresshold)
  {
    csVBORenderBuffer* master = new csVBORenderBuffer (size, type,
      CS_BUFCOMP_BYTE, 1, false, ext);
    buf.AttachNew (master);
    buffers.Push (buf);
    for(int i=1;i<count;i++)
    {
      csVBORenderBuffer *interleaved = new csVBORenderBuffer (
        master);
      interleaved->SetInterleaved ();
      buf.AttachNew (interleaved);
      buffers.Push (buf);
    }
  }
  else
  {
    char *mem = new char[size];
    csSysRenderBuffer *master = new csSysRenderBuffer (mem,
      size, type, CS_BUFCOMP_BYTE, 1);
    
    buf.AttachNew (master);
    buffers.Push (buf);

    for(int i=1;i<count;i++) 
    {
      csSysRenderBuffer *interleaved = new csSysRenderBuffer (mem,
        size, type, CS_BUFCOMP_BYTE, 1);
      interleaved->SetInterleaved ();
      buf.AttachNew (interleaved);
      buffers.Push (buf);
    }
  }
}
