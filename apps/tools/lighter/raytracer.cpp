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

#include "cssysdef.h"
#include "raytracer.h"
#include "kdtree.h"
#include "lightmesh.h"

// Helperstruct for kd traversal
struct kdTraversalS
{
  csKDTreeNode *node;
  float tnear, tfar;
  
  kdTraversalS ()
  {
  }

  kdTraversalS (csKDTreeNode *node, float tn, float tf)
    : node (node), tnear (tn), tfar (tf)
  {
  }
};

bool csRaytracer::TraceAnyHit (const csRay &ray, csHitPoint &hit)
{
  // Must have a tree
  if (!tree) return false;

  // Clip the ray
  float mint = 0.0f, maxt = FLT_MAX*0.9f;
  if (!ray.Clip (tree->boundingBox, mint, maxt))
    return false;

  // Setup for traversal
  csArray<kdTraversalS> kdstack;
  kdstack.SetSize (64);
  float t;
  csHitPoint hitpoint;
  hitpoint.distance = maxt;

  csKDTreeNode *node = tree->rootNode;

  while (1)
  {
    while (csKDTreeNodeH::GetFlag (node))
    {
      //traverse until we hit a leafnode
      uint splitDim = csKDTreeNodeH::GetDimension (node);
      t = (node->splitLocation - ray.origin[splitDim]) / ray.direction[splitDim];

      if (t <= mint)
      {
        // t <= mint <= maxt -> cull left and traverse right
        node = ((csKDTreeNode*)csKDTreeNodeH::GetPointer (node))+1;
      }
      else if (t >= maxt)
      {
        // mint <= maxt <= t -> cull right and traverse left
        node = ((csKDTreeNode*)csKDTreeNodeH::GetPointer (node));
      }
      else
      {
        // mint < t < maxt -> traverse both left and right
        kdstack.Push (kdTraversalS(((csKDTreeNode*)csKDTreeNodeH::GetPointer (node))+1, t, maxt));
        node = ((csKDTreeNode*)csKDTreeNodeH::GetPointer (node));
        maxt = t;
      }
    }
    // Ok, we have a leaf, use it
    if (IntersectTriangles (node, ray, hitpoint, true))
      return true;

    if (kdstack.Length () == 0)
    {
      //stack empty, no hits return
      return false;
    }

    kdTraversalS s = kdstack.Pop ();
    node = s.node;
    mint = s.tnear;
    maxt = s.tfar;
  }

  return false;
}

bool csRaytracer::TraceClosestHit (const csRay &ray, csHitPoint &hit)
{
  // Must have a tree
  if (!tree) return false;

  // Clip the ray
  float mint = 0.0f, maxt = FLT_MAX*0.9f;
  if (!ray.Clip (tree->boundingBox, mint, maxt))
    return false;

  // Setup for traversal
  csArray<kdTraversalS> kdstack;
  kdstack.SetSize (64);
  float t;
  csHitPoint hitpoint;
  hitpoint.distance = maxt;

  csKDTreeNode *node = tree->rootNode;

  while (1)
  {
    while (csKDTreeNodeH::GetFlag (node))
    {
      //traverse until we hit a leafnode
      uint splitDim = csKDTreeNodeH::GetDimension (node);
      t = (node->splitLocation - ray.origin[splitDim]) / ray.direction[splitDim];

      if (t <= mint)
      {
        // t <= mint <= maxt -> cull left and traverse right
        node = ((csKDTreeNode*)csKDTreeNodeH::GetPointer (node))+1;
      }
      else if (t >= maxt)
      {
        // mint <= maxt <= t -> cull right and traverse left
        node = ((csKDTreeNode*)csKDTreeNodeH::GetPointer (node));
      }
      else
      {
        // mint < t < maxt -> traverse both left and right
        kdstack.Push (kdTraversalS(((csKDTreeNode*)csKDTreeNodeH::GetPointer (node))+1, t, maxt));
        node = ((csKDTreeNode*)csKDTreeNodeH::GetPointer (node));
        maxt = t;
      }
    }
    // Ok, we have a leaf, use it
    IntersectTriangles (node, ray, hitpoint);

    // Early termination
    if (maxt <= hitpoint.distance)
      return true;

    if (kdstack.Length () == 0)
    {
      //stack empty, no hits return
      return false;
    }

    kdTraversalS s = kdstack.Pop ();
    node = s.node;
    mint = s.tnear;
    maxt = s.tfar;
  }

  return false;
}


// Intersection routines
//@@TODO: Make this aligned
static const uint mod5[] = {0,1,2,0,1};

bool IntersectRayTriangle (const csMeshPatchAccStruct &tri, const csRay &ray,
                           csHitPoint &hit)
{
  const uint k = tri.k;
  const uint ku = mod5[tri.k+1];
  const uint kv = mod5[tri.k+2];

  // prefetch?

  const float nd = 1.0f / (ray.direction[k] + 
    tri.normal_u * ray.direction[ku] + tri.normal_v * ray.direction[kv]);

  const float f = (tri.normal_d - ray.origin[k] - 
    tri.normal_u * ray.origin[ku] - tri.normal_v * ray.origin[kv]) * nd;

  // Check for distance..
  if (!(hit.distance > f && f > FLT_EPSILON)) return false;

  // Compute hitpoint on plane
  const float hu = (ray.origin[ku] + f * ray.direction[ku]);
  const float hv = (ray.origin[kv] + f * ray.direction[kv]);

  // First barycentric coordinate
  const float lambda = (hu * tri.b_nu + hv * tri.b_nv + tri.b_d);
  if (lambda < 0.0f) return false;

  // Second barycentric coordinate
  const float mu = (hu * tri.c_nu + hv * tri.c_nv + tri.c_d);
  if (mu < 0.0f) return false;

  // Third barycentric coordinate
  if (lambda + mu > 1.0f) return false;

  // Ok, is a hit, store it
  hit.distance = f;
  hit.tri = (csMeshPatchAccStruct*)&tri;
  hit.lambda = lambda;
  hit.mu = mu;

  return true;
}

bool csRaytracer::IntersectTriangles (const csKDTreeNode* node, const csRay &ray, 
                                      csHitPoint &hit, bool earlyExit)
{
  //assume valid input
  uint nIdx, nMax;
  nMax = node->numberOfPrimitives;
  bool haveHit = false;
  csMeshPatchAccStruct **primList = (csMeshPatchAccStruct**)csKDTreeNodeH::GetPointer (node);

  for (nIdx = 0; nIdx < nMax; nIdx++)
  {
    haveHit = IntersectRayTriangle (*primList[nIdx], ray, hit);
    if (haveHit && earlyExit) return true;
  }

  return haveHit;
}
