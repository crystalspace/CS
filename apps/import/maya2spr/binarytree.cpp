/*
 * binarytree.cpp by Keith Fulton <keith@paqrat.com>
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
#ifndef __BINARYTREE_CPP__
#define __BINARYTREE_CPP__

#include <stdio.h>

#ifndef __BINARYTREE_H__
#include "binarytree.h"
#endif


template <class T>
BinaryTree<T>::BinaryTree()
{
    pRoot = 0;
    lCount = 0;
}


template <class T>
BinaryTree<T>::~BinaryTree()
{
    delete pRoot;
}

template <class T>
void BinaryTree<T>::Clear()
{
    delete pRoot;
    pRoot = 0;
    lCount = 0;
}


template <class T>
BinaryTreeNode<T> *BinaryTree<T>::Add(T *pT,int iOwner)
{
    int x=1;

    if (!pRoot)
	pRoot = new BinaryTreeNode<T>(pT,iOwner);
    else
	x = pRoot->AddOffspring(pT,iOwner);
    
    if (x)
    	lCount++;

    return 0;
}

template <class T>
int BinaryTree<T>::Delete(T *pT)
{
    if (!pRoot)
	return 0;
    
    if (*pRoot->pObj == *pT)  // Deleting root of tree
    {
	BinaryTreeNode<T> *pOldRoot = pRoot;

	pRoot = pRoot->pLeft;
	if (pRoot)
	{
	    if (pOldRoot->pRight)
		pRoot->AddSubTree(pOldRoot->pRight);
	}
	else
	{
	    pRoot = pOldRoot->pRight;
	}

        pOldRoot->pLeft = pOldRoot->pRight = 0;
	delete pOldRoot;
	lCount--;
    }
    else
    {
	int x = pRoot->Delete(pT);
	if (x == -1)
	    lCount--;
	return x;
    }

    return -1;
}


template <class T>
T *BinaryTree<T>::Find(T *pT) const
{
    const BinaryTreeNode<T> *pNode = GetNodeRef(pT);

    if (pNode)
	return pNode->pObj;
    else
	return 0;
}

template <class T>
T *BinaryTree<T>::FindPartial(T *pPartialT,int bGuaranteeUnique) const
{
    if (!pRoot)
	return 0;
    else
    {
	BinaryTreeNode<T> *pNode = pRoot->FindPartial(pPartialT,bGuaranteeUnique);

    if (pNode)
	    return pNode->pObj;
	else
	    return 0;
    }
}



template <class T>
const BinaryTreeNode<T> *BinaryTree<T>::GetNodeRef(T *pT) const
{
    if (pRoot)
	return pRoot->Find(pT);
    else
	return 0;
}

template <class T>
BinaryTreeNode<T>::BinaryTreeNode(T *pT,bool bOwnObject)
{
    pObj   = pT;
    iRefCount = 1;
    bOwner = bOwnObject;
    pLeft  = 0;
    pRight = 0;
}

template <class T>
BinaryTreeNode<T>::~BinaryTreeNode()
{
    if (bOwner)
	delete pObj;
    if (pLeft)
	delete pLeft;
    if (pRight)
	delete pRight;
}

template <class T>
int BinaryTreeNode<T>::Delete(T *pKey)
{
    BinaryTreeNode<T> *pNode = this;
    BinaryTreeNode<T> *pParent = 0;
    int iWhichWay=0;

    // Must know parent.  Can't use Find()
    while ((pNode) && (!(*pNode->pObj == *pKey)))
    {
	if (*pKey < *pNode->pObj)
	{
	    if (pNode->pLeft)
	    {
		pParent = pNode;
		pNode = pNode->pLeft;
		iWhichWay = -1;
	    }
	    else
		pNode = 0;
	}
	else
	{
	    if (pNode->pRight)
	    {
		pParent = pNode;
		pNode = pNode->pRight;
		iWhichWay = 1;
	    }
	    else
		pNode = 0;
	}
    }
    if (!pNode)
	return 0; // Node not found.
    
    pNode->iRefCount--;
    if (pNode->iRefCount)
	return pNode->iRefCount; // Node deleted but still present.

    if (pParent) // Clear parent's ptr to this child
    {
	if (iWhichWay == -1)
	    pParent->pLeft = 0;
	else if (iWhichWay == 1)
	    pParent->pRight = 0;

	if (pNode->pLeft) // Add nodes lesser children back into tree
	    AddSubTree(pNode->pLeft);
	if (pNode->pRight) // Add nodes greater children back into tree
	    AddSubTree(pNode->pRight);

	pNode->pRight = 0;
	pNode->pLeft  = 0;
	delete pNode;
	return -1;
    }
    
    // Deleting root node is handled by Tree level function.
    return 0;
}

template <class T>
const BinaryTreeNode<T> *BinaryTreeNode<T>::Find(T *pT) const
{
    if (*this->pObj == *pT)
	return this;
    
    if (*pT < *this->pObj)
    {
	if (pLeft)
	    return pLeft->Find(pT);
	
    	return 0;
    }
    else
    {
	if (pRight)
	    return pRight->Find(pT);

	return 0;
    }
}

template <class T>
const BinaryTreeNode<T> *BinaryTreeNode<T>::FindPartial(T *pT,int bGuaranteeUnique) const
{
    int iEqu=0;

    if (! (iEqu = this->pObj->PartialEquals(*pT)) )
    {
	if (!bGuaranteeUnique)
	    return this;
	
	if (pLeft)
	{
	    if (pLeft->FindPartial(pT,0))
		return 0;
	}
	if (pRight)
	{
	    if (pRight->FindPartial(pT,0))
		return 0;
	}
	return this;
    }
    else
    {
	if (iEqu > 0)
	{
	    if (pLeft)
		return pLeft->FindPartial(pT,bGuaranteeUnique);
	    else
		return 0;
	}
	else
	{
	    if (pRight)
		return pRight->FindPartial(pT,bGuaranteeUnique);
	    else
		return 0;
	}
    }
}

template <class T>
void BinaryTreeNode<T>::AddSubTree(BinaryTreeNode<T> *pTree)
{
    if (!pTree || *pTree->pObj == *pObj)
	return;

    if (*pTree->pObj < *pObj)
    {
	if (pLeft)
	    pLeft->AddSubTree(pTree);
	else
	    pLeft = pTree;
	
	return;
    }
    else
    {
	if (pRight)
	    pRight->AddSubTree(pTree);
	else
	    pRight = pTree;

	return;
    }
}

template <class T>
int BinaryTreeNode<T>::AddOffspring(T *pNew,int iOwner)
{
    if (*pNew == *pObj)
    {
	iRefCount++;
	return 0;   // Didn't really add anything
    }
		
    if (*pNew < *pObj)
    {
	if (pLeft)
	    return pLeft->AddOffspring(pNew,iOwner);
	else
	{
	    pLeft = new BinaryTreeNode<T>(pNew,iOwner);
	    return 1;
	}
    }
    else
    {
	if (pRight)
	    return pRight->AddOffspring(pNew,iOwner);
	else
	{
	    pRight = new BinaryTreeNode<T>(pNew,iOwner);
	    return 1;
	}
    }
}

template <class T>
BinaryTreeIterator<T>::BinaryTreeIterator(const BinaryTree<T> *pTree,
	iterorder iOrder)
{
    pMyTree    = pTree;
    pNodeList  = new BinaryTreeNode<T> *[2000];
    iDepth     = -1;
    lIndex     = -1;
    iIterOrder = iOrder;
}

template <class T>
BinaryTreeIterator<T>::~BinaryTreeIterator()
{
    delete[] pNodeList;
}

template <class T>
T *BinaryTreeIterator<T>::First(void)
{
    if (!pMyTree->pRoot)
    {
	iDepth = -1;
	lIndex = -1;
	return 0;
    }

    switch(iIterOrder)
    {
	case ITER_PREORDER:
	    {
		iDepth=0;
		pNodeList[iDepth]=pMyTree->pRoot;
		lIndex = 0;
		return pNodeList[iDepth]->pObj;
	    }
	case ITER_INORDER:
	    {
		iDepth = 0;
		pNodeList[iDepth] = pMyTree->pRoot;

		while (pNodeList[iDepth]->pLeft)
		{
		    pNodeList[iDepth+1] = pNodeList[iDepth]->pLeft;
		    iDepth++;
		}
		lIndex = 0;
		return pNodeList[iDepth]->pObj;
	    }
    }

    return 0;
}

template <class T>
T *BinaryTreeIterator<T>::Last(void)
{
    if (!pMyTree->pRoot)
    {
	iDepth = -1;
	lIndex = -1;
	return 0;
    }

    // Last node same for PRE,IN order
    iDepth = 0;
    pNodeList[0] = pMyTree->pRoot;

    while (pNodeList[iDepth]->pRight)
    	pNodeList[++iDepth] = pNodeList[iDepth-1]->pRight;
    
    lIndex = pMyTree->lCount;
    return pNodeList[iDepth]->pObj;
}

template <class T>
T *BinaryTreeIterator<T>::Next(void)
{
    if (pMyTree->pRoot == 0)
	return 0;
    
    switch(iIterOrder)
    {
	case ITER_PREORDER:
	    {
		if (pNodeList[iDepth]->pLeft)
		{
		    if (pNodeList[iDepth]->pRight)
		    {
			pNodeList[iDepth+1] = pNodeList[iDepth]->pLeft;
			iDepth++;
		    }
		    else
			pNodeList[iDepth] = pNodeList[iDepth]->pLeft;
		    
    		    lIndex++;
		    return pNodeList[iDepth]->pObj;
		}
		else if (pNodeList[iDepth]->pRight)
		{
		    pNodeList[iDepth] = pNodeList[iDepth]->pRight;
		    lIndex++;
		    return pNodeList[iDepth]->pObj;
		}
		else // no children
		{
		    iDepth--;
		    if (iDepth > -1)
		    {
			if (pNodeList[iDepth]->pRight)
			{
			    pNodeList[iDepth] = pNodeList[iDepth]->pRight;
			    lIndex++;
			    return pNodeList[iDepth]->pObj;
			}
			else
			{
			    lIndex = -1;
			    iDepth = -1;
			    return 0;
			}
		    }
		    else
		    {
			lIndex=-1;
			return 0;
		    }
		}
	    }
	case ITER_INORDER:
	    {
		if (pNodeList[iDepth]->pRight)
		{
		    pNodeList[iDepth+1] = pNodeList[iDepth]->pRight;
		    iDepth++;
		    while (pNodeList[iDepth]->pLeft)
		    {
			pNodeList[iDepth+1] = pNodeList[iDepth]->pLeft;
			iDepth++;
		    }
		    lIndex++;
		    return pNodeList[iDepth]->pObj;
		}
		else
		{
		    iDepth--;
		    while (iDepth>=0 && pNodeList[iDepth]->pRight == pNodeList[iDepth+1])
			iDepth--;
		    lIndex++;
		    if (iDepth>=0)
			return pNodeList[iDepth]->pObj;
		    else
			return 0;
		}
	    }
    }
    return 0;
}

template <class T>
T *BinaryTreeIterator<T>::Prev(void)
{
    csPrintf ("***BinaryTreeIterator::Prev not implemented yet!!!!\n");
    return 0; // not implemented yet
}

template <class T>
T *BinaryTreeIterator<T>::Curr(void)
{
    if (iDepth<0)
	return 0;

    return pNodeList[iDepth]->pObj;
}

#endif
