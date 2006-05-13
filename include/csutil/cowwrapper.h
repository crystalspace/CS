/*
    Copyright (C) 2006 by Frank Richter

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

#ifndef __CS_CSUTIL_COWWRAPPER_H__
#define __CS_CSUTIL_COWWRAPPER_H__

// Hack: Work around problems caused by #defining 'new'.
#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# undef new
#endif
#include <new>

#include "allocator.h"

/**\file
 */

namespace CS
{

  template<typename T, class MemoryAllocator = Memory::AllocatorMalloc>
  class CowWrapper
  {
    struct WrappedData
    {
      int refcount;
  
      void IncRef () { refcount++; }
      void DecRef () 
      { 
	refcount--; 
        if (refcount == 0) 
        {
          this->~WrappedData();
          MemoryAllocator::Free (this);
        }
      }
      int GetRefCount () const { return refcount; }
      
      T data;
    public:
      WrappedData (const T& other) : refcount (1), data (other) {}
      WrappedData (const WrappedData& other) : refcount (1), data (other.data) {}
    };
  public:
    static const size_t allocSize = sizeof (WrappedData);
  private:
    csRef<WrappedData> data;
    inline WrappedData* NewData (const T& other)
    {
      WrappedData* p = (WrappedData*)MemoryAllocator::Alloc (allocSize);
      new (p) WrappedData (other);
      return p;
    }
  public:
    CowWrapper (const CowWrapper& other) : data (other.data) {}
    CowWrapper (const T& other)
    {
      data.AttachNew (NewData (other));
    }

    const T& operator* () const
    {
      return data->data;
    }
    T& operator* ()
    {
      if (data->GetRefCount() > 1)
      {
	WrappedData* newData = NewData (data->data);
        data.AttachNew (newData);
      }
      return data->data;
    }
    const T* operator -> () const
    { return &(operator*()); }
    T* operator -> ()
    { return &(operator*()); }
  };

} // namespace CS

#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# define new CS_EXTENSIVE_MEMDEBUG_NEW
#endif

#endif // __CS_CSUTIL_COWWRAPPER_H__
