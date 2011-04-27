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

#ifndef __CS_UTIL_ROOTEDHIERCACHE_H__
#define __CS_UTIL_ROOTEDHIERCACHE_H__

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
   * iHierarchicalCache providing a 'view' into a path of another cache.
   * This cache takes a 'wrapped' cache and a cache path which serves as
   * the root path of all accesses to this cache. In other words, the
   * given cache path is prepended before passing all cache requests to
   * the 'wrapped' cache.
   */
  class CS_CRYSTALSPACE_EXPORT RootedHierarchicalCache : 
    public scfImplementation1<RootedHierarchicalCache, iHierarchicalCache>
  {
  private:
    csRef<iHierarchicalCache> wrappedCache;
    csString rootdir;
    
    csString AdjustPath (const char* org);
  public:
    /**
     * Construct.
     * \param cache The cache to wrap.
     * \param root The root directory in \a cache for this cache.
     */
    RootedHierarchicalCache (iHierarchicalCache* cache, const char* root)
     : scfImplementationType (this), wrappedCache (cache), rootdir (root) {}
  
    virtual ~RootedHierarchicalCache () {}
  
    /**\name iHierarchicalCache implementation
      * @{ */
    virtual bool CacheData (const void* data, size_t size,
      const char* path)
    { return wrappedCache->CacheData (data, size, AdjustPath (path)); }
    virtual csPtr<iDataBuffer> ReadCache (const char* path)
    { return wrappedCache->ReadCache (AdjustPath (path)); }
    virtual bool ClearCache (const char* path)
    { return wrappedCache->ClearCache (AdjustPath (path)); }
    virtual void Flush ()
    { wrappedCache->Flush (); }
    virtual csPtr<iHierarchicalCache> GetRootedCache (const char* base);
    virtual csPtr<iStringArray> GetSubItems (const char* path)
    { return wrappedCache->GetSubItems (AdjustPath (path)); }
    virtual iHierarchicalCache* GetTopCache()
    { return wrappedCache->GetTopCache(); }
    virtual bool IsCacheWriteable() const
    { return wrappedCache->IsCacheWriteable(); }
    /** @} */
  };
} // namespace Utility
} // namespace CS

#endif // __CS_UTIL_ROOTEDHIERCACHE_H__

