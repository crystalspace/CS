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

#include "cssysdef.h"

#include "csplugincommon/shader/shadercachehelper.h"
#include "csutil/checksum.h"
#include "csutil/csendian.h"
#include "csutil/parasiticdatabuffer.h"

#include "iutil/hiercache.h"

#include "stringstore.h"

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{
  static void UI64Split (uint64 v, uint32& lo, uint32& hi)
  {
    union
    {
      uint64 ui64;
      uint32 ui32[2];
    } x;
    x.ui64 = v;
  #ifdef CS_LITTLE_ENDIAN
    lo = x.ui32[0];
    hi = x.ui32[1];
  #else
    lo = x.ui32[1];
    hi = x.ui32[0];
  #endif
  }
  
  static uint64 UI64Merge (uint32 lo, uint32 hi)
  {
    union
    {
      uint64 ui64;
      uint32 ui32[2];
    } x;
  #ifdef CS_LITTLE_ENDIAN
    x.ui32[0] = lo;
    x.ui32[1] = hi;
  #else
    x.ui32[1] = lo;
    x.ui32[0] = hi;
  #endif
    return x.ui64;
  }
  
  StringStore::StringStore (iHierarchicalCache* storageCache) :
    storageCache (storageCache), currentCacheTime (0), flushNeeded (false)
  {
    ReadBins();
  }
  
  StringStore::~StringStore()
  {
    Flush();
  }

  void StringStore::ReadBins ()
  {
    for (size_t b = 0; b < numStorageBins; b++)
    {
      bins[b].Clear();
    }
    
    csRef<iDataBuffer> headerData = storageCache->ReadCache ("/strshead");
    while (headerData.IsValid())
    {
      csMemFile headerFile (headerData, true);
      uint32 diskMagic;
      if (headerFile.Read ((char*)&diskMagic, sizeof (diskMagic))
          != sizeof (diskMagic))
        break;
      if (csLittleEndian::UInt32 (diskMagic) != storeFileMagic)
        break;
        
      uint32 diskCacheTime;
      if (headerFile.Read ((char*)&diskCacheTime, sizeof (diskCacheTime))
          != sizeof (diskCacheTime))
        break;
      currentCacheTime = csLittleEndian::UInt32 (diskCacheTime);
        
      for (size_t b = 0; b < numStorageBins; b++)
      {
        csRef<iDataBuffer> binHeadData =
          CS::PluginCommon::ShaderCacheHelper::ReadDataBuffer (&headerFile);
        csRef<iDataBuffer> binStrData =
          storageCache->ReadCache (csString().Format ("/strs%zu", b));
        bool okay = false;
        if (binHeadData.IsValid() && binStrData.IsValid())
        {
	  csMemFile binHeadFile (binHeadData, true);
	  if (bins[b].ReadHeader (&binHeadFile))
	  {
	    csMemFile binDataFile (binStrData, true);
	    okay = bins[b].ReadStringData (&binDataFile);
	  }
	}
        if (!okay) 
          bins[b].Clear();
      }
      
      headerData.Invalidate();
    }
    currentCacheTime++;
  }
  
  bool StringStore::WriteBins ()
  {
    csMemFile headerFile;
    uint32 diskMagic = csLittleEndian::UInt32 (storeFileMagic);
    if (headerFile.Write ((char*)&diskMagic, sizeof (diskMagic))
	!= sizeof (diskMagic))
      return false;
      
    uint32 diskCacheTime = csLittleEndian::UInt32 (currentCacheTime);
    if (headerFile.Write ((char*)&diskCacheTime, sizeof (diskCacheTime))
	!= sizeof (diskCacheTime))
      return false;
    
    for (size_t b = 0; b < numStorageBins; b++)
    {
      csMemFile binDataFile;
      if (!bins[b].WriteStringData (&binDataFile)) return false;
      if (!storageCache->CacheData (binDataFile.GetData(),
          binDataFile.GetSize(), csString().Format ("/strs%zu", b)))
        return false;
      
      csMemFile binHeadFile;
      if (!bins[b].WriteHeader (&binHeadFile)) return false;
      csRef<iDataBuffer> headerBuf (binHeadFile.GetAllData());
      if (!CS::PluginCommon::ShaderCacheHelper::WriteDataBuffer (&headerFile,
          headerBuf))
        return false;
    }
    return storageCache->CacheData (headerFile.GetData(),
      headerFile.GetSize(), "/strshead");
  }
  
  // Work around 'shift count >= width of type' warnings
  static inline uint rshift (uint x, uint v)
  {
    return (v >= sizeof(x) * 8) ? 0 : x >> v;
  }

  static inline uint lshift (uint x, uint v)
  {
    return (v >= sizeof(x) * 8) ? 0 : x << v;
  }

  const char* StringStore::GetStringForID (ID id)
  {
    uint32 crc, binAndID;
    UI64Split (id, crc, binAndID);
    
    const uint binNumberMask = (1 << storageBinBits) - 1;
    uint bin = rshift (binAndID, (32 - storageBinBits)) & (binNumberMask);
    BinID binId = binAndID & (lshift (1, (32 - storageBinBits))-1);
    
    return bins[bin].GetStringForID (currentCacheTime, binId, crc);
  }
  
  StringStore::ID StringStore::GetIDForString (const char* str)
  {
    HashedStr hashStr (str);
    const uint binNumberMask = (1 << storageBinBits) - 1;
    uint bin = hashStr.hash & binNumberMask;
    BinID binID;
    uint32 crc;
    flushNeeded |= bins[bin].GetIDForString (hashStr, currentCacheTime, binID, crc);
    uint32 binAndID = binID | lshift (bin, (32 - storageBinBits));
    return UI64Merge (crc, binAndID);
  }
  
  void StringStore::Flush()
  {
    if (!flushNeeded) return;
    WriteBins();
    flushNeeded = false;
  }

  //-------------------------------------------------------------------------

  const char* StringStore::StorageBin::GetStringForID (uint32 timestamp,
                                                       BinID id, uint32 crc)
  {
    BinEntry* entry = entries.GetElementPointer (id);
    if (entry == 0) return 0;
    if (entry->crc != crc) return 0;
    entry->lastUsedTime = timestamp;
    return stringDataFile->GetData() + entry->dataOffset;
  }

  bool StringStore::StorageBin::GetIDForString (const HashedStr& str,
                                                uint32 timestamp,
                                                BinID& id, uint32& crc)
  {
    HashedIDsType::Iterator idsIt (hashedIDs.GetIterator (str.hash));
    while (idsIt.HasNext ())
    {
      BinID currentID = idsIt.Next();
      BinEntry* entry = entries.GetElementPointer (currentID);
      if (entry == 0) continue;
      const char* entryData = stringDataFile->GetData() + entry->dataOffset;
      if (strcmp (entryData, str.s) != 0) continue;
      entry->lastUsedTime = timestamp;
      id = currentID;
      crc = entry->crc;
      return false;
    }
    
    size_t newId = takenIDs.GetFirstBitUnset();
    if (newId == csArrayItemNotFound)
    {
      newId = takenIDs.GetSize();
      takenIDs.SetSize (takenIDs.GetSize()+32);
    }
    takenIDs.SetBit (newId);
    BinEntry newEntry;
    newEntry.crc = CS::Utility::Checksum::CRC32 ((uint8*)str.s, str.len);
    newEntry.lastUsedTime = timestamp;
    newEntry.dataOffset = (uint32)stringDataFile->GetSize();
    stringDataFile->Write (str.s, str.len);
    char null = 0;
    stringDataFile->Write (&null, 1);
    entries.PutUnique ((BinID)newId, newEntry);
    hashedIDs.Put (str.hash, (BinID)newId);
    id = (BinID)newId;
    crc = newEntry.crc;
    return true;
  }

  void StringStore::StorageBin::Clear ()
  {
    stringDataFile.AttachNew (new csMemFile);
    takenIDs.Clear ();
    entries.DeleteAll ();
    hashedIDs.DeleteAll ();
  }
  
  bool StringStore::StorageBin::ReadHeader (iFile* file)
  {
    while (!file->AtEOF ())
    {
      uint32 diskID;
      if (file->Read ((char*)&diskID, sizeof (diskID))
          != sizeof (diskID))
        return false;
      uint32 diskTime;
      if (file->Read ((char*)&diskTime, sizeof (diskTime))
          != sizeof (diskTime))
        return false;
      uint32 diskOffs;
      if (file->Read ((char*)&diskOffs, sizeof (diskOffs))
          != sizeof (diskOffs))
        return false;
      BinEntry newEntry;
      newEntry.crc = (uint32)~0;
      newEntry.lastUsedTime = csLittleEndian::UInt32 (diskTime);
      newEntry.dataOffset = csLittleEndian::UInt32 (diskOffs);
      BinID id = csLittleEndian::UInt32 (diskID);
      entries.PutUnique (id, newEntry);
      if (id >= takenIDs.GetSize())
      {
        takenIDs.SetSize ((id+1+31) & ~31);
      }
      takenIDs.SetBit (id);
    }
    return true;
  }
  
  bool StringStore::StorageBin::WriteHeader (iFile* file)
  {
    EntryHash::GlobalIterator entriesIt (entries.GetIterator());
    while (entriesIt.HasNext ())
    {
      BinID id;
      BinEntry& entry = entriesIt.Next (id);
      
      uint32 diskID = csLittleEndian::UInt32 (id);
      if (file->Write ((char*)&diskID, sizeof (diskID))
          != sizeof (diskID))
        return false;
      uint32 diskTime = csLittleEndian::UInt32 (entry.lastUsedTime);
      if (file->Write ((char*)&diskTime, sizeof (diskTime))
          != sizeof (diskTime))
        return false;
      uint32 diskOffs = csLittleEndian::UInt32 (entry.dataOffset);
      if (file->Write ((char*)&diskOffs, sizeof (diskOffs))
          != sizeof (diskOffs))
        return false;
    }
    return true;
  }
  
  bool StringStore::StorageBin::ReadStringData (iFile* file)
  {
    bool okay = true;
    uint32 diskMagic;
    if (file->Read ((char*)&diskMagic, sizeof (diskMagic))
	!= sizeof (diskMagic))
      okay = false;
    if (okay && csLittleEndian::UInt32 (diskMagic) != binFileMagic)
      okay = false;
      
    if (okay)
    {
      csRef<iDataBuffer> fileData (file->GetAllData());
      csRef<iDataBuffer> strData;
      strData.AttachNew (new csParasiticDataBuffer (fileData, 4));
      stringDataFile.AttachNew (new csMemFile (strData, true));
      stringDataFile->SetPos (stringDataFile->GetSize ());
      
      EntryHash::GlobalIterator entriesIt (entries.GetIterator());
      while (entriesIt.HasNext ())
      {
        BinID id;
        BinEntry& entry = entriesIt.Next (id);
        const char* entryStr = stringDataFile->GetData() + entry.dataOffset;
	size_t len = strlen (entryStr);
	entry.crc = CS::Utility::Checksum::CRC32 ((uint8*)entryStr, len);
        uint hash = csHashCompute (entryStr, len);
        hashedIDs.Put (hash, id);
      }
    }
    return okay;
  }
  
  bool StringStore::StorageBin::WriteStringData (iFile* file)
  {
    EntryHash newEntries;
  
    uint32 diskMagic = csLittleEndian::UInt32 (binFileMagic);
    if (file->Write ((char*)&diskMagic, sizeof (diskMagic))
	!= sizeof (diskMagic))
      return false;
      
    csRef<csMemFile> newStringsFile;
    newStringsFile.AttachNew (new csMemFile);
    EntryHash::GlobalIterator entriesIt (entries.GetIterator());
    while (entriesIt.HasNext ())
    {
      BinID id;
      BinEntry newEntry (entriesIt.Next (id));
      const char* entryStr = stringDataFile->GetData() + newEntry.dataOffset;
      size_t len = strlen (entryStr);
      if (CS::Utility::Checksum::CRC32 ((uint8*)entryStr, len) != newEntry.crc)
        continue;
      newEntry.dataOffset = (uint32)newStringsFile->GetPos();
      if (newStringsFile->Write (entryStr, len+1) != len+1)
        return false;
      newEntries.Put (id, newEntry);
    }
    
    stringDataFile = newStringsFile;
    entries = newEntries;
    
    if (file->Write (newStringsFile->GetData(), newStringsFile->GetSize())
        != newStringsFile->GetSize())
      return false;
    
    return true;
  }

}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)
