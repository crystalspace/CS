/*
  Copyright (C) 2007 by Frank Richter

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

#ifndef __CHUNKYALLOC_H__
#define __CHUNKYALLOC_H__

#include "csutil/compileassert.h"

#include "swappable.h"

namespace lighter
{
  /**
   * An allocator that allocates from bigger chunks of memory which can be
   * swapped out with lighter2's swapping system.
   * Since the chunks can be swapped out, allocations don't return a block
   * of memory, but a handle. Before using the memory, the handle must be
   * locked, and unlocked afterwards.
   *
   * \remarks The allocator currently only supports allocation, no freeing of
   *  memory. This is due to freeing not being needed at the time of 
   *  implementation, no deeper architectural or performance-related reason.
   */
  template<int ChunkSize = 1*1024*1024>
  class ChunkyAllocator : public CS::Memory::CustomAllocated
  {
    struct Chunk : public Swappable
    {
      size_t freeCount;
      size_t firstFree;

      size_t dataSize;
      uint8* data;

      Chunk (size_t dataSize) : freeCount (ChunkSize), firstFree (0), 
        dataSize (dataSize),
        data ((uint8*)SwappableHeap::Alloc (dataSize)) // FIXME: alloc only once really needed
      {
      }

      ~Chunk ()
      {
        Lock();
        SwappableHeap::Free (data);
      }

      Chunk (const Chunk& other) : freeCount (other.freeCount), 
        firstFree (other.firstFree), dataSize (other.dataSize),
        data ((uint8*)SwappableHeap::Alloc (dataSize))
      {
        ScopedSwapLock<Chunk> otherLock (other);
        this->Lock();
        memcpy (data, other.data, dataSize);
        this->Unlock();
      }

      void* ToPointer (size_t idx)
      {
        return data + idx;
      }

      size_t Alloc (size_t n)
      {
        if (n > freeCount) return (size_t)~0;
        size_t ret = firstFree;
        firstFree += n;
        freeCount -= n;
        return ret;
      }

      virtual void GetSwapData (void*& data, size_t& size)
      {
        data = this->data; this->data = 0;
        size = dataSize;
      }
      virtual size_t GetSwapSize() { return dataSize; }
      virtual void SwapIn (void* data, size_t size)
      {
        CS_ASSERT(size == dataSize);
        this->data = (uint8*)data;
      }
    };
    csSafeCopyArray<Chunk> allocatedChunks;

    size_t freeChunk;
  public:
    ChunkyAllocator() : freeChunk (~0) {}
    ~ChunkyAllocator()
    {
    }

    size_t Alloc (size_t num)
    {
      size_t p = (size_t)~0;
      Chunk* chunk = 0;
      size_t theChunk;
      if (num > ChunkSize)
      {
	theChunk = allocatedChunks.Push (Chunk (num));
	chunk = &allocatedChunks[theChunk];
        p = chunk->Alloc (num);
      }
      else
      {
	if (freeChunk != (size_t)~0)
	{
	  theChunk = freeChunk;
	  chunk = &allocatedChunks[freeChunk];
	  p = chunk->Alloc (num);
	}
	if (p == (size_t)~0) 
	{
	  for (size_t c = 0; c < allocatedChunks.GetSize(); c++)
	  {
	    chunk = &allocatedChunks[c];
	    p = chunk->Alloc (num);
	    if (p != (size_t)~0) 
	    {
	      theChunk = c;
	      break;
	    }
	  }
	}
	if (p == (size_t)~0) 
	{
	  theChunk = freeChunk = allocatedChunks.Push (Chunk (ChunkSize));
	  chunk = &allocatedChunks[freeChunk];
	  p = chunk->Alloc (num);
	}
      }
      CS_ASSERT(p != (size_t)~0);
      return theChunk * ChunkSize + p;
    }
    
    void* Lock (size_t id)
    {
      size_t chunkNum = id / ChunkSize;
      Chunk* chunk = &allocatedChunks[chunkNum];
      chunk->Lock ();
      size_t chunkOfs = id % ChunkSize;
      return chunk->data + chunkOfs;
    }
    void Unlock (size_t id)
    {
      size_t chunkNum = id / ChunkSize;
      Chunk* chunk = &allocatedChunks[chunkNum];
      chunk->Unlock ();
    }

    void LockAll ()
    {
      for (size_t c = 0; c < allocatedChunks.GetSize(); c++)
      {
        allocatedChunks[c].Lock ();
      }
    }
    void UnlockAll ()
    {
      for (size_t c = 0; c < allocatedChunks.GetSize(); c++)
      {
        allocatedChunks[c].Unlock ();
      }
    }
  };

  template<typename Alloc>
  class ScopedChunkyLock
  {
    Alloc& alloc;
    size_t item;
    void* itemPtr;
  public:
    ScopedChunkyLock (Alloc& alloc, size_t item) : alloc (alloc), item (item),
      itemPtr (alloc.Lock (item)) { }
    ~ScopedChunkyLock() { alloc.Unlock (item); }

    template<typename T>
    operator T* () { return (T*)itemPtr; }
  };
}

#endif // __CHUNKYALLOC_H__
