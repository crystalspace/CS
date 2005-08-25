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

#ifndef __CS_UTIL_VFSCACHE_H__
#define __CS_UTIL_VFSCACHE_H__

/**\file
 * VFS Cache Manager
 */
#include "csextern.h"
#include "iutil/cache.h"
#include "csutil/csstring.h"

struct iObjectRegistry;
struct iVFS;

/**
 * This is a general cache that can cache data on VFS.
 */
class CS_CRYSTALSPACE_EXPORT csVfsCacheManager : public iCacheManager
{
private:
  iObjectRegistry* object_reg;
  char* vfsdir;
  char* current_type;
  char* current_scope;
  csRef<iVFS> vfs;
  bool readonly;

  iVFS* GetVFS ();

  void CacheName (csStringFast<512>& buf, const char* type, const char* scope,
  	uint32 id);

public:
  /**
   * Construct the cache manager with the given directory.
   * All cached data will be put somewhere in that directory.
   */
  csVfsCacheManager (iObjectRegistry* object_reg, const char* vfsdir);

  virtual ~csVfsCacheManager ();

  SCF_DECLARE_IBASE;

  virtual void SetReadOnly (bool ro) { readonly = ro; }
  virtual bool IsReadOnly () const { return readonly; }

  /**
   * Set current type. 
   */
  virtual void SetCurrentType (const char* type);
  /**
   * Get current type or 0 if none set.
   */
  virtual const char* GetCurrentType () const { return current_type; }
  /**
   * Set current scope. 
   */
  virtual void SetCurrentScope (const char* scope);
  /**
   * Get current scope or 0 if none set.
   */
  virtual const char* GetCurrentScope () const { return current_scope; }
  /**
   * Cache some data. Returns true if this succeeded.
   */
  virtual bool CacheData (const void* data, size_t size,
  	const char* type, const char* scope, uint32 id);
  /**
   * Retrieve some data from the cache. Returns 0 if the
   * data could not be found in the cache.
   */
  virtual csPtr<iDataBuffer> ReadCache (const char* type, const char* scope,
  	uint32 id);
  /**
   * Clear items from the cache. 
   */
  virtual bool ClearCache (const char* type = 0, const char* scope = 0,
  	const uint32* id = 0);

  /**
   * Flush VFS.
   */
  virtual void Flush ();
};

#endif // __CS_UTIL_VFSCACHE_H__

