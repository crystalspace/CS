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

#ifndef __CS_CSPLUGINCOMMON_SHADER_SHADERCACHEHELPER_H__
#define __CS_CSPLUGINCOMMON_SHADER_SHADERCACHEHELPER_H__

/**\file
 */

#include "iutil/hiercache.h"

#include "csutil/fifo.h"
#include "csutil/md5.h"
#include "csutil/memfile.h"
#include "csutil/mempool.h"
#include "csutil/ref.h"
#include "csutil/set.h"

struct iDataBuffer;
struct iDocumentNode;
struct iDocumentSystem;
struct iFile;
struct iObjectRegistry;

/**\addtogroup plugincommon
 * @{ */

namespace CS
{
  namespace PluginCommon
  {
    namespace ShaderCacheHelper
    {
      /**
       * Computes a hash for all referenced documents documents (ie external 
       * files pulled in via "file" attributes or ?Include? PIs) of a shader
       * document.
       */
      class CS_CRYSTALSPACE_EXPORT ShaderDocHasher
      {
        struct DocStackEntry
        {
          csRef<iDocumentNode> docNode;
          csRef<iDataBuffer> sourceData;
          csString fullPath;
          
          CS::Utility::Checksum::MD5::Digest ComputeHash ();
        };
        csFIFO<DocStackEntry> scanStack;
        void PushReferencedFiles (DocStackEntry& entry);
        void PushReferencedFiles (iDocumentNode* node);
        bool AddFile (const char* filename);
      
        csRef<iDocumentSystem> docSys;
        csRef<iVFS> vfs;
        csMemFile actualHashes;
        csSet<csString> seenFiles;
      public:
        /**
         * Set up a hasher for the given node.
         * \remarks A hash for \a doc itself is <b>not</b> recorded!
         */
        ShaderDocHasher (iObjectRegistry* objReg, iDocumentNode* doc);
        
        /// Get the hash data for the given node and all included documents
        csPtr<iDataBuffer> GetHashStream ();
        /// Check if the given hash data identifies the given node
        bool ValidateHashStream (iDataBuffer* stream);
      };
      
      /// Write a complete data buffer to a file
      CS_CRYSTALSPACE_EXPORT bool WriteDataBuffer (iFile* file, iDataBuffer* buf);
      /// Get a complete data buffer written with WriteDataBuffer from a file
      CS_CRYSTALSPACE_EXPORT csPtr<iDataBuffer> ReadDataBuffer (iFile* file);
      
      /// Write a character string to a file
      CS_CRYSTALSPACE_EXPORT bool WriteString (iFile* file, const char* str);
      /// Read a character string from a file
      CS_CRYSTALSPACE_EXPORT csString ReadString (iFile* file);
      
      /// Helper to write strings in an efficient way (each string once)
      class CS_CRYSTALSPACE_EXPORT StringStoreWriter
      {
        csMemFile strings;
        csHash<uint32, csString> stringPositions;
        csRef<iFile> file;
        size_t headPos;
      public:
        /**
         * Start using a string store. Strings will ultimatively be written
         * to file. Call EndUse() after use.
         */
        bool StartUse (iFile* file);
        /**
         * End string store usage.
         * \remarks This will write more data to the file!
         */
        bool EndUse ();
        
        /// Get an ID for a string.
        uint32 GetID (const char* string);
      };
      
      /// Helper to read strings written with StringStoreWriter
      class CS_CRYSTALSPACE_EXPORT StringStoreReader
      {
        csRef<iFile> file;
        const char* stringBlock;
        csRef<iDataBuffer> blockBuf;
        size_t endPos;
      public:
        /**
         * Start using a string store. 
         * \remarks The place this is called when reading must be mirror the 
         *  place this is called when writing the file!
         */
        bool StartUse (iFile* file);
        /**
         * End using the string store. 
         * \remarks The place this is called when reading must be mirror the 
         *  place this is called when writing the file!
         */
        bool EndUse ();
        
        /// Get a string for an ID
        const char* GetString (uint32 id) const;
      };
      
      /**
       * A simple archive format.
       * It is not terribly efficient to add or modify files in it. Also,
       * archive data is kept in memory, so it's not suitable for very large
       * data. However, it's a simple format with little overhead, ideal to
       * 'pack' a set of small files together.
       */
      class CS_CRYSTALSPACE_EXPORT MicroArchive
      {
        csRef<iDataBuffer> originalData;
        csMemoryPool addedNames;
        struct Entry
        {
          /// Name, points either into originalData or addedNames
          const char* name;
          /// Buffer with data (either added or pointing inside originalData)
          csRef<iDataBuffer> data;
          /// Size of data
          size_t size;
          /// Offset of data in originalData
          size_t offset;
        };
        csArray<Entry> entries;
	bool dirty;
        
        iDataBuffer* GetEntryData (Entry& entry);
        Entry* FindEntry (const char* id);
      public:
	MicroArchive() : dirty (false) {}

        bool Read (iFile* file);
        bool Write (iFile* file);
      
        iDataBuffer* ReadEntry (const char* id);
        bool WriteEntry (const char* id, iDataBuffer* data);
        bool DeleteEntry (const char* id);
        void DeleteAllEntries ();
        
        size_t GetEntriesNum () const
        { return entries.GetSize(); }
        const char* GetEntryName (size_t index) const
        { return entries[index].name; }
        iDataBuffer* GetEntryData (size_t index)
        { return GetEntryData (entries[index]); }
        void DeleteEntry (size_t i) { entries.DeleteIndex (i); }
      };
      
      /**
       * iHierarchicalCache implementation storing everything in a
       * MicroArchive.
       */
      class CS_CRYSTALSPACE_EXPORT MicroArchiveCache :
        public scfImplementation1<MicroArchiveCache, iHierarchicalCache>
      {
        MicroArchive archive;
        csRef<iHierarchicalCache> parentCache;
        csString cacheItem;
      public:
        /**
         * Construct.
         * \param parentCache The cache in which the archive will be stored
         * \param cacheItem The path of the cache item for the archive
         */
        MicroArchiveCache (iHierarchicalCache* parentCache,
          const char* cacheItem);
        ~MicroArchiveCache();
        
	/**\name iHierarchicalCache implementation
	* @{ */
	virtual bool CacheData (const void* data, size_t size,
	  const char* path);
	virtual csPtr<iDataBuffer> ReadCache (const char* path);
	virtual bool ClearCache (const char* path);
	virtual void Flush ();
	virtual csPtr<iHierarchicalCache> GetRootedCache (const char* base);
	virtual csPtr<iStringArray> GetSubItems (const char* path);
	virtual iHierarchicalCache* GetTopCache();
	virtual bool IsCacheWriteable() const
	{ return parentCache->IsCacheWriteable(); }
	/** @} */
      };
    } // namespace ShaderCacheHelper
  } // namespace PluginCommon
} // namespace CS

/** @} */

#endif // __CS_CSPLUGINCOMMON_SHADER_SHADERCACHEHELPER_H__

