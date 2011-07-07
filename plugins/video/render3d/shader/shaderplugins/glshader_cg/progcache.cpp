/*
  Copyright (C) 2009 by Frank Richter
  
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

#include "progcache.h"

#include "iutil/databuff.h"
#include "iutil/hiercache.h"
#include "csplugincommon/shader/shadercachehelper.h"
#include "csutil/csendian.h"
#include "csutil/md5.h"

#include "profile_limits.h"

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{
  /* This define specifies whether the original source should be stored in
    the program cache. Otherwise, cache items are only identified by the
    hash of the source. Obviously, this runs the risk of collisions - otoh,
    this has not been observed as an issue so far. Enable source storing once
    that does.
  */
  #define PROG_CACHE_STORE_SOURCE 0
  
  static const uint32 cacheFileMagicCC = 0x05435043
					^ (0x00202020 * PROG_CACHE_STORE_SOURCE);

  enum
  {
    cpsValid = 0x6b726f77,
    cpsInvalid = 0x6b723062
  };
  
  
  /*class ProgramCache;

  struct ProgramObjectID
  {
    csString path;
    csString item;
  };

  class ProgramObject
  {
    friend class ProgramCache;
    
    ProgramObjectID id;
    
    ProgramObject ();
  public:
    bool IsValid () const;
    
    const ProgramObjectID& GetID() const { return id; }
  };

  class ProgramCache
  {
  public:
    ProgramObjectID WriteObject (const char* source, const ProfileLimits& limits,
      const char* objectCode, csString& failReason);
  };*/
  
  ProgramObject::ProgramObject () : flags (0) {}
    
  ProgramObject::ProgramObject (const char* objectCode, uint flags,
    const csSet<csString>& unusedParams) : objectCode (objectCode),
    flags (flags), unusedParams (unusedParams) {}
  
  //--------------------------------------------------------------------------
  
  bool ProgramCache::LoadObject (const ProgramObjectID& id, ProgramObject& program)
  {
    CS_ASSERT(!id.archive.IsEmpty());
    CS_ASSERT(!id.item.IsEmpty());
    
    if (cache == 0) return false;
  
    csRef<iDataBuffer> cacheArcBuf = cache->ReadCache (id.archive);
    if (!cacheArcBuf.IsValid()) return false;
    CS::PluginCommon::ShaderCacheHelper::MicroArchive cacheArc;
    {
      csMemFile cacheArcFile (cacheArcBuf, true);
      if (!cacheArc.Read (&cacheArcFile)) return false;
    }
      
    csRef<iDataBuffer> cacheBuf = cacheArc.ReadEntry (id.item);
    if (!cacheBuf.IsValid()) return false;
    
    csRef<iFile> cacheFile;
    cacheFile.AttachNew (new csMemFile (cacheBuf, true));
    
    uint32 diskMagic;
    if (cacheFile->Read ((char*)&diskMagic, sizeof (diskMagic))
	!= sizeof (diskMagic)) return false;
    if (csLittleEndian::UInt32 (diskMagic) != cacheFileMagicCC)
      return false;
      
    {
  #if PROG_CACHE_STORE_SOURCE
      csRef<iDataBuffer> skipBuf = 
	CS::PluginCommon::ShaderCacheHelper::ReadDataBuffer (cacheFile);
  #else
      uint32 dummy;
      cacheFile->Read ((char*)&dummy, sizeof (dummy));
  #endif
    }
  
    
    if (!cacheFile.IsValid()) return false;
    
    program = ProgramObject();
    {
      uint32 diskState;
      if (cacheFile->Read ((char*)&diskState, sizeof (diskState))
	  != sizeof (diskState)) return false;
      if (csLittleEndian::UInt32 (diskState) != cpsValid)
      {
	return true;
      }
    }
    
    {
      uint32 diskFlags;
      if (cacheFile->Read ((char*)&diskFlags, sizeof (diskFlags))
	  != sizeof (diskFlags)) return false;
      program.flags =  csLittleEndian::UInt32 (diskFlags);
    }
    
    program.objectCode =
      CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile);
      
    {
      csString p;
      while (!(p = CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile))
	.IsEmpty())
      {
	program.unusedParams.Add (p);
      }
    }
    
    return true;
  }
  
  bool ProgramCache::SearchObject (const char* source, const ProfileLimits& limits,
                                   ProgramObject& program)
  {
    if (cache == 0) return false;
    
    csString objectCodeCachePathArc, objectCodeCachePathItem;
    
  #if PROG_CACHE_STORE_SOURCE
    csMemFile stringIDs;
    {
      csStringReader reader (source);
      csString line;
      while (reader.GetLine (line))
      {
	StringStore::ID stringID = shaderPlug->stringStore->GetIDForString (line);
	uint64 diskID = csLittleEndian::UInt64 (stringID);
	stringIDs.Write ((char*)&diskID, sizeof (diskID));
      }
    }
  #endif // PROG_CACHE_STORE_SOURCE
    
    CS::Utility::Checksum::MD5::Digest sourceMD5 =
      CS::Utility::Checksum::MD5::Encode (source);
    csString cacheArcPath;
    cacheArcPath.Format ("/%s",
      sourceMD5.HexString().GetData());
      
    csRef<iDataBuffer> cacheArcBuf = cache->ReadCache (cacheArcPath);
    if (!cacheArcBuf.IsValid()) return false;
    CS::PluginCommon::ShaderCacheHelper::MicroArchive cacheArc;
    {
      csMemFile cacheArcFile (cacheArcBuf, true);
      if (!cacheArc.Read (&cacheArcFile)) return false;
    }
    
    csRef<iFile> foundFile;
  #if PROG_CACHE_STORE_SOURCE
    csString entryPrefix (limits.ToString().GetData());
    itemPrefix.Append ("/");
    for (size_t i = 0; i < cacheArc.GetEntriesNum(); i++)
    {
      const char* arcEntry = cacheArc.GetEntryName (i);
      if (strncmp (arcEntry, entryPrefix, entryPrefix.Length()) != 0) continue;
      csRef<iDataBuffer> cacheBuf = cacheArc.ReadEntry (arcEntry);
      if (!cacheBuf.IsValid()) continue;
      
      csRef<iFile> cacheFile;
      cacheFile.AttachNew (new csMemFile (cacheBuf, true));
    
      uint32 diskMagic;
      if (cacheFile->Read ((char*)&diskMagic, sizeof (diskMagic))
	  != sizeof (diskMagic)) continue;
      if (csLittleEndian::UInt32 (diskMagic) != cacheFileMagicCC)
	continue;
  
      csRef<iDataBuffer> cachedIDs =
	CS::PluginCommon::ShaderCacheHelper::ReadDataBuffer (cacheFile);
      
      if (cachedIDs->GetSize() != stringIDs.GetSize())
	continue;
      if (memcmp (cachedIDs->GetData(), stringIDs.GetData(),
	  stringIDs.GetSize()) != 0)
	continue;
	
      foundFile = cacheFile;
      objectCodeCachePathArc = cacheArcPath;
      objectCodeCachePathItem = arcEntry;
      break;
    }
  #else
    {
      csRef<iDataBuffer> cacheBuf =
	cacheArc.ReadEntry (limits.ToString().GetData());
      if (!cacheBuf.IsValid()) return false;
      
      csRef<iFile> cacheFile;
      cacheFile.AttachNew (new csMemFile (cacheBuf, true));
    
      uint32 diskMagic;
      if (cacheFile->Read ((char*)&diskMagic, sizeof (diskMagic))
	  != sizeof (diskMagic)) return false;;
      if (csLittleEndian::UInt32 (diskMagic) != cacheFileMagicCC)
	return false;;
	
      uint32 diskSize;
      if (cacheFile->Read ((char*)&diskSize, sizeof (diskSize))
	  != sizeof (diskSize)) return false;;
      if (csLittleEndian::UInt32 (diskSize) != strlen (source))
	return false;;
	
      foundFile = cacheFile;
      objectCodeCachePathArc = cacheArcPath;
      objectCodeCachePathItem = limits.ToString().GetData();
    }
  #endif
  
    if (!foundFile.IsValid()) return false;
    
    program = ProgramObject();
    
    {
      uint32 diskState;
      if (foundFile->Read ((char*)&diskState, sizeof (diskState))
	  != sizeof (diskState)) return false;
      if (csLittleEndian::UInt32 (diskState) != cpsValid)
      {
	return true;
      }
    }
    
    {
      uint32 diskFlags;
      if (foundFile->Read ((char*)&diskFlags, sizeof (diskFlags))
	  != sizeof (diskFlags)) return false;
      program.flags =  csLittleEndian::UInt32 (diskFlags);
    }
    
    program.objectCode =
      CS::PluginCommon::ShaderCacheHelper::ReadString (foundFile);
    if (program.objectCode.IsEmpty()) return false;
    
    {
      csString p;
      while (!(p = CS::PluginCommon::ShaderCacheHelper::ReadString (foundFile))
	.IsEmpty())
      {
	program.unusedParams.Add (p);
      }
    }
  
    /*if (program)
    {
      cgDestroyProgram (program);
    }
    cgGetError(); // Clear error
    program = cgCreateProgram (shaderPlug->context, 
      CG_OBJECT, objectCode, limits.profile, 0, 0);
    if (!program) return false;
    CGerror err = cgGetError();
    if (err != CG_NO_ERROR)
    {
      const char* errStr = cgGetErrorString (err);
      shaderPlug->Report (CS_REPORTER_SEVERITY_WARNING,
	"Cg error %s", errStr);
      return false;
    }*/
    
    //this->objectCodeCachePathArc = objectCodeCachePathArc;
    //this->objectCodeCachePathItem = objectCodeCachePathItem;
    program.id.archive = objectCodeCachePathArc;
    program.id.item = objectCodeCachePathItem;
    
    return true;
  }
  
  bool ProgramCache::WriteObject (const char* source,
    const ProfileLimits& limits, const ProgramObject& program,
    ProgramObjectID& id, csString& failReason)
  {
    if (cache == 0) return false;
    
  #if PROG_CACHE_STORE_SOURCE
    csMemFile stringIDs;
    {
      csStringReader reader (source);
      csString line;
      while (reader.GetLine (line))
      {
	StringStore::ID stringID = shaderPlug->stringStore->GetIDForString (line);
	uint64 diskID = csLittleEndian::UInt64 (stringID);
	stringIDs.Write ((char*)&diskID, sizeof (diskID));
      }
    }
  #endif
    
    CS::Utility::Checksum::MD5::Digest sourceMD5 =
      CS::Utility::Checksum::MD5::Encode (source);
    csString cacheArcPath;
    cacheArcPath.Format ("/%s", sourceMD5.HexString().GetData());
    
    CS::PluginCommon::ShaderCacheHelper::MicroArchive cacheArc;
    {
      csRef<iDataBuffer> cacheArcBuf = cache->ReadCache (cacheArcPath);
      if (cacheArcBuf.IsValid())
      {
	csMemFile cacheArcFile (cacheArcBuf, true);
	cacheArc.Read (&cacheArcFile);
      }
    }
    
  #if PROG_CACHE_STORE_SOURCE
    csString subItem;
    csString entryPrefix (limits.ToString().GetData());
    itemPrefix.Append ("/");
    for (size_t i = 0; i < cacheArc.GetEntriesNum(); i++)
    {
      const char* arcEntry = cacheArc.GetEntryName (i);
      if (strncmp (arcEntry, entryPrefix, entryPrefix.Length()) != 0) continue;
      csRef<iDataBuffer> cacheBuf = cacheArc.ReadEntry (arcEntry);
      if (!cacheBuf.IsValid()) continue;
      
      csRef<iFile> cacheFile;
      cacheFile.AttachNew (new csMemFile (cacheBuf, true));
    
      uint32 diskMagic;
      if (cacheFile->Read ((char*)&diskMagic, sizeof (diskMagic))
	  != sizeof (diskMagic)) continue;
      if (csLittleEndian::UInt32 (diskMagic) != cacheFileMagicCC)
	continue;
      
      csRef<iDataBuffer> cachedIDs =
	CS::PluginCommon::ShaderCacheHelper::ReadDataBuffer (cacheFile);
      
      if (cachedIDs->GetSize() != stringIDs.GetSize())
	continue;
      if (memcmp (cachedIDs->GetData(), stringIDs.GetData(),
	  stringIDs.GetSize()) != 0)
	continue;
	
      subItem = arcEntry;
      break;
    }
    
    if (subItem.IsEmpty())
    {
      uint n = 0;
      csRef<iDataBuffer> item;
      do
      {
	subItem.Format ("%s%u", entryPrefix.GetData(), n++);
	item = cacheArc.ReadEntry (subItem);
      }
      while (item.IsValid());
    }
  #endif
    
    csMemFile cacheFile;
    
    uint32 diskMagic = csLittleEndian::UInt32 (cacheFileMagicCC);
    if (cacheFile.Write ((char*)&diskMagic, sizeof (diskMagic))
	!= sizeof (diskMagic))
    {
      failReason = "write error (magic)";
      return false;
    }
    
  #if PROG_CACHE_STORE_SOURCE
    {
      csRef<iDataBuffer> idsBuffer (stringIDs.GetAllData());
      if (!CS::PluginCommon::ShaderCacheHelper::WriteDataBuffer (&cacheFile,
	  idsBuffer))
      {
	failReason = "write error (source)";
	return false;
      }
    }
  #else
    uint32 diskSize = csLittleEndian::UInt32 ((uint32)strlen (source));
    if (cacheFile.Write ((char*)&diskSize, sizeof (diskSize))
	!= sizeof (diskSize))
    {
      failReason = "write error (source size)";
      return false;
    }
  #endif
    
    if (!program.IsValid())
    {
      uint32 diskState = csLittleEndian::UInt32 (cpsInvalid);
      if (cacheFile.Write ((char*)&diskState, sizeof (diskState))
	  != sizeof (diskState))
      {
	failReason = "write error (state-invalid)";
	return false;
      }
    }
    else
    {
      {
	uint32 diskState = csLittleEndian::UInt32 (cpsValid);
	if (cacheFile.Write ((char*)&diskState, sizeof (diskState))
	    != sizeof (diskState))
	{
	  failReason = "write error (state-valid)";
	  return false;
	}
      }
      
      {
	uint32 diskFlags = csLittleEndian::UInt32 (program.flags);
	if (cacheFile.Write ((char*)&diskFlags, sizeof (diskFlags))
	    != sizeof (diskFlags))
	{
	  failReason = "write error (flags)";
	  return false;
	}
      }
      
      if (!CS::PluginCommon::ShaderCacheHelper::WriteString (&cacheFile,
          program.objectCode))
      {
	failReason = "write error (object code)";
	return false;
      }
      
      csSet<csString>::GlobalIterator iter (program.unusedParams.GetIterator());
      while (iter.HasNext())
      {
	const csString& s = iter.Next();
	if (!CS::PluginCommon::ShaderCacheHelper::WriteString (&cacheFile, s))
	{
	  failReason = "write error (unused param)";
	  return false;
	}
      }
  
      if (!CS::PluginCommon::ShaderCacheHelper::WriteString (&cacheFile, 0))
      {
	failReason = "write error (empty string)";
	return false;
      }
    }
    
    csRef<iDataBuffer> cacheData = cacheFile.GetAllData();
    csString objectCodeCachePathItem;
  #if PROG_CACHE_STORE_SOURCE
    objectCodeCachePathItem = subItem;
  #else
    objectCodeCachePathItem = limits.ToString().GetData();
  #endif
    if (!cacheArc.WriteEntry (objectCodeCachePathItem, cacheData))
    {
      failReason = "failed writing cache entry";
      return false;
    }
    
    csMemFile cacheArcFile;
    bool cacheArcWrite;
    if ((!(cacheArcWrite = cacheArc.Write (&cacheArcFile)))
      || !(cache->CacheData (cacheArcFile.GetData(), cacheArcFile.GetSize(),
	cacheArcPath)))
    {
      if (!cacheArcWrite)
	failReason = "failed writing archive";
      else
	failReason = "failed caching archive";
      return false;
    }
    id.archive = cacheArcPath;
    id.item = objectCodeCachePathItem;
    
    return true;
  }
  
  
}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)
