/*
  Copyright (C) 2005-2006 by Marten Svanfeldt

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

#include "common.h"

#include "kdtree.h"
#include "material.h"
#include "primitive.h"
#include "statistics.h"
#include "object.h"

#include "csutil/alignedalloc.h"

using namespace CS;

namespace lighter
{

  KDTree::~KDTree ()
  {
    CS::Memory::AlignedFree (this->primitives);
    CS::Memory::AlignedFree (this->nodeList);
  }

  KDTreeBuilder::KDTreeBuilder ()
    : boxAllocator (1024), nodeAllocator (1024)
  {}

  KDTree* KDTreeBuilder::BuildTree (ObjectHash::GlobalIterator& objects,
                                    Statistics::Progress& progress)
  {
    progress.SetProgress (0);
    objects.Reset ();

    if (!objects.HasNext ())
      return 0;

    // Collect all primitives into endpoints and boxes for building
    SetupEndpoints (objects);
    progress.SetProgress (0.33f);

    // Recursively build internal nodes from the boxes lists
    KDNode* rootNode = nodeAllocator.Alloc ();
    BuildKDNodeRecursive (&endPointList, rootNode, objectExtents, numPrimitives, 0);
    progress.SetProgress (0.66f);

    // Optimize them and create a real kd-tree and nodes
    KDTree* tree = SetupRealTree (rootNode);
    
    //Clean up some memory
    boxAllocator.DeleteAll ();
    nodeAllocator.DeleteAll ();

    progress.SetProgress (1);
    return tree;
  }

  bool KDTreeBuilder::SetupEndpoints (ObjectHash::GlobalIterator& objects)
  {
    numPrimitives = 0;
    PrimBox *box = 0, *next = boxAllocator.Alloc (), *first = next;
    while (objects.HasNext())
    {
      csRef<Object> obj = objects.Next ();

      csArray<PrimitiveArray>& allPrimitives = obj->GetPrimitives ();
      for (size_t i = 0; i < allPrimitives.GetSize (); ++i)
      {
        //For all submeshes...
        PrimitiveArray& primArray = allPrimitives[i];
        for (size_t j = 0; j < primArray.GetSize (); ++j)
        {
          Primitive& prim = primArray[j];
          numPrimitives++;

          box = next;
          next = boxAllocator.Alloc ();
          box->flags = PrimBox::STATE_STRADDLING;
          box->primitive = &prim;

          //Extract AABB and extract end-points
          primHelper.Init (&prim);
          objectExtents.AddBoundingBox (primHelper.aabb);
          
          for (size_t a = 0; a < 3; ++a)
          {
            //Min
            box->side[0].SetPosition (a, primHelper.aabb.Min (a));

            //Max
            box->side[1].SetNext (a, &next->side[0]);
            box->side[1].SetPosition (a, primHelper.aabb.Max (a));

            if (primHelper.aabb.Min (a) == primHelper.aabb.Max (a))
            {
              //planar in axis
              box->side[0].SetSide (a, EndPoint::SIDE_PLANAR);
              box->side[1].SetSide (a, EndPoint::SIDE_PLANAR);
              box->side[0].SetNext (a, &next->side[0]);
            }
          }

        }
      }
    }

    if (box != 0)
    {
      //Finish and sort the lists
      for (size_t a = 0; a < 3; ++a)
      {
	//Finish last box
	box->side[1].SetNext (a, 0);
	if (box->side[0].GetSide (a) == EndPoint::SIDE_PLANAR)
	  box->side[0].SetNext (a, 0);
  
	//Save first and sort
	endPointList.head[a] = &first->side[0];
	endPointList.SortList (a);
      }
    }

    return true;
  }

  bool KDTreeBuilder::BuildKDNodeRecursive (EndPointList* epList, KDNode* node, 
    csBox3 aabb, size_t numPrim, size_t treeDepth)
  {
    //Setup SAH variables
    const float fullArea = aabb.Area ();
    const float invFA = 1.0f/fullArea;

    //Initialize best cost to not splitting
    float bestCost = INTERSECTION_CONST * numPrim;
    float bestPosition = -1;

    //Best axis and side for planar
    uint bestAxis = ~0, bestSide = ~0;
    size_t bestNLeft = 0, bestNRight = 0, bestNPlanar = 0;

    for (uint axis = 0; axis < 3; ++axis)
    {
      // Don't try to split if it is too small
      if (aabb.GetSize ()[axis] < (1e-9 * NODE_SIZE_EPSILON))
        continue;

      //Counters
      size_t NLeft = 0, NRight = numPrim, NPlanar = 0;

      //Side-boxes
      csBox3 LBox = aabb, RBox = aabb;

      //Iterate
      EndPoint* ep = epList->head[axis];
      while (ep)
      {
        const float pos = ep->GetPosition (axis);
        size_t dl = 0, dr = 0;

        //Update on left
        while (ep && ep->GetPosition (axis) == pos && ep->GetSide (axis) == EndPoint::SIDE_END)
        {
          dl++;
          ep = ep->GetNext (axis);
        }

        //Update planar
        while (ep && ep->GetPosition (axis) == pos && ep->GetSide (axis) == EndPoint::SIDE_PLANAR)
        {
          NPlanar++;
          ep = ep->GetNext (axis);
        }

        //Update on right
        while (ep && ep->GetPosition (axis) == pos && ep->GetSide (axis) == EndPoint::SIDE_START)
        {
          dr++;
          ep = ep->GetNext (axis);
        }

        NRight -= (NPlanar + dl);

        //Update box-sizes
        LBox.SetMax (axis, pos);
        RBox.SetMin (axis, pos);

        //Compute SAH and a bonus (courtesy of Arauna realtime ray tracer)
        const float LA = LBox.Area ();
        const float RA = RBox.Area ();

        const float totalWidth = aabb.GetSize ()[axis];
        const float fractionL = LBox.GetSize ()[axis] / totalWidth;
        const float fractionR = RBox.GetSize ()[axis] / totalWidth;

        float bonusL = 1.0f, bonusR = 1.0f;

        const float minFraction = 0.1f + 0.01f * treeDepth;

        //Empty cutoff bonus, see which side is better for planars
        if ((NLeft == 0 || (NRight + NPlanar) == 0) &&
          fractionL > minFraction && fractionR > minFraction)
        {
          if (NLeft == 0)
            bonusR = 0.7f + 0.3f * fractionR;
          else
            bonusR = 0.7f + 0.3f * fractionL;
        }
        if (((NLeft + NPlanar) == 0 || NRight == 0) &&
          fractionL > minFraction && fractionR > minFraction)
        {
          if (NRight == 0)
            bonusL = 0.7f + 0.3f * fractionL;
          else
            bonusL = 0.7f + 0.3f * fractionR;
        }

        //Compute the costs
        const float costL = TRAVERSAL_COST + 
          INTERSECTION_CONST * bonusL * invFA * (LA * (NLeft + NPlanar) + RA * NRight);
        const float costR = TRAVERSAL_COST + 
          INTERSECTION_CONST * bonusR * invFA * (LA * NLeft + RA * (NRight + NPlanar));

        if (costL < bestCost)
        {
          bestCost = costL;
          bestPosition = pos;
          bestSide = 0;
          bestAxis = axis;
          bestNLeft = NLeft; bestNPlanar = NPlanar; bestNRight = NRight;
        }
        if (costR < bestCost)
        {
          bestCost = costR;
          bestPosition = pos;
          bestSide = 1;
          bestAxis = axis;
          bestNLeft = NLeft; bestNPlanar = NPlanar; bestNRight = NRight;
        }

        // Update the counts
        NLeft += (dr + NPlanar);
        NPlanar = 0;
      }
    }

    if (bestSide == (uint)~0 || numPrim <= PRIMS_PER_LEAF || treeDepth == MAX_DEPTH)
    {
      //No best side, make a leaf
      EndPoint* ep = epList->head[0];
      while (ep)
      {
        PrimBox* pb = ep->GetBox ();
        if (pb->flags != PrimBox::STATE_PROCESSED)
        {
          node->primitives.Push (pb->primitive);
          pb->flags = PrimBox::STATE_PROCESSED;
        }

        ep = ep->GetNext (0);
      }
      return true;
    }

    //Build child nodes
    size_t NLR[2] = {bestNLeft + (bestSide == 0 ? bestNPlanar : 0),
      bestNRight + (bestSide == 0 ? 0 : bestNPlanar)};
    csBox3 boxLR[2] = {aabb, aabb};
    boxLR[0].SetMax (bestAxis, bestPosition);
    boxLR[1].SetMin (bestAxis, bestPosition);

    //Classify all boxes
    {
      EndPoint* ep = epList->head[bestAxis];
      while (ep)
      {
        const size_t side = ep->GetSide (bestAxis);
        const float pos = ep->GetPosition (bestAxis);
        PrimBox* pb = ep->GetBox ();

        if (side == EndPoint::SIDE_END && 
          pos <= bestPosition)
          pb->flags = PrimBox::STATE_LEFT;
        else if (side == EndPoint::SIDE_START &&
          pos >= bestPosition)
          pb->flags = PrimBox::STATE_RIGHT;
        else if (side == EndPoint::SIDE_PLANAR)
        {
          //Use bestSide to classify right/left
          if(bestSide == 0)
            pb->flags = (pos <= bestPosition ? 
            PrimBox::STATE_LEFT : PrimBox::STATE_RIGHT);
          else
            pb->flags = (pos < bestPosition ? 
            PrimBox::STATE_LEFT : PrimBox::STATE_RIGHT);
        }

        ep = ep->GetNext (bestAxis);
      }
    }

    //Split into two new EndPointLists
    EndPointList newEPList[2];
    for (size_t axis = 0; axis < 3; ++axis)
    {
      EndPoint* ep = epList->head[axis];
      EndPoint *tailL = 0, *tailR = 0;
      
      while (ep)
      {
        EndPoint* next = ep->GetNext (axis);
        PrimBox* pb = ep->GetBox ();

        //Split according to box classification
        if (pb->flags == PrimBox::STATE_LEFT)
        {
          if (tailL)
            tailL->SetNext (axis, ep);
          else
            newEPList[0].head[axis] = ep;

          ep->SetNext (axis, 0);
          tailL = ep; 
        } 
        else if (pb->flags == PrimBox::STATE_RIGHT)
        {
          if (tailR)
            tailR->SetNext (axis, ep);
          else
            newEPList[1].head[axis] = ep;

          ep->SetNext (axis, 0);
          tailR = ep;
        }
        else
        {
          //Straddling, have to split it into two
          pb->flags = PrimBox::STATE_PROCESSED;
          if (!pb->clone)
          {
            pb->clone = boxAllocator.Alloc ();
            memcpy (pb->clone, pb, sizeof(PrimBox));
          }

          const size_t otherSide = ep->GetSide (axis) == EndPoint::SIDE_END ? 1 : 0;
          EndPoint* clEp = &pb->clone->side[otherSide];

          if (tailL)
            tailL->SetNext (axis, ep);
          else
            newEPList[0].head[axis] = ep;
          ep->SetNext (axis, 0);
          tailL = ep;

          if (tailR)
            tailR->SetNext (axis, clEp);
          else
            newEPList[1].head[axis] = clEp;
          clEp->SetNext (axis, 0);
          tailR = clEp;
        }

        ep = next;
      }
    }

    //Prune invalid primitives
    for (int i = 0; i < 2; ++i)
    {
      EndPoint* ep = newEPList[i].head[0];

      bool needCleanup = false, needResort = false;
      //Clip primitives if box is split into two
      while (ep)
      {
        PrimBox* pb = ep->GetBox ();

        if (pb->flags == PrimBox::STATE_PROCESSED)
        {
          pb->flags = PrimBox::STATE_LEFT + i;
          pb->clone = 0;

          primHelper.Init (pb->primitive);
          primHelper.Clip (boxLR[i]);

          if (primHelper.numVerts < 3)
          {
            //degenerated, clean it up
            for (size_t axis = 0; axis < 3; ++axis)
            {
              pb->side[0].SetSide (axis, EndPoint::SIDE_INVALID);
              pb->side[1].SetSide (axis, EndPoint::SIDE_INVALID);
            }
            NLR[i]--;
            needCleanup = true;
          }
          else
          {
            //new positions, need to resort
            for (size_t axis = 0; axis < 3; ++axis)
            {
              pb->side[0].SetPosition (axis, primHelper.aabb.GetMin (axis));
              pb->side[1].SetPosition (axis, primHelper.aabb.GetMax (axis));
            }
            needResort = true;
          }
        }

        pb->flags = PrimBox::STATE_STRADDLING; //reset
        ep = ep->GetNext (0);
      }

      //Do the accual resort/cleanup
      for (uint axis = 0; axis < 3; ++axis)
      {
        if (needCleanup)
        {
          EndPoint* ep = newEPList[i].head[axis];
          EndPoint* prev = 0;

          while (ep)
          {
            if (ep->GetSide (axis) == EndPoint::SIDE_INVALID)
            {
              if (!prev)
                newEPList[i].head[axis] = ep->GetNext (axis);
              else
                prev->SetNext (axis, ep->GetNext (axis));
            }
            else
            {
              prev = ep;
            }
            ep = ep->GetNext (axis);
          }
        }

        if (needResort)
          newEPList[i].SortList (axis);
      }
    }

    //Now recurse and build nodes etc
    node->splitDimension = bestAxis;
    node->splitLocation = bestPosition;

    //Split here
    node->leftChild = nodeAllocator.Alloc ();
    node->rightChild = nodeAllocator.Alloc ();
    
    BuildKDNodeRecursive (&newEPList[0], node->leftChild, boxLR[0],
      NLR[0], treeDepth+1);
    BuildKDNodeRecursive (&newEPList[1], node->rightChild, boxLR[1],
      NLR[1], treeDepth+1);
    
    return true;
  }

  //Copy the tree
  struct CopyFunctor
  {
    CopyFunctor (KDTree* t)
      : tree (t), usedNodes (2  ), usedPrims (0)
    {
    }

    void CopyNodes (KDTreeBuilder::KDNode* node, KDTreeNode* newNode)
    {
      if (!node)
        return;

      //Copy node
      if (node->leftChild)
      {
        // inner node
        KDTreeNode_Op::SetLeaf (newNode, false);
        KDTreeNode_Op::SetDimension (newNode, node->splitDimension);
        KDTreeNode_Op::SetLocation (newNode, node->splitLocation);

        // Get nodes for children
        KDTreeNode* left = tree->nodeList + usedNodes++;
        KDTreeNode* right = tree->nodeList + usedNodes++;

        KDTreeNode_Op::SetLeft (newNode, left);

        CopyNodes (node->leftChild, left);
        CopyNodes (node->rightChild, right);
      }
      else
      {
        // leaf node
        KDTreeNode_Op::SetLeaf (newNode, true);
        KDTreeNode_Op::SetPrimitiveListSize (newNode, 
          node->primitives.GetSize ());

        KDTreePrimitive* prims = tree->primitives + usedPrims;
        usedPrims += node->primitives.GetSize ();

        KDTreeNode_Op::SetPrimitiveList (newNode, prims);

        // Setup all primitives
        for (size_t i = 0; i < node->primitives.GetSize (); ++i)
        {
          Primitive* prim = node->primitives[i];

          KDTreePrimitive &optPrim = prims[i];

          // Setup optimized
          optPrim.primPointer = prim;

          int32 kdFlags = 0;

          if (prim->GetObject ()->GetFlags ().Check (OBJECT_FLAG_NOSHADOW))
            kdFlags |= KDPRIM_FLAG_NOSHADOW;
	  if (prim->GetMaterial() && prim->GetMaterial()->IsTransparent())
	    kdFlags |= KDPRIM_FLAG_TRANSPARENT;

          //Extract our info
          const csVector3& N = prim->GetPlane ().Normal ();
          ObjectVertexData &vdata = prim->GetVertexData ();
          const Primitive::TriangleType& t = prim->GetTriangle ();
          const csVector3& A = vdata.positions[t.a];
          const csVector3& B = vdata.positions[t.b];
          const csVector3& C = vdata.positions[t.c];

          // Find max normal direction
          int k = N.DominantAxis ();

          optPrim.normal_K = k | kdFlags;

          size_t u = (k+1)%3;
          size_t v = (k+2)%3;

          // precalc normal
          float nkinv = 1.0f/N[k];
          optPrim.normal_U = N[u] * nkinv;
          optPrim.normal_V = N[v] * nkinv;
          optPrim.normal_D = (N * A) * nkinv;


          csVector3 bb = C - A;
          csVector3 cc = B - A;

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

    KDTree* tree;
    size_t usedNodes, usedPrims;
  };

  //Count number of nodes and number of primitive slots  
  struct CountFunctor
  {
    CountFunctor ()
      : numNodes (0), numPrimSlots (0), maxDepth (0), sumDepth (0)
    {
    }
    
    void CountNode (KDTreeBuilder::KDNode* node, size_t depth = 0)
    {
      if (!node)
        return;

      numNodes++;
      numPrimSlots += node->primitives.GetSize ();

      //At the same time, collect global statistics
      if (!node->leftChild)
      {
        // Leaf
        globalStats.kdtree.leafNodes++;
        sumDepth += depth;
        maxDepth = csMax (maxDepth, depth);
      }


      CountNode (node->leftChild, depth+1);
      CountNode (node->rightChild, depth+1);
    }

    size_t numNodes, numPrimSlots, maxDepth, sumDepth;
  };

  KDTree* KDTreeBuilder::SetupRealTree (KDNode* rootNode)
  {
    CountFunctor counter;
    counter.CountNode (rootNode);

    globalStats.kdtree.numNodes += counter.numNodes;
    globalStats.kdtree.numPrimitives += counter.numPrimSlots;
    globalStats.kdtree.sumDepth += counter.sumDepth;
    globalStats.kdtree.maxDepth = csMax(counter.maxDepth, globalStats.kdtree.maxDepth);

    //Allocate
    KDTree* newTree = new KDTree;
    newTree->boundingBox = objectExtents;
    newTree->nodeList = static_cast<KDTreeNode*> (
      CS::Memory::AlignedMalloc (sizeof(KDTreeNode) * (counter.numNodes + 1), 32));
    newTree->primitives = static_cast<KDTreePrimitive*> (
      CS::Memory::AlignedMalloc (sizeof(KDTreePrimitive) * counter.numPrimSlots, 32));

    CopyFunctor copyer (newTree);
    copyer.CopyNodes (rootNode, newTree->nodeList);

    return newTree;
  }


  //Helper functions
  void KDTreeBuilder::EndPointList::Insert (size_t axis, EndPoint* ep)
  {
    //Do a linear search for the place to insert it
    float epPos = ep->GetPosition (axis);
    size_t epSide = ep->GetSide (axis);

    EndPoint *it = head[axis], *prev = 0;
    while (it)
    {
      float itPos = it->GetPosition (axis);
      size_t itSide = it->GetSide (axis);

      if ((epPos < itPos) ||
          ((epPos == itPos) && (epSide <= itSide)))
      {
        //Found a spot, insert it
        if (prev)
        {
          ep->SetNext (axis, prev->GetNext (axis));
          prev->SetNext (axis, ep);
        }
        else
        {
          ep->SetNext (axis, head[axis]);
          head[axis] = ep;
        }
        return; //finished
      }

      prev = it;
      it = it->GetNext (axis);
    }

    //If we haven't found a spot, insert at the back
    ep->SetNext (axis, 0);
    if (prev)
      prev->SetNext (axis, ep);
    else
      head[axis] = ep;
  }

  void KDTreeBuilder::EndPointList::Remove (size_t axis, EndPoint* ep)
  {
    EndPoint *it = head[axis], *prev = 0;
    while (it)
    {
      if (it == ep)
      {
        //Unlink
        if (prev)
          prev->SetNext (axis, it->GetNext (axis));
        else
          head[axis] = it->GetNext (axis);

        return;
      }

      prev = it;
      it = it->GetNext (axis);
    }
  }

  void KDTreeBuilder::EndPointList::SortList (size_t axis)
  {
    //Merge-sort for linked lists as described by Simon Tatham
    
    if (!head[axis] || !head[axis]->GetNext (axis))
      return; //Trivial cases with 0 or 1 element

    size_t insize, numMerges, pSize, qSize;
    EndPoint *p, *q, *e;

    insize = 1;

    while (1)
    {
      //Setup pass
      p = head[axis];
      head[axis] = tail[axis] = 0;

      numMerges = 0;

      while (p)
      {
        //At least one merge...
        numMerges++;

        //Step insize steps forward from p
        q = p;
        pSize = 0;
        for (size_t i = 0; i < insize; ++i)
        {
          pSize++;
          q = q->GetNext (axis);
          if (!q)
            break;
        }

        //Unless we're out of q, there is something to merge
        qSize = insize;

        //Merge the two lists
        while (pSize > 0 || (qSize > 0 && q))
        {
          //decide where to fetch the element
          if (pSize == 0)
          {
            // empty p, take from q
            e = q; q = q->GetNext (axis); qSize--;
          }
          else if (qSize == 0 || !q)
          {
            // empty q, take from p
            e = p; p = p->GetNext (axis); pSize--;
          }
          else
          {
            //check whichever is smallest
            float ppos = p->GetPosition (axis);
            float qpos = q->GetPosition (axis);
            
            if (ppos < qpos ||
              (ppos == qpos && p->GetSide (axis) < q->GetSide (axis)))
            {
              //p smaller
              e = p; p = p->GetNext (axis); pSize--;
            }
            else
            {
              //q smaller
              e = q; q = q->GetNext (axis); qSize--;
            }
          }

          //Link it into the list
          if (tail[axis])
            tail[axis]->SetNext (axis, e);
          else
            head[axis] = e;

          tail[axis] = e;
        }

        //p has stepped, and so has q
        p = q;  
      }

      tail[axis]->SetNext (axis, 0);

      if (numMerges <= 1)
        break; //finished!

      insize *= 2;
    }
  }

  void KDTreeBuilder::PrimHelper::Init (const Primitive* prim)
  {
    const ObjectVertexData &vdata = prim->GetVertexData ();
    const Primitive::TriangleType& t = prim->GetTriangle ();
    const csVector3& A = vdata.positions[t.a];
    const csVector3& B = vdata.positions[t.b];
    const csVector3& C = vdata.positions[t.c];

    vertices[0] = A;
    vertices[1] = B;
    vertices[2] = C;
    numVerts = 3;
    numTempVerts = 0;
    UpdateBB ();
  }

  void KDTreeBuilder::PrimHelper::Clip (const csBox3& box)
  {
    bool haveClipped = false;

    haveClipped |= ClipAxis (box.MinX (), 1, 0);
    haveClipped |= ClipAxis (box.MaxX (),-1, 0);
    haveClipped |= ClipAxis (box.MinY (), 1, 1);
    haveClipped |= ClipAxis (box.MaxY (),-1, 1);
    haveClipped |= ClipAxis (box.MinZ (), 1, 2);
    haveClipped |= ClipAxis (box.MaxZ (),-1, 2);

    if (haveClipped)
      UpdateBB ();
  }

  bool KDTreeBuilder::PrimHelper::ClipAxis(float pos, float direction, size_t axis)
  {
    if (!numVerts)
      return false;

    csPlane3 p;
    p.norm = csVector3(0.0f);
    p.norm[axis] = -direction;
    p.DD = direction * pos;

    numTempVerts = 10;
    uint8 clipStatus = p.ClipPolygon (vertices, numVerts, tempVertices, 
      numTempVerts, 0);

    if (clipStatus == CS_CLIP_OUTSIDE)
    {
      numVerts = 0;
      return true;
    }
    else if (clipStatus == CS_CLIP_INSIDE)
    {
      return false;
    }
    else
    {
      memcpy (vertices, tempVertices, numTempVerts*sizeof(csVector3));
      numVerts = numTempVerts;
    }
    return true;
  }

  void KDTreeBuilder::PrimHelper::UpdateBB ()
  {
    if (!numVerts)
      return;

    aabb.StartBoundingBox (vertices[0]);
    for (size_t i = 1; i < numVerts; ++i)
    {
      aabb.AddBoundingVertexSmart (vertices[i]);
    }
  }

  // -- Helper 
  bool KDTreeHelper::CollectPrimitives(const KDTree *tree, 
    PrimitivePtrArray &primArray, const csBox3 &overlapAABB)
  {
    // Walk and get all primitives within overlapAABB
    if (!tree->boundingBox.TestIntersect (overlapAABB))
      return false; //No intersection at all

    PrimitivePtrSet collectedPrimitives;

    CollectPrimitives (tree, tree->nodeList, tree->boundingBox, collectedPrimitives,
      overlapAABB);

    // Copy result to array
    PrimitivePtrSet::GlobalIterator si (collectedPrimitives.GetIterator ());
    while (si.HasNext ())
    {
      primArray.Push (si.Next ());
    }

    return collectedPrimitives.GetSize () > 0;
  }

  void KDTreeHelper::CollectPrimitives(const KDTree* tree, 
    const KDTreeNode *node, csBox3 currentBox, PrimitivePtrSet& outPrims, 
    const csBox3 &overlapAABB)
  {
    if (KDTreeNode_Op::IsLeaf (node))
    {
      // Leaf node
      KDTreePrimitive* primList = KDTreeNode_Op::GetPrimitiveList (node);
      size_t numPrim = KDTreeNode_Op::GetPrimitiveListSize (node);

      for (size_t i = 0; i < numPrim; i++)
      {
        outPrims.Add (primList[i].primPointer);
      }
    }
    else
    {
      // Internal node
      csBox3 childBox = currentBox;

      size_t dim = KDTreeNode_Op::GetDimension (node);
      float pos = KDTreeNode_Op::GetLocation (node);

      // Left
      childBox.SetMax (dim, pos);
      if (childBox.TestIntersect (overlapAABB))
      {
        CollectPrimitives (tree, KDTreeNode_Op::GetLeft (node), 
          childBox, outPrims, overlapAABB);
      }

      // Right
      childBox = currentBox;
      childBox.SetMin (dim, pos);
      if (childBox.TestIntersect (overlapAABB))
      {
        CollectPrimitives (tree, KDTreeNode_Op::GetLeft (node) + 1,
          childBox, outPrims, overlapAABB);
      }
    }
  }
}
