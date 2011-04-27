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
#include "csutil/callstack.h"
#include "csutil/scf.h"
#include "csutil/sysfunc.h"
#include "csgeom/math.h"

#include "csgfx/renderbuffer.h"

// Define if you want to debug locking/unlocking mismatch issues
//#define DEBUG_LOCKING

CS_LEAKGUARD_IMPLEMENT (csRenderBuffer);

csRenderBuffer::csRenderBuffer (size_t size, csRenderBufferType type, 
  csRenderBufferComponentType componentType, uint componentCount,
  size_t rangeStart, size_t rangeEnd, bool copy) :
    scfImplementationType (this, 0), bufferSize (size), props (type, 
    componentType, componentCount, copy), rangeStart (rangeStart), 
    rangeEnd (rangeEnd), version (0),  buffer (0)
{
#if defined (CS_DEBUG) && defined (DEBUG_LOCKING)
  lockStack = 0;
#endif
}

csRenderBuffer::~csRenderBuffer ()
{
#if defined(CS_DEBUG) && defined(DEBUG_LOCKING)
  if (lockStack != 0)
  {
    lockStack->Print ();
    lockStack->Free ();
  }
#endif
  //Notify callback
  if (callback.IsValid ())
    callback->RenderBufferDestroyed (this);

  if (props.doDelete) cs_free (buffer);
}

void* csRenderBuffer::Lock (csRenderBufferLockType lockType)
{
  if (props.isLocked)
  {
    if ((lockType > CS_BUF_LOCK_READ) 
      || (props.lastLock > CS_BUF_LOCK_READ)
      || ((uint)lockType != props.lastLock))
    {
#if defined (CS_DEBUG) && defined (DEBUG_LOCKING)
      if (lockStack)
      {
        csPrintf ("Renderbuffer %p Lock() while locked:\n", this);
        lockStack->Print ();
      }
#endif
      return (void*)-1;
    }
  }

  props.lastLock = lockType;
  props.isLocked = true;

  void* rb = 0;

  if (masterBuffer.IsValid ())
  {
    rb = masterBuffer->Lock (lockType);
    if (rb == (void*)-1) return rb;
    rb = (uint8*)rb + props.offset;
  }
  else
  {
    if (buffer == 0)
    {
      buffer = (unsigned char*)cs_malloc (bufferSize);
      props.doDelete = true;
    }
    rb = buffer;
  }

  CS_ASSERT (rb != 0);

#if defined (CS_DEBUG) && defined (DEBUG_LOCKING)
  if (lockStack == 0) 
    lockStack = csCallStackHelper::CreateCallStack (0, true);
#endif

  return rb;
}

void csRenderBuffer::Release ()
{
  if (masterBuffer.IsValid ())
    masterBuffer->Release ();

  if (props.lastLock == CS_BUF_LOCK_NORMAL)
  {
    version++;
  }
  props.isLocked = false;
#if defined (CS_DEBUG) && defined (DEBUG_LOCKING)
  if (lockStack != 0)
  {
    lockStack->Free ();
    lockStack = 0;
  }
#endif
}

void csRenderBuffer::CopyInto (const void* data, size_t elementCount, 
  size_t elemOffset)
{
  if (masterBuffer.IsValid ()) return;
  const size_t elemSize = 
    csRenderBufferComponentSizes[props.comptype & ~CS_BUFCOMP_NORMALIZED] * props.compCount;
  const size_t byteOffs = elemSize * elemOffset;
  version++;
  if (!props.doCopy)
  {
    buffer = (unsigned char*)data;
  }
  else
  {
    if ((buffer == 0) || !props.doDelete)
    {
      unsigned char* oldBuffer = buffer;
      buffer = (unsigned char*)cs_malloc (bufferSize);
      props.doDelete = true;
      if (oldBuffer != 0)
      {
        if (byteOffs > 0)
          memcpy (buffer, oldBuffer, byteOffs);
        size_t endOffs = (byteOffs + elementCount * elemSize);
        if (endOffs < bufferSize)
          memcpy (buffer+endOffs, oldBuffer+endOffs, bufferSize - endOffs);
      }
    }
    memcpy (buffer + byteOffs, data, csMin (bufferSize - byteOffs, 
      elementCount * elemSize));
  }
}

size_t csRenderBuffer::GetElementCount () const
{
  if (masterBuffer.IsValid ())
    return masterBuffer->GetElementCount ();

  return bufferSize / 
    (props.compCount * csRenderBufferComponentSizes[props.comptype & ~CS_BUFCOMP_NORMALIZED]);
}

void csRenderBuffer::SetData (const void *data)
{
  if (masterBuffer.IsValid ()) return;
  version++;
  if (props.doDelete)
  {
    cs_free (buffer);
    props.doDelete = false;
  }
  buffer = (unsigned char*)data;
}

csRef<csRenderBuffer> csRenderBuffer::CreateRenderBuffer (size_t elementCount, 
  csRenderBufferType type, csRenderBufferComponentType componentType,
  uint componentCount)
{
  if (componentCount > 255) return 0;

  size_t size = elementCount * componentCount * 
    csRenderBufferComponentSizes[componentType & ~CS_BUFCOMP_NORMALIZED];
  csRenderBuffer* buf = new csRenderBuffer (size, type, 
    componentType, componentCount, 0, 0);
  return csPtr<csRenderBuffer> (buf);
}

csRef<csRenderBuffer> csRenderBuffer::CreateRenderBuffer (size_t elementCount, 
  csRenderBufferType type, csRenderBufferComponentType componentType,
  uint componentCount, bool copy)
{
  if (componentCount > 255) return 0;

  size_t size = elementCount * componentCount * 
    csRenderBufferComponentSizes[componentType & ~CS_BUFCOMP_NORMALIZED];
  csRenderBuffer* buf = new csRenderBuffer (size, type, 
    componentType, componentCount, 0, 0, copy);
  return csPtr<csRenderBuffer> (buf);
}

csRef<csRenderBuffer> csRenderBuffer::CreateIndexRenderBuffer (size_t elementCount, 
  csRenderBufferType type, csRenderBufferComponentType componentType,
  size_t rangeStart, size_t rangeEnd)
{
  size_t size = elementCount * csRenderBufferComponentSizes[componentType & ~CS_BUFCOMP_NORMALIZED];
  csRenderBuffer* buf = new csRenderBuffer (size, type, 
    componentType, 1, rangeStart, rangeEnd);
  buf->props.isIndex = true;
  return csPtr<csRenderBuffer> (buf);
}

csRef<csRenderBuffer> csRenderBuffer::CreateIndexRenderBuffer (size_t elementCount, 
  csRenderBufferType type, csRenderBufferComponentType componentType,
  size_t rangeStart, size_t rangeEnd, bool copy)
{
  size_t size = elementCount * csRenderBufferComponentSizes[componentType & ~CS_BUFCOMP_NORMALIZED];
  csRenderBuffer* buf = new csRenderBuffer (size, type, 
    componentType, 1, rangeStart, rangeEnd, copy);
  buf->props.isIndex = true;
  return csPtr<csRenderBuffer> (buf);
}

csRef<csRenderBuffer> csRenderBuffer::CreateInterleavedRenderBuffers (size_t elementCount, 
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
      * csRenderBufferComponentSizes[element.componentType & ~CS_BUFCOMP_NORMALIZED];
  }
  elementSize = offsets[count];
  if (elementSize > 255) return 0;
  csRef<csRenderBuffer> master;
  master.AttachNew (new csRenderBuffer (elementCount * elementSize,
    type, CS_BUFCOMP_BYTE, (uint)elementSize, 0, 0, true));
  for (i = 0; i < count; i++)
  {
    const csInterleavedSubBufferOptions& element = elements[i];
    csRenderBuffer* rbuf = new csRenderBuffer (0, type, 
      element.componentType, element.componentCount, 0, 0, false);
    rbuf->props.offset = (uint)offsets[i];
    rbuf->props.stride = (uint)elementSize;
    rbuf->masterBuffer = master;
    (buffers + i)->AttachNew (rbuf);
  }
  return master;
}

const char* csRenderBuffer::GetDescrFromBufferName (csRenderBufferName bufferName)
{
  static const char* const bufferDescr[CS_BUFFER_COUNT] = {
    "index", "position", "normal", "lit color", "unlit color", 
    "texture coordinate 0", "texture coordinate 1", 
    "texture coordinate 2", "texture coordinate 3", 
    "texture coordinate lightmap", "generic 0", "generic 1", "generic 2",
    "generic 3", "tangent", "bitangent"
  };

  if ((bufferName >= 0) && (bufferName < CS_BUFFER_COUNT))
    return bufferDescr[bufferName];
  return 0;
}

csRenderBufferName csRenderBuffer::GetBufferNameFromDescr (const char* name)
{
  if (name == 0) return CS_BUFFER_NONE; 
  
  struct StrToName
  {
    const char* descr;
    csRenderBufferName name;
  };
  static const StrToName strMap[] = {
    // Sort alphabetically by name!
    {"binormal",		    CS_BUFFER_BINORMAL},
    {"bitangent",		    CS_BUFFER_BINORMAL},
    {"color",			    CS_BUFFER_COLOR},
    {"generic 0",		    CS_BUFFER_GENERIC0},
    {"generic 1",		    CS_BUFFER_GENERIC1},
    {"generic 2",		    CS_BUFFER_GENERIC2},
    {"generic 3",		    CS_BUFFER_GENERIC3},
    {"index",			    CS_BUFFER_INDEX},
    {"lit color",		    CS_BUFFER_COLOR},
    {"none",			    CS_BUFFER_NONE},
    {"normal",			    CS_BUFFER_NORMAL},
    {"position",		    CS_BUFFER_POSITION},
    {"primary color",		    CS_BUFFER_COLOR},
    {"tangent",			    CS_BUFFER_TANGENT},
    {"texture coordinate",	    CS_BUFFER_TEXCOORD0},
    {"texture coordinate 0",	    CS_BUFFER_TEXCOORD0},
    {"texture coordinate 1",	    CS_BUFFER_TEXCOORD1},
    {"texture coordinate 2",	    CS_BUFFER_TEXCOORD2},
    {"texture coordinate 3",	    CS_BUFFER_TEXCOORD3},
    {"texture coordinate lightmap", CS_BUFFER_TEXCOORD_LIGHTMAP},
    {"unlit color",		    CS_BUFFER_COLOR_UNLIT}
  };

  size_t l = 0, r = sizeof (strMap) / sizeof (StrToName);
  while (l < r)
  {
    size_t m = (l + r) / 2;
    int i = strcmp (strMap[m].descr, name);
    if (i == 0) return strMap[m].name;
    if (i < 0)
      l = m + 1;
    else
      r = m;
  }
  return CS_BUFFER_NONE;
}

void csRenderBuffer::SetRenderBufferProperties (size_t elementCount, 
  csRenderBufferType type, csRenderBufferComponentType componentType,
  uint componentCount, bool copy)
{
  CS_ASSERT (!props.isIndex);
  if (componentCount > 255) return;

#ifdef CS_DEBUG
  size_t newSize = elementCount * componentCount * 
    csRenderBufferComponentSizes[componentType & ~CS_BUFCOMP_NORMALIZED];
  CS_ASSERT (newSize <= bufferSize);
#else
  (void)elementCount; // unused except for above so silence the warning
#endif
  props.bufferType = type;
  props.comptype = componentType;
  props.compCount = componentCount;
  props.doCopy = copy;
}

void csRenderBuffer::SetIndexBufferProperties (size_t elementCount, 
  csRenderBufferType type, csRenderBufferComponentType componentType,
  size_t rangeStart, size_t rangeEnd, bool copy)
{
  CS_ASSERT (props.isIndex);

#ifdef CS_DEBUG
  size_t newSize = elementCount * csRenderBufferComponentSizes[componentType & ~CS_BUFCOMP_NORMALIZED];
  CS_ASSERT (newSize <= bufferSize);
#else
  (void)elementCount; // unused except for above assert so silence the warning
#endif
  props.bufferType = type;
  props.comptype = componentType;
  this->rangeStart = rangeStart;
  this->rangeEnd = rangeEnd;
  props.doCopy = copy;
}
