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

#ifndef __CS_GL_RENDERBUFFER_H__
#define __CS_GL_RENDERBUFFER_H__

#include "ivideo/rndbuf.h"

enum csGLRenderBufferLockType
{
  CS_GLBUF_RENDERLOCK_ARRAY,
  CS_GLBUF_RENDERLOCK_ELEMENTS
};

class csGLRenderBuffer : public iRenderBuffer
{
protected:
  friend class csGLGraphics3D;

  size_t size;
  int compcount, compSize, stride, offset;
  csRenderBufferType type;
  csRenderBufferComponentType comptype;
  GLenum compGLType;
  bool nodelete;
public:
  SCF_DECLARE_IBASE;

  csGLRenderBuffer (int size, csRenderBufferType type,
    csRenderBufferComponentType comptype, int compcount);
  
  virtual ~csGLRenderBuffer ();

  /// Get type of buffer (where it's located)
  virtual csRenderBufferType GetBufferType() const { return type; }

  /// Get the size of the buffer (in bytes)
  virtual int GetSize() const { return size; }

  /// Sets the number of components per element
  virtual void SetComponentCount (int count)
  { compcount = count; }

  /// Gets the number of components per element
  virtual int GetComponentCount () const { return compcount; }

  /// Sets the component type
  virtual void SetComponentType (csRenderBufferComponentType type);

  /// Gets the component type
  virtual csRenderBufferComponentType GetComponentType () const
  { return comptype; }

  virtual void* RenderLock (csGLRenderBufferLockType type) = 0;

  virtual void SetStride (int s)
  { stride = s; }

  virtual int GetStride () const
  { return stride; }

  virtual void SetOffset (int o)
  { offset = o; }

  void SetInterleaved () { nodelete = true; }
};


SCF_VERSION(csSysRenderBuffer, 0,0,2);
/**
* This is a general buffer to be used by the renderer. It can ONLY be
* created by the renderer
*/
class csSysRenderBuffer : public csGLRenderBuffer
{
private:
  void *buffer;
  bool locked;
public:
  csSysRenderBuffer (void *buffer, int size, csRenderBufferType type,
    csRenderBufferComponentType comptype, int compcount) :
    csGLRenderBuffer (size, type, comptype, compcount)
  {
    csSysRenderBuffer::buffer = buffer;
    locked = false;
  }

  virtual ~csSysRenderBuffer ()
  {
    if(!nodelete) delete[] (char *)buffer;
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

  virtual void CopyToBuffer(const void *data, int length)
  {
    memcpy(buffer, data, length);
  }

  virtual void* RenderLock (csGLRenderBufferLockType type);
};


SCF_VERSION(csVBORenderBuffer, 0,0,2);
/**
* This is a HW buffer to be used by the renderer. It can ONLY be
* created by the renderer
*/
class csVBORenderBuffer : public csGLRenderBuffer
{
private:
  char* tempbuf;
  GLuint bufferId;
  csRenderBufferType buftype;
  bool locked;
  csRenderBufferLockType lastLock;
  csGLRenderBufferLockType lastRLock;
  csGLExtensionManager *ext;
  bool index;
public:
  csVBORenderBuffer (int size, csRenderBufferType type,
    csRenderBufferComponentType comptype, int compcount, 
    bool index, csGLExtensionManager *ext) :
    csGLRenderBuffer (size, type, comptype, compcount)
  {
    csVBORenderBuffer::ext = ext;
    csVBORenderBuffer::index = index;
    csVBORenderBuffer::buftype = type;
    locked = false;
    ext->glGenBuffersARB (1, &bufferId);
    const GLenum bufferType = 
      index ? GL_ELEMENT_ARRAY_BUFFER_ARB : GL_ARRAY_BUFFER_ARB;
    ext->glBindBufferARB (bufferType, bufferId);
    ext->glBufferDataARB (bufferType, size, 0, 
      (type==CS_BUF_STATIC) ? GL_STATIC_DRAW_ARB : GL_DYNAMIC_DRAW_ARB);
    lastLock = CS_BUF_LOCK_NOLOCK;
    ext->glBindBufferARB (bufferType, 0);
  }
  csVBORenderBuffer (csVBORenderBuffer *copy) :
    csGLRenderBuffer (copy->size, copy->type, copy->comptype, copy->compcount)
  {
    ext = copy->ext;
    index = copy->index;
    buftype = copy->buftype;
    bufferId = copy->bufferId;
    locked = false;
    lastLock = CS_BUF_LOCK_NOLOCK;
  }

  virtual ~csVBORenderBuffer ()
  {
    GLuint buf = bufferId;
    if(!nodelete) ext->glDeleteBuffersARB (1, &buf);
  }

  /**
  * Lock the buffer to allow writing and give us a pointer to the data
  * The pointer will be 0 if there was some error
  */
  virtual void* Lock(csRenderBufferLockType lockType)
  {
    if (!locked)
    {
      switch (lockType)
      {
	case CS_BUF_LOCK_NORMAL:
	  RenderLock (CS_GLBUF_RENDERLOCK_ARRAY);
	  lastLock = CS_BUF_LOCK_NORMAL;
          return ext->glMapBufferARB (index?
                      GL_ELEMENT_ARRAY_BUFFER_ARB:GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
	case CS_BUF_LOCK_NOLOCK:
	  break;
      }
    }
    return (void*)-1;
  }

  /// Releases the buffer. After this all writing to the buffer is illegal
  virtual void Release() 
  {
    const GLenum bufferType = 
      index ? GL_ELEMENT_ARRAY_BUFFER_ARB : GL_ARRAY_BUFFER_ARB;
    if (lastLock == CS_BUF_LOCK_NORMAL)
    {
      ext->glBindBufferARB (bufferType, bufferId);
      // @@@ Should be real error check.
      ext->glUnmapBufferARB (bufferType);
    }
    ext->glBindBufferARB (bufferType, 0);
    locked = false;
    lastLock = CS_BUF_LOCK_NOLOCK;
  }

  virtual void CopyToBuffer(const void *data, int length)
  {
    ext->glBindBufferARB (index?
      GL_ELEMENT_ARRAY_BUFFER_ARB:GL_ARRAY_BUFFER_ARB, bufferId);
    ext->glBufferDataARB (index?
      GL_ELEMENT_ARRAY_BUFFER_ARB:GL_ARRAY_BUFFER_ARB, length, data, 
      (buftype==CS_BUF_STATIC) ? GL_STATIC_DRAW_ARB : GL_DYNAMIC_DRAW_ARB);
  }

  virtual void* RenderLock (csGLRenderBufferLockType type);
};


#endif //__CS_GL_RENDERBUFFER_H__
