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

#include "common.h"

#include "raytracer.h"
#include "kdtree.h"
#include "primitive.h"

namespace lighter
{
  RaytraceCore globalRaycore;

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

  RaytraceState::RaytraceState ()
    : traversalStackPtr (0)
  {
    traversalStack = static_cast<KDTraversalNode*> (CS::Memory::AlignedMalloc (
      sizeof (KDTraversalNode) * MAX_STACK_DEPTH, 32));
  }

  RaytraceState::~RaytraceState ()
  {
    CS::Memory::AlignedFree (traversalStack);
  }

  static bool IntersectPrimitiveRay (const KDTreePrimitive &primitive, const Ray &ray,
    HitPoint &hit)
  {
    const uint k = primitive.normal_K & ~KDPRIM_FLAG_MASK;
    const uint ku = CS::Math::NextModulo3(k);  // ku = (k+1) % 3
    const uint kv = CS::Math::NextModulo3(ku); // kv = (k+2) % 3

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
    hit.kdFlags = primitive.normal_K & KDPRIM_FLAG_MASK;

    return true;
  }

  enum IntersectState
  {
    isctNoHit,
    isctHit,
    isctHitAndExit
  };

  template<bool ExitFirstHit, typename HitCallback, typename HitIgnore>
  static IntersectState IntersectPrimitives (const KDTreeNode* node, const Ray &ray, 
    HitPoint &hit, RaytraceState& state, HitCallback& hitCB, HitIgnore& ignoreCB)
  {
    size_t nIdx, nMax;
    nMax = KDTreeNode_Op::GetPrimitiveListSize (node);
    bool haveHit = false;
    bool haveAnyHit = false;

    HitPoint thisHit;

    KDTreePrimitive* primList = KDTreeNode_Op::GetPrimitiveList (node);

    for (nIdx = 0; nIdx < nMax; nIdx++)
    {
      KDTreePrimitive* prim = primList + nIdx;

      if (ray.ignoreFlags & (prim->normal_K & KDPRIM_FLAG_MASK)) 
        continue; 

      if (ray.ignorePrimitive == prim->primPointer ||
        state.mailbox.PutPrimitiveRay (prim->primPointer, ray.rayID) ||
        ignoreCB (prim->primPointer))
          continue;

      if (ray.ignorePrimitive && 
        ray.ignorePrimitive->GetPlane () == prim->primPointer->GetPlane ())
        continue;

      if ((ray.ignoreObject != 0)
	  && prim->primPointer->GetObject() == ray.ignoreObject)
        continue;

      haveHit = IntersectPrimitiveRay (*prim, ray, thisHit);
      if (haveHit)
      {
        bool continueTrace = hitCB (ray, thisHit);

        haveAnyHit = true;
        if (ExitFirstHit) 
        {
          hit = thisHit;
          return isctHit;
        }
        if (thisHit.distance < hit.distance) 
          hit = thisHit;
	if (!continueTrace) return isctHitAndExit;
      }
    }

    return haveAnyHit ? isctHit : isctNoHit;
  }

  // Generic traversal function

  template<bool ExitFirstHit, typename HitCallback, typename HitIgnore>
  static bool TraceFunction (const KDTree* tree, const Ray& ray, HitPoint& hit, 
    HitCallback& hitCB, HitIgnore& ignoreCB)
  {
    if (!tree || !tree->nodeList) 
      return false;

    RaytraceProfiler prof (1);

    //Copy and clip the ray
    Ray myRay = ray;
    if (!myRay.Clip (tree->boundingBox))
      return false;

    // Get a state object
    RaytraceState& state = globalRaycore.GetRaytraceState ();

    state.SetupState (myRay);
    float tmin (myRay.minLength), tmax (myRay.maxLength);

    KDTreeNode* node = tree->nodeList;

    enum 
    {
      HaveHit = 0x01,
      TraversalFinished = 0x02
    };

    size_t traversalState = 0;

    while (!(traversalState & TraversalFinished))
    {
      while (!KDTreeNode_Op::IsLeaf (node))
      {
        uint dim = KDTreeNode_Op::GetDimension (node);
        //float thit = (node->inner.splitLocation - ray.origin[dim]) / ray.direction[dim];
        const float thit = (node->inner.splitLocation - ray.origin[dim]) * state.invD[dim];

        const csVector3 hitPoint = myRay.origin + myRay.direction * thit;

        KDTreeNode *nearNode, *farNode, *leftNode;
        leftNode = KDTreeNode_Op::GetLeft (node);

        state.GetNewNodes (leftNode, dim, nearNode, farNode);

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
          state.PutNode (farNode, thit, tmax);

          node = nearNode;
          tmax = thit;
        }
      }

      IntersectState isct = 
        IntersectPrimitives<ExitFirstHit, HitCallback, HitIgnore> (node,
          myRay, hit, state, hitCB, ignoreCB);
      if (isct != isctNoHit)
      {
        traversalState |= HaveHit;

        if (ExitFirstHit || (isct == isctHitAndExit))
          traversalState |= TraversalFinished;
      }

      if (!state.GetNode (node, tmin, tmax))
        traversalState |= TraversalFinished;
    }

    return traversalState & HaveHit;
  }

  // Hit callback functors
  struct HitCallbackNone 
  {
    CS_FORCEINLINE bool operator () (const Ray& ray, const HitPoint& hit)
    { return true; }
  };

  struct HitCallbackObj
  {
    HitCallbackObj (HitPointCallback* hitCallback)
      : cb (hitCallback)
    {}

    CS_FORCEINLINE bool operator () (const Ray& ray, const HitPoint& hit)
    {
      return cb->RegisterHit (ray, hit);
    }

    HitPointCallback* cb;
  };

  // Hit ignore functors
  struct IgnoreCallbackNone
  {
    CS_FORCEINLINE bool operator () (const Primitive* prim)
    {
      return false;
    }
  };

  struct IgnoreCallbackObj
  {
    IgnoreCallbackObj (HitIgnoreCallback* cb)
      : cb (cb)
    {}

    CS_FORCEINLINE bool operator () (const Primitive* prim)
    {
      return cb->IgnoreHit (prim);
    }

    HitIgnoreCallback* cb;
  };


  //-- Primary ray trace functions

  bool Raytracer::TraceAnyHit (const KDTree* tree, const Ray &ray, 
    HitPoint& hit, HitIgnoreCallback* ignoreCB)
  {
    HitCallbackNone hitCB;
    if (ignoreCB)
    {
      IgnoreCallbackObj ignCB (ignoreCB);
      return TraceFunction<true> (tree, ray, hit, hitCB, ignCB);
    }
    else
    {
      IgnoreCallbackNone ignCB;
      return TraceFunction<true> (tree, ray, hit, hitCB, ignCB);
    }
  }
    
  bool Raytracer::TraceAnyHit (const KDTree* tree, const Ray &ray, 
    HitPointCallback* hitCallback, HitIgnoreCallback* ignoreCB)
  {
    HitCallbackObj hitCB (hitCallback);
    HitPoint hit;
    if (ignoreCB)
    {
      IgnoreCallbackObj ignCB (ignoreCB);
      return TraceFunction<true> (tree, ray, hit, hitCB, ignCB);
    }
    else
    {
      IgnoreCallbackNone ignCB;
      return TraceFunction<true> (tree, ray, hit, hitCB, ignCB);
    }
  }

  bool Raytracer::TraceClosestHit (const KDTree* tree, const Ray &ray, 
    HitPoint &hit, HitIgnoreCallback* ignoreCB) 
  {
    HitCallbackNone hitCB;
    if (ignoreCB)
    {
      IgnoreCallbackObj ignCB (ignoreCB);
      return TraceFunction<false> (tree, ray, hit, hitCB, ignCB);
    }
    else
    {
      IgnoreCallbackNone ignCB;
      return TraceFunction<false> (tree, ray, hit, hitCB, ignCB);
    }
  }

  bool Raytracer::TraceAllHits (const KDTree* tree, const Ray &ray, 
    HitPointCallback* hitCallback, HitIgnoreCallback* ignoreCB)
  {
    HitCallbackObj hitCB (hitCallback);
    HitPoint hit;
    if (ignoreCB)
    {
      IgnoreCallbackObj ignCB (ignoreCB);
      return TraceFunction<false> (tree, ray, hit, hitCB, ignCB);
    }
    else
    {
      IgnoreCallbackNone ignCB;
      return TraceFunction<false> (tree, ray, hit, hitCB, ignCB);
    }
  }
}

