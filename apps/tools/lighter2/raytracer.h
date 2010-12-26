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
//#include "primitive.h"
#include "kdtree.h"

namespace lighter
{
  class KDTree;
  struct KDTreePrimitive;
  class Primitive;
  class PrimitiveBase;

  enum RayType {
    RAY_TYPE_EYE,     /// Originating from the camera/eye
    RAY_TYPE_REFLECT, //< Originating from a reflected ray
    RAY_TYPE_REFRACT, //< Originating from a refracted ray
    RAY_TYPE_SHADOW,  //< Used for occlusion testing in shadow calculation

    /// User defined ray types
    RAY_TYPE_OTHER1,
    RAY_TYPE_OTHER2,
    RAY_TYPE_OTHER3,
    RAY_TYPE_OTHER4,
    RAY_TYPE_OTHER5,

    /// Do not count this ray
    RAY_TYPE_IGNORE
  };

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

    // Also ignore all primitives from this object
    const Object* ignoreObject;

    RayType type;

    Ray () 
      : origin (0,0,0), direction (1,0,0), minLength (0), maxLength (FLT_MAX*0.9f),
      rayID (0), ignoreFlags (0), ignorePrimitive (0), ignoreObject (0), type(RAY_TYPE_EYE)
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
    // Distance along ray (t)
    float distance;

    // Hitpoint (world coordinates)
    csVector3 hitPoint;

    // The primitive we hit
    Primitive *primitive;
    
    // KD tree primitive flags
    int32 kdFlags;

    HitPoint ()
      : distance (0), hitPoint (0,0,0), primitive (0)
    {}

    bool operator< (const HitPoint& o) const
    {
      return distance < o.distance;
    }
  };

  struct HitPointCallback
  {
    virtual ~HitPointCallback () {}

    /// Returns whether to continue tracing.
    virtual bool RegisterHit (const Ray &ray, const HitPoint &hit) = 0;
  };

  struct HitIgnoreCallback
  {
    virtual ~HitIgnoreCallback () {}

    virtual bool IgnoreHit (const Primitive* prim) = 0;
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

  struct RaytraceState
  {
    RaytraceState ();
    ~RaytraceState ();

    // Setup some state data
    CS_FORCEINLINE void SetupState (Ray& ray)
    {
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

      ray.rayID = mailbox.GetRayID ();
    }

    // Stack handling
    CS_FORCEINLINE void PutNode (KDTreeNode* node, float tnear, float tfar)
    {
      traversalStack[traversalStackPtr].node = node;
      traversalStack[traversalStackPtr].tnear = tnear;
      traversalStack[traversalStackPtr].tfar = tfar;
      traversalStackPtr++;
      CS_ASSERT(traversalStackPtr < MAX_STACK_DEPTH);
    }

    CS_FORCEINLINE bool GetNode (KDTreeNode*& node, float& tnear, float& tfar)
    {
      if (traversalStackPtr == 0)
        return false;

      traversalStackPtr--;
      node = traversalStack[traversalStackPtr].node;
      tnear = traversalStack[traversalStackPtr].tnear;
      tfar = traversalStack[traversalStackPtr].tfar;
      return true;
    }

    CS_FORCEINLINE void GetNewNodes (KDTreeNode* left, uint dim, 
      KDTreeNode*& nearNode, KDTreeNode*& farNode)
    {
      nearNode = left + nodeOffset[dim][0];
      farNode = left + nodeOffset[dim][1];
    }

    enum
    {
      MAX_STACK_DEPTH = 64
    };

    // Data
    MailboxHash mailbox;

    // Helperstruct for kd traversal
    struct KDTraversalNode
    {
      KDTreeNode *node;
#if (CS_PROCESSOR_SIZE == 32)
      void* pad;
#endif
      float tnear, tfar;
    };
    size_t traversalStackPtr;
    KDTraversalNode* traversalStack;

    size_t nodeOffset[3][2];
    csVector3 invD;
  };

  class RaytraceCore
  {
  public:
    RaytraceState& GetRaytraceState ()
    {
      return rayState;
    }

  private:
    // For now, only one, later we want per-thread state
    CS::Threading::ThreadLocal<RaytraceState> rayState;
  };
  extern RaytraceCore globalRaycore;

  class Raytracer
  {
  public:
    //@{
    /**
     * Raytrace until there is any hit.
     * This might not be the closest hit so it is faster but not suitable
     * for all kind of calculations.
     */
    static bool TraceAnyHit (const KDTree* tree, const Ray &ray, HitPoint &hit, 
      HitIgnoreCallback* ignoreCB = 0);
    static bool TraceAnyHit (const KDTree* tree, const Ray &ray, 
      HitPointCallback* hitCallback, HitIgnoreCallback* ignoreCB = 0);
    //@}

    /**
     * Raytrace for closest hit. 
     */
    static bool TraceClosestHit (const KDTree* tree, const Ray &ray, 
      HitPoint &hit, HitIgnoreCallback* ignoreCB = 0);    

    /**
     * Raytrace for all hits along a ray.
     */
    static bool TraceAllHits (const KDTree* tree, const Ray &ray, 
      HitPointCallback* hitCallback, HitIgnoreCallback* ignoreCB = 0);
  };

  class RaytraceProfiler
  {
  public:
    RaytraceProfiler (uint numRays, RayType type = RAY_TYPE_EYE)
    {
      if(type == RAY_TYPE_IGNORE) return;

      globalStats.raytracer.numRays += numRays;

      switch(type)
      {
        case RAY_TYPE_REFLECT: globalStats.raytracer.numReflectionRays += numRays; break;
        case RAY_TYPE_REFRACT: globalStats.raytracer.numRefractionRays += numRays; break;
        case RAY_TYPE_SHADOW: globalStats.raytracer.numShadowRays += numRays; break;

        // Types specific to photon mapping
        case RAY_TYPE_OTHER1: globalStats.raytracer.numLightRays += numRays; break;
        case RAY_TYPE_OTHER2: globalStats.raytracer.numFinalGatherRays += numRays; break;

        default:
        case RAY_TYPE_EYE: globalStats.raytracer.numEyeRays += numRays; break;
      }
    }
  };
}

#endif
