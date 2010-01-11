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

#include "cssysdef.h"

#include "csutil/memheap.h"
#include "csutil/scopedlock.h"
#include "csutil/sysfunc.h"

namespace CS
{
  namespace Memory
  {
    // dlmalloc functions
    extern "C"
    {
      typedef void* mspace;
      mspace create_mspace(size_t capacity, int locked);
      size_t destroy_mspace(mspace msp);
      void* mspace_malloc(mspace msp, size_t bytes);
      void mspace_free(mspace msp, void* mem);
      void* mspace_realloc(mspace msp, void* mem, size_t newsize);
      int mspace_trim(mspace msp, size_t pad);
      size_t mspace_footprint(mspace msp);
    };
    
    Heap::Heap()
    {
      mspace = create_mspace (0, 1);
    }
    Heap::~Heap()
    {
      destroy_mspace (mspace);
    }
    
    void* Heap::Alloc (const size_t n)
    {
      csScopedLock<SpinLock> foo (lock);
      return mspace_malloc (mspace, n);
    }
    void Heap::Free (void* p)
    {
      csScopedLock<SpinLock> foo (lock);
      return mspace_free (mspace, p);
    }
    void* Heap::Realloc (void* p, size_t newSize)
    {
      csScopedLock<SpinLock> foo (lock);
      return mspace_realloc (mspace, p, newSize);
    }
    void Heap::Trim (size_t pad)
    {
      csScopedLock<SpinLock> foo (lock);
      mspace_trim (mspace, pad);
    }
    size_t Heap::Footprint ()
    {
      csScopedLock<SpinLock> foo (lock);
      return mspace_footprint (mspace);
    }
  } // namespace Memory
} // namespace CS

