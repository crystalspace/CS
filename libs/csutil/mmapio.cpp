#define CS_SYSDEF_PROVIDE_HARDWARE_MMIO 1

#include "cssysdef.h"
#include "csutil/mmapio.h"

csMemoryMappedIO::csMemoryMappedIO(unsigned _block_size, char *filename):
  block_size(_block_size)
{
  valid_mmio_object = MemoryMapFile(&platform, filename);
}

csMemoryMappedIO::~csMemoryMappedIO()
{
  UnMemoryMapFile(&platform);
}

/////////////////////////////////////////// Software Support for Memory Mapping ///////////////////////////////////

#ifndef CS_HAS_MEMORY_MAPPED_IO

bool
csMemoryMappedIO::MemoryMapFile(mmioInfo *_platform, char *filename)
{
  // Clear the page map
  page_map = NULL;

  // Initialize cache management variables
  cache_block_size = csmmioDefaultCacheBlockSize;
  cache_max_size = csmmioDefaultCacheSize;
  cache_block_count=0;
  
  // Initialize the cache so that all buckets have at least one block
  
  unsigned i;
  for(i=0; i<csmmioDefaultHashSize; ++i)
  {
    CacheBlock *cp=new CacheBlock;
    
    ++cache_block_count;

    // Initialize it
    cp->data = new unsigned char[block_size * cache_block_size];

    // Set young so that it can be used again.
    cp->age=0;

    // This is the least likely page value.
    cp->page=0xffffffff;

    // Nothing next
    cp->next=0;

    cache[i] = cp;
  }
  
#ifdef CS_DEBUG
  hits=0;
  misses=0;
#endif
  
  if ((_platform->hMappedFile=fopen(filename, "rb")) == NULL)
  {
    return false;
  }
  else
  {
    // Create a page map with one bit per cache block.
    page_map = new csBitArray(_platform->file_size/cache_block_size);

    // Clear all of it's bits
    page_map->Clear();
    
    // Set that we're good to go.
    return true;
  }
}

void
csMemoryMappedIO::UnMemoryMapFile(mmioInfo *_platform)
{
  if (_platform->hMappedFile)
    fclose(_platform->hMappedFile);

  if (page_map)
    delete page_map;

  unsigned int i;
  CacheBlock *cp, *np;

  // Free all allocated memory.
  for(i=0; i<csmmioDefaultHashSize; ++i)
  {    
    cp=cache[i];
    while(cp)
    {
     np=cp->next;
     delete [] cp->data;
     delete cp;

     cp=np;
    }
  }
}

void 
csMemoryMappedIO::CachePage(unsigned int page)
{
  unsigned int bucket = page % csmmioDefaultHashSize;

  CacheBlock *cp;

  if (cache_block_count < cache_max_size)
  {
    cp = new CacheBlock;
    ++cache_block_count;

    // Insert it into the bucket.
    cp->next=cache[bucket];
    cache[bucket]=cp;

    // Initialize it
    assert((cp->data = new unsigned char[block_size * cache_block_size])!=NULL);
  }
  else
  {
    CacheBlock *block;
   
    // Find the least used block in this bucket.
    cp=cache[bucket];
    block=cp->next;

    while(block)
    { 
      if (block->age < cp->age)
        cp=block;

      block=block->next;
    }

    // Unmark this page as allocated
    if (cp->page!=0xffffffff) page_map->ClearBit(cp->page); 
  }
    
  // Get the data for it.
  cp->offset=page*cache_block_size;
  cp->page=page;
  cp->age=0;

  // Mark this page as allocated
  page_map->SetBit(page);
    
  // Read the page from the file
  fseek(platform.hMappedFile, page*cache_block_size*block_size, SEEK_SET);
  fread(cp->data, block_size, cache_block_size, platform.hMappedFile);
}


#endif
