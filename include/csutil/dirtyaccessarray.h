/*
    Crystal Space utility library: vector class interface
    Copyright (C) 1998,1999,2000 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CS_DIRTYACCESSARRAY_H__
#define __CS_DIRTYACCESSARRAY_H__

/**\file 
 * Templated array class, allowing direct access to the internally stored 
 * array.
 */

//-----------------------------------------------------------------------------
// Note *1*: The explicit "this->" is needed by modern compilers (such as gcc
// 3.4.x) which distinguish between dependent and non-dependent names in
// templates.  See: http://gcc.gnu.org/onlinedocs/gcc/Name-lookup.html
//-----------------------------------------------------------------------------

#include "array.h"

/**
 * A templated array class.  The only difference with csArray is that this 
 * class allows you to get the address of the internal array. This is of 
 * course dangerous so use of this class should be restricted and avoided.
 * 
 * The main use of this class is when you absolutely need access
 * to the internal array that is in this class. This can be useful
 * if you want to access some external module (like OpenGL).
 */
template <class T,
	  class ElementHandler = csArrayElementHandler<T>,
          class MemoryAllocator = CS::Container::ArrayAllocDefault,
          class CapacityHandler = CS::Container::ArrayCapacityDefault>
class csDirtyAccessArray : 
  public csArray<T, ElementHandler, MemoryAllocator, CapacityHandler>
{
public:
  /**
   * Initialize object to have initial capacity of \c in_capacity elements.
   * The storage increase depends on the specified capacity handler. The
   * default capacity handler accepts a threshold parameter by which the 
   * storage is increased each time the upper bound is exceeded.
   */
  csDirtyAccessArray (size_t in_capacity = 0,
    const CapacityHandler& ch = CapacityHandler())
    : csArray<T, ElementHandler, MemoryAllocator, CapacityHandler> (
      in_capacity, ch) {}

  /// Get the pointer to the start of the array.
  T* GetArray ()
  {
    if (this->GetSize () > 0) // see *1*
      return &this->Get (0);
    else
      return 0;
  }

  /// Get the pointer to the start of the array.
  const T* GetArray () const
  {
    if (this->GetSize () > 0) // see *1*
      return &this->Get (0);
    else
      return 0;
  }

  /**
   * Get a copy of the array. The caller is responsible for
   * deleting this with 'delete[]'. Returns 0 if there are no
   * items in the array.
   */
  T* GetArrayCopy ()
  {
    if (this->GetSize () > 0) // see *1*
    {
      T* copy = new T [this->GetSize ()];
      memcpy (copy, &this->Get (0), sizeof (T) * this->GetSize ());
      return copy;
    }
    else
      return 0;
  }
  
  /**
   * Get the pointer to the start of the array and set internal array
   * pointer to null and size and capacity to 0.
   * After the data is detached it's up to you to destroy all elements
   * and free the memory!
   */
  T* Detach ()
  {
    T* ptr = GetArray ();
    this->SetDataVeryUnsafe (nullptr);
    this->SetSizeVeryUnsafe (0);
    this->SetCapacityVeryUnsafe (0);
    return ptr;
  }
};

/**
 * A variant of csDirtyAccessArray with reference-counted contents. That is,
 * when the reference count of the array drops to 0, it's contents are deleted
 * (however, not the array object itself).
 */
template <class T, class ElementHandler = csArrayElementHandler<T>,
          class MemoryAllocator = CS::Container::ArrayAllocDefault,
          class CapacityHandler = CS::Container::ArrayCapacityDefault>
class csDirtyAccessArrayRefCounted : 
  public csDirtyAccessArray<T, ElementHandler, MemoryAllocator, 
                            CapacityHandler>
{
private:
  int RefCount;
public:
  csDirtyAccessArrayRefCounted (int ilimit = 0,
    const CapacityHandler& ch = CapacityHandler())
    : csDirtyAccessArray<T, ElementHandler, MemoryAllocator, 
                         CapacityHandler> (ilimit, ch), RefCount (0)
  { }

  /// Reference counting.
  void IncRef () { RefCount++; }

  /// Reference counting. Delete array contents when reference reaches 0.
  void DecRef ()
  {
    if (RefCount == 1) { this->DeleteAll (); } // see *1*
    RefCount--;
  }

};


#endif // __CS_DIRTYACCESSARRAY_H__
