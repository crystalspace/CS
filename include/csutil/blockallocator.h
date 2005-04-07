/*
  Crystal Space Generic Object Block Allocator
  Copyright (C)2005 by Eric sunshine <sunshine@sunshineco.com>

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
#ifndef __CSUTIL_BLOCK_ALLOCATOR_H__
#define __CSUTIL_BLOCK_ALLOCATOR_H__

/**\file
 * Generic Memory Block Allocator
 */

#include "csextern.h"
#include "array.h"
#include "bitarray.h"
#include "sysfunc.h"

// hack: work around problems caused by #defining 'new'
#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# undef new
#endif
#include <new>

#ifdef CS_MEMORY_TRACKER
#include "csutil/memdebug.h"
#include <typeinfo>
#endif

/**
 * This class implements a memory allocator which can efficiently allocate
 * objects that all have the same size. It has no memory overhead per
 * allocation (unless the objects are smaller than sizeof(void*) bytes) and is
 * extremely fast, both for Alloc() and Free(). The only restriction is that
 * any specific allocator can be used for just one type of object (the type for
 * which the template is instantiated).
 * \remarks The objects are properly constructed and destructed.
 * <p>
 * \remarks Assumes that the class \c T with which the template is instantiated
 *   has a default (zero-argument) constructor. Alloc() uses this constructor
 *   to initialize each vended object.
 * <p>
 * \remarks Defining the macro CS_BLOCKALLOC_DEBUG will cause freed objects to
 *   be overwritten with '0xfb' bytes. This can be useful to track use of
 *   already freed objects, as they can be more easily recognized (as some
 *   members will be likely bogus.)
 * \sa csArray
 * \sa csMemoryPool
 */
template <class T>
class csBlockAllocator
{
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

  csArray<uint8*> blocks;	// List of allocated blocks; sorted by address.
  size_t size;			// Number of elements per block.
  size_t elsize;		// Element size; >= sizeof(void*).
  size_t blocksize;		// Size in bytes per block.
  FreeNode* freenode;		// Head of the chain of free nodes.
  bool pedantic;		// Warn about nodes not explicitly freed.

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
    return blocks.FindSortedKey(
      csArrayCmp<uint8*,BlockKey>(BlockKey((uint8*)m, blocksize), FuzzyCmp));
  }

  /**
   * Allocate a block and initialize its free-node chain.
   * \return The returned address is both the reference to the overall block,
   *   and the address of the first free node in the chain.
   */
  uint8* AllocBlock() const
  {
    uint8* block;
#ifdef CS_MEMORY_TRACKER
    char buf[255];
    sprintf (buf, "csBlockAllocator<%s>", typeid (T).name());
    int32* ptr = (int32*)malloc (blocksize + sizeof (int32)*2);
    *ptr++ = (int32)mtiRegisterAlloc (blocksize, buf);
    *ptr++ = blocksize;
    block = (uint8*)ptr;
#else
    block = (uint8*)malloc(blocksize);
#endif

    // Build the free-node chain (all nodes are free in the new block).
    FreeNode* nextfree = 0;
    uint8* node = block + (size - 1) * elsize;
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
  void FreeBlock(uint8* p) const
  {
#ifdef CS_MEMORY_TRACKER
    int32* ptr = ((int32*)p)-2;
    mtiRegisterFree ((csMemTrackerInfo*)*ptr, (size_t)ptr[1]);
    free (ptr);
#else
    free (p);
#endif
  }

  /**
   * Destroy an object, optionally warning if pedanticism is desired.
   */
  void DestroyObject(T* p, bool warn = false) const
  {
    p->~T();
#ifdef CS_DEBUG
    if (warn)
      csPrintfErr("NOTFIY: csBlockAllocator(%p) destroying potentially leaked "
		  "object at %p.\n", this, p);
#endif
#ifdef CS_BLOCKALLOC_DEBUG
    memset (p, 0xfb, elsize);
#endif
  }

  /**
   * Get a usage mask showing all used (1's) and free (0's) nodes in
   * the entire allocator.
   */
  csBitArray GetAllocationMap() const
  {
    csBitArray mask(size * blocks.GetSize());
    mask.FlipAllBits();
    for (FreeNode* p = freenode; p != 0; p = p->next)
    {
      size_t const n = FindBlock(p);
      CS_ASSERT(n != csArrayItemNotFound);
      size_t const slot = ((uint8*)p - blocks[n]) / elsize; // Slot in block.
      mask.ClearBit(n * size + slot);
    }
    return mask;
  }

  /**
   * Destroys all living objects and releases all memory allocated by the pool.
   * \param warn_unfreed If true, in debug mode warn about objects not
   *   explicitly released via Free().
   */
  void DisposeAll(bool warn_unfreed)
  {
    csBitArray const mask(GetAllocationMap());
    size_t node = 0;
    for (size_t b = 0, bN = blocks.GetSize(); b < bN; b++)
    {
      for (uint8 *p = blocks[b], *pN = p + blocksize; p < pN; p += elsize)
	if (mask.IsBitSet(node++))
	  DestroyObject((T*)p, warn_unfreed);
      FreeBlock(blocks[b]);
    }
    blocks.DeleteAll();
    freenode = 0;
  }

public:
  /**
   * Construct a new block allocator.
   * \param nelem Number of elements to store in each allocation unit.
   * \param warn_unfreed If true, in debug mode warn about objects not
   *   explicitly released via Free().
   * \remarks Bigger values for \c nelem will improve allocation performance,
   *   but at the cost of having some potential waste if you do not add
   *   \c nelem elements to each pool.  For instance, if \c nelem is 50 but you
   *   only add 3 elements to the pool, then the space for the remaining 47
   *   elements, though allocated, will remain unused (until you add more
   *   elements).
   * <p>
   * \remarks If use use csBlockAllocator as a convenient and lightweight
   *   garbage collection facility (for which it is well-suited), and expect it
   *   to dispose of allocated objects when the pool itself is destroyed, then
   *   set \c warn_unfreed to false. On the other hand, if you use
   *   csBlockAllocator only as a fast allocator but intend to manage each
   *   object's life time manually, then you may want to set \c warn_unfreed to
   *   true in order to receive diagnostics about objects which you have
   *   forgotten to release explicitly via manual invocation of Free().
   */
  csBlockAllocator(size_t nelem = 32, bool warn_unfreed = false) :
    size(nelem), elsize(sizeof(T)), freenode(0), pedantic(warn_unfreed)
  {
    if (elsize < sizeof (FreeNode))
      elsize = sizeof (FreeNode);
    blocksize = elsize * size;
  }

  /**
   * Destroy all allocated objects and release memory.
   */
  ~csBlockAllocator()
  {
    DisposeAll(pedantic);
  }

  /**
   * Destroy all objects allocated by the pool.
   * \remarks All pointers returned by Alloc() are invalidated. It is safe to
   *   perform new allocations from the pool after invoking Empty().
   */
  void Empty()
  {
    DisposeAll(false);
  }

  /**
   * Compact the block allocator so that all blocks that are completely unused
   * are removed. The blocks that still contain elements are not touched.
   */
  void Compact()
  {
    bool compacted = false;
    csBitArray mask(GetAllocationMap());
    for (size_t b = blocks.GetSize(); b-- > 0; )
    {
      size_t const node = b * size;
      if (!mask.AreSomeBitsSet(node, size))
      {
	FreeBlock(blocks[b]);
	blocks.DeleteIndex(b);
	mask.Delete(node, size);
	compacted = true;
      }
    }

    // If blocks were deleted, then free-node chain broke, so rebuild it.
    if (compacted)
    {
      FreeNode* nextfree = 0;
      size_t const bN = blocks.GetSize();
      size_t node = bN * size;
      for (size_t b = bN; b-- > 0; )
      {
	uint8* const p0 = blocks[b];
	for (uint8* p = p0 + (size - 1) * elsize; p >= p0; p -= elsize)
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
   * Allocate a new object. Its default (no-argument) constructor is invoked.
   */
  T* Alloc()
  {
    if (freenode == 0)
    {
      uint8* p = AllocBlock();
      blocks.InsertSorted(p);
      freenode = (FreeNode*)p;
    }
    void* const node = freenode;
    freenode = freenode->next;
    return new (node) T;
  }

  /**
   * Deallocate an object. It is safe to provide a null pointer.
   */
  void Free(T* p)
  {
    if (p != 0)
    {
      CS_ASSERT(FindBlock(p) != csArrayItemNotFound);
      DestroyObject(p, false);
      FreeNode* f = (FreeNode*)p;
      f->next = freenode;
      freenode = f;
    }
  }
};

#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# define new CS_EXTENSIVE_MEMDEBUG_NEW
#endif

#endif // __CSUTIL_BLOCK_ALLOCATOR_H__
