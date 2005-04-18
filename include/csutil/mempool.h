/*
    Copyright (C) 2005 by Eric Sunshine <sunshine@sunshineco.com>

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

#ifndef __CS_CSUTIL_MEMPOOL_H__
#define __CS_CSUTIL_MEMPOOL_H__

/**\file
 * Generic Memory Allocator
 */

#include "cssysdef.h"
#include "csextern.h"
#include "csutil/parray.h"

// Hack: Work around problems caused by #defining 'new'.
#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# undef new
#endif
#include <new>

/**
 * A quick-allocation pool for storage of arbitrary data. Pointers to
 * allocations made from the pool are guaranteed to remain valid as long as the
 * pool is alive; the pool contents are never relocated.  All individually
 * allocated memory chunks are freed when the pool itself is destroyed.  This
 * memory management scheme is suitable for algorithms which need to allocate
 * and manipulate many chunks of memory in a non-linear fashion where the
 * life-time of each memory chunk is not predictable. Rather than complicating
 * the algorithm by having it carefully track each memory chunk to determine
 * when it would be safe to dispose of the memory, it can instead create a
 * csMemoryPool at the start, and destroy the pool at the end.  During
 * processing, it can allocate memory chunks from the pool as needed, and
 * simply forget about them when no longer needed, knowing that they will be
 * freed en-masse when the pool itself is disposed.  This is often a cheaper,
 * simpler, and faster alternative to reference-counting or automatic garbage
 * collection.
 * \sa csBlockAllocator
 * \sa csArray
 */
class CS_CRYSTALSPACE_EXPORT csMemoryPool
{
private:
  /// Array of pointers to heap-allocated blocks.
  csPDelArray<uint8> blocks;
  /// Number of unallocated bytes remaining in most recently allocated block.
  size_t remaining;
  /// Number of bytes to allocate for each block.
  size_t granularity;

  csMemoryPool(csMemoryPool const&);   // Illegal; unimplemented.
  void operator=(csMemoryPool const&); // Illegal; unimplemented.

public:
  /**
   * Construct a new memory pool. If a size is provided, it is taken as a
   * recommendation in bytes of the granularity of the internal allocations
   * made by the pool, but is not a hard limit. Client allocations from the
   * pool can be both smaller and larger than this number.  A larger number
   * will result in fewer interactions with the system heap (which translates
   * to better performance), but at the cost of potential unused but allocated
   * space. A smaller number translates to a greater number of interactions
   * with the system heap (which is slow), but means less potential wasted
   * memory.
   */
  csMemoryPool(size_t gran = 4096) : remaining(0), granularity(gran) {}
  /// Destroy the memory pool, freeing all allocated storage.
  ~csMemoryPool() { Empty(); }

  /**
   * Allocate the specified number of bytes.
   * \return A pointer to the allocated memory.
   * \remarks The allocated space is not initialized in any way (it is not even
   *   zeroed); the caller is responsible for populating the allocated space.
   * <p>
   * \remarks The specified size must be greater than zero.
   */
  void* Alloc(size_t);
  /**
   * Release all memory allocated by the pool.
   * \remarks All pointers returned by Alloc() and Store() are invalidated. It
   *   is safe to perform new allocations from the pool after invoking Empty().
   */
  void Empty();
  /**
   * Store a copy of a block of memory of the indicated size.
   * \return A pointer to the stored copy.
   * \remarks The specified size must be greater than zero.
   */
  void const* Store(void const*, size_t);
  /**
   * Store a null-terminated C-string.
   * \return A pointer to the stored copy.
   * \remarks It is safe to store a zero-length string. A null pointer is
   *   treated like a zero-length string.
   */
  char const* Store(char const*);
};

/** @{ */
/**
 * Convenience \c new operator which makes the allocation from a csMemoryPool
 * rather than from the system heap. For instance, if \c pool is a pointer or
 * reference to a csMemoryPool, and you want to allocate an object of type
 * FooBar from the csMemoryPool, then you can do this:
 * \code
 * FooBar* foobar = new (pool) FooBar;
 * \endcode
 * It is your responsibility to invoke the destructor of the objects you
 * allocate from the csMemoryPool before the csMemoryPool itself is destroyed
 * (since it knows nothing about the objects which you placed into it). For
 * instance, continuing the above example, before destroying \c pool, you
 * should invoke \c foobar's destructor as follows:
 * \code
 * foobar->~FooBar();
 * // ... it is now safe to destroy `pool' ...
 * \endcode
 */
static inline void* operator new(size_t n, csMemoryPool& p)
{ return p.Alloc(n); }
static inline void* operator new(size_t n, csMemoryPool* p)
{ return p->Alloc(n); }
/** @} */

#endif //  __CS_CSUTIL_MEMPOOL_H__
