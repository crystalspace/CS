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
 * The contents can be accessed either directly, array-style or iterator-style 
 * in a typed way.
 * \remarks The TbufferKeeper template argument can be used to have the
 *  lock store the buffer in a simple iRenderBuffer* (instead a csRef<>)
 *  to avoid an IncRef() and DecRef() if it is known that the buffer will
 *  not be destroyed as long as the lock exists.
 */
template <class T, class TbufferKeeper = csRef<iRenderBuffer> >
class csRenderBufferLock
{
  /// Buffer that is being locked
  TbufferKeeper buffer;
  /// State of locking (type and whether this buffer is locked)
  uint lockState;
  /// Buffer data
  T* lockBuf;
  /// Distance between two elements
  size_t bufStride;
#ifdef CS_DEBUG
  /// Number of elements
  size_t elements;
#endif
  /// Index of current element
  size_t currElement;
  
  enum { LockedFlag = 0x10000, LockTypeMask = 0xffff };
  inline bool IsLocked() { return lockState & LockedFlag; }
  inline void SetLocked (bool b) 
  { lockState = b ? (lockState | LockedFlag) : (lockState & ~LockedFlag); }
  inline csRenderBufferLockType LockType() 
  { return (csRenderBufferLockType)(lockState & LockTypeMask); }
  
  csRenderBufferLock() {}
public:
  /**
   * Construct the helper.
   */
  csRenderBufferLock (iRenderBuffer* buf, 
    csRenderBufferLockType lock = CS_BUF_LOCK_NORMAL) : buffer(buf),
    lockState(lock), lockBuf(0), 
    bufStride(buf ? buf->GetElementDistance() : 0),
    currElement(0)
  {
#ifdef CS_DEBUG
    elements = buf->GetElementCount();
#endif
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
    if (!IsLocked())
    {
      lockBuf = 
	buffer ? ((T*)((uint8*)buffer->Lock (LockType()))) : (T*)-1;
      SetLocked (true);
    }
    return lockBuf;
  }
  
  /// Unlock the renderbuffer.
  void Unlock ()
  {
    if (IsLocked ())
    {
      if (buffer) buffer->Release();
      SetLocked (false);
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

  /// Get current element.
  T& operator*()
  {
    return Get (currElement);
  }

  /// Set current element to the next.
  void operator++ ()
  {
    currElement++;
    CS_ASSERT(currElement<elements);
  }

  /// Retrieve an item in the render buffer.
  T& operator [] (size_t n)
  {
    return Get (n);
  }

  /// Retrieve an item in the render buffer.
  T& Get (size_t n)
  {
    CS_ASSERT (n < elements);
    return *((T*)((uint8*)Lock() + n * bufStride));
  }
  
  /// Retrieve number of items in buffer.
  size_t GetSize() const
  {
    return buffer ? buffer->GetElementCount() : 0;
  }

  /// Returns whether the buffer is valid (ie not null).
  bool IsValid() const { return buffer.IsValid(); }
};

#endif // __CS_CSTOOL_RBUFLOCK_H__
