#ifndef __CS_MEMORY_MAPPED_IO__
#define __CS_MEMORY_MAPPED_IO__

#include "csutil/bitarray.h"

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

    /// The starting offset of all the block in this cache block.
    unsigned offset;

    /// The next item in our block list.
    CacheBlock *next;

    /// The data
    unsigned char *data;
  };

  struct SimpleHash
  {
    /// Hash table for active blocks
    CacheBlock *blocks[csmmioDefaultHashSize];
    
    /// List of inactive blocks
    CacheBlock *inactive;

  } cache;

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

  /// Returns the pointer to the memory location we need
  void *LookupIndex(unsigned int page, unsigned int index);
};

#endif