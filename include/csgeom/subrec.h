/*
    Copyright (C) 2001-2005 by Jorrit Tyberghein
		  2003-2005 by Frank Richter

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

#ifndef __CS_SUBREC_H__
#define __CS_SUBREC_H__

/**\file 
 * Stuff small rectangles into a bigger one
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"

#include "csutil/array.h"
#include "csutil/blockallocator.h"
#include "csgeom/csrect.h"

/**
 * A class managing allocations of sub-rectangles. i.e. this class represents
 * a rectangular region from which a client can allocate smaller rectangles
 * until the region is full.
 */
class CS_CRYSTALSPACE_EXPORT csSubRectangles
{
public:
  /**
  * Sub-rectangle
  */
  class SubRect
  {
  protected:
    friend class csSubRectangles;
    typedef csBlockAllocator<SubRect> SubRectAlloc;
    friend class SubRectAlloc;

    enum SplitType
    {
      SPLIT_UNSPLIT,
      SPLIT_H,
      SPLIT_V
    };
    enum AllocPos
    {
      ALLOC_INVALID = -1,
      ALLOC_RIGHT,
      ALLOC_BELOW,
      ALLOC_NEW
    };
    struct AllocInfo
    {
      SubRect* node;
      int d;
      AllocPos allocPos;
      bool res;
      
      AllocInfo() : node(0), d(0x7fffffff), allocPos(ALLOC_INVALID), 
	res(false) {};
    };

    csRect rect;
    csRect allocedRect;
    int splitPos;
    SplitType splitType;

    csSubRectangles* superrect;
    SubRect* parent;
    SubRect* children[2];

    SubRect ();

    /// searches for the "ideal" position of a rectangle
    void TestAlloc (int w, int h, AllocInfo& ai);
    /// Do the actual allocation.
    SubRect* Alloc (int w, int h, const AllocInfo& ai, csRect& r);
    /// De-allocate
    void Reclaim ();
    /// Test whether both children are empty.
    void TestCollapse ();

    /// Decide whether a H or V split is better.
    /// The better split is the one where the bigger chunk results.
    void DecideBestSplit (const csRect& rect, int splitX, int splitY,
      SubRect::SplitType& splitType);
  };

protected:
  /// Dimensions of this region.
  csRect region;
  /// Root of the region tree
  SubRect* root;

  SubRect::SubRectAlloc alloc;
  inline SubRect* AllocSubrect ()
  { return alloc.Alloc(); }
  void FreeSubrect (SubRect* sr);

  /// Leaves of the region tree
  csArray<SubRect*> leaves;
  void AddLeaf (SubRect* sr)
  {
    leaves.InsertSorted (sr);
  }
  void RemoveLeaf (SubRect* sr)
  {
    size_t index = leaves.FindSortedKey (
      csArrayCmp<SubRect*, SubRect*> (sr));
    leaves.DeleteIndex (index);
  }
  
  void Grow (SubRect* sr, int ow, int oh, int nw, int nh);
public:
  /// Allocate a new empty region with the given size.
  csSubRectangles (const csRect& region);

  /// Remove this region and sub-regions.
  ~csSubRectangles ();

  /// Get the rectangle for this region.
  const csRect& GetRectangle () const { return region; }

  /**
   * Free all rectangles in this region.
   */
  void Clear ();

  /**
   * Allocate a new rectangle. Returns 0 if there is no room
   */
  SubRect* Alloc (int w, int h, csRect& rect);

  /**
   * Reclaim a subrectangle, meaning, the space occupied by the subrect can be
   * reused by subsequent Alloc() calls.
   */
  void Reclaim (SubRect* subrect);

  /**
   * Increase the size of the region.
   * You can only grow upwards.
   */
  bool Grow (int newWidth, int newHeight);

  /**
   * For debugging: dump all free rectangles.
   */
  void Dump ();
};

typedef csSubRectangles::SubRect csSubRect;

/** @} */

#endif // __CS_SUBREC_H__

