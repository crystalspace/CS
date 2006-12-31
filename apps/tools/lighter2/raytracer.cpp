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
    : tree (tree)
  {
    traversalStack = static_cast<kdTraversalS*> (CS::Memory::AlignedMalloc (
      sizeof (kdTraversalS) * MAX_STACK_DEPTH, 32));
  }

  Raytracer::~Raytracer ()
  {
    CS::Memory::AlignedFree (traversalStack);
  }

  class IgnoreNone
  {
  public:
    IgnoreNone () {}

    bool Ignore (const Primitive* prim) const
    {
      return false;
    }
    void RegisterHit (const Primitive* prim) {}
  };

  class IgnorePrimitive
  {
    const Primitive* prim;
  public:
    IgnorePrimitive (const Primitive* prim) : prim (prim) {}

    bool Ignore (const Primitive* prim) const
    {
      return this->prim == prim;
    }
    void RegisterHit (const Primitive* prim) {}
  };

  class IgnoreCallback
  {
    HitIgnoreCallback* cb;
  public:
    IgnoreCallback (HitIgnoreCallback* cb) : cb (cb) {}

    bool Ignore (const Primitive* prim) const
    {
      return cb->Ignore (prim);
    }
    void RegisterHit (const Primitive* prim)
    {
      cb->RegisterHit (prim);
    }
  };

  enum 
  {
    TraverseHit      = 0x1,
    TraverseFinished = 0x2
  };

  template<bool exitFirstHit, class IgnoreTest>
  class KDTTraverser
  {
    Raytracer& raytracer;
    const Ray& ray;
    IgnoreTest& ignore;
    float tmin, tmax;

    KDTreeNode* node;

    size_t nodeOffset[3][2];
    csVector3 invD;
    size_t traversalStackPtr;
    Raytracer::kdTraversalS* traversalStack;
  public:
    KDTTraverser (Raytracer& raytracer, const Ray& ray, 
                  IgnoreTest& ignore) : 
      raytracer (raytracer),  ray (ray), ignore (ignore),
      tmin (ray.minLength), tmax (ray.maxLength), 
      node (raytracer.tree->nodeList), traversalStackPtr (0), 
      traversalStack (raytracer.traversalStack)
    {
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
    }

    int Traverse (HitPoint& hit)
    {
      while (!KDTreeNode_Op::IsLeaf (node))
      {
        uint dim = KDTreeNode_Op::GetDimension (node);
        //float thit = (node->inner.splitLocation - ray.origin[dim]) / ray.direction[dim];
        float thit = (node->inner.splitLocation - ray.origin[dim]) * invD[dim];

        csVector3 hitPoint = ray.origin + ray.direction * thit;

        KDTreeNode *nearNode, *farNode, *leftNode;
        leftNode = KDTreeNode_Op::GetLeft (node);

        nearNode = leftNode + nodeOffset[dim][0];
        farNode = leftNode + nodeOffset[dim][1];        

        if (thit < tmin)
        {
          node = farNode;
        }
        else if (thit > tmax)
        {
          node = nearNode;
        }
        else
        {
          traversalStack[traversalStackPtr].node = farNode;
          traversalStack[traversalStackPtr].tnear = thit;
          traversalStack[traversalStackPtr].tfar = tmax;
          traversalStackPtr++;
          CS_ASSERT(traversalStackPtr < Raytracer::MAX_STACK_DEPTH);

          node = nearNode;
          tmax = thit;
        }
      }

      int result = 0;
      if (raytracer.IntersectPrimitives<exitFirstHit, IgnoreTest> (node, ray, hit, ignore))
        result |= TraverseHit;

      if (traversalStackPtr == 0)
        result |= TraverseFinished;

      traversalStackPtr--;
      node = traversalStack[traversalStackPtr].node;
      tmin = traversalStack[traversalStackPtr].tnear;
      tmax = traversalStack[traversalStackPtr].tfar;

      return result;
    }
  };

  bool Raytracer::TraceAnyHit (const Ray &ray, HitPoint& hit, 
                               HitIgnoreCallback* ignoreCB)
  {
    if (!tree || !tree->nodeList) return false;

    //RaytraceProfiler prof(1);

    //Copy and clip the ray
    Ray myRay = ray;
    if (!myRay.Clip (tree->boundingBox))
      return false;

    myRay.rayID = mailboxes.GetRayID ();
    hit.distance = FLT_MAX*0.95f;

    if (ignoreCB)
    {
      IgnoreCallback ignorer (ignoreCB);
      KDTTraverser<true, IgnoreCallback> traverser (*this, myRay, ignorer);
      while (true)
      {
        int result = traverser.Traverse (hit);
        if (result & TraverseHit)
          return true;
        
        if (result & TraverseFinished)
          return false;
      }
    }
    else if (ray.ignorePrimitive)
    {
      IgnorePrimitive ignorer (ray.ignorePrimitive);
      KDTTraverser<true, IgnorePrimitive> traverser (*this, myRay, ignorer);
      while (true)
      {
        int result = traverser.Traverse (hit);
        if (result & TraverseHit)
          return true;
        
        if (result & TraverseFinished)
          return false;
      }
    }
    else
    {
      IgnoreNone ignorer;
      KDTTraverser<true, IgnoreNone> traverser (*this, myRay, ignorer);
      while (true)
      {
        int result = traverser.Traverse (hit);
        if (result & TraverseHit)
          return true;
        
        if (result & TraverseFinished)
          return false;
      }
    }

    return true;
  }

  bool Raytracer::TraceClosestHit (const Ray &ray, HitPoint &hit, 
                                   HitIgnoreCallback* ignoreCB)
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

    bool haveAnyHit = false;
    if (ignoreCB)
    {
      IgnoreCallback ignorer (ignoreCB);
      KDTTraverser<false, IgnoreCallback> traverser (*this, myRay, ignorer);
      while (true)
      {
        int result = traverser.Traverse (hit);
        if (result & TraverseHit)
          haveAnyHit = true;
        
        if (result & TraverseFinished)
          return haveAnyHit;
      }
    }
    else if (ray.ignorePrimitive)
    {
      IgnorePrimitive ignorer (ray.ignorePrimitive);
      KDTTraverser<false, IgnorePrimitive> traverser (*this, myRay, ignorer);
      while (true)
      {
        int result = traverser.Traverse (hit);
        if (result & TraverseHit)
          haveAnyHit = true;
        
        if (result & TraverseFinished)
          return haveAnyHit;
      }
    }
    else
    {
      IgnoreNone ignorer;
      KDTTraverser<false, IgnoreNone> traverser (*this, myRay, ignorer);
      while (true)
      {
        int result = traverser.Traverse (hit);
        if (result & TraverseHit)
          haveAnyHit = true;
        
        if (result & TraverseFinished)
          return haveAnyHit;
      }
    }

    return haveAnyHit;
  }

  static const uint mod5[] = {0,1,2,0,1};

  static bool IntersectPrimitiveRay (const KDTreePrimitive &primitive, const Ray &ray,
    HitPoint &hit)
  {
    const uint k = primitive.normal_K & ~KDPRIM_FLAG_MASK;
    const uint ku = mod5[k+1];
    const uint kv = mod5[k+2];

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

  template<bool exitFirstHit, class IgnoreTest>
  bool Raytracer::IntersectPrimitives (const KDTreeNode* node, const Ray &ray, 
    HitPoint &hit, IgnoreTest& ignoreTest)
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

      if (ignoreTest.Ignore (prim->primPointer)
        || mailboxes.PutPrimitiveRay (prim->primPointer, ray.rayID))
        continue;

      if (ray.ignoreFlags & (prim->normal_K & KDPRIM_FLAG_MASK))
        continue;

      haveHit = IntersectPrimitiveRay (*prim, ray, thisHit);
      if (haveHit)
      {
        ignoreTest.RegisterHit (prim->primPointer);
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

