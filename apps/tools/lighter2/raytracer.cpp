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

#include "raytracer.h"
#include "kdtree.h"

namespace lighter
{
  MailboxHash::MailboxHash ()
    : rayID (1)
  {
    hash = static_cast<HashEntry*> (CS::Memory::AlignedMalloc (
      sizeof (HashEntry) * HASH_SIZE, 16));
    memset (hash, 0, sizeof (HashEntry) * HASH_SIZE);
  }

  MailboxHash::~MailboxHash ()
  {
    CS::Memory::AlignedFree (hash);
  }

  Raytracer::Raytracer (KDTree *tree)
    : tree (tree), traversalStackPtr (0)
  {
    traversalStack = static_cast<kdTraversalS*> (CS::Memory::AlignedMalloc (
      sizeof (kdTraversalS) * MAX_STACK_DEPTH, 32));
  }

  Raytracer::~Raytracer ()
  {
    CS::Memory::AlignedFree (traversalStack);
  }

  bool Raytracer::TraceAnyHit (const Ray &ray, HitPoint& hit)
  {
    if (!tree || !tree->nodeList) return false;

    //RaytraceProfiler prof(1);

    //Copy and clip the ray
    Ray myRay = ray;
    if (!myRay.Clip (tree->boundingBox))
      return false;

    myRay.rayID = mailboxes.GetRayID ();
    float tmin (myRay.minLength), tmax (myRay.maxLength);

    hit.distance = FLT_MAX*0.95f;

    KDTreeNode* node = tree->nodeList;

    size_t nodeOffset[3][2];
    csVector3 invD;
    
    traversalStackPtr = 0;

    for (size_t dim = 0; dim < 3; ++dim)
    {
      if (ray.direction[dim] > 0)
      {
        nodeOffset[dim][0] = 0;
        nodeOffset[dim][1] = 1;
      }
      else
      {
        nodeOffset[dim][0] = 1;
        nodeOffset[dim][1] = 0;
      }
      invD[dim] = 1.0f / ray.direction[dim];
    }
    
    while (true)
    {
      while (!KDTreeNode_Op::IsLeaf (node))
      {
        uint dim = KDTreeNode_Op::GetDimension (node);
        //float thit = (node->inner.splitLocation - ray.origin[dim]) / ray.direction[dim];
        float thit = (node->inner.splitLocation - ray.origin[dim]) * invD[dim];

        KDTreeNode *nearNode, *farNode, *leftNode;
        leftNode = KDTreeNode_Op::GetLeft (node);

        nearNode = leftNode + nodeOffset[dim][0];
        farNode = leftNode + nodeOffset[dim][1];        

        if (thit <= tmin)
        {
          node = farNode;
        }
        else if (thit >= tmax)
        {
          node = nearNode;
        }
        else
        {
          traversalStack[traversalStackPtr].node = farNode;
          traversalStack[traversalStackPtr].tnear = thit;
          traversalStack[traversalStackPtr].tfar = tmax;
          traversalStackPtr++;
          CS_ASSERT(traversalStackPtr < MAX_STACK_DEPTH);

          node = nearNode;
          tmax = thit;
        }
      }

      if (IntersectPrimitives<true> (node, myRay, hit))
        return true;

      if (traversalStackPtr == 0)
        return false;

      traversalStackPtr--;
      node = traversalStack[traversalStackPtr].node;
      tmin = traversalStack[traversalStackPtr].tnear;
      tmax = traversalStack[traversalStackPtr].tfar;
    }

    return true;
  }

  bool Raytracer::TraceClosestHit (const Ray &ray, HitPoint &hit) 
  {
    if (!tree || !tree->nodeList) return false;

    //RaytraceProfiler prof(1);

    //Copy and clip the ray
    Ray myRay = ray;
    if (!myRay.Clip (tree->boundingBox))
      return false;

    myRay.rayID = mailboxes.GetRayID ();
    float tmin (myRay.minLength), tmax (myRay.maxLength);

    KDTreeNode* node = tree->nodeList;

    hit.distance = FLT_MAX*0.95f;

    size_t nodeOffset[3][2];
    csVector3 invD;
    bool haveAnyHit = false;

    traversalStackPtr = 0;

    for (size_t dim = 0; dim < 3; ++dim)
    {
      if (ray.direction[dim] > 0)
      {
        nodeOffset[dim][0] = 0;
        nodeOffset[dim][1] = 1;
      }
      else
      {
        nodeOffset[dim][0] = 1;
        nodeOffset[dim][1] = 0;
      }
      invD[dim] = 1.0f / ray.direction[dim];
    }

    while (true)
    {
      while (!KDTreeNode_Op::IsLeaf (node))
      {
        uint dim = KDTreeNode_Op::GetDimension (node);
        //float thit = (node->inner.splitLocation - ray.origin[dim]) / ray.direction[dim];
        float thit = (node->inner.splitLocation - ray.origin[dim]) * invD[dim];

        KDTreeNode *nearNode, *farNode, *leftNode;
        leftNode = KDTreeNode_Op::GetLeft (node);

        nearNode = leftNode + nodeOffset[dim][0];
        farNode = leftNode + nodeOffset[dim][1];        

        if (thit <= tmin)
        {
          node = farNode;
        }
        else if (thit >= tmax)
        {
          node = nearNode;
        }
        else
        {
          traversalStack[traversalStackPtr].node = farNode;
          traversalStack[traversalStackPtr].tnear = thit;
          traversalStack[traversalStackPtr].tfar = tmax;
          traversalStackPtr++;
          CS_ASSERT(traversalStackPtr < MAX_STACK_DEPTH);

          node = nearNode;
          tmax = thit;
        }
      }

      if (IntersectPrimitives<false> (node, myRay, hit))
        haveAnyHit = true;

      if (traversalStackPtr == 0)
        return haveAnyHit;

      traversalStackPtr--;
      node = traversalStack[traversalStackPtr].node;
      tmin = traversalStack[traversalStackPtr].tnear;
      tmax = traversalStack[traversalStackPtr].tfar;
    }

    return haveAnyHit;
  }

  static const uint mod5[] = {0,1,2,0,1};

  static bool IntersectPrimitiveRay (const KDTreePrimitive &primitive, const Ray &ray,
    HitPoint &hit)
  {
    const uint k = primitive.normal_K;
    const uint ku = mod5[primitive.normal_K+1];
    const uint kv = mod5[primitive.normal_K+2];

    // prefetch?

    const float nd = 1.0f / (ray.direction[k] + 
      primitive.normal_U * ray.direction[ku] + primitive.normal_V * ray.direction[kv]);

    const float f = (primitive.normal_D - ray.origin[k] - 
      primitive.normal_U * ray.origin[ku] - primitive.normal_V * ray.origin[kv]) * nd;

    // Check for distance..
    if (!(ray.maxLength > f && f > ray.minLength)) return false;

    // Compute hitpoint on plane
    const float hu = (ray.origin[ku] + f * ray.direction[ku]);
    const float hv = (ray.origin[kv] + f * ray.direction[kv]);

    // First barycentric coordinate
    const float lambda = (hu * primitive.edgeA_U + hv * primitive.edgeA_V + primitive.edgeA_D);
    if (lambda < 0.0f) 
      return false;

    // Second barycentric coordinate
    const float mu = (hu * primitive.edgeB_U + hv * primitive.edgeB_V + primitive.edgeB_D);
    if (mu < 0.0f) 
      return false;

    // Third barycentric coordinate
    if (lambda + mu > 1.0f) 
      return false;

    // Ok, is a hit, store it
    hit.hitPoint = ray.origin + ray.direction * f;
    hit.distance = f;
    hit.primitive = primitive.primPointer;

    return true;
  }

  template<bool exitFirstHit>
  bool Raytracer::IntersectPrimitives (const KDTreeNode* node, const Ray &ray, 
    HitPoint &hit)
  {

    size_t nIdx, nMax;
    nMax = KDTreeNode_Op::GetPrimitiveListSize (node);;
    bool haveHit = false;
    bool haveAnyHit = false;

    HitPoint thisHit;

    KDTreePrimitive* primList = KDTreeNode_Op::GetPrimitiveList (node);

    for (nIdx = 0; nIdx < nMax; nIdx++)
    {
      KDTreePrimitive* prim = primList + nIdx;

      if (mailboxes.PutPrimitiveRay (prim->primPointer, ray.rayID))
        continue;

      haveHit = IntersectPrimitiveRay (*prim, ray, thisHit);
      if (haveHit)
      {
        haveAnyHit = true;
        if (exitFirstHit) 
        {
          hit = thisHit;
          return true;
        }
        if (thisHit.distance < hit.distance) 
          hit = thisHit;
      }
    }

    return haveAnyHit;
  }
}

