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

#ifndef __CSUTIL_VFSCACHE_H__
#define __CSUTIL_VFSCACHE_H__

#include "iutil/cache.h"

struct iObjectRegistry;
struct iVFS;

/**
 * This is a general cache that can cache data on VFS.
 */
class csVfsCacheManager : public iCacheManager
{
private:
  iObjectRegistry* object_reg;
  char* vfsdir;
  char* current_type;
  char* current_scope;
  iVFS* vfs;

  iVFS* GetVFS ();

  void CacheName (char* buf, const char* type, const char* scope,
  	uint32 id);

public:
  /**
   * Construct the cache manager with the given directory.
   * All cached data will be put somewhere in that directory.
   */
  csVfsCacheManager (iObjectRegistry* object_reg, const char* vfsdir);

  virtual ~csVfsCacheManager ();

  SCF_DECLARE_IBASE;

  virtual void SetCurrentType (const char* type);
  virtual const char* GetCurrentType () const { return current_type; }
  virtual void SetCurrentScope (const char* scope);
  virtual const char* GetCurrentScope () const { return current_scope; }
  virtual bool CacheData (void* data, uint32 size,
  	const char* type, const char* scope, uint32 id);
  virtual iDataBuffer* ReadCache (const char* type, const char* scope,
  	uint32 id);
  virtual bool ClearCache (const char* type = NULL, const char* scope = NULL,
  	const uint32* id = NULL);
};

#endif // __CSUTIL_VFSCACHE_H__

