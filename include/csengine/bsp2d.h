/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef __CS_BSP2D_H__
#define __CS_BSP2D_H__

#include "csgeom/math2d.h"
#include "csgeom/segment.h"
#include "csengine/arrays.h"
#include "csengine/bsp.h"
#include "csutil/csvector.h"

class csBspTree2D;

/**
 * An dynamic array of csSegment2 objects.
 */
class csSegmentArray : public csVector
{
public:
  /// Create the segment array object
  csSegmentArray (int iLimit, int iDelta) : csVector (iLimit, iDelta)
  { }

  /// Destroy the segment array and all inserted segments.
  virtual ~csSegmentArray ();

  /// Delete a particular array element.
  virtual bool FreeItem (csSome Item)
  {
    delete (csSegment2*)Item;
    return true;
  }

  /// Get a segment given its index in the array.
  csSegment2* Get (int iIndex) const
  {
    return (csSegment2*)csVector::Get (iIndex);
  }

  /// Get the entire array of segments as an array of pointers.
  csSegment2** GetArray () { return (csSegment2**)root; }
};


/**
 * A 2D BSP node.
 */
class csBspNode2D
{
  friend class csBspTree2D;
private:
  /**
   * All the lines in this node.
   * These lines are all on the same line.
   * The 'front' and 'back' children in this node are seperated
   * by that line.
   */
  csSegmentArray segments;

  /// The splitter plane.
  csPlane2 splitter;

  /// The front node.
  csBspNode2D* front;
  /// The back node.
  csBspNode2D* back;

private:
  /// Make an empty BSP node.
  csBspNode2D ();

  /// Destroy this BSP node.
  virtual ~csBspNode2D ();

  /**
   * Add a segment to this BSP node.
   */
  void AddSegment (csSegment2* seg);
};

typedef void* (csTree2DVisitFunc)(csSegment2**, int num, void*);

/**
 * The BSP tree.
 * This bsp tree is ment mostly for adding segments dynamically.
 * As such it does not provide a global build routine. That can
 * of course easily be added if needed.
 */
class csBspTree2D
{
private:
  /// The root.
  csBspNode2D* root;

  /**
   * Add one segment to the tree.
   */
  void Add (csBspNode2D* node, csSegment2* segment);

  /// Traverse the tree from back to front starting at 'node' and 'pos'.
  void* Back2Front (csBspNode2D* node, const csVector2& pos,
  	csTree2DVisitFunc* func, void* data);
  /// Traverse the tree from front to back starting at 'node' and 'pos'.
  void* Front2Back (csBspNode2D* node, const csVector2& pos,
  	csTree2DVisitFunc* func, void* data);

public:
  /**
   * Create an empty tree.
   */
  csBspTree2D ();

  /**
   * Destroy the whole BSP tree.
   */
  virtual ~csBspTree2D ();

  /**
   * Add one segment to the tree. The segment will be freed
   * by this tree at some point. So don't depend on the given pointer
   * anymore.
   */
  void Add (csSegment2* segment);

  /// Traverse the tree from back to front starting at the root and 'pos'.
  void* Back2Front (const csVector2& pos, csTree2DVisitFunc* func, void* data);
  /// Traverse the tree from front to back starting at the root and 'pos'.
  void* Front2Back (const csVector2& pos, csTree2DVisitFunc* func, void* data);
};

#endif // __CS_BSP2D_H__

