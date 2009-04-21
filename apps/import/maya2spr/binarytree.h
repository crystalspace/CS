/*
 * binarytree.h by Keith Fulton <keith@paqrat.com>
 *
 * Copyright (C) 2001 PlaneShift Team (info@planeshift.it, 
 * http://www.planeshift.it)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation (version 2 of the License)
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
#ifndef __BINARYTREE_H__
#define __BINARYTREE_H__

template <class T> class BinaryTreeNode;
template <class T> class BinaryTreeIterator;

template <class T> class BinaryTree
{
protected:
    BinaryTreeNode<T> *pRoot;
    long lCount;

public:
    friend class BinaryTreeIterator<T>;

    BinaryTree();
    ~BinaryTree();

    long Count(void) const { return lCount; };
    // Node operations
    BinaryTreeNode<T> *Add(T *pT,int owner = false);
    int Delete(T *pT);
    
    T *Find(T *pT) const;
    T *FindPartial(T *pPartialT,int bGuaranteeUnique=1) const;
    
    const BinaryTreeNode<T> *GetNodeRef(T *pT) const;
    
    void Clear(void);
};

template <class T> class BinaryTreeNode
{
protected:
    T *pObj;
    int iRefCount;
    bool bOwner;
    BinaryTreeNode<T> *pLeft;
    BinaryTreeNode<T> *pRight;

public:
    friend class BinaryTree<T>;
    friend class BinaryTreeIterator<T>;
    
    BinaryTreeNode(T *pT, bool  bOwner);
    ~BinaryTreeNode();

    const BinaryTreeNode<T> *Find(T *pT) const;
    const BinaryTreeNode<T> *FindPartial(T *pPartialT,int bGuaranteeUnique=1) const;

    int  AddOffspring(T *pNew,int iOwner);
    void AddSubTree(BinaryTreeNode<T> *pTree);
    int  Delete(T *pKey);
};

template <class T> class BinaryTreeIterator
{
public:
	typedef enum
	{
	    ITER_INORDER = 2,
	    ITER_PREORDER = 1
	} iterorder;
	long lIndex;

	BinaryTreeIterator(const BinaryTree<T> *pTree,
		iterorder iOrder=ITER_INORDER);
	~BinaryTreeIterator();

	T *First ();
	T *Last ();

	T *Next ();
	T *Prev ();
	T *Curr ();

	T *operator++(void) { return Next(); };
	T *operator--(void) { return Prev(); };

protected:
	const BinaryTree<T> *pMyTree;
	BinaryTreeNode<T> **pNodeList;
	int iDepth;
	iterorder iIterOrder;
};

#include "binarytree.cpp"

#endif

