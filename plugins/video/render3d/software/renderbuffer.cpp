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

#include "soft_g3d.h"
#include "renderbuffer.h"

SCF_IMPLEMENT_IBASE (csSoftRenderBuffer)
  SCF_IMPLEMENTS_INTERFACE (iRenderBuffer)
SCF_IMPLEMENT_IBASE_END

static const int compSizes[] = 
  {sizeof (char), sizeof (unsigned char), 
   sizeof (short), sizeof (unsigned short),
   sizeof (int), sizeof (unsigned int),
   sizeof (float),
   sizeof (double)
  };

csSoftRenderBuffer::csSoftRenderBuffer 
  (void *buffer, size_t size, csRenderBufferType type, 
  csRenderBufferComponentType comptype, int compcount)
{
  /*static const GLenum compGLtypes[] =
    {GL_BYTE, GL_UNSIGNED_BYTE,
     GL_SHORT, GL_UNSIGNED_SHORT,
     GL_INT, GL_UNSIGNED_INT,
     GL_FLOAT,
     GL_DOUBLE};*/

  SCF_CONSTRUCT_IBASE (0)
  csSoftRenderBuffer::size = size;
  csSoftRenderBuffer::type = type;
  csSoftRenderBuffer::comptype = comptype;
  csSoftRenderBuffer::compcount = compcount;

  offset = stride = 0;

  compSize = compSizes [comptype];

  csSoftRenderBuffer::buffer = buffer;
  locked = false;
}

csSoftRenderBuffer::~csSoftRenderBuffer ()
{
  if (buffer != 0)
    delete[] (char *)buffer;
  SCF_DESTRUCT_IBASE()
}

void csSoftRenderBuffer::SetComponentType (csRenderBufferComponentType type)
{
  comptype = type; 
  compSize = compSizes[type];
}

//-----------------------------------------------------------------

csPtr<iRenderBuffer> csSoftwareGraphics3DCommon::CreateRenderBuffer (size_t size, 
  csRenderBufferType type, csRenderBufferComponentType componentType,
  int componentCount)
{
  csSoftRenderBuffer *buffer = new csSoftRenderBuffer (
    new char[size], size, type, componentType, componentCount);
  return csPtr<iRenderBuffer> (buffer);
}

csPtr<iRenderBuffer> csSoftwareGraphics3DCommon::CreateIndexRenderBuffer (
  size_t size, csRenderBufferType type, csRenderBufferComponentType componentType,
  size_t rangeStart, size_t rangeEnd)
{
  csSoftRenderBuffer *buffer = new csSoftRenderBuffer (
    new char[size], size, type, componentType, 1);
  return csPtr<iRenderBuffer> (buffer);
}

void csSoftwareGraphics3DCommon::CreateInterleavedRenderBuffers (size_t size,
  csRenderBufferType type, int count, csRefArray<iRenderBuffer>& buffers)
{
  char *mem = new char[size];
  csRef<iRenderBuffer> buf;
  csSoftRenderBuffer *master = new csSoftRenderBuffer (
    mem, size, type, CS_BUFCOMP_BYTE, 1);
  buf.AttachNew (master);
  buffers.Push (buf);

  for (int i = 0; i < count; i ++) 
  {
    csSoftRenderBuffer *interleaved = new csSoftRenderBuffer (mem,
      size, type, CS_BUFCOMP_BYTE, 1);
    buf.AttachNew (interleaved);
    buffers.Push (buf);
  }
}
