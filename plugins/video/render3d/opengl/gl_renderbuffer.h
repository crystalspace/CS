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

  int size, compcount, compSize;
  csRenderBufferType type;
  csRenderBufferComponentType comptype;
  GLenum compGLType;
public:
  SCF_DECLARE_IBASE;

  csGLRenderBuffer (int size, csRenderBufferType type,
    csRenderBufferComponentType comptype, int compcount);
  
  virtual ~csGLRenderBuffer ();

  /// Get type of buffer (where it's located)
  virtual csRenderBufferType GetBufferType() const { return type; }

  /// Get the size of the buffer (in bytes)
  virtual int GetSize() const { return size; }

  /// Gets the number of components per element
  virtual int GetComponentCount () const { return compcount; }

  /// Gets the component type
  virtual csRenderBufferComponentType GetComponentType () const
  { return comptype; }

  virtual void* RenderLock (csGLRenderBufferLockType type) = 0;
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
    locked = false;
    ext->glGenBuffersARB (1, &bufferId);
    ext->glBindBufferARB (index?
      GL_ELEMENT_ARRAY_BUFFER_ARB:GL_ARRAY_BUFFER_ARB, bufferId);
    ext->glBufferDataARB (index?
      GL_ELEMENT_ARRAY_BUFFER_ARB:GL_ARRAY_BUFFER_ARB, size, 0, 
      (type==CS_BUF_STATIC) ? GL_STATIC_DRAW_ARB : GL_DYNAMIC_DRAW_ARB);
  }

  virtual ~csVBORenderBuffer ()
  {
    GLuint buf = bufferId;
    ext->glDeleteBuffersARB (1, &buf);
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
    if (lastLock == CS_BUF_LOCK_NORMAL)
    {
      ext->glBindBufferARB (index?
                            GL_ELEMENT_ARRAY_BUFFER_ARB:GL_ARRAY_BUFFER_ARB, bufferId);
      // @@@ Should be real error check.
      ext->glUnmapBufferARB (index?GL_ELEMENT_ARRAY_BUFFER_ARB:GL_ARRAY_BUFFER_ARB);
    }
    locked = false;
    lastLock = CS_BUF_LOCK_NOLOCK;
  }

  virtual void* RenderLock (csGLRenderBufferLockType type);
};


#endif //__CS_GL_RENDERBUFFER_H__
