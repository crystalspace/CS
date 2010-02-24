/*
    Copyright (C) 2008 by Frank Richter

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

#ifndef __CS_UTIL_VFSHIERCACHE_H__
#define __CS_UTIL_VFSHIERCACHE_H__

/**\file
 * VFS Hierarchical Cache
 */
#include "csextern.h"
#include "iutil/cache.h"
#include "iutil/hiercache.h"
#include "csutil/csstring.h"
#include "csutil/scf_implementation.h"

struct iObjectRegistry;
struct iVFS;

namespace CS
{
  namespace Utility
  {
    /**
    * This is a general cache that can cache data on VFS.
    */
    class CS_CRYSTALSPACE_EXPORT VfsHierarchicalCache : 
      public scfImplementation1<VfsHierarchicalCache, iHierarchicalCache>
    {
    private:
      csRef<VfsHierarchicalCache> parent;
      csString vfsdir;
      csRef<iVFS> vfs;
      bool readonly;
      
      VfsHierarchicalCache (VfsHierarchicalCache* parentCache, const char* vfsdir);
    
      /// Makes sure all parts of \a path are directories
      void EnsureDirectories (const char* path);
      /// Makes sure the last part of \a path is not a directory
      void EnsureFile (const char* path);
      /// Revursively delete \a path and all directories below it
      bool RecursiveDelete (const char* path);
    public:
      /**
       * Construct the cache with the given directory.
       * All cached data will be put somewhere in that directory.
       */
      VfsHierarchicalCache (iObjectRegistry* object_reg, const char* vfsdir);
    
      virtual ~VfsHierarchicalCache ();
    
      void SetReadOnly (bool ro) { readonly = ro; }
      bool IsReadOnly () const { return readonly; }
    
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
      virtual bool IsCacheWriteable() const { return !readonly; }
      /** @} */
    };
  } // namespace Utility
} // namespace CS

#endif // __CS_UTIL_VFSHIERCACHE_H__

