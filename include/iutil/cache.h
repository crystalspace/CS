/*
    Copyright (C) 2002 by Jorrit Tyberghein

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __IUTIL_CACHE_H__
#define __IUTIL_CACHE_H__

#include "csutil/scf.h"

struct iDataBuffer;

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
 * and scope.
 * Avoid using '/' and '\' characters in both type and scope as this
 * may cause conflicts with the given cache (if it caches on a file
 * system for example).
 * <p>
 * Note that both 'type' and 'scope' can be made default using
 * SetCurrentType() and SetCurrentScope(). If those are set then
 * you can use NULL for type and/or scope in CacheData() and
 * ReadCache(). If you don't use NULL then the given value will
 * override the default values.
 */
struct iCacheManager : public iBase
{
  /**
   * Set current type. This will be used in CacheData() and
   * ReadCache() when the given 'type' there is NULL.
   */
  virtual void SetCurrentType (const char* type) = 0;

  /**
   * Get current type or NULL if none set.
   */
  virtual const char* GetCurrentType () const = 0;

  /**
   * Set current scope. This will be used in CacheData() and
   * ReadCache() when the given 'scope' there is NULL.
   */
  virtual void SetCurrentScope (const char* scope) = 0;

  /**
   * Get current scope or NULL if none set.
   */
  virtual const char* GetCurrentScope () const = 0;

  /**
   * Cache some data. Returns true if this succeeded.
   */
  virtual bool CacheData (void* data, uint32 size,
  	const char* type, const char* scope, uint32 id) = 0;

  /**
   * Retreive some data from the cache. Returns NULL if the
   * data could not be found in the cache.
   * The returned data buffer should be DecRef()'ed if you are
   * ready with it.
   */
  virtual iDataBuffer* ReadCache (
  	const char* type, const char* scope, uint32 id) = 0;

  /**
   * Clear items from the cache. There are four ways to call
   * this function:
   * <ul>
   * <li>NULL, NULL, NULL: clear entire cache.
   * <li>'type', NULL, NULL: clear everything of this type.
   * <li>'type', 'scope', NULL: clear everything of this type and scope.
   * <li>'type', 'scope', id: clear the specific item.
   * </ul>
   * Returns true if items were deleted. Returns false if item was not
   * found or deletion is not possible.
   */
  virtual bool ClearCache (const char* type = NULL, const char* scope = NULL,
  	const uint32* id = NULL) = 0;
};

#endif // __IUTIL_CACHE_H__

