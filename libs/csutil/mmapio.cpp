#include "cssysdef.h"
#include "csutil/mmapio.h"

#ifdef _LINUX_

#include <unistd.h>
#include <sys/mman.h>

#endif

csMemoryMappedIO::csMemoryMappedIO(unsigned _block_size, char *filename):
  block_size(_block_size)
{

#ifdef  _HAS_MEMORY_MAPPED_IO_

#if defined WIN32

  // Have Windows map this file in for use
  if ((platform->hMappedFile = OpenFileMapping(FILE_MAP_READ, false, filename)) == NULL || 
      (file_size=GetFileSize(platform->hMappedFile, NULL) == 0xFFFFFFFF                 || 
      (platform->data = MapViewofFile(platform->hMappedFile, FILE_MAP_READ, 0, 0, file_size))==NULL
     )
  {
    valid_mmio_object=false;
    return;
  }
  else
    valid_mmio_object=true;

#elif defined _LINUX_ || defined _MACOSX_

  struct stat statInfo;
  
  // Have [*]nix map this file in for use
  if ((platform->hMappedFile = open(filename, O_RDONLY) == -1   ||
      (fstat(platform->hMappedFile, &statInfo ) == -1           ||
      (platform->data = mmap(0, statInfo.st_size, PROT_READ, 0, platform->hMappedFile)) == -1
     )
  {
    valid_mmio_object=false;
    return;
  }
  else
  {
    valid_mmio_object=true;
    file_size=statInfo.st_size;
  }


#endif // platform specific loading


#else

  if ((mapped_file=fopen(filename, "rb")) != NULL)
  {
    valid_mmio_object=false;
    return;
  }
  else
  {
    // Create a page map with one bit per cache_block_size * block_size bytes.
    page_map = new csBitArray(file_size/(cache_block_size * block_size));

    // Clear the cache
    memset(&cache, 0, sizeof(cache));

    // Set that we're good to go.
    valid_mmio_object=true;
  }


#endif

}

csMemoryMappedIO::~csMemoryMappedIO()
{

#ifdef  _HAS_MEMORY_MAPPED_IO_ //////////////////////////////////////////////////////////////////////

#if defined WIN32
    
  if (platform.data!=NULL)
    UnmapViewofFile(platform.data);

  if (platform.hMappedFile!=NULL)
    CloseHandle(platform.hMappedFile);

#elif defined _LINUX_
    
  if (platform.data != -1)
    munmap(platform.data, file_size);

  if (platform.hMappedFile != -1)
    close(platform.hMappedFile);

#endif // platform specific loading


#else ////////////////////////////////////////////////////////////////////////////////////////////////

  if (mapped_file)
    fclose(mapped_file);

  if (page_map)
    delete page_map;

#endif


}

void *
csMemoryMappedIO::GetPointer(unsigned int index)
{

#if defined _HAS_MEMORY_MAPPED_IO_

  return platform.data + (index*block_size);

#else

  unsigned int i = index/cache_block_size;

  if (!(*page_map)[i])
    CachePage(i);

  return LookupIndex(i, index);
  
#endif

}

void 
csMemoryMappedIO::CachePage(unsigned int page)
{
  unsigned int bucket = page % csmmioDefaultHashSize;

  CacheBlock *cp;

  if (cache.inactive)
  {
    // Re-use a page off the inactive block.
    cp = cache.inactive;
    cache.inactive=cp->next;

    // Insert it into the bucket.
    cp->next=cache.blocks[bucket];
    cache.blocks[bucket]=cp;
  }
  else if (cache_block_count < cache_max_size)
  {
    cp = new CacheBlock;
    ++cache_block_count;

    // Insert it into the bucket.
    cp->next=cache.blocks[bucket];
    cache.blocks[bucket]=cp;
  }
  else
  {
    CacheBlock *block;
   
    // Find the least used block in this bucket.
    cp=cache.blocks[bucket];
    block=cp->next;

    while(block)
    { 
      if (block->age < cp->age)
        cp=block;

      block=block->next;
    }
  }

    
  // Get the data for it.
  cp->offset=page;
  cp->age=0;
    
  fseek(mapped_file, page*block_size, SEEK_SET);
  fread(cp->data, block_size, cache_block_size, mapped_file);
  
}

void *
csMemoryMappedIO::LookupIndex(unsigned int page, unsigned int index)
{
  unsigned int bucket = page % csmmioDefaultHashSize;

  

  return NULL;
}
