/*
  Copyright (C) 2006-2007 by Marten Svanfeldt
            (C) 2007 by Frank Richter

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

#include "common.h"

#define Byte z_Byte	/* Kludge to avoid conflicting typedef in zconf.h */
#include <zlib.h>
#undef Byte

#include "swappable.h"

namespace lighter
{
  SwapManager::SwapManager (size_t maxSize) : entryAlloc (1000),   
    currentCacheSize (0), swappedOutSize (0), currentUnlockTime (1)
  {
    size_t maxVirtSize = CS::Platform::GetMaxVirtualSize();
    size_t maxVirtBytes;
    if (maxVirtSize > SIZE_MAX / 1024)
      maxVirtBytes = SIZE_MAX;
    else
      maxVirtBytes = maxVirtSize * 1024;
    /* Cap maximum cache to half the max virtual address space to
       avoid that we run out of virtual address space */
    maxCacheSize = csMin (maxSize, maxVirtBytes / 2);
  }

  SwapManager::~SwapManager ()
  {
  #ifdef CS_DEBUG
    // Sanity check that nothing is registered any more.
    if (swapCache.GetSize() > 0)
    {
      csPrintf ("Leftover swappables:\n");
      SwapCacheType::GlobalIterator it (swapCache.GetIterator());
      while (it.HasNext())
      {
	csPtrKey<iSwappable> p;
	it.Next (p);
	csPrintf ("%p: ", (iSwappable*)p);
	fflush (stdout);
	csPrintf ("%s\n", p->Describe());
      }
    }
    // Also, annoy.
    CS_ASSERT(swapCache.GetSize() == 0);
  #endif
  }

  void SwapManager::RegisterSwappable (iSwappable* obj)
  {
    CS::Threading::MutexScopedLock lock (swapMutex);

    CS_ASSERT(swapCache.Get (obj, 0) == 0);
    SwapEntry* e = entryAlloc.Alloc();
    e->obj = obj;
    e->lastUnlockTime = currentUnlockTime++;
    swapCache.Put (obj, e);
    unlockedCacheEntries.Add (e);
  }

  void SwapManager::UnregisterSwappable (iSwappable* obj)
  {    
    CS::Threading::MutexScopedLock lock (swapMutex);

    SwapEntry* e = swapCache.Get (obj, 0);
    CS_ASSERT(e != 0);
    swapCache.Delete (obj, e);
   
    if (!SwapIn (e))
    { 
      csPrintfErr ("Error swapping in data for %p\n", obj);
      // Not nice but...
      abort();
    }

    unlockedCacheEntries.Delete (e);
    if (e->lastSize != (size_t)~0)
      currentCacheSize -= e->lastSize;

    csString tmpFileName = GetFileName (e->obj);
    globalLighter->vfs->DeleteFile (tmpFileName);

    entryAlloc.Free (e);
  }

  void SwapManager::Lock (iSwappable* obj)
  {
    SwapEntry* e = 0;
    {
      CS::Threading::MutexScopedLock lock (swapMutex);
      e = swapCache.Get (obj, 0);
      CS_ASSERT(e != 0);
      unlockedCacheEntries.Delete (e);

      AccountEntrySize (e);
    }

    if (currentCacheSize > maxCacheSize)
    {
      FreeMemory ((currentCacheSize - maxCacheSize) * 2);
    }

    if (!SwapIn (e))
    { 
      csPrintfErr ("Error swapping in data for %p\n", obj);
      // Not nice but...
      abort();
    }
  }

  void SwapManager::Unlock (iSwappable* obj)
  {
    CS::Threading::MutexScopedLock lock (swapMutex);

    SwapEntry* e = swapCache.Get (obj, 0);
    CS_ASSERT(e != 0);
    unlockedCacheEntries.Add (e);
    e->lastUnlockTime = currentUnlockTime++;
  }

  void SwapManager::UpdateSize (iSwappable* obj)
  {
    CS::Threading::MutexScopedLock lock (swapMutex);

    SwapEntry* e = swapCache.Get (obj, 0);
    CS_ASSERT(e != 0);
    if (e->swapStatus == e->swappedIn)
    {
      if (e->lastSize != (size_t)~0)
        currentCacheSize -= e->lastSize;
      e->lastSize = obj->GetSwapSize ();
      currentCacheSize += e->lastSize;
    }
  }

  static const uint32 swapFileMagic = 0x4c325357;

  bool SwapManager::SwapOut (SwapEntry* e)
  {
    // If nothing to swap out, nothing to do    
    int32 oldStatus = 
      CS::Threading::AtomicOperations::CompareAndSet (
      &e->swapStatus, SwapEntry::swapping, SwapEntry::swappedIn);

    if (oldStatus != SwapEntry::swappedIn)
      return true;

    csString tmpFileName = GetFileName (e->obj);

    void* swapData;
    size_t swapSize;

    e->obj->GetSwapData (swapData, swapSize);

    if (swapSize > 0)
    {
      CS_ASSERT(swapData != 0);
      csRef<iFile> file = globalLighter->vfs->Open (tmpFileName, VFS_FILE_WRITE);

      if (!file.IsValid ())
        return false;

      // Write a smallish header
      file->Write ((const char*)&swapFileMagic, sizeof (uint32));
      file->Write ((const char*)&swapSize, sizeof (size_t));

      // Write the raw data
      z_stream zs;
      memset (&zs, 0, sizeof(z_stream));

      zs.next_in = (z_Byte*)swapData;
      zs.avail_in = (uInt)(swapSize);

      int rc = deflateInit (&zs, 1);
      if (rc != Z_OK)
      {
        csPrintfErr ("%s: zlib error %d: %s\n", CS_FUNCTION_NAME, rc, zs.msg);
        return false;
      }

      size_t compressBufferSize = 128*1024;
      CS_ALLOC_STACK_ARRAY(z_Byte, compressBuffer, compressBufferSize);

      while (true)
      {
        zs.next_out = compressBuffer;
        zs.avail_out = (uInt)compressBufferSize;

        rc = deflate (&zs, Z_FINISH);
        size_t size = compressBufferSize - zs.avail_out;

        if (file->Write ((const char*)compressBuffer, size) != size)
        {
          csPrintfErr ("%s: could not write to %s\n", CS_FUNCTION_NAME, 
            tmpFileName.GetData());
          deflateEnd (&zs);
          return false;
        }

        if (rc == Z_STREAM_END)
          break;
        if (rc != Z_OK)
        {
          csPrintfErr ("%s: zlib error %d: %s\n", CS_FUNCTION_NAME, rc, 
            zs.msg);
          deflateEnd (&zs);
          return false;
        }
      }
      deflateEnd (&zs);

      e->swapStatus = SwapEntry::swappedOut;
      // Delete the memory
      swapHeap.Free (swapData);
    }
    else
      e->swapStatus = SwapEntry::swappedOutEmpty;

    // Mark it as swapped out too..
    {
      CS::Threading::MutexScopedLock lock (swapMutex);
      unlockedCacheEntries.Delete (e);    
      currentCacheSize -= e->lastSize;
      swappedOutSize += e->lastSize;
    }

    return true;
  }

  bool SwapManager::SwapIn (SwapEntry* e)
  {
    // MT Note: Not supported trying to both swap out and in an entry at same time
    // Double in or double out is however supported.

    // Set it to swapped in atomic
    int32 oldStatus = 
      CS::Threading::AtomicOperations::Set (&e->swapStatus, SwapEntry::swappedIn);
    
    if (oldStatus == SwapEntry::swappedIn ||
      oldStatus == SwapEntry::swapping) 
      return true;

    if (oldStatus == SwapEntry::swappedOutEmpty)
    {
      e->obj->SwapIn (0, 0);      
      return true;
    }

    csString tmpFileName = GetFileName (e->obj);

    csRef<iFile> file = globalLighter->vfs->Open (tmpFileName, VFS_FILE_READ);
    if (!file.IsValid ())
      return false;
    
    uint32 magic;
    file->Read ((char*)&magic, sizeof (uint32));

    // Bail on wrong header
    if (magic != swapFileMagic)
      return false;

    // Read size
    size_t swapSize;
    file->Read ((char*)&swapSize, sizeof (size_t));

    // Allocate
    void* swapData = swapHeap.Alloc (swapSize);
    if (swapData == 0)
    {
      /* Perhaps don't give up so soon and try to evict more swap memory and 
         allocate again? */
      csPrintfErr ("%s: Error allocating %zu bytes\n", CS_FUNCTION_NAME, 
        swapSize);
      return false;
    }

    z_stream zs;
    memset (&zs, 0, sizeof(z_stream));

    zs.next_out = (z_Byte*)swapData;
    zs.avail_out = (uInt)(swapSize);

    if (inflateInit (&zs) != Z_OK)
    {
      cs_free (swapData);
      return false;
    }

    size_t compressBufferSize = 128*1024;
    CS_ALLOC_STACK_ARRAY(z_Byte, compressBuffer, compressBufferSize);

    while (true)
    {
      size_t readSize = file->Read ((char*)compressBuffer, compressBufferSize);

      zs.next_in = compressBuffer;
      zs.avail_in = (uInt)readSize;

      int rc = inflate (&zs, Z_FINISH);

      if (rc == Z_STREAM_END)
        break;
      if ((rc != Z_OK) && (rc != Z_BUF_ERROR))
      {
        csPrintfErr ("%s: zlib error %d: %s\n", CS_FUNCTION_NAME, rc, zs.msg);
        inflateEnd (&zs);
        cs_free (swapData);
        return false;
      }
    }
    inflateEnd (&zs);

    e->obj->SwapIn (swapData, swapSize);
    e->swapStatus = SwapEntry::swappedIn;
    e->lastSize = swapSize;

    {
      CS::Threading::MutexScopedLock lock (swapMutex);
      currentCacheSize += e->lastSize;
      swappedOutSize -= e->lastSize;
    }    

    return true;
  }

  void SwapManager::FreeMemory (size_t desiredAmount)
  {
    // Walk through the unlocked set of entries, swap them out until we have enough free memory
    csArray<SwapEntry*> sortedList;

    {
      CS::Threading::MutexScopedLock lock (swapMutex);

      UnlockedEntriesType::GlobalIterator git = unlockedCacheEntries.GetIterator ();
      while (git.HasNext ())
      {
        SwapEntry* e = git.Next ();
        AccountEntrySize (e);
        sortedList.InsertSorted (e, SwapEntryAgeCompare);
      }
    }

    size_t targetSize = csMax ((long)currentCacheSize - (long)desiredAmount, 0L);
    
    csArray<SwapEntry*>::Iterator sit = sortedList.GetIterator ();
    while ((targetSize < currentCacheSize) && sit.HasNext ())
    {
      SwapEntry* e = sit.Next ();      

      if (!SwapOut (e))
      { 
        csPrintfErr ("Error swapping out data for %p\n", e->obj);
        // Not nice but...
        abort();
      }
    }
  }
  
  void SwapManager::GetSizes (uint64& swappedIn, uint64& swappedOut,
                              uint64& maxSize)
  {
    swappedIn = currentCacheSize;
    swappedOut = swappedOutSize;
    maxSize = maxCacheSize;
  }

  csString SwapManager::GetFileName (iSwappable* obj)
  {
    csString tmp;
    tmp.Format ("/tmp/lighter2/swp%p.tmp", obj);
    return tmp;
  }

}
