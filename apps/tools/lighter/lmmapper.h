/*
    Copyright (C) 2004 by Jorrit Tyberghein
                  2005 by Marten Svanfeldt

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

#ifndef __LMMAPPER_H__
#define __LMMAPPER_H__

#include <stdarg.h>
#include <csutil/array.h>
#include <csgeom/vector2.h>
#include <csgeom/vector3.h>
#include <csgeom/plane3.h>

#define LIT_AXIS_X 0
#define LIT_AXIS_Y 1
#define LIT_AXIS_Z 2

/**
 * This class controls the mapping from polygon 3D coordinates
 * to lightmap coordinates and vice versa.
 */
class litLightMapMapper
{
private:
  // Axis that we selected to map this lightmap. We will always try
  // to find the axis from which side you can view most of the lightmap.
  // One of LIT_AXIS_...
  int axis;

  // Density of this mapping.
  float density;

  // Offset to get to the correct mapping.
  csVector2 offset;

public:
  litLightMapMapper ();

  /**
   * Initialize this mapper for a given polygon.
   * \param poly is the polygon for which we will calculate the mapping.
   * \param plane is the plane of the polygon. The normal of this plane
   *        must be normalized.
   * \param density is the number of lightmap units in a world unit.
   */
  void InitializeMapping (const csArray<csVector3>& poly,
  	const csPlane3& plane, float density);

  /**
   * Transform a point that must be on the polygon plane to a
   * lightmap coordinate.
   */
  void PolyToLM (const csVector3& vpoly, csVector2& vlm);

  /**
   * Transform a lightmap coordinate to a point on the polygon.
   */
  void LMToPoly (const csVector2& vlm, csVector3& vpoly);
};

#endif // __LMMAPPER_H__

