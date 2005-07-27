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

#include "cssysdef.h"
#include "csutil/csmmap.h"
#include "csutil/mmapio.h"
#include "csutil/ref.h"

#include "iutil/databuff.h"
#include "iutil/vfs.h"

#ifdef CS_HAVE_MEMORY_MAPPED_IO
bool csMemoryMapFile(csMemMapInfo*, char const* filename);
void csUnMemoryMapFile(csMemMapInfo*);
#endif

csMemoryMappedIO::csMemoryMappedIO(unsigned _block_size, char const *filename, 
                                   iVFS* vfs):
  block_size(_block_size)
{
  const char* realpath = 0;
  if (vfs)
  {
    csRef<iDataBuffer> rpath = vfs->GetRealPath (filename);
    realpath = (char*)(rpath->GetData ());
  }
  else
  {
    realpath = filename;
  }
  valid_mmio_object = false;
  if (realpath)
  {
   #ifdef CS_HAVE_MEMORY_MAPPED_IO
    valid_platform = csMemoryMapFile(&platform, realpath);
    if (!valid_platform)
   #endif
    {
      valid_mmio_object = SoftMemoryMapFile(&emulatedPlatform, realpath);
    }
   #ifdef CS_HAVE_MEMORY_MAPPED_IO
    else
      valid_mmio_object = true;
   #endif
  }
}

csMemoryMappedIO::~csMemoryMappedIO()
{
  if (valid_mmio_object)
  {
   #ifdef CS_HAVE_MEMORY_MAPPED_IO
    if (valid_platform)
    {
      csUnMemoryMapFile(&platform);
    }
    else
   #endif
    {
      SoftUnMemoryMapFile(&emulatedPlatform);
    }
  }
}

/////////////////////////////////////////// Software Support for Memory Mapping ///////////////////////////////////

bool
csMemoryMappedIO::SoftMemoryMapFile(emulatedMmioInfo *_platform, char const *filename)
{
  // Clear the page map
  page_map = 0;

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
  
  if ((_platform->hMappedFile=fopen(filename, "rb")) == 0)
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
csMemoryMappedIO::SoftUnMemoryMapFile(emulatedMmioInfo *_platform)
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
    assert((cp->data = new unsigned char[block_size * cache_block_size])!=0);
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
  fseek(emulatedPlatform.hMappedFile, page*cache_block_size*block_size, SEEK_SET);
  fread(cp->data, block_size, cache_block_size, emulatedPlatform.hMappedFile);
}

void*
csMemoryMappedIO::GetPointer(unsigned int index)
{
#ifdef CS_HAVE_MEMORY_MAPPED_IO
  if (valid_platform) 
  {
    return platform.data + (index*block_size);
  }
  else
#endif
  {
    unsigned int page = index/cache_block_size;
  
    if (!valid_mmio_object) return 0;

    if (!(*page_map)[page])
    {
      #ifdef CS_DEBUG
      ++misses;
      #endif
      CachePage(page);
    }
#ifdef CS_DEBUG
    else ++hits;
#endif

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
    return 0;
  }
}

bool 
csMemoryMappedIO::IsValid()
{
  return valid_mmio_object;
}
