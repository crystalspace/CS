#ifndef __CS_MEMORY_MAPPED_IO__
#define __CS_MEMORY_MAPPED_IO__

#include "csutil/bitarray.h"

/*******************************************************************

  Design notes:

   1. Although the offset and page in the cache block can both be 
   calculated from either value, i chose to precalculate and store
   BOTH items.  The reason for this is that it avoids one 
   multiplication on each access, and one division when paging data
   in to cache.  For the default values data storage per cache is
   8192 bytes with 20 bytes additional overhead.  This means that
   0.2% of the data is overhead.  That's worth it to avoid a heavy
   op like multiplication

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
    
 *******************************************************************/
 

/// Default cache block size
const unsigned csmmioDefaultCacheBlockSize = 256;

/// Default cache size (in cache blocks)
const unsigned csmmioDefaultCacheSize = 2048;

/// Default size for hash table (best to use primes here)
const unsigned csmmioDefaultHashSize = 1559;


/// Defines a simple memory-mapped IO class that is portable.  Requires that data be organized in fixed block sizes.
class csMemoryMappedIO
{
  /// Minimum size of a single block 
  unsigned int block_size;
  
  /// Size of file in bytes to memory map
  unsigned int file_size;

  /// Set to true if this object is valid
  bool valid_mmio_object;

#ifndef _HAS_MEMORY_MAPPED_IO_

  /// Size of a cache block in block_size blocks (software emulation)
  unsigned cache_block_size;

  /// Size of in-memory cache in cache_block_size blocks, region is noncontiguous (software emulation)
  unsigned int cache_max_size;

  /// Number of cache blocks in cache
  unsigned int cache_block_count;

  /// Current age of system, the closer a cache-block's age is to this, the younger it is.
  unsigned int age;

  /// Array of bits where one bit = cache_block_size * block_size bytes.  A set bit indicates we have the page in memory.
  csBitArray *page_map;

  /// Pointer to file that contains the mapped data.
  FILE *mapped_file;

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
  
#else

  /// Holds information specific to the platform for hardware paging.
  struct PlatformData
  {

#if defined WIN32
    /// Handle to the mapped file (windows)
    HANDLE hMappedFile;
        
#elif defined _LINUX
    /// Handle to the mapped file (linux)
    int hMappedFile;

#endif // platform specific data defines

    /// Base pointer to the data
    unsigned char *data;

  } platform;

#endif // end else doesn't have memory-mapped i/o

public:
  /** Block size is the size of blocks that you want to get from the file, filename is the name of the file to
   * map.  Indexes will be resolved to absolute_index=index*block_size */
  csMemoryMappedIO(unsigned _block_size, char *filename);

  /** Destroys the mmapio object, closes open files, and releases memory.
   */
  csMemoryMappedIO::~csMemoryMappedIO();

  /** This pointer will only be valid for a little while.  Read, at least until the next call to GetPointer.
   * NEVER EVER EVER SAVE THIS POINTER.  You must recall this pointer when you want access to the data again.
   */
  inline void *GetPointer(unsigned int index);

private:
  /// Reads a cache-page in from the disk.
  void CachePage(unsigned int page);

};

#endif