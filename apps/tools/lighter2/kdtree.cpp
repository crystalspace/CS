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
      RadPrimitiveArray &primArray = obj->GetPrimitives ();
      RadObjectVertexData &vdata = obj->GetVertexData ();
      for (unsigned int i = 0; i < primArray.GetSize (); i++)
      {
        //Push triangles
        RadPrimitive& prim = primArray[i];
        
        //root->radPrimitives.Push (&primArray[i]);
        const SizeTDArray & ia = prim.GetIndexArray ();
        //compute bb at same time
        unsigned int j = 0;
        for (j = 0; j < ia.GetSize ()-2; j++)
        {
          tree->boundingBox.AddBoundingVertexSmart (vdata.vertexArray[ia[j]].position);
          KDTreePrimitive p;
          p.primPointer = &prim;
          p.vertices[0] = vdata.vertexArray[ia[j]].position;
          p.vertices[1] = vdata.vertexArray[ia[j+1]].position;
          p.vertices[2] = vdata.vertexArray[ia[j+2]].position;
          p.normal = prim.GetPlane ().norm;

          // Setup optimized
          int k = 0;
          // Find max normal direction
          if (fabsf (p.normal.x) > fabsf (p.normal.y))
          {
            if (fabsf (p.normal.x) > fabsf (p.normal.z)) k = 0;
            else k = 2;
          }
          else
          {
            if (fabsf (p.normal.y) > fabsf (p.normal.z)) k = 1;
            else k = 2;
          }

          p.k = k;

          uint u = (k+1)%3;
          uint v = (k+2)%3;

          const csVector3& A = p.vertices[0];

          // precalc normal
          float nkinv = 1.0f/p.normal[k];
          p.n_u = p.normal[u] * nkinv;
          p.n_v = p.normal[v] * nkinv;
          p.n_d = (p.normal * A) * nkinv;


          csVector3 b = p.vertices[2] - p.vertices[0];
          csVector3 c = p.vertices[1] - p.vertices[0];

          float tmp = 1.0f/(b[u] * c[v] - b[v] * c[u]);

          // edge 1
          p.b_nu = -b[v] * tmp;
          p.b_nv = b[u] * tmp;
          p.b_d = (b[v] * A[u] - b[u] * A[v]) * tmp;

          // edge 2
          p.c_nu = c[v] * tmp;
          p.c_nv = -c[u] * tmp;
          p.c_d = (c[u] * A[v] - c[v] * A[u]) * tmp;

          tree->allTriangles.Push (p);
        }
        for (; j < ia.GetSize (); j++)
        {
          tree->boundingBox.AddBoundingVertexSmart (vdata.vertexArray[ia[j]].position);
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
    if(depth++ > 20) 
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
    if (bestCost > node->triangleIndices.GetSize ()*2.0f)
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



  bool KDTreeHelper::CollectPrimitives(const KDTree *tree, 
    RadPrimitivePtrArray &primArray, const csBox3 &overlapAABB)
  {
    // Walk and get all primitives within overlapAABB
    if (!tree->boundingBox.TestIntersect (overlapAABB))
      return false; //No intersection at all

    RadPrimitivePtrSet collectedPrimitives;
   
    CollectPrimitives (tree, tree->rootNode, tree->boundingBox, collectedPrimitives,
      overlapAABB);

    // Copy result to array
    RadPrimitivePtrSet::GlobalIterator si (collectedPrimitives.GetIterator ());
    while (si.HasNext ())
    {
      primArray.Push (si.Next ());
    }

    return collectedPrimitives.GetSize () > 0;
  }

  void KDTreeHelper::CollectPrimitives(const KDTree* tree, const KDTreeNode *node, 
    csBox3 currentBox, RadPrimitivePtrSet& outPrims, const csBox3 &overlapAABB)
  {
    if (node->leftChild)
    {
      // Internal node
      csBox3 childBox = currentBox;

      // Left
      childBox.SetMax (node->splitDimension, node->splitLocation);
      if (childBox.TestIntersect (overlapAABB))
      {
        CollectPrimitives (tree, node->leftChild, childBox, outPrims, overlapAABB);
      }

      // Right
      childBox = currentBox;
      childBox.SetMin (node->splitDimension, node->splitLocation);
      if (childBox.TestIntersect (overlapAABB))
      {
        CollectPrimitives (tree, node->rightChild, childBox, outPrims, overlapAABB);
      }

    }
    else
    {
      // Leaf node
      const csArray<size_t>& triIndices = node->triangleIndices;
      const csArray<KDTreePrimitive>& allTriangles = tree->allTriangles;

      for (unsigned int i = 0; i < triIndices.GetSize (); i++)
      {
        const KDTreePrimitive& prim = allTriangles[triIndices[i]];
        outPrims.Add (prim.primPointer);
      }
    }
  }
}
