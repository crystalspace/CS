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
#include "ddgsplay.h"
#else
#include "sysdef.h"
#include "csterr/ddgsplay.h"
#endif
ddgSplayKey ddgSplayTree::_maxNode;
ddgSplayKey ddgSplayTree::_minNode;
ddgSplayIndex ddgSplayTree::_nList = 0;
ddgSplayIndex ddgSplayTree::_fList = 0;
unsigned int ddgSplayTree::_refCount = 0;
unsigned int ddgSplayTree::_maxSize = 12000;
unsigned int ddgSplayTree::_totalSize = 0;

ddgSplayTree::~ddgSplayTree(void)
{
	_root = clear(_root);
    _refCount--;
    if (_refCount == 0)
    {
        delete _nList;
        _nList = 0;
        _fList = 0;
    }
}

ddgSplayTree::ddgSplayTree(void)
{
    _refCount++;
	_nullNode = 0;
    _header = 0;
	_size = 0;
    if (_nList == 0)
    {
        // Pre allocate a block of nodes.
        _nList = new ddgSplayNode[_maxSize];
		ddgAsserts(_nList,"Failed to Allocate memory");
		ddgMemorySet(ddgSplayNode,_maxSize);
	    _fList = _nList; // 0
	    _minNode.set(ddgSplayKey(0,0,0xFFFF));
	    _maxNode.set(ddgSplayKey(0,0,0));
    }

	if (!_nList)
	{
		ddgErrorSet(Memory,"Failed to allocate ddgSplayTree buffer");
#ifdef DDG
		ddgError::report();
#endif
	}
	else
	{
		// Initialize the free list.
		unsigned int i = _maxSize-1;
		_nList[i]._left = 0;
		while (i)
		{
			_nList[i-1]._left = &(_nList[i]); // i
			i--;
		}
		// Create a null node and header node.
		_nullNode = allocNode();
        _header = allocNode();
		left(_nullNode, _nullNode);
        right(_nullNode, _nullNode);
        // Don't count these items.
        _size = 0;
		_totalSize = 0;
	}
    _root = _nullNode;
}

// Allocator for ddgSplayNodes.
ddgSplayIndex ddgSplayTree::allocNode(void)
{
    ddgAssert(_nList);
    ddgAssert(_totalSize < _maxSize-2);
	ddgSplayIndex n = _fList;
	ddgAsserts(n,"Ran out of free nodes");
	_fList = left(n);
	_size++;
	_totalSize++;
	return n;
}

// Deallocator for ddgSplayNodes.
void ddgSplayTree::freeNode(ddgSplayIndex n)
{
    ddgAssert(_size && _totalSize);
	left(n, _fList);
	_fList = n;
	_size--;
	_totalSize--;
}

ddgSplayIndex ddgSplayTree::clear( ddgSplayIndex n )
{
    if( n != _nullNode )
    {
        clear( left(n) );
        clear( right(n) );
        freeNode(n);
    }
    return _nullNode;
}

/// Find the minimum value in the tree.
ddgSplayIndex ddgSplayTree::findMin( ddgSplayIndex n )
{ return ddgSplay( _minNode, n ); }
/// Find the maximum value in the tree.
ddgSplayIndex ddgSplayTree::findMax( ddgSplayIndex n )
{ return ddgSplay( _maxNode, n ); }

// This function can be called only if K2 has a left child 
// Perform a rotate between a node (K2) and its left child 
// Update heights, then return new root 

ddgSplayIndex ddgSplayTree::SingleRotateWithLeft( ddgSplayIndex k2 )
{
    ddgSplayIndex k1;

    k1 = left(k2);
    left(k2, right(k1));
    right(k1, k2);

    return k1;  // New root 
}

// This function can be called only if K1 has a right child 
// Perform a rotate between a node (K1) and its right child 
// Update heights, then return new root 

ddgSplayIndex ddgSplayTree::SingleRotateWithRight( ddgSplayIndex k1 )
{
    ddgSplayIndex k2;

    k2 = right(k1);
    right(k1, left(k2));
    left(k2, k1);

    return k2;  // New root
}

// Top-down ddgSplay procedure,
// not requiring Item to be in tree

ddgSplayIndex ddgSplayTree::ddgSplay( ddgSplayKey sk, ddgSplayIndex n )
{
    ddgSplayIndex leftTreeMax, rightTreeMin;

    left(_header, _nullNode); 
    right(_header, _nullNode);
    leftTreeMax = rightTreeMin = _header;
	
    key(_nullNode, sk);

    while( sk.compare( key(n)) != 0 )
    {
        if( sk.compare( key(n)) < 0)
        {
            if( sk.compare( key(left(n))) < 0)
                n = SingleRotateWithLeft( n );
            if( left(n) == _nullNode )
                break;
            // Link right
            left(rightTreeMin, n);
            rightTreeMin = n;
            n = left(n);
        }
        else
        {
            if( sk.compare( key(right(n))) > 0 )
                n = SingleRotateWithRight( n );
            if( right(n) == _nullNode )
                break;
            // Link left 
            right(leftTreeMax, n);
            leftTreeMax = n;
            n = right(n);
        }
    }  // while item != n->_key 

    // Reassemble 
    right(leftTreeMax, left(n));
    left(rightTreeMin, right(n));
    left(n, right(_header));
    right(n, left(_header));

    return n;
}

ddgSplayIndex ddgSplayTree::insert( ddgSplayKey sk, ddgSplayIndex n )
{
    static ddgSplayIndex newNode = 0;

    if( newNode == 0 )
    {
        newNode = allocNode();
    }
    key(newNode, sk );

    if( n == _nullNode )
    {
        left(newNode, _nullNode);
        right(newNode, _nullNode);
        n = newNode;
    }
    else
    {
        n = ddgSplay( sk, n );
        if( sk.compare( key(n)) < 0)
        {
            left(newNode, left(n));
            right(newNode, n);
            left(n, _nullNode);
            n = newNode;
        }
        else
        if( sk.compare( key(n)) > 0 )
        {
            right(newNode, right(n));
            left(newNode, n);
            right(n, _nullNode);
            n = newNode;
        }
        else
            return n;  // Already in the tree 
    }

    newNode = 0;   // So next insert will allocate a new node.
    return n;
}

ddgSplayIndex ddgSplayTree::remove( ddgSplayKey sk, ddgSplayIndex n )
{
    ddgSplayIndex newTree;
    if( n != _nullNode )
    {
        n = ddgSplay( sk, n );
        ddgAsserts( sk.compare( key(n)) == 0, "could not find node to remove from SplayTree." );
        {
            // Found it!
            if( left(n) == _nullNode )
                newTree = right(n);
            else
            {
                newTree = left(n);
                newTree = ddgSplay( sk, newTree );
                right(newTree, right(n));
            }
            freeNode( n );
            n = newTree;
        }
    }

    return n;
}

#if (defined(_DEBUG) && DDG)
ostream& operator << ( ostream&s, ddgSplayKey t )
{
	unsigned int tree, i, k;
	i = t.index();
    tree = t.tree();
    k = t.key();
	return s << "(" << k << "," << tree << "," << i << ")";
}

int ddgSplayTree::printTree( ddgSplayIndex n )
{
	static int d = 0, dm = 0;
    if( n != _nullNode )
    {
		d++;
		if (d > dm) dm = d;
        printTree( left(n) );
        cerr << key(n) << " ";
        printTree( right(n) );
		d--;
    }
	return dm;
}
#endif


unsigned int ddgSplayTree::size(void)
{
	return _size;
}
