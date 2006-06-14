/*
    Copyright (C) 2005 by Jorrit Tyberghein

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
#ifndef __CS_ENGINETOOLS_H__
#define __CS_ENGINETOOLS_H__

/**\file
 * Helper functions for working on engine data.
 */

#include "csextern.h"

#include "csutil/set.h"
#include "csutil/array.h"
#include "csgeom/vector3.h"

struct iSector;

/**
 * Result structure for csEngineTools::FindShortestDistance().
 */
struct csShortestDistanceResult
{
  /**
   * Squared distance between the two points or negative if the distance
   * goes beyond the maximum radius.
   */
  float sqdistance;

  /**
   * This is a direction towards the destination point but corrected
   * with space warping portals. That means that if you go from the
   * start position in the direction returned here you will end up at
   * the destination point.
   */
  csVector3 direction;
};

/**
 * This is a class with static helper functions for working on engine
 * data.
 */
class CS_CRYSTALSPACE_EXPORT csEngineTools
{
private:
  static float FindShortestDistance (
  	const csVector3& source, iSector* sourceSector,
  	const csVector3& dest, iSector* destSector,
  	float maxradius,
	csSet<csPtrKey<iSector> >& visited_sectors,
	csVector3& direction,
	bool accurate);

public:
  /**
   * Given two positions in the world, try to find the shortest distance (using
   * portals if needed) between them and return the final squared distance.
   * <p>
   * Note! This function will ignore all portals if the source and destination
   * sectors are the same. Even if there might be a possible shorter path
   * between the two positions using some space warping portal. An exception
   * to this is if the distance is greater then the max distance. In that
   * case this function will attempt to try out portals in the current sector
   * to see if there is a shorter path anyway.
   * <p>
   * Note that this routine will ignore visibility. It will simply calculate
   * the distance between the two points through some portal path. However,
   * this function will check if the portal is oriented towards the source
   * point (i.e it doesn't ignore visibility with regards to backface culling).
   * <p>
   * Note that this function (by default) only considers the center point
   * of a portal for calculating the distance. This might skew results with
   * very big portals. Set the 'accurate' parameter to true if you don't
   * want this.
   * <p>
   * This function will correctly account for space warping portals.
   * <p>
   * \param source is the source position to start from.
   * \param sourceSector is the sector for that position.
   * \param dest is the destination position to start from.
   * \param destSector is the destination for that position.
   * \param maxradius is the maximum radius before we stop recursion.
   * \param accurate if this is true then this routine will use a more
   * accurate distance test to see if the portal is close enough. With
   * small portals this is probably rarely needed but if you need to be
   * certain to find all cases then you can set this to true.
   * \return an instance of csShortestDistanceResult which contains
   * the squared distance between the two points or a negative
   * number if the distance goes beyond the maximum radius. It also
   * contains a space-warping corrected direction from the source point
   * to the destination point.
   */
  static csShortestDistanceResult FindShortestDistance (
  	const csVector3& source, iSector* sourceSector,
  	const csVector3& dest, iSector* destSector,
  	float maxradius, bool accurate = false);
};

#endif // __CS_ENGINETOOLS_H__
