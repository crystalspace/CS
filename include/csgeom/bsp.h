/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef BSP_H
#define BSP_H

#include "csgeom/math3d.h"

class csPolygonInt;
class csPolygonParentInt;
class csBspTree;
class Dumper;

/**
 * A BSP node.
 */
class csBspNode
{
  ///
  friend class csBspTree;
  friend class Dumper;

private:
  /// All the polygons in this node.
  csPolygonInt** polygons;
  ///
  int num;
  ///
  int max;
  /// The front node.
  csBspNode* front;
  /// The back node.
  csBspNode* back;

private:
  /// Make an empty BSP node.
  csBspNode ();
  /**
   * Destroy this BSP node. The list of polygons
   * will be deleted but not the polygons themselves.
   */
  ~csBspNode ();

  /// Add a polygon to this BSP node.
  void AddPolygon (csPolygonInt* poly);
};

/**
 * Visit a node in the BSP tree. If this function returns non-NULL
 * the scanning will stop and the pointer will be returned.
 */
typedef void* (csBspVisitFunc)(csPolygonParentInt*, csPolygonInt**, int num, void*);

/**
 * The BSP tree.
 */
class csBspTree
{
  friend class Dumper;

private:
  /// The root of the tree.
  csBspNode* root;

  /// The parent that this tree is made for.
  csPolygonParentInt* pset;

  /// Build the tree from the given node and number of polygons.
  void Build (csBspNode* node, csPolygonInt** polygons, int num);

  /// Clear the node.
  void Clear (csBspNode* node);

  /// Traverse the tree from back to front starting at 'node' and 'pos'.
  void* Back2Front (csBspNode* node, const csVector3& pos, csBspVisitFunc* func, void* data);
  /// Traverse the tree from front to back starting at 'node' and 'pos'.
  void* Front2Back (csBspNode* node, const csVector3& pos, csBspVisitFunc* func, void* data);

public:
  /**
   * Create the tree for the given parent. The polygons are fetched
   * from the parent.
   */
  csBspTree (csPolygonParentInt* pset);

  /// Create the tree with a given set of polygons.
  csBspTree (csPolygonParentInt* pset, csPolygonInt** polygons, int num);

  /**
   * Destroy the whole BSP tree (but not the actual polygons and parent
   * objects).
   */
  ~csBspTree ();

  /// Traverse the tree from back to front starting at the root and 'pos'.
  void* Back2Front (const csVector3& pos, csBspVisitFunc* func, void* data);
  /// Traverse the tree from front to back starting at the root and 'pos'.
  void* Front2Back (const csVector3& pos, csBspVisitFunc* func, void* data);

  /// Get the polygonset for this BSP tree.
  csPolygonParentInt* GetParent () { return pset; }
};

#endif /*BSP_H*/

