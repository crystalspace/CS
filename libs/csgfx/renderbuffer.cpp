/*
  Copyright (C) 2003-2005 by Marten Svanfeldt
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
#include "csgeom/math.h"

#include "csgfx/renderbuffer.h"

CS_LEAKGUARD_IMPLEMENT (csRenderBuffer);
SCF_IMPLEMENT_IBASE (csRenderBuffer)
  SCF_IMPLEMENTS_INTERFACE (iRenderBuffer)
SCF_IMPLEMENT_IBASE_END

csRenderBuffer::csRenderBuffer (size_t size, csRenderBufferType type, 
				csRenderBufferComponentType componentType, 
				uint componentCount, size_t rangeStart, 
				size_t rangeEnd, bool copy)
  : bufferType (type), comptype (componentType), compCount (componentCount), 
  stride(0), offset (0), rangeStart (rangeStart), rangeEnd (rangeEnd), 
  version (0), doCopy (copy), doDelete (false), isLocked (false), 
  isIndex (false), buffer (0)
{
  SCF_CONSTRUCT_IBASE (0);

  bufferSize = size;
  if (doCopy) 
  {
    buffer = new unsigned char[size];
    doDelete = true;
  }
}

csRenderBuffer::~csRenderBuffer ()
{
  if (doDelete) delete[] buffer;
  buffer = 0;
  SCF_DESTRUCT_IBASE ();
}

void* csRenderBuffer::Lock (csRenderBufferLockType lockType)
{
  if (isLocked)
  {
    if ((lockType > CS_BUF_LOCK_READ) || (lastLock > CS_BUF_LOCK_READ)
      || (lockType != lastLock))
      return (void*)-1;
  }

  lastLock = lockType;
  isLocked = true;
  CS_ASSERT(buffer != 0);
  return (void*)buffer;
}

void csRenderBuffer::Release ()
{
  if (lastLock == CS_BUF_LOCK_NORMAL)
  {
    version++;
  }
  isLocked = false;
}

void csRenderBuffer::CopyInto (const void *data, size_t elementCount)
{
  if (masterBuffer.IsValid()) return;
  version++;
  if (doCopy)
  {
    memcpy (buffer, data, csMin (bufferSize, 
      elementCount * csRenderBufferComponentSizes[comptype] * compCount));
  }
  else
  {
    buffer = (unsigned char*)data;
  }
}

size_t csRenderBuffer::GetElementCount() const
{
  return bufferSize / (compCount * csRenderBufferComponentSizes[comptype]);
}

csRef<iRenderBuffer> csRenderBuffer::CreateRenderBuffer (size_t elementCount, 
  csRenderBufferType type, csRenderBufferComponentType componentType,
  uint componentCount, bool copy)
{
  size_t size = elementCount * componentCount * 
    csRenderBufferComponentSizes[componentType];
  csRenderBuffer* buf = new csRenderBuffer (size, type, 
    componentType, componentCount, 0, 0, copy);
  return csPtr<iRenderBuffer> (buf);
}

csRef<iRenderBuffer> csRenderBuffer::CreateIndexRenderBuffer (size_t elementCount, 
    csRenderBufferType type, csRenderBufferComponentType componentType,
    size_t rangeStart, size_t rangeEnd, bool copy)
{
  size_t size = elementCount * csRenderBufferComponentSizes[componentType];
  csRenderBuffer* buf = new csRenderBuffer (size, type, 
    componentType, 1, rangeStart, rangeEnd, copy);
  buf->isIndex = true;
  return csPtr<iRenderBuffer> (buf);
}

csRef<iRenderBuffer> csRenderBuffer::CreateInterleavedRenderBuffers (size_t elementCount, 
  csRenderBufferType type, uint count, const csInterleavedSubBufferOptions* elements, 
  csRef<iRenderBuffer>* buffers)
{
  size_t elementSize = 0;
  CS_ALLOC_STACK_ARRAY(size_t, offsets, count + 1);
  uint i;
  offsets[0] = 0;
  for (i = 0; i < count; i++)
  {
    const csInterleavedSubBufferOptions& element = elements[i];
    offsets[i+1] = offsets[i] + element.componentCount
      * csRenderBufferComponentSizes[element.componentType];
  }
  elementSize = offsets[count];
  csRef<iRenderBuffer> master;
  master.AttachNew (new csRenderBuffer (elementCount * elementSize,
    type, CS_BUFCOMP_BYTE, elementSize, 0, 0, true));
  for (i = 0; i < count; i++)
  {
    const csInterleavedSubBufferOptions& element = elements[i];
    csRenderBuffer* rbuf = new csRenderBuffer (0, type, 
      element.componentType, element.componentCount, 0, 0, false);
    rbuf->offset = offsets[i];
    rbuf->stride = elementSize;
    rbuf->masterBuffer = master;
    (buffers + i)->AttachNew (rbuf);
  }
  return master;
}
