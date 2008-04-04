/*
    Copyright (C) 2003 by Mat Sutcliffe <oktal@gmx.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_UTIL_WEAKREFHASH_H__
#define __CS_UTIL_WEAKREFHASH_H__

#include "csutil/hash.h"
#include "csutil/weakref.h"

/**
 * A hash table class, to be used with csWeakRef<> as the value type.
 */
template <class T, class K = unsigned int, 
  class ArrayMemoryAlloc = CS::Memory::AllocatorMalloc>
class csWeakRefHash :
  public csHash<csWeakRef<T>, K, 
                ArrayMemoryAlloc,
                csArraySafeCopyElementHandler<CS::Container::HashElement<
		  csWeakRef<T>, K> > >
{
public:
  typedef csHash<csWeakRef<T>, K, 
    ArrayMemoryAlloc, csArraySafeCopyElementHandler<
      CS::Container::HashElement<csWeakRef<T>, K> > > SuperClass;
  csWeakRefHash (size_t size = 23, size_t grow_rate = 5, 
    size_t max_size = 20000)
    : SuperClass(size, grow_rate, max_size)
  {
  }

  /// Copy constructor.
  csWeakRefHash (const csWeakRefHash<csWeakRef<T>, K, ArrayMemoryAlloc> &o) : 
  SuperClass(o) {}

  /**
  * Compacts the hash by removing entries which have been deleted.
  */
  void Compact()
  {
    for(size_t i=0; i<this->Elements.GetSize(); i++)
    {
      typename SuperClass::ElementArray& values = this->Elements[i];
      for (size_t j = values.GetSize(); j > 0; j--)
      {
        const size_t idx = j - 1;
        if(csComparator<csWeakRef<T>, csWeakRef<T> >::Compare (
	    values[idx].GetValue(), NULL) == 0)
        {
          values.DeleteIndexFast(idx);
          this->Size--;
        }
      }
    }
  }  
};

#endif // __CS_UTIL_WEAKREFHASH_H__
