/*
    Copyright (C) 1997, 1998, 1999 by Alex Pfaffe
	(Digital Dawn Graphics Inc)
  
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
#ifndef _ddgCache_Class_
#define _ddgCache_Class_
#ifdef DDG
#include "ddgerror.h"
#else
#include "types.h"
#include "csterr/ddg.h"
#endif
/**
 * This object manages a cache of preallocated memory in fixed sized blocks. <br>
 * allocation time is O(k).												<br>
 * deallocation time is O(k)											<br>
 * access time is by cache index is O(k).								<br>
 * reset (free all) time is O(k)										<br>
 * These times are all constant.										<br>
 * Memory overhead is 0 bytes per node.									<br>
 * Iteration through the cache is order O(m) where s is the max items used in the cache size.<br>
 * The items being stored are unordered.								<br>
 * A cache entry with 0xFFFFFFFF in the 2nd 8 bytes is considered empty.<br>
 * Blocks must be at least 8 bytes in size.								<br>
 * The cache can hold at most 0xFFFFFFFE entries.						<br>
 */
typedef unsigned short ddgCacheIndex;

class WEXP ddgCache
{
	/// The buffer of all allocated node entries.
	unsigned int * _nList;
	/// Header to the List of free nodes.
	ddgCacheIndex _fList;
	/// The number of entries that can be managed by this cache.
	unsigned int _cacheSize;
	/// The number of elements currently in use in the cache.
	unsigned int _size;
	/// Node Size.
	unsigned int _nodeSize;
	/// Maximum number of items in use.
	unsigned int _maxSize;

public:
	/** 
	 * Constructor.  The number of items to be allocated in the node
	 * cache can be specified only at construction time.
	 */
	ddgCache(void );
    /// Destructor.
    ~ddgCache(void);
	/// Initialize the cache.
	bool init( unsigned int cacheSize, unsigned int nodeSize );
	/// Allocator for ddgCacheNodes.
	ddgCacheIndex allocNode(void);
	/// Deallocator for ddgSplayNodes.
	void freeNode(ddgCacheIndex n);
	/// Return the current cache size.
	inline unsigned int size(void) { return _size; }
	/// Reset the cache.
	void reset(void);
	/// Get the nth item.
	inline unsigned int *get(ddgCacheIndex i) { return &(_nList[i*_nodeSize]); }
	/**
	 * Get the next item.  Initialize by calling with 0, returns 0 at end.
	 * Eg.
	 * ddgCacheIndex ci = 0;
	 * while (ci = cache(ci)) { do someting }
	 */
	inline ddgCacheIndex next( ddgCacheIndex cur )
	{
		while(cur < _maxSize)
		{
			cur++;
			// See if there is an item here.
			if (_nList[cur*_nodeSize+1] != 0xFFFFFFFF)
				return cur;
		}
		return 0;
	}
};


#endif
