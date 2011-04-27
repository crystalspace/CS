/*
  Crystal Space Fixed Size Block Allocator
  Copyright (C) 2005 by Eric Sunshine <sunshine@sunshineco.com>
	    (C) 2006 by Frank Richter

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
#ifndef __CSUTIL_FIXEDSIZEALLOCATOR_H__
#define __CSUTIL_FIXEDSIZEALLOCATOR_H__

/**\file
 * Fixed Size Block Allocator
 */

#include "csextern.h"
#include "csutil/array.h"
#include "csutil/bitarray.h"
#include "csutil/sysfunc.h"

#ifdef CS_DEBUG
#include <typeinfo>
#endif

#if defined(CS_DEBUG) && !defined(CS_FIXEDSIZEALLOC_DEBUG)
#define _CS_FIXEDSIZEALLOC_DEBUG_DEFAULTED
#define CS_FIXEDSIZEALLOC_DEBUG
#endif

/**\addtogroup util_memory
 * @{ */

/**
 * This class implements a memory allocator which can efficiently allocate
 * objects that all have the same size. It has no memory overhead per
 * allocation (unless the objects are smaller than sizeof(void*) bytes) and is
 * extremely fast, both for Alloc() and Free(). The only restriction is that
 * any specific allocator can be used for just one type of object (the type for
 * which the template is instantiated).
 * 
 * \remarks Defining the macro CS_FIXEDSIZEALLOC_DEBUG will cause freed 
 *   objects to be overwritten with '0xfb' bytes. This can be useful to track 
 *   use of already freed objects, as they can be more easily recognized 
 *   (as some members will be likely bogus.)
 * \sa csArray
 * \sa csMemoryPool
 * \sa CS::Memory::FixedSizeAllocatorSafe for a thread-safe version
 */
template <size_t Size, class Allocator = CS::Memory::AllocatorMalloc>
class csFixedSizeAllocator
{
public:
  typedef csFixedSizeAllocator<Size, Allocator> ThisType;
  typedef Allocator AllocatorType;

protected: // 'protected' allows access by test-suite.
  struct FreeNode
  {
    FreeNode* next;
  };

  struct BlockKey
  {
    uint8 const* addr;
    size_t blocksize;
    BlockKey(uint8 const* p, size_t n) : addr(p), blocksize(n) {}
  };

  struct BlocksWrapper : public Allocator
  {
    csArray<uint8*> b;

    BlocksWrapper () {}
    BlocksWrapper (const Allocator& alloc) : Allocator (alloc) {}
  };
  /// List of allocated blocks; sorted by address.
  BlocksWrapper blocks;
  /// Number of elements per block.
  size_t elcount;
  /// Element size; >= sizeof(void*).
  size_t elsize;
  /// Size in bytes per block.
  size_t blocksize;
  /// Head of the chain of free nodes.
  FreeNode* freenode;
  /** 
   * Flag to ignore calls to Compact() and Free() if they're called recursively
   * while disposing the entire allocation set. Recursive calls to Alloc() will
   * signal an assertion failure.
   */
  bool insideDisposeAll;

  /**
   * Comparison function for FindBlock() which does a "fuzzy" search given an
   * arbitrary address.  It checks if the address falls somewhere within a
   * block rather than checking if the address exactly matches the start of the
   * block (which is the only information recorded in blocks[] array).
   */
  static int FuzzyCmp(uint8* const& block, BlockKey const& k)
  {
    return (block + k.blocksize <= k.addr ? -1 : (block > k.addr ? 1 : 0));
  }

  /**
   * Find the memory block which contains the given memory.
   */
  size_t FindBlock(void const* m) const
  {
    return blocks.b.FindSortedKey(
      csArrayCmp<uint8*,BlockKey>(BlockKey((uint8*)m, blocksize), FuzzyCmp));
  }

  /**
   * Allocate a block and initialize its free-node chain.
   * \return The returned address is both the reference to the overall block,
   *   and the address of the first free node in the chain.
   */
  uint8* AllocBlock()
  {
    uint8* block = (uint8*)blocks.Alloc (blocksize);

    // Build the free-node chain (all nodes are free in the new block).
    FreeNode* nextfree = 0;
    uint8* node = block + (elcount - 1) * elsize;
    for ( ; node >= block; node -= elsize)
    {
      FreeNode* slot = (FreeNode*)node;
      slot->next = nextfree;
      nextfree = slot;
    }
    CS_ASSERT((uint8*)nextfree == block);
    return block;
  }

  /**
   * Dispose of a block.
   */
  void FreeBlock(uint8* p)
  {
    blocks.Free (p);
  }

  /**
   * Destroy an object.
   */
  template<typename Disposer>
  void DestroyObject (Disposer& disposer, void* p) const
  {
    disposer.Dispose (p);
#ifdef CS_FIXEDSIZEALLOC_DEBUG
    memset (p, 0xfb, elsize);
#endif
  }

  /**
   * Get a usage mask showing all used (1's) and free (0's) nodes in
   * the entire allocator.
   */
  csBitArray GetAllocationMap() const
  {
    csBitArray mask(elcount * blocks.b.GetSize());
    mask.FlipAllBits();
    for (FreeNode* p = freenode; p != 0; p = p->next)
    {
      size_t const n = FindBlock(p);
      CS_ASSERT(n != csArrayItemNotFound);
      size_t const slot = ((uint8*)p - blocks.b[n]) / elsize; // Slot in block.
      mask.ClearBit(n * elcount + slot);
    }
    return mask;
  }

  /// Default disposer mixin, just reporting leaks.
  class DefaultDisposer
  {
  #ifdef CS_DEBUG
    bool doWarn;
    const char* parentClass;
    const void* parent;
    size_t count;
  #endif
  public:
  #ifdef CS_DEBUG
    template<typename BA>
    DefaultDisposer (const BA& ba, bool legit) :
      doWarn (!legit), parentClass (typeid(BA).name()), parent (&ba),
      count (0)
    { 
    }
  #else
    template<typename BA>
    DefaultDisposer (const BA&, bool legit)
    { (void)legit; }
  #endif
    ~DefaultDisposer()
    {
  #ifdef CS_DEBUG
      if ((count > 0) && doWarn)
      {
        csPrintfErr("%s %p leaked %zu objects.\n", parentClass, (void*)this, 
          count);
      }
  #endif
    }
    void Dispose (void* /*p*/) 
    {
  #ifdef CS_DEBUG
      count++;
  #endif
    }
  };
  /**
   * Destroys all living objects and releases all memory allocated by the pool.
   * \param disposer Object with a Dispose(void* p) method which is called prior
   *  to freeing the actual memory.
   */
  template<typename Disposer>
  void DisposeAll(Disposer& disposer)
  {
    insideDisposeAll = true;
    csBitArray const mask(GetAllocationMap());
    size_t node = 0;
    for (size_t b = 0, bN = blocks.b.GetSize(); b < bN; b++)
    {
      for (uint8 *p = blocks.b[b], *pN = p + blocksize; p < pN; p += elsize)
        if (mask.IsBitSet(node++))
          DestroyObject (disposer, p);
      FreeBlock(blocks.b[b]);
    }
    blocks.b.DeleteAll();
    freenode = 0;
    insideDisposeAll = false;
  }

  /**
   * Deallocate a chunk of memory. It is safe to provide a null pointer.
   * \param disposer Disposer object that is passed to DestroyObject().
   * \param p Pointer to deallocate.
   */
  template<typename Disposer>
  void Free (Disposer& disposer, void* p)
  {
    if (p != 0 && !insideDisposeAll)
    {
      CS_ASSERT(FindBlock(p) != csArrayItemNotFound);
      DestroyObject (disposer, p);
      FreeNode* f = (FreeNode*)p;
      f->next = freenode;
      freenode = f;
    }
  }
  /**
   * Try to delete a chunk of memory. Usage is the same as Free(), the 
   * difference being that \c false is returned if the deallocation failed 
   * (the reason is most likely that the memory was not allocated by the 
   * allocator).
   */
  template<typename Disposer>
  bool TryFree (Disposer& disposer, void* p)
  {
    if (p != 0 && !insideDisposeAll)
    {
      if (FindBlock(p) == csArrayItemNotFound) return false;
      DestroyObject (disposer, p);
      FreeNode* f = (FreeNode*)p;
      f->next = freenode;
      freenode = f;
    }
    return true;
  }
  /**
   * Free all objects without releasing the memory blocks themselves.
   * This works almost as DisposeAll but does not free the memory.
   * \param disposer Disposer object that is passed to DestroyObject().
   */
  template<typename Disposer>
  void FreeAll (Disposer& disposer)
  {
    insideDisposeAll = true;
    csBitArray const mask(GetAllocationMap());
    size_t node = 0;
    for (size_t b = 0, bN = blocks.b.GetSize(); b < bN; b++)
    {
      for (uint8 *p = blocks.b[b], *pN = p + blocksize; p < pN; p += elsize)
      {
        if (mask.IsBitSet(node++))
        {
          DestroyObject (disposer, p);
          FreeNode* f = (FreeNode*)p;
          f->next = freenode;
          freenode = f;          
        }
      }
    }
    insideDisposeAll = false;
  }

  /// Find and allocate a block
  void* AllocCommon ()
  {
    if (insideDisposeAll)
    {
      csPrintfErr("ERROR: csFixedSizeAllocator(%p) tried to allocate memory "
	"while inside DisposeAll()", (void*)this);
      CS_ASSERT(false);
    }

    if (freenode == 0)
    {
      uint8* p = AllocBlock();
      blocks.b.InsertSorted(p);
      freenode = (FreeNode*)p;
    }
    union
    {
      FreeNode* a;
      void* b;
    } pun;
    pun.a = freenode;
    freenode = freenode->next;
#ifdef CS_FIXEDSIZEALLOC_DEBUG
    memset (pun.b, 0xfa, elsize);
#endif
    return pun.b;
  }
private:
  void operator= (csFixedSizeAllocator const&); 	// Illegal; unimplemented.
public:
  //@{
  /**
   * Construct a new fixed size allocator.
   * \param nelem Number of elements to store in each allocation unit.
   * \remarks Bigger values for \c nelem will improve allocation performance,
   *   but at the cost of having some potential waste if you do not add
   *   \c nelem elements to each pool.  For instance, if \c nelem is 50 but you
   *   only add 3 elements to the pool, then the space for the remaining 47
   *   elements, though allocated, will remain unused (until you add more
   *   elements).
   */
  csFixedSizeAllocator (size_t nelem = 32) :
    elcount (nelem), elsize(Size), freenode(0), insideDisposeAll(false)
  {
#ifdef CS_MEMORY_TRACKER
    blocks.SetMemTrackerInfo (typeid(*this).name());
#endif
    if (elsize < sizeof (FreeNode))
      elsize = sizeof (FreeNode);
    blocksize = elsize * elcount;
  }
  csFixedSizeAllocator (size_t nelem, const Allocator& alloc) : blocks (alloc),
    elcount (nelem), elsize(Size), freenode(0), insideDisposeAll(false)
  {
#ifdef CS_MEMORY_TRACKER
    blocks.SetMemTrackerInfo (typeid(*this).name());
#endif
    if (elsize < sizeof (FreeNode))
      elsize = sizeof (FreeNode);
    blocksize = elsize * elcount;
  }
  //@}
  
  /**
   * Construct a new fixed size allocator, copying the amounts of elements to
   * store in an allocation unit.
   * \remarks Copy-constructing an allocator is only valid if the allocator
   *   copied from is not empty. Attempting to copy a non-empty allocator will
   *   cause an assertion to fail at runtime!
   */
  csFixedSizeAllocator (csFixedSizeAllocator const& other) : 
    elcount (other.elcount), elsize (other.elsize), 
    blocksize (other.blocksize), freenode (0), insideDisposeAll (false)
  {
#ifdef CS_MEMORY_TRACKER
    blocks.SetMemTrackerInfo (typeid(*this).name());
#endif
    /* Technically, an allocator can be empty even with freenode != 0 */
    CS_ASSERT(other.freenode == 0);
  }
  
  /**
   * Destroy all allocated objects and release memory.
   */
  ~csFixedSizeAllocator()
  {
    DefaultDisposer disposer (*this, false);
    DisposeAll (disposer);
  }

  /**
   * Destroy all chunks allocated.
   * \remarks All pointers returned by Alloc() are invalidated. It is safe to
   *   perform new allocations from the pool after invoking Empty().
   */
  void Empty()
  {
    DefaultDisposer disposer (*this, true);
    DisposeAll (disposer);
  }

  /**
   * Compact the allocator so that all blocks that are completely unused
   * are removed. The blocks that still contain elements are not touched.
   */
  void Compact()
  {
    if (insideDisposeAll) return;

    bool compacted = false;
    csBitArray mask(GetAllocationMap());
    for (size_t b = blocks.b.GetSize(); b-- > 0; )
    {
      size_t const node = b * elcount;
      if (!mask.AreSomeBitsSet(node, elcount))
      {
        FreeBlock(blocks.b[b]);
        blocks.b.DeleteIndex(b);
        mask.Delete(node, elcount);
        compacted = true;
      }
    }

    // If blocks were deleted, then free-node chain broke, so rebuild it.
    if (compacted)
    {
      FreeNode* nextfree = 0;
      size_t const bN = blocks.b.GetSize();
      size_t node = bN * elcount;
      for (size_t b = bN; b-- > 0; )
      {
        uint8* const p0 = blocks.b[b];
        for (uint8* p = p0 + (elcount - 1) * elsize; p >= p0; p -= elsize)
        {
          if (!mask.IsBitSet(--node))
          {
            FreeNode* slot = (FreeNode*)p;
            slot->next = nextfree;
            nextfree = slot;
          }
        }
      }
      freenode = nextfree;
    }
  }
  
  /**
   * Return number of allocated elements (potentially slow).
   */
  size_t GetAllocatedElems() const
  {
    csBitArray mask(GetAllocationMap());
    return mask.NumBitsSet();
  }

  /**
   * Allocate a chunk of memory. 
   */
  void* Alloc ()
  {
    return AllocCommon();
  }

  /**
   * Deallocate a chunk of memory. It is safe to provide a null pointer.
   * \param p Pointer to deallocate.
   */
  void Free (void* p)
  {
    DefaultDisposer disposer (*this, true);
    Free (disposer, p);
  }
  /**
   * Try to delete a chunk of memory. Usage is the same as Free(), the 
   * difference being that \c false is returned if the deallocation failed 
   * (the reason is most likely that the memory was not allocated by the 
   * allocator).
   */
  bool TryFree (void* p)
  {
    DefaultDisposer disposer (*this, true);
    return TryFree (disposer, p);
  }
  /// Query number of elements per block.
  size_t GetBlockElements() const { return elcount; }
  
  /**\name Functions for useability as a allocator template parameter
   * @{ */
  void* Alloc (size_t n)
  {
    CS_ASSERT (n == Size);
    return Alloc();
  }
  void* Alloc (void* p, size_t newSize)
  {
    CS_ASSERT (newSize == Size);
    return p;
  }
  void SetMemTrackerInfo (const char* /*info*/) { }
  /** @} */
};

namespace CS
{
  namespace Memory
  {
    /**
     * Thread-safe allocator for blocks of the same size.
     * Has the same purpose and interface as csFixedSizeAllocator but is safe
     * to be used concurrently from different threads.
     */
    template <size_t Size, class Allocator = CS::Memory::AllocatorMalloc>
    class FixedSizeAllocatorSafe :
      public CS::Memory::AllocatorSafe<csFixedSizeAllocator<Size, Allocator> >
    {
    protected:
      typedef csFixedSizeAllocator<Size, Allocator> WrappedAllocatorType;
      typedef CS::Memory::AllocatorSafe<csFixedSizeAllocator<Size, Allocator> >
        AllocatorSafeType;
    public:
      FixedSizeAllocatorSafe (size_t nelem = 32) : AllocatorSafeType (nelem)
      {
      }
      FixedSizeAllocatorSafe (size_t nelem, const Allocator& alloc) :
        AllocatorSafeType (nelem, alloc)
      {
      }
      
      FixedSizeAllocatorSafe (FixedSizeAllocatorSafe const& other) : 
	AllocatorSafeType (other)
      {
      }
      
      void Empty()
      {
        CS::Threading::RecursiveMutexScopedLock lock (AllocatorSafeType::mutex);
        WrappedAllocatorType::Empty();
      }
    
      void Compact()
      {
        CS::Threading::RecursiveMutexScopedLock lock (AllocatorSafeType::mutex);
        WrappedAllocatorType::Compact();
      }
      
      size_t GetAllocatedElems() const
      {
        CS::Threading::RecursiveMutexScopedLock lock (AllocatorSafeType::mutex);
        return WrappedAllocatorType::GetAllocatedElems();
      }
    
      void* Alloc ()
      {
        CS::Threading::RecursiveMutexScopedLock lock (AllocatorSafeType::mutex);
        return WrappedAllocatorType::Alloc();
      }
      using AllocatorSafeType::Alloc;
    
      bool TryFree (void* p)
      {
        CS::Threading::RecursiveMutexScopedLock lock (AllocatorSafeType::mutex);
        return WrappedAllocatorType::TryFree (p);
      }
      size_t GetBlockElements() const
      {
        CS::Threading::RecursiveMutexScopedLock lock (AllocatorSafeType::mutex);
        return WrappedAllocatorType::GetBlockElements();
      }
    };
  } // namespace Memory
} // namespace CS

/** @} */

#ifdef _CS_FIXEDSIZEALLOC_DEBUG_DEFAULTED
#undef CS_FIXEDSIZEALLOC_DEBUG
#undef _CS_FIXEDSIZEALLOC_DEBUG_DEFAULTED
#endif

#endif // __CSUTIL_FIXEDSIZEALLOCATOR_H__
