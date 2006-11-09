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

  // Helperstruct for kd traversal
  struct kdTraversalS
  {
    KDTreeNode_Opt *node;
    float tnear, tfar;
  };


  bool Raytracer::TraceRecursive(const Ray &ray, HitPoint& hit, 
    KDTreeNode_Opt* node, float tmin, float tmax) const
  {
    /*if(node->leftChild)
    {
      //Inner
      // Check intersection with ray-plane
      int dim = node->splitDimension;
      float thit = (node->splitLocation - ray.origin[dim]) / ray.direction[dim];
      KDTreeNode *nearNode, *farNode;
      if(ray.direction[dim] > 0) 
      {
        nearNode = node->leftChild;
        farNode = node->rightChild;
      }
      else
      {
        nearNode = node->rightChild;
        farNode = node->leftChild;
      }

      if (thit <= tmin)
      {
        if(TraceRecursive (ray, hit, farNode, tmin, tmax))
          return true;
      }
      else if(thit >= tmax)
      {
        if(TraceRecursive (ray, hit, nearNode, tmin, tmax))
          return true;
      }
      else
      {
        if(TraceRecursive (ray, hit, nearNode, tmin, thit))
          return true;
        if(TraceRecursive (ray, hit, farNode, thit, tmax))
          return true;
      }
    }
    else
    {
      //Leaf
      bool haveHit = IntersectPrimitives (node, ray, hit, true); 
      if(haveHit)
      {
        return true;
      }
    }*/
    return false;
  }

  bool Raytracer::TraceAnyHit (const Ray &ray, HitPoint& hit) const
  {
    if (!tree || !tree->nodeList) return false;

    RaytraceProfiler prof(1);

    //Copy and clip the ray
    Ray myRay = ray;
    if (!myRay.Clip (tree->boundingBox))
      return false;

    float tmin (myRay.minLength), tmax (myRay.maxLength);

    //return TraceRecursive (myRay, hit, tree->rootNode, tmin, tmax);

    KDTreeNode_Opt* node = tree->nodeList;

    //Setup a stack for traversal
    csArray<kdTraversalS> traceStack;
    traceStack.SetCapacity (64);
    
    while (true)
    {
      while (node->inner.flagDimensionAndOffset & 0x04)
      {
        uint dim = node->inner.flagDimensionAndOffset & 0x03;
        float thit = (node->inner.splitLocation - ray.origin[dim]) / ray.direction[dim];

        KDTreeNode_Opt *nearNode, *farNode;
        if(ray.direction[dim] > 0) 
        {
          nearNode = reinterpret_cast<KDTreeNode_Opt*> 
            (node->inner.flagDimensionAndOffset & ~0x07);
          farNode = reinterpret_cast<KDTreeNode_Opt*> 
            (node->inner.flagDimensionAndOffset & ~0x07) + 1;
        }
        else
        {
          nearNode = reinterpret_cast<KDTreeNode_Opt*> 
            (node->inner.flagDimensionAndOffset & ~0x07) + 1;
          farNode = reinterpret_cast<KDTreeNode_Opt*> 
            (node->inner.flagDimensionAndOffset & ~0x07);
        }

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
          kdTraversalS e = {farNode, thit, tmax};
          node = nearNode;
          tmax = thit;
          traceStack.Push (e);
        }
      }

      if (IntersectPrimitives (node, ray, hit, true))
        return true;

      if (traceStack.IsEmpty ())
        return false;

      kdTraversalS n = traceStack.Pop ();
      node = n.node;
      tmin = n.tnear;
      tmax = n.tfar;
    }

    return true;

  }

  bool Raytracer::TraceClosestHit (const Ray &ray, HitPoint &hit) const
  {
    return false;
    /*if (!tree || !tree->rootNode) return false;

    //Copy and clip the ray
    Ray myRay = ray;
    if (!myRay.Clip (tree->rootNode->boundingBox))
      return false;

    float minT (myRay.minLength), maxT (myRay.maxLength);


    // Setup for traversal
    csArray<kdTraversalS> kdstack;
    kdstack.SetCapacity (64);
    float t;
    HitPoint hitPoint, bestHit;
    hitPoint.distance = myRay.maxLength;
    bestHit.distance = FLT_MAX*0.9f;

    KDTreeNode *node = tree->rootNode;

    while (true)
    {
      // traverse until we hit a leaf
      while (node->leftChild)
      {
        uint splitDim = node->splitDimension;

        t = (node->splitLocation - ray.origin[splitDim]) / ray.direction[splitDim];

        if (t <= minT)
        {
          // t <= minLength <= maxLength, cull left traverse right
          node = node->rightChild;
        }
        else if (t >= maxT)
        {
          // minLength <= maxLength <= t, cull right traverse left
          node = node->leftChild;
        }
        else
        {
          // minLength < t < maxLength, traverse both
          kdstack.Push (kdTraversalS (node->rightChild, t, maxT));
          node = node->leftChild;
          maxT = t;
        }
      }

      // Ok, down to leaf.. process it
      bool anyHit = IntersectPrimitives (node, myRay, hitPoint);
      
      if (anyHit)
      {
        if (hitPoint.distance < bestHit.distance) bestHit = hitPoint;
      }

      // Early termination
      if (anyHit && maxT <= hitPoint.distance)
      {
        hit = bestHit;
        return true;
      }

      if (kdstack.GetSize () == 0)
      {
        // no more nodes, and no hit.. return
        hit = bestHit;
        return anyHit;
      }

      kdTraversalS s = kdstack.Pop ();
      node = s.node;
      minT = s.tnear;
      maxT = s.tfar;
    }

    return false;*/
  }

  static const uint mod5[] = {0,1,2,0,1};

  bool IntersectPrimitiveRay (const KDTreePrimitive_Opt &primitive, const Ray &ray,
    HitPoint &hit)
  {
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
    }


    return true;
  }

  bool Raytracer::IntersectPrimitives (const KDTreeNode_Opt* node, const Ray &ray, 
    HitPoint &hit, bool earlyExit /* = false */) const
  {

    size_t nIdx, nMax;
    nMax = node->leaf.numberOfPrimitives;
    bool haveHit = false;
    bool haveAnyHit = false;

    HitPoint bestHit, thisHit;
    bestHit.distance = FLT_MAX*0.95f;

    KDTreePrimitive_Opt* primList = reinterpret_cast<KDTreePrimitive_Opt*> 
      (node->leaf.flagAndOffset & ~0x07);

    for (nIdx = 0; nIdx < nMax; nIdx++)
    {
      haveHit = IntersectPrimitiveRay (primList[nIdx], ray, thisHit);
      if (haveHit)
      {
        haveAnyHit = true;
        if (earlyExit) 
        {
          hit = thisHit;
          return true;
        }
        if (thisHit.distance < bestHit.distance) bestHit = thisHit;
      }
    }

    hit = bestHit;

    return haveAnyHit;
  }
}

