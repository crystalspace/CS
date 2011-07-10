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
#include "csutil/tuple.h"

CS_PLUGIN_NAMESPACE_BEGIN(Engine)
{
  /**
   * A map of available positions.
   */
  class PositionMap
  {
  public:
    PositionMap(const float* minRadii, size_t numMinRadii,
		const csBox2& box, int randomSeed);
    ~PositionMap ();

    typedef csTuple2<size_t, size_t> AreaID;
    /**
     * Get a random available position. 
     * \param xpos X position.
     * \param zpos Z position.
     * \param radius The radius from the (x, z) coordinates to mark off as used.
     * \param minRadius The minimum radius used by all geometries.
     */
    bool GetRandomPosition (const float radius, float& xpos, float& zpos,
			    AreaID& area);

    void MarkAreaUsed (const AreaID& area, 
		       const float radius, const float xpos, const float zpos);
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
      struct Area
      {
	/// Sum of children areas
	float area;
	/// Free box, if node is leaf
	csBox2* box;
	
	Area() : area (0), box (nullptr) {}
      };
      /// Actually a tree of free areas.
      csArray<Area> freeAreas;
      
      Bucket (float minSide) : minSide (minSide) {}
      
      bool operator< (const Bucket& other) const
      { return minSide > other.minSide; }
    };
    csArray<Bucket> buckets;
    /// Insert an area into the right bucket (or discard if too small)
    void InsertNewArea (const csBox2& area);
    /// Add an amount of summed areas to all parents of a node
    void BubbleAreaIncrease (Bucket& bucket, size_t index, float amount);
    
    size_t FindBox (const Bucket& bucket, float pos);
  };
}
CS_PLUGIN_NAMESPACE_END(Engine)

#endif // __CS_MESHGEN_POSITIONMAP_H__
