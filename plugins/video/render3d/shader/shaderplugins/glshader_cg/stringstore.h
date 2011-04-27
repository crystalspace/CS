/*
  Copyright (C) 2008 by Frank Richter

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __STRINGSTORE_H__
#define __STRINGSTORE_H__

#include "csutil/bitarray.h"
#include "csutil/hash.h"
#include "csutil/memfile.h"

struct iHierarchicalCache;

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{
  /* The string store can store strings in multiple 'bins'.
     The original idea was to allow for not having all strings
     in memory at all time, thus the segmenting into bins; however,
     in practice, the size of string data is small enough to making
     the segmenting unnecessary. Setting the bin bits number to 0
     essentially disables the features. */
  static const uint storageBinBits = 0;
  /* The most significant byte is the version, change when the format
   * changes */
  static const uint32 storeFileMagic = 0x01807353
                                       | (storageBinBits << 16);
  static const uint32 binFileMagic = 0x00627353;
  
  class StringStore
  {
  public:
    typedef uint64 ID;
    typedef uint32 BinID;
    
  protected:
    enum { numStorageBins = 1 << storageBinBits };
    
    struct HashedStr
    {
      const char* s;
      size_t len;
      uint hash;
      
      HashedStr (const char* s) : s (s), len (strlen (s)),
        hash (csHashCompute (s, len)) {}
      HashedStr (const char* s, size_t slen) : s (s),
        len (slen), hash (csHashCompute (s, slen)) {}
    };
  
    csRef<iHierarchicalCache> storageCache;
    uint32 currentCacheTime;
    bool flushNeeded;
    struct BinEntry
    {
      uint32 crc;
      uint32 dataOffset;
      uint32 lastUsedTime;
    };
    class StorageBin
    {
      csBitArray takenIDs;
      typedef csHash<BinEntry, BinID> EntryHash;
      EntryHash entries;
      typedef csHash<BinID, uint> HashedIDsType;
      HashedIDsType hashedIDs;
      csRef<csMemFile> stringDataFile;
    public:
      const char* GetStringForID (uint32 timestamp,
        BinID id, uint32 crc);
      bool GetIDForString (const HashedStr& str, 
        uint32 timestamp, BinID& id, uint32& crc);
      
      void Clear();
      
      bool ReadHeader (iFile* file);
      bool WriteHeader (iFile* file);
      bool ReadStringData (iFile* file);
      bool WriteStringData (iFile* file);
    };
    StorageBin bins[numStorageBins];
    
    void ReadBins ();
    bool WriteBins ();
  public:
    StringStore (iHierarchicalCache* storageCache);
    ~StringStore();
  
    const char* GetStringForID (ID id);
    ID GetIDForString (const char* str);
    
    void Flush();
  };

}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)

#endif // __STRINGSTORE_H__
