/*
    Crystal Space Entity Layer
    Copyright (C) 2011 by Matthieu Kraus
  
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
    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

typedef struct
{
  float3 low;
  float3 high;
} bbox_t;

typedef struct
{
  uint flags;
  float3 normal;
  float3 a;
  float3 b;
} primitive_t;

typedef struct
{
  uint flags;
  float3 vertex[3];
} raw_primitive_t;

typedef struct
{
  uint flags;
  float2 split;
} node_t;

typedef struct
{
  float3 start;
  float3 end;
} ray_t;

typedef struct
{
  uint primitive;
  float2 coordinate;
  float dist;
} hit_t;

typedef struct
{
  float3 location;
  uint3 primitives;
} primitive_sort_t;

// bitonic sort configuration
#define element_t primitive_sort_t
// sorts descending
#define CMPXCHG(a,b)\
  {\
    element_t left = a;\
    element_t right = b;\
    int3 diff = inc ^ (signbit(left.location-right.location) & 1);\
    a.location   = select(left.location   , right.location  , diff);\
    a.primitives = select(left.primitives , right.primitives, diff);\
    b.location   = select(right.location  , left.location   , diff);\
    b.primitives = select(right.primitives, left.primitives , diff);\
  }

// enums
enum primitive_flag_t
{
  PRIM_AXIS = 3
};

enum flag_t
{
  NODE_AXIS = 3,
  NODE_INVERTED = 4,
  NODE_PRIMITIVE = 8
};

enum traceMode_t
{
  TRACE_ANY,
  TRACE_CLOSEST,
  TRACE_ALL
};

// yay for language design failures -.-
#define SHUFFLE(a,b) shuffle(a,(uint2)(b,b)).s0

hit_t IntersectPrimitive(const ray_t* ray, __global primitive_t* primitive)
{
  hit_t hit;

  // get shuffle mask
  uint3 axes;
  axes.x = primitive->flags & PRIM_AXIS;
  axes.y = (1 << axes.x) & 3;
  axes.z = (1 << axes.y) & 3;

  // get shuffled ray data
  float3 direction = shuffle(normalize(ray->end - ray->start),axes);
  float3 origin = shuffle(ray->start, axes);

  // calculate distance to plane
  float3 normalFactor = 1;
  normalFactor.yz = primitive->normal.xy;
  hit.dist = (primitive->normal.z - dot(normalFactor, origin))
             / dot(normalFactor, direction);

  // calculate barycentric coordinates
  float3 hitFactor = 1;
  hitFactor.xy = origin.yz + hit.dist*direction.yz;
  hit.coordinate.x = dot(hitFactor, primitive->a);
  hit.coordinate.y = dot(hitFactor, primitive->b);

  // check whether the hit was inside the primitive
  if(any(hit.coordinate < 0) || (hit.coordinate.x + hit.coordinate.y) > 1)
  {
    // hit outside primitive
    hit.dist = NAN;
  }

  return hit;
}

uint ClipRay(ray_t* ray, uint axis, float split, uint clipFar)
{
  // {start, end}
  float2 segment = shuffle2(ray->start, ray->end, (uint2)(axis,axis+3));

  // check whether the ray intersects the plane
  int2 classify = signbit(segment-split);
  if(!(classify.x ^ classify.y))
  {
    // ray doesn't intersect split axis
    return 0;
  }

  // calculate distance to split plane in ray lengths
  float factor = distance(segment.x, split) / distance(segment.x, segment.y);

  float3 newPoint = mix(ray->start, ray->end, factor);
  if(clipFar)
  {
    ray->end = newPoint;
  }
  else
  {
    ray->start = newPoint;
  }

  return 1;
}

void RayTrace(const bbox_t treeBox, __constant uint* levelOffsets, __global node_t* treeData,
              __global primitive_t* primitives, ray_t ray, const uint traceMode, __global hit_t* hits)
{
  // keep track of the hits we got
  uint hitCount = 0;

  // current index of the node we're working on
  uint index = 0;

  // current depth-level in the tree
  uint level = 0;

  // this flag will mark to which level we shall return later
  // (i.e. where to jump to)
  uint returnFlag = 0;
  uint returnHit = 0;

  // these flags keep track of where the last clipping of the ray
  // occured
  uint2 clipFlags = 0;

  // pre-calculate direction of the ray in the various axes
  int3 direction = select(1, 0, signbit(ray.end - ray.start));

  // clip ray to tree bbox
  if(any(signbit(select(ray.start - treeBox.low, treeBox.high - ray.start, direction))))
  {
    // ray lies completely outside the tree
    return;
  }

  // clip ray to bbox
  ClipRay(&ray, 0, treeBox.low.x, 0);
  ClipRay(&ray, 1, treeBox.low.y, 0);
  ClipRay(&ray, 2, treeBox.low.z, 0);
  ClipRay(&ray, 0, treeBox.high.x, 1);
  ClipRay(&ray, 1, treeBox.high.y, 1);
  ClipRay(&ray, 2, treeBox.high.z, 1);

  // keep a backup of our ray
  ray_t origRay = ray;

  node_t node = treeData[0]; // start at root node

  while(1)
  {
    int2 traverse = 0;
    uint axis = node.flags & NODE_AXIS;
    uint dir = SHUFFLE(direction, axis);
    uint leftNear = dir ^ select(0, 1, node.flags & NODE_INVERTED);

    if(node.flags & NODE_PRIMITIVE)
    {
      // we reached the primitive level obtain a pointer to the primitive
      uint primitiveId = as_uint(node.split.x);
      __global primitive_t* p = primitives + primitiveId;

      // perform a ray-primitive intersection
      hit_t hit = IntersectPrimitive(&origRay, p);
      hit.primitive = primitiveId;

      if(isnormal(hit.dist))
      {
        if(traceMode == TRACE_CLOSEST && hitCount)
        {
          if(hits[0].dist > hit.dist)
          {
            hits[0] = hit;
          }
        }
        else
        {
          hits[hitCount++] = hit;
        }

        if(traceMode == TRACE_ANY || !returnHit)
        {
          // we only trace one hit or are done
          break;
        }

        // find the depth-level on which the parent to continue with resides in
        const uint srcLevel = 8*sizeof(returnHit) - clz(returnHit);

        // locate the parent in this level and traverse to the other sibling
        index = (index << (srcLevel - level - 1)) ^ (1 << srcLevel);

        // locate the parent(s) at which clipping the near/far side of the ray occured
        uint2 clipLevels = 8*sizeof(clipFlags.x) - clz(clipFlags);

        // clear according parts of return and clip flags
        const uint clearMask = ~(~0 << (level - 1));
        clipFlags &= clearMask;
        returnFlag &= clearMask;
        returnHit &= clearMask;

        // restore the original ray
        ray = origRay;

        // clip near side of the ray
        uint clipIndex = (index << (clipLevels.x - level - 1)) ^ (1 << clipLevels.x);
        node = treeData[levelOffsets[clipLevels.x]+clipIndex];
        axis = node.flags & NODE_AXIS;
        dir = SHUFFLE(direction, axis);
        leftNear = dir ^ select(0, 1, node.flags & NODE_INVERTED);
        ClipRay(&ray, axis, SHUFFLE(node.split, leftNear), 0);

        // clip far side of the ray
        clipIndex = (index << (clipLevels.y - level - 1)) ^ (1 << clipLevels.y);
        node = treeData[levelOffsets[clipLevels.y]+clipIndex];
        axis = node.flags & NODE_AXIS;
        dir = SHUFFLE(direction, axis);
        leftNear = dir ^ select(0, 1, node.flags & NODE_INVERTED);
        ClipRay(&ray, axis, SHUFFLE(node.split, 1-leftNear), 1);

        // update the level now
        level = srcLevel;
      }
    }
    else
    {
      // dists = { ray.start[axis]-split[1-leftNear], ray.end[axis]-split[leftNear] }
      // @@note: for kd-tree the shuffle for node.split wouldn't be needed as there's only 1 split axis
      float2 dists = shuffle2(ray.start, ray.end, (uint2)(axis, axis+3))
                   - shuffle(node.split, (uint2)(1-leftNear, leftNear));

      // traverse = { near ray end < near node axis, far ray end > far node axis }
      traverse = select(0, 1, dists > 0) == (int2)(1-dir, dir);

      // if the ray goes through both childs, we may have to return to this
      // point if the path we chose didn't yield a hit
      // @@note: for kd-tree this flag would replace returnHit
      returnFlag |= all(traverse) << level;

      // if the ray goes through both childs and the childs overlap, we have
      // to return here even if we got a hit
      // @@note: for kd-tree this flag would be obsoleted by the regular return flag
      returnHit |= (all(traverse)
                  & (
                      ((node.flags & NODE_INVERTED) << 2)
                    ^ signbit(node.split.y - node.split.x)
                    )
                   ) << level;
    }

    if(any(traverse))
    {
      // one of the childs was hit, so traverse into the nearest hit one
      uint traverseNear = leftNear ^ traverse.x;
      index = (index << 1) | traverseNear;

      // clip ray here if required and update clipflags accordingly
      if(ClipRay(&ray, axis, SHUFFLE(node.split, traverseNear), traverseNear))
      {
        clipFlags |= (uint2)(0,1) ^ traverseNear;
      }
    }
    else if(returnFlag)
    {
      // find the depth-level on which the parent to continue with resides in
      const uint srcLevel = 8*sizeof(returnFlag) - clz(returnFlag);

      // locate the parent in this level and traverse to the other sibling
      index = (index << (srcLevel - level - 1)) ^ (1 << srcLevel);

      // locate the parent(s) at which clipping the near/far side of the ray occured
      uint2 clipLevels = 8*sizeof(clipFlags.x) - clz(clipFlags);

      // clear according parts of return and clip flags
      const uint clearMask = ~(~0 << (level - 1));
      clipFlags &= clearMask;
      returnFlag &= clearMask;
      returnHit &= clearMask;

      // restore the original ray
      ray = origRay;

      // clip near side of the ray
      uint clipIndex = (index << (clipLevels.x - level - 1)) ^ (1 << clipLevels.x);
      node = treeData[levelOffsets[clipLevels.x]+clipIndex];
      axis = node.flags & NODE_AXIS;
      dir = SHUFFLE(direction, axis);
      leftNear = dir ^ select(0, 1, node.flags & NODE_INVERTED);
      ClipRay(&ray, axis, SHUFFLE(node.split, leftNear), 0);

      // clip far side of the ray
      clipIndex = (index << (clipLevels.y - level - 1)) ^ (1 << clipLevels.y);
      node = treeData[levelOffsets[clipLevels.y]+clipIndex];
      axis = node.flags & NODE_AXIS;
      dir = SHUFFLE(direction, axis);
      leftNear = dir ^ select(0, 1, node.flags & NODE_INVERTED);
      ClipRay(&ray, axis, SHUFFLE(node.split, 1-leftNear), 1);

      // update the level now
      level = srcLevel;
    }
    else
    {
      // no hit an no place to return to
      break;
    }

    // traverse to child-level
    ++level;

    // obtain new node
    node = treeData[levelOffsets[level]+index];
  }
}

// this function creates the primitive indices used during tree building
// and converts the primitives from a vertex array to a format suitable
// for intersection testing
__kernel
void PrepareBuildTree(__global raw_primitive_t* rawPrimitives, __global primitive_t* primitives,
                      __global bbox_t* boxes, __global primitive_sort_t* indicesLow, __global primitive_sort_t* indicesHigh)
{
  const uint gId = get_global_id(0);
  raw_primitive_t rawPrimitive = rawPrimitives[gId];

  // calculate primitve bbox
  bbox_t box;
  box.low = min(min(rawPrimitive.vertex[0],
                    rawPrimitive.vertex[1]),
                    rawPrimitive.vertex[2]);
  box.high = max(max(rawPrimitive.vertex[0],
                     rawPrimitive.vertex[1]),
                     rawPrimitive.vertex[2]);
  boxes[gId] = box;

  // create sorting indices
  primitive_sort_t index;
  index.primitives = gId;

  index.location = box.low;
  indicesLow[gId] = index;

  index.location = box.high;
  indicesHigh[gId] = index;

  // calculate normalized node values
  primitive_t primitive;

  // calculate primitive base
  float3 baseA = rawPrimitive.vertex[2]-rawPrimitive.vertex[0];
  float3 baseB = rawPrimitive.vertex[1]-rawPrimitive.vertex[0];
  float3 normal = normalize(cross(baseA, baseB));

  // find dominant axis
  uint3 axes;
  axes.x = select(select(2, 1, normal.y > normal.z), 0,
                  normal.x > normal.y && normal.x > normal.z);
  axes.x = 0; // dominant axis
  axes.y = (1 << axes.x) & 3;
  axes.z = (1 << axes.y) & 3;

  // shuffle our base
  normal = shuffle(normal, axes);
  baseA = shuffle(baseA, axes);
  baseB = shuffle(baseB, axes);
  float3 base = shuffle(rawPrimitive.vertex[0],axes);

  // set dominant axis on primitive
  primitive.flags = axes.x;

  // calculate normal
  primitive.normal.xy = normal.yz;
  primitive.normal.z = dot(normal, base);
  primitive.normal = primitive.normal / normal.x;

  // calculate first edge
  primitive.a.x = -baseA.z;
  primitive.a.y = baseA.y;
  primitive.a.z = dot(primitive.a.xy,base.yz);

  // calculate second edge
  primitive.b.x = baseB.z;
  primitive.b.y = -baseB.y;
  primitive.b.z = dot(primitive.b.xy,base.yz);

  // scale edges
  float factor = -1.0f/dot(primitive.a.xy,baseB.yz);
  primitive.a = primitive.a * factor;
  primitive.b = primitive.b * factor;

  // output primitive
  primitives[gId] = primitive;
}

// the actual tree builder - creates the nodes for a given level
// using the sorted primitives for this level to find a good split
// @param usedAxis the axis used to split node n (used for sorting for the next step)
// @param primitivesLow primitives sorted component-wise by bbox.low in chunks of 2^(ldPrimitiveCount-level)
// @param primitivesHigh primitivies of primitivesLow sorted component-wise in reversed order in chunks of 2^(ldPrimitiveCount-level-1)
// @param level level to build
// @param primitiveCount number of primitives in the buffers
// @param ldPrimitiveCount floor(log2(primitiveCount))
// @@todo: take advantage of possibility to invert the axis meanings for non-pow2 nodes
__kernel
void BuildTree(__constant uint* levelOffsets, __global node_t* treeData,
               __global uint* usedAxis,
               __global primitive_sort_t* primitivesLow,
               __global primitive_sort_t* primitivesHigh,
               const uint level, const uint primitiveCount,
               const uint ldPrimitiveCount)
{
  const uint gId = get_global_id(0);
  const uint exponent = ldPrimitiveCount - level;
  const uint indexLow = gId << exponent;
  const uint indexHigh = min(((gId+1) << exponent) - 1, primitiveCount);
  const uint splitIndex = 1 << exponent;

  if(indexLow == indexHigh)
  {
    // leaf node
    uint primitive = primitivesLow[gId].primitives.x;

    node_t node;
    node.flags = NODE_PRIMITIVE;
    node.split.x = as_float(primitive);
    treeData[gId+levelOffsets[level]] = node;
    return;
  }

  // we'll evaluate 3 candidates that seem the most
  // natural choice for our splitting scheme (left-balanced tree)

  // normal candidates
  primitive_sort_t candidatesL = primitivesLow[indexLow+splitIndex];
  primitive_sort_t candidatesR = primitivesHigh[indexLow+splitIndex+1];

  // find the candidate that cuts off most surface area
  float3 cutoff = candidatesR.location-candidatesL.location;
  uint best = select(select(2, 1, cutoff.y > cutoff.z), 0,
                     cutoff.x > cutoff.y && cutoff.x > cutoff.z);

  // create node
  node_t node;
  node.flags = best;
  node.split = shuffle2(candidatesL.location,candidatesR.location,(uint2)(best,best+3));
  treeData[gId+levelOffsets[level]] = node;

  // set used axis so the primitives can be sorted for the next step accordingly
  usedAxis[gId] = best;
}

// function used during tree building
// has to be used once per level prior to sorting (except for the root level)
__kernel
void InitializeSort(__global primitive_sort_t* primitivesLow, __global primitive_sort_t* primitivesHigh,
                    __global uint* usedAxis, __global bbox_t* boxes,
                    const uint level, const uint ldPrimitiveCount)
{
  const uint gId = get_global_id(0);
  const uint index = gId >> (ldPrimitiveCount - level);
  const uint3 axes = usedAxis[index];

  // re-initialize low sorting primitive
  primitive_sort_t primitive = primitivesLow[index];
  primitive.primitives = shuffle(primitive.primitives,axes);
  primitive.location = boxes[primitive.primitives.x].low;
  primitivesLow[index] = primitive;

  // re-initialize high sorting primitive
  primitive = primitivesHigh[index];
  primitive.primitives = shuffle(primitive.primitives,axes);
  primitive.location = boxes[primitive.primitives.x].high;
  primitivesHigh[index] = primitive;
}

__kernel
void test(const bbox_t treeBox, __constant uint* levelOffsets, __global node_t* treeData,
          __global primitive_t* primitives, __global ray_t* rays, const uint traceMode,
          __global hit_t* hits)
{
  const uint gId = get_global_id(0);
  ray_t ray = rays[gId];
  RayTrace(treeBox, levelOffsets, treeData, primitives, ray, traceMode, hits+gId);
}
