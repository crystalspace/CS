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

#ifndef __CS_UTIL_PLEXHIERCACHE_H__
#define __CS_UTIL_PLEXHIERCACHE_H__

#include "csextern.h"
#include "iutil/cache.h"
#include "iutil/hiercache.h"
#include "csutil/csstring.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"
#include "csutil/set.h"

struct iObjectRegistry;
struct iVFS;

CS_PLUGIN_NAMESPACE_BEGIN(ShaderManager)
{
  class PlexHierarchicalCache : 
    public scfImplementation1<PlexHierarchicalCache, iHierarchicalCache>
  {
  private:
    struct SubCache
    {
      csRef<iHierarchicalCache> cache;
      int priority;
      /// Directories to check for being empty and, if so, to be removed
      csSet<csString> delayedClearDirs;
    };
    static int CacheSort (const SubCache& item,
      iHierarchicalCache* const& key)
    { return item.cache - key; }
    csArray<SubCache> caches;
    bool redundantRemove;
    
    csString GetParentDir (const char* path);
    void DelayClearDirs (SubCache& cache);
  public:
    PlexHierarchicalCache (bool redundantRemove);
    virtual ~PlexHierarchicalCache ();
  
    void AddSubShaderCache (iHierarchicalCache* cache,
      int priority);
    void RemoveSubShaderCache (iHierarchicalCache* cache);
    void RemoveAllSubShaderCaches ();
  
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
    virtual bool IsCacheWriteable() const;
    /** @} */
  };
}
CS_PLUGIN_NAMESPACE_END(ShaderManager)

#endif // __CS_UTIL_PLEXHIERCACHE_H__

