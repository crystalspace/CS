/*
    Copyright (C) 1997, 1998, 1999, 2000 by Alex Pfaffe
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

#include "util/ddgerror.h"
/**
 * This object manages a cache of preallocated memory in fixed sized blocks. <br>
 * allocation time is O(k).												<br>
 * deallocation time is O(k)											<br>
 * access time is by cache index is O(k).								<br>
 * reset (free all) time is O(k)										<br>
 * These times are all constant.										<br>
 * Memory overhead is 2 bytes per node.									<br>
 * The items being stored are unordered.								<br>
 * Blocks must be at n*2 bytes in size.									<br>
 * The cache can hold at most 0xFFFE entries.							<br>
 * You can iterate across the nodes in constant time.					<br>
 * You cannot remove individual cache nodes.							<br>
 * And index of 0 implies a NULL node.
 */
typedef unsigned short ddgCacheIndex;

class WEXP ddgCacheNode
{
	friend class ddgCache;
	/// Index of next Node for this binTree.						(2 byte)
	ddgCacheIndex	_next;
public:
	/// Default constructor.
	ddgCacheNode(void) : _next(0) {}
	/// Return the next node in the chain.
	inline ddgCacheIndex next(void) { return _next; }
	/// Set the next node in the chain.
	inline void next(ddgCacheIndex ci) { _next = ci; }
};

class WEXP ddgCache
{
	/// The buffer of all allocated node entries.
	ddgCacheNode * _nList;
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
	inline ddgCacheNode *get(ddgCacheIndex i)
	{
		ddgAsserts(i >= 0 && i < _cacheSize,"Invalid cache index");
		return &(_nList[i*_nodeSize]);
	}
};


/**
 * Linked cache node.
 * the nodes in this cache are connected.
 * Max number of entries this cache can hold is 0xFFFF.
 * The way this cache works is as follows
 */
class WEXP ddgLNode : public ddgCacheNode {
	friend class ddgLCache;
private:
	/// Previous queue node. (To support removing entries)			(2 byte)
	ddgCacheIndex _prev;
public:
	/// Set the previous node.
	void prev(ddgCacheIndex ci ) { _prev = ci; }
	/// Get the previous node.
	ddgCacheIndex prev(void) { return _prev; }
};

/**
 *  Linked cache.
 *  This cache can insert and remove items at will.
 */
class WEXP ddgLCache : public ddgCache {
	typedef ddgCache super;
public:
	/// Get entry.
	inline ddgLNode* get(ddgCacheIndex n)
	{
		return (ddgLNode*) super::get(n);
	}
	/**
	 * Insert a node into the cache before the specified node.
	 * returns the position in the cache that it was inserted into.
	 */
	inline ddgCacheIndex insert(ddgCacheIndex nextNode)
	{
		ddgCacheIndex ci = allocNode();
		// Initialize the new node.
		ddgLNode *n = get(ci);
		// Insert at head of the list.
		n->next(nextNode);
		if (nextNode)
		{
			n->prev(get(nextNode)->prev());
			// Update the neighbours.
			if (n->prev())
				get(n->prev())->next(ci);
			get(n->next())->prev(ci);
		}
		else
			n->prev(0);
		ddgAssert(ci);
		return ci;
	}
	/// Remove this node from the cache.
	inline void remove(ddgCacheIndex ci)
	{
		ddgAssert(ci);
		ddgLNode *n = get(ci);
		if (n->prev())
		{
			get(n->prev())->next(n->next());
		}
		if (n->next())
		{
			get(n->next())->prev(n->prev());
		}
		freeNode(ci);
	}
	/**
	 * Insert a node into the cache before the head node.
	 * Assumes we are inserting at the head of the list. 
	 */
	inline ddgCacheIndex insertHead(ddgCacheIndex nextNode)
	{
		ddgCacheIndex ci = allocNode();
		// Initialize the new node.
		ddgLNode *n = get(ci);
		// Insert at head of the list.
		n->next(nextNode);
		if (nextNode)
		{
			get(n->next())->prev(ci);
		}
		n->prev(0);
		ddgAssert(ci);
		return ci;
	}
	/**
	 * Insert a node into the cache before the head node.
	 * Assumes we are inserting at the tail of the list. 
	 */
	inline ddgCacheIndex insertTail(ddgCacheIndex prevNode)
	{
		ddgCacheIndex ci = allocNode();
		// Initialize the new node.
		ddgLNode *n = get(ci);
		// Insert at head of the list.
		n->prev(prevNode);
		if (prevNode)
		{
			get(n->prev())->next(ci);
		}
		n->next(0);
		ddgAssert(ci);
		return ci;
	}
};




/**
 * Sorted cache.
 * This cache keeps items sorted by index.
 */
class WEXP ddgSNode : public ddgLNode {
	friend class ddgSCache;
private:
	/// Bucket of this node.				(2 byte)
	short   _bucket;

public:
	/// Get the bucket that this node is in.
	inline short bucket(void) { return _bucket; }
	/// Set the bucket that this node is in.
	inline void bucket(short b) { _bucket = b; }
};


/**
 *  This class caches a roughly sorted set of triangles.
 */
class WEXP ddgSCache : public ddgLCache {
	typedef ddgLCache super;

	/// The bucket of queue entries.
	ddgCacheIndex	*_bucket;
	/// The bucket with the item of highest priority in the queue.
	ddgCacheIndex	_head;
	/// The bucket with the item of lowest priority in the queue.
	ddgCacheIndex	_tail;
	/// The number of buckets to use.
	short	_bucketNo;
	/// Sort cache in reverse.
	bool _reverse;

public:
	/// Constructor.
	ddgSCache(void)
	{
		_bucket = 0;
		_bucketNo = 0;
		_head = 0;
		_tail = 0;
	}
	/// Destructor.
	~ddgSCache(void)
	{
		delete[] _bucket;
		_head = 0;
		_tail = 0;
		_bucket = 0;
	}
	/// Reset the queue.
	inline void reset(void)
	{
		ddgCache::reset();
		_head = 0;
		_tail = 0;
		unsigned int i = _bucketNo;
		while (i)
			_bucket[--i] = 0;
	}
	/// Initialize the cache.
	void init (unsigned int size, unsigned int nodeSize, unsigned int bn, bool r = false ) {
		ddgAssert(bn < 0xFFFF);
		_bucketNo = bn;
		_reverse = r;
		super::init(size,nodeSize);
		_bucket = new ddgCacheIndex[_bucketNo];
		ddgAsserts(_bucket, "Failed to allocate memory.");
		ddgMemorySet(ddgCacheIndex,_bucketNo);
		reset();
	}
	/// Get entry.
	inline ddgSNode* get(unsigned short index)
	{
		return (ddgSNode*) ddgCache::get(index);
	}
	/// Return the number of buckets/key slots.
	inline short bucketNo(void) { return _bucketNo; }
	/// Convert bucket # back to key for reversed cache.
	inline short convert( short b )
	{
		return _reverse ? _bucketNo - b - 1 : b;
	}
	/**
	 * Insert a node into the cache.
	 * returns the position in the cache that it was inserted into.
	 */
	inline ddgCacheIndex insert(short b)
	{
		ddgAsserts(b >= 0 && b < _bucketNo,"Bucket key is out of range!");
		ddgAsserts(_bucket, "Failed to allocate memory.");
		if (_reverse)
			b = _bucketNo - b - 1;
		ddgCacheIndex ci = super::insert(_bucket[b]);
		// Initialize the new node.
		ddgSNode *n = get(ci);
		n->bucket(b);
		// We are now the head of the list.
		_bucket[b] = ci;
		// Reset the head and tail of the queue if need be.
		if (_head < ddgCacheIndex(b))
			_head = b;
		else if (_tail > ddgCacheIndex(b))
			_tail = b;
		ddgAssert(ci);
		return ci;
	}
	/// Remove this node from the queue.
	inline void remove(ddgCacheIndex ci)
	{
		ddgAssert(ci);
		ddgSNode *n = get(ci);
		short		b = n->bucket();
		ddgAssert(b >= 0 && b < _bucketNo);
		ddgAsserts(_bucket, "Failed to allocate memory.");
		// If the deleted node was the head of the queue update the head.
		if (_bucket[b] == ci)
		{
			_bucket[b] = n->next();
		}
		// Remove the actual item and free it.
		super::remove(ci);
	}
	/// Return the item at the head of the queue.
	inline ddgCacheIndex head(void)
	{
		while (!_bucket[_head] && _head > 0 )
			_head--;
		return _bucket[_head];
	}
	/// Return the item at the head of the queue.
	inline ddgCacheIndex tail(void)
	{
		while (!_bucket[_tail] && _tail < _bucketNo-1 )
			_tail++;
		return _bucket[_tail];
	}
	/// Return the item following this one ci must be valid.
	inline ddgCacheIndex next(ddgCacheIndex ci)
	{
		ddgAssert(ci);
		ddgSNode	*n = get(ci);
		short		b = n->bucket();
		const short	blim = (short)_tail;
		if (n->next())
			return n->next();
		// Find next bucket with something in it.
		while (--b > blim && !_bucket[b] ){}
		return b < 0 ? 0 : _bucket[b];
	}
};
#endif
