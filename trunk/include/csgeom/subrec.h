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

#include "csgeom/csrect.h"

#include "csutil/array.h"
#include "csutil/blockallocator.h"

namespace CS
{

class SubRectanglesCompact;

/**
 * A class managing allocations of sub-rectangles. i.e. this class represents
 * a rectangular region from which a client can allocate smaller rectangles
 * until the region is full.
 * \remarks works best if bigger rectangles are inserted first.
 */
class CS_CRYSTALSPACE_EXPORT SubRectangles : public CS::Memory::CustomAllocated
{
public:
  /**
   * Sub-rectangle
   */
  class SubRect
  {
  private:
    csRect rect;
    csRect allocedRect;
  protected:
    friend class SubRectangles;
    friend class SubRectanglesCompact;
    typedef csBlockAllocator<SubRect> SubRectAlloc;
    friend class csBlockAllocator<SubRect>; // SubRectAlloc

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
    friend struct AllocInfo; // Give MSVC6 access to ALLOC_INVALID.

    int splitPos;
    SplitType splitType;

    SubRectangles* superrect;
    SubRect* parent;
    SubRect* children[2];

    SubRect ();
    SubRect& operator= (const SubRect& other);

    /// Return the area this subrectangle covers
    const csRect& GetRect() const { return rect; }
    /// Return the area allocated from this subrectangle
    const csRect& GetAllocedRect() const { return allocedRect; }
    /// Clear allocated area
    void MakeEmpty ()
    { allocedRect.Set (0, 0, -1, -1); }
    /// Test if allocated area is empty
    bool IsEmpty () const
    { return (allocedRect.xmax < 0) || (allocedRect.ymax < 0); }

    /// searches for the "ideal" position of a rectangle
    void TestAlloc (int w, int h, AllocInfo& ai);
    /// Do the actual allocation.
    SubRect* Alloc (int w, int h, const AllocInfo& ai, csRect& r);
    /// De-allocate
    void Reclaim ();
    bool IsReclaimed() const
    { return IsEmpty() && (splitType == SPLIT_UNSPLIT); }
    /// Test whether both children are empty.
    void TestCollapse ();

    /// Decide whether a H or V split is better.
    /// The better split is the one where the bigger chunk results.
    void DecideBestSplit (const csRect& rect, int splitX, int splitY,
      SubRect::SplitType& splitType);
  };
  friend class SubRect; // Give MSVC6 access to enclosing protected scope.
protected:
  /// Dimensions of this region.
  csRect region;
  /// Root of the region tree
  SubRect* root;

  SubRect::SubRectAlloc alloc;
  inline SubRect* AllocSubrect ()
  { 
    SubRect* sr = alloc.Alloc(); 
    sr->superrect = this;
    return sr;
  }
  void FreeSubrect (SubRect* sr);

  /// Leaves of the region tree
  csArray<SubRect*> leaves;
  static int SubRectCompare (SubRect* const& sr1, SubRect* const& sr2);
  inline void AddLeaf (SubRect* sr)
  {
    leaves.InsertSorted (sr, SubRectCompare);
  }
  void RemoveLeaf (SubRect* sr)
  {
    size_t index = leaves.FindSortedKey (
      csArrayCmp<SubRect*, SubRect*> (sr, SubRectCompare));
    leaves.DeleteIndex (index);
  }

  /// Helper function to split a node.
  void Split (SubRect* subRect, SubRect::SplitType split, int splitPos);
  
  void Grow (SubRect* sr, int ow, int oh, int nw, int nh,
    int touch);
  bool Shrink (SubRect* sr, int ow, int oh, int nw, int nh);
  csRect GetMinimumRectangle (SubRect* sr) const;
  void DupeWithOffset (const SubRect* from, SubRect* to, 
    int x, int y, csHash<SubRect*, csConstPtrKey<SubRect> >* map,
    const csRect& outerAllocated, const csRect& outerRect);
public:
  /// Allocate a new empty region with the given size.
  SubRectangles (const csRect& region);
  SubRectangles (const SubRectangles& other);

  /// Remove this region and sub-regions.
  virtual ~SubRectangles ();

  /// Get the rectangle for this region.
  const csRect& GetRectangle () const { return region; }

  /**
   * Free all rectangles in this region.
   */
  virtual void Clear ();

  /**
   * Returns whether the allocator is empty (ie no rectangles have been 
   * allocated at all or all allocated rectangles have been reclaimed).
   */
  bool IsEmpty() const { return root->IsReclaimed(); }

  /**
   * Allocate a new rectangle. Returns 0 if there is no room
   */
  virtual SubRect* Alloc (int w, int h, csRect& rect);

  /**
   * Reclaim a subrectangle, meaning, the space occupied by the subrect can be
   * reused by subsequent Alloc() calls.
   */
  void Reclaim (SubRect* subrect);

  /**
   * Increase the size of the region.
   * You can only grow upwards.
   */
  virtual bool Grow (int newWidth, int newHeight);

  /**
   * Decrease the size of the region.
   * If the region can't be shrunk to the desired size because some already 
   * allocated subrectangles would be cut off \c false is returned. You
   * can check if shrinking to a size is possible by comparing the result of
   * GetMinimumRectangle() with the desired size.
   */
  virtual bool Shrink (int newWidth, int newHeight);

  /**
   * Return the rectangle to which the allocator can be shrunk to at most.
   */
  csRect GetMinimumRectangle () const
  { return GetMinimumRectangle (root); }

  /**
   * Place the subrectangles of another allocator into a rectangle allocated
   * from this allocator.
   */
  virtual bool PlaceInto (const SubRectangles* rectangles, 
    SubRect* subRect, 
    csHash<SubRect*, csConstPtrKey<SubRect> >* newRectangles = 0);

  /**
   * For debugging: dump all free rectangles.
   * \param object_reg Object registry. Used to obtain some required plugins.
   * \param tag String appended to the filename dumped to.
   */
  void Dump (iObjectRegistry* object_reg, const char* tag = 0);

  /**
   * For debugging: dump all free rectangles.
   * Works the same as Dump(iObjectRegistry*, const char*), although has no 
   * effect Crystal Space was not compiled in debug mode.
   */
  void Dump (const char* tag = 0);
};

/**
 * A variation of SubRectangles that tries to place rectangles in a rectangular
 * fashion. This means all allocated rectangles are attempted to be placed 
 * compactly in a rectangle in the upper left corner, where as the normal
 * SubRectangles quickly "expands" in the X and Y directions.
 *
 * This variation is useful if the rectangle you fill is an upper limit but the
 * covered area should be as small as possible. It has slightly more overhead 
 * than the normal SubRectangles, though.
 *
 * \remarks works best if bigger rectangles are inserted first. The malus from
 *  not doing so is bigger than the CS::SubRectangles one.
 */
class CS_CRYSTALSPACE_EXPORT SubRectanglesCompact : public SubRectangles
{
  const csRect maxArea;
  bool growPO2;

  inline int NewSize (int amount, int inc)
  { return growPO2 ? csFindNearestPowerOf2 (amount + inc) : amount + inc; }
public:
  SubRectanglesCompact (const csRect& maxArea);
  SubRectanglesCompact (const SubRectanglesCompact& other);

  void Clear ();
  SubRect* Alloc (int w, int h, csRect& rect);

  /// Return the upper limit of the rectangle.
  const csRect& GetMaximumRectangle () const { return maxArea; }

  /**
   * Enable growing to PO2 dimensions. Means that if an enlargement
   * of the rectangle is necessary it will be to a PO2 dimension.
   * Useful when e.g. the rectangle is to be used as a texture.
   */
  void SetGrowPO2 (bool growPO2) { this->growPO2 = growPO2; }
  /// Return whether growing to PO2 dimensions is enabled.
  bool GetGrowPO2 () const { return growPO2; }

  /// Allocate a rectangle but don't attempt growing to fit.
  SubRect* AllocNoGrow (int w, int h, csRect& rect)
  {
    return SubRectangles::Alloc (w, h, rect);
  }
};

} // namespace CS

typedef CS_DEPRECATED_TYPE_MSG("csSubRectangles renamed to CS::SubRectangles")
  CS::SubRectangles csSubRectangles;
typedef CS_DEPRECATED_TYPE_MSG("csSubRect renamed to CS::SubRectangles::SubRect")
  CS::SubRectangles::SubRect csSubRect;

/** @} */

#endif // __CS_SUBREC_H__

