/*
  Crystal Space Generic Memory Block Allocator
  Copyright (C) 2003 by Jorrit Tyberghein

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
#ifndef __CSUTIL_BLKALLOC_H__
#define __CSUTIL_BLKALLOC_H__

/**\file
 * Generic Memory Block Allocator
 */

#include "csextern.h"
#include "array.h"

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
 * allocation (unless the objects are smaller than 8 bytes) and is extremely
 * fast, both for Alloc() and Free(). Only restriction is it can only be used
 * for the same type of object.
 * \remarks The objects are properly constructed and destructed.
 * <p>
 * \remarks Defining the macro CS_BLOCKALLOC_DEBUG will cause freed objects to
 *   be overwritten with '0xfb' bytes. This can be useful to track use of
 *   already freed objects, as they can be more easily recognized (as some
 *   members will be likely bogus.)
 * \warning This class does VERY little error checking!
 * \sa csArray
 * \sa csMemoryPool
 */
template <class T>
class csBlockAllocator
{
private:
  // A dummy structure for a linked list of free items.
  struct csFreeList
  {
    csFreeList* next;
    size_t numfree;		// Free elements in this block.
  };

  // A memory block (a series of 'size' objects).
  struct csBlock
  {
    void* memory;
    csFreeList* firstfree;	// Linked list of free items in this block.
    csBlock () : memory (0), firstfree (0) {}
    ~csBlock ()
    {
      if (memory)
      {
#	ifdef CS_MEMORY_TRACKER
	int32* ptr = ((int32*)memory)-2;
	mtiRegisterFree ((MemTrackerInfo*)*ptr, (size_t)ptr[1]);
	free (ptr);
#	else
        free (memory);
#	endif
      }
    }
  };

  csArray<csBlock> blocks;
  size_t size;			// Number of elements per block.
  size_t elsize;		// Element size (bigger than 8).
  size_t blocksize;		// Size in bytes per block.

  // First block that contains a free element.
  size_t firstfreeblock;

  /**
   * Find the memory block which contains the given memory.
   */
  size_t FindBlock (void* m)
  {
    size_t i;
    for (i = 0 ; i < blocks.Length () ; i++)
    {
      csBlock* b = &blocks[i];
      if (b->memory <= m)
      {
        char* eb = ((char*)b->memory) + blocksize;
	if (((char*)m) < eb)
	  return i;
      }
    }
    return (size_t)-1;
  }

  /**
   * If the current free block is full, find a new one. If needed
   * create a new one.
   */
  void FindAndUpdateFreeBlock (size_t potentialnextfree)
  {
    CS_ASSERT (blocks.Length() != 0);
    firstfreeblock = potentialnextfree;
    while (firstfreeblock < blocks.Length ()
		&& blocks[firstfreeblock].firstfree == 0)
      ++firstfreeblock;

    if (firstfreeblock == blocks.Length ())
    {
      firstfreeblock = blocks.Push (csBlock ());
      csBlock& bl = blocks[firstfreeblock];
#     ifdef CS_MEMORY_TRACKER
      char buf[255];
      sprintf (buf, "csBlockAllocator<%s>", typeid (T).name());
      int32* ptr = (int32*)malloc (blocksize + sizeof (int32)*2);
      *ptr++ = (int32)mtiRegisterAlloc (blocksize, buf);
      *ptr++ = blocksize;
      bl.memory = (void*)ptr;
#     else
      bl.memory = (void*)malloc (blocksize);
#     endif
      bl.firstfree = (csFreeList*)bl.memory;
      bl.firstfree->next = 0;
      bl.firstfree->numfree = size;
    }
  }

  /**
   * Destroys all living objects and releases all memory allocated by the pool.
   * \remarks Unconditionally disposes of all memory. Does NOT maintain the one
   *   pre-allocated page expected by Alloc() and other methods, thus after
   *   invocation of this method, it is not safe to invoke any other methods
   *   which expect at least one block to be present.
   */
  void DisposeAll()
  {
    for (size_t i = blocks.Length(); i-- > 0; )
    {
      csBlock const& bl = blocks[i];
      uint8* el = (uint8*)bl.memory;
      uint8 const* blockend = el + blocksize;
      uint8 const* nextfree = (uint8*)bl.firstfree;
      CS_ASSERT(nextfree == 0 || (nextfree >= el && nextfree < blockend));
      for ( ; el < blockend; el += elsize)
      {
	if (nextfree == 0 || el < nextfree)
	  ((T*)el)->~T();
	else if (nextfree != 0)
	  nextfree = (uint8*)((csFreeList*)nextfree)->next;
      }
    }
    blocks.Empty();
    firstfreeblock = (size_t)-1;
  }

public:
  /**
   * Construct a new block allocator which uses \c nelem elements per
   * block. Bigger values for \c nelem will improve allocation performance, but
   * at the cost of having some potential waste if you do not add \c nelem
   * elements to the pool.  For instance, if \c nelem is 50 but you only add 3
   * elements to the pool, then the space for the remaining 47 elements, though
   * allocated, will remain unused (until you add more elements).
   */
  csBlockAllocator (size_t nelem = 32)
  {
    size = nelem;
    elsize = sizeof (T);
    if (elsize < sizeof (csFreeList))
      elsize = sizeof (csFreeList);
    blocksize = elsize * size;

    size_t idx = blocks.Push (csBlock ());
    csBlock& bl = blocks[idx];
#   ifdef CS_MEMORY_TRACKER
    char buf[255];
    sprintf (buf, "csBlockAllocator<%s>", typeid (T).name());
    int32* ptr = (int32*)malloc (blocksize + sizeof (int32)*2);
    *ptr++ = (int32)mtiRegisterAlloc (blocksize, buf);
    *ptr++ = blocksize;
    bl.memory = (void*)ptr;
#   else
    bl.memory = (void*)malloc (blocksize);
#   endif
    bl.firstfree = (csFreeList*)bl.memory;
    bl.firstfree->next = 0;
    bl.firstfree->numfree = size;

    firstfreeblock = 0;
  }

  /**
   * Destroy all allocated objects and release memory.
   */
  ~csBlockAllocator ()
  {
    DisposeAll();
  }

  /**
   * Destroy all objects allocated by the pool.
   * \remarks All pointers returned by Alloc() are invalidated. It is safe to
   *   perform new allocations from the pool after invoking Empty().
   */
  void Empty()
  {
    DisposeAll();
    FindAndUpdateFreeBlock(0);
  }

  /**
   * Compact the block allocator so that all blocks that are completely
   * free are removed. The blocks that still contain elements are not touched.
   */
  void Compact ()
  {
    CS_ASSERT (blocks.Length() != 0);
    size_t i = blocks.Length () - 1;
    while (i > firstfreeblock)
    {
      if (blocks[i].firstfree == (csFreeList*)blocks[i].memory 
	&& blocks[i].firstfree->numfree == size)
      {
	CS_ASSERT (blocks[i].firstfree->next == 0);
        blocks.DeleteIndex (i);
      }
      i--;
    }
  }

  /**
   * Allocate a new element.
   */
  T* Alloc ()
  {
    CS_ASSERT (blocks.Length() != 0);

    // This routine makes sure there is ALWAYS free space available.
    csBlock& freebl = blocks[firstfreeblock];
    void* ptr = (void*)(freebl.firstfree);

    if (freebl.firstfree->numfree >= 2)
    {
      // Still room in this block after allocation.
      csFreeList* nf = (csFreeList*)(((char*)ptr)+elsize);
      nf->next = freebl.firstfree->next;
      nf->numfree = freebl.firstfree->numfree-1;
      freebl.firstfree = nf;
    }
    else
    {
      // We need a new block later.
      freebl.firstfree = freebl.firstfree->next;
      if (!freebl.firstfree)
      {
        // This block has no more free space. We need a new one.
	FindAndUpdateFreeBlock (firstfreeblock + 1);
      }
    }

    return new (ptr) T;
  }

  /**
   * Deallocate an element. It is ok to give a 0 pointer here.
   */
  void Free (T* el)
  {
    if (!el) return;

    size_t idx = FindBlock ((void*)el);
    CS_ASSERT_MSG (
      "csBlockAllocator<>::Free() called with invalid element address",
      idx != (size_t)-1);

    el->~T();

#ifdef CS_BLOCKALLOC_DEBUG
    memset (el, 0xfb, elsize);
#endif

    // If the index is lower than the index of the first free block
    // then we update the firstfreeblock index.
    if (idx < firstfreeblock)
      firstfreeblock = idx;

    csBlock& bl = blocks[idx];
    if (bl.firstfree == 0)
    {
      // Block has no free items so we create the first free item
      // here.
      bl.firstfree = (csFreeList*)el;
      bl.firstfree->next = 0;
      bl.firstfree->numfree = 1;
    }
    else
    {
      csFreeList* p_el = (csFreeList*)el;

      if (p_el < bl.firstfree)
      {
        // New free element is before the current free element.
	if (((char*)bl.firstfree) - ((char*)p_el) == (int)elsize)
	{
	  // New free element is directly before the current free
	  // element.
	  p_el->next = bl.firstfree->next;
	  p_el->numfree = bl.firstfree->numfree+1;
	}
	else
	{
	  // There is a gap.
	  p_el->next = bl.firstfree;
	  p_el->numfree = 1;
	}

	bl.firstfree = p_el;
      }
      else
      {
        // New free element is after the current free element.
	// First we find the two free blocks that enclose the new
	// free element.
	csFreeList* fl_before = bl.firstfree;
	csFreeList* fl_after = bl.firstfree->next;
	while (fl_after != 0 && fl_after < p_el)
	{
	  fl_before = fl_after;
	  fl_after = fl_after->next;
	}

	// 'fl_before_end' points to the memory right after the free block
	// that is directly before the new free element. We use
	// 'fl_before_end' later.
	char* fl_before_end = ((char*)fl_before) + fl_before->numfree * elsize;

	if (!fl_after)
	{
	  // The new free element is after the last free block. First check
	  // if the new free element directly follows that free block.
	  // If so we can extend that.
	  if (fl_before_end == (char*)p_el)
	  {
	    // Yes, extend.
	    ++(fl_before->numfree);
	  }
	  else
	  {
	    // No, we have to create a new free block.
	    p_el->next = 0;
	    p_el->numfree = 1;
	    fl_before->next = p_el;
	  }
	}
	else
	{
	  // We have a block before and after the new free block.
	  // First we check if the new free item exactly between
	  // fl_before and fl_after.
	  if (fl_before_end == (char*)p_el
	  	&& ((char*)p_el) + elsize == (char*)fl_after)
	  {
	    // Perfect fit, merge fl_before and fl_after.
	    fl_before->next = fl_after->next;
	    fl_before->numfree = fl_before->numfree + 1 + fl_after->numfree;
	  }
	  else if (fl_before_end == (char*)p_el)
	  {
	    // New free item fits directly after fl_before.
	    ++(fl_before->numfree);
	  }
	  else if (((char*)p_el) + elsize == (char*)fl_after)
	  {
	    // New free item fits directly before fl_after.
	    fl_before->next = p_el;
	    p_el->next = fl_after->next;
	    p_el->numfree = fl_after->numfree+1;
	  }
	  else
	  {
	    // We need a new free block.
	    fl_before->next = p_el;
	    p_el->next = fl_after;
	    p_el->numfree = 1;
	  }
	}
      }
    }
  }

  /**
   * For debugging: dump contents.
   */
  void Dump ()
  {
    int i;
    printf ("=============================\nelsize = %d\n", elsize);
    for (i = 0 ; i < blocks.Length () ; i++)
    {
      printf ("Block %d\n", i);
      csFreeList* fl = blocks[i].firstfree;
      char* m = (char*)blocks[i].memory;
      while (fl)
      {
        printf ("  free %d %d\n", (((char*)fl) - m) / elsize, fl->numfree);
	fl = fl->next;
      }
    }
    printf ("=============================\n");
    fflush (stdout);
  }
};

#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# define new CS_EXTENSIVE_MEMDEBUG_NEW
#endif

#endif
