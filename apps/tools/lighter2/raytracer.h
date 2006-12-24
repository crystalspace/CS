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

#ifndef __RAYTRACER_H__
#define __RAYTRACER_H__

#include "statistics.h"
#include "primitive.h"

namespace lighter
{
  class KDTree;
  union KDTreeNode;
  struct KDTreePrimitive;
  class Primitive;

  /**
   * A ray in space.
   * The ray is parameterized as O+t*D, t=[mindist, maxdist)
   */
  struct Ray
  {
    // Origin (O)
    csVector3 origin;

    // Direction (normalized)
    csVector3 direction;

    // Min/max parameter
    float minLength, maxLength;

    // Unique id
    size_t rayID;

    // Flags for primitives to ignore
    uint32 ignoreFlags;

    // Specific primitive to ignore
    const Primitive* ignorePrimitive;

    Ray () 
      : origin (0,0,0), direction (1,0,0), minLength (0), maxLength (FLT_MAX*0.9f),
      rayID (0), ignoreFlags (0), ignorePrimitive (0)
    {}

    // Clip to box. Returns if ray touch box at all
    bool Clip (const csBox3 &box)
    {
      const csVector3& minBox = box.Min ();
      const csVector3& maxBox = box.Max ();

      float mint = minLength;
      float maxt = maxLength;

      // Check if ray have any chance of going through box
      int i = 0;
      for (i = 0; i < 3; i++)
      {
        if (direction[i] < 0)
        {
          if (origin[i] < minBox[i]) return false;
        }
        else if (direction[i] > 0)
        {
          if (origin[i] > maxBox[i]) return false;
        }
      }

      // Clip one dimension at a time
      for (i = 0; i < 3; i++)
      {
        float pos = origin[i] + direction[i] * maxt;

        // Ray going "left"
        if (direction[i] < 0)
        {
          //Clip end
          if (pos < minBox[i])
          {
            maxt = mint + (maxt - mint) * ((origin[i] - minBox[i]) / (origin[i] - pos));
          }
          //Clip start
          if (origin[i] > maxBox[i])
          {
            mint += (maxt - mint) * ((origin[i] - maxBox[i]) / (maxt * direction[i]));
          }
        }
        else // Ray going straight or "right"
        {
          //Clip end
          if (pos > maxBox[i])
          {
            maxt = mint + (maxt - mint) * ((maxBox[i] - origin[i]) / (pos - origin[i]));
          }
          //Clip start
          if (origin[i] < minBox[i])
          {
            mint += (maxt - mint) * ((minBox[i] - origin[i]) / (maxt * direction[i]));
          }
        }
        if (mint > maxt) return false;
      }

      minLength = mint;
      maxLength = maxt;

      return true;
    }
  };


  // Hitpoint returned by the raytracer
  struct HitPoint
  {
    // The primitive we hit
    Primitive *primitive;

    // Distance along ray (t)
    float distance;

    // Hitpoint (world coordinates)
    csVector3 hitPoint;

    HitPoint ()
      : primitive (0), distance (0), hitPoint (0,0,0)
    {}

    bool operator< (const HitPoint& o)
    {
      return distance < o.distance;
    }
  };

  class MailboxHash
  {
  public:
    MailboxHash ();
    ~MailboxHash ();

    CS_FORCEINLINE size_t GetRayID ()
    {
      return rayID++;
    }

    CS_FORCEINLINE bool PutPrimitiveRay (const Primitive* prim, size_t ray)
    {
      size_t hashPos = (reinterpret_cast<uintptr_t> (prim) >> 3) & (HASH_SIZE-1);
      
      HashEntry& e = hash[hashPos];
      
      //Check if we already are in the hash
      if (e.primPointer == prim &&
        e.rayID == ray)
        return true;

      //Insert new entry
      e.primPointer = prim;
      e.rayID = ray;
      return false;
    }

  private:
    size_t rayID;

    enum
    {
      HASH_SIZE = 64
    };

    struct HashEntry
    {
      const Primitive* primPointer;
      size_t rayID;
    };
    HashEntry* hash;
  };


  class Raytracer
  {
  public:
    Raytracer (KDTree *tree);

    ~Raytracer ();

    /**
     * Raytrace until there is any hit.
     * This might not be the closest hit so it is faster but not suitable
     * for all kind of calculations.
     */
    bool TraceAnyHit (const Ray &ray, HitPoint &hit);

    /**
     * Raytrace for closest hit. 
     */
    bool TraceClosestHit (const Ray &ray, HitPoint &hit);    

  protected:
    /// Traverse all primitives in a given node and do intersection against them
    template<bool exitFirstHit>
    bool IntersectPrimitives (const KDTreeNode* node, const Ray &ray, 
      HitPoint &hit);

    KDTree *tree;


    enum
    {
      MAX_STACK_DEPTH = 64
    };

    // Helperstruct for kd traversal
    struct kdTraversalS
    {
      KDTreeNode *node;
#if (CS_PROCESSOR_SIZE == 32)
      void* pad;
#endif
      float tnear, tfar;
    };
    size_t traversalStackPtr;
    kdTraversalS* traversalStack;

    MailboxHash mailboxes;
  };

  /// Helper to profile raytracing
  class RaytraceProfiler
  {
  public:

    RaytraceProfiler (uint numRays)
    {
      startTime = csGetMicroTicks ();
      globalStats.raytracer.numRays += numRays;
    }

    ~RaytraceProfiler ()
    {
      int64 endTime = csGetMicroTicks ();
      int64 diffTime = endTime-startTime;
      if(diffTime > 0)
      {
        globalStats.raytracer.usRaytracing += diffTime;
      }
    }

  private:
    int64 startTime;
  };

  /// Helper for often-used ray tracing operations
  class RaytraceFunctions
  {
  public:
    // Vistest rayhelpers

    static inline float Vistest1 (Raytracer& rt, const csVector3& viscenter,
      const csVector3& endp)
    {
      // Perform a 1 point vistest

      const csVector3& primP = viscenter;
      const csVector3 dir = primP - endp;

      Ray ray;
      HitPoint hit;
      ray.minLength = 0;
      ray.maxLength = dir.Norm ();
      ray.origin = endp;
      ray.direction = dir / ray.maxLength;
      ray.maxLength -= 0.001f;
      return rt.TraceAnyHit (ray, hit) ? 0.0f : 1.0f;
    }

    static inline float Vistest5 (Raytracer& rt, const csVector3& viscenter,
      const csVector3& visu, const csVector3& visv, const csVector3& endp,
      const Primitive& prim)
    {
      // Perform a 5 point vistest

      const csVector3 halfu = visu * 0.5f;
      const csVector3 halfv = visv * 0.5f;

      const csVector3 rayStarts[] = {
        viscenter, 
        viscenter + halfu + halfv,
        viscenter + halfu - halfv,
        viscenter - halfu + halfv,
        viscenter - halfu - halfv
      };

      uint insideNum = 0;
      uint hitNum = 0;
      for (unsigned int i = 0; i < 5; i++)
      {
        const csVector3& primP = rayStarts[i];
        const csVector3 dir = primP - endp;

        Ray ray;
        HitPoint hit;
        ray.minLength = 0;
        ray.maxLength = dir.Norm ();
        ray.origin = endp;
        ray.direction = dir / ray.maxLength;
        ray.maxLength -= 0.001f;
        // FIXME: Only do for actual "border lumels"
        bool inside = prim.PointInside (primP);
        bool hitAny = rt.TraceAnyHit (ray, hit);
        if (inside)
        {
          insideNum++;
          if (hitAny) hitNum++;
        }
      }
      if (insideNum == 0) insideNum = 5;
      return 1.0f-((float)hitNum/(float)insideNum);
    }
  };
}

#endif
