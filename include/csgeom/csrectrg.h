/*
    Copyright (C) 2001 by Christopher Nelson

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

#ifndef __CS_CSRECTREGION_H__
#define __CS_CSRECTREGION_H__

/**\file 
 * Splittable 2D rectangles.
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"

#include "csgeom/csrect.h"

#ifdef CS_DEBUG     
// defining this causes the csRectRegionDebug class to be compiled in  
//#define CS_DEBUG_RECT_REG 
#endif

#ifdef CS_DEBUG_RECT_REG
  class csRandomGen;
#endif

const int FRAGMENT_BUFFER_SIZE=64;

/**
 * A rect region is a class that implements splittable 2d rectangles.  The
 * region may be composed of one or more rectangles that do not need to occupy
 * the same area.  The idea being that you need to be able to have a number of
 * disparate rectangular regions for updates.  This class uses csRect for
 * nearly all rectangle operations.
 * <p>
 * To use you can Include and Exclude rectangles from this region.  When
 * finished, this class will have a list of optimal rectangles that occupy a
 * region.  If used properly, it will result in faster overall painting
 * performance, since several areas will not require overwriting.
 */
class CS_CRYSTALSPACE_EXPORT csRectRegion
{
protected:
  /// The pointer the list of regions
  csRect* region;
  /// The number of regions currently stored
  int region_count;
  /// The size of the region buffer (in rects)
  int region_max;
  /// The fragment buffer, used for performing fragment operations
  csRect  fragment[FRAGMENT_BUFFER_SIZE];
  /// The gather marker
  int gather_mark;

  /// Pushes a new rect into the region, increases buffer if necessary
  void pushRect(csRect const &);
  /// Removes a rect from the region.
  void deleteRect(int);


  /**
   * Controls fragmentContainedRect, used to perform all-side clipping
   * and edge intersection.
   */
  void fragmentRect(csRect&, csRect&, int mode);
  void nkSplit(csRect& r1, csRect& r2);
  /// Work method fragments rects properly when they intersect.
  void fragmentContainedRect(csRect &r1, csRect &r2);
  /// Marks the current region insertion point for gather.
  void markForGather();
  /// Gathers all regions since the mark into the fragment buffer.
  void gatherFragments();

public:
  /// Constructor.
  csRectRegion();
  /// Destructor
  ~csRectRegion();

  /**
   * Add a rect to this region; may cause unions, but will not adjance
   * (see csRect).
   */
  void Include(const csRect &rect);
  /// Exclude a rect from this region; may cause splitting.
  void Exclude(const csRect &rect);
  /// Clips everything in the region to the borders given.
  void ClipTo(csRect &clip);

  /// Returns the number of rectangles in this region
  inline int Count() const { return region_count; }
  /// Returns the rect at a specific index
  inline csRect& RectAt(int i) const {  return region[i]; }
  /// Resets the region count to zero.
  void makeEmpty();
};



#ifdef CS_DEBUG_RECT_REG

// this class is an alternate implementation of rectRegion 
// it uses a 100 x 100 bool array to represent which pixels
// in that area are contained in a rectangle. This class is
// intended solely to aid in debugging the csRectRegion class.

#define CS_RECT_REG_SIZE 100

class CS_CRYSTALSPACE_EXPORT csRectRegionDebug
{
private:
  bool area[CS_RECT_REG_SIZE][CS_RECT_REG_SIZE];
  csRandomGen* rand;

  // these values ought to help someone with a debugger
  // quickly locate and reproduce failed tests.
  unsigned int rand_seed;
  int num_tests_complete;

public:
  csRectRegionDebug();
  ~csRectRegionDebug();

  /**
   * Add a rect to this region; may cause unions, but will not adjance
   * (see csRect).
   */
  void Include(const csRect &rect);

  /// Exclude a rect from this region
  void Exclude(const csRect &rect);

  /// Clips everything in the region to the borders given.
  void ClipTo(const csRect &clip);
  
  /// Resets the region count to zero.
  void MakeEmpty();

  /**
   * Checks to see if a csRectRegion contains rectangles which exactly
   * represent the area of this region.
   */
  void AssertEqual(const csRectRegion &r);

  /// Ensures that the contents of a rect are within (0,0)-(size-1,size-1)
  bool CheckBounds(const csRect &clip);

  /// Tests the current csRectRegion implmentation
  void UnitTest();

  /// Generates a random rect within the debug region
  csRect RandRect();

  /// Generates a random non-empty rect within the debug region
  csRect RandNonEmptyRect();
};

#endif // CS_DEBUG

/** @} */


#endif // __CS_CSRECTREGION_H__
