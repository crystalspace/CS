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

#include "csgeom/box.h"
#include "csgeom/vector3.h"

struct csMeshPatchAccStruct;
class csKDTree;
union csKDTreeNode;

/**
 * A ray.
 * The ray is parameterized as O+t*D, t=[0, maxdist) with D being normalized
 * direction.
 */
struct csRay
{
  /// Origin
  csVector3 origin;

  /// Direction (normalized)
  csVector3 direction;

  /// Maxlength
  float maxLength;

  csRay ()
    : origin (0,0,0), direction (1,0,0), maxLength (FLT_MAX*0.9f)
  {
  }

  /**
   * Clip the ray to given boundingbox and return min and max t
   * Returns false if ray is outside box.
   */
  bool Clip (const csBox3& box, float &mint, float &maxt) const
  {
    const csVector3& minBox = box.Min ();
    const csVector3& maxBox = box.Max ();

    mint = 0;
    maxt = maxLength;

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

    return true;
  }
};

/**
 * A hitpoint
 * A hitpoint between a ray and a triangle
 */
struct csHitPoint
{
  /// Triangle which is hit
  csMeshPatchAccStruct *tri;

  /// Distance along ray for hitpoint
  float distance;

  /// First barycentric coordinate for hit
  float lambda;

  /// Second barycentric coordinate for hit
  float mu;

  csHitPoint ()
    : tri (0), distance (0), lambda (0), mu (0)
  {}
};

/**
 * A triangle raytracer using KD-tree. 
 */
class csRaytracer
{
public:
  csRaytracer (csKDTree *tree)
    : tree (tree)
  {
  }

  /**
   * Raytrace until there is any hit.
   * This might not be the closest hit so it is faster but not suitable
   * for all kind of calculations.
   */
  bool TraceAnyHit (const csRay &ray, csHitPoint &hit);

  /**
   * Raytrace for closest hit. 
   */
  bool TraceClosestHit (const csRay &ray, csHitPoint &hit);

protected:
  /// Traverse all triangles in a given node and do intersection against them
  inline bool IntersectTriangles (const csKDTreeNode* node, const csRay &ray, 
    csHitPoint &hit, bool earlyExit = false);

  /// Associated KD-tree
  csKDTree *tree;
};

#endif
