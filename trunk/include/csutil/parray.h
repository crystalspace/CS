/*
  Crystal Space Pointer Array
  Copyright (C) 2003 by Jorrit Tyberghein

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

#ifndef __CS_PTRARR_H__
#define __CS_PTRARR_H__

/**\file
 * Pointer Array
 */

//-----------------------------------------------------------------------------
// Note *1*: The explicit "this->" is needed by modern compilers (such as gcc
// 3.4.x) which distinguish between dependent and non-dependent names in
// templates.  See: http://gcc.gnu.org/onlinedocs/gcc/Name-lookup.html
//-----------------------------------------------------------------------------

#include "csextern.h"
#include "csutil/array.h"

template <class T>
class csPDelArrayElementHandler : public csArrayElementHandler<T>
{
public:
  static void Construct (T* address, T const& src)
  {
    *address = src;
  }

  static void Destroy (T* address)
  {
    delete *address;
  }

  static void InitRegion (T* address, size_t count)
  {
    memset (address, 0, count*sizeof (T));
  }
};

/**
 * An array of pointers. No ref counting is done on the elements in this
 * array. Use csRefArray if you want ref counting to happen.
 * This array will delete elements (using 'delete') as needed.
 * This array properly initializes new elements in the array to 0 (the NULL
 * pointer).
 */
template <class T,
          class MemoryAllocator = CS::Container::ArrayAllocDefault,
          class CapacityHandler = csArrayCapacityFixedGrow<16> >
class csPDelArray : 
  public csArray<T*, csPDelArrayElementHandler<T*>, MemoryAllocator,
                 CapacityHandler>
{
  typedef csArray<T*, csPDelArrayElementHandler<T*>, MemoryAllocator,
    CapacityHandler> superclass;

private:
  csPDelArray (const csPDelArray&);            // Illegal; unimplemented.
  csPDelArray& operator= (const csPDelArray&); // Illegal; unimplemented.

public:
  /**
   * Initialize object to hold initially 'ilimit' elements, and increase
   * storage by 'ithreshold' each time the upper bound is exceeded.
   */
  csPDelArray (size_t ilimit = 0,
    const CapacityHandler& ch = CapacityHandler()) :
    superclass (ilimit, ch) {}

  /**
   * Get and clear the element 'n' from vector. This spot in the array
   * will be set to 0. You are responsible for deleting the returned
   * pointer later.
   */
  T* GetAndClear (size_t n)
  {
    T* ret = this->Get (n); // see *1*
    this->InitRegion (n, 1);
    return ret;
  }

  /**
   * Extract element number 'n' from vector. The element is deleted
   * from the array and returned. You are responsible for deleting the
   * pointer later.
   */
  T* Extract (size_t n)
  {
    T* ret = GetAndClear (n);
    this->DeleteIndex (n); // see *1*
    return ret;
  }

  /// Pop an element from tail end of array.
  T* Pop ()
  {
    CS_ASSERT (this->GetSize () > 0);
    T* ret = GetAndClear (this->GetSize () - 1); // see *1*
    Truncate (this->GetSize () - 1);
    return ret;
  }

  /**
   * Set the actual number of items in this array. This can be used to shrink
   * an array (like Truncate()) or to enlarge an array, in which case it will
   * properly construct all new items using their default (zero-argument)
   * constructor.
   * \param n New array length.
   */
  void SetSize (size_t n)
  {
    superclass::SetSize (n, 0);
  }

  /**
   * Variant of SetLength() which copies the pointed-to object instead of
   * the actual pointer.
   */
  void SetSize (size_t n, T const &what)
  {
    if (n <= this->GetSize ()) // see *1*
    {
      this->Truncate (n);
    }
    else
    {
      size_t old_len = this->GetSize (); // see *1*
      superclass::SetSize (n);
      for (size_t i = old_len ; i < n ; i++) this->Get(i) = new T (what);
    }
  }
};

#endif // __CS_PTRARR_H__

