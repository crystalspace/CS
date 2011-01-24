/*
    Copyright (C) 2005-2006 by Jorrit Tyberghein
	      (C) 2011 by Frank Richter

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

#ifndef __CS_MESHGEN_POSITIONMAP_H__
#define __CS_MESHGEN_POSITIONMAP_H__

#include "csgeom/box.h"
#include "csgeom/vector4.h"
#include "csutil/array.h"
#include "csutil/randomgen.h"

CS_PLUGIN_NAMESPACE_BEGIN(Engine)
{
  /**
   * A map of available positions.
   */
  class PositionMap
  {
  public:
    PositionMap(const float* minRadii, size_t numMinRadii,
		const csBox2& box);

    /**
     * Get a random available position. 
     * \param xpos X position.
     * \param zpos Z position.
     * \param radius The radius from the (x, z) coordinates to mark off as used.
     * \param minRadius The minimum radius used by all geometries.
     */
    bool GetRandomPosition (const float radius, float& xpos, float& zpos);

  private:
    csRandomGen posGen;
    
    /**
     * A bucket of free areas.
     * Each bucket contains all areas with their smallest side larger or equal
     * to \c minSide, but smaller than the next larger bucket.
     */
    struct Bucket
    {
      float minSide;
      csArray<csBox2> freeAreas;
      
      Bucket (float minSide) : minSide (minSide) {}
      
      bool operator< (const Bucket& other) const
      { return minSide > other.minSide; }
    };
    csArray<Bucket> buckets;
    // Insert an area into the right bucket (or discard if too small)
    void InsertNewArea (const csBox2& area);
  };
}
CS_PLUGIN_NAMESPACE_END(Engine)

#endif // __CS_MESHGEN_POSITIONMAP_H__
