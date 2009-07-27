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
 *
 * The buffer is locked upon construction of the csRenderBufferLock<> object
 * and unlocked on destruction.
 *
 * The contents can be accessed either directly, array-style or iterator-style 
 * in a typed way.
 * \remarks The TbufferKeeper template argument can be used to have the
 *  lock store the buffer in a csRef<iRenderBuffer> (instead a iRenderBuffer*)
 *  in case there is a risk that the buffer gets destroyed while thelock 
 *  exists.
 */
template <class T, class TbufferKeeper = iRenderBuffer*>
class csRenderBufferLock
{
  /// Buffer that is being locked
  TbufferKeeper buffer;
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

  typedef csRenderBufferLock<T, TbufferKeeper> LockType;
  /**
   * Helper class used when returning values from operators such as
   * ++ or +=. The idea is that these operators should return a pointer
   * to the element, yet should check accesses for validity in debug mode.
   * However, it is quite common to increment the element pointer beyond
   * the last element in loops, but not use it. Checking the element number
   * for validity may throw a false alarm in this cases. Hence this class
   * to allow for "delayed" checking.
   */
  struct PointerProxy
  {
  #ifdef CS_DEBUG
    const LockType& parent;
    size_t elemNum;
  #else
    T* p;
  #endif

    PointerProxy (const LockType& parent, size_t elemNum) 
  #ifdef CS_DEBUG
      : parent (parent), elemNum (elemNum)
  #else
      : p (&parent.Get (elemNum))
  #endif
    { }
    T& operator*()
    {
  #ifdef CS_DEBUG
      return parent.Get (elemNum);
  #else
      return *p;
  #endif
    }
    const T& operator*() const
    {
  #ifdef CS_DEBUG
      return parent.Get (elemNum);
  #else
      return *p;
  #endif
    }
    operator T*() const
    {
  #ifdef CS_DEBUG
      return &(parent.Get (elemNum));
  #else
      return p;
  #endif
    }
  };

  csRenderBufferLock() {}
  // Copying the locking stuff is somewhat nasty so ... prevent it
  csRenderBufferLock (const csRenderBufferLock& other) {}

  /// Unlock the renderbuffer.
  void Unlock ()
  {
    if (buffer) buffer->Release();
  }
public:
  /**
   * Construct the helper. Locks the buffer.
   */
  csRenderBufferLock (iRenderBuffer* buf, 
    csRenderBufferLockType lock = CS_BUF_LOCK_NORMAL) : buffer(buf),
    lockBuf(0), bufStride(buf ? buf->GetElementDistance() : 0),
    currElement(0)
  {
#ifdef CS_DEBUG
    elements = buf ? buf->GetElementCount() : 0;
#endif    
    lockBuf = 
      buffer ? ((T*)((uint8*)buffer->Lock (lock))) : (T*)-1;
  }
  
  /**
   * Destruct the helper. Unlocks the buffer.
   */
  ~csRenderBufferLock()
  {
    Unlock();
  }
  
  /**
   * Lock the renderbuffer. Returns a pointer to the contained data.
   * \remarks Watch the stride of the buffer.
   */
  T* Lock () const
  {
    return lockBuf;
  }
  
  /**
   * Retrieve a pointer to the contained data.
   * \remarks Watch the stride of the buffer.
   **/
  operator T* () const
  {
    return Lock();
  }

  /// Get current element.
  T& operator*() const
  {
    return Get (currElement);
  }

  /// Set current element to the next, pre-increment version.
  PointerProxy operator++ ()  
  {
    currElement++;
    return PointerProxy (*this, currElement);
  }

  /// Set current element to the next, post-increment version.
  PointerProxy operator++ (int)
  {
    size_t n = currElement;
    currElement++;
    return PointerProxy (*this, n);
  }

  /// Add a value to the current element index.
  PointerProxy operator+= (int n)
  {
    currElement += n;
    return PointerProxy (*this, currElement);
  }

  /// Retrieve an item in the render buffer.
  T& operator [] (size_t n) const
  {
    return Get (n);
  }

  /// Retrieve an item in the render buffer.
  T& Get (size_t n) const
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
