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
#ifndef _ddgSplay_Class_
#define _ddgSplay_Class_

#include "csterr/ddgerror.h"

/**
 * This class defines the data element stored by the ddgSplay tree.
 * this class can be subclassed with other members which map to
 * an unsigned int.
 * The key is a 64 bit value.
 */
class WEXP ddgSplayKey
{
	/// Key by which the ddgSplay tree is sorted.
	unsigned int _key;
	/// Value stored by the ddgSplay tree.
    union {
        /// Index and Tree and bits combined (for faster comparison).
        unsigned int _value;
        /// The value broken into pieces.
        struct {
            /// The tree index.
            unsigned short _tree;
            /// Triangle index.
	        unsigned short _index;
        } _ti;
    };

public:
	/// The default constructor.
    inline ddgSplayKey( void ) : _key(0), _value(0) { }
	/// The initializing constructor.
	inline ddgSplayKey( unsigned int tree, unsigned int tindex, unsigned int priority )
    {_key = priority; _ti._index = tindex; _ti._tree = tree; }
	/// Set the value.
	inline void set( ddgSplayKey sk )
    { _key = sk._key; _ti._index = sk._ti._index; _ti._tree = sk._ti._tree; }
	/// Compare two keys.
	inline int compare( ddgSplayKey sk )
	{
		if (_key < sk._key) return -1;
		if (_key > sk._key) return 1;
#if (defined(OS_SOLARIS))
		if (_ti._tree < sk._ti._tree) return -1;
		if (_ti._tree > sk._ti._tree) return 1;
		if (_ti._index < sk._ti._index) return -1;
		if (_ti._index > sk._ti._index) return 1;
#else
		if (_value < sk._value) return -1;
		if (_value > sk._value) return 1;
#endif
		// They are equal.
		return 0; 
	}
	/// Get the triangle index part of the key.
	inline unsigned int index(void)
	{
		return _ti._index;
	}
	/// Get the tree part of the key.
	inline unsigned int tree(void)
	{
		return _ti._tree;
	}
	/// Get the tree part of the key.
	inline unsigned int key(void)
	{
		return _key;
	}
};

// 16 bit index allows for ddgSplay trees to maintain up to ~65000 entries.
class ddgSplayTree;
class ddgSplayIterator;
class ddgSplayNode;
/**
 * ddgSplayNodes are managed by a ddgSplay tree.
 * ddgSplay nodes are not allocated or freed but instead are
 * retrieved and released from a preallocated ddgSplayNode cache
 * of fixed size.  This cache is shared by all ddgSplayTrees.
 * key = 64 bits, left = 32, right = 32 = 128 bits.
 * TODO: PERF rewrite using parent pointer.
 */
typedef ddgSplayNode *ddgSplayIndex;
class WEXP ddgSplayNode
{
	friend ddgSplayTree;
	friend ddgSplayIterator;
	/// The Implementation specific Data stored by a ddgSplay tree entry.
	ddgSplayKey		_key;
	/// The left child (also used by the free list to point to next free node).
    ddgSplayIndex     _left;
	/// The right child.
    ddgSplayIndex     _right;
public:
	/// Set the key.
	inline void set( ddgSplayKey k ) { _key.set(k); }
};

/**
 * The ddgSplay Tree Class is a semi balanced binary tree which guarantees
 * amortized O(ln(n)) access time for insert/remove and find operations.
 * Individual accesses may not be O(ln(n)).
 * Operations carried out on this tree cause the requested item
 * to become the root of the tree.  This allows nearby nodes to
 * found very quickly on subsequent accesses.
 * There are two sets of functions, the first require acess through
 * a specified ddgSplayNode.  The second set work from the
 * current root node.
 * <br>
 * Note: To find the minimum/maximum and to iterate over this
 * tree without disturbing the tree, use the ddgSplayIterator class.
 */
class WEXP ddgSplayTree
{
	/// The buffer of all allocated node entries for all ddgSplay trees.
	static ddgSplayIndex _nList;
	/// Header to the List of free nodes.
	static ddgSplayIndex _fList;
    /// Number of active ddgSplayTrees.
    static unsigned int _refCount;
	/// The number of entries that can be managed by this ddgSplay tree.
	static unsigned int _maxSize;
    /// The minimum item in the tree
    static ddgSplayKey _minNode;
    /// The maximum item in the tree
    static ddgSplayKey _maxNode;
	/// The node that represents a left or terminal node.
	ddgSplayIndex _nullNode;
	/// Internal node for insertion.
    ddgSplayIndex _header;
	/// The number of elements currently in the tree.
	unsigned int _size;

	/// ddgSplay the tree for this item.
	ddgSplayIndex ddgSplay( ddgSplayKey Item, ddgSplayIndex n );
	/// Rebalance the tree for node with left children.
	ddgSplayIndex SingleRotateWithLeft( ddgSplayIndex n );
	/// Rebalance the tree for node with right children.
	ddgSplayIndex SingleRotateWithRight( ddgSplayIndex n );
public:
	/** 
	 * Constructor.  The number of items to be allocated in the node
	 * cache can be specified only at construction time.
	 */
	ddgSplayTree(void);
    /// Destructor.
    ~ddgSplayTree(void);
	/// Release node and all child nodes.
	ddgSplayIndex clear( ddgSplayIndex n );
	/// Find the node corresponding to X.
	inline ddgSplayIndex find( ddgSplayKey k, ddgSplayIndex n )
    { return ddgSplay( k, n ); }
    /// Find the minimum value in the tree.
    inline ddgSplayIndex findMin( ddgSplayIndex n );
	/// Find the maximum value in the tree.
    inline ddgSplayIndex findMax( ddgSplayIndex n );
	/// Insert a node.
	ddgSplayIndex insert( ddgSplayKey k, ddgSplayIndex n );
	/// Remove a node.
	ddgSplayIndex remove( ddgSplayKey k, ddgSplayIndex n );

private:
	/// The root of the tree. Below are all the root operations.
	ddgSplayIndex _root;
public:
	/// Release node and all child nodes.
	inline void clear( void  )       { _root = clear(_root); }
	/// Find the node corresponding to X.
	inline void find( ddgSplayKey ti )  { _root = find(ti,_root); }
	/// Find the minimum value in the tree.
	inline void findMin( void )      { _root = findMin(_root); }
	/// Find the maximum value in the tree.
	inline void findMax( void )      { _root = findMax(_root); }
	/// Insert a node.
	inline void insert( ddgSplayKey k ) { _root = insert(k,_root); }
	/// Remove a node.
	inline void remove( ddgSplayKey k ) { _root = remove(k,_root); }
	/// Insert a node.
	inline void insert( unsigned int tree, unsigned int tindex, unsigned int priority )
    { _root = insert(ddgSplayKey(tree,tindex,priority),_root); }
	/// Remove a node.
	inline void remove( unsigned int tree, unsigned int tindex, unsigned int priority )
    { _root = remove(ddgSplayKey(tree,tindex,priority),_root); }

	/// Return the root node.
	inline ddgSplayIndex root(void)     { return _root; }
	/// Return the index stored by this element.
    inline ddgSplayKey * retrieve(void) { return &(_root->_key); }
	/// Set the key and value stored by this element.
	inline void data(ddgSplayKey k )    { _root->set(k); }
	/// Get the left child node for this index.
	inline ddgSplayIndex left(ddgSplayIndex si)
	{ return si->_left; }
	/// Get the right child node for this index.
	inline ddgSplayIndex right(ddgSplayIndex si)
	{ return si->_right; }
	/// Set the left child node for this index.
	inline void left(ddgSplayIndex si, ddgSplayIndex sil)
	{ si->_left = sil; }
	/// Set the right child node for this index.
	inline void right(ddgSplayIndex si, ddgSplayIndex sir)
	{ si->_right = sir; }
	/// Get the key for this index.
	inline ddgSplayKey key(ddgSplayIndex si)
	{ return si->_key; }
	/// Get the key for this index.
	inline void key(ddgSplayIndex si, ddgSplayKey sk)
	{ si->_key.set(sk); }
	/// Return the index stored by this element.
	inline ddgSplayKey * retrieve(ddgSplayIndex si)
    { return &(si->_key); }
	/// Set the index and tree stored by this element.
	inline void data(ddgSplayKey k, ddgSplayIndex si )
    { si->set(k); }

	/// Allocator for ddgSplayNodes.
	ddgSplayIndex allocNode(void);
	/// Deallocator for ddgSplayNodes.
	void freeNode(ddgSplayIndex n);
	/// Return the current tree size.
	unsigned int size(void);
	/// Return the null node.
	inline ddgSplayIndex null(void) { return _nullNode; }
	/// Return if the given node is null.
	inline bool isnull(ddgSplayIndex n) { return _nullNode == n; }
#ifdef _DEBUG
	/// Print the current content of the tree.
	int printTree( ddgSplayIndex n );
#endif
};

/**
 * An iterator to traverse the ddgSplay tree without
 * disturbing the tree.
 */
class WEXP ddgSplayIterator
{
	/// The maximum number of levels in the tree.
#define stackMax 1000
	/// The ddgSplay tree over which to iterate.
	ddgSplayTree	*_ddgSplaytree;
	/// The stack. At most stackMax levels deep
	ddgSplayIndex	_stack[stackMax];
	/// The current node in the tree.
	ddgSplayIndex	_current;
	/// Current stack depth.
	unsigned int _depth;
#ifdef _DEBUG
	/// Keep track of the max recorded depth.
	unsigned int _maxDepth;
#endif
public:
	/// Move the iterator to the minimum/maximum node of the tree.
	void reset(bool reverse = false)
	{
#ifdef _DEBUG
		_maxDepth = 0;
#endif
		_depth = 0;
		_current = _ddgSplaytree->root();
		// find smallest (leftmost) or largest (rightmost) entry.
		while (_current != _ddgSplaytree->null()) {
			asserts(_depth < stackMax, "ddgSplay Iterator ran out of stack space");
#ifdef _DEBUG
			if (_depth > _maxDepth) _maxDepth = _depth;
#endif
			_stack[_depth] = _current;
			_depth++;
			_current = reverse ? _ddgSplaytree->right(_current) : _ddgSplaytree->left(_current);
		}
		if (_depth)
		{
			_depth--;
			_current = _stack[_depth];
		}
	}
	/// The constuctor initializes the iterator a the start or end of tree.
	ddgSplayIterator( ddgSplayTree *st, bool reverse = false)
	{
		_ddgSplaytree = st;
		reset(reverse);
	}

	/// Get the next item.
	inline void next(void)
	{

		if (_current != _ddgSplaytree->null())
		{
			_current = _ddgSplaytree->right(_current);
			while (true)
			{  // we'll use break to exit the loop
				// If there is a left child, go to it.
				if (_current != _ddgSplaytree->null())
				{ 
					asserts(_depth < stackMax, "ddgSplay Iterator ran out of stack space");
#ifdef _DEBUG
			        if (_depth > _maxDepth) _maxDepth = _depth;
#endif
					_stack[_depth] = _current;
					_depth++;
					_current = _ddgSplaytree->left(_current);
				} 
				else
				{  // backtrack 
					if (_depth)
					{
						_depth--;
						_current = _stack[_depth];
						break;
					} 
					else  // stack is empty; done 
					{ 
						_current = _ddgSplaytree->null();
						break;
					}
				} 
			}
		}
	}
	/// Get the prev item.
	inline void prev(void)
	{

		if (_current != _ddgSplaytree->null())
		{
			_current = _ddgSplaytree->left(_current);
			while (true)
			{  // we'll use break to exit the loop
				// If there is a right child, go to it.
				if (_current != _ddgSplaytree->null())
				{ 
					asserts(_depth < stackMax, "ddgSplay Iterator ran out of stack space");
#ifdef _DEBUG
			        if (_depth > _maxDepth) _maxDepth = _depth;
#endif
					_stack[_depth] = _current;
					_depth++;
					_current = _ddgSplaytree->right(_current);
				} 
				else
				{  // backtrack 
					if (_depth)
					{
						_depth--;
						_current = _stack[_depth];
						break;
					} 
					else  // stack is empty; done 
					{ 
						_current = _ddgSplaytree->null();
						break;
					}
				} 
			}
		}
		
	}

	/// Return the ddgSplay node that the iterator is at.
	inline ddgSplayIndex current(void) { return _current; }
	/// Return true if the iterator reached the end of the list.
    inline bool end(void) { return (_current == _ddgSplaytree->null()) ? true : false; }
};

#endif
