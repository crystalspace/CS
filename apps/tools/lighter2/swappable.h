/*
  Copyright (C) 2007 by Frank Richter

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

#ifndef __SWAPPABLE_H__
#define __SWAPPABLE_H__

#include "lighter.h"
#include "csutil/threading/mutex.h"
#include "csutil/threading/atomicops.h"

namespace lighter
{
  /// The functions that need to be implemented by an object to be swappable.
  struct iSwappable
  {
    virtual ~iSwappable() {}
    /**
     * Get the data to be swapped out. \a data is assumed to have been 
     * allocated with SwappableHeap.
     */
    virtual void GetSwapData (void*& data, size_t& size) = 0;
    /// Return size that would be swapped out.
    virtual size_t GetSwapSize() = 0;
    /**
     * Swap in data again.
     * \remarks \a data is allocated with SwappableHeap. Callee is supposed
     *   to take ownership.
     */
    virtual void SwapIn (void* data, size_t size) = 0;
    /// Return a descriptive string of the swappable. Used for debugging.
    virtual const char* Describe() const = 0;
  };
  
  /// Class that manages swapping in/swapping out of objects
  class SwapManager
  {
  public:
    CS::Memory::Heap swapHeap;

    SwapManager (size_t maxSize);
    ~SwapManager ();
  
    /// Register an object that can be swapped out
    void RegisterSwappable (iSwappable* obj);
    /// Unregister.
    void UnregisterSwappable (iSwappable* obj);
    /// Ensure object is in "locked" memory
    void Lock (iSwappable* obj);
    /// Mark object as "unlocked" so it can be swapped out
    void Unlock (iSwappable* obj);
    /// Notify of a size change of an object
    void UpdateSize (iSwappable* obj);

    /// Free memory, around \a desiredAmount bytes
    void FreeMemory (size_t desiredAmount);
    
    /// Get sizes of swapmanager-managed data
    void GetSizes (uint64& swappedIn, uint64& swappedOut, uint64& maxSize);
  private:
    // One in-memory entry
    struct SwapEntry
    {
      SwapEntry ()
        : obj (0), swapStatus (swappedIn), lastUnlockTime (0), 
        lastSize ((size_t)~0)
      {
      }

      iSwappable* obj;
      enum
      {
        swappedOut = 0,
        swappedOutEmpty = 1,
        swappedIn = 2,
        swapping = 3
      };

      int32 swapStatus;
      size_t lastUnlockTime;
      size_t lastSize;
    };
    csBlockAllocator<SwapEntry> entryAlloc;

    // Swap out one swap-entry to disk
    bool SwapOut (SwapEntry* e);

    // Swap in one swap-entry from disk
    bool SwapIn (SwapEntry* e);

    // Given an object, get a temporary filename for the cache
    csString GetFileName (iSwappable* obj);

    // Compare two LM entries
    static int SwapEntryAgeCompare (SwapEntry* const & e1, 
                                    SwapEntry* const& e2)
    {
      if (e1->lastUnlockTime < e2->lastUnlockTime)
        return -1;
      else if (e2->lastUnlockTime < e1->lastUnlockTime)
        return 1;
      else
        return 0;
    }
    
    //All current LM cache entries
    typedef csHash<SwapEntry*, csPtrKey<iSwappable> > SwapCacheType;
    SwapCacheType swapCache;

    //Currently unlocked LM cache entires (potential to be swapped out)
    typedef csSet<csPtrKey<SwapEntry> > UnlockedEntriesType;
    UnlockedEntriesType unlockedCacheEntries;

    //Statistics for house-keeping
    size_t maxCacheSize, currentCacheSize;
    uint64 swappedOutSize;
    size_t currentUnlockTime;

    CS::Threading::Mutex swapMutex;

    void AccountEntrySize (SwapEntry* e)
    {
      if (e->lastSize == (size_t)~0)
      {
        e->lastSize = e->obj->GetSwapSize();
        currentCacheSize += e->lastSize;
      }
    }
  };
  
  /// Memory allocator for swappable data
  class SwappableHeap
  {
  public:
    static void* Alloc (const size_t n)
    {
      return globalLighter->swapManager->swapHeap.Alloc (n);
    }
    static void Free (void* p)
    {
      globalLighter->swapManager->swapHeap.Free (p);
    }
    static void* Realloc (void* p, size_t newSize)
    {
      return globalLighter->swapManager->swapHeap.Realloc (p, newSize);
    }
  };

  /**
   * Base implementation for a swappable object. Also supports nested
   * locking. 
   */
  class Swappable : public iSwappable
  {
    mutable CS::Threading::Mutex lockMutex;
    mutable int32 lockCount;
  public:
    Swappable() : lockCount (0)
    {
      globalLighter->swapManager->RegisterSwappable (this);
      // All swappables start off as unlocked!
    }
    Swappable(const Swappable& other) : lockCount (0)
    {
      globalLighter->swapManager->RegisterSwappable (this);
      // All swappables start off as unlocked!
    }
    virtual ~Swappable()
    {
      globalLighter->swapManager->UnregisterSwappable (this);
    }

    bool IsLocked () const
    { return CS::Threading::AtomicOperations::Read (&lockCount) != 0; }
    void Lock () const
    {
      CS::Threading::ScopedLock<CS::Threading::Mutex> swapLock (lockMutex);
      if (lockCount == 0)
        globalLighter->swapManager->Lock (
          const_cast<iSwappable*> ((iSwappable*)this));
      lockCount++;
    }
    void Unlock () const
    {
      CS::Threading::ScopedLock<CS::Threading::Mutex> swapLock (lockMutex);
      CS_ASSERT(lockCount > 0);
      lockCount--;
      if (lockCount == 0)
        globalLighter->swapManager->Unlock (
          const_cast<iSwappable*> ((iSwappable*)this));
    }
    void UpdateSizeInSwapManager () const
    {
      globalLighter->swapManager->UpdateSize (
          const_cast<iSwappable*> ((iSwappable*)this));
    }
  };

  /// Helper class to lock some data in a scope.
  template<typename T>
  class ScopedSwapLock 
  {
    const T& obj;
  public:
    ScopedSwapLock (const T& obj) : obj (obj) { obj.Lock(); }
    ~ScopedSwapLock () { obj.Unlock(); }
  };

  /// Array variant whose contents can be swapped out.
  template <class T,
	    class ElementHandler = csArrayElementHandler<T>,
            class CapacityHandler = csArrayCapacityDefault>
  class SwappableArray : 
    public csDirtyAccessArray<T, 
                              ElementHandler,
                              SwappableHeap,
                              CapacityHandler>,
    public Swappable
  {
  public:
    SwappableArray (size_t in_capacity = 0,
      const CapacityHandler& ch = CapacityHandler())
      : csDirtyAccessArray<T, ElementHandler, SwappableHeap, 
        CapacityHandler> (0, ch) 
    {
      if (in_capacity > 0)
      {
        Lock ();
        this->SetCapacity (in_capacity);
        Unlock ();
      }
    }
    SwappableArray (const SwappableArray& other) : 
      csDirtyAccessArray<T, ElementHandler, SwappableHeap, 
        CapacityHandler> (0, CapacityHandler()) 
    {
      Lock ();
      this->SetCapacity (other.GetSize());
      for (size_t i=0 ; i<other.GetSize () ; i++)
        Push (other[i]);
      Unlock ();
    }
    ~SwappableArray ()
    {
      // Force data being in memory
      globalLighter->swapManager->Lock (this);
    }

    virtual void GetSwapData (void*& data, size_t& size)
    {
      data = this->GetArray();
      size = this->Capacity() * sizeof (T);
      this->SetData (0);
    }
    virtual size_t GetSwapSize()
    {
      return this->Capacity() * sizeof (T);
    }
    virtual void SwapIn (void* data, size_t size)
    {
      CS_ASSERT (size == this->Capacity() * sizeof (T));
      this->SetData ((T*)data);
    }

  };

}

#endif // __SWAPPABLE_H__
