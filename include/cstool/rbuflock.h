/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

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

#ifndef __CS_CSTOOL_RBUFLOCK_H__
#define __CS_CSTOOL_RBUFLOCK_H__

/**\file
 * Helper class for convenient locking/unlocking of an iRenderBuffer.
 */

#include "csutil/ref.h"
#include "ivideo/rndbuf.h"

/**
 * Helper class for convenient locking/unlocking of an iRenderBuffer.
 * The contents can be accessed either directly or array-style in typed
 * way.
 */
template <class T>
class csRenderBufferLock
{
  csRef<iRenderBuffer> buffer;
  csRenderBufferLockType lockType;
  bool isLocked;
  T* lockBuf;
  size_t bufStride;
  
  csRenderBufferLock() {}
public:
  /**
   * Construct the helper.
   */
  csRenderBufferLock (iRenderBuffer* buf, 
    csRenderBufferLockType lock = CS_BUF_LOCK_NORMAL) : buffer(buf),
    lockType(lock), isLocked(false), lockBuf(0), 
    bufStride(buf ? buf->GetElementDistance() : 0)
  {
  }
  
  /**
   * Destruct the helper. Automatically unlocks the buffer if it was locked.
   */
  ~csRenderBufferLock()
  {
    Unlock();
  }
  
  /**
   * Lock the renderbuffer. Returns a pointer to the contained data.
   * \remarks Watch the stride of the buffer.
   */
  T* Lock ()
  {
    if (!isLocked)
    {
      lockBuf = 
	buffer ? ((T*)((uint8*)buffer->Lock (lockType))) : (T*)-1;
      isLocked = true;
    }
    return lockBuf;
  }
  
  /// Unlock the renderbuffer.
  void Unlock ()
  {
    if (isLocked)
    {
      if (buffer) buffer->Release();
      isLocked = false;
    }
  }
  
  /**
   * Retrieve a pointer to the contained data.
   * \remarks Watch the stride of the buffer.
   **/
  operator T* ()
  {
    return Lock();
  }

  /// Retrieve an item in the render buffer.
  T& operator [] (size_t n)
  {
    return Get (n);
  }

  /// Retrieve an item in the render buffer.
  T& Get (size_t n)
  {
    return *((T*)((uint8*)Lock() + n * bufStride));
  }
  
  /// Retrieve number of items in buffer.
  size_t GetSize() const
  {
    return buf ? buffer->GetElementCount() : 0;
  }

  /// Returns whether the buffer is valid (ie not null).
  bool IsValid() const { return buffer.IsValid(); }
};

#endif // __CS_CSTOOL_RBUFLOCK_H__
