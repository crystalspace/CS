/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS_MEMORY_MAPPED_IO__
#define __CS_MEMORY_MAPPED_IO__

/**\file 
 * Platform-independent Memory-Mapped IO
 */

#include "csextern.h"
#include "bitarray.h"

struct iVFS;

/** \internal
 * Default cache block size
 */
const unsigned csmmioDefaultCacheBlockSize = 256;

/** \internal
 * Default cache size (in cache blocks)
 */
const unsigned csmmioDefaultCacheSize = 256;//2048;

/** \internal
 * Default size for hash table (best to use primes here)
 */
const unsigned csmmioDefaultHashSize = 211;//1559;


/**
  Defines a simple memory-mapped IO class that is portable.  Requires that
  data is organized in fixed block sizes.
  <p>
  Design notes:
  <p>
   1. Although the offset and page in the cache block can both be 
   calculated from either value, I chose to precalculate and store
   BOTH items.  The reason for this is that it avoids one 
   multiplication on each access, and one division when paging data
   in to cache.  For the default values data storage per cache is
   8192 bytes with 20 bytes additional overhead.  This means that
   0.2% of the data is overhead.  That's worth it to avoid a heavy
   op like multiplication
  <p>
   2. Usage of the singly-linked list for storage was chosen over a
   static array of cache blocks because there's no simple way to
   provide a direct index into the cache list from the index or page.
   Since we can probably guarantee that all blocks are NOT going to
   be in memory at once, we cannot index into the array based on a
   page index, etc.  A sorted list would probably provide the fastest
   lookup times, but would it be worth the overhead?  A hash table
   provides a number of short lists, which in this case are unsorted.
   Using QuickSort, lookup on a sorted table of 2048 entries would take
   about log2(2048) operations. With the default hash table size, each 
   list is about 1.5 entries long if spread uniformly.  This means that
   most lookups on a cache of this size should take between two and 
   four operations per (counting the modulus.)  While the hash table 
   does consume about 4k more memory, I think the slight memory usage 
   is worth it, considering the massive speedup.
 */  
class CS_CRYSTALSPACE_EXPORT csMemoryMappedIO
{
private:
  /// Minimum size of a single block 
  unsigned int block_size;
  
  /// Set to true if this object is valid
  bool valid_mmio_object;

  /// Size of a cache block in block_size blocks (software emulation)
  unsigned cache_block_size;

  /**
   * Size of in-memory cache in cache_block_size blocks, region is
   * noncontiguous (software emulation)
   */
  unsigned int cache_max_size;

  /// Number of cache blocks in cache
  unsigned int cache_block_count;
  
  /**
   * Array of bits where one bit = cache_block_size * block_size bytes.
   * A set bit indicates we have the page in memory.
   */
  csBitArray *page_map;

#ifdef CS_DEBUG

public:
  /// Cache hits
  unsigned int hits;

  /// Cache misses
  unsigned int misses;

private:
#endif

  /// Holds a contiguous array of cache blocks
  struct CacheBlock
  {    
    /// The age, used to decide when to release a cache block
    unsigned age;

    /// The starting offset for this page
    unsigned offset;

    /// The page number of this block
    unsigned page;
    
    /// The next item in our block list.
    CacheBlock *next;

    /// The data
    unsigned char *data;
  };

  /// Hash table for active blocks
  CacheBlock *cache[csmmioDefaultHashSize];

  // Software specific csMemMapInfo struct, should only be defined for
  // platforms w/o hardware mmio.
  struct emulatedMmioInfo 
  {          
    /// Handle to the mapped file 
    FILE *hMappedFile;

    /// Base pointer to the data
    unsigned char *data;

    /// File size
    unsigned int file_size;
  } emulatedPlatform;
  
#ifdef CS_HAVE_MEMORY_MAPPED_IO
  /// Holds information specific to the platform for hardware paging.
  csMemMapInfo platform;
  
  /// true if \c platform contains valid data.
  bool valid_platform;
#endif
public:
  /** 
   * Block size is the size of blocks that you want to get from the file,
   * filename is the name of the file to map. Indexes will be resolved to
   * absolute_index=index*block_size. If you supply a VFS,
   * \c filename is tried to be resolved to a native path. Otherwise,
   * \c filename is used as is, hence it must already be a native path.
   */
  csMemoryMappedIO(unsigned _block_size, char const *filename, iVFS* vfs = 0);

  /** 
   * Destroys the mmapio object, closes open files, and releases memory.
   */
  ~csMemoryMappedIO();

  /** 
   * This pointer will only be valid for a little while.  Read, at least until
   * the next call to GetPointer.
   * NEVER EVER EVER SAVE THIS POINTER.  You must recall this pointer when
   * you want access to the data again.
   */
  void *GetPointer(unsigned int index);
  
  /**
   * Returns true the memory was mapped successfully.
   */
  bool IsValid();

private:
  /// Reads a cache-page in from the disk.
  void CachePage(unsigned int page);

  /// Maps file into memory
  bool SoftMemoryMapFile(emulatedMmioInfo *platform, char const *filename);

  /// Unmaps file from memory
  void SoftUnMemoryMapFile(emulatedMmioInfo *platform);
};

#endif // __CS_MEMORY_MAPPED_IO__

