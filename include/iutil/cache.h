/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS_IUTIL_CACHE_H__
#define __CS_IUTIL_CACHE_H__

#include "csutil/scf.h"
#include "iutil/databuff.h"

/**\file
 * Cache manager interface
 */

SCF_VERSION (iCacheManager, 0, 0, 1);

/**
 * A generic cache system. A client can use this to cache data
 * that was hard to calculate. The data is not converted in any
 * way so it is the responsability of the client to correct platform
 * depending issues like endianess and structure padding.
 * <p>
 * Every item in a cache has a 'type', a 'scope', and a unique id.
 * The 'type' can be something like 'lightmap', 'octree', ...
 * The 'scope' can be something like 'myRegion_mySector', ...
 * The 'id' is a unique ID for the cached item in the given type
 * and scope. ~0 can be taken for the cases "no ID" or "ID unused".
 * Avoid using '/' and '\' characters in both type and scope as this
 * may cause conflicts with the given cache (if it caches on a file
 * system for example).
 * <p>
 * Note that both 'type' and 'scope' can be made default using
 * SetCurrentType() and SetCurrentScope(). If those are set then
 * you can use 0 for type and/or scope in CacheData() and
 * ReadCache(). If you don't use 0 then the given value will
 * override the default values.
 */
struct iCacheManager : public iBase
{
  /**
   * Set current type. This will be used in CacheData() and
   * ReadCache() when the given 'type' there is 0.
   */
  virtual void SetCurrentType (const char* type) = 0;

  /**
   * Get current type or 0 if none set.
   */
  virtual const char* GetCurrentType () const = 0;

  /**
   * Set current scope. This will be used in CacheData() and
   * ReadCache() when the given 'scope' there is 0.
   */
  virtual void SetCurrentScope (const char* scope) = 0;

  /**
   * Get current scope or 0 if none set.
   */
  virtual const char* GetCurrentScope () const = 0;

  /**
   * Cache some data. Returns true if this succeeded.
   */
  virtual bool CacheData (const void* data, size_t size,
  	const char* type, const char* scope, uint32 id) = 0;

  /**
   * Retrieve some data from the cache. Returns 0 if the
   * data could not be found in the cache.
   * \remark Returned buffer is NOT null-terminated. 
   * \remark Don't modify returned buffer!
   */
  virtual csPtr<iDataBuffer> ReadCache (
  	const char* type, const char* scope, uint32 id) = 0;

  /**
   * Clear items from the cache. There are four ways to call
   * this function:
   * <ul>
   * <li>0, 0, 0: clear entire cache.
   * <li>'type', 0, 0: clear everything of this type.
   * <li>'type', 'scope', 0: clear everything of this type and scope.
   * <li>'type', 'scope', id: clear the specific item.
   * </ul>
   * Returns true if items were deleted. Returns false if item was not
   * found or deletion is not possible.
   */
  virtual bool ClearCache (const char* type = 0, const char* scope = 0,
  	const uint32* id = 0) = 0;

  /**
   * Ensure that the cached data is written on whatever medium is
   * behind the cache.
   */
  virtual void Flush () = 0;
};

#endif // __CS_IUTIL_CACHE_H__

