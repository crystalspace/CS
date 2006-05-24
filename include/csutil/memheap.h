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

#ifndef __CS_CSUTIL_MEMHEAP_H__
#define __CS_CSUTIL_MEMHEAP_H__

/**\file
 * Separate memory heap.
 */

#if defined(CS_MEMORY_TRACKER)
#include "csutil/csstring.h"
#include "csutil/memdebug.h"
#include <typeinfo>
#endif

/**\addtogroup util_memory
 * @{ */

namespace CS
{
  namespace Memory
  {
    /**
     * A separate heap from which memory can be allocated.
     * \remarks Memory must be freed by the same heap it was allocated from.
     * \remarks Thread-safe.
     */
    class CS_CRYSTALSPACE_EXPORT Heap
    {
      /// The 'mspace' used by this heap.
      void* mspace;
      
      Heap (Heap const&);   		// Illegal; unimplemented.
      void operator= (Heap const&); 	// Illegal; unimplemented.
    public:
      Heap();
      ~Heap();
    
      /// Allocate a block of memory of size \p n.
      void* Alloc (const size_t n);
      /// Free the block \p p.
      void Free (void* p);
      /// Resize the allocated block \p p to size \p newSize.
      void* Realloc (void* p, size_t newSize);

      /**
       * Try to return as much unused memory to the system as possible.
       * \p pad optionally specifies a minimum amount of memory to be 
       * retained, in case future allocations are anticipated.
       */
      void Trim (size_t pad = 0);
    };

    /**
     * A memory allocator that allocates from a heap.
     * The \p HeapAccess template argument should behave like the Heap
     * class. \sa HeapAccessPointer
     */
    template<class HeapAccess>
    class AllocatorHeapBase : protected HeapAccess
    {
    #if defined(CS_MEMORY_TRACKER)
      csMemTrackerInfo* mti;
    #endif
    public:
    #if defined(CS_MEMORY_TRACKER)
      AllocatorHeapBase () : mti (0) { }
      AllocatorHeapBase (const HeapAccess& heap) : HeapAccess (heap), mti (0) {}
    #else
      AllocatorHeapBase () { }
      AllocatorHeapBase (const HeapAccess& heap) : HeapAccess (heap) {}
    #endif
      /// Allocate a block of memory of size \p n.
      void* Alloc (const size_t n)
      {
      #if defined(CS_MEMORY_TRACKER)
        size_t* p = (size_t*)HeapAccess::Alloc (n + sizeof (size_t));
        *p = n;
        p++;
        if (mti == 0)
        {
	  /*csString mtiInfo;
          mtiInfo.Format ("%s with %p", typeid(*this).name(), HeapAccess::GetHeap());*/
	  mti = mtiRegisterAlloc (n, /*mtiInfo*/typeid(*this).name());
        }
        else
	  mtiUpdateAmount (mti, 1, int (n));
        return p;
      #else
        return HeapAccess::Alloc (n);
      #endif
      }
      /// Free the block \p p.
      void Free (void* p)
      {
      #if defined(CS_MEMORY_TRACKER)
        size_t* x = (size_t*)p;
        x--;
        size_t allocSize = *x;
        HeapAccess::Free (x);
        if (mti) mtiUpdateAmount (mti, -1, -int (allocSize));
      #else
        HeapAccess::Free (p);
      #endif
      }
      /// Resize the allocated block \p p to size \p newSize.
      void* Realloc (void* p, size_t newSize)
      {
      #ifdef CS_MEMORY_TRACKER
        if (p == 0) return Alloc (newSize);
        size_t* x = (size_t*)p;
        x--;
        if (mti) mtiUpdateAmount (mti, -1, -int (*x));
        size_t* np = 
	  (size_t*)HeapAccess::Realloc (x, newSize + sizeof (size_t));
        *np = newSize;
        np++;
        if (mti) mtiUpdateAmount (mti, 1, int (newSize));
        return np;
      #else
        return HeapAccess::Realloc (p, newSize);
      #endif
      }
      /// Set the information used for memory tracking.
      void SetMemTrackerInfo (const char* info)
      {
      #ifdef CS_MEMORY_TRACKER
        if (mti != 0) return;
        /*csString mtiInfo;
        mtiInfo.Format ("%s with %p for %s", typeid(*this).name(), 
          HeapAccess::GetHeap(), info);*/
        mti = mtiRegister (/*mtiInfo*/info);
      #else
        (void)info;
      #endif
      }
    };

    /**
     * Heap accessor for AllocatorHeapBase.
     * The \p HeapContainer template argument should behave like a pointer to
     * an object of type Heap* or compatible interface.
     */
    template<class HeapContainer = Heap*>
    struct HeapAccessPointer
    {
      HeapContainer heap;
      void* Alloc (const size_t n)
      {
        return heap->Alloc (n);
      }
      void Free (void* p)
      {
        heap->Free (p);
      }
      void* Realloc (void* p, size_t newSize)
      {
        return heap->Realloc (p, newSize);
      }
      const HeapContainer& GetHeap ()
      {
        return heap;
      }
    };

    /**
     * A memory allocator that allocates from a heap.
     * The \p HeapContainer template argument should behave like a pointer to
     * an object of type Heap* or compatible interface.
     */
    template<class HeapPtr = Heap*>
    class AllocatorHeap : public AllocatorHeapBase<HeapAccessPointer<HeapPtr> >
    {
    public:
    };
  } // namespace Memory
} // namespace CS

/** @} */

#endif // __CS_CSUTIL_MEMHEAP_H__
