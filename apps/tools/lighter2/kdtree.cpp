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

using namespace CrystalSpace;

namespace lighter
{
  // Some helpers for the building.. 
  struct SplitPositionFinder
  {
    SplitPositionFinder (size_t num)
    {
      //Alloc num entries
      splitPoolDEL = splitPool = new SplitPosition[num*2+8];
      for (uint i=0; i<(num*2+6); i++)
      {
        splitPool[i].next = &splitPool[i+1];
      }
      splitPool[num*2+7].next = 0;
    }

    ~SplitPositionFinder ()
    {
      delete[] splitPoolDEL;
    }

    // Prepare for a new node
    void Reset () 
    {
      if (firstEntry)
      {
        SplitPosition *list = firstEntry;
        while (list->next) list = list->next;
        list->next = splitPool;
        splitPool = firstEntry;
      }
      firstEntry = 0;
    }

    // Add a new possible splitpoint
    void AddPosition (float pos)
    {
      SplitPosition* entry = splitPool;
      splitPool = splitPool->next;

      entry->next = 0;
      entry->position = pos;
      if (!firstEntry)
        firstEntry = entry;
      else
      {
        if (pos < firstEntry->position)
        {
          entry->next = firstEntry;
          firstEntry = entry;
        }
        else if (pos == firstEntry->position)
        {
          //recycle, no need to store this position
          entry->next = splitPool;
          splitPool = entry;
        }
        else
        {
          SplitPosition *list = firstEntry;
          while ((list->next) && (pos >= list->next->position))
          {
            if (pos == list->next->position)
            {
              //recycle, no need to store this position
              entry->next = splitPool;
              splitPool = entry;
              return;
            }
            list = list->next;
          }
          entry->next = list->next;
          list->next = entry;
        }
      }
    }


    // Holder for possible splitpoint
    struct SplitPosition
    {
      float position;
      SplitPosition* next;
    };
    SplitPosition  *splitPoolDEL, *splitPool, *firstEntry;
  };
  // The split position finder helper instance
  SplitPositionFinder* splitPositionFinder;
  
  // Functor to collect all split-positions. Use the global helper, so
  // make sure it is initiated
  struct CollectSplitPositions
  {
    CollectSplitPositions (uint axis)
      : axis (axis)
    {}

    void operator () (RadPrimitive *prim)
    {
      float a,b;
      prim->GetExtent (axis,a,b);
      splitPositionFinder->AddPosition (a);
      splitPositionFinder->AddPosition (b);
    }

    uint axis;
  };

  // Functor to compute left/right count
  struct CountLeftRight
  {
    CountLeftRight (uint splitDimension, float splitPos)
      : splitPos (splitPos), splitDimension (splitDimension), 
        left (0), right (0)
    {}

    void operator () (RadPrimitive* prim)
    {
      float minP, maxP;
      prim->GetExtent (splitDimension, minP, maxP);
      if (minP <= splitPos) left++;
      if (maxP >= splitPos) right++;
    }

    float splitPos;
    uint splitDimension, left, right;
  };

  // Functor to distribute left/right
  struct DistributeLeftRight
  {
    DistributeLeftRight (uint splitDimension, float splitPos,
      KDTreeNode *left, KDTreeNode *right)
      : splitPos (splitPos), splitDimension (splitDimension), 
        left (left), right (right)
    {}

    void operator () (RadPrimitive* prim)
    {
      float minP, maxP;
      prim->GetExtent (splitDimension, minP, maxP);
      if (minP <= splitPos) left->radPrimitives.Push (prim);
      if (maxP >= splitPos) right->radPrimitives.Push (prim);
    }

    float splitPos;
    uint splitDimension;
    KDTreeNode *left, *right;
  };

  void KDTreeNode::Subdivide ()
  {
    // Work out split direction, might have different logic later?
    csVector3 bbsize = boundingBox.GetSize ();
    if ((bbsize.x >= bbsize.y) && (bbsize.x >= bbsize.z)) 
      splitDimension = CS_AXIS_X;
    else if ((bbsize.y >= bbsize.x) && (bbsize.y >= bbsize.z)) 
      splitDimension = CS_AXIS_Y;
    else
      splitDimension = CS_AXIS_Z;

    // Find all possible split-locations
    splitPositionFinder->Reset ();
    ForEach (radPrimitives.GetIterator (), CollectSplitPositions (splitDimension));

    // Compute split-cost at each location using SAH
    float SA_inv = 0.5f / (bbsize.x*bbsize.y + bbsize.x*bbsize.z + bbsize.y*bbsize.z);
    float lowestCost = FLT_MAX;
    float bestSplit = 0;

    csBox3 leftBox = boundingBox;
    csBox3 rightBox = boundingBox;

    SplitPositionFinder::SplitPosition *splitPosList = 
      splitPositionFinder->firstEntry;
    while (splitPosList)
    {
      float splitPos = splitPosList->position;
      leftBox.SetMax (splitDimension, splitPos);
      rightBox.SetMin (splitDimension, splitPos);

      //Count elements
      CountLeftRight counter(splitDimension, splitPos);
      ForEach (radPrimitives.GetIterator (), counter);
      
      // Compute the SAH-cost for children
      csVector3 leftSize = leftBox.GetSize ();
      csVector3 rightSize = rightBox.GetSize ();
      
      float SA_left = 2 * (leftSize.x*leftSize.y + leftSize.x*leftSize.z + 
        leftSize.y*leftSize.z);
      float SA_right = 2 * (rightSize.x*rightSize.y + rightSize.x*rightSize.z + 
        rightSize.y*rightSize.z);
      float splitCost = 0.2f + 1.0f * SA_inv * (SA_left*counter.left + 
        SA_right*counter.right);

      if (splitCost < lowestCost)
      {
        lowestCost = splitCost;
        bestSplit = splitPos;
      }
      splitPosList = splitPosList->next;
    }

    // Ok, now we have a best split location for this node, see if it is good enough
    if (lowestCost > radPrimitives.GetSize () * 1.0f)
    {
      return;
    }

    // Split it
    splitLocation = bestSplit;
    leftChild = new KDTreeNode;
    rightChild = new KDTreeNode;

    leftChild->boundingBox = boundingBox;
    rightChild->boundingBox = boundingBox;

    leftChild->boundingBox.SetMax (splitDimension, splitLocation);
    rightChild->boundingBox.SetMin (splitDimension, splitLocation);

    // Distribute nodes between children
    ForEach (radPrimitives.GetIterator (),
      DistributeLeftRight (splitDimension, splitLocation, 
      leftChild, rightChild));

    //@TODO: Add a maxdepth check
    leftChild->Subdivide ();
    rightChild->Subdivide ();

    // To save memory, clear our array
    radPrimitives.DeleteAll ();
  }

  // KDTree functors
  struct CollectPrimitives
  {
    CollectPrimitives (KDTreeNode *root)
      : root (root)
    {
      root->boundingBox.StartBoundingBox (csVector3 (0,0,0));
    }

    void operator() (RadObject* obj)
    {
      RadPrimitiveArray &primArray = obj->GetPrimitives ();
      for (unsigned int i = 0; i < primArray.GetSize (); i++)
      {
        root->radPrimitives.Push (&primArray[i]);
        const Vector3DArray & va = primArray[i].GetVertices ();
        //compute bb at same time
        for (unsigned int j = 0; j < va.GetSize (); j++)
        {
          root->boundingBox.AddBoundingVertexSmart (va[j]);
        }
      }
    }

    KDTreeNode *root;
  };

  void KDTree::BuildTree (const RadObjectHash::Iterator& objectIt)
  {
    rootNode = new KDTreeNode;
    
    // Collect geometries
    CollectPrimitives primitiveCollector (rootNode);
    ForEach (objectIt, primitiveCollector);

    //Setup the splitpointnode helper
    splitPositionFinder = new SplitPositionFinder (rootNode->radPrimitives.GetSize ());

    // Build the kd
    rootNode->Subdivide ();

    delete splitPositionFinder; splitPositionFinder = 0;
  }
}