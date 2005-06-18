/*
  Copyright (C) 2005 by Marten Svanfeldt

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

#include "cssysdef.h"
#include "kdtree.h"
#include "lightmesh.h"

#include "csutil/algorithms.h"

//--- BUILD TREE ---
csKDBuildTree::csKDBuildTree ()
{
}

csKDBuildTree::~csKDBuildTree ()
{
  //@@TODO : Add deallocation here
}

/**
 * Helper functor to iterate over meshes/faces/patches and
 * collect those into a single array
 */
class PatchCollector
{
public:
  void operator () (csLightingMesh *mesh)
  {
    csForEach (mesh->faces.GetIterator (), *this); 
  }

  void operator () (csMeshFace *face)
  {
    csForEach (face->patches.GetIterator (), *this); 
  }

  void operator () (csMeshPatch *patch)
  {
    if (patch->HasChildren ())
    {
      (*this) (patch->child[0]);
      (*this) (patch->child[1]);
      (*this) (patch->child[2]);
      (*this) (patch->child[3]);
    }
    else
      patches.Push (patch);
  }

  PatchCollector (csArray<csMeshPatch*> patches)
    : patches (patches)
  {}

private:
  csArray<csMeshPatch*> &patches;
};

/**
 * Helper functor to compute bounding box 
 */
struct BBComputer
{
  void operator () (csLightingMesh *mesh)
  {
    csForEach ((mesh->faces).GetIterator (), *this); 
  }

  void operator() (csMeshFace *face)
  {
    csArray<csVector3> &vertexList = (face)->mesh->vertexList;
  
    for (uint i = 0; i < (face)->mesh->orignalVertexCount; i++)
    {
      boundingBox.AddBoundingVertex (vertexList[i]);
    }
  }

  csBox3 boundingBox;
};

void csKDBuildTree::BuildTree (const csArray<csLightingMesh*>& meshes)
{
  // Collect all toplevel patches
  csArray<csMeshPatch*> patches;
    
  csForEach (meshes.GetIterator (), PatchCollector (patches));

  //Compute boundaries
  BBComputer bbc;
  csForEach (meshes.GetIterator (), bbc);

  //Setup first node
  rootNode = new csKDBuildTreeNode;

  rootNode->boundingBox = bbc.boundingBox;
  rootNode->patches = patches;
  

  SplitPositionFixer *fixer = new SplitPositionFixer (patches.Length ());
  // Ok, subdivide
  rootNode->Subdivide (fixer);
  delete fixer;
}

struct CollectSplitpositions
{
  CollectSplitpositions (int axis, SplitPositionFixer *sphelper)
    : axis (axis), sphelper (sphelper)
  {
  }

  void operator() (csMeshPatch *patch)
  {
    float a, b;
    patch->GetExtent (axis, a, b);
    sphelper->AddPosition (a);
    sphelper->AddPosition (b);
  }

  int axis;
  SplitPositionFixer *sphelper;
};

void csKDBuildTreeNode::Subdivide (SplitPositionFixer *sphelper)
{
  // Work out subdivision axis
  csVector3 bbsize = boundingBox.GetSize ();
  if ((bbsize.x >= bbsize.y) && (bbsize.x >= bbsize.z)) 
    splitDimension = 0;
  else if ((bbsize.y >= bbsize.x) && (bbsize.y >= bbsize.z)) 
    splitDimension = 1;
  else 
    splitDimension = 2;

  //Find all split-locations (vertices)
  //Try to prune those that are not needed
  sphelper->Reset ();

  CollectSplitpositions collector (splitDimension, sphelper);
  csForEach (patches.GetIterator (), collector);

  csBox3 leftBox = boundingBox;
  csBox3 rightBox = boundingBox;

  //Compute node area inverse
  float SA_inv = 0.5f / (bbsize.x * bbsize.y + bbsize.x * bbsize.z + bbsize.y * bbsize.z);
  float lowestCost = FLT_MAX;
  float bestSplit = 0;
  float minP, maxP;

  SplitPositionFixer::SplitPosition *list = sphelper->firstEntry;
  while (list)
  {
    //Setup the box
    float splitPos = list->position;
    leftBox.SetMax (splitDimension, splitPos);
    rightBox.SetMin (splitDimension, splitPos);

    float leftCount = 0, rightCount = 0;
    
    for (uint i = 0; i < patches.Length (); i++)
    {
      patches[i]->GetExtent (splitDimension, minP, maxP);
      if (minP <= splitPos)
      {
        leftCount++;
      }
      if (maxP >= splitPos)
      {
        rightCount++;
      }
    }

    //compute cost
    csVector3 leftsize = leftBox.GetSize ();
    csVector3 rightsize = rightBox.GetSize ();
    
    float SA_left = 2 * (leftsize.x*leftsize.y + leftsize.x*leftsize.z + leftsize.y*leftsize.z);
    float SA_right = 2 * (rightsize.x*rightsize.y + rightsize.x*rightsize.z + rightsize.y*rightsize.z);
    float splitCost = 0.2f + 1.0f * SA_inv * (SA_left * leftCount + SA_right*rightCount);

    //Save if it is best
    if (splitCost < lowestCost)
    {
      lowestCost = splitCost;
      bestSplit = splitPos;
    }

    list = list->next;
  }

  // Check if split is good
  if (lowestCost > patches.Length () * 1.0f)
  {
    //stop splitting
    return;
  }

  splitLocation = bestSplit;

  // Now divide nodes between left and right
  csKDBuildTreeNode *leftNode = new csKDBuildTreeNode;
  csKDBuildTreeNode *rightNode = new csKDBuildTreeNode;
  leftNode->boundingBox = boundingBox;
  rightNode->boundingBox = boundingBox;

  leftNode->boundingBox.SetMax (splitDimension, bestSplit);
  rightNode->boundingBox.SetMin (splitDimension, bestSplit);

  for (uint i = 0; i < patches.Length (); i++)
  {
    patches[i]->GetExtent (splitDimension, minP, maxP);
    if (minP <= bestSplit)
    {
      leftNode->patches.Push (patches[i]);
    }
    if (maxP >= bestSplit)
    {
      rightNode->patches.Push (patches[i]);
    }
  }

  //@@TOOD, add maxdepth check
  leftNode->Subdivide (sphelper);
  rightNode->Subdivide (sphelper);
}

/// Optimized kd-tree
csKDTree::csKDTree ()
{
}

csKDTree::~csKDTree ()
{
}

void csKDTree::BuildTree (csKDBuildTree *buildTree)
{
  // Tree without a root is no good
  if (!buildTree || !buildTree->rootNode) return;

  // Save toplevel BB
  boundingBox = buildTree->rootNode->boundingBox;

  //allocate a "scrap"-node
  nodeAllocator.Alloc ();

  // Allocate and populate root-node
  rootNode = nodeAllocator.Alloc ();

  // Try to optimize and transfer the tree
  TransferNode (buildTree->rootNode, rootNode);
}

void csKDTree::TransferNode (csKDBuildTreeNode *fromNode, csKDTreeNode *toNode)
{
  if (!fromNode || !toNode) return;

  // See if this is a node with children or not
  if (fromNode->left)
  {
    // inner node (no patches in node)
    csKDTreeNodeH::SetFlag (toNode, true);
    csKDTreeNodeH::SetDimension (toNode, fromNode->splitDimension);
    toNode->splitLocation = fromNode->splitLocation;
    
    // Allocate children
    csKDTreeNode *left = nodeAllocator.Alloc ();
    csKDTreeNode *right = nodeAllocator.Alloc ();

    // Better check?
    if (right != (left+1))
    {
      //different blocks, try again ;)
      left = nodeAllocator.Alloc ();
      right = nodeAllocator.Alloc ();
    }

    csKDTreeNodeH::SetPointer (toNode, (uintptr_t)left);
    TransferNode (fromNode->left, left);
    TransferNode (fromNode->right, right);
  }
  else
  {
    // node with primitives
    csKDTreeNodeH::SetFlag (toNode, false);
    
    toNode->numberOfPrimitives = 0;
    
    fromNode->patches.Sort ();

    uint i = 0, j = 0;
    for (i = 0; i<fromNode->patches.Length (); i++)
    {
      fromNode->patches[i]->ConstructAccelerationStruct ();
      toNode->numberOfPrimitives++;
      if (fromNode->patches[i]->IsQuad ()) 
        toNode->numberOfPrimitives++;
    }

    // Allocate a list of pointers..
    csMeshPatchAccStruct **primlist = (csMeshPatchAccStruct**)csAlignedMalloc 
      (sizeof (csMeshPatchAccStruct*) * toNode->numberOfPrimitives, 8);

    csKDTreeNodeH::SetPointer (toNode, (uintptr_t)primlist);
    // And transfer all prims
    for (i = 0, j = 0; i<fromNode->patches.Length (); i++)
    {
      primlist[j++] = fromNode->patches[i]->accStruct;
      if (fromNode->patches[i]->IsQuad ()) 
        primlist[j++] = (fromNode->patches[i]->accStruct+1);
    }
  }
}
