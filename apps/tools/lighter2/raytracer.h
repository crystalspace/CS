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

namespace lighter
{
  class KDTree;
  class KDTreeNode;
  class RadPrimitive;

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

    Ray () 
      : origin (0,0,0), direction (1,0,0), minLength (0), maxLength (FLT_MAX*0.9f)
    {}

    // Clip to box. Returns if ray touch box at all
    bool Clip (const csBox3 &box)
    {
      const csVector3& minBox = box.Min ();
      const csVector3& maxBox = box.Max ();

      float mint = 0;
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
    RadPrimitive *primitive;

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

  class Raytracer
  {
  public:
    Raytracer (KDTree *tree)
      : tree (tree)
    {
    }

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
    bool IntersectPrimitives (const KDTreeNode* node, const Ray &ray, 
      HitPoint &hit, bool earlyExit = false);


    KDTree *tree;
  };
}

#endif
