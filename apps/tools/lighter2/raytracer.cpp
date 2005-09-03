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
    KDTreeNode *node;
    float tnear, tfar;

    kdTraversalS ()
    {
    }

    kdTraversalS (KDTreeNode *node, float tn, float tf)
      : node (node), tnear (tn), tfar (tf)
    {
    }
  };


  bool Raytracer::TraceAnyHit (const Ray &ray, HitPoint &hit)
  {
    if (!tree || !tree->rootNode) return false;

    //Copy and clip the ray
    Ray myRay = ray;
    if (!myRay.Clip (tree->rootNode->boundingBox))
      return false;

    float minT (myRay.minLength), maxT (myRay.maxLength);


    // Setup for traversal
    csArray<kdTraversalS> kdstack;
    kdstack.SetCapacity (64);
    float t;
    HitPoint hitPoint;
    hitPoint.distance = myRay.maxLength;

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
      if (IntersectPrimitives (node, myRay, hitPoint, true))
        return true;

      if (kdstack.GetSize () == 0)
      {
        // no more nodes, and no hit.. return
        return false;
      }

      kdTraversalS s = kdstack.Pop ();
      node = s.node;
      minT = s.tnear;
      maxT = s.tfar;
    }

    return false;
  }

  bool Raytracer::TraceClosestHit (const Ray &ray, HitPoint &hit)
  {
    if (!tree || !tree->rootNode) return false;

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

    return false;
  }

  bool IntersectPrimitiveRay (RadPrimitive* primitive, const Ray &ray,
    HitPoint &hit)
  {
    csSegment3 seg (ray.origin, ray.origin + ray.direction*ray.maxLength);

    csVector3 isect;
    bool haveHit = csIntersect3::SegmentPolygon (seg, *primitive, primitive->GetPlane (), isect);
    if (haveHit)
    {
      hit.hitPoint = isect;
      hit.distance = (isect - ray.origin) * ray.direction;
      hit.primitive = primitive;
    }

    return haveHit;
  }

  bool Raytracer::IntersectPrimitives (const KDTreeNode* node, const Ray &ray, 
    HitPoint &hit, bool earlyExit /* = false */)
  {
    size_t nIdx, nMax;
    nMax = node->radPrimitives.GetSize ();
    bool haveHit = false;
    bool haveAnyHit = false;

    HitPoint bestHit, thisHit;
    bestHit.distance = FLT_MAX*0.95f;

    for (nIdx = 0; nIdx < nMax; nIdx++)
    {
      haveHit = IntersectPrimitiveRay (node->radPrimitives[nIdx], ray, thisHit);
      if (haveHit)
      {
        haveAnyHit = true;
        if (earlyExit) 
          return true;
        if (thisHit.distance < bestHit.distance) bestHit = thisHit;
      }
    }

    hit = bestHit;

    return haveAnyHit;
  }
}
