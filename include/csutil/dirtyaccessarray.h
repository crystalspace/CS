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
 * <p>
 * The main use of this class is when you absolutely need access
 * to the internal array that is in this class. This can be useful
 * if you want to access some external module (like OpenGL).
 */
template <class T>
class csDirtyAccessArray : public csArray<T>
{
private:
  int RefCount;

public:
  /**
   * Initialize object to hold initially 'ilimit' elements, and increase
   * storage by 'ithreshold' each time the upper bound is exceeded.
   */
  csDirtyAccessArray (int ilimit = 0, int ithreshold = 0)
  	: csArray<T> (ilimit, ithreshold)
  {
    RefCount = 0;
  }

  // Reference counting.
  void IncRef () { RefCount++; }

  // Reference counting. Delete array when reference reaches 0.
  void DecRef ()
  {
    if (RefCount == 1) { this->DeleteAll (); } // see *1*
    RefCount--;
  }

  /// Get the pointer to the start of the array.
  T* GetArray ()
  {
    if (this->Length () > 0) // see *1*
      return &this->Get (0);
    else
      return 0;
  }

  /// Get the pointer to the start of the array.
  const T* GetArray () const
  {
    if (this->Length () > 0) // see *1*
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
    if (this->Length () > 0) // see *1*
    {
      T* copy = new T [this->Length ()];
      memcpy (copy, &this->Get (0), sizeof (T) * this->Length ());
      return copy;
    }
    else
      return 0;
  }
};

#endif // __CS_DIRTYACCESSARRAY_H__
