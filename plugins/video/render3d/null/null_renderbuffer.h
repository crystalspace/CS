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

#ifndef __CS_NULL_RENDERBUFFER_H__
#define __CS_NULL_RENDERBUFFER_H__

#include "ivideo/rndbuf.h"

class csNullRenderBuffer : public iRenderBuffer
{
protected:
  friend class csNullGraphics3D;

  size_t size;
  int compcount, compSize, stride, offset;
  csRenderBufferType type;
  csRenderBufferComponentType comptype;
public:
  SCF_DECLARE_IBASE;

  csNullRenderBuffer (size_t size, csRenderBufferType type,
    csRenderBufferComponentType comptype, int compcount);
  
  virtual ~csNullRenderBuffer ();

  /// Get type of buffer (where it's located)
  virtual csRenderBufferType GetBufferType() const { return type; }

  /// Get the size of the buffer (in bytes)
  virtual size_t GetSize() const { return size; }

  /// Sets the number of components per element
  virtual void SetComponentCount (int count)
  { compcount = count; }

  /// Gets the number of components per element
  virtual int GetComponentCount () const { return compcount; }

  /// Sets the component type
  virtual void SetComponentType (csRenderBufferComponentType type)
  { comptype = type; }

  /// Gets the component type
  virtual csRenderBufferComponentType GetComponentType () const
  { return comptype; }

  virtual void SetStride (int s)
  { stride = s; }

  virtual int GetStride () const 
  { return stride; }

  virtual void SetOffset (int o)
  { offset = o; }
};


SCF_VERSION(csSysRenderBuffer, 0,0,3);
/**
* This is a general buffer to be used by the renderer. It can ONLY be
* created by the renderer
*/
class csSysRenderBuffer : public csNullRenderBuffer
{
private:
  void *buffer;
  bool locked;
  uint version;
public:
  csSysRenderBuffer (void *buffer, size_t size, csRenderBufferType type,
    csRenderBufferComponentType comptype, int compcount) :
    csNullRenderBuffer (size, type, comptype, compcount)
  {
    csSysRenderBuffer::buffer = buffer;
    locked = false;
    version = 0;
  }

  virtual ~csSysRenderBuffer ()
  {
    delete[] (char *)buffer;
  }

  /**
  * Lock the buffer to allow writing and give us a pointer to the data
  * The pointer will be 0 if there was some error
  */
  virtual void* Lock(csRenderBufferLockType lockType, 
    bool samePointer = false)
  {
    version++;
    locked = true;
    return buffer;
  }

  /// Releases the buffer. After this all writing to the buffer is illegal
  virtual void Release() { locked = false; }

  virtual void CopyToBuffer(const void *data, size_t length)
  {
    version++;
    memcpy(buffer, data, length);
  }

  virtual unsigned int GetVersion () 
  {
    return version;
  }
};

#endif //__CS_NULL_RENDERBUFFER_H__
