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

void *
csMemoryMappedIO::GetPointer(unsigned int index)
{

#if defined _HAS_MEMORY_MAPPED_IO_

  return platform.data + (index*block_size);

#else

  unsigned int page = index/cache_block_size;
  
  if (!(*page_map)[page])
    CachePage(page);

  // This MUST come AFTER CachPage b/c CachePage might re-orient things.
  CacheBlock *cp = cache[page % csmmioDefaultHashSize];

  while(cp)
  { 
    if (cp->page==page)
    {
      // Decrease age     
      ++cp->age;
	      
      return cp->data + ((index-cp->offset)*block_size);
    }

    cp=cp->next;
  }

  //Serious error! The page is marked as here, but we could not find it!
  return NULL;
  
#endif

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
    cp->data = new unsigned char[block_size * cache_block_size];
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
    page_map->ClearBit(cp->page); 
  }
    
  // Get the data for it.
  cp->offset=page*cache_block_size;
  cp->page=page;
  cp->age=0;

  // Mark this page as allocated
  page_map->SetBit(page);
    
  // Read the page from the file
  fseek(mapped_file, page*cache_block_size*block_size, SEEK_SET);
  fread(cp->data, block_size, cache_block_size, mapped_file);
}


/////////////////////////////////////////// Software Support for Memory Mapping ///////////////////////////////////

#ifndef CS_HAS_MEMORY_MAPPED_IO

bool
csMemoryMappedIO::MemoryMapFile(mmioInfo *_platform, char *filename)
{
  if ((_platform->hMappedFile=fopen(filename, "rb")) != NULL)
  {
    return false;
  }
  else
  {
    // Create a page map with one bit per cache block.
    page_map = new csBitArray(_platform->file_size/cache_block_size);

    // Clear the cache
    memset(&cache, 0, sizeof(cache));

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


#endif