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
#ifdef DDG
#include "ddgcache.h"
#else
#include "sysdef.h"
#include "csterr/ddgcache.h"
#endif
ddgCache::ddgCache(void)
{
    _nList = 0;
    _fList = 0;
	_size = 0;
	_cacheSize = 0;
	_nodeSize = 0;
}

ddgCache::~ddgCache(void)
{
    delete [] _nList;
    _nList = 0;
    _fList = 0;
}

bool ddgCache::init(unsigned int cacheSize, unsigned int nodeSize )
{
	_cacheSize = cacheSize;
	_nodeSize = nodeSize/(sizeof(unsigned int));
	_size = 0;
    // Pre allocate a block of nodes.
    _nList = new unsigned int[_cacheSize * (_nodeSize+1)];
	// Node 0 is never used because an index of 0 implies that the
	// node next to this one is also available.
	if (!_nList)
		return ddgFailure;

	ddgMemorySet(unsigned int,_cacheSize * _nodeSize);
	reset();
	return ddgSuccess;
}

// Allocator for ddgCacheNodes.
ddgCacheIndex ddgCache::allocNode(void)
{
    ddgAssert(_nList);
    ddgAsserts(_size < _cacheSize,"Ran out of free nodes");
	ddgCacheIndex n = _fList;
	// If the freeList is NULL, then the nodes from this point on are all available.
	_fList = _nList[n*_nodeSize];
	if (_fList == 0)
	{
		// Initialize the next node in preparation for the next alloc.
		_fList = n+1;
		_nList[_fList*_nodeSize] = 0;
		_nList[_fList*_nodeSize+1] = 0xFFFFFFFF;
	}
	_size++;
	if (_size > _maxSize)
		_maxSize = _size;
	return n;
}

// Deallocator for ddgCacheNodes.
void ddgCache::freeNode(ddgCacheIndex n)
{
    ddgAssert(_nList);
    ddgAsserts(_size,"Trying to free node from empty list");
	// Return this node to the free list.
	_nList[n*_nodeSize] = _fList;
	_nList[n*_nodeSize+1] = 0xFFFFFFFF;

	_fList = n;
	_size--;
}

/// Reset the cache
void ddgCache::reset(void)
{
	// Initialize the free list.
	_fList =  1;
	_nList[0*_nodeSize] = 0;
	_nList[0*_nodeSize+1] = 0xFFFFFFFF;
	_nList[1*_nodeSize] = 0;
	_nList[1*_nodeSize+1] = 0xFFFFFFFF;
	_size = 0;
	_maxSize = 0;
}
