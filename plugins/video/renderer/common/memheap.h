/*
    Copyright (C) 1998 by Olivier Langlois <olanglois@sympatico.ca>

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

#ifndef   _MEMHEAP_H_
#define   _MEMHEAP_H_

#include <stddef.h> // For size_t

/// The default texture cache size.
#define DEFAULT_CACHE_SIZE	5*1024*1024

/*
 * Structures
 */

struct s_bufhd		// Buffer header
{
  ULong size;		// Size of the buffer + its header
  ULong full;		// ULong have been used instead of a bool to
			// ensure that sizeof(bufhd) will have the same value
			// with different compilers.
};

struct s_heap		// Heap header
{
  ULong start;		// A pointer on the first buffer header
  ULong first_free;	// A pointer on the first free buffer
  ULong end;		// A pointer on the end of the heap
};

typedef struct s_bufhd bufhd;
typedef struct s_heap  heap;

/**
 * This class is intended to manage a memory area and allocate buffer
 * space from that area.<p>
 *
 * The buffers are allocated on a first fit basis. To maximize the use
 * of the first memory pages, the list is always scanned in full,
 * merging adjacent empty buffers if found.<p>
 *
 * If merging buffers fails to allocate sufficient area, then compaction
 * is not tried. This makes it possible to use the buffer pointers directly
 * by the application program.
 */
class MemoryHeap
{
public:
  /// s: Size of Memory heap
  MemoryHeap (size_t s  = DEFAULT_CACHE_SIZE);
  ///
  ~MemoryHeap();

  ///
  char *alloc( size_t s );

  /**
   * Modify the size of a currently allocated buffer to a new size, either by extending
   * the current buffer or moving it to a new location inside the heap.<p>
   *
   * buf: Currently allocated buffer<dt>
   * ns : New requested size
   */
  char *realloc( void *buf, size_t ns );

  /**
   * To free an allocated buffer in heap<p>
   *
   * Return value: Success(0), Failure(-1)
   */
  int free( void *buf );

private:
  /// Address of the memory buffer
  char *memory;
  size_t cache_size;

  /*
   * dump_pool() is outside the MemoryHeap class in order to
   * decouple system.h from this class.
   */
  friend void dump_pool( MemoryHeap * );
};

#endif /* _MEMHEAP_H_ */
