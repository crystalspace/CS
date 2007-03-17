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
    maxCacheSize (maxSize), currentCacheSize (0), currentUnlockTime (1)
  {
  }

  SwapManager::~SwapManager ()
  {
    // Sanity check that nothing is registered any more.
    CS_ASSERT(swapCache.GetSize() == 0);
  }

  void SwapManager::RegisterSwappable (iSwappable* obj)
  {
    CS_ASSERT(swapCache.Get (obj, 0) == 0);
    SwapEntry* e = entryAlloc.Alloc();
    e->obj = obj;
    e->lastUnlockTime = currentUnlockTime++;
    swapCache.Put (obj, e);
    unlockedCacheEntries.Add (e);
  }

  void SwapManager::UnregisterSwappable (iSwappable* obj)
  {
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
    SwapEntry* e = swapCache.Get (obj, 0);
    CS_ASSERT(e != 0);
    unlockedCacheEntries.Delete (e);

    AccountEntrySize (e);

    if (currentCacheSize > maxCacheSize)
    {
      FreeMemory (currentCacheSize - maxCacheSize);
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
    SwapEntry* e = swapCache.Get (obj, 0);
    CS_ASSERT(e != 0);
    unlockedCacheEntries.Add (e);
    e->lastUnlockTime = currentUnlockTime++;
  }

  static const uint32 swapFileMagic = 0x4c325357;

  bool SwapManager::SwapOut (SwapEntry* e)
  {
    // If nothing to swap out, nothing to do
    if (e->swapStatus != SwapEntry::swappedIn) return true;

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

      if (deflateInit (&zs, 1) != Z_OK)
        return false;

      size_t compressBufferSize = 128*1024;
      CS_ALLOC_STACK_ARRAY(z_Byte, compressBuffer, compressBufferSize);

      while (true)
      {
        zs.next_out = compressBuffer;
        zs.avail_out = (uInt)compressBufferSize;

        int rc = deflate (&zs, Z_FINISH);
        size_t size = compressBufferSize - zs.avail_out;

        if (file->Write ((const char*)compressBuffer, size) != size)
        {
          deflateEnd (&zs);
          return false;
        }

        if (rc == Z_STREAM_END)
          break;
        if (rc != Z_OK)
        {
          csPrintfErr ("zlib error: %s\n", zs.msg);
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
    unlockedCacheEntries.Delete (e);
    
    currentCacheSize -= e->lastSize;

    return true;
  }

  bool SwapManager::SwapIn (SwapEntry* e)
  {
    if (e->swapStatus == SwapEntry::swappedIn) return true;

    if (e->swapStatus == SwapEntry::swappedOutEmpty)
    {
      e->obj->SwapIn (0, 0);
      e->swapStatus = SwapEntry::swappedIn;
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
      if (rc != Z_OK)
      {
        csPrintfErr ("zlib error: %s\n", zs.msg);
        inflateEnd (&zs);
        return false;
      }
    }
    inflateEnd (&zs);

    e->obj->SwapIn (swapData, swapSize);
    e->swapStatus = SwapEntry::swappedIn;
    e->lastSize = swapSize;
    currentCacheSize += e->lastSize;

    return true;
  }

  void SwapManager::FreeMemory (size_t desiredAmount)
  {
    // Walk through the unlocked set of entries, swap them out until we have enough free memory
    size_t targetSize = currentCacheSize - desiredAmount;

    csArray<SwapEntry*> sortedList;

    UnlockedEntriesType::GlobalIterator git = unlockedCacheEntries.GetIterator ();
    while (git.HasNext ())
    {
      SwapEntry* e = git.Next ();
      sortedList.InsertSorted (e, SwapEntryAgeCompare);
    }

    csArray<SwapEntry*>::Iterator sit = sortedList.GetIterator ();
    while ((targetSize < currentCacheSize) && sit.HasNext ())
    {
      SwapEntry* e = sit.Next ();

      AccountEntrySize (e);

      if (!SwapOut (e))
      { 
        csPrintfErr ("Error swapping out data for %p\n", e->obj);
        // Not nice but...
        abort();
      }
    }
  }

  csString SwapManager::GetFileName (iSwappable* obj)
  {
    csString tmp;
    tmp.Format ("/tmp/lighter2/swp%p.tmp", obj);
    return tmp;
  }

}
