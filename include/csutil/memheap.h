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

#include "csutil/spinlock.h"

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
      SpinLock lock;
      
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

      /**
       * Return the total amount of memory used for this heap.
       */
      size_t Footprint ();
    };

    /**
     * A memory allocator that allocates from a heap.
     * The \p HeapAccess template argument must behave like the Heap
     * class. \sa HeapAccessPointer
     */
    template<class HeapAccess>
    class AllocatorHeapBase : protected HeapAccess
    {
    #if defined(CS_MEMORY_TRACKER)
      const char* mti;
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
        void* p = HeapAccess::Alloc (n);
        if (mti == 0)
        {
	  /*csString mtiInfo;
          mtiInfo.Format ("%s with %p", typeid(*this).name(), HeapAccess::GetHeap());*/
	  mti = /*mtiInfo*/typeid(*this).name();
        }
	CS::Debug::MemTracker::RegisterAlloc (p, n, mti);
        return p;
      #else
        return HeapAccess::Alloc (n);
      #endif
      }
      /// Free the block \p p.
      void Free (void* p)
      {
      #if defined(CS_MEMORY_TRACKER)
        CS::Debug::MemTracker::RegisterFree (p);
      #endif
        HeapAccess::Free (p);
      }
      /// Resize the allocated block \p p to size \p newSize.
      void* Realloc (void* p, size_t newSize)
      {
      #ifdef CS_MEMORY_TRACKER
        if (p == 0) return Alloc (newSize);
        void* np = HeapAccess::Realloc (p, newSize);
        CS::Debug::MemTracker::UpdateSize (p, np, newSize);
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
        mti = info;
      #else
        (void)info;
      #endif
      }
    };

    /**
     * Heap accessor for AllocatorHeapBase.
     * The \p HeapContainer template argument must behave like a pointer to
     * an object of type Heap* or compatible interface.
     */
    template<class HeapContainer = Heap*>
    struct HeapAccessPointer
    {
      HeapContainer heap;

      CS_DEPRECATED_METHOD_MSG ("HeapAccessPointer instance uninitialized")
      HeapAccessPointer () {}
      HeapAccessPointer (HeapContainer heap) : heap (heap) {}

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
     * The \p HeapPtr template argument must be a type that can be converted
     * to Heap*.
     */
    template<class HeapPtr = Heap*>
    class AllocatorHeap : public AllocatorHeapBase<HeapAccessPointer<HeapPtr> >
    {
    public:
      CS_DEPRECATED_METHOD_MSG ("AllocatorHeap instance uninitialized")
      AllocatorHeap () {}

      AllocatorHeap (HeapPtr heap) : 
        AllocatorHeapBase<HeapAccessPointer<HeapPtr> > (
          HeapAccessPointer<HeapPtr> (heap)) {}
    };
  } // namespace Memory
} // namespace CS

/** @} */

#endif // __CS_CSUTIL_MEMHEAP_H__
