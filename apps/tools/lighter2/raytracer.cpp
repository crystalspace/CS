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
  };


  bool Raytracer::TraceRecursive(const Ray &ray, HitPoint& hit, KDTreeNode* node, float tmin, float tmax) const
  {
    if(node->leftChild)
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
    }
    return false;
  }

  bool Raytracer::TraceAnyHit (const Ray &ray, HitPoint& hit) const
  {
    if (!tree || !tree->rootNode) return false;

    RaytraceProfiler prof(1);

    //Copy and clip the ray
    Ray myRay = ray;
    if (!myRay.Clip (tree->boundingBox))
      return false;

    float tmin (myRay.minLength), tmax (myRay.maxLength);

    //return TraceRecursive (myRay, hit, tree->rootNode, tmin, tmax);

    KDTreeNode* node = tree->rootNode;

    //Setup a stack for traversal
    csArray<kdTraversalS> traceStack;
    traceStack.SetCapacity (64);
    
    while (true)
    {
      while (node->leftChild)
      {
        uint dim = node->splitDimension;
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

  bool IntersectPrimitiveRay (const KDTreePrimitive &primitive, const Ray &ray,
    HitPoint &hit)
  {
    float nom = - (primitive.normal * (ray.origin - primitive.vertices[0]));
    float den = primitive.normal * ray.direction;

    if(den < 0) 
      return false; //backface culling


    float dist = nom / den;
    if(dist < ray.minLength || dist > ray.maxLength)
      return false;

    csVector3 I = ray.origin + ray.direction * dist;

    // Is inside?
    csVector3 u, v, w;
    float uu, uv, vv, wu, wv, D;

    u = primitive.vertices[2] - primitive.vertices[0];
    v = primitive.vertices[1] - primitive.vertices[0];

    uu = u*u;
    uv = u*v;
    vv = v*v;
    w = I - primitive.vertices[0];
    wu = w*u;
    wv = w*v;
    D = uv*uv - uu*vv;

    // Test barycentric
    float s, t;
    s = (uv*wv - vv*wu) / D;
    if (s < 0.0f || s > 1.0f)
      return false;

    t = (uv*wu - uu*wv) / D;
    if (t < 0.0f || (t+s) > 1.0f)
      return false;

    hit.hitPoint = I;
    hit.distance = dist;
    hit.primitive = primitive.primPointer;
 


    return true;
  }

  bool Raytracer::IntersectPrimitives (const KDTreeNode* node, const Ray &ray, 
    HitPoint &hit, bool earlyExit /* = false */) const
  {
    size_t nIdx, nMax;
    nMax = node->triangleIndices.GetSize ();
    bool haveHit = false;
    bool haveAnyHit = false;

    HitPoint bestHit, thisHit;
    bestHit.distance = FLT_MAX*0.95f;

    for (nIdx = 0; nIdx < nMax; nIdx++)
    {
      haveHit = IntersectPrimitiveRay (tree->allTriangles[node->triangleIndices[nIdx]], ray, thisHit);
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

