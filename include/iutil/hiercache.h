/*
    Copyright (C) 2002 by Jorrit Tyberghein
              (C) 2008 by Frank Richter

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

#ifndef __CS_IUTIL_HIERCACHE_H__
#define __CS_IUTIL_HIERCACHE_H__

#include "csutil/ref.h"
#include "csutil/scf_interface.h"

struct iDataBuffer;

/**\file
 * Hierarchical cache manager interface
 */

/**
 * A generic, hierarchical cache system. A client can use this to cache data
 * that was hard to calculate. The data is not converted in any
 * way so it is the responsability of the client to correct platform
 * depending issues like endianess and structure padding.
 *
 * Every item in a cache has a path, similar to a filesystem path. Like these
 * each path component is separated by '/'.
 */
struct iHierarchicalCache : public virtual iBase
{
  SCF_INTERFACE(iHierarchicalCache, 1,0,3);

  /**
   * Cache some data. Returns true if this succeeded.
   * \a path needs to be absolute.
   */
  virtual bool CacheData (const void* data, size_t size,
    const char* path) = 0;

  /**
   * Retrieve some data from the cache. Returns 0 if the
   * data could not be found in the cache. \a path needs to be absolute.
   * \remark Don't modify returned buffer!
   */
  virtual csPtr<iDataBuffer> ReadCache (const char* path) = 0;

  /**
   * Clear items from the cache. Clears all items below the given path.
   * \a path needs to be absolute.
   */
  virtual bool ClearCache (const char* path) = 0;

  /**
   * Ensure that the cached data is written on whatever medium is
   * behind the cache.
   */
  virtual void Flush () = 0;
  
  /**
   * Create a new hierarchical cache which is a view of this cache with the
   * items root at \a base. You can imagine the returned cache prepending
   * \a base to all item requests or storage.
   */
  virtual csPtr<iHierarchicalCache> GetRootedCache (const char* base) = 0;
  
  /// Get cache items directly under \a path.
  virtual csPtr<iStringArray> GetSubItems (const char* path) = 0;
  
  /// Get the cache which is the ultimate hierarchical ancestor of a cache
  virtual iHierarchicalCache* GetTopCache() = 0;
  
  /// Query if cache can be written to (as some caches may be static/read-only)
  virtual bool IsCacheWriteable() const = 0;
};

#endif // __CS_IUTIL_HIERCACHE_H__

