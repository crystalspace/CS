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

#ifndef __CS_GL_RENDERBUFFER_H__
#define __CS_GL_RENDERBUFFER_H__

#include "ivideo/rndbuf.h"

SCF_VERSION(csSysRenderBuffer, 0,0,2);
/**
* This is a general buffer to be used by the renderer. It can ONLY be
* created by the renderer
*/
class csSysRenderBuffer : public iRenderBuffer
{
private:
  void *buffer;
  int size, compcount;
  csRenderBufferType type;
  csRenderBufferComponentType comptype;
  bool locked;
public:
  SCF_DECLARE_IBASE;

  csSysRenderBuffer (void *buffer, int size, csRenderBufferType type,
    csRenderBufferComponentType comptype, int compcount)
  {
    SCF_CONSTRUCT_IBASE (0)

    csSysRenderBuffer::buffer = buffer;
    csSysRenderBuffer::size = size;
    csSysRenderBuffer::type = type;
    csSysRenderBuffer::comptype = comptype;
    csSysRenderBuffer::compcount = compcount;
    locked = false;
  }

  virtual ~csSysRenderBuffer ()
  {
    if (buffer != 0)
      delete[] (char *)buffer;
  }

  /**
  * Lock the buffer to allow writing and give us a pointer to the data
  * The pointer will be 0 if there was some error
  */
  virtual void* Lock(csRenderBufferLockType lockType)
  {
    locked = true;
    return buffer;
  }

  /// Releases the buffer. After this all writing to the buffer is illegal
  virtual void Release() { locked = false; }

  /// Get type of buffer (where it's located)
  virtual csRenderBufferType GetBufferType() const { return type; }

  /// Get the size of the buffer (in bytes)
  virtual int GetSize() const { return size; }

  /// Gets the number of components per element
  virtual int GetComponentCount () const { return compcount; }

  /// Gets the component type
  virtual csRenderBufferComponentType GetComponentType () const
  { return comptype; }
};


SCF_VERSION(csVBORenderBuffer, 0,0,2);
/**
* This is a HW buffer to be used by the renderer. It can ONLY be
* created by the renderer
*/
class csVBORenderBuffer : public iRenderBuffer
{
private:
  uint bufferId;
  int size, compcount;
  csRenderBufferType type;
  csRenderBufferComponentType comptype;
  bool locked;
  csRenderBufferLockType lastLock;
  csGLExtensionManager *ext;
public:
  SCF_DECLARE_IBASE;

  csVBORenderBuffer (int size, csRenderBufferType type,
    csRenderBufferComponentType comptype, int compcount, csGLExtensionManager *ext)
  {
    SCF_CONSTRUCT_IBASE (0)
    csVBORenderBuffer::size = size;
    csVBORenderBuffer::type = type;
    csVBORenderBuffer::comptype = comptype;
    csVBORenderBuffer::compcount = compcount;
    csVBORenderBuffer::ext = ext;
    locked = false;
    ext->glGenBuffersARB (1, &bufferId);
    ext->glBindBufferARB (GL_ARRAY_BUFFER_ARB, bufferId);
    ext->glBufferDataARB (GL_ARRAY_BUFFER_ARB, size, 0, 
      (type==CS_BUF_STATIC) ? GL_STATIC_DRAW_ARB : GL_DYNAMIC_DRAW_ARB);
  }

  virtual ~csVBORenderBuffer ()
  {
    uint buf[1];
    buf[0] = bufferId;
    ext->glDeleteBuffersARB (1, buf);
  }

  /**
  * Lock the buffer to allow writing and give us a pointer to the data
  * The pointer will be 0 if there was some error
  */
  virtual void* Lock(csRenderBufferLockType lockType)
  {
    if (locked)
      return 0;

    switch (lockType)
    {
    case CS_BUF_LOCK_RENDER:
      locked = true; lastLock = lockType;
      return (void*)bufferId;
    case CS_BUF_LOCK_NORMAL:
      locked = true; lastLock = lockType;
      ext->glBindBufferARB (GL_ARRAY_BUFFER_ARB, bufferId);
      return ext->glMapBufferARB (GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
    }
    return 0;
  }

  /// Releases the buffer. After this all writing to the buffer is illegal
  virtual void Release() 
  {
    if (lastLock = CS_BUF_LOCK_NORMAL)
    {
      ext->glBindBufferARB (GL_ARRAY_BUFFER_ARB, bufferId);
      ext->glUnmapBufferARB (GL_ARRAY_BUFFER_ARB);
    }
    locked = false;
    lastLock = CS_BUF_LOCK_NOLOCK;
  }

  /// Get type of buffer (where it's located)
  virtual csRenderBufferType GetBufferType() const { return type; }

  /// Get the size of the buffer (in bytes)
  virtual int GetSize() const { return size; }

  /// Gets the number of components per element
  virtual int GetComponentCount () const { return compcount; }

  /// Gets the component type
  virtual csRenderBufferComponentType GetComponentType () const
  { return comptype; }
};


#endif //__CS_GL_RENDERBUFFER_H__