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

#include "crystalspace.h"

#include "kdtree.h"
#include "radprimitive.h"

#include "csutil/alignedalloc.h"

using namespace CS;

namespace lighter
{

  // KDTree functors
  struct CollectPrimitives
  {
    CollectPrimitives (KDTree *tree)
      : tree (tree)
    {
      tree->boundingBox.StartBoundingBox (csVector3 (0,0,0));
    }

    void operator() (RadObject* obj)
    {
      RadObjectVertexData &vdata = obj->GetVertexData ();
      csArray<RadPrimitiveArray>& primArrays = obj->GetPrimitives ();
      for (size_t p = 0; p < primArrays.GetSize(); p++)
      {
        RadPrimitiveArray &primArray = primArrays[p];
        for (unsigned int i = 0; i < primArray.GetSize (); i++)
        {
          //Push triangles
          RadPrimitive& prim = primArray[i];
          
          //root->radPrimitives.Push (&primArray[i]);
          const size_t* ia = prim.GetIndexArray ();
          //compute bb at same time
          unsigned int j = 0;
          tree->boundingBox.AddBoundingVertexSmart (vdata.vertexArray[ia[0]].position);
          KDTreePrimitive p;
          p.primPointer = &prim;
          p.vertices[0] = vdata.vertexArray[ia[0]].position;
          p.vertices[1] = vdata.vertexArray[ia[1]].position;
          p.vertices[2] = vdata.vertexArray[ia[2]].position;
          p.normal = prim.GetPlane ().norm;

          tree->allTriangles.Push (p);
          for (uint j = 1; j < 3; j++)
          {
            tree->boundingBox.AddBoundingVertexSmart (vdata.vertexArray[ia[j]].position);
          }
        }
      }
    }

    KDTree *tree;
  };

  struct CountLeftRight
  {
    CountLeftRight(KDTree* tree, size_t axis)
      : tree (tree), axis (axis)
    {
    }

    inline void Reset ()
    {
      countLeft = 0;
      countRight = 0;
    }

    void operator() (size_t index, float position)
    {
      const KDTreePrimitive& prim = tree->allTriangles[index];
    
      if((prim.vertices[0][axis] <= position) ||
         (prim.vertices[1][axis] <= position) ||
         (prim.vertices[2][axis] <= position))
         countLeft++;

      if((prim.vertices[0][axis] >= position) ||
         (prim.vertices[1][axis] >= position) ||
         (prim.vertices[2][axis] >= position))
         countRight++;
    }


    KDTree* tree;
    size_t axis;
    size_t countLeft;
    size_t countRight;
  };


  struct DistributeLeftRight
  {
    DistributeLeftRight(KDTree* tree, KDTreeNode* left, KDTreeNode* right,
      size_t axis, float position)
      : tree (tree), left (left), right (right), axis (axis), position (position)
    {
    }

    void operator() (size_t index)
    {
      const KDTreePrimitive& prim = tree->allTriangles[index];
    
      if((prim.vertices[0][axis] <= position) ||
         (prim.vertices[1][axis] <= position) ||
         (prim.vertices[2][axis] <= position))
         left->triangleIndices.Push (index);

      if((prim.vertices[0][axis] >= position) ||
         (prim.vertices[1][axis] >= position) ||
         (prim.vertices[2][axis] >= position))
         right->triangleIndices.Push (index);
    }


    KDTree* tree;
    KDTreeNode* left;
    KDTreeNode* right;
    size_t axis;
    float position;
  };

  KDTree* KDTreeBuilder::BuildTree(const csHash<csRef<RadObject>,csString>::GlobalIterator &objectIt)
  {
    KDTree *newTree = new KDTree;

    //Collect the geometry
    CollectPrimitives primitiveCollector (newTree);
    ForEach (objectIt, primitiveCollector);

    //Allocate first node
    newTree->rootNode = new KDTreeNode;

    //Add all triangles to it
    for(size_t i = 0; i < newTree->allTriangles.GetSize (); i++)
      newTree->rootNode->triangleIndices.Push (i);

    // Start tree node subdivision
    Subdivide (newTree, newTree->rootNode, newTree->boundingBox);

    return newTree;
  }

  void KDTreeBuilder::Subdivide (KDTree *tree, KDTreeNode *node, 
    const csBox3& currentAABB, unsigned int depth,
    unsigned int lastAxis, float lastPosition)
  {
    if(depth++ > 16) 
      return;

    // Work out split direction, might have different logic later?
    csVector3 bbsize = currentAABB.GetSize ();
    if ((bbsize.x >= bbsize.y) && (bbsize.x >= bbsize.z)) 
      node->splitDimension = CS_AXIS_X;
    else if ((bbsize.y >= bbsize.x) && (bbsize.y >= bbsize.z)) 
      node->splitDimension = CS_AXIS_Y;
    else
      node->splitDimension = CS_AXIS_Z;

    if(lastAxis != node->splitDimension)
      lastPosition = -FLT_MAX;
    
    //Collect candidates for split positions
    csDirtyAccessArray<float> possiblePositions;
    possiblePositions.SetCapacity (node->triangleIndices.GetSize ());

    for (size_t i = 0; i < node->triangleIndices.GetSize (); i++)
    {
      size_t idx = node->triangleIndices[i];
      const KDTreePrimitive &prim = tree->allTriangles[idx];

      possiblePositions.Push (prim.vertices[0][node->splitDimension]);
      possiblePositions.Push (prim.vertices[1][node->splitDimension]);
      possiblePositions.Push (prim.vertices[2][node->splitDimension]);
    }

    // Sort it to help pruning duplicate ones
    csRadixSorter RS;
    RS.Sort (possiblePositions.GetArray (), possiblePositions.GetSize ());

    size_t* ranks = RS.GetRanks ();
    // Iterate over positions
    float oldPosition = possiblePositions[ranks[0]];
    float bestPosition = 0;
    float bestCost = FLT_MAX;

    float SA_inv = 1.0/currentAABB.Area ();

    csBox3 leftBox (currentAABB);
    csBox3 rightBox (currentAABB);

    CountLeftRight counter (tree, node->splitDimension); 
    
    for (size_t i = 0; i <= possiblePositions.GetSize (); i++)  //extra-iteration on purpose
    {
      float newPosition;

      if(i != possiblePositions.GetSize ())
      {
        newPosition = possiblePositions[ranks[i]];
      }
      else
      {
        newPosition = FLT_MAX;
      }

      if(oldPosition != newPosition)
      {
        float value = oldPosition;
        oldPosition = newPosition;

        if(value == lastPosition) continue;
        //Changed position, so it is a non-duplicate, try it

        leftBox.SetMax (node->splitDimension, value);
        rightBox.SetMin (node->splitDimension, value);

        // Compute number of elements right vs left
        counter.Reset ();

        ForEach (node->triangleIndices.GetIterator (), counter, value);

        size_t countL = counter.countLeft;
        size_t countR = counter.countRight;

        float currentCost = 0.3f + 1.0f* SA_inv * (leftBox.Area ()*countL + rightBox.Area ()*countR);

        if(currentCost < bestCost)
        {
          bestCost = currentCost;
          bestPosition = value;
        }
      }
    }
  
    // check if any split is good enough
    if (bestCost > node->triangleIndices.GetSize ()*1.0f)
    {
      return;
    }

    // Split it
    //node->splitLocation = bbsize[node->splitDimension]/2 + currentAABB.Min ()[node->splitDimension];
    node->splitLocation = bestPosition;
    node->leftChild = new KDTreeNode;
    node->rightChild = new KDTreeNode;

    // Distribute nodes
    DistributeLeftRight distributor (tree, node->leftChild, node->rightChild, 
      node->splitDimension, node->splitLocation);
    ForEach (node->triangleIndices.GetIterator (), distributor);

    // Subdivide 
    leftBox.SetMax (node->splitDimension, node->splitLocation);
    rightBox.SetMin (node->splitDimension, node->splitLocation);

    // And remove from ourselves
    node->triangleIndices.DeleteAll ();

    if (node->leftChild->triangleIndices.GetSize () > 1)
      Subdivide (tree, node->leftChild, leftBox, depth, node->splitDimension,
      node->splitLocation);

    if (node->rightChild->triangleIndices.GetSize () > 1)
      Subdivide (tree, node->rightChild, rightBox, depth, node->splitDimension,
      node->splitLocation);
  }

  struct CountNodesAndTriangles
  {
    CountNodesAndTriangles ()
      : numNodes (0), numTriIndices (0)
    {}

    void CountTree (KDTreeNode* node)
    {
      if (!node)
        return;

      numNodes++;

      numTriIndices += node->triangleIndices.GetSize ();

      CountTree (node->leftChild);
      CountTree (node->rightChild);
    }

    size_t numNodes, numTriIndices;
  };

  struct OptimizedNodeBuilderConf
  {
    KDTreeNode_Opt* freeNodeList;
    size_t usedNodes;

    KDTreePrimitive_Opt* freePrimitivesList;
    size_t usedPrimitives;

    KDTree* tree;
  };

  void SetupOptimizedNode (KDTreeNode* origNode, KDTreeNode_Opt* node,
    OptimizedNodeBuilderConf& c)
  {
    if (origNode->leftChild)
    {
      // inner node
      node->inner.flagDimensionAndOffset = 0x04;
      
      node->inner.flagDimensionAndOffset =
        (node->inner.flagDimensionAndOffset & ~0x03) | 
        (origNode->splitDimension & 0x03);

      node->inner.splitLocation = origNode->splitLocation;

      // Get nodes for children
      KDTreeNode_Opt* left = c.freeNodeList + c.usedNodes++;
      KDTreeNode_Opt* right = c.freeNodeList + c.usedNodes++;

      node->inner.flagDimensionAndOffset = 
        (node->inner.flagDimensionAndOffset & 0x07) | (((uintptr_t)left) & ~0x07);

      // Setup children
      SetupOptimizedNode (origNode->leftChild, left, c);
      SetupOptimizedNode (origNode->rightChild, right, c);
    }
    else
    {
      // leaf node
      node->leaf.flagAndOffset = 0;

      node->leaf.numberOfPrimitives = origNode->triangleIndices.GetSize ();
      
      KDTreePrimitive_Opt* prims = c.freePrimitivesList + c.usedPrimitives;
      c.usedPrimitives += node->leaf.numberOfPrimitives;
      
      node->leaf.flagAndOffset = 
        (node->leaf.flagAndOffset & 0x07) | (((uintptr_t)prims) & ~0x07);


      // Setup all primitives
      for (size_t i = 0; i < node->leaf.numberOfPrimitives; ++i)
      {
        size_t triIndex = origNode->triangleIndices[i];
        KDTreePrimitive &prim = c.tree->allTriangles[triIndex];

        KDTreePrimitive_Opt &optPrim = prims[i];
        
        // Setup optimized
        optPrim.primPointer = prim.primPointer;
        // Find max normal direction
        size_t k = prim.normal.DominantAxis ();
        
        optPrim.normal_K = k;

        size_t u = (k+1)%3;
        size_t v = (k+2)%3;

        const csVector3& A = prim.vertices[0];

        // precalc normal
        float nkinv = 1.0f/prim.normal[k];
        optPrim.normal_U = prim.normal[u] * nkinv;
        optPrim.normal_V = prim.normal[v] * nkinv;
        optPrim.normal_D = (prim.normal * A) * nkinv;


        csVector3 bb = prim.vertices[2] - prim.vertices[0];
        csVector3 cc = prim.vertices[1] - prim.vertices[0];

        float tmp = 1.0f/(bb[u] * cc[v] - bb[v] * cc[u]);

        // edge 1
        optPrim.edgeA_U = -bb[v] * tmp;
        optPrim.edgeA_V = bb[u] * tmp;
        optPrim.edgeA_D = (bb[v] * A[u] - bb[u] * A[v]) * tmp;

        // edge 2
        optPrim.edgeB_U = cc[v] * tmp;
        optPrim.edgeB_V = -cc[u] * tmp;
        optPrim.edgeB_D = (cc[u] * A[v] - cc[v] * A[u]) * tmp;

      }
    }
  }
  
  KDTree_Opt* KDTreeBuilder::OptimizeTree (KDTree* tree)
  {
    // null tree -> null result
    if (!tree)
      return 0;

    // Simple part
    KDTree_Opt* optTree = new KDTree_Opt;
    optTree->boundingBox = tree->boundingBox;

    // Count number of nodes and triangle "indices"
    CountNodesAndTriangles counter;
    counter.CountTree (tree->rootNode);

    if (counter.numNodes == 0)
    {
      optTree->primitives = 0;
      optTree->nodeList = 0;
      return optTree;
    }

    // Setup arrays
    optTree->nodeList = static_cast<KDTreeNode_Opt*> (
      CS::Memory::AlignedMalloc (sizeof (KDTreeNode_Opt) * counter.numNodes+1, 16));
    optTree->primitives = static_cast<KDTreePrimitive_Opt*> (
      CS::Memory::AlignedMalloc (sizeof (KDTreePrimitive_Opt) * counter.numTriIndices, 16));

    OptimizedNodeBuilderConf c = {};
    c.freeNodeList = optTree->nodeList;
    c.freePrimitivesList = optTree->primitives;
    c.tree = tree;

    // Get root node    
    KDTreeNode_Opt* root = c.freeNodeList + c.usedNodes++;
    c.usedNodes++;
    SetupOptimizedNode (tree->rootNode, root, c);

    return optTree;
  }



  // -- Helper 
  bool KDTreeHelper::CollectPrimitives(const KDTree_Opt *tree, 
    RadPrimitivePtrArray &primArray, const csBox3 &overlapAABB)
  {
    // Walk and get all primitives within overlapAABB
    if (!tree->boundingBox.TestIntersect (overlapAABB))
      return false; //No intersection at all

    RadPrimitivePtrSet collectedPrimitives;
   
    CollectPrimitives (tree, tree->nodeList, tree->boundingBox, collectedPrimitives,
      overlapAABB);

    // Copy result to array
    RadPrimitivePtrSet::GlobalIterator si (collectedPrimitives.GetIterator ());
    while (si.HasNext ())
    {
      primArray.Push (si.Next ());
    }

    return collectedPrimitives.GetSize () > 0;
  }

  void KDTreeHelper::CollectPrimitives(const KDTree_Opt* tree, 
    const KDTreeNode_Opt *node, csBox3 currentBox, RadPrimitivePtrSet& outPrims, 
    const csBox3 &overlapAABB)
  {
    if (node->inner.flagDimensionAndOffset & 0x04)
    {
      // Internal node
      csBox3 childBox = currentBox;

      // Left
      childBox.SetMax (node->inner.flagDimensionAndOffset & 0x03, 
        node->inner.splitLocation);
      if (childBox.TestIntersect (overlapAABB))
      {
        CollectPrimitives (tree, 
          reinterpret_cast<KDTreeNode_Opt*> (node->inner.flagDimensionAndOffset & ~0x07), 
          childBox, outPrims, overlapAABB);
      }

      // Right
      childBox = currentBox;
      childBox.SetMin (node->inner.flagDimensionAndOffset & 0x03, 
        node->inner.splitLocation);
      if (childBox.TestIntersect (overlapAABB))
      {
        CollectPrimitives (tree, 
          reinterpret_cast<KDTreeNode_Opt*> (node->inner.flagDimensionAndOffset & ~0x07) + 1,
          childBox, outPrims, overlapAABB);
      }

    }
    else
    {
      // Leaf node
      /*const csArray<size_t>& triIndices = node->triangleIndices;
      const csArray<KDTreePrimitive>& allTriangles = tree->allTriangles;
*/
      KDTreePrimitive_Opt* primList = 
        reinterpret_cast<KDTreePrimitive_Opt*> (node->leaf.flagAndOffset & ~0x07);

      for (unsigned int i = 0; i < node->leaf.numberOfPrimitives; i++)
      {
        outPrims.Add (primList[i].primPointer);
      }
    }
  }

  KDTree_Opt::~KDTree_Opt ()
  {
    CS::Memory::AlignedFree (this->primitives);
    CS::Memory::AlignedFree (this->nodeList);
  }
}
