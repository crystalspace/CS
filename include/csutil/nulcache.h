/*
    Copyright (C) 2002 by Frank Richter

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

#ifndef __CSUTIL_NULCACHE_H__
#define __CSUTIL_NULCACHE_H__

/**\file
 * 'Null' Cache Manager
 */
#include "iutil/cache.h"

/**
 * This is a general cache that doesn't cache anything at all.
 */
class csNullCacheManager : public iCacheManager
{
private:
public:
  /**
   * Does nothing.
   */
  csNullCacheManager ();

  virtual ~csNullCacheManager ();

  SCF_DECLARE_IBASE;

  /**
   * Does nothing.
   */
  virtual void SetCurrentType (const char* type);
  /**
   * Always returns NULL.
   */
  virtual const char* GetCurrentType () const { return NULL; }
  /**
   * Does nothing. 
   */
  virtual void SetCurrentScope (const char* scope);
  /**
   * Always returns NULL.
   */
  virtual const char* GetCurrentScope () const { return NULL; }
  /**
   * Always returns false.
   */
  virtual bool CacheData (void* data, uint32 size,
  	const char* type, const char* scope, uint32 id);
  /**
   * Always returns NULL.
   */
  virtual csPtr<iDataBuffer> ReadCache (const char* type, const char* scope,
  	uint32 id);
  /**
   * Does nothing. 
   */
  virtual bool ClearCache (const char* type = NULL, const char* scope = NULL,
  	const uint32* id = NULL);
};

#endif // __CSUTIL_NULCACHE_H__

