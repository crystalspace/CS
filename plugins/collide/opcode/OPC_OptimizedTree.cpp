///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for optimized trees.
 *	\file		OPC_OptimizedTree.cpp
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A standard AABB tree.
 *
 *	\class		AABBCollisionTree
 *	\author		Pierre Terdiman
 *	\version	1.2
 *	\date		March, 20, 2001
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A no-leaf AABB tree.
 *
 *	\class		AABBNoLeafTree
 *	\author		Pierre Terdiman
 *	\version	1.2
 *	\date		March, 20, 2001
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A quantized AABB tree.
 *
 *	\class		AABBQuantizedTree
 *	\author		Pierre Terdiman
 *	\version	1.2
 *	\date		March, 20, 2001
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A quantized no-leaf AABB tree.
 *
 *	\class		AABBQuantizedNoLeafTree
 *	\author		Pierre Terdiman
 *	\version	1.2
 *	\date		March, 20, 2001
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Precompiled Header
#include "Stdafx.h"

using namespace Opcode;

//! Compilation flag:
//! - true to fix quantized boxes (i.e. make sure they enclose the original ones)
//! - false to see the effects of quantization errors (faster, but wrong results in some cases)
static bool gFixQuantized = true;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Builds an implicit tree from a standard one. An implicit tree is a complete tree (2*N-1 nodes) whose negative
 *	box pointers and primitive pointers have been made implicit, hence packing 3 pointers in one.
 *
 *	Layout for implicit trees:
 *	Node:
 *			- box
 *			- data (32-bits value)
 *
 *	if data's LSB = 1 =>	remaining bits are a primitive pointer
 *	else					remaining bits are a P-node pointer, and N = P + 1
 *
 *	\relates	AABBCollisionNode
 *	\fn			_BuildCollisionTree(AABBCollisionNode* linear, const udword boxid, udword& curid, const AABBTreeNode* curnode)
 *	\param		linear		[in] base address of destination nodes
 *	\param		boxid		[in] index of destination node
 *	\param		curid		[in] current running index
 *	\param		curnode		[in] current node from input tree
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void _BuildCollisionTree(AABBCollisionNode* linear, const udword boxid, udword& curid, const AABBTreeNode* curnode)
{
	// Current node from input tree is "curnode". Must be flattened into "linear[boxid]".

	// Store the AABB
	curnode->GetAABB()->GetCenter(linear[boxid].mAABB.mCenter);
	curnode->GetAABB()->GetExtents(linear[boxid].mAABB.mExtents);
	// Store remaining info
	if(curnode->IsLeaf())
	{
		// The input tree must be complete => i.e. one primitive/leaf
		ASSERT(curnode->GetNbPrimitives()==1);
		// Get the primitive index from the input tree
		udword PrimitiveIndex = curnode->GetPrimitives()[0];
		// Setup box data as the primitive index, marked as leaf
		linear[boxid].mData = (PrimitiveIndex<<1)|1;
	}
	else
	{
		// To make the negative one implicit, we must store P and N in successive order
		udword PosID = curid++;	// Get a new id for positive child
		udword NegID = curid++;	// Get a new id for negative child
		// Setup box data as the forthcoming new P pointer
		linear[boxid].mData = (udword)&linear[PosID];
		// Make sure it's not marked as leaf
		ASSERT(!(linear[boxid].mData&1));
		// Recurse with new IDs
		_BuildCollisionTree(linear, PosID, curid, curnode->GetPos());
		_BuildCollisionTree(linear, NegID, curid, curnode->GetNeg());
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Builds a "no-leaf" tree from a standard one. This is a tree whose leaf nodes have been removed.
 *
 *	Layout for no-leaf trees:
 *
 *	Node:
 *			- box
 *			- P pointer => a node (LSB=0) or a primitive (LSB=1)
 *			- N pointer => a node (LSB=0) or a primitive (LSB=1)
 *
 *	\relates	AABBNoLeafNode
 *	\fn			_BuildNoLeafTree(AABBNoLeafNode* linear, const udword boxid, udword& curid, const AABBTreeNode* curnode)
 *	\param		linear		[in] base address of destination nodes
 *	\param		boxid		[in] index of destination node
 *	\param		curid		[in] current running index
 *	\param		curnode		[in] current node from input tree
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void _BuildNoLeafTree(AABBNoLeafNode* linear, const udword boxid, udword& curid, const AABBTreeNode* curnode)
{
	const AABBTreeNode* P = curnode->GetPos();
	const AABBTreeNode* N = curnode->GetNeg();
	// Leaf nodes here?!
	ASSERT(P);
	ASSERT(N);
	// Internal node => keep the box
	curnode->GetAABB()->GetCenter(linear[boxid].mAABB.mCenter);
	curnode->GetAABB()->GetExtents(linear[boxid].mAABB.mExtents);

	if(P->IsLeaf())
	{
		// The input tree must be complete => i.e. one primitive/leaf
		ASSERT(P->GetNbPrimitives()==1);
		// Get the primitive index from the input tree
		udword PrimitiveIndex = P->GetPrimitives()[0];
		// Setup prev box data as the primitive index, marked as leaf
		linear[boxid].mData = (PrimitiveIndex<<1)|1;
	}
	else
	{
		// Get a new id for positive child
		udword PosID = curid++;
		// Setup box data
		linear[boxid].mData = (udword)&linear[PosID];
		// Make sure it's not marked as leaf
		ASSERT(!(linear[boxid].mData&1));
		// Recurse
		_BuildNoLeafTree(linear, PosID, curid, P);
	}

	if(N->IsLeaf())
	{
		// The input tree must be complete => i.e. one primitive/leaf
		ASSERT(N->GetNbPrimitives()==1);
		// Get the primitive index from the input tree
		udword PrimitiveIndex = N->GetPrimitives()[0];
		// Setup prev box data as the primitive index, marked as leaf
		linear[boxid].mData2 = (PrimitiveIndex<<1)|1;
	}
	else
	{
		// Get a new id for positive child
		udword NegID = curid++;
		// Setup box data
		linear[boxid].mData2 = (udword)&linear[NegID];
		// Make sure it's not marked as leaf
		ASSERT(!(linear[boxid].mData2&1));
		// Recurse
		_BuildNoLeafTree(linear, NegID, curid, N);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBCollisionTree::AABBCollisionTree() : mNodes(null)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBCollisionTree::~AABBCollisionTree()
{
	DELETEARRAY(mNodes);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Builds the collision tree from a generic AABB tree.
 *	\param		tree			[in] generic AABB tree
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBCollisionTree::Build(AABBTree* tree)
{
	// Checkings
	if(!tree)	return false;
	// Check the input tree is complete
	udword NbTriangles	= tree->GetNbPrimitives();
	udword NbNodes		= tree->GetNbNodes();
	if(NbNodes!=NbTriangles*2-1)	return false;

	// Get nodes
	mNbNodes = NbNodes;
	mNodes = new AABBCollisionNode[mNbNodes];
	CHECKALLOC(mNodes);

	// Build the tree
	udword CurID = 1;
	_BuildCollisionTree(mNodes, 0, CurID, tree);
	ASSERT(CurID==mNbNodes);

#ifdef __ICECORE_H__
	Log("Original tree: %d nodes, depth %d\n", NbNodes, tree->ComputeDepth());
	Log("AABB Collision tree: %d nodes, %d bytes - Alignment: %d\n", mNbNodes, GetUsedBytes(), Alignment(udword(mNodes)));
#endif

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBNoLeafTree::AABBNoLeafTree() : mNodes(null)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBNoLeafTree::~AABBNoLeafTree()
{
	DELETEARRAY(mNodes);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Builds the collision tree from a generic AABB tree.
 *	\param		tree			[in] generic AABB tree
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBNoLeafTree::Build(AABBTree* tree)
{
	// Checkings
	if(!tree)	return false;
	// Check the input tree is complete
	udword NbTriangles	= tree->GetNbPrimitives();
	udword NbNodes		= tree->GetNbNodes();
	if(NbNodes!=NbTriangles*2-1)	return false;

	// Get nodes
	mNbNodes = NbTriangles-1;
	mNodes = new AABBNoLeafNode[mNbNodes];
	CHECKALLOC(mNodes);

	// Build the tree
	udword CurID = 1;
	_BuildNoLeafTree(mNodes, 0, CurID, tree);
	ASSERT(CurID==mNbNodes);

#ifdef __ICECORE_H__
	Log("Original tree: %d nodes, depth %d\n", NbNodes, tree->ComputeDepth());
	Log("AABB quantized tree: %d nodes, %d bytes - Alignment: %d\n", mNbNodes, GetUsedBytes(), Alignment(udword(mNodes)));
#endif

	return true;
}

// Quantization notes:
// - We could use the highest bits of mData to store some more quantized bits. Dequantization code
//   would be slightly more complex, but number of overlap tests would be reduced (and anyhow those
//   bits are currently wasted). Of course it's not possible if we move to 16 bits mData.
// - Something like "16 bits floats" could be tested, to bypass the int-to-float conversion.
// - A dedicated BV-BV test could be used, dequantizing while testing for overlap. (i.e. it's some
//   lazy-dequantization which may save some work in case of early exits). At the very least some
//   muls could be saved by precomputing several more matrices. But maybe not worth the pain.
// - Do we need to dequantize anyway? Not doing the extents-related muls only implies the box has
//   been scaled, for example.
// - The deeper we move into the hierarchy, the smaller the extents should be. May not need a fixed
//   number of quantization bits. Even better, could probably be best delta-encoded.

// Find max values (could use the first node only with min/max boxes)
#define FIND_MAX_VALUES																			\
	/* Get max values */																		\
	Point CMax(MIN_FLOAT, MIN_FLOAT, MIN_FLOAT);												\
	Point EMax(MIN_FLOAT, MIN_FLOAT, MIN_FLOAT);												\
	udword i;  \
	for(i=0;i<mNbNodes;i++)																\
	{																							\
		if(fabs(Nodes[i].mAABB.mCenter.x)>CMax.x)	CMax.x = (float)fabs(Nodes[i].mAABB.mCenter.x);	\
		if(fabs(Nodes[i].mAABB.mCenter.y)>CMax.y)	CMax.y = (float)fabs(Nodes[i].mAABB.mCenter.y);	\
		if(fabs(Nodes[i].mAABB.mCenter.z)>CMax.z)	CMax.z = (float)fabs(Nodes[i].mAABB.mCenter.z);	\
		if(fabs(Nodes[i].mAABB.mExtents.x)>EMax.x)	EMax.x = (float)fabs(Nodes[i].mAABB.mExtents.x);	\
		if(fabs(Nodes[i].mAABB.mExtents.y)>EMax.y)	EMax.y = (float)fabs(Nodes[i].mAABB.mExtents.y);	\
		if(fabs(Nodes[i].mAABB.mExtents.z)>EMax.z)	EMax.z = (float)fabs(Nodes[i].mAABB.mExtents.z);	\
	}
#define INIT_QUANTIZATION             \
 udword nbc=15; /* Keep one bit for sign */        \
 udword nbe=15; /* Keep one bit for fix */        \
 if(!gFixQuantized) nbe++;            \
                   \
 /* Compute quantization coeffs */          \
 Point CQuantCoeff, EQuantCoeff;           \
 CQuantCoeff.x = CMax.x!=0.0f ? float((1<<nbc)-1)/CMax.x : 0.0f;   \
 CQuantCoeff.y = CMax.y!=0.0f ? float((1<<nbc)-1)/CMax.y : 0.0f;   \
 CQuantCoeff.z = CMax.z!=0.0f ? float((1<<nbc)-1)/CMax.z : 0.0f;   \
 EQuantCoeff.x = EMax.x!=0.0f ? float((1<<nbe)-1)/EMax.x : 0.0f;   \
 EQuantCoeff.y = EMax.y!=0.0f ? float((1<<nbe)-1)/EMax.y : 0.0f;   \
 EQuantCoeff.z = EMax.z!=0.0f ? float((1<<nbe)-1)/EMax.z : 0.0f;   \
 /* Compute and save dequantization coeffs */       \
 mCenterCoeff.x = CQuantCoeff.x!=0.0f ? 1.0f / CQuantCoeff.x : 0.0f;  \
 mCenterCoeff.y = CQuantCoeff.y!=0.0f ? 1.0f / CQuantCoeff.y : 0.0f;  \
 mCenterCoeff.z = CQuantCoeff.z!=0.0f ? 1.0f / CQuantCoeff.z : 0.0f;  \
 mExtentsCoeff.x = EQuantCoeff.x!=0.0f ? 1.0f / EQuantCoeff.x : 0.0f; \
 mExtentsCoeff.y = EQuantCoeff.y!=0.0f ? 1.0f / EQuantCoeff.y : 0.0f; \
 mExtentsCoeff.z = EQuantCoeff.z!=0.0f ? 1.0f / EQuantCoeff.z : 0.0f; \
    
#define PERFORM_QUANTIZATION														\
	/* Quantize */																	\
	mNodes[i].mAABB.mCenter[0] = sword(Nodes[i].mAABB.mCenter.x * CQuantCoeff.x);	\
	mNodes[i].mAABB.mCenter[1] = sword(Nodes[i].mAABB.mCenter.y * CQuantCoeff.y);	\
	mNodes[i].mAABB.mCenter[2] = sword(Nodes[i].mAABB.mCenter.z * CQuantCoeff.z);	\
	mNodes[i].mAABB.mExtents[0] = uword(Nodes[i].mAABB.mExtents.x * EQuantCoeff.x);	\
	mNodes[i].mAABB.mExtents[1] = uword(Nodes[i].mAABB.mExtents.y * EQuantCoeff.y);	\
	mNodes[i].mAABB.mExtents[2] = uword(Nodes[i].mAABB.mExtents.z * EQuantCoeff.z);	\
	/* Fix quantized boxes */														\
	if(gFixQuantized)																\
	{																				\
		/* Make sure the quantized box is still valid */							\
		Point Max = Nodes[i].mAABB.mCenter + Nodes[i].mAABB.mExtents;				\
		Point Min = Nodes[i].mAABB.mCenter - Nodes[i].mAABB.mExtents;				\
		/* For each axis */															\
		udword j;    \
		for(j=0;j<3;j++)														\
		{	/* Dequantize the box center */											\
			float qc = float(mNodes[i].mAABB.mCenter[j]) * mCenterCoeff[j];			\
			bool FixMe=true;														\
			do																		\
			{	/* Dequantize the box extent */										\
				float qe = float(mNodes[i].mAABB.mExtents[j]) * mExtentsCoeff[j];	\
				/* Compare real & dequantized values */								\
				if(qc+qe<Max[j] || qc-qe>Min[j])	mNodes[i].mAABB.mExtents[j]++;	\
				else								FixMe=false;					\
				/* Prevent wrapping */												\
				if(!mNodes[i].mAABB.mExtents[j])									\
				{																	\
					mNodes[i].mAABB.mExtents[j]=0xffff;								\
					FixMe=false;													\
				}																	\
			}while(FixMe);															\
		}																			\
	}

#define REMAP_DATA(member)											\
	/* Fix data */													\
	Data = Nodes[i].member;											\
	if(!(Data&1))													\
	{																\
		/* Compute box number */									\
		udword Nb = (Data - udword(Nodes))/Nodes[i].GetNodeSize();	\
		Data = udword(&mNodes[Nb]);									\
	}																\
	/* ...remapped */												\
	mNodes[i].member = Data;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBQuantizedTree::AABBQuantizedTree() : mNodes(null)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBQuantizedTree::~AABBQuantizedTree()
{
	DELETEARRAY(mNodes);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Builds the collision tree from a generic AABB tree.
 *	\param		tree			[in] generic AABB tree
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBQuantizedTree::Build(AABBTree* tree)
{
	// Checkings
	if(!tree)	return false;
	// Check the input tree is complete
	udword NbTriangles	= tree->GetNbPrimitives();
	udword NbNodes		= tree->GetNbNodes();
	if(NbNodes!=NbTriangles*2-1)	return false;

	// Get nodes
	mNbNodes = NbNodes;
	AABBCollisionNode* Nodes = new AABBCollisionNode[mNbNodes];
	CHECKALLOC(Nodes);

	// Build the tree
	udword CurID = 1;
	_BuildCollisionTree(Nodes, 0, CurID, tree);

	// Quantize
	{
		mNodes = new AABBQuantizedNode[mNbNodes];
		CHECKALLOC(mNodes);

		// Get max values
		FIND_MAX_VALUES

		// Quantization
		INIT_QUANTIZATION

		// Quantize
		udword Data;
		for(i=0;i<mNbNodes;i++)
		{
			PERFORM_QUANTIZATION
			REMAP_DATA(mData)
		}

		DELETEARRAY(Nodes);
	}

#ifdef __ICECORE_H__
	Log("Original tree: %d nodes, depth %d\n", NbNodes, tree->ComputeDepth());
	Log("AABB quantized tree: %d nodes, %d bytes - Alignment: %d\n", mNbNodes, GetUsedBytes(), Alignment(udword(mNodes)));
#endif
	return true;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBQuantizedNoLeafTree::AABBQuantizedNoLeafTree() : mNodes(null)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBQuantizedNoLeafTree::~AABBQuantizedNoLeafTree()
{
	DELETEARRAY(mNodes);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Builds the collision tree from a generic AABB tree.
 *	\param		tree			[in] generic AABB tree
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBQuantizedNoLeafTree::Build(AABBTree* tree)
{
	// Checkings
	if(!tree)	return false;
	// Check the input tree is complete
	udword NbTriangles	= tree->GetNbPrimitives();
	udword NbNodes		= tree->GetNbNodes();
	if(NbNodes!=NbTriangles*2-1)	return false;

	// Get nodes
	mNbNodes = NbTriangles-1;
	AABBNoLeafNode* Nodes = new AABBNoLeafNode[mNbNodes];
	CHECKALLOC(Nodes);

	// Build the tree
	udword CurID = 1;
	_BuildNoLeafTree(Nodes, 0, CurID, tree);
	ASSERT(CurID==mNbNodes);

	// Quantize
	{
		mNodes = new AABBQuantizedNoLeafNode[mNbNodes];
		CHECKALLOC(mNodes);

		// Get max values
		FIND_MAX_VALUES

		// Quantization
		INIT_QUANTIZATION

		// Quantize
		udword Data;
		for(i=0;i<mNbNodes;i++)
		{
			PERFORM_QUANTIZATION
			REMAP_DATA(mData)
			REMAP_DATA(mData2)
		}

		DELETEARRAY(Nodes);
	}

#ifdef __ICECORE_H__
	Log("Original tree: %d nodes, depth %d\n", NbNodes, tree->ComputeDepth());
	Log("AABB quantized no-leaf tree: %d nodes, %d bytes - Alignment: %d\n", mNbNodes, GetUsedBytes(), Alignment(udword(mNodes)));
#endif

	return true;
}
